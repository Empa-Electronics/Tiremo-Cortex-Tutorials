/******************************************************************************
 * @file    tiremo_jobs.c
 * @brief   notify → start-next → health SUCCEEDED / FIRMWARE_UPDATE applied.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Include/tiremo_jobs.h"
#include "../Include/tiremo_json.h"
#include "../Include/tiremo_systick.h"
#include "../Include/tiremo_debug.h"
#include "../Include/tiremo_ota.h"
#include "../Include/tiremo_version.h"
#include "../../Drivers/esp32/Include/tiremo_esp32.h"

#include <stdio.h>
#include <string.h>

#define JOBS_TOPIC_MAX       (192U)
#define JOBS_ID_MAX          (64U)
/* A real Tiremo Cloud FOTA execution document is dominated by a pre-signed
 * S3 URL (with an STS token) -- several KB, not a few hundred bytes. The
 * module hands it over in ~1 KB +MQTTSUBRECV fragments, reassembled here. */
#define JOBS_DOC_MAX         (12288U)
#define JOBS_START_TIMEOUT_MS (20000U)
#define JOBS_FW_VERSION      TIREMO_FW_VERSION

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

/**
 * @brief Decide whether an execution is a firmware update from its
 *        jobDocument content -- the jobId prefix (1_/2_) is not a
 *        reliable discriminator on Tiremo Cloud (a FOTA job is 1_...).
 */
static bool prvIsOtaJob(const char *pPayload, uint32_t payloadLen,
                        const char *pJobId)
{
    const char *pExec = NULL;
    const char *pDoc = NULL;
    uint32_t execLen = 0U;
    uint32_t docLen = 0U;
    const char *pScan = pPayload;

    (void)pJobId;

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

    /* Tiremo Cloud FOTA marker set, plus the generic ones. */
    return ((strstr(pScan, "firmwareUpdateType") != NULL) ||
            (strstr(pScan, "\"FOTA\"") != NULL) ||
            (strstr(pScan, "Firmware-Update") != NULL) ||
            (strstr(pScan, "FIRMWARE_UPDATE") != NULL) ||
            (strstr(pScan, "s3Path") != NULL) ||
            (strstr(pScan, "\"fileInfo\"") != NULL) ||
            (strstr(pScan, "\"files\"") != NULL) ||
            prvContainsI(pScan, "firmware"));
}

/**
 * @brief Pull the jobDocument object slice out of a start-next / notify
 *        payload so it can be handed to the OTA layer.
 *
 * @return true when a jobDocument object was located.
 */
