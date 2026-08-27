/******************************************************************************
 * @file    tiremo_jobs.c
 * @brief   notify → start-next → health SUCCEEDED / OTA REJECTED.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Include/tiremo_jobs.h"
#include "../Include/tiremo_json.h"
#include "../Include/tiremo_systick.h"
#include "../Include/tiremo_debug.h"
#include "../../Drivers/esp32/Include/tiremo_esp32.h"

#include <stdio.h>
#include <string.h>

#define JOBS_TOPIC_MAX       (192U)
#define JOBS_ID_MAX          (64U)
#define JOBS_DOC_MAX         (2048U)
#define JOBS_CHUNK_MAX       (1024U)
#define JOBS_START_TIMEOUT_MS (12000U)
#define JOBS_FW_VERSION      "cortex-07"

static char s_thing[64];
static char s_topicNotify[JOBS_TOPIC_MAX];
static char s_topicStart[JOBS_TOPIC_MAX];
static char s_topicStartAcc[JOBS_TOPIC_MAX];
static char s_topicNotifyNext[JOBS_TOPIC_MAX];
static char s_doc[JOBS_DOC_MAX];
static char s_json[512];
static char s_pendingJobId[JOBS_ID_MAX];
static bool s_ready = false;
static bool s_needStartNext = false;
static uint32_t s_bootMs = 0U;

static bool prvTopic(char *pOut, uint32_t outMax, const char *pThing,
                     const char *pSuffix)
{
    int n = snprintf(pOut, outMax, "$aws/things/%s/%s", pThing, pSuffix);

    return ((n > 0) && ((uint32_t)n < outMax));
}

static bool prvIsHealthJobId(const char *pJobId)
{
    return ((pJobId != NULL) && (pJobId[0] == '1') && (pJobId[1] == '_'));
}

static bool prvIsOtaJobId(const char *pJobId)
{
    return ((pJobId != NULL) && (pJobId[0] == '2') && (pJobId[1] == '_'));
}

static bool prvContainsI(const char *pHay, const char *pNeedle)
{
    size_t nLen;
    const char *pH;

    if ((pHay == NULL) || (pNeedle == NULL) || (pNeedle[0] == '\0'))
    {
        return false;
    }
    nLen = strlen(pNeedle);
    for (pH = pHay; *pH != '\0'; ++pH)
    {
        size_t i;

        for (i = 0U; i < nLen; ++i)
        {
            char a = pH[i];
            char b = pNeedle[i];

            if (a == '\0')
            {
                return false;
            }
            if ((a >= 'A') && (a <= 'Z'))
            {
                a = (char)(a - 'A' + 'a');
            }
            if ((b >= 'A') && (b <= 'Z'))
            {
                b = (char)(b - 'A' + 'a');
            }
            if (a != b)
            {
                break;
            }
        }
        if (i == nLen)
        {
            return true;
        }
    }
    return false;
}

static bool prvIsOtaJob(const char *pPayload, uint32_t payloadLen,
                        const char *pJobId)
{
    const char *pExec = NULL;
    const char *pDoc = NULL;
    uint32_t execLen = 0U;
    uint32_t docLen = 0U;
    const char *pScan = pPayload;

    if (prvIsHealthJobId(pJobId))
    {
        return false;
    }
    if (prvIsOtaJobId(pJobId) ||
        prvContainsI(pJobId, "ota") || prvContainsI(pJobId, "firmware"))
    {
        return true;
    }
    if ((pPayload == NULL) || (payloadLen == 0U))
    {
        return false;
    }
    if (TIREMO_Json_FindObject(pPayload, payloadLen, "execution",
                               &pExec, &execLen))
    {
        pScan = pExec;
        payloadLen = execLen;
    }
    if (TIREMO_Json_FindObject(pScan, payloadLen, "jobDocument",
                               &pDoc, &docLen))
    {
        pScan = pDoc;
        payloadLen = docLen;
    }
    if ((strstr(pScan, "\"files\"") != NULL) ||
        prvContainsI(pScan, "firmware") || prvContainsI(pScan, "ota"))
    {
        return true;
    }
    return false;
}

static bool prvScanJobId(const char *pJson, uint32_t jsonLen,
                         char *pOut, uint32_t outSize)
{
    const char *p;
    const char *pEnd;
    uint32_t i = 0U;

    if ((pJson == NULL) || (jsonLen == 0U) || (pOut == NULL) ||
        (outSize < 2U))
    {
        return false;
    }
    p = strstr(pJson, "\"jobId\"");
    if ((p == NULL) || ((uint32_t)(p - pJson) >= jsonLen))
    {
        return false;
    }
    p = strchr(p + 7, '"');
    if (p == NULL)
    {
        return false;
    }
    p++;
    pEnd = pJson + jsonLen;
    while ((p < pEnd) && (*p != '"') && (*p != '\0') && ((i + 1U) < outSize))
    {
        pOut[i++] = *p++;
    }
    pOut[i] = '\0';
    return (i > 0U);
}

static bool prvFindJobId(const char *pJson, uint32_t jsonLen,
                         char *pJobId, uint32_t jobIdMax)
{
    const char *pExec = NULL;
    uint32_t execLen = 0U;

    if ((pJson == NULL) || (pJobId == NULL) || (jobIdMax < 2U))
    {
        return false;
    }
    if (TIREMO_Json_FindObject(pJson, jsonLen, "execution",
                               &pExec, &execLen) &&
        TIREMO_Json_FindString(pExec, execLen, "jobId", pJobId, jobIdMax))
    {
        return true;
    }
    if (TIREMO_Json_FindString(pJson, jsonLen, "jobId", pJobId, jobIdMax))
    {
        return true;
    }
    return prvScanJobId(pJson, jsonLen, pJobId, jobIdMax);
}

static bool prvPublishUpdate(const char *pJobId, const char *pStatus,
                             const char *pReason)
{
    char topic[JOBS_TOPIC_MAX];
    char suffix[96];
    TIREMO_JsonWriter_t w;
    uint32_t len;
    uint32_t uptimeS;

    if ((s_thing[0] == '\0') || (pJobId == NULL) || (pJobId[0] == '\0') ||
        (pStatus == NULL))
    {
        return false;
    }
    (void)snprintf(suffix, sizeof(suffix), "jobs/%s/update", pJobId);
    if (!prvTopic(topic, sizeof(topic), s_thing, suffix))
    {
        return false;
    }

    uptimeS = 0U;
    if (s_bootMs != 0U)
    {
        uptimeS = (TIREMO_SysTick_GetMs() - s_bootMs) / 1000U;
    }

    TIREMO_JsonWriter_Init(&w, s_json, sizeof(s_json));
    if (!TIREMO_JsonWriter_ObjectStart(&w) ||
        !TIREMO_JsonWriter_Key(&w, "status") ||
        !TIREMO_JsonWriter_String(&w, pStatus) ||
        !TIREMO_JsonWriter_Key(&w, "statusDetails") ||
        !TIREMO_JsonWriter_ObjectStart(&w))
    {
        return false;
    }
    if (strcmp(pStatus, "SUCCEEDED") == 0)
    {
        if (!TIREMO_JsonWriter_Key(&w, "result") ||
            !TIREMO_JsonWriter_String(&w, "ok") ||
            !TIREMO_JsonWriter_Key(&w, "firmware_version") ||
            !TIREMO_JsonWriter_String(&w, JOBS_FW_VERSION) ||
            !TIREMO_JsonWriter_Key(&w, "connectivity") ||
            !TIREMO_JsonWriter_String(&w, "wifi") ||
            !TIREMO_JsonWriter_Key(&w, "mqtt") ||
            !TIREMO_JsonWriter_String(&w, "connected") ||
            !TIREMO_JsonWriter_Key(&w, "uptime_s") ||
            !TIREMO_JsonWriter_Uint(&w, uptimeS))
        {
            return false;
        }
    }
    else if (pReason != NULL)
    {
        if (!TIREMO_JsonWriter_Key(&w, "reason") ||
            !TIREMO_JsonWriter_String(&w, pReason))
        {
            return false;
        }
    }
    if (!TIREMO_JsonWriter_ObjectEnd(&w) ||
        !TIREMO_JsonWriter_ObjectEnd(&w))
    {
        return false;
    }
    len = TIREMO_JsonWriter_Finish(&w);
    if (len == 0U)
    {
        return false;
    }
    TIREMO_LOG_INFO("jobs: update %s status=%s (%lu bytes)",
                    pJobId, pStatus, (unsigned long)len);
    return TIREMO_ESP32_MqttPublishRaw(topic, (const uint8_t *)s_json,
                                       len, 1U, false);
}

static bool prvHandleExecution(const char *pPayload, uint32_t payloadLen)
{
    char jobId[JOBS_ID_MAX];

    if (!prvFindJobId(pPayload, payloadLen, jobId, sizeof(jobId)))
    {
        TIREMO_LOG_INFO("jobs: no pending execution");
        return true;
    }

    if (prvIsOtaJob(pPayload, payloadLen, jobId))
    {
        TIREMO_LOG_INFO("jobs: OTA '%s' -- REJECTED (not implemented here)",
                        jobId);
        return prvPublishUpdate(jobId, "REJECTED", "ota_not_implemented");
    }

    TIREMO_LOG_INFO("jobs: health-check '%s' -- SUCCEEDED%s", jobId,
                    prvIsHealthJobId(jobId) ? " (type 1_)" : "");
    return prvPublishUpdate(jobId, "SUCCEEDED", NULL);
}

static bool prvWaitStartNextAccepted(uint32_t *pLen)
{
    char topic[JOBS_TOPIC_MAX];
    char chunk[JOBS_CHUNK_MAX];
    uint32_t startMs;
    uint32_t fragLen = 0U;
    uint32_t accum = 0U;

    *pLen = 0U;
    startMs = TIREMO_SysTick_GetMs();
    while ((TIREMO_SysTick_GetMs() - startMs) < JOBS_START_TIMEOUT_MS)
    {
        if (!TIREMO_ESP32_MqttWaitMessage(
                topic, sizeof(topic), (uint8_t *)chunk,
                sizeof(chunk) - 1U, &fragLen, 500U))
        {
            continue;
        }
        chunk[fragLen] = '\0';
        if ((strcmp(topic, s_topicStartAcc) != 0) &&
            (strcmp(topic, s_topicNotifyNext) != 0))
        {
            continue;
        }
        if ((fragLen > 0U) && (chunk[0] == '{'))
        {
            accum = 0U;
        }
        if ((accum + fragLen) >= sizeof(s_doc))
        {
            TIREMO_LOG_ERROR("jobs: start-next document too large");
            return false;
        }
        memcpy(s_doc + accum, chunk, fragLen);
        accum += fragLen;
        s_doc[accum] = '\0';
        if (strstr(s_doc, "\"timestamp\"") != NULL)
        {
            *pLen = accum;
            TIREMO_LOG_INFO("jobs: MQTT RX topic=%s accum=%lu",
                            topic, (unsigned long)accum);
            return true;
        }
    }
    return false;
}

static bool prvStartNextAndHandle(bool *pHadExecution)
{
    char req[80];
    uint32_t token;
    uint32_t len = 0U;

    if (pHadExecution != NULL)
    {
        *pHadExecution = false;
    }

    token = TIREMO_SysTick_GetMs();
    (void)snprintf(req, sizeof(req),
                   "{\"statusDetails\":{\"clientToken\":\"%lu\"}}",
                   (unsigned long)token);

    TIREMO_LOG_INFO("jobs: start-next");
    if (!TIREMO_ESP32_MqttPublishRaw(
            s_topicStart, (const uint8_t *)req, (uint32_t)strlen(req),
            1U, false))
    {
        TIREMO_LOG_ERROR("jobs: start-next publish failed");
        return false;
    }

    if (!prvWaitStartNextAccepted(&len))
    {
        TIREMO_LOG_INFO("jobs: start-next empty/timeout (no queued job)");
        return true;
    }
    if ((strstr(s_doc, "\"execution\"") == NULL) ||
        (strstr(s_doc, "\"jobId\"") == NULL))
    {
        TIREMO_LOG_INFO("jobs: start-next accepted with no execution");
        return true;
    }
    if (pHadExecution != NULL)
    {
        *pHadExecution = true;
    }
    return prvHandleExecution(s_doc, len);
}

static bool prvClaimById(const char *pJobId)
{
    TIREMO_LOG_INFO("jobs: start-next empty -- claiming '%s' directly", pJobId);
    (void)prvPublishUpdate(pJobId, "IN_PROGRESS", NULL);
    if (prvIsOtaJob(NULL, 0U, pJobId))
    {
        return prvPublishUpdate(pJobId, "REJECTED", "ota_not_implemented");
    }
    return prvPublishUpdate(pJobId, "SUCCEEDED", NULL);
}

bool TIREMO_Jobs_Init(const char *pThingName)
{
    if ((pThingName == NULL) || (pThingName[0] == '\0'))
    {
        return false;
    }
    if ((uint32_t)snprintf(s_thing, sizeof(s_thing), "%s", pThingName) >=
        sizeof(s_thing))
    {
        return false;
    }
    if (!prvTopic(s_topicNotify, sizeof(s_topicNotify), pThingName,
                  "jobs/notify") ||
        !prvTopic(s_topicStart, sizeof(s_topicStart), pThingName,
                  "jobs/start-next") ||
        !prvTopic(s_topicStartAcc, sizeof(s_topicStartAcc), pThingName,
                  "jobs/start-next/accepted") ||
        !prvTopic(s_topicNotifyNext, sizeof(s_topicNotifyNext), pThingName,
                  "jobs/notify-next"))
    {
        return false;
    }

    s_bootMs = TIREMO_SysTick_GetMs();
    s_pendingJobId[0] = '\0';
    s_needStartNext = false;

    TIREMO_LOG_INFO("jobs: subscribe notify-next start-next/accepted");
    if (!TIREMO_ESP32_MqttSubscribe(s_topicNotifyNext, 1U) ||
        !TIREMO_ESP32_MqttSubscribe(s_topicStartAcc, 1U))
    {
        TIREMO_LOG_ERROR("jobs: start-next subscribe failed");
        return false;
    }

    (void)prvStartNextAndHandle(NULL);

    if (!TIREMO_ESP32_MqttSubscribe(s_topicNotify, 1U))
    {
        TIREMO_LOG_ERROR("jobs: notify subscribe failed");
        return false;
    }
    TIREMO_LOG_INFO("jobs: subscribed jobs/notify");
    s_ready = true;
    return true;
}

void TIREMO_Jobs_OnMessage(const char *pTopic, const char *pPayload,
                           uint32_t payloadLen)
{
    char jobId[JOBS_ID_MAX];

    if ((!s_ready) || (pTopic == NULL) || (pPayload == NULL))
    {
        return;
    }
    if (strstr(pTopic, "/jobs/") == NULL)
    {
        return;
    }
    if ((strcmp(pTopic, s_topicStartAcc) == 0) ||
        (strcmp(pTopic, s_topicNotifyNext) == 0))
    {
        /* start-next replies are consumed inside prvWaitStartNextAccepted. */
        return;
    }

    TIREMO_LOG_INFO("jobs: MQTT RX topic=%s len=%lu",
                    pTopic, (unsigned long)payloadLen);

    if ((strstr(pPayload, "\"execution\"") != NULL) &&
        prvFindJobId(pPayload, payloadLen, jobId, sizeof(jobId)))
    {
        (void)prvHandleExecution(pPayload, payloadLen);
        return;
    }

    if (prvFindJobId(pPayload, payloadLen, jobId, sizeof(jobId)))
    {
        (void)snprintf(s_pendingJobId, sizeof(s_pendingJobId), "%s", jobId);
        TIREMO_LOG_INFO("jobs: notify jobId='%s'", s_pendingJobId);
    }
    else
    {
        s_pendingJobId[0] = '\0';
        TIREMO_LOG_INFO("jobs: notify (no jobId at this depth)");
    }
    s_needStartNext = true;
}

void TIREMO_Jobs_Poll(void)
{
    bool hadExec = false;
    char pending[JOBS_ID_MAX];

    if ((!s_ready) || (!s_needStartNext))
    {
        return;
    }
    s_needStartNext = false;
    (void)snprintf(pending, sizeof(pending), "%s", s_pendingJobId);
    s_pendingJobId[0] = '\0';

    hadExec = false;
    if (!prvStartNextAndHandle(&hadExec))
    {
        TIREMO_LOG_WARN("jobs: start-next handle failed");
    }
    if (hadExec)
    {
        return;
    }
    if (pending[0] != '\0')
    {
        (void)prvClaimById(pending);
    }
}
