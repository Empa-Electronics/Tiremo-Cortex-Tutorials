/******************************************************************************
 * @file    tiremo_mic_bsp.h
 * @brief   Private microphone BSP interface — hardware capture layer.
 *
 * Responsibilities: PA0 analog-input setup, Timer1 sample-rate GRA, ADC
 * burst-mode runtime enable, HPL DMA arm/service, and delivery of captured
 * blocks to the app layer via a registered callback.
 *
 * ADC/Timer1 peripheral init stays in the vendor-generated user_adc.c /
 * user_timer1.c; this file only drives runtime start/stop/service on top of
 * that fixed configuration. Not for use outside Drivers/mic — tiremo_mic.h
 * is the public surface.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_MIC_BSP_H
#define TIREMO_MIC_BSP_H

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
 * Public Macros
 *****************************************************************************/

/** @brief Samples per DMA capture block. */
#define TIREMO_MIC_BSP_DMA_BLOCK_SIZE    (64U)

/******************************************************************************
 * Public Types
 *****************************************************************************/

/** @brief Raw DMA block ready — halfwords straight from the ADC DDR. */
typedef void (*TIREMO_MIC_BSP_DmaBlockFn_t)(const uint16_t *pRawSamples,
                                            uint32_t sampleCount);

/******************************************************************************
 * Public Function Prototypes
 *****************************************************************************/

/**
 * @brief Register the callback invoked when a DMA capture block completes.
 *
 * @param[in] handler  Callback, or NULL to clear it.
 */
void TIREMO_MIC_BSP_SetDataHandler(TIREMO_MIC_BSP_DmaBlockFn_t handler);

/**
 * @brief Configure PA0 as an analog input.
 */
void TIREMO_MIC_BSP_Init(void);

/**
 * @brief Set the Timer1 period for the requested sample rate.
 *
 * @param[in] sampleRateHz  Desired sample rate in Hz.
 *
 * @return true on success.
 *
 * @warning Must not be called while capturing.
 */
bool TIREMO_MIC_BSP_SetSampleRate(uint32_t sampleRateHz);

/**
 * @brief Start burst capture: ADC runtime enable, Timer1, DMA arm.
 *
 * @return true on success.
 */
bool TIREMO_MIC_BSP_Start(void);

/**
 * @brief Stop Timer1, ADC, and DMA.
 */
void TIREMO_MIC_BSP_Stop(void);

/**
 * @brief Poll the ADC DMA-complete flag and dispatch a completed block.
 *
 * @note Call frequently while capturing (no interrupt-driven path).
 */
void TIREMO_MIC_BSP_Service(void);

/**
 * @brief Number of DMA capture blocks completed since the last Start().
 */
uint32_t TIREMO_MIC_BSP_GetDmaBlockCount(void);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_MIC_BSP_H */
