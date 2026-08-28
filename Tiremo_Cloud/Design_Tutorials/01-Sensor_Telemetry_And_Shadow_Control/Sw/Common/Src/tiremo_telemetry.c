/******************************************************************************
 * @file    tiremo_telemetry.c
 * @brief   Shadow-gated accelerometer / temperature / humidity telemetry.
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

static char s_topicTelemetry[TELEMETRY_TOPIC_MAX];
static char s_json[TELEMETRY_JSON_MAX];
static bool s_ready = false;
static bool s_gyroSkipLogged = false;
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

static int32_t prvMilliToUnit(int32_t milli)
{
    if (milli >= 0)
    {
        return milli / 1000;
    }
    return -((-milli) / 1000);
}

static bool prvPublishBuf(const char *pTopic, uint32_t len)
{
    if ((len == 0U) ||
        !TIREMO_ESP32_MqttPublishRaw(pTopic, (const uint8_t *)s_json,
                                     len, 0U, false))
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

    if (TIREMO_Shadow_GyroEn())
    {
        if (!s_gyroSkipLogged)
        {
            TIREMO_LOG_INFO("telemetry: GYRO_en=1 but Cortex has no gyro; omitting gyro keys");
            s_gyroSkipLogged = true;
        }
    }
    else
    {
        s_gyroSkipLogged = false;
    }

    needSht = TIREMO_Shadow_TempEn() || TIREMO_Shadow_HumEn();
    if (needSht)
    {
        if (TIREMO_SHT40_Read(&sht) == TIREMO_SHT40_STATUS_OK)
        {
            if (TIREMO_Shadow_TempEn())
            {
                if (!prvAppendInt(&w, "tempC",
                                  prvMilliToUnit(sht.temperatureMilliC)))
                {
                    TIREMO_LOG_ERROR("telemetry: json failed");
                    return;
                }
            }
            if (TIREMO_Shadow_HumEn())
            {
                if (!prvAppendInt(&w, "humPct",
                                  prvMilliToUnit(sht.humidityMilliRh)))
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
    (void)prvPublishBuf(s_topicTelemetry, len);
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
    s_ready = true;
    s_gyroSkipLogged = false;
    s_lastTelemetryMs = 0U;
    TIREMO_LOG_INFO("telemetry: topic %s", s_topicTelemetry);
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

    anyEn = TIREMO_Shadow_AccEn() || TIREMO_Shadow_GyroEn() ||
            TIREMO_Shadow_TempEn() || TIREMO_Shadow_HumEn();
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
