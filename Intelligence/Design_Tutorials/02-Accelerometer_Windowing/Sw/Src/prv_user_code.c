/******************************************************************************
 * @file    prv_user_code.c
 * @brief   LIS2DE12 tumbling window of 8 samples x 3 axes, UART dump.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Common/Include/tiremo_accel_window.h"
#include "../Common/Include/tiremo_banner.h"
#include "../Common/Include/tiremo_debug.h"
#include "../Common/Include/tiremo_systick.h"
#include "../Drivers/lis2de12/Include/tiremo_lis2de12.h"

#include <stdint.h>

static uint32_t s_windowIndex;

static bool InitAccel(void)
{
    uint8_t whoAmI = 0U;

    if (!TIREMO_LIS2DE12_Init())
    {
        TIREMO_LOG_ERROR("LIS2DE12: init failed (WHO_AM_I or Configure)");
        return false;
    }

    if (!TIREMO_LIS2DE12_Configure(TIREMO_ACCEL_SCALE_G, TIREMO_ACCEL_HW_ODR_HZ))
    {
        TIREMO_LOG_ERROR("LIS2DE12: Configure 2 g / 10 Hz failed");
        return false;
    }

    (void)TIREMO_LIS2DE12_ReadWhoAmI(&whoAmI);
    TIREMO_LOG_INFO("LIS2DE12: WHO_AM_I=0x%02X  FS=%u g  ODR=%u Hz  pace=%u ms",
                    whoAmI,
                    (unsigned)TIREMO_ACCEL_SCALE_G,
                    (unsigned)TIREMO_ACCEL_HW_ODR_HZ,
                    (unsigned)TIREMO_ACCEL_SAMPLE_INTERVAL_MS);
    return true;
}

static void DumpWindow(const float *pFeatures)
{
    uint8_t i;

    TIREMO_LOG_INFO("window[%u] samples=%u features=%u",
                    (unsigned)s_windowIndex,
                    (unsigned)TIREMO_ACCEL_WINDOW_SAMPLES,
                    (unsigned)TIREMO_ACCEL_FEATURE_COUNT);

    for (i = 0U; i < TIREMO_ACCEL_WINDOW_SAMPLES; i++)
    {
        TIREMO_LOG_INFO("  s%u  x=%d  y=%d  z=%d mg",
                        (unsigned)i,
                        (int)pFeatures[(i * TIREMO_ACCEL_NUM_AXES) + 0U],
                        (int)pFeatures[(i * TIREMO_ACCEL_NUM_AXES) + 1U],
                        (int)pFeatures[(i * TIREMO_ACCEL_NUM_AXES) + 2U]);
    }

    s_windowIndex++;
}

static bool SampleOnce(TIREMO_AccelWindow_t *pWindow)
{
    int16_t xMg;
    int16_t yMg;
    int16_t zMg;

    if (!TIREMO_LIS2DE12_ReadAccel(&xMg, &yMg, &zMg))
    {
        TIREMO_LOG_WARN("LIS2DE12: read failed");
        return false;
    }

    return TIREMO_AccelWindow_Push(pWindow,
                                   (float)xMg,
                                   (float)yMg,
                                   (float)zMg);
}

void PRV_USER_Code(void)
{
    TIREMO_AccelWindow_t window;
    uint32_t now;
    uint32_t lastSampleMs;

    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();
    TIREMO_Banner_Print("Accelerometer Windowing");

    if (!InitAccel())
    {
        for (;;)
        {
        }
    }

    TIREMO_AccelWindow_Reset(&window);
    s_windowIndex = 0U;
    lastSampleMs = TIREMO_SysTick_GetMs();

    for (;;)
    {
        now = TIREMO_SysTick_GetMs();

        if ((now - lastSampleMs) >= TIREMO_ACCEL_SAMPLE_INTERVAL_MS)
        {
            lastSampleMs = now;
            if (SampleOnce(&window) && TIREMO_AccelWindow_IsFull(&window))
            {
                DumpWindow(TIREMO_AccelWindow_Data(&window));
                TIREMO_AccelWindow_Reset(&window);
            }
        }
    }
}
