/******************************************************************************
 * @file    board_config.h
 * @brief   Tiremo Cortex board pin configuration (AT Command Transport).
 *
 * Trimmed to UART2 + PA12 ESP32 supply gate. Debug console stays on
 * UART0 via PRV_UART_Init() / TIREMO_Debug_Init(); this file only names
 * the module link.
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
#include "../Tiremo_Hal/Include/tiremo_hal_i2c.h"
#include "../Tiremo_Hal/Include/tiremo_hal_uart.h"

/**
 * UART port used to send AT commands to the ESP32 WiFi module.
 *
 * PA8 = ESP32 TX -> MCU RX, PA9 = ESP32 RX <- MCU TX (UART2).
 * Debug logging stays on UART0; these are two different peripherals.
 */
#define TIREMO_BOARD_ESP32_UART_PORT        TIREMO_HAL_UART_PORT_2

/** @brief GPIO that gates the ESP32 module's power supply (PA12). */
#define TIREMO_BOARD_ESP32_PWR_PORT         TIREMO_HAL_GPIO_PORT_A
#define TIREMO_BOARD_ESP32_PWR_PIN          TIREMO_HAL_GPIO_PIN_12

/**
 * ESP32 supply enable polarity. PA12 is active-high: HIGH = powered.
 */
#define TIREMO_BOARD_ESP32_PWR_ON           TIREMO_HAL_GPIO_LEVEL_HIGH
#define TIREMO_BOARD_ESP32_PWR_OFF          TIREMO_HAL_GPIO_LEVEL_LOW

/** @brief How long to hold the supply off before turning it back on. */
#define TIREMO_BOARD_ESP32_PWR_OFF_MS       (500U)

/** @brief Boot delay after supply-on before the first AT probe. */
#define TIREMO_BOARD_ESP32_PWR_BOOT_MS      (3000U)

/** @brief The USER button is active-low: pressed reads LOW. */
#define TIREMO_BOARD_BUTTON_ACTIVE_LOW     (1U)
#define TIREMO_BOARD_BUTTON_COUNT          (1U)

#define TIREMO_BOARD_USER_BUTTON_PORT      TIREMO_HAL_GPIO_PORT_C
#define TIREMO_BOARD_USER_BUTTON_PIN       TIREMO_HAL_GPIO_PIN_9
#define TIREMO_BOARD_USER_BUTTON_IRQ_PRIO  (3U)

/** @brief I2C bus used by the LIS2DE12 sensor. */
#define TIREMO_BOARD_LIS2DE12_I2C_BUS       TIREMO_HAL_I2C_BUS_2

/**
 * @brief 7-bit I2C address of the LIS2DE12 sensor.
 *
 * SA0 = 1 maps to ST 8-bit write address 0x33 -> 7-bit 0x19.
 */
#define TIREMO_BOARD_LIS2DE12_I2C_ADDRESS   (0x19U)

/** @brief Full-scale in g passed to TIREMO_LIS2DE12_Configure(). */
#define TIREMO_ACCEL_SCALE_G                (2U)

/** @brief Nearest LIS2DE12 ODR to the 8 Hz software sample rate. */
#define TIREMO_ACCEL_HW_ODR_HZ              (10U)

/** @brief Software sample period: 8 Hz. */
#define TIREMO_ACCEL_SAMPLE_INTERVAL_MS     (125U)

/** @brief Tumbling window length (matches Tiremo_AI TIREMO_WINDOW_SIZE). */
#define TIREMO_ACCEL_WINDOW_SAMPLES         (8U)

#define TIREMO_ACCEL_NUM_AXES               (3U)
#define TIREMO_ACCEL_FEATURE_COUNT          (TIREMO_ACCEL_WINDOW_SAMPLES * TIREMO_ACCEL_NUM_AXES)

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_BOARD_CONFIG_H */
