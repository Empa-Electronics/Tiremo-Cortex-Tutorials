/******************************************************************************
 * @file    tiremo_clock.c
 * @brief   Tiremo software wall-clock implementation.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "../Include/tiremo_clock.h"
#include "../Include/tiremo_systick.h"

#include <stddef.h>

/******************************************************************************
 * Private Variables
 *****************************************************************************/

static bool s_synced = false;
static uint32_t s_epochAtSync = 0U;
static uint32_t s_msAtSync = 0U;

/******************************************************************************
 * Public Functions
 *****************************************************************************/

void TIREMO_Clock_SetEpoch(uint32_t epochSeconds)
{
    s_epochAtSync = epochSeconds;
    s_msAtSync = TIREMO_SysTick_GetMs();
    s_synced = true;
}

bool TIREMO_Clock_GetEpoch(uint32_t *pEpochSeconds)
{
    uint32_t elapsedMs;

    if ((!s_synced) || (pEpochSeconds == NULL))
    {
        return false;
    }

    elapsedMs = TIREMO_SysTick_GetMs() - s_msAtSync;
    *pEpochSeconds = s_epochAtSync + (elapsedMs / 1000U);

    return true;
}

bool TIREMO_Clock_IsSynced(void)
{
    return s_synced;
}
