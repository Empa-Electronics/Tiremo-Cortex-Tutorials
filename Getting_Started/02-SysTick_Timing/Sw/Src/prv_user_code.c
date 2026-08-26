/******************************************************************************
 * @file    prv_user_code.c
 * @brief   SysTick Timing -- two LEDs, two independent rates, one loop.
 *
 * Replaces Tutorial 01's blocking busy-wait with elapsed-time comparisons
 * against TIREMO_SysTick_GetMs(). Neither branch below ever blocks, so
 * both LEDs run at their own accurate, independent rate from a single
 * for(;;) -- the same non-blocking shape TIREMO_APP_Run() uses in the
 * full reference firmware.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Board/board_config.h"
#include "../Tiremo_Hal/Include/tiremo_hal_gpio.h"
#include "../Common/Include/tiremo_systick.h"

#include <stdint.h>

/** @brief LED1 half-period -- 1000 ms, so a full on/off cycle takes 2 s. */
#define LED1_INTERVAL_MS (1000U)

/** @brief LED2 half-period -- 5000 ms (10 s full cycle), deliberately much
 *  slower than LED1's so the two rates are unmistakable at a glance on
 *  real hardware. */
#define LED2_INTERVAL_MS (5000U)

void PRV_USER_Code(void)
{
    uint32_t led1LastMs;
    uint32_t led2LastMs;
    uint32_t now;

    (void)TIREMO_SysTick_Init();

    (void)TIREMO_HAL_GPIO_SetMode(TIREMO_BOARD_LED1_PORT, TIREMO_BOARD_LED1_PIN,
                                  TIREMO_HAL_GPIO_MODE_OUTPUT_PP);
    (void)TIREMO_HAL_GPIO_SetMode(TIREMO_BOARD_LED2_PORT, TIREMO_BOARD_LED2_PIN,
                                  TIREMO_HAL_GPIO_MODE_OUTPUT_PP);

    /* Active-low: HIGH = off. Start both from a known state. */
    (void)TIREMO_HAL_GPIO_WritePin(TIREMO_BOARD_LED1_PORT, TIREMO_BOARD_LED1_PIN,
                                   TIREMO_HAL_GPIO_LEVEL_HIGH);
    (void)TIREMO_HAL_GPIO_WritePin(TIREMO_BOARD_LED2_PORT, TIREMO_BOARD_LED2_PIN,
                                   TIREMO_HAL_GPIO_LEVEL_HIGH);

    led1LastMs = TIREMO_SysTick_GetMs();
    led2LastMs = led1LastMs;

    for (;;)
    {
        now = TIREMO_SysTick_GetMs();

        /* Unsigned subtraction: correct across the ms counter's wraparound
         * at UINT32_MAX, same idiom the reference firmware uses throughout
         * its connection/retry timing. */
        if ((now - led1LastMs) >= LED1_INTERVAL_MS)
        {
            (void)TIREMO_HAL_GPIO_TogglePin(TIREMO_BOARD_LED1_PORT,
                                            TIREMO_BOARD_LED1_PIN);
            led1LastMs = now;
        }

        if ((now - led2LastMs) >= LED2_INTERVAL_MS)
        {
            (void)TIREMO_HAL_GPIO_TogglePin(TIREMO_BOARD_LED2_PORT,
                                            TIREMO_BOARD_LED2_PIN);
            led2LastMs = now;
        }
    }
}
