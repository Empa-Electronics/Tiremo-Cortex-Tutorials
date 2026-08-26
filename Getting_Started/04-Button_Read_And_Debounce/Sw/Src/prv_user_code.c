/******************************************************************************
 * @file    prv_user_code.c
 * @brief   Button Read And Debounce -- Tutorial 03's two-LED loop, plus a
 *          debounced USER button with press/release/hold-duration logging.
 *
 * Keeps Tutorial 02/03's non-blocking LED1/LED2 toggle running exactly as
 * before, so the loop now shows three independent, non-blocking timed
 * things sharing one `for(;;)`. The new part is PollButton(): a small
 * debounce state machine on top of TIREMO_Button_IsPressed(), plus a
 * long-hold threshold check that fires the moment 10 s is reached without
 * waiting for release -- see the README's "Understanding Debounce" section
 * for why the reference driver itself has no debounce logic at all, and
 * why that's fine for the one thing it actually uses this pattern for
 * (factory-reset hold detection).
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Board/board_config.h"
#include "../Tiremo_Hal/Include/tiremo_hal_gpio.h"
#include "../Common/Include/tiremo_systick.h"
#include "../Common/Include/tiremo_debug.h"
#include "../Drivers/button/Include/tiremo_button.h"

#include <stdbool.h>
#include <stdint.h>

/** @brief LED1 half-period -- 2 s full blink cycle. */
#define LED1_INTERVAL_MS (1000U)

/** @brief LED2 half-period -- 10 s full cycle, deliberately much slower
 *  than LED1's so the two rates are unmistakable at a glance. */
#define LED2_INTERVAL_MS (5000U)

/** @brief A raw level must hold steady this long before it's accepted as
 *  a real press/release edge -- see the README for why 30 ms. */
#define BUTTON_DEBOUNCE_MS (30U)

/** @brief How long a debounced press must be held before it counts as a
 *  "long hold" rather than a short press. */
#define BUTTON_HOLD_THRESHOLD_MS (10000U)

/**
 * @brief Debounce raw button reads and log press/release/hold events.
 *
 * Two-stage state machine:
 *  1. Debounce: a raw level only becomes the "stable" level once it has
 *     read the same way for BUTTON_DEBOUNCE_MS straight.
 *  2. Hold detection: once stably pressed, a separate check fires exactly
 *     once at the BUTTON_HOLD_THRESHOLD_MS mark, without waiting for
 *     release -- this is the same shape the reference firmware's
 *     factory-reset hold check uses.
 *
 * @param[in] now  Current TIREMO_SysTick_GetMs() reading for this loop tick.
 */
static void PollButton(uint32_t now)
{
    static bool stablePressed = false;
    static bool candidatePressed = false;
    static uint32_t candidateSinceMs = 0U;
    static uint32_t pressStartMs = 0U;
    static bool holdLogged = false;
    bool rawPressed;

    rawPressed = TIREMO_Button_IsPressed(TIREMO_BUTTON_USER);

    if (rawPressed != candidatePressed)
    {
        /* The raw reading changed -- restart the debounce timer, but
         * don't accept it as real yet. Mechanical bounce shows up here
         * as several of these restarts in a couple of milliseconds. */
        candidatePressed = rawPressed;
        candidateSinceMs = now;
    }

    if ((candidatePressed != stablePressed) &&
        ((now - candidateSinceMs) >= BUTTON_DEBOUNCE_MS))
    {
        /* The candidate held steady for the full debounce window --
         * promote it to the stable state and log the edge. */
        stablePressed = candidatePressed;

        if (stablePressed)
        {
            pressStartMs = now;
            holdLogged = false;
            TIREMO_LOG_WARN("USER button: pressed");
        }
        else
        {
            uint32_t heldMs = now - pressStartMs;
            TIREMO_LOG_WARN("USER button: released after %lu ms%s",
                            (unsigned long)heldMs,
                            holdLogged ? " (was a long hold)" : "");
        }
    }

    if (stablePressed && !holdLogged &&
        ((now - pressStartMs) >= BUTTON_HOLD_THRESHOLD_MS))
    {
        /* Still held, and just crossed the threshold -- log it once,
         * right now, instead of waiting for the eventual release. */
        holdLogged = true;
        TIREMO_LOG_INFO("USER button: long hold reached (%u ms)",
                        BUTTON_HOLD_THRESHOLD_MS);
    }
}

void PRV_USER_Code(void)
{
    uint32_t led1LastMs;
    uint32_t led2LastMs;
    uint32_t now;

    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();

    TIREMO_LOG_INFO("Button_Read_And_Debounce: boot");

    (void)TIREMO_HAL_GPIO_SetMode(TIREMO_BOARD_LED1_PORT, TIREMO_BOARD_LED1_PIN,
                                  TIREMO_HAL_GPIO_MODE_OUTPUT_PP);
    (void)TIREMO_HAL_GPIO_SetMode(TIREMO_BOARD_LED2_PORT, TIREMO_BOARD_LED2_PIN,
                                  TIREMO_HAL_GPIO_MODE_OUTPUT_PP);

    /* Active-low: HIGH = off. Start both from a known state. */
    (void)TIREMO_HAL_GPIO_WritePin(TIREMO_BOARD_LED1_PORT, TIREMO_BOARD_LED1_PIN,
                                   TIREMO_HAL_GPIO_LEVEL_HIGH);
    (void)TIREMO_HAL_GPIO_WritePin(TIREMO_BOARD_LED2_PORT, TIREMO_BOARD_LED2_PIN,
                                   TIREMO_HAL_GPIO_LEVEL_HIGH);

    TIREMO_Button_Init();

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

        PollButton(now);
    }
}
