/******************************************************************************
 * @file    board_config.h
 * @brief   Tiremo Cortex board pin configuration (battery ADC tutorial).
 *
 * No dedicated battery-sense pin: AVDD is inferred from the internal
 * VCORE bandgap channel on ADC1.
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

#include "../Tiremo_Hal/Include/tiremo_hal_adc.h"

/** @brief ADC peripheral used for the VCORE reference read. */
#define TIREMO_BOARD_BATTERY_ADC_ID         TIREMO_HAL_ADC_ID_1

/** @brief Internal VCORE bandgap reference channel (AVDD calculation). */
#define TIREMO_BOARD_BATTERY_VCORE_CHANNEL  (23U)

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_BOARD_CONFIG_H */
