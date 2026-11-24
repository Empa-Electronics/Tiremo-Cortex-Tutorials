/******************************************************************************
 * @file    tiremo_telemetry.c
 * @brief   Shadow-gated accelerometer / temperature / humidity telemetry,
 *          plus a TEMP_HIGH alarm (raise above 30 C, clear below 30 C) on the
 *          real SHT40 temperature -- same alarm format as Feature 06.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Include/tiremo_telemetry.h"
#include "../Include/tiremo_shadow.h"
#include "../Include/tiremo_json.h"
#include "../Include/tiremo_debug.h"
#include "../../Drivers/esp32/Include/tiremo_esp32.h"
#include "../../Drivers/lis2de12/Include/tiremo_lis2de12.h"
#include "../../Drivers/sht40/Include/tiremo_sht40.h"

#include <stdio.h>
#include <string.h>

#define TELEMETRY_TOPIC_MAX   (192U)
#define TELEMETRY_JSON_MAX    (256U)
#define INTERVAL_S_MAX        (3600U)
#define ALARM_EXCEPTION_MAX   (64U)

/* TEMP_HIGH latch on the SHT40 temperature, in milli-degrees C. Raise
 * once when the temperature goes above 30 C, clear once when it drops
 * back below 30 C. Only the edges publish -- nothing repeats while the
 * temperature stays on one side. (A temperature sitting right at 30 C
 * can cross back and forth; lower ALARM_CLEAR_BELOW_MILLI_C, e.g. to
 * 29500, to add a margin.) */
#define ALARM_RAISE_ABOVE_MILLI_C   (30000)
#define ALARM_CLEAR_BELOW_MILLI_C   (30000)

static char s_topicTelemetry[TELEMETRY_TOPIC_MAX];
static char s_topicAlarm[TELEMETRY_TOPIC_MAX];
static char s_json[TELEMETRY_JSON_MAX];
static bool s_ready = false;
static bool s_alarmActive = false;
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

/* Append "key": value with two decimals (e.g. 27.45) from a milli-unit
 * reading. The JSON writer has no float type -- and the firmware avoids
 * float printf -- so the number is formatted as text and inserted raw. */
