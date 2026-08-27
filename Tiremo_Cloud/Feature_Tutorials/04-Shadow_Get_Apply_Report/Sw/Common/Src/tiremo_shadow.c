/******************************************************************************
 * @file    tiremo_shadow.c
 * @brief   One-shot Device Shadow get / apply / report (no live delta).
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Include/tiremo_shadow.h"
#include "../Include/tiremo_json.h"
#include "../Include/tiremo_debug.h"
#include "../../Drivers/esp32/Include/tiremo_esp32.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SHADOW_TOPIC_MAX      (192U)
#define SHADOW_RESPONSE_MAX   (8192U)
#define SHADOW_CHUNK_MAX      (2048U)
#define SHADOW_REPORT_MAX     (4096U)
#define SHADOW_TIMEOUT_MS     (20000U)
#define SHADOW_GET_TOKEN      "tiremo-boot-get"
#define SHADOW_UPD_TOKEN      "tiremo-boot-report"

static char s_response[SHADOW_RESPONSE_MAX];
static char s_chunk[SHADOW_CHUNK_MAX];
static char s_report[SHADOW_REPORT_MAX];
static char s_topicGet[SHADOW_TOPIC_MAX];
static char s_topicGetAcc[SHADOW_TOPIC_MAX];
static char s_topicGetRej[SHADOW_TOPIC_MAX];
static char s_topicUpdate[SHADOW_TOPIC_MAX];
static char s_topicUpdAcc[SHADOW_TOPIC_MAX];
static char s_topicUpdRej[SHADOW_TOPIC_MAX];

/**
 * @brief True when p[0..len) is one complete JSON object (brace depth 0,
 *        strings skipped). Same rule as fleet provisioning: ESP-AT's
 *        MQTT buffer is 1024 bytes, so a get/accepted can arrive as
 *        several +MQTTSUBRECV URCs on the same topic.
 */
static bool prvJsonObjectComplete(const char *pJson, uint32_t jsonLen)
{
    uint32_t i;
    uint32_t depth = 0U;
    bool inString = false;
    bool escape = false;

    if ((pJson == NULL) || (jsonLen == 0U) || (pJson[0] != '{'))
    {
        return false;
    }

    for (i = 0U; i < jsonLen; i++)
    {
        char c = pJson[i];
        if (inString)
        {
            if (escape)
            {
                escape = false;
            }
            else if (c == '\\')
            {
                escape = true;
            }
            else if (c == '"')
            {
                inString = false;
            }
            continue;
        }
        if (c == '"')
        {
            inString = true;
        }
        else if (c == '{')
        {
            depth++;
        }
        else if (c == '}')
        {
            if (depth == 0U)
            {
                return false;
            }
            depth--;
            if (depth == 0U)
            {
                i++;
                while (i < jsonLen)
                {
                    char w = pJson[i];
                    if ((w != ' ') && (w != '\t') && (w != '\r') &&
                        (w != '\n'))
                    {
                        return false;
                    }
                    i++;
                }
                return true;
            }
        }
    }
    return false;
}

static bool prvBuildTopics(const char *pThingName)
{
    int n;

    n = snprintf(s_topicGet, sizeof(s_topicGet),
                 "$aws/things/%s/shadow/get", pThingName);
    if ((n <= 0) || ((uint32_t)n >= sizeof(s_topicGet)))
    {
        return false;
    }
    n = snprintf(s_topicGetAcc, sizeof(s_topicGetAcc),
                 "$aws/things/%s/shadow/get/accepted", pThingName);
    if ((n <= 0) || ((uint32_t)n >= sizeof(s_topicGetAcc)))
    {
        return false;
    }
    n = snprintf(s_topicGetRej, sizeof(s_topicGetRej),
                 "$aws/things/%s/shadow/get/rejected", pThingName);
    if ((n <= 0) || ((uint32_t)n >= sizeof(s_topicGetRej)))
    {
        return false;
    }
    n = snprintf(s_topicUpdate, sizeof(s_topicUpdate),
                 "$aws/things/%s/shadow/update", pThingName);
    if ((n <= 0) || ((uint32_t)n >= sizeof(s_topicUpdate)))
    {
        return false;
    }
    n = snprintf(s_topicUpdAcc, sizeof(s_topicUpdAcc),
                 "$aws/things/%s/shadow/update/accepted", pThingName);
    if ((n <= 0) || ((uint32_t)n >= sizeof(s_topicUpdAcc)))
    {
        return false;
    }
    n = snprintf(s_topicUpdRej, sizeof(s_topicUpdRej),
                 "$aws/things/%s/shadow/update/rejected", pThingName);
    return (n > 0) && ((uint32_t)n < sizeof(s_topicUpdRej));
}

