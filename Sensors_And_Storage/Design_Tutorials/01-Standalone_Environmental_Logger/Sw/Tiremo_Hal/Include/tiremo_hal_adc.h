/******************************************************************************
 * @file    tiremo_hal_adc.h
 * @brief   Tiremo ADC hardware abstraction API.
 *
 * Provides a vendor-independent single-channel polled ADC read over the
 * ABOV ADC HAL.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_HAL_ADC_H
#define TIREMO_HAL_ADC_H

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
 * Public Types
 *****************************************************************************/

typedef enum
{
    TIREMO_HAL_ADC_ID_0 = 0,
    TIREMO_HAL_ADC_ID_1,
    TIREMO_HAL_ADC_ID_MAX
} TIREMO_HAL_ADC_Id_t;

typedef enum
{
    TIREMO_HAL_ADC_STATUS_OK = 0,
    TIREMO_HAL_ADC_STATUS_ERROR
} TIREMO_HAL_ADC_Status_t;

/******************************************************************************
 * Public Function Prototypes
 *****************************************************************************/

/**
 * @brief Read one ADC channel using a single polled conversion.
 *
 * Configures an independent-trigger sequence on the given channel, starts a
 * software-triggered conversion, waits for it to complete, and reads back
 * the raw result.
 *
 * @param[in]  id            Target ADC peripheral.
 * @param[in]  channel       Channel number (0-N, or an internal reference
 *                           channel such as the VCORE bandgap channel).
 * @param[in]  samplingTime  Channel sampling time (HAL units).
 * @param[out] pRaw          Destination for the raw conversion result.
 *
 * @return ADC operation status.
 *
 * @retval TIREMO_HAL_ADC_STATUS_OK     Conversion completed.
 * @retval TIREMO_HAL_ADC_STATUS_ERROR  Invalid argument or vendor HAL failure.
 *
 * @warning Call PRV_ADC_Init() before using this function.
 */
TIREMO_HAL_ADC_Status_t TIREMO_HAL_ADC_ReadChannel(TIREMO_HAL_ADC_Id_t id,
                                                   uint8_t channel,
                                                   uint8_t samplingTime,
                                                   uint16_t *pRaw);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_HAL_ADC_H */
