/******************************************************************************
 * @file    tiremo_telemetry.c
 * @brief   Periodic telemetry + TEMP_HIGH hysteresis alarm.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Include/tiremo_telemetry.h"
#include "../Include/tiremo_shadow.h"
#include "../Include/tiremo_json.h"
#include "../Include/tiremo_debug.h"
#include "../../Drivers/esp32/Include/tiremo_esp32.h"

#include <stdio.h>
#include <string.h>

#define TELEMETRY_TOPIC_MAX   (192U)
#define TELEMETRY_JSON_MAX    (192U)
#define TEMP_AMBIENT_C        (22U)
#define TEMP_HELD_C           (40U)
#define TEMP_RAISE_C          (35U)
#define TEMP_CLEAR_C          (30U)
#define INTERVAL_S_MAX        (3600U)
#define ALARM_EXCEPTION_MAX   (64U)

static char s_topicTelemetry[TELEMETRY_TOPIC_MAX];
static char s_topicAlarms[TELEMETRY_TOPIC_MAX];
static char s_json[TELEMETRY_JSON_MAX];
static bool s_ready = false;
static bool s_alarmActive = false;
static bool s_userHeldLogged = false;
static uint32_t s_lastTelemetryMs = 0U;
static uint32_t s_bootMs = 0U;

static uint32_t prvIntervalMs(void)
{
    uint32_t intervalS = TIREMO_Shadow_IntervalS();

    if (intervalS > INTERVAL_S_MAX)
    {
        intervalS = INTERVAL_S_MAX;
    }
    return intervalS * 1000U;
}

static uint32_t prvSampleTempC(bool userHeld)
{
    if (userHeld != s_userHeldLogged)
    {
        s_userHeldLogged = userHeld;
        TIREMO_LOG_INFO("telemetry: demo tempC=%u (%s)",
                        userHeld ? TEMP_HELD_C : TEMP_AMBIENT_C,
                        userHeld ? "USER held" : "released");
    }
    return userHeld ? TEMP_HELD_C : TEMP_AMBIENT_C;
}

static bool prvPublishBuf(const char *pTopic, uint8_t qos, uint32_t len)
{
    if ((len == 0U) ||
        !TIREMO_ESP32_MqttPublishRaw(pTopic, (const uint8_t *)s_json,
                                     len, qos, false))
    {
        TIREMO_LOG_ERROR("telemetry: publish failed topic=%s", pTopic);
        return false;
    }
    TIREMO_LOG_INFO("telemetry: pub %s (%lu bytes) %s",
                    pTopic, (unsigned long)len, s_json);
    return true;
}

static void prvPublishTelemetry(uint32_t nowMs, uint32_t tempC)
{
    TIREMO_JsonWriter_t w;
    uint32_t len;
    uint32_t uptimeS = (nowMs - s_bootMs) / 1000U;

    TIREMO_JsonWriter_Init(&w, s_json, sizeof(s_json));
    if (!TIREMO_JsonWriter_ObjectStart(&w) ||
        !TIREMO_JsonWriter_Key(&w, "uptimeS") ||
        !TIREMO_JsonWriter_Uint(&w, uptimeS) ||
        !TIREMO_JsonWriter_Key(&w, "tempC") ||
        !TIREMO_JsonWriter_Uint(&w, tempC) ||
        !TIREMO_JsonWriter_ObjectEnd(&w))
    {
        TIREMO_LOG_ERROR("telemetry: json failed");
        return;
    }
    len = TIREMO_JsonWriter_Finish(&w);
    (void)prvPublishBuf(s_topicTelemetry, 0U, len);
}

static void prvPublishAlarm(bool active)
{
    TIREMO_JsonWriter_t w;
    char exception[ALARM_EXCEPTION_MAX];
    uint32_t len;
    int n;

    if (active)
    {
        n = snprintf(exception, sizeof(exception),
                     "Threshold %uC exceeded", TEMP_RAISE_C);
    }
    else
    {
        n = snprintf(exception, sizeof(exception),
                     "Temperature returned to normal");
    }
    if ((n <= 0) || ((uint32_t)n >= sizeof(exception)))
    {
        TIREMO_LOG_ERROR("alarm: exception string failed");
        return;
    }

    /* Ingest accepts only these four keys. severity is MAJOR / MINOR /
     * WARNING / INFO — not CRITICAL. exception is a string, not a bool. */
    TIREMO_JsonWriter_Init(&w, s_json, sizeof(s_json));
    if (!TIREMO_JsonWriter_ObjectStart(&w) ||
        !TIREMO_JsonWriter_Key(&w, "alarmType") ||
        !TIREMO_JsonWriter_String(&w, "TEMPERATURE") ||
        !TIREMO_JsonWriter_Key(&w, "severity") ||
        !TIREMO_JsonWriter_String(&w, active ? "MAJOR" : "INFO") ||
        !TIREMO_JsonWriter_Key(&w, "code") ||
        !TIREMO_JsonWriter_String(&w, active ? "TEMP_HIGH" : "TEMP_HIGH_NORMAL") ||
        !TIREMO_JsonWriter_Key(&w, "exception") ||
        !TIREMO_JsonWriter_String(&w, exception) ||
        !TIREMO_JsonWriter_ObjectEnd(&w))
    {
        TIREMO_LOG_ERROR("alarm: json failed");
        return;
    }
    len = TIREMO_JsonWriter_Finish(&w);
    (void)prvPublishBuf(s_topicAlarms, 1U, len);
}