/**
 * @brief Reassemble fragments on pAccepted. pRejected is a complete
 *        (usually small) error document.
 *
 * @return  1 accepted (complete JSON in s_response)
 *          0 rejected (error JSON in s_response)
 *         -1 timeout / overflow
 */
static int prvWaitAcceptedOrRejected(const char *pAccepted,
                                     const char *pRejected,
                                     uint32_t *pLength)
{
    char topic[SHADOW_TOPIC_MAX];
    uint32_t fragLen = 0U;
    uint32_t accum = 0U;

    s_response[0] = '\0';

    for (;;)
    {
        if (!TIREMO_ESP32_MqttWaitMessage(
                topic, sizeof(topic), (uint8_t *)s_chunk,
                sizeof(s_chunk) - 1U, &fragLen, SHADOW_TIMEOUT_MS))
        {
            TIREMO_LOG_ERROR("shadow: MQTT wait timed out");
            return -1;
        }
        if (fragLen >= sizeof(s_chunk))
        {
            TIREMO_LOG_ERROR("shadow: fragment too large (%lu)",
                             (unsigned long)fragLen);
            return -1;
        }
        s_chunk[fragLen] = '\0';

        if (strcmp(topic, pRejected) == 0)
        {
            if (fragLen >= sizeof(s_response))
            {
                TIREMO_LOG_ERROR("shadow: rejected payload too large");
                return -1;
            }
            memcpy(s_response, s_chunk, fragLen);
            s_response[fragLen] = '\0';
            if (pLength != NULL)
            {
                *pLength = fragLen;
            }
            TIREMO_LOG_WARN("shadow: rejected %s", s_response);
            return 0;
        }
        if (strcmp(topic, pAccepted) != 0)
        {
            continue;
        }

        if ((fragLen > 0U) && (s_chunk[0] == '{'))
        {
            accum = 0U;
        }
        if ((accum + fragLen) >= sizeof(s_response))
        {
            TIREMO_LOG_ERROR("shadow: reassembled payload too large");
            return -1;
        }
        memcpy(s_response + accum, s_chunk, fragLen);
        accum += fragLen;
        s_response[accum] = '\0';

        TIREMO_LOG_INFO("shadow: MQTT RX topic=%s frag=%lu accum=%lu",
                        topic, (unsigned long)fragLen, (unsigned long)accum);

        if (prvJsonObjectComplete(s_response, accum))
        {
            if (pLength != NULL)
            {
                *pLength = accum;
            }
            return 1;
        }
    }
}

static void prvApplyLocal(const char *pDesired, uint32_t desiredLen)
{
    bool enabled = true;
    uint32_t intervalS = 60U;
    bool ledOn = false;
    const char *pRep = NULL;
    uint32_t repLen = 0U;
    const char *pLed = NULL;
    uint32_t ledLen = 0U;

    if ((pDesired != NULL) && (desiredLen >= 2U))
    {
        if (TIREMO_Json_FindObject(pDesired, desiredLen, "reporting",
                                   &pRep, &repLen))
        {
            (void)TIREMO_Json_FindBool(pRep, repLen, "enabled", &enabled);
            (void)TIREMO_Json_FindUint(pRep, repLen, "intervalS", &intervalS);
        }
        if (TIREMO_Json_FindObject(pDesired, desiredLen, "led",
                                   &pLed, &ledLen))
        {
            (void)TIREMO_Json_FindBool(pLed, ledLen, "on", &ledOn);
        }
    }

    TIREMO_LOG_INFO("shadow: applied reporting.enabled=%s intervalS=%lu led.on=%s",
                    enabled ? "true" : "false",
                    (unsigned long)intervalS,
                    ledOn ? "true" : "false");
}

