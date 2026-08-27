/******************************************************************************
 * @file    tiremo_provision.c
 * @brief   AWS IoT Fleet Provisioning by Claim (ESP32-only tutorial).
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Include/tiremo_provision.h"
#include "../Include/tiremo_json.h"
#include "../Include/tiremo_debug.h"
#include "../../Drivers/esp32/Include/tiremo_esp32.h"
#include "../../aws_iot_config.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern const unsigned char g_rootCaPem[];
extern const size_t g_rootCaPem_len;
extern const unsigned char g_claimCertPem[];
extern const size_t g_claimCertPem_len;
extern const unsigned char g_claimKeyPem[];
extern const size_t g_claimKeyPem_len;

#define PROV_TOPIC_MAX        (192U)
#define PROV_RESPONSE_MAX     (8192U)
#define PROV_CHUNK_MAX        (2048U)
#define PROV_TOKEN_MAX        (512U)
#define PROV_CERT_MAX         (3072U)
#define PROV_KEY_MAX          (3072U)
#define PROV_REGISTER_MAX     (1024U)
#define PROV_THING_NAME_MAX   (128U)
#define PROV_TIMEOUT_MS       (60000U)
#define MQTT_CONNECT_MS       (45000U)

static char s_response[PROV_RESPONSE_MAX];
static char s_chunk[PROV_CHUNK_MAX];
static char s_token[PROV_TOKEN_MAX];
static char s_cert[PROV_CERT_MAX];
static char s_key[PROV_KEY_MAX];
static char s_register[PROV_REGISTER_MAX];
static char s_topicAccepted[PROV_TOPIC_MAX];
static char s_topicRejected[PROV_TOPIC_MAX];

static bool prvPemLooksReal(const unsigned char *pPem, size_t length)
{
    if ((pPem == NULL) || (length < 256U) ||
        (strstr((const char *)pPem, "BEGIN") == NULL))
    {
        return false;
    }
    return (strstr((const char *)pPem, "REPLACE_WITH_REAL") == NULL) &&
           (strstr((const char *)pPem, "Copy your AWS") == NULL) &&
           (strstr((const char *)pPem, "Copy the claim") == NULL);
}

static bool prvHasRealInputs(void)
{
    return prvPemLooksReal(g_claimCertPem, g_claimCertPem_len) &&
           prvPemLooksReal(g_claimKeyPem, g_claimKeyPem_len) &&
           (strstr(TIREMO_AWS_PRODUCT_SECRET, "REPLACE_WITH_REAL") == NULL) &&
           (strstr(TIREMO_AWS_ENDPOINT, "xxxxxxxx") == NULL) &&
           (strstr(TIREMO_AWS_TEMPLATE_NAME, "your-fleet") == NULL);
}

static bool prvLoad(TIREMO_ESP32_CredSlot_t slot,
                    const unsigned char *pData,
                    size_t length,
                    const char *pLabel)
{
    TIREMO_LOG_INFO("provision: write %s (%lu bytes)", pLabel,
                    (unsigned long)length);
    return TIREMO_ESP32_CredWrite(slot, pData, (uint32_t)length);
}

static bool prvConnect(const char *pClientId)
{
    return TIREMO_ESP32_MqttUserConfigMtls(pClientId) &&
           TIREMO_ESP32_MqttSni(TIREMO_AWS_ENDPOINT) &&
           TIREMO_ESP32_MqttConnect(TIREMO_AWS_ENDPOINT, 8883U,
                                    MQTT_CONNECT_MS);
}

/**
 * @brief True when p[0..len) is one complete JSON object (brace depth 0,
 *        strings skipped so a '}' inside a PEM does not count).
 *
 * ESP-AT's MQTT buffer is 1024 bytes, so CreateKeysAndCertificate arrives
 * as several +MQTTSUBRECV URCs on the same topic. We append those
 * fragments until this returns true.
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

static bool prvWaitAccepted(const char *pAccepted,
                            const char *pRejected,
                            const char *pRequiredMarker,
                            uint32_t *pLength)
{
    char topic[PROV_TOPIC_MAX];
    uint32_t fragLen = 0U;
    uint32_t accum = 0U;

    s_response[0] = '\0';

    for (;;)
    {
        if (!TIREMO_ESP32_MqttWaitMessage(
                topic, sizeof(topic), (uint8_t *)s_chunk,
                sizeof(s_chunk) - 1U, &fragLen, PROV_TIMEOUT_MS))
        {
            return false;
        }
        if (fragLen >= sizeof(s_chunk))
        {
            TIREMO_LOG_ERROR("provision: fragment too large (%lu)",
                             (unsigned long)fragLen);
            return false;
        }
        s_chunk[fragLen] = '\0';

        if (strcmp(topic, pRejected) == 0)
        {
            TIREMO_LOG_ERROR("provision: AWS rejected request: %s", s_chunk);
            return false;
        }
        if (strcmp(topic, pAccepted) != 0)
        {
            continue;
        }

        /* A new JSON document starts with '{'. Anything else on this
         * topic is the next 1024-byte slice of the same PEM payload. */
        if ((fragLen > 0U) && (s_chunk[0] == '{'))
        {
            accum = 0U;
        }
        if ((accum + fragLen) >= sizeof(s_response))
        {
            TIREMO_LOG_ERROR("provision: reassembled payload too large");
            return false;
        }
        memcpy(s_response + accum, s_chunk, fragLen);
        accum += fragLen;
        s_response[accum] = '\0';

        TIREMO_LOG_INFO("provision: MQTT RX topic=%s frag=%lu accum=%lu",
                        topic, (unsigned long)fragLen, (unsigned long)accum);

        if ((strstr(s_response, pRequiredMarker) != NULL) &&
            prvJsonObjectComplete(s_response, accum))
        {
            if (pLength != NULL)
            {
                *pLength = accum;
            }
            return true;
        }
    }
}

