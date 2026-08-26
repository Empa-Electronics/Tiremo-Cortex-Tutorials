/******************************************************************************
 * @file    board_config.h
 * @brief   Tiremo Cortex board pin configuration (environmental logger).
 *
 * Union of the Feature Tutorial 02–05 maps: I2C2 sensors, VCORE battery
 * channel, and vendor-typed mic burst capture.
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
#include "../Tiremo_Hal/Include/tiremo_hal_adc.h"
#include "hal_adc.h"
#include "hal_timer1.h"
#include "hal_pcu.h"

#define TIREMO_BOARD_SHT40_I2C_BUS          TIREMO_HAL_I2C_BUS_2
#define TIREMO_BOARD_SHT40_I2C_ADDRESS      (0x44U)

#define TIREMO_BOARD_LIS2DE12_I2C_BUS       TIREMO_HAL_I2C_BUS_2
#define TIREMO_BOARD_LIS2DE12_I2C_ADDRESS   (0x19U)

#define TIREMO_BOARD_BATTERY_ADC_ID         TIREMO_HAL_ADC_ID_1
#define TIREMO_BOARD_BATTERY_VCORE_CHANNEL  (23U)

#define TIREMO_BOARD_MIC_ADC_ID             ADC_ID_0
#define TIREMO_BOARD_MIC_ADC_CHANNEL        (0U)
#define TIREMO_BOARD_MIC_TIMER_ID           TIMER1_ID_0
#define TIREMO_BOARD_MIC_GPIO_PORT          PCU_ID_A
#define TIREMO_BOARD_MIC_GPIO_PIN           PCU_PIN_ID_0
#define TIREMO_BOARD_MIC_GPIO_ALT           PCU_ALT_7

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_BOARD_CONFIG_H */
