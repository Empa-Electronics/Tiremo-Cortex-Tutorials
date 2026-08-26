/******************************************************************************
 * @file    prv_user_code.c
 * @brief   LIS2DE12 Accelerometer -- configure ±2 g / 100 Hz, log mg.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Common/Include/tiremo_systick.h"
#include "../Common/Include/tiremo_debug.h"
#include "../Drivers/lis2de12/Include/tiremo_lis2de12.h"

#include <stdint.h>

#define READ_INTERVAL_MS (500U)

static void ReadAndLog(void)
{
    int16_t xMg;
    int16_t yMg;
    int16_t zMg;

    if (!TIREMO_LIS2DE12_ReadAccel(&xMg, &yMg, &zMg))
    {
        TIREMO_LOG_WARN("LIS2DE12: read failed");
        return;
    }

    TIREMO_LOG_INFO("LIS2DE12: X=%d mg  Y=%d mg  Z=%d mg",
                    (int)xMg, (int)yMg, (int)zMg);
}

void PRV_USER_Code(void)
{
    uint8_t whoAmI = 0U;
    uint32_t now;
    uint32_t lastReadMs;

    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();

    TIREMO_LOG_INFO("LIS2DE12_Accelerometer: boot");

    if (!TIREMO_LIS2DE12_Init())
    {
        TIREMO_LOG_ERROR("LIS2DE12: init failed (WHO_AM_I or Configure)");
        for (;;)
        {
        }
    }

    (void)TIREMO_LIS2DE12_ReadWhoAmI(&whoAmI);
    TIREMO_LOG_INFO("LIS2DE12: WHO_AM_I=0x%02X  FS=2g  ODR=100 Hz", whoAmI);

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