static bool prvFindJobDoc(const char *pPayload, uint32_t payloadLen,
                          const char **ppDoc, uint32_t *pDocLen)
{
    const char *pExec = NULL;
    uint32_t execLen = 0U;
    const char *pScan = pPayload;
    uint32_t scanLen = payloadLen;

    *ppDoc = NULL;
    *pDocLen = 0U;
    if ((pPayload == NULL) || (payloadLen == 0U))
    {
        return false;
    }
    if (TIREMO_Json_FindObject(pScan, scanLen, "execution", &pExec, &execLen))
    {
        pScan = pExec;
        scanLen = execLen;
    }
    return TIREMO_Json_FindObject(pScan, scanLen, "jobDocument",
                                  ppDoc, pDocLen);
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

/**
 * @brief Echo a whole inbound job JSON to the log, bracketed so it stands
 *        out. The leveled logger's line buffer is far too small for an
 *        execution document, so this goes straight to the UART.
 */
static void prvLogRawJson(const char *pWhat, const char *pJson,
                          uint32_t jsonLen)
{
    if ((pJson == NULL) || (pWhat == NULL))
    {
        return;
    }

    TIREMO_Debug_Puts("[INFO] jobs: ");
    TIREMO_Debug_Puts(pWhat);
    TIREMO_Debug_Puts(" json <<<\r\n");

    if (jsonLen == 0U)
    {
        TIREMO_Debug_Puts(pJson);   /* NUL-terminated */
    }
    else
    {
        uint32_t i;
        uint32_t cap = (jsonLen < JOBS_DOC_MAX) ? jsonLen : JOBS_DOC_MAX;

        for (i = 0U; i < cap; ++i)
        {
            TIREMO_Debug_PutChar((uint8_t)pJson[i]);
        }
    }
    TIREMO_Debug_Puts("\r\n[INFO] jobs: >>>\r\n");
}

static bool prvHandleExecution(const char *pPayload, uint32_t payloadLen)
{
    char jobId[JOBS_ID_MAX];

    prvLogRawJson("execution", pPayload, payloadLen);

    if (!prvFindJobId(pPayload, payloadLen, jobId, sizeof(jobId)))
    {
        TIREMO_LOG_INFO("jobs: no pending execution");
        return true;
    }

    if (prvIsOtaJob(pPayload, payloadLen, jobId))
    {
        const char *pDoc = NULL;
        uint32_t docLen = 0U;

        TIREMO_LOG_INFO("jobs: FIRMWARE_UPDATE '%s' -- applying", jobId);
        (void)prvPublishUpdate(jobId, "IN_PROGRESS", NULL);
        (void)prvFindJobDoc(pPayload, payloadLen, &pDoc, &docLen);

        if (!TIREMO_OTA_Apply(jobId, pDoc, docLen))
        {
            TIREMO_LOG_ERROR("jobs: OTA '%s' -- FAILED (%s)", jobId,
                             TIREMO_OTA_LastError());
            return prvPublishUpdate(jobId, "FAILED", TIREMO_OTA_LastError());
        }
        return true;   /* TIREMO_OTA_Apply() resets on success */
    }

    TIREMO_LOG_INFO("jobs: health-check '%s' -- SUCCEEDED", jobId);
    return prvPublishUpdate(jobId, "SUCCEEDED", NULL);
}

/**
 * @brief True once @p pJson holds a complete, brace-balanced JSON object
 *        (braces inside strings ignored).
 */
static bool prvJsonComplete(const char *pJson, uint32_t len)
{
    int32_t depth = 0;
    bool inStr = false;
    bool esc = false;
    bool sawOpen = false;
    uint32_t i;

    for (i = 0U; i < len; ++i)
    {
        char c = pJson[i];

        if (inStr)
        {
            if (esc)            { esc = false; }
            else if (c == '\\') { esc = true; }
            else if (c == '"')  { inStr = false; }
            continue;
        }
        if (c == '"')      { inStr = true; }
        else if (c == '{') { depth++; sawOpen = true; }
        else if (c == '}')
        {
            depth--;
            if (sawOpen && (depth == 0))
            {
                return true;
            }
        }
    }
    return false;
}

static bool prvWaitStartNextAccepted(uint32_t *pLen)
{
    char topic[JOBS_TOPIC_MAX];
    uint32_t startMs;
    uint32_t fragLen = 0U;
    uint32_t accum = 0U;
    uint32_t frags = 0U;

    *pLen = 0U;
    startMs = TIREMO_SysTick_GetMs();
    while ((TIREMO_SysTick_GetMs() - startMs) < JOBS_START_TIMEOUT_MS)
    {
        uint32_t room = (uint32_t)sizeof(s_doc) - accum - 1U;

        if (room == 0U)
        {
            TIREMO_LOG_WARN("jobs: start-next doc exceeds %u B buffer (%lu frags)",
                            (unsigned)sizeof(s_doc), (unsigned long)frags);
            return false;
        }

        /* Each call yields one complete +MQTTSUBRECV fragment; large
         * documents arrive as several, so append and stop when the JSON
         * object closes. */
        if (!TIREMO_ESP32_MqttWaitMessage(topic, sizeof(topic),
                                          (uint8_t *)(s_doc + accum), room,
                                          &fragLen, 500U))
        {
            continue;
        }
        if ((strcmp(topic, s_topicStartAcc) != 0) &&
            (strcmp(topic, s_topicNotifyNext) != 0))
        {
            continue;   /* not our reply -- the bytes just written get overwritten */
        }
        if (fragLen > room)
        {
            fragLen = room;
        }
        if ((accum == 0U) && ((fragLen == 0U) || (s_doc[0] != '{')))
        {
            continue;   /* wait for a fragment that begins the object */
        }

        accum += fragLen;
        s_doc[accum] = '\0';
        frags++;

        if (prvJsonComplete(s_doc, accum))
        {
            *pLen = accum;
            TIREMO_LOG_INFO("jobs: MQTT RX topic=%s len=%lu (%lu frags)",
                            topic, (unsigned long)accum, (unsigned long)frags);
            return true;
        }
    }
    TIREMO_LOG_WARN("jobs: start-next timed out mid-doc (%lu B, %lu frags)",
                    (unsigned long)accum, (unsigned long)frags);
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
        /* No execution document on this path -- apply the staged image
         * with no cloud-side size/crc gate (the on-flash verify still
         * runs). */
        if (!TIREMO_OTA_Apply(pJobId, NULL, 0U))
        {
            TIREMO_LOG_ERROR("jobs: OTA '%s' -- FAILED (%s)", pJobId,
                             TIREMO_OTA_LastError());
            return prvPublishUpdate(pJobId, "FAILED", TIREMO_OTA_LastError());
        }
        return true;   /* TIREMO_OTA_Apply() resets on success */
    }
    return prvPublishUpdate(pJobId, "SUCCEEDED", NULL);
}

void TIREMO_Jobs_ReportResult(const char *pJobId, bool ok, const char *pDetail)
{
    if ((pJobId == NULL) || (pJobId[0] == '\0') || (s_thing[0] == '\0'))
    {
        return;
    }
    (void)prvPublishUpdate(pJobId, ok ? "SUCCEEDED" : "FAILED",
                           ok ? NULL : pDetail);
}

void TIREMO_Jobs_ReportResultDirect(const char *pThingName, const char *pJobId,
                                    bool ok, const char *pDetail)
{
    if ((pThingName == NULL) || (pThingName[0] == '\0'))
    {
        return;
    }
    /* Only needs the Thing name to build the update topic -- MQTT just
     * has to be connected. Init() overwrites s_thing with the same value
     * moments later, so this is safe to call before it. */
    (void)snprintf(s_thing, sizeof(s_thing), "%s", pThingName);
    TIREMO_Jobs_ReportResult(pJobId, ok, pDetail);
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
    prvLogRawJson("notify", pPayload, payloadLen);

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
