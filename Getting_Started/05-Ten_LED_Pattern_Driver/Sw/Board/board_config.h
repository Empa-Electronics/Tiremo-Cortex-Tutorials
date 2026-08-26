/******************************************************************************
 * @file    board_config.h
 * @brief   Tiremo Cortex board pin configuration (Ten-LED Pattern Driver
 *          tutorial).
 *
 * Now carries the full LED bank (all 10) plus the USER button. See the
 * production board_config.h in the reference firmware for the complete
 * pin map (also sensors, ESP32).
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
#define TIREMO_BOARD_LED_COUNT         (10U)

#define TIREMO_BOARD_LED1_PORT         TIREMO_HAL_GPIO_PORT_B
#define TIREMO_BOARD_LED1_PIN          TIREMO_HAL_GPIO_PIN_4

#define TIREMO_BOARD_LED2_PORT         TIREMO_HAL_GPIO_PORT_B
#define TIREMO_BOARD_LED2_PIN          TIREMO_HAL_GPIO_PIN_9

#define TIREMO_BOARD_LED3_PORT         TIREMO_HAL_GPIO_PORT_F
#define TIREMO_BOARD_LED3_PIN          TIREMO_HAL_GPIO_PIN_7

#define TIREMO_BOARD_LED4_PORT         TIREMO_HAL_GPIO_PORT_B
#define TIREMO_BOARD_LED4_PIN          TIREMO_HAL_GPIO_PIN_5

#define TIREMO_BOARD_LED5_PORT         TIREMO_HAL_GPIO_PORT_B
#define TIREMO_BOARD_LED5_PIN          TIREMO_HAL_GPIO_PIN_10

#define TIREMO_BOARD_LED6_PORT         TIREMO_HAL_GPIO_PORT_B
#define TIREMO_BOARD_LED6_PIN          TIREMO_HAL_GPIO_PIN_11

#define TIREMO_BOARD_LED7_PORT         TIREMO_HAL_GPIO_PORT_B
#define TIREMO_BOARD_LED7_PIN          TIREMO_HAL_GPIO_PIN_12

#define TIREMO_BOARD_LED8_PORT         TIREMO_HAL_GPIO_PORT_B
#define TIREMO_BOARD_LED8_PIN          TIREMO_HAL_GPIO_PIN_13

#define TIREMO_BOARD_LED9_PORT         TIREMO_HAL_GPIO_PORT_B
#define TIREMO_BOARD_LED9_PIN          TIREMO_HAL_GPIO_PIN_14

#define TIREMO_BOARD_LED10_PORT        TIREMO_HAL_GPIO_PORT_B
#define TIREMO_BOARD_LED10_PIN         TIREMO_HAL_GPIO_PIN_15

/** @brief The USER button is active-low: pressed reads LOW. */
#define TIREMO_BOARD_BUTTON_ACTIVE_LOW     (1U)
#define TIREMO_BOARD_BUTTON_COUNT          (1U)

#define TIREMO_BOARD_USER_BUTTON_PORT      TIREMO_HAL_GPIO_PORT_C
#define TIREMO_BOARD_USER_BUTTON_PIN       TIREMO_HAL_GPIO_PIN_9
#define TIREMO_BOARD_USER_BUTTON_IRQ_PRIO  (3U)

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_BOARD_CONFIG_H */
