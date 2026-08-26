/******************************************************************************
 * @file    prv_user_code.c
 * @brief   Battery Voltage via ADC -- log VCORE raw code and AVDD mV.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Common/Include/tiremo_systick.h"
#include "../Common/Include/tiremo_debug.h"
#include "../Drivers/battery/Include/tiremo_battery.h"

#include <stdint.h>

#define READ_INTERVAL_MS (1000U)

static void ReadAndLog(void)
{
    uint32_t avddMv;
    uint16_t vcoreRaw;

    if (!TIREMO_BAT_Read(&avddMv, &vcoreRaw))
    {
        TIREMO_LOG_WARN("BAT: read failed");
        return;
    }

    TIREMO_LOG_INFO("BAT: VCORE raw=%u  AVDD=%u mV",
                    (unsigned)vcoreRaw, (unsigned)avddMv);
}

void PRV_USER_Code(void)
{
    uint32_t now;
    uint32_t lastReadMs;

    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();

    TIREMO_LOG_INFO("Battery_Voltage_ADC: boot");

    if (!TIREMO_BAT_Init())
    {
        TIREMO_LOG_ERROR("BAT: init failed");
        for (;;)
        {
        }
    }

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
