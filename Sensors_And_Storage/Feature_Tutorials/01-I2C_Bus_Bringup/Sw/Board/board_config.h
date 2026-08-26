/******************************************************************************
 * @file    board_config.h
 * @brief   Tiremo Cortex board pin configuration (I2C Bus Bring-up tutorial).
 *
 * Trimmed to just what this tutorial needs: the shared I2C2 bus and the
 * two sensor addresses on it. See the production board_config.h for the
 * complete pin map (LEDs, button, mic, ESP32, ...).
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

#include "../Tiremo_Hal/Include/tiremo_hal_i2c.h"

/******************************************************************************
 * SHT40 Temperature / Humidity Sensor
 *****************************************************************************/

/** @brief I2C bus used by the SHT40 sensor. */
#define TIREMO_BOARD_SHT40_I2C_BUS          TIREMO_HAL_I2C_BUS_2

/** @brief 7-bit I2C address of the SHT40 sensor. */
#define TIREMO_BOARD_SHT40_I2C_ADDRESS      (0x44U)

/******************************************************************************
 * LIS2DE12 Accelerometer
 *****************************************************************************/

/** @brief I2C bus used by the LIS2DE12 sensor (shared with SHT40). */
#define TIREMO_BOARD_LIS2DE12_I2C_BUS       TIREMO_HAL_I2C_BUS_2

/**
 * @brief 7-bit I2C address of the LIS2DE12 sensor.
 *
 * SA0 = 1 maps to ST 8-bit write address 0x33 -> 7-bit 0x19.
 */
#define TIREMO_BOARD_LIS2DE12_I2C_ADDRESS   (0x19U)

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_BOARD_CONFIG_H */
