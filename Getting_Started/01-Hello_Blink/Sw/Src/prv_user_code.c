/******************************************************************************
 * @file    prv_user_code.c
 * @brief   Hello Blink -- toggle LED1 on a crude busy-wait interval.
 *
 * Deliberately not using TIREMO_SysTick here -- that's Tutorial 02
 * (SysTick Timing), which replaces this exact loop with a non-blocking,
 * millisecond-accurate version. BLINK_DELAY_LOOPS is an approximate,
 * hand-tuned iteration count, not a calibrated time base; adjust it on
 * your own board if the blink rate is noticeably off from ~1 Hz.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Board/board_config.h"
#include "../Tiremo_Hal/Include/tiremo_hal_gpio.h"

#include <stdint.h>

/** @brief Empty-loop iterations per half-period. Approximate -- see file
 *  header. Tuned for the PLL-derived core clock PRV_SCU_CLK_Init() selects
 *  on the A34G43x Starter Kit. */
#define BLINK_DELAY_LOOPS (600000UL)

static void prvDelay(void)
{
    volatile uint32_t i;

    for (i = 0UL; i < BLINK_DELAY_LOOPS; i++)
    {
        /* empty -- volatile counter defeats -Og dead-loop elimination */
    }
}

void PRV_USER_Code(void)
{
    (void)TIREMO_HAL_GPIO_SetMode(TIREMO_BOARD_LED1_PORT, TIREMO_BOARD_LED1_PIN,
                                  TIREMO_HAL_GPIO_MODE_OUTPUT_PP);

    /* Active-low: HIGH = off. Start from a known state. */
    (void)TIREMO_HAL_GPIO_WritePin(TIREMO_BOARD_LED1_PORT, TIREMO_BOARD_LED1_PIN,
                                   TIREMO_HAL_GPIO_LEVEL_HIGH);

    for (;;)
    {
        (void)TIREMO_HAL_GPIO_TogglePin(TIREMO_BOARD_LED1_PORT,
                                        TIREMO_BOARD_LED1_PIN);
        prvDelay();
    }
}
