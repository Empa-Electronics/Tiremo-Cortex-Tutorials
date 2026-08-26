/******************************************************************************
 * @file    prv_user_code.c
 * @brief   SHT40 Temperature / Humidity -- read once a second and log.
 *
 * Application policy only: 1 Hz sample rate and how the milli-units
 * are printed. The SHT4x command/response, CRC, and conversion live in
 * Drivers/sht40 -- same split Tutorial 04 documented (generic reusable
 * protocol in the driver, one-off loop policy here).
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Common/Include/tiremo_systick.h"
#include "../Common/Include/tiremo_debug.h"
#include "../Drivers/sht40/Include/tiremo_sht40.h"

#include <stdint.h>

/** @brief How often a new reading is taken after the initial boot read. */
#define READ_INTERVAL_MS (1000U)

static void LogMilli(const char *pLabel, int32_t milli, const char *pUnit)
{
    const char *pSign = "";
    int32_t whole;
    int32_t frac;

    if (milli < 0)
    {
        pSign = "-";
        milli = -milli;
    }

    whole = milli / 1000;
    frac = milli % 1000;

    TIREMO_LOG_INFO("%s: %s%d.%03d %s", pLabel, pSign, (int)whole, (int)frac, pUnit);
}

static void ReadAndLog(void)
{
    TIREMO_SHT40_Reading_t reading;

    if (TIREMO_SHT40_Read(&reading) != TIREMO_SHT40_STATUS_OK)
    {
        TIREMO_LOG_WARN("SHT40: read failed (NACK or CRC)");
        return;
    }

    LogMilli("SHT40 T", reading.temperatureMilliC, "C");
    LogMilli("SHT40 RH", reading.humidityMilliRh, "%RH");
}

void PRV_USER_Code(void)
{
    uint32_t now;
    uint32_t lastReadMs;

    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();
    TIREMO_SHT40_Init();

    TIREMO_LOG_INFO("SHT40_Temperature_Humidity: boot");

    ReadAndLog();
    lastReadMs = TIREMO_SysTick_GetMs();

    for (;;)
    {
        now = TIREMO_SysTick_GetMs();

        if ((now - lastReadMs) >= READ_INTERVAL_MS)
        {
            ReadAndLog();
            lastReadMs = now;
        }
    }
}