static bool prvBuildRegister(const char *pDeviceId)
{
    TIREMO_JsonWriter_t w;

    TIREMO_JsonWriter_Init(&w, s_register, sizeof(s_register));
    (void)TIREMO_JsonWriter_ObjectStart(&w);
    (void)TIREMO_JsonWriter_Key(&w, "certificateOwnershipToken");
    (void)TIREMO_JsonWriter_String(&w, s_token);
    (void)TIREMO_JsonWriter_Key(&w, "parameters");
    (void)TIREMO_JsonWriter_ObjectStart(&w);
    (void)TIREMO_JsonWriter_Key(&w, "deviceIdentifier");
    (void)TIREMO_JsonWriter_String(&w, pDeviceId);
    (void)TIREMO_JsonWriter_Key(&w, "productIdentifier");
    (void)TIREMO_JsonWriter_String(&w, TIREMO_AWS_PRODUCT_IDENTIFIER);
    (void)TIREMO_JsonWriter_Key(&w, "productSecret");
    (void)TIREMO_JsonWriter_String(&w, TIREMO_AWS_PRODUCT_SECRET);
    (void)TIREMO_JsonWriter_Key(&w, "isGateway");
    (void)TIREMO_JsonWriter_String(&w, TIREMO_AWS_IS_GATEWAY);
    (void)TIREMO_JsonWriter_ObjectEnd(&w);
    (void)TIREMO_JsonWriter_ObjectEnd(&w);
    return (TIREMO_JsonWriter_Finish(&w) > 0U);
}

static void prvWipe(void)
{
    memset(s_response, 0, sizeof(s_response));
    memset(s_token, 0, sizeof(s_token));
    memset(s_cert, 0, sizeof(s_cert));
    memset(s_key, 0, sizeof(s_key));
    memset(s_register, 0, sizeof(s_register));
}

bool TIREMO_Provision_IsDone(const char *pDeviceId)
{
    uint8_t marker[64];
    uint32_t length = 0U;

    if ((pDeviceId == NULL) ||
        !TIREMO_ESP32_CredRead(TIREMO_ESP32_CREDSLOT_PROV_MARKER,
                               marker, sizeof(marker) - 1U, &length))
    {
        TIREMO_LOG_INFO("provision: no matching marker");
        return false;
    }
    if (length >= sizeof(marker))
    {
        return false;
    }
    marker[length] = '\0';
    if (strstr((const char *)marker, pDeviceId) == NULL)
    {
        TIREMO_LOG_WARN("provision: marker belongs to another device (got \"%s\")",
                        (const char *)marker);
        return false;
    }
    TIREMO_LOG_INFO("provision: marker matches %s", pDeviceId);
    return true;
}