static void prvEvaluateAlarm(uint32_t tempC)
{
    if (!s_alarmActive && (tempC >= TEMP_RAISE_C))
    {
        s_alarmActive = true;
        TIREMO_LOG_WARN("alarm: RAISE TEMP_HIGH tempC=%u", tempC);
        prvPublishAlarm(true);
    }
    else if (s_alarmActive && (tempC <= TEMP_CLEAR_C))
    {
        s_alarmActive = false;
        TIREMO_LOG_INFO("alarm: CLEAR TEMP_HIGH_NORMAL tempC=%u", tempC);
        prvPublishAlarm(false);
    }
}

bool TIREMO_Telemetry_Init(const char *pProductId, const char *pDeviceId)
{
    int n;

    if ((pProductId == NULL) || (pProductId[0] == '\0') ||
        (pDeviceId == NULL) || (pDeviceId[0] == '\0'))
    {
        return false;
    }
    n = snprintf(s_topicTelemetry, sizeof(s_topicTelemetry),
                 "pub/%s/%s/telemetry", pProductId, pDeviceId);
    if ((n <= 0) || ((uint32_t)n >= sizeof(s_topicTelemetry)))
    {
        return false;
    }
    n = snprintf(s_topicAlarms, sizeof(s_topicAlarms),
                 "pub/%s/%s/alarm", pProductId, pDeviceId);
    if ((n <= 0) || ((uint32_t)n >= sizeof(s_topicAlarms)))
    {
        return false;
    }
    s_ready = true;
    s_alarmActive = false;
    s_userHeldLogged = false;
    s_lastTelemetryMs = 0U;
    TIREMO_LOG_INFO("telemetry: topics %s  %s",
                    s_topicTelemetry, s_topicAlarms);
    return true;
}

void TIREMO_Telemetry_Poll(uint32_t nowMs, bool userHeld)
{
    uint32_t tempC;
    uint32_t intervalMs;

    if (!s_ready)
    {
        return;
    }
    if (s_bootMs == 0U)
    {
        s_bootMs = nowMs;
    }

    tempC = prvSampleTempC(userHeld);
    prvEvaluateAlarm(tempC);

    if (!TIREMO_Shadow_ReportingEnabled())
    {
        return;
    }
    intervalMs = prvIntervalMs();
    if ((s_lastTelemetryMs == 0U) ||
        ((nowMs - s_lastTelemetryMs) >= intervalMs))
    {
        prvPublishTelemetry(nowMs, tempC);
        s_lastTelemetryMs = nowMs;
    }
}
