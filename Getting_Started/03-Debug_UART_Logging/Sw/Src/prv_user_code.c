/******************************************************************************
 * @file    prv_user_code.c
 * @brief   Debug UART Logging -- Tutorial 02's two-LED loop, now narrated
 *          over UART with leveled logging.
 *
 * Same non-blocking LED1/LED2 shape as Tutorial 02, with one addition:
 * every toggle is logged via TIREMO_LOG_INFO(). Rebuild with
 * `make TIREMO_DEBUG_LEVEL=2` (WARN) and these INFO lines disappear
 * entirely -- compiled out, not just filtered at runtime.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Board/board_config.h"
#include "../Tiremo_Hal/Include/tiremo_hal_gpio.h"
#include "../Common/Include/tiremo_systick.h"
#include "../Common/Include/tiremo_debug.h"

#include <stdint.h>

/** @brief LED1 half-period -- 2 s full blink cycle. */
#define LED1_INTERVAL_MS (1000U)

/** @brief LED2 half-period -- 10 s full cycle, deliberately much slower
 *  than LED1's so the two rates are unmistakable at a glance. */
#define LED2_INTERVAL_MS (5000U)

void PRV_USER_Code(void)
{
    uint32_t led1LastMs;
    uint32_t led2LastMs;
    uint32_t now;

    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();

    TIREMO_LOG_INFO("Debug_UART_Logging: boot");

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

        if ((now - led1LastMs) >= LED1_INTERVAL_MS)
        {
            (void)TIREMO_HAL_GPIO_TogglePin(TIREMO_BOARD_LED1_PORT,
                                            TIREMO_BOARD_LED1_PIN);
            TIREMO_LOG_INFO("LED1 toggle @ %lu ms", (unsigned long)now);
            led1LastMs = now;
        }

        if ((now - led2LastMs) >= LED2_INTERVAL_MS)
        {
            (void)TIREMO_HAL_GPIO_TogglePin(TIREMO_BOARD_LED2_PORT,
                                            TIREMO_BOARD_LED2_PIN);
            TIREMO_LOG_INFO("LED2 toggle @ %lu ms", (unsigned long)now);
            led2LastMs = now;
        }
    }
}
