/******************************************************************************
 * @file    prv_user_code.c
 * @brief   Software Wall Clock -- SetEpoch once, log GetEpoch every second.
 *
 * NTP is not in this tutorial. A known epoch is planted by hand so the
 * offset math can be checked against a real elapsed-time clock.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Common/Include/tiremo_systick.h"
#include "../Common/Include/tiremo_debug.h"
#include "../Common/Include/tiremo_clock.h"

#include <stdint.h>

/** @brief 2024-01-01 00:00:00 UTC -- a known value, not a live NTP sample. */
#define TUTORIAL_EPOCH_SECONDS (1704067200UL)

#define LOG_INTERVAL_MS (1000U)

void PRV_USER_Code(void)
{
    uint32_t now;
    uint32_t lastLogMs;
    uint32_t epoch;

    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();

    TIREMO_LOG_INFO("Software_Wall_Clock: boot");

    TIREMO_Clock_SetEpoch(TUTORIAL_EPOCH_SECONDS);
    TIREMO_LOG_INFO("clock: SetEpoch(%lu)", (unsigned long)TUTORIAL_EPOCH_SECONDS);

    lastLogMs = TIREMO_SysTick_GetMs();

    for (;;)
    {
        now = TIREMO_SysTick_GetMs();

        if ((now - lastLogMs) >= LOG_INTERVAL_MS)
        {
            if (TIREMO_Clock_GetEpoch(&epoch))
            {
                TIREMO_LOG_INFO("clock: epoch=%lu  elapsed=%lu s",
                                (unsigned long)epoch,
                                (unsigned long)(epoch - TUTORIAL_EPOCH_SECONDS));
            }
            else
            {
                TIREMO_LOG_WARN("clock: not synced");
            }
            lastLogMs = now;
        }
    }
}