static bool prvWriteDefaults(TIREMO_JsonWriter_t *pW)
{
    return TIREMO_JsonWriter_ObjectStart(pW) &&
           TIREMO_JsonWriter_Key(pW, "reporting") &&
           TIREMO_JsonWriter_ObjectStart(pW) &&
           TIREMO_JsonWriter_Key(pW, "enabled") &&
           TIREMO_JsonWriter_Bool(pW, true) &&
           TIREMO_JsonWriter_Key(pW, "intervalS") &&
           TIREMO_JsonWriter_Uint(pW, 60U) &&
           TIREMO_JsonWriter_ObjectEnd(pW) &&
           TIREMO_JsonWriter_Key(pW, "led") &&
           TIREMO_JsonWriter_ObjectStart(pW) &&
           TIREMO_JsonWriter_Key(pW, "on") &&
           TIREMO_JsonWriter_Bool(pW, false) &&
           TIREMO_JsonWriter_ObjectEnd(pW) &&
           TIREMO_JsonWriter_ObjectEnd(pW);
}

static bool prvBuildReport(const char *pDesired, uint32_t desiredLen)
{
    TIREMO_JsonWriter_t w;

    TIREMO_JsonWriter_Init(&w, s_report, sizeof(s_report));
    if (!TIREMO_JsonWriter_ObjectStart(&w) ||
        !TIREMO_JsonWriter_Key(&w, "state") ||
        !TIREMO_JsonWriter_ObjectStart(&w) ||
        !TIREMO_JsonWriter_Key(&w, "reported"))
    {
        return false;
    }

    /* Echo the whole desired object, nested reporting/led included.
     * Publishing only the key that changed leaves a permanent delta. */
    if ((pDesired != NULL) && (desiredLen >= 2U))
    {
        if (!TIREMO_JsonWriter_Raw(&w, pDesired, desiredLen))
        {
            return false;
        }
    }
    else if (!prvWriteDefaults(&w))
    {
        return false;
    }

    /* state object ends here; clientToken is a sibling of state. */
    if (!TIREMO_JsonWriter_ObjectEnd(&w) ||
        !TIREMO_JsonWriter_Key(&w, "clientToken") ||
        !TIREMO_JsonWriter_String(&w, SHADOW_UPD_TOKEN) ||
        !TIREMO_JsonWriter_ObjectEnd(&w))
    {
        return false;
    }
    return (TIREMO_JsonWriter_Finish(&w) > 0U);
}

static bool prvGetOnce(uint32_t *pLength, bool *pNoShadow)
{
    int wait;
    uint32_t code = 0U;

    *pNoShadow = false;
    if (!TIREMO_ESP32_MqttPublishRaw(
            s_topicGet, (const uint8_t *)"{\"clientToken\":\"" SHADOW_GET_TOKEN "\"}",
            (uint32_t)strlen("{\"clientToken\":\"" SHADOW_GET_TOKEN "\"}"),
            1U, false))
    {
        TIREMO_LOG_ERROR("shadow: get publish failed");
        return false;
    }

    wait = prvWaitAcceptedOrRejected(s_topicGetAcc, s_topicGetRej, pLength);
    if (wait < 0)
    {
        return false;
    }
    if (wait == 0)
    {
        if (TIREMO_Json_FindUint(s_response, *pLength, "code", &code) &&
            (code == 404U))
        {
            TIREMO_LOG_INFO("shadow: no document yet (404); will report defaults");
            *pNoShadow = true;
            return true;
        }
        TIREMO_LOG_ERROR("shadow: get rejected (is thingName correct?)");
        return false;
    }
    return true;
}

