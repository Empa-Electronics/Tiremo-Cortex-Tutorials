/******************************************************************************
 * @file    tiremo_shadow.c
 * @brief   Device Shadow get / apply / report plus live update/delta.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Include/tiremo_shadow.h"
#include "../Include/tiremo_json.h"
#include "../Include/tiremo_jobs.h"
#include "../Include/tiremo_telemetry.h"
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
#define SHADOW_DESIRED_MAX    (4096U)
#define SHADOW_JSON_KEY_MAX   (64U)
#define SHADOW_TIMEOUT_MS     (20000U)
#define SHADOW_POLL_MS        (200U)
#define SHADOW_GET_TOKEN      "tiremo-boot-get"
#define SHADOW_UPD_TOKEN      "tiremo-boot-report"
#define SHADOW_DELTA_TOKEN    "tiremo-delta-report"

static char s_response[SHADOW_RESPONSE_MAX];
static char s_chunk[SHADOW_CHUNK_MAX];
static char s_report[SHADOW_REPORT_MAX];
static char s_desired[SHADOW_DESIRED_MAX];
static uint32_t s_desiredLen = 0U;
static char s_lastEcho[SHADOW_DESIRED_MAX];
static uint32_t s_lastEchoLen = 0U;
static char s_topicGet[SHADOW_TOPIC_MAX];
static char s_topicGetAcc[SHADOW_TOPIC_MAX];
static char s_topicGetRej[SHADOW_TOPIC_MAX];
static char s_topicUpdate[SHADOW_TOPIC_MAX];
static char s_topicUpdAcc[SHADOW_TOPIC_MAX];
static char s_topicUpdRej[SHADOW_TOPIC_MAX];
static char s_topicDelta[SHADOW_TOPIC_MAX];

static bool s_accEn = true;
static bool s_gyroEn = false;
static bool s_tempEn = true;
static bool s_humEn = true;
static uint32_t s_intervalS = 60U;
static bool s_deltaReady = false;
static uint32_t s_deltaAccum = 0U;

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
    if ((n <= 0) || ((uint32_t)n >= sizeof(s_topicUpdRej)))
    {
        return false;
    }
    n = snprintf(s_topicDelta, sizeof(s_topicDelta),
                 "$aws/things/%s/shadow/update/delta", pThingName);
    return (n > 0) && ((uint32_t)n < sizeof(s_topicDelta));
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

/**
 * @brief Apply known fields from a desired or delta `state` object.
 *
 * Missing keys keep the previous applied values (overlay). A boot get
 * that has no desired resets to defaults first by calling this with
 * pObj == NULL.
 */
/**
 * @brief Read one enable flag as JSON bool or 0/1.
 *
 * Tiremo UI State Controller writes `true` / `false`. The tutorial
 * schema also allows integer 0/1.
 */
static bool prvApplyFlag(const char *pObj, uint32_t objLen,
                         const char *pKey, bool *pOut)
{
    bool asBool;
    uint32_t asUint;

    if (TIREMO_Json_FindBool(pObj, objLen, pKey, &asBool))
    {
        *pOut = asBool;
        return true;
    }
    if (TIREMO_Json_FindUint(pObj, objLen, pKey, &asUint))
    {
        *pOut = (asUint != 0U);
        return true;
    }
    return false;
}

static void prvApplyLocal(const char *pObj, uint32_t objLen, bool resetToDefaults)
{
    const char *pRep = NULL;
    uint32_t repLen = 0U;

    if (resetToDefaults)
    {
        s_accEn = true;
        s_gyroEn = false;
        s_tempEn = true;
        s_humEn = true;
        s_intervalS = 60U;
    }

    if ((pObj != NULL) && (objLen >= 2U))
    {
        (void)prvApplyFlag(pObj, objLen, "ACC_en", &s_accEn);
        (void)prvApplyFlag(pObj, objLen, "GYRO_en", &s_gyroEn);
        (void)prvApplyFlag(pObj, objLen, "TEMP_en", &s_tempEn);
        (void)prvApplyFlag(pObj, objLen, "HUM_en", &s_humEn);
        if (TIREMO_Json_FindObject(pObj, objLen, "reporting",
                                   &pRep, &repLen))
        {
            (void)TIREMO_Json_FindUint(pRep, repLen, "intervalS", &s_intervalS);
        }
    }

    TIREMO_LOG_INFO("shadow: applied ACC_en=%u GYRO_en=%u TEMP_en=%u HUM_en=%u intervalS=%lu",
                    s_accEn ? 1U : 0U,
                    s_gyroEn ? 1U : 0U,
                    s_tempEn ? 1U : 0U,
                    s_humEn ? 1U : 0U,
                    (unsigned long)s_intervalS);
}

