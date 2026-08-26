/******************************************************************************
 * @file    tiremo_battery.h
 * @brief   Tiremo battery (AVDD supply) driver API.
 *
 * Measures the MCU's own AVDD supply rail by reading the internal VCORE
 * bandgap reference channel and back-computing the supply voltage. Useful
 * as a coarse battery/supply-voltage indicator with no dedicated sense pin.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_BATTERY_H
#define TIREMO_BATTERY_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/******************************************************************************
 * Public Function Prototypes
 *****************************************************************************/

/**
 * @brief Initialize the battery driver.
 *
 * Waits for the internal VCORE reference to stabilize.
 *
 * @return true when the driver was initialized successfully.
 *
 * @warning Call PRV_ADC_Init() before this function.
 */
bool TIREMO_BAT_Init(void);

/**
 * @brief Deinitialize the battery driver.
 */
void TIREMO_BAT_Deinit(void);

/**
 * @brief Check whether the battery driver is initialized.
 *
 * @return true when TIREMO_BAT_Init() succeeded.
 */
bool TIREMO_BAT_IsInitialized(void);

/**
 * @brief Read the AVDD supply voltage.
 *
 * @param[out] pAvddMv    AVDD supply voltage in millivolts.
 * @param[out] pVcoreRaw  Optional raw VCORE ADC value (NULL to ignore).
 *
 * @return true when the measurement completed successfully.
 */
bool TIREMO_BAT_Read(uint32_t *pAvddMv, uint16_t *pVcoreRaw);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_BATTERY_H */
