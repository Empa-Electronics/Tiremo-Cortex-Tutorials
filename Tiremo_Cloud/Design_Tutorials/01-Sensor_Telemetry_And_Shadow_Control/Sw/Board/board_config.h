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
#include "../Tiremo_Hal/Include/tiremo_hal_uart.h"
#include "../Tiremo_Hal/Include/tiremo_hal_i2c.h"

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

#define TIREMO_BOARD_SHT40_I2C_BUS          TIREMO_HAL_I2C_BUS_2
#define TIREMO_BOARD_SHT40_I2C_ADDRESS      (0x44U)

#define TIREMO_BOARD_LIS2DE12_I2C_BUS       TIREMO_HAL_I2C_BUS_2
#define TIREMO_BOARD_LIS2DE12_I2C_ADDRESS   (0x19U)

/* LED bank (same pins as Getting Started 05), driven from the shadow's
 * LIGHT_on key (all ten together). */
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

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_BOARD_CONFIG_H */
