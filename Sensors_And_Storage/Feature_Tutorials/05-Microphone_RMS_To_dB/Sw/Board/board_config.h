/******************************************************************************
 * @file    board_config.h
 * @brief   Tiremo Cortex board pin configuration (microphone tutorial).
 *
 * Mic wiring is expressed in vendor HAL types because the driver talks to
 * hal_adc / hal_timer1 / hal_pcu directly -- Tiremo_Hal has no burst-DMA
 * capture API. Copied from the reference board_config.h.
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

#include "hal_adc.h"
#include "hal_timer1.h"
#include "hal_pcu.h"

/** @brief ADC peripheral and channel the microphone is sampled on. */
#define TIREMO_BOARD_MIC_ADC_ID             ADC_ID_0
#define TIREMO_BOARD_MIC_ADC_CHANNEL        (0U)

/** @brief Timer1 instance that triggers each ADC burst capture. */
#define TIREMO_BOARD_MIC_TIMER_ID           TIMER1_ID_0

/** @brief PA0 (AN0) — analog input pin the microphone is wired to. */
#define TIREMO_BOARD_MIC_GPIO_PORT          PCU_ID_A
#define TIREMO_BOARD_MIC_GPIO_PIN           PCU_PIN_ID_0
#define TIREMO_BOARD_MIC_GPIO_ALT           PCU_ALT_7

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_BOARD_CONFIG_H */