static bool prvWriteApplied(TIREMO_JsonWriter_t *pW)
{
    return TIREMO_JsonWriter_ObjectStart(pW) &&
           TIREMO_JsonWriter_Key(pW, "ACC_en") &&
           TIREMO_JsonWriter_Bool(pW, s_accEn) &&
           TIREMO_JsonWriter_Key(pW, "GYRO_en") &&
           TIREMO_JsonWriter_Bool(pW, s_gyroEn) &&
           TIREMO_JsonWriter_Key(pW, "TEMP_en") &&
           TIREMO_JsonWriter_Bool(pW, s_tempEn) &&
           TIREMO_JsonWriter_Key(pW, "HUM_en") &&
           TIREMO_JsonWriter_Bool(pW, s_humEn) &&
           TIREMO_JsonWriter_ObjectEnd(pW);
}

static bool prvBuildReport(const char *pDesired, uint32_t desiredLen,
                           const char *pToken)
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

    /* Always echo the whole desired object when we have one -- including
     * keys this firmware does not apply. Rebuilding only reporting+led
     * leaves those siblings in AWS's delta and storms update/delta. */
    if ((pDesired != NULL) && (desiredLen >= 2U))
    {
        if (!TIREMO_JsonWriter_Raw(&w, pDesired, desiredLen))
        {
            return false;
        }
    }
    else if (!prvWriteApplied(&w))
    {
        return false;
    }

    if (!TIREMO_JsonWriter_ObjectEnd(&w) ||
        !TIREMO_JsonWriter_Key(&w, "clientToken") ||
        !TIREMO_JsonWriter_String(&w, pToken) ||
        !TIREMO_JsonWriter_ObjectEnd(&w))
    {
        return false;
    }
    return (TIREMO_JsonWriter_Finish(&w) > 0U);
}

