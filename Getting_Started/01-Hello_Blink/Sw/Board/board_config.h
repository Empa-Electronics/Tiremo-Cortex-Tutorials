/******************************************************************************
 * @file    board_config.h
 * @brief   Tiremo Cortex board pin configuration (Hello Blink tutorial).
 *
 * Trimmed to just what this tutorial needs: LED1 only. See the full
 * production board_config.h in the reference firmware for the complete
 * pin map (all 10 LEDs, sensors, USER button, ESP32).
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_BOARD_CONFIG_H
#define TIREMO_BOARD_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../Tiremo_Hal/Include/tiremo_hal_gpio.h"

/** @brief LEDs are active-low on this board. */
#define TIREMO_BOARD_LED_ACTIVE_LOW    (1U)

#define TIREMO_BOARD_LED1_PORT         TIREMO_HAL_GPIO_PORT_B
#define TIREMO_BOARD_LED1_PIN          TIREMO_HAL_GPIO_PIN_4

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_BOARD_CONFIG_H */
