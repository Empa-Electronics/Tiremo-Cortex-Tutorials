/******************************************************************************
 * @file    prv_user_code.c
 * @brief   Ten-LED Pattern Driver -- all ten LEDs run the same pattern
 *          type at once, each on its own staggered interval, cycling
 *          SOLID -> BLINK -> PULSE automatically on a timer or
 *          immediately on a debounced USER button press.
 *
 * Retires Tutorial 02/03/04's dedicated LED1/LED2 loop -- they're just
 * LED_1/LED_2 among all ten now, driven by the same pattern engine as
 * everything else. PollButton() is Tutorial 04's exact debounce state
 * machine, now returning whether this tick was a press edge so the
 * caller can react to it.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Board/board_config.h"
#include "../Common/Include/tiremo_systick.h"
#include "../Common/Include/tiremo_debug.h"
#include "../Common/Include/tiremo_led_pattern.h"
#include "../Drivers/led/Include/tiremo_led.h"
#include "../Drivers/button/Include/tiremo_button.h"

#include <stdbool.h>
#include <stdint.h>

/** @brief How long each scene runs before auto-advancing. */
#define SCENE_INTERVAL_MS (8000U)

/** @brief LED_1's BLINK/PULSE period; each LED after it runs
 *  PATTERN_STEP_MS slower, so the ten LEDs visibly cascade instead of
 *  all changing in lockstep -- the point being made is that they're
 *  independently addressable, not just simultaneously lit. */
#define PATTERN_BASE_INTERVAL_MS (300U)
#define PATTERN_STEP_MS          (150U)

#define BUTTON_DEBOUNCE_MS       (30U)
#define BUTTON_HOLD_THRESHOLD_MS (10000U)

typedef enum
{
    SCENE_SOLID = 0,
    SCENE_BLINK,
    SCENE_PULSE,
    SCENE_COUNT
} Scene_t;

static const char *SceneName(Scene_t scene)
{
    switch (scene)
    {
    case SCENE_SOLID:
        return "SOLID";
    case SCENE_BLINK:
        return "BLINK";
    case SCENE_PULSE:
        return "PULSE";
    default:
        return "?";
    }
}

static void ApplyScene(Scene_t scene, uint32_t now)
{
    uint32_t index;
    TIREMO_LEDPattern_Type_t pattern;
    uint32_t intervalMs;

    switch (scene)
    {
    case SCENE_BLINK:
        pattern = TIREMO_LED_PATTERN_BLINK;
        break;
    case SCENE_PULSE:
        pattern = TIREMO_LED_PATTERN_PULSE;
        break;
    case SCENE_SOLID:
    default:
        pattern = TIREMO_LED_PATTERN_SOLID;
        break;
    }

    for (index = 0U; index < TIREMO_BOARD_LED_COUNT; index++)
    {
        intervalMs = PATTERN_BASE_INTERVAL_MS + (index * PATTERN_STEP_MS);
        TIREMO_LEDPattern_Set((TIREMO_LED_Id_t)index, pattern, intervalMs, now);
    }
}

/**
 * @brief Debounce the USER button; log press/release/hold as Tutorial 04
 *        did, and report whether this tick was a fresh debounced press.
 *
 * @param[in] now  Current TIREMO_SysTick_GetMs() reading for this loop tick.
 *
 * @return true exactly on the tick a debounced press edge is detected.
 */
static bool PollButton(uint32_t now)
{
    static bool stablePressed = false;
    static bool candidatePressed = false;
    static uint32_t candidateSinceMs = 0U;
    static uint32_t pressStartMs = 0U;
    static bool holdLogged = false;
    bool rawPressed;
    bool pressEdge = false;

    rawPressed = TIREMO_Button_IsPressed(TIREMO_BUTTON_USER);

    if (rawPressed != candidatePressed)
    {
        candidatePressed = rawPressed;
        candidateSinceMs = now;
    }

    if ((candidatePressed != stablePressed) &&
        ((now - candidateSinceMs) >= BUTTON_DEBOUNCE_MS))
    {
        stablePressed = candidatePressed;

        if (stablePressed)
        {
            pressStartMs = now;
            holdLogged = false;
            pressEdge = true;
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
        holdLogged = true;
        TIREMO_LOG_INFO("USER button: long hold reached (%u ms)",
                        BUTTON_HOLD_THRESHOLD_MS);
    }

    return pressEdge;
}

void PRV_USER_Code(void)
{
    uint32_t now;
    uint32_t sceneLastMs;
    Scene_t scene = SCENE_SOLID;

    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();

    TIREMO_LOG_INFO("Ten_LED_Pattern_Driver: boot");

    TIREMO_LED_Init();
    TIREMO_LEDPattern_Init();
    TIREMO_Button_Init();

    now = TIREMO_SysTick_GetMs();
    ApplyScene(scene, now);
    TIREMO_LOG_INFO("scene -> %s @ %lu ms", SceneName(scene), (unsigned long)now);
    sceneLastMs = now;

    for (;;)
    {
        now = TIREMO_SysTick_GetMs();

        TIREMO_LEDPattern_Service(now);

        if ((now - sceneLastMs) >= SCENE_INTERVAL_MS)
        {
            scene = (Scene_t)((scene + 1U) % SCENE_COUNT);
            ApplyScene(scene, now);
            TIREMO_LOG_INFO("scene -> %s (auto) @ %lu ms", SceneName(scene), (unsigned long)now);
            sceneLastMs = now;
        }

        if (PollButton(now))
        {
            scene = (Scene_t)((scene + 1U) % SCENE_COUNT);
            ApplyScene(scene, now);
            TIREMO_LOG_WARN("scene -> %s (button) @ %lu ms", SceneName(scene), (unsigned long)now);
            sceneLastMs = now;
        }
    }
}
