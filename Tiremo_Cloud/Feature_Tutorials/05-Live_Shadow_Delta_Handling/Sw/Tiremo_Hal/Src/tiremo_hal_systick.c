/******************************************************************************
 * @file    tiremo_hal_systick.c
 * @brief   Tiremo SysTick hardware abstraction implementation.
 *
 * Adapts the Tiremo SysTick API to the ARM CMSIS core interface.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "../Include/tiremo_hal_systick.h"

#include "abov_config.h"
#include <stddef.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

#define TIREMO_HAL_SYSTICK_FREQUENCY_HZ    (1000UL)

/******************************************************************************
 * Private Variables
 *****************************************************************************/

static TIREMO_HAL_SysTick_Callback_t s_tickCallback = NULL;

/******************************************************************************
 * Public Functions
 *****************************************************************************/

bool TIREMO_HAL_SysTick_Init1Ms(TIREMO_HAL_SysTick_Callback_t callback)
{
    uint32_t reloadTicks;

    if ((callback == NULL) ||
        (SystemCoreClock < TIREMO_HAL_SYSTICK_FREQUENCY_HZ))
    {
        return false;
    }

    reloadTicks = SystemCoreClock / TIREMO_HAL_SYSTICK_FREQUENCY_HZ;
    s_tickCallback = callback;

    if (SysTick_Config(reloadTicks) != 0U)
    {
        s_tickCallback = NULL;
        return false;
    }

    return true;
}

/**
 * @brief Handle the ARM SysTick interrupt.
 */
void SysTick_Handler(void)
{
    if (s_tickCallback != NULL)
    {
        s_tickCallback();
    }
}
