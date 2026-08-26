/**
 *******************************************************************************
 * @file        user_timer1.c
 * @author      ABOV R&D Division
 * @brief       Template User Application Peripheral Timer1 V1x
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
#include "hal_timer1.h"

#ifndef True
#define True true
#endif

#ifndef False
#define False false
#endif

/* Generated Code */
/* >>> Placeholder for code generation */

extern void Timer1_IRQHandler_TIMER1_ID_0(uint32_t un32Event, void *pContext);
static void Init_TIMER1_ID_0(void)
{
    HAL_ERR_e eErr = HAL_ERR_OK;

    TIMER1_CLK_CFG_t tClkCfg =
    {
        .eClk = TIMER1_CLK_PCLK,
        .uSubClk.ePClkDiv = TIMER1_PCLK_DIV_2,
        .un16PreScale = 0,
    };

    TIMER1_CFG_t tCfg =
    {
        .eMode = TIMER1_MODE_PERIODIC,
        .ePol = TIMER1_POL_LOW,
        .bIntrEnable = False,
        .bOVFIntrEnable = False,
        .utData.tGRD.un16DataA = 5624,
        .utData.tGRD.un16DataB = 1,
    };

    TIMER1_ADCTRG_CFG_t tAdcCfg =
    {
        .bEnable = True,
        .eMode = TIMER1_ADCTRG_MODE_NORMAL,
    };

    eErr = HAL_TIMER1_Init(TIMER1_ID_0);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    eErr = HAL_TIMER1_SetClkConfig(TIMER1_ID_0, &tClkCfg);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    eErr = HAL_TIMER1_SetConfig(TIMER1_ID_0, &tCfg);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    eErr = HAL_TIMER1_SetIRQ(TIMER1_ID_0, TIMER1_OPS_POLL, Timer1_IRQHandler_TIMER1_ID_0, NULL, 3);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    eErr = HAL_TIMER1_SetInOutPort(TIMER1_ID_0, TIMER1_PORT_IN);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    eErr = HAL_TIMER1_SetInversionPort(TIMER1_ID_0, TIMER1_INV_OFF);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    eErr = HAL_TIMER1_SetAdcTrgConfig(TIMER1_ID_0, &tAdcCfg);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }
    return;
}


/* <<< End of Placeholder */

void PRV_Timer1_Init(void)
{

    Init_TIMER1_ID_0();

    return;
}