bool TIREMO_Provision_Run(const char *pDeviceId, const char *pClientId)
{
    uint32_t responseLen;
    char thingName[PROV_THING_NAME_MAX];

    if ((pDeviceId == NULL) || (pClientId == NULL) || !prvHasRealInputs())
    {
        TIREMO_LOG_ERROR("provision: copy real claim PEMs/config from examples");
        return false;
    }

    if (!prvLoad(TIREMO_ESP32_CREDSLOT_ROOT_CA, g_rootCaPem,
                 g_rootCaPem_len, "root CA") ||
        !prvLoad(TIREMO_ESP32_CREDSLOT_CLIENT_CERT, g_claimCertPem,
                 g_claimCertPem_len, "claim certificate") ||
        !prvLoad(TIREMO_ESP32_CREDSLOT_CLIENT_KEY, g_claimKeyPem,
                 g_claimKeyPem_len, "claim private key"))
    {
        return false;
    }
    if (!prvConnect(pClientId))
    {
        TIREMO_LOG_ERROR("provision: claim MQTT connect failed");
        return false;
    }
    TIREMO_LOG_INFO("provision: claim MQTT connected");

    TIREMO_LOG_INFO("provision: CreateKeysAndCertificate");
    if (!TIREMO_ESP32_MqttSubscribe(
            "$aws/certificates/create/json/accepted", 1U) ||
        !TIREMO_ESP32_MqttSubscribe(
            "$aws/certificates/create/json/rejected", 1U) ||
        !TIREMO_ESP32_MqttPublishRaw(
            "$aws/certificates/create/json", (const uint8_t *)"{}",
            2U, 1U, false) ||
        !prvWaitAccepted("$aws/certificates/create/json/accepted",
                         "$aws/certificates/create/json/rejected",
                         "certificateOwnershipToken", &responseLen))
    {
        (void)TIREMO_ESP32_MqttDisconnect();
        prvWipe();
        return false;
    }

    if (!TIREMO_Json_FindString(s_response, responseLen,
                                "certificateOwnershipToken",
                                s_token, sizeof(s_token)) ||
        !TIREMO_Json_FindString(s_response, responseLen, "certificatePem",
                                s_cert, sizeof(s_cert)) ||
        !TIREMO_Json_FindString(s_response, responseLen, "privateKey",
                                s_key, sizeof(s_key)) ||
        !prvBuildRegister(pDeviceId))
    {
        TIREMO_LOG_ERROR("provision: create response parse/build failed");
        (void)TIREMO_ESP32_MqttDisconnect();
        prvWipe();
        return false;
    }

    (void)snprintf(s_topicAccepted, sizeof(s_topicAccepted),
                   "$aws/provisioning-templates/%s/provision/json/accepted",
                   TIREMO_AWS_TEMPLATE_NAME);
    (void)snprintf(s_topicRejected, sizeof(s_topicRejected),
                   "$aws/provisioning-templates/%s/provision/json/rejected",
                   TIREMO_AWS_TEMPLATE_NAME);

    TIREMO_LOG_INFO("provision: RegisterThing deviceIdentifier=%s", pDeviceId);
    {
        char pubTopic[PROV_TOPIC_MAX];
        (void)snprintf(pubTopic, sizeof(pubTopic),
                       "$aws/provisioning-templates/%s/provision/json",
                       TIREMO_AWS_TEMPLATE_NAME);
        if (!TIREMO_ESP32_MqttSubscribe(s_topicAccepted, 1U) ||
            !TIREMO_ESP32_MqttSubscribe(s_topicRejected, 1U) ||
            !TIREMO_ESP32_MqttPublishRaw(
                pubTopic, (const uint8_t *)s_register,
                (uint32_t)strlen(s_register), 1U, false) ||
            !prvWaitAccepted(s_topicAccepted, s_topicRejected,
                             "thingName", &responseLen))
        {
            (void)TIREMO_ESP32_MqttDisconnect();
            prvWipe();
            return false;
        }
    }

    if (TIREMO_Json_FindString(s_response, responseLen, "thingName",
                               thingName, sizeof(thingName)))
    {
        TIREMO_LOG_INFO("provision: registered Thing %s", thingName);
    }

    (void)TIREMO_ESP32_MqttDisconnect();
    if (!TIREMO_ESP32_CredWrite(TIREMO_ESP32_CREDSLOT_CLIENT_CERT,
                                (const uint8_t *)s_cert,
                                (uint32_t)strlen(s_cert)) ||
        !TIREMO_ESP32_CredWrite(TIREMO_ESP32_CREDSLOT_CLIENT_KEY,
                                (const uint8_t *)s_key,
                                (uint32_t)strlen(s_key)) ||
        !TIREMO_ESP32_CredWrite(TIREMO_ESP32_CREDSLOT_PROV_MARKER,
                                (const uint8_t *)pDeviceId,
                                (uint32_t)strlen(pDeviceId)))
    {
        TIREMO_LOG_ERROR("provision: permanent credential install failed");
        prvWipe();
        return false;
    }
    prvWipe();

    if (!prvConnect(pClientId))
    {
        TIREMO_LOG_ERROR("provision: permanent MQTT reconnect failed");
        return false;
    }
    TIREMO_LOG_INFO("provision: complete; permanent MQTT connected");
    return true;
}

bool TIREMO_Provision_FactoryReset(void)
{
    bool ok;

    TIREMO_LOG_WARN("factory reset: erase mqtt_cert, mqtt_key, provisioned marker");
    (void)TIREMO_ESP32_MqttDisconnect();
    ok = TIREMO_ESP32_CredErase(TIREMO_ESP32_CREDSLOT_CLIENT_CERT);
    ok = TIREMO_ESP32_CredErase(TIREMO_ESP32_CREDSLOT_CLIENT_KEY) && ok;
    ok = TIREMO_ESP32_CredErase(TIREMO_ESP32_CREDSLOT_PROV_MARKER) && ok;
    if (!ok)
    {
        TIREMO_LOG_ERROR("factory reset: one or more SYSMFG erases failed");
    }
    return ok;
}