static bool prvPublishReport(const char *pToken)
{
    TIREMO_LOG_INFO("shadow: update reported (%lu bytes, full desired echo)",
                    (unsigned long)strlen(s_report));
    (void)pToken;
    if (!TIREMO_ESP32_MqttPublishRaw(
            s_topicUpdate, (const uint8_t *)s_report,
            (uint32_t)strlen(s_report), 1U, false))
    {
        TIREMO_LOG_ERROR("shadow: update publish failed");
        return false;
    }
    return true;
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

static bool prvCacheDesired(const char *pDesired, uint32_t desiredLen)
{
    if ((pDesired == NULL) || (desiredLen < 2U) ||
        (desiredLen >= sizeof(s_desired)))
    {
        s_desiredLen = 0U;
        return false;
    }
    memcpy(s_desired, pDesired, desiredLen);
    s_desired[desiredLen] = '\0';
    s_desiredLen = desiredLen;
    return true;
}

static bool prvKeyCstr(const char *pKey, uint32_t keyLen,
                       char *pOut, uint32_t outSize)
{
    if ((pKey == NULL) || (pOut == NULL) || ((keyLen + 1U) >= outSize))
    {
        return false;
    }
    memcpy(pOut, pKey, keyLen);
    pOut[keyLen] = '\0';
    return true;
}

static bool prvIsNull(const char *pVal, uint32_t valLen)
{
    const char *pEnd;
    const char *p;

    if ((pVal == NULL) || (valLen == 0U))
    {
        return false;
    }
    pEnd = pVal + valLen;
    p = pVal;
    while ((p < pEnd) &&
           ((*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n')))
    {
        p++;
    }
    if (((uint32_t)(pEnd - p) < 4U) ||
        (p[0] != 'n') || (p[1] != 'u') || (p[2] != 'l') || (p[3] != 'l'))
    {
        return false;
    }
    p += 4;
    while ((p < pEnd) &&
           ((*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n')))
    {
        p++;
    }
    return (p == pEnd);
}

static bool prvIsObject(const char *pVal, uint32_t valLen)
{
    const char *pEnd;
    const char *p;

    if ((pVal == NULL) || (valLen == 0U))
    {
        return false;
    }
    pEnd = pVal + valLen;
    p = pVal;
    while ((p < pEnd) &&
           ((*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n')))
    {
        p++;
    }
    return (p < pEnd) && (*p == '{');
}

static bool prvWriteMergedObject(TIREMO_JsonWriter_t *pW,
                                 const char *pBase, uint32_t baseLen,
                                 const char *pOver, uint32_t overLen);

static bool prvWriteMergedValue(TIREMO_JsonWriter_t *pW,
                                const char *pBaseVal, uint32_t baseValLen,
                                const char *pOverVal, uint32_t overValLen)
{
    if (pOverVal == NULL)
    {
        return TIREMO_JsonWriter_Raw(pW, pBaseVal, baseValLen);
    }
    if (prvIsNull(pOverVal, overValLen))
    {
        return true;
    }
    if ((pBaseVal != NULL) && prvIsObject(pBaseVal, baseValLen) &&
        prvIsObject(pOverVal, overValLen))
    {
        return prvWriteMergedObject(pW, pBaseVal, baseValLen,
                                    pOverVal, overValLen);
    }
    return TIREMO_JsonWriter_Raw(pW, pOverVal, overValLen);
}

typedef struct
{
    TIREMO_JsonWriter_t *pW;
    const char *pOver;
    uint32_t overLen;
    bool ok;
} MergeBaseCtx_t;

typedef struct
{
    TIREMO_JsonWriter_t *pW;
    const char *pBase;
    uint32_t baseLen;
    bool ok;
} MergeOverCtx_t;

static bool prvMergeBaseKey(const char *pKey, uint32_t keyLen,
                            const char *pVal, uint32_t valLen, void *pUser)
{
    MergeBaseCtx_t *pCtx = (MergeBaseCtx_t *)pUser;
    char key[SHADOW_JSON_KEY_MAX];
    const char *pOverVal = NULL;
    uint32_t overValLen = 0U;
    bool haveOver;

    if ((pCtx == NULL) || (!pCtx->ok) ||
        !prvKeyCstr(pKey, keyLen, key, sizeof(key)))
    {
        if (pCtx != NULL)
        {
            pCtx->ok = false;
        }
        return false;
    }
    haveOver = TIREMO_Json_FindRaw(pCtx->pOver, pCtx->overLen, key,
                                   &pOverVal, &overValLen);
    if (haveOver && prvIsNull(pOverVal, overValLen))
    {
        return true;
    }
    if (!TIREMO_JsonWriter_Key(pCtx->pW, key) ||
        !prvWriteMergedValue(pCtx->pW, pVal, valLen,
                             haveOver ? pOverVal : NULL,
                             haveOver ? overValLen : 0U))
    {
        pCtx->ok = false;
        return false;
    }
    return true;
}

static bool prvMergeOverKey(const char *pKey, uint32_t keyLen,
                            const char *pVal, uint32_t valLen, void *pUser)
{
    MergeOverCtx_t *pCtx = (MergeOverCtx_t *)pUser;
    char key[SHADOW_JSON_KEY_MAX];
    const char *pBaseVal = NULL;
    uint32_t baseValLen = 0U;

    if ((pCtx == NULL) || (!pCtx->ok) ||
        !prvKeyCstr(pKey, keyLen, key, sizeof(key)))
    {
        if (pCtx != NULL)
        {
            pCtx->ok = false;
        }
        return false;
    }
    if (TIREMO_Json_FindRaw(pCtx->pBase, pCtx->baseLen, key,
                            &pBaseVal, &baseValLen))
    {
        return true;
    }
    if (prvIsNull(pVal, valLen))
    {
        return true;
    }
    if (!TIREMO_JsonWriter_Key(pCtx->pW, key) ||
        !prvWriteMergedValue(pCtx->pW, NULL, 0U, pVal, valLen))
    {
        pCtx->ok = false;
        return false;
    }
    return true;
}

static bool prvWriteMergedObject(TIREMO_JsonWriter_t *pW,
                                 const char *pBase, uint32_t baseLen,
                                 const char *pOver, uint32_t overLen)
{
    MergeBaseCtx_t baseCtx;
    MergeOverCtx_t overCtx;

    if (!TIREMO_JsonWriter_ObjectStart(pW))
    {
        return false;
    }

    baseCtx.pW = pW;
    baseCtx.pOver = pOver;
    baseCtx.overLen = overLen;
    baseCtx.ok = true;
    if (!TIREMO_Json_ForEachKey(pBase, baseLen, prvMergeBaseKey, &baseCtx) ||
        (!baseCtx.ok))
    {
        return false;
    }

    overCtx.pW = pW;
    overCtx.pBase = pBase;
    overCtx.baseLen = baseLen;
    overCtx.ok = true;
    if (!TIREMO_Json_ForEachKey(pOver, overLen, prvMergeOverKey, &overCtx) ||
        (!overCtx.ok))
    {
        return false;
    }
    return TIREMO_JsonWriter_ObjectEnd(pW);
}

static bool prvMergeDesired(const char *pOverlay, uint32_t overlayLen,
                            char *pOut, uint32_t outSize, uint32_t *pOutLen)
{
    TIREMO_JsonWriter_t w;
    const char *pBase = (s_desiredLen >= 2U) ? s_desired : "{}";
    uint32_t baseLen = (s_desiredLen >= 2U) ? s_desiredLen : 2U;
    uint32_t n;

    if ((pOverlay == NULL) || (pOut == NULL) || (pOutLen == NULL) ||
        (!prvIsObject(pOverlay, overlayLen)))
    {
        return false;
    }

    TIREMO_JsonWriter_Init(&w, pOut, outSize);
    if (!prvWriteMergedObject(&w, pBase, baseLen, pOverlay, overlayLen))
    {
        return false;
    }
    n = TIREMO_JsonWriter_Finish(&w);
    if (n < 2U)
    {
        return false;
    }
    *pOutLen = n;
    return true;
}

static bool prvEchoDesired(const char *pToken)
{
    const char *pDesired = (s_desiredLen >= 2U) ? s_desired : NULL;
    uint32_t desiredLen = s_desiredLen;

    if (!prvBuildReport(pDesired, desiredLen, pToken))
    {
        TIREMO_LOG_ERROR("shadow: report JSON build failed");
        return false;
    }

    /* Same leftover desired, same report -- publishing again retriggers
     * update/delta and never converges. */
    if ((s_lastEchoLen == s_desiredLen) &&
        (s_desiredLen > 0U) &&
        (memcmp(s_lastEcho, s_desired, s_desiredLen) == 0))
    {
        TIREMO_LOG_WARN("shadow: desired unchanged (%lu bytes); skip report to break delta storm",
                        (unsigned long)s_desiredLen);
        return true;
    }

    if (!prvPublishReport(pToken))
    {
        return false;
    }
    if (s_desiredLen > 0U)
    {
        memcpy(s_lastEcho, s_desired, s_desiredLen);
        s_lastEcho[s_desiredLen] = '\0';
        s_lastEchoLen = s_desiredLen;
    }
    return true;
}

/**
 * @brief A delta is only the keys that differ. Reporting reporting+led
 *        (or the delta slice) leaves every other desired sibling in
 *        AWS's delta and the topic storms. Overlay the slice onto the
 *        last full desired and echo that object, same as tutorial 04.
 */
static bool prvHandleDelta(const char *pJson, uint32_t jsonLen)
{
    const char *pState = NULL;
    uint32_t stateLen = 0U;
    uint32_t mergedLen = 0U;

    TIREMO_LOG_INFO("shadow: delta %lu bytes", (unsigned long)jsonLen);

    if (TIREMO_Json_FindObject(pJson, jsonLen, "state", &pState, &stateLen))
    {
        prvApplyLocal(pState, stateLen, false);
    }
    else
    {
        pState = pJson;
        stateLen = jsonLen;
        prvApplyLocal(pJson, jsonLen, false);
    }
    TIREMO_Telemetry_Kick();

    if (!prvMergeDesired(pState, stateLen, s_report, sizeof(s_report),
                         &mergedLen) ||
        !prvCacheDesired(s_report, mergedLen))
    {
        TIREMO_LOG_ERROR("shadow: desired merge failed");
        return false;
    }

    TIREMO_LOG_INFO("shadow: desired now %lu bytes (merged)",
                    (unsigned long)s_desiredLen);
    return prvEchoDesired(SHADOW_DELTA_TOKEN);
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

    s_deltaReady = false;
    s_desiredLen = 0U;
    s_lastEchoLen = 0U;
    if ((pThingName == NULL) || (pThingName[0] == '\0') ||
        !prvBuildTopics(pThingName))
    {
        TIREMO_LOG_ERROR("shadow: bad thingName");
        return false;
    }

    TIREMO_LOG_INFO("shadow: thingName=%s", pThingName);
    TIREMO_LOG_INFO("shadow: subscribe get/accepted get/rejected (delta after boot report)");

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
                               &pState, &stateLen) &&
        TIREMO_Json_FindObject(pState, stateLen, "desired",
                               &pDesired, &desiredLen))
    {
        TIREMO_LOG_INFO("shadow: desired %lu bytes",
                        (unsigned long)desiredLen);
        (void)prvCacheDesired(pDesired, desiredLen);
        prvApplyLocal(s_desired, s_desiredLen, true);
    }
    else
    {
        TIREMO_LOG_INFO("shadow: no desired; reporting defaults");
        s_desiredLen = 0U;
        prvApplyLocal(NULL, 0U, true);
    }

    if (!prvEchoDesired(SHADOW_UPD_TOKEN))
    {
        return false;
    }

    wait = prvWaitAcceptedOrRejected(s_topicUpdAcc, s_topicUpdRej, &length);
    if (wait <= 0)
    {
        TIREMO_LOG_ERROR("shadow: update not accepted");
        return false;
    }
    TIREMO_LOG_INFO("shadow: update accepted");

    if (!prvConfirmNoDelta())
    {
        return false;
    }

    /* Subscribe after the boot echo so an existing delta is not
     * consumed (and dropped) while waiting for get/update accepted. */
    if (!TIREMO_ESP32_MqttSubscribe(s_topicDelta, 1U))
    {
        TIREMO_LOG_ERROR("shadow: update/delta subscribe failed");
        return false;
    }
    TIREMO_LOG_INFO("shadow: subscribed update/delta");
    s_deltaReady = true;
    s_deltaAccum = 0U;
    return true;
}

void TIREMO_Shadow_Poll(void)
{
    char topic[SHADOW_TOPIC_MAX];
    uint32_t fragLen = 0U;

    if (!s_deltaReady)
    {
        return;
    }

    if (!TIREMO_ESP32_MqttWaitMessage(
            topic, sizeof(topic), (uint8_t *)s_chunk,
            sizeof(s_chunk) - 1U, &fragLen, SHADOW_POLL_MS))
    {
        return;
    }
    if (fragLen >= sizeof(s_chunk))
    {
        TIREMO_LOG_ERROR("shadow: poll fragment too large");
        return;
    }
    s_chunk[fragLen] = '\0';

    if (strstr(topic, "/jobs/") != NULL)
    {
        TIREMO_Jobs_OnMessage(topic, s_chunk, fragLen);
        return;
    }

    if (strcmp(topic, s_topicUpdRej) == 0)
    {
        TIREMO_LOG_WARN("shadow: update rejected: %s", s_chunk);
        return;
    }
    if (strcmp(topic, s_topicDelta) != 0)
    {
        return;
    }

    if ((fragLen > 0U) && (s_chunk[0] == '{'))
    {
        s_deltaAccum = 0U;
    }
    if ((s_deltaAccum + fragLen) >= sizeof(s_response))
    {
        TIREMO_LOG_ERROR("shadow: delta too large");
        s_deltaAccum = 0U;
        return;
    }
    memcpy(s_response + s_deltaAccum, s_chunk, fragLen);
    s_deltaAccum += fragLen;
    s_response[s_deltaAccum] = '\0';

    TIREMO_LOG_INFO("shadow: MQTT RX topic=%s frag=%lu accum=%lu",
                    topic, (unsigned long)fragLen, (unsigned long)s_deltaAccum);

    if (prvJsonObjectComplete(s_response, s_deltaAccum))
    {
        (void)prvHandleDelta(s_response, s_deltaAccum);
        s_deltaAccum = 0U;
    }
}

bool TIREMO_Shadow_AccEn(void)
{
    return s_accEn;
}

bool TIREMO_Shadow_GyroEn(void)
{
    return s_gyroEn;
}

bool TIREMO_Shadow_TempEn(void)
{
    return s_tempEn;
}

bool TIREMO_Shadow_HumEn(void)
{
    return s_humEn;
}

uint32_t TIREMO_Shadow_IntervalS(void)
{
    return (s_intervalS == 0U) ? 60U : s_intervalS;
}
