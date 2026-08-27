/******************************************************************************
 * @file    tiremo_systick.c
 * @brief   Tiremo system tick timing implementation.
 *
 * Provides millisecond timing services using the Tiremo SysTick HAL.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "../Include/tiremo_systick.h"

#include "../../Tiremo_Hal/Include/tiremo_hal_systick.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

#if defined(__GNUC__) || defined(__clang__)
#define TIREMO_SYSTICK_WEAK    __attribute__((weak))
#else
#define TIREMO_SYSTICK_WEAK
#endif

/******************************************************************************
 * Private Variables
 *****************************************************************************/

static volatile uint32_t s_delayCounter = 0U;
static volatile uint32_t s_msTick = 0U;
static bool s_initialized = false;

/******************************************************************************
 * Private Function Prototypes
 *****************************************************************************/

static void prvSysTickHandleTick(void);

/******************************************************************************
 * Public Functions
 *****************************************************************************/

bool TIREMO_SysTick_Init(void)
{
    s_delayCounter = 0U;
    s_msTick = 0U;
    s_initialized = TIREMO_HAL_SysTick_Init1Ms(prvSysTickHandleTick);

    return s_initialized;
}

void TIREMO_SysTick_DelayMs(uint32_t delayMs)
{
    if (!s_initialized)
    {
        return;
    }

    s_delayCounter = delayMs;
    while (s_delayCounter > 0U)
    {
        /* The SysTick interrupt decrements the delay counter. */
    }
}

uint32_t TIREMO_SysTick_GetMs(void)
{
    return s_msTick;
}

TIREMO_SYSTICK_WEAK void TIREMO_SysTick_OnTick1ms(void)
{
    /* Override in the application when one-millisecond work is required. */
}

/******************************************************************************
 * Private Functions
 *****************************************************************************/

static void prvSysTickHandleTick(void)
{
    if (s_delayCounter > 0U)
    {
        s_delayCounter--;
    }

    s_msTick++;
    TIREMO_SysTick_OnTick1ms();
}