static bool prvAppendMilli2(TIREMO_JsonWriter_t *pW, const char *pKey,
                            int32_t milli)
{
    char num[16];
    uint32_t absMilli = (milli < 0) ? (uint32_t)(-milli) : (uint32_t)milli;
    uint32_t centi = (absMilli + 5U) / 10U;   /* round to 0.01 */
    int n;

    n = snprintf(num, sizeof(num), "%s%lu.%02lu", (milli < 0) ? "-" : "",
                 (unsigned long)(centi / 100U), (unsigned long)(centi % 100U));
    if ((n <= 0) || ((uint32_t)n >= sizeof(num)))
    {
        return false;
    }
    return TIREMO_JsonWriter_Key(pW, pKey) &&
           TIREMO_JsonWriter_Raw(pW, num, (uint32_t)n);
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

static bool prvAppendInt(TIREMO_JsonWriter_t *pW, const char *pKey, int32_t v)
{
    return TIREMO_JsonWriter_Key(pW, pKey) &&
           TIREMO_JsonWriter_Int(pW, v);
}

static void prvPublishAlarm(bool active)
{
    TIREMO_JsonWriter_t w;
    char exception[ALARM_EXCEPTION_MAX];
    uint32_t len;
    int n;

    if (active)
    {
        n = snprintf(exception, sizeof(exception), "Threshold %dC exceeded",
                     (int)(ALARM_RAISE_ABOVE_MILLI_C / 1000));
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

    /* Same four keys as Feature 06: Tiremo Cloud's alarm ingest accepts
     * only these. severity is MAJOR / MINOR / WARNING / INFO. */
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
    (void)prvPublishBuf(s_topicAlarm, 1U, len);
}

static void prvEvaluateAlarm(int32_t tempMilliC)
{
    if (!s_alarmActive && (tempMilliC > ALARM_RAISE_ABOVE_MILLI_C))
    {
        s_alarmActive = true;
        TIREMO_LOG_WARN("alarm: RAISE TEMP_HIGH temp=%d.%03d C",
                        (int)(tempMilliC / 1000), (int)(tempMilliC % 1000));
        prvPublishAlarm(true);
    }
    else if (s_alarmActive && (tempMilliC < ALARM_CLEAR_BELOW_MILLI_C))
    {
        s_alarmActive = false;
        TIREMO_LOG_INFO("alarm: CLEAR TEMP_HIGH_NORMAL temp=%d.%03d C",
                        (int)(tempMilliC / 1000), (int)(tempMilliC % 1000));
        prvPublishAlarm(false);
    }
}

static void prvPublishTelemetry(uint32_t nowMs)
{
    TIREMO_JsonWriter_t w;
    uint32_t len;
    uint32_t uptimeS = (nowMs - s_bootMs) / 1000U;
    int16_t ax = 0;
    int16_t ay = 0;
    int16_t az = 0;
    TIREMO_SHT40_Reading_t sht;
    bool needSht;
    bool haveTemp = false;

    TIREMO_JsonWriter_Init(&w, s_json, sizeof(s_json));
    if (!TIREMO_JsonWriter_ObjectStart(&w) ||
        !TIREMO_JsonWriter_Key(&w, "uptimeS") ||
        !TIREMO_JsonWriter_Uint(&w, uptimeS))
    {
        TIREMO_LOG_ERROR("telemetry: json failed");
        return;
    }

    if (TIREMO_Shadow_AccEn())
    {
        if (TIREMO_LIS2DE12_ReadAccel(&ax, &ay, &az))
        {
            if (!prvAppendInt(&w, "accX_mg", ax) ||
                !prvAppendInt(&w, "accY_mg", ay) ||
                !prvAppendInt(&w, "accZ_mg", az))
            {
                TIREMO_LOG_ERROR("telemetry: json failed");
                return;
            }
        }
        else
        {
            TIREMO_LOG_WARN("telemetry: accel read failed");
        }
    }

    needSht = TIREMO_Shadow_TempEn() || TIREMO_Shadow_HumEn();
    if (needSht)
    {
        if (TIREMO_SHT40_Read(&sht) == TIREMO_SHT40_STATUS_OK)
        {
            haveTemp = true;
            if (TIREMO_Shadow_TempEn())
            {
                if (!prvAppendMilli2(&w, "tempC", sht.temperatureMilliC))
                {
                    TIREMO_LOG_ERROR("telemetry: json failed");
                    return;
                }
            }
            if (TIREMO_Shadow_HumEn())
            {
                if (!prvAppendMilli2(&w, "humPct", sht.humidityMilliRh))
                {
                    TIREMO_LOG_ERROR("telemetry: json failed");
                    return;
                }
            }
        }
        else
        {
            TIREMO_LOG_WARN("telemetry: SHT40 read failed");
        }
    }

    if (!TIREMO_JsonWriter_ObjectEnd(&w))
    {
        TIREMO_LOG_ERROR("telemetry: json failed");
        return;
    }
    len = TIREMO_JsonWriter_Finish(&w);
    (void)prvPublishBuf(s_topicTelemetry, 0U, len);

    /* After the telemetry publish: the alarm reuses s_json. */
    if (haveTemp)
    {
        prvEvaluateAlarm(sht.temperatureMilliC);
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
    n = snprintf(s_topicAlarm, sizeof(s_topicAlarm),
                 "pub/%s/%s/alarm", pProductId, pDeviceId);
    if ((n <= 0) || ((uint32_t)n >= sizeof(s_topicAlarm)))
    {
        return false;
    }
    s_ready = true;
    s_alarmActive = false;
    s_lastTelemetryMs = 0U;
    TIREMO_LOG_INFO("telemetry: topics %s  %s", s_topicTelemetry, s_topicAlarm);
    return true;
}

void TIREMO_Telemetry_Poll(uint32_t nowMs)
{
    uint32_t intervalMs;
    bool anyEn;

    if (!s_ready)
    {
        return;
    }
    if (s_bootMs == 0U)
    {
        s_bootMs = nowMs;
    }

    anyEn = TIREMO_Shadow_AccEn() || TIREMO_Shadow_TempEn() ||
            TIREMO_Shadow_HumEn();
    if (!anyEn)
    {
        return;
    }
    intervalMs = prvIntervalMs();
    if ((s_lastTelemetryMs == 0U) ||
        ((nowMs - s_lastTelemetryMs) >= intervalMs))
    {
        prvPublishTelemetry(nowMs);
        s_lastTelemetryMs = nowMs;
    }
}

void TIREMO_Telemetry_Kick(void)
{
    s_lastTelemetryMs = 0U;
}
