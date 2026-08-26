/******************************************************************************
 * @file    tiremo_led_pattern.h
 * @brief   Non-blocking solid/blink/pulse pattern engine for Tiremo LEDs.
 *
 * There is no reference implementation for this module -- the production
 * firmware's LED driver (Drivers/led) only ever turns LEDs fully on or
 * off; it has no concept of a running pattern. This engine is built on
 * top of that raw driver, the same way Common/tiremo_debug.c is built on
 * top of Tiremo_Hal/tiremo_hal_uart.c: a generic, reusable service layer
 * over a hardware driver, not one-off application logic.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_LED_PATTERN_H
#define TIREMO_LED_PATTERN_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../Drivers/led/Include/tiremo_led.h"

#include <stdint.h>

typedef enum
{
    TIREMO_LED_PATTERN_OFF = 0,
    TIREMO_LED_PATTERN_SOLID,
    TIREMO_LED_PATTERN_BLINK,
    TIREMO_LED_PATTERN_PULSE
} TIREMO_LEDPattern_Type_t;

/**
 * @brief Reset every LED's pattern state to OFF.
 *
 * @warning Call TIREMO_LED_Init() before this function.
 */
void TIREMO_LEDPattern_Init(void);

/**
 * @brief Assign a pattern to one LED, independent of every other LED.
 *
 * Takes effect immediately: OFF/SOLID apply the LED state right away;
 * BLINK/PULSE start their first phase timed from @p now.
 *
 * @param[in] ledId      Target LED identifier.
 * @param[in] pattern    Pattern to run.
 * @param[in] intervalMs Full period in ms for BLINK/PULSE; ignored for
 *                       OFF/SOLID.
 * @param[in] now        Current TIREMO_SysTick_GetMs() reading.
 */
void TIREMO_LEDPattern_Set(TIREMO_LED_Id_t ledId,
                           TIREMO_LEDPattern_Type_t pattern,
                           uint32_t intervalMs,
                           uint32_t now);

/**
 * @brief Advance every LED's pattern state machine by one tick.
 *
 * Non-blocking; call this every main-loop iteration. LEDs set to
 * OFF/SOLID cost nothing here -- only BLINK/PULSE do per-tick timing.
 *
 * @param[in] now  Current TIREMO_SysTick_GetMs() reading.
 */
void TIREMO_LEDPattern_Service(uint32_t now);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_LED_PATTERN_H */
