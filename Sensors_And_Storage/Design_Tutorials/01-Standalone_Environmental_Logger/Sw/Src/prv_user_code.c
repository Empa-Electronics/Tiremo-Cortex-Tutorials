/******************************************************************************
 * @file    prv_user_code.c
 * @brief   Standalone Environmental Logger -- sample, persist, replay.
 *
 * Composes Feature Tutorials 02–07: SHT40, LIS2DE12, battery, mic, flash
 * ring, SysTick. On boot, dumps flash history, then samples on a fixed
 * interval and appends to the ring.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Common/Include/tiremo_systick.h"
#include "../Common/Include/tiremo_debug.h"
#include "../Common/Include/tiremo_banner.h"
#include "../Common/Include/tiremo_envlog.h"
#include "../Drivers/sht40/Include/tiremo_sht40.h"
#include "../Drivers/lis2de12/Include/tiremo_lis2de12.h"
#include "../Drivers/battery/Include/tiremo_battery.h"
#include "../Drivers/mic/Include/tiremo_mic.h"

#include <stddef.h>
#include <stdint.h>

#define SAMPLE_INTERVAL_MS (3000U)

#define FLAG_SHT40    (1U << 0)
#define FLAG_ACCEL    (1U << 1)
#define FLAG_BATTERY  (1U << 2)

static void LogSample(const char *pTag, const TIREMO_EnvSample_t *pSample)
{
    int32_t t = pSample->temperatureMilliC;
    int32_t rh = pSample->humidityMilliRh;
    const char *tSign = "";
    const char *rhSign = "";
    int32_t tWhole;
    int32_t tFrac;
    int32_t rhWhole;
    int32_t rhFrac;

    if (t < 0)
    {
        tSign = "-";
        t = -t;
    }
    if (rh < 0)
    {
        rhSign = "-";
        rh = -rh;
    }
    tWhole = t / 1000;
    tFrac = t % 1000;
    rhWhole = rh / 1000;
    rhFrac = rh % 1000;

    (void)pTag;
    TIREMO_LOG_INFO("%s seq=%u up=%u ms  T=%s%d.%03d C  RH=%s%d.%03d %%RH  "
                    "acc=%d/%d/%d mg  AVDD=%u mV  mic=%d dB  flags=0x%02X",
                    pTag,
                    (unsigned)pSample->seq,
                    (unsigned)pSample->uptimeMs,
                    tSign, (int)tWhole, (int)tFrac,
                    rhSign, (int)rhWhole, (int)rhFrac,
                    (int)pSample->accelX_mg,
                    (int)pSample->accelY_mg,
                    (int)pSample->accelZ_mg,
                    (unsigned)pSample->avddMv,
                    (int)pSample->micDb,
                    (unsigned)pSample->flags);
}

static void DumpHistory(void)
{
    uint32_t i;
    uint32_t count = TIREMO_EnvLog_Count();
    TIREMO_EnvSample_t sample;

    if (count == 0U)
    {
        TIREMO_LOG_INFO("history: (empty)");
        return;
    }

    TIREMO_LOG_INFO("history: %u sample(s), oldest first", (unsigned)count);
    for (i = 0U; i < count; i++)
    {
        if (TIREMO_EnvLog_GetOldest(i, &sample))
        {
            LogSample("hist", &sample);
        }
    }
}

static void TakeSample(void)
{
    TIREMO_EnvSample_t sample;
    TIREMO_SHT40_Reading_t sht;
    int16_t ax = 0;
    int16_t ay = 0;
    int16_t az = 0;
    uint32_t avdd = 0U;

    sample.seq = 0U;
    sample.uptimeMs = TIREMO_SysTick_GetMs();
    sample.temperatureMilliC = 0;
    sample.humidityMilliRh = 0;
    sample.accelX_mg = 0;
    sample.accelY_mg = 0;
    sample.accelZ_mg = 0;
    sample.flags = 0U;
    sample.avddMv = 0U;
    sample.micDb = TIREMO_MIC_GetVoiceLevelDb();

    if (TIREMO_SHT40_Read(&sht) == TIREMO_SHT40_STATUS_OK)
    {
        sample.temperatureMilliC = sht.temperatureMilliC;
        sample.humidityMilliRh = sht.humidityMilliRh;
        sample.flags |= FLAG_SHT40;
    }

    if (TIREMO_LIS2DE12_ReadAccel(&ax, &ay, &az))
    {
        sample.accelX_mg = ax;
        sample.accelY_mg = ay;
        sample.accelZ_mg = az;
        sample.flags |= FLAG_ACCEL;
    }

    if (TIREMO_BAT_Read(&avdd, NULL))
    {
        sample.avddMv = avdd;
        sample.flags |= FLAG_BATTERY;
    }

    if (!TIREMO_EnvLog_Append(&sample))
    {
        TIREMO_LOG_WARN("sample: flash append failed");
        LogSample("live", &sample);
        return;
    }

    LogSample("live", &sample);
}

void PRV_USER_Code(void)
{
    uint32_t now;
    uint32_t lastSampleMs;

    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();

    TIREMO_Banner_Print("Standalone Environmental Logger");

    TIREMO_LOG_INFO("Standalone_Environmental_Logger: boot");

    if (!TIREMO_EnvLog_Init())
    {
        TIREMO_LOG_ERROR("envlog: flash init failed");
        for (;;)
        {
        }
    }

    TIREMO_SHT40_Init();
    if (!TIREMO_LIS2DE12_Init())
    {
        TIREMO_LOG_WARN("LIS2DE12: init failed, accel samples will be empty");
    }
    if (!TIREMO_BAT_Init())
    {
        TIREMO_LOG_WARN("BAT: init failed");
    }
    if (!TIREMO_MIC_Init() || !TIREMO_MIC_Start())
    {
        TIREMO_LOG_WARN("MIC: init/start failed");
    }

    /* After sensor init so tdf.py monitor's reset-then-open window
     * usually still catches the replay (same edge case as tutorial 04). */
    DumpHistory();

    lastSampleMs = TIREMO_SysTick_GetMs();

    for (;;)
    {
        TIREMO_MIC_Service();

        now = TIREMO_SysTick_GetMs();
        if ((now - lastSampleMs) >= SAMPLE_INTERVAL_MS)
        {
            TakeSample();
            lastSampleMs = now;
        }
    }
}
