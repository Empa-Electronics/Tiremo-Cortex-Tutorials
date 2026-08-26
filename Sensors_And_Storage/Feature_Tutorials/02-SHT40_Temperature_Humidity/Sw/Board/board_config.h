/******************************************************************************
 * @file    board_config.h
 * @brief   Tiremo Cortex board pin configuration (SHT40 tutorial).
 *
 * Trimmed to just what this tutorial needs: I2C2 and the SHT40 address
 * on it. LIS2DE12 shares the same bus but is out of scope here -- see
 * 01-I2C_Bus_Bringup and 03-LIS2DE12_Accelerometer.
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

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_BOARD_CONFIG_H */
