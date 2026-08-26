/******************************************************************************
 * @file    tiremo_led_pattern.c
 * @brief   Non-blocking solid/blink/pulse pattern engine for Tiremo LEDs.
 *
 * See tiremo_led_pattern.h for why this module exists and where it sits
 * in the layering. One state machine per LED, each independently timed
 * off TIREMO_SysTick_GetMs() -- the same unsigned-subtraction idiom
 * every tutorial since 02-SysTick_Timing has used.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Include/tiremo_led_pattern.h"
#include "../../Board/board_config.h"

#include <stdbool.h>

/** @brief PULSE's "on" slice as a percentage of its full period -- a
 *  short bright burst followed by a long rest, so it reads as a
 *  distinct heartbeat rather than a second BLINK. There is no PWM/dimming
 *  here: TIREMO_HAL_GPIO is digital-only, so "pulse" means duty cycle,
 *  not brightness. */
#define TIREMO_LEDPATTERN_PULSE_ON_PCT (20U)

typedef struct
{
    TIREMO_LEDPattern_Type_t pattern;
    uint32_t intervalMs;
    uint32_t lastChangeMs;
    bool isOn;
} TIREMO_LEDPattern_State_t;

static TIREMO_LEDPattern_State_t s_state[TIREMO_BOARD_LED_COUNT];

static bool prvIsValidId(TIREMO_LED_Id_t ledId)
{
    return ((uint32_t)ledId < TIREMO_BOARD_LED_COUNT);
}

void TIREMO_LEDPattern_Init(void)
{
    uint32_t index;

    for (index = 0U; index < TIREMO_BOARD_LED_COUNT; index++)
    {
        s_state[index].pattern = TIREMO_LED_PATTERN_OFF;
        s_state[index].intervalMs = 0U;
        s_state[index].lastChangeMs = 0U;
        s_state[index].isOn = false;
    }
}

void TIREMO_LEDPattern_Set(TIREMO_LED_Id_t ledId,
                           TIREMO_LEDPattern_Type_t pattern,
                           uint32_t intervalMs,
                           uint32_t now)
{
    TIREMO_LEDPattern_State_t *pState;

    if (!prvIsValidId(ledId))
    {
        return;
    }

    pState = &s_state[ledId];
    pState->pattern = pattern;
    pState->intervalMs = intervalMs;
    pState->lastChangeMs = now;

    switch (pattern)
    {
    case TIREMO_LED_PATTERN_SOLID:
        pState->isOn = true;
        break;
    case TIREMO_LED_PATTERN_OFF:
    default:
        pState->isOn = false;
        break;
    }

    TIREMO_LED_SetState(ledId, pState->isOn);
}

void TIREMO_LEDPattern_Service(uint32_t now)
{
    uint32_t index;
    TIREMO_LEDPattern_State_t *pState;
    uint32_t onMs;
    uint32_t offMs;
    uint32_t phaseMs;

    for (index = 0U; index < TIREMO_BOARD_LED_COUNT; index++)
    {
        pState = &s_state[index];

        if ((pState->pattern != TIREMO_LED_PATTERN_BLINK) &&
            (pState->pattern != TIREMO_LED_PATTERN_PULSE))
        {
            continue; /* OFF/SOLID need no per-tick timing */
        }

        if (pState->pattern == TIREMO_LED_PATTERN_BLINK)
        {
            phaseMs = pState->intervalMs / 2U; /* symmetric 50/50 duty cycle */
        }
        else
        {
            onMs = (pState->intervalMs * TIREMO_LEDPATTERN_PULSE_ON_PCT) / 100U;
            offMs = pState->intervalMs - onMs;
            phaseMs = pState->isOn ? onMs : offMs;
        }

        if ((now - pState->lastChangeMs) >= phaseMs)
        {
            pState->isOn = !pState->isOn;
            TIREMO_LED_SetState((TIREMO_LED_Id_t)index, pState->isOn);
            pState->lastChangeMs = now;
        }
    }
}
