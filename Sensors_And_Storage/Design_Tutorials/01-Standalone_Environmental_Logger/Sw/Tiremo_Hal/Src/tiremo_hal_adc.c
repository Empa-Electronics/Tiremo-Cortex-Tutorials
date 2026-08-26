/******************************************************************************
 * @file    tiremo_hal_adc.c
 * @brief   Tiremo ADC hardware abstraction implementation.
 *
 * Adapts the Tiremo ADC API to the ABOV ADC HAL.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "../Include/tiremo_hal_adc.h"

#include "hal_adc.h"

#include <stddef.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/** @brief Busy-wait loops allowed for one polled conversion to complete. */
#define TIREMO_HAL_ADC_CONV_DELAY_LOOPS    (10000U)

/******************************************************************************
 * Private Function Prototypes
 *****************************************************************************/

static bool prvADCIsIdValid(TIREMO_HAL_ADC_Id_t id);

/******************************************************************************
 * Public Functions
 *****************************************************************************/

TIREMO_HAL_ADC_Status_t TIREMO_HAL_ADC_ReadChannel(TIREMO_HAL_ADC_Id_t id,
                                                   uint8_t channel,
                                                   uint8_t samplingTime,
                                                   uint16_t *pRaw)
{
    HAL_ERR_e vendorStatus;
    ADC_SEQ_TRG_CFG_t seqCfg;
    ADC_SEQ_DATA_t seqData;
    volatile uint32_t i;

    if ((!prvADCIsIdValid(id)) || (pRaw == NULL))
    {
        return TIREMO_HAL_ADC_STATUS_ERROR;
    }

    seqCfg.eType = ADC_TRG_TYPE_INDEPENDENT;
    seqCfg.eTrgSrc = ADC_TRG_SRC_ADST;
    seqCfg.un8TrgNum = 0U;
    seqCfg.utCfg.tInd.un8SeqNum = 0U;
    seqCfg.utCfg.tInd.un8ChNum = channel;
    seqCfg.utCfg.tInd.un8SamplingTime = samplingTime;

    vendorStatus = HAL_ADC_SetSeqConfig((ADC_ID_e)id, &seqCfg);
    if (vendorStatus != HAL_ERR_OK)
    {
        return TIREMO_HAL_ADC_STATUS_ERROR;
    }

    vendorStatus = HAL_ADC_Start((ADC_ID_e)id);
    if (vendorStatus != HAL_ERR_OK)
    {
        return TIREMO_HAL_ADC_STATUS_ERROR;
    }

    for (i = 0U; i < TIREMO_HAL_ADC_CONV_DELAY_LOOPS; i++)
    {
    }

    seqData.bReadDDR = false;
    vendorStatus = HAL_ADC_GetData((ADC_ID_e)id, 0U, &seqData);
    if (vendorStatus != HAL_ERR_OK)
    {
        (void)HAL_ADC_Stop((ADC_ID_e)id);
        return TIREMO_HAL_ADC_STATUS_ERROR;
    }

    *pRaw = seqData.un16Result;
    (void)HAL_ADC_Stop((ADC_ID_e)id);

    return TIREMO_HAL_ADC_STATUS_OK;
}

/******************************************************************************
 * Private Functions
 *****************************************************************************/

static bool prvADCIsIdValid(TIREMO_HAL_ADC_Id_t id)
{
    return ((uint32_t)id < (uint32_t)TIREMO_HAL_ADC_ID_MAX);
}
