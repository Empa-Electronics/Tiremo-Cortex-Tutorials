/******************************************************************************
 * @file    tiremo_lis2de12.h
 * @brief   Tiremo LIS2DE12 accelerometer driver API.
 *
 * Register-level driver over Tiremo_Hal_I2C (WriteRead / repeated START).
 * Production firmware wraps ST's PID library; this tutorial keeps CTRL_REG1,
 * CTRL_REG4, and OUT_X/Y/Z_H visible. See the README.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_LIS2DE12_H
#define TIREMO_LIS2DE12_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

bool TIREMO_LIS2DE12_Init(void);

/**
 * @brief Apply full-scale (g) and output data rate (Hz).
 *
 * Supported scale_g: 2, 4, 8, 16 (anything else falls back to 2).
 * Supported ODR: 0 (power-down), 1, 10, 25, 50, 100, 200, 400 Hz; a value
 * in between is rounded up to the next supported rate, and anything above
 * 200 selects 400 Hz.
 */
bool TIREMO_LIS2DE12_Configure(uint8_t scaleG, uint16_t dataRateHz);

bool TIREMO_LIS2DE12_ReadWhoAmI(uint8_t *pWhoAmI);

/**
 * @brief Read the acceleration in milli-g, averaged over 4 samples.
 *
 * Waits for each new sample (STATUS_REG ZYXDA, 200 ms timeout) and uses
 * the datasheet sensitivity that matches the last successful Configure()
 * call. Blocks for roughly 4 ODR periods (~40 ms at 100 Hz).
 */
bool TIREMO_LIS2DE12_ReadAccel(int16_t *pAccelX_mg,
                               int16_t *pAccelY_mg,
                               int16_t *pAccelZ_mg);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_LIS2DE12_H */
