/**
 *******************************************************************************
 * @file        user_adc.c
 * @author      ABOV R&D Division
 * @brief       Template User Application Peripheral ADC V1x
 *
 * Copyright 2024 ABOV Semiconductor Co.,Ltd. All rights reserved.
 *
 * This file is licensed under terms that are found in the LICENSE file
 * located at Document directory.
 * If this file is delivered or shared without applicable license terms,
 * the terms of the BSD-3-Clause license shall be applied.
 * Reference: https://opensource.org/licenses/BSD-3-Clause
 ******************************************************************************/

#include "abov_config.h"
#include "abov_module_config.h"
#include "hal_adc.h"

#ifndef True
#define True true
#endif

#ifndef False
#define False false
#endif

/* Generated Code */
/* >>> Placeholder for code generation */

extern void ADC_IRQHandler_ADC_ID_0(uint32_t un32Event, void *pContext);
static void Init_ADC_ID_0(void)
{
    HAL_ERR_e eErr = HAL_ERR_OK;

    ADC_CLK_CFG_t tClkCfg = 
    {
        .eClk = ADC_CLK_PCLK,
        .un8PClkDiv = 2,
        .bPClkInv = False
    };

    ADC_CFG_t tCfg =
    {
        .eMode = ADC_MODE_BURST,
        .eBaseTrgSrc = ADC_TRG_SRC_ADST,
        .un8SeqCnt = 0,
        .un8SamplingTime = 0,
        .bChInfo = False,
        .bTrgInfo = False,
        .bSleep = True,
    };

    ADC_SEQ_TRG_CFG_t tSeqCfg =
    {
        .eType = ADC_TRG_TYPE_INDEPENDENT
    };

    eErr = HAL_ADC_Init(ADC_ID_0);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    eErr = HAL_ADC_SetClkConfig(ADC_ID_0, &tClkCfg);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    eErr = HAL_ADC_SetConfig(ADC_ID_0, &tCfg);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    eErr = HAL_ADC_SetIRQ(ADC_ID_0, ADC_OPS_POLL, ADC_IRQHandler_ADC_ID_0, NULL, 3);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    /* ADC Sequence 0 */
    tSeqCfg.eTrgSrc = ADC_TRG_SRC_TIMER1;
    tSeqCfg.un8TrgNum = 0;
    tSeqCfg.utCfg.tInd.un8SeqNum = 0;
    tSeqCfg.utCfg.tInd.un8ChNum = 0;
    tSeqCfg.utCfg.tInd.un8SamplingTime = 8;
    eErr = HAL_ADC_SetSeqConfig(ADC_ID_0, &tSeqCfg);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    return;
}


extern void ADC_IRQHandler_ADC_ID_1(uint32_t un32Event, void *pContext);
static void Init_ADC_ID_1(void)
{
    HAL_ERR_e eErr = HAL_ERR_OK;

    ADC_CLK_CFG_t tClkCfg = 
    {
        .eClk = ADC_CLK_PCLK,
        .un8PClkDiv = 2,
        .bPClkInv = False
    };

    ADC_CFG_t tCfg =
    {
        .eMode = ADC_MODE_SINGLE,
        .eBaseTrgSrc = ADC_TRG_SRC_ADST,
        .un8SeqCnt = 0,
        .un8SamplingTime = 0,
        .bChInfo = False,
        .bTrgInfo = False,
        .bSleep = True,
    };

    eErr = HAL_ADC_Init(ADC_ID_1);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    eErr = HAL_ADC_SetClkConfig(ADC_ID_1, &tClkCfg);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    eErr = HAL_ADC_SetConfig(ADC_ID_1, &tCfg);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    eErr = HAL_ADC_SetIRQ(ADC_ID_1, ADC_OPS_POLL, ADC_IRQHandler_ADC_ID_1, NULL, 3);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    return;
}


/* <<< End of Placeholder */

void PRV_ADC_Init(void)
{

    Init_ADC_ID_0();
    Init_ADC_ID_1();

    return;
}
