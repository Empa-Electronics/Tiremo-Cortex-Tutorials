/******************************************************************************
 * @file    board_config.h
 * @brief   Tiremo Cortex board pin configuration (LIS2DE12 tutorial).
 *
 * Trimmed to I2C2 and the LIS2DE12 7-bit address. SHT40 shares the bus
 * but is out of scope here.
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