static bool prvConfirmNoDelta(void)
{
    uint32_t length = 0U;
    bool noShadow = false;
    const char *pState = NULL;
    uint32_t stateLen = 0U;
    const char *pDelta = NULL;
    uint32_t deltaLen = 0U;

    TIREMO_LOG_INFO("shadow: get again to confirm delta is gone");
    if (!prvGetOnce(&length, &noShadow))
    {
        return false;
    }
    if (noShadow)
    {
        TIREMO_LOG_ERROR("shadow: still no document after report");
        return false;
    }
    if (!TIREMO_Json_FindObject(s_response, length, "state",
                                &pState, &stateLen))
    {
        TIREMO_LOG_ERROR("shadow: confirm get has no state object");
        return false;
    }
    if (TIREMO_Json_FindObject(pState, stateLen, "delta",
                               &pDelta, &deltaLen))
    {
        TIREMO_LOG_ERROR("shadow: leftover delta (%lu bytes) -- reported did not match desired",
                         (unsigned long)deltaLen);
        return false;
    }
    TIREMO_LOG_INFO("shadow: no leftover delta");
    return true;
}

bool TIREMO_Shadow_GetApplyReport(const char *pThingName)
{
    uint32_t length = 0U;
    bool noShadow = false;
    const char *pState = NULL;
    uint32_t stateLen = 0U;
    const char *pDesired = NULL;
    uint32_t desiredLen = 0U;
    int wait;

    if ((pThingName == NULL) || (pThingName[0] == '\0') ||
        !prvBuildTopics(pThingName))
    {
        TIREMO_LOG_ERROR("shadow: bad thingName");
        return false;
    }

    TIREMO_LOG_INFO("shadow: thingName=%s", pThingName);
    TIREMO_LOG_INFO("shadow: subscribe get/accepted get/rejected (no update/delta)");

    if (!TIREMO_ESP32_MqttSubscribe(s_topicGetAcc, 1U) ||
        !TIREMO_ESP32_MqttSubscribe(s_topicGetRej, 1U) ||
        !TIREMO_ESP32_MqttSubscribe(s_topicUpdAcc, 1U) ||
        !TIREMO_ESP32_MqttSubscribe(s_topicUpdRej, 1U))
    {
        TIREMO_LOG_ERROR("shadow: subscribe failed");
        return false;
    }

    TIREMO_LOG_INFO("shadow: get");
    if (!prvGetOnce(&length, &noShadow))
    {
        return false;
    }

    if ((!noShadow) &&
        TIREMO_Json_FindObject(s_response, length, "state",
                               &pState, &stateLen))
    {
        if (TIREMO_Json_FindObject(pState, stateLen, "desired",
                                   &pDesired, &desiredLen))
        {
            TIREMO_LOG_INFO("shadow: desired %lu bytes",
                            (unsigned long)desiredLen);
        }
        else
        {
            TIREMO_LOG_INFO("shadow: no desired; reporting defaults");
            pDesired = NULL;
            desiredLen = 0U;
        }
    }
    else
    {
        pDesired = NULL;
        desiredLen = 0U;
    }

    prvApplyLocal(pDesired, desiredLen);
    if (!prvBuildReport(pDesired, desiredLen))
    {
        TIREMO_LOG_ERROR("shadow: report JSON build failed");
        return false;
    }

    TIREMO_LOG_INFO("shadow: update reported (%lu bytes, full desired echo)",
                    (unsigned long)strlen(s_report));
    if (!TIREMO_ESP32_MqttPublishRaw(
            s_topicUpdate, (const uint8_t *)s_report,
            (uint32_t)strlen(s_report), 1U, false))
    {
        TIREMO_LOG_ERROR("shadow: update publish failed");
        return false;
    }

    wait = prvWaitAcceptedOrRejected(s_topicUpdAcc, s_topicUpdRej, &length);
    if (wait <= 0)
    {
        TIREMO_LOG_ERROR("shadow: update not accepted");
        return false;
    }
    TIREMO_LOG_INFO("shadow: update accepted");

    return prvConfirmNoDelta();
}
