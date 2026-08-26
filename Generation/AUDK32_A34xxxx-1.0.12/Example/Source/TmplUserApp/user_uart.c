/**
 *******************************************************************************
 * @file        user_uart.c
 * @author      ABOV R&D Division
 * @brief       Template User Application Peripheral UART V1x
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
#include "hal_uart.h"

#ifndef True
#define True true
#endif

#ifndef False
#define False false
#endif

/* Generated Code */
/* >>> Placeholder for code generation */

extern void UART_IRQHandler_UART_ID_0(uint32_t un32Event, void *pContext);
static void Init_UART_ID_0(void)
{
    HAL_ERR_e eErr = HAL_ERR_OK;

    UART_CLK_CFG_t tClkCfg =
    {
        .eClk = UART_CLK_MCCR,
        .eMccr = UART_CLK_MCCR_MCLK,
        .un8MccrDiv = 2,
    };

    UART_CFG_t tCfg =
    {
        .un32BaudRate = 115200,
        .eData = UART_DATA_8,
        .eParity = UART_PARITY_NONE,
        .eStop = UART_STOP_1,
        .bIntrLSEnable = True
    };

    eErr = HAL_UART_Init(UART_ID_0);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    eErr = HAL_UART_SetClkConfig(UART_ID_0, &tClkCfg);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    eErr = HAL_UART_SetConfig(UART_ID_0, &tCfg);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    eErr = HAL_UART_SetIRQ(UART_ID_0, UART_OPS_INTR, UART_IRQHandler_UART_ID_0, NULL, 3);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    return;
}


extern void UART_IRQHandler_UART_ID_1(uint32_t un32Event, void *pContext);
static void Init_UART_ID_1(void)
{
    HAL_ERR_e eErr = HAL_ERR_OK;

    UART_CLK_CFG_t tClkCfg =
    {
        .eClk = UART_CLK_MCCR,
        .eMccr = UART_CLK_MCCR_MCLK,
        .un8MccrDiv = 2,
    };

    UART_CFG_t tCfg =
    {
        .un32BaudRate = 115200,
        .eData = UART_DATA_8,
        .eParity = UART_PARITY_NONE,
        .eStop = UART_STOP_1,
        .bIntrLSEnable = True
    };

    eErr = HAL_UART_Init(UART_ID_1);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    eErr = HAL_UART_SetClkConfig(UART_ID_1, &tClkCfg);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    eErr = HAL_UART_SetConfig(UART_ID_1, &tCfg);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    eErr = HAL_UART_SetIRQ(UART_ID_1, UART_OPS_INTR, UART_IRQHandler_UART_ID_1, NULL, 3);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    return;
}


extern void UART_IRQHandler_UART_ID_2(uint32_t un32Event, void *pContext);
static void Init_UART_ID_2(void)
{
    HAL_ERR_e eErr = HAL_ERR_OK;

    UART_CLK_CFG_t tClkCfg =
    {
        .eClk = UART_CLK_MCCR,
        .eMccr = UART_CLK_MCCR_MCLK,
        .un8MccrDiv = 2,
    };

    UART_CFG_t tCfg =
    {
        .un32BaudRate = 115200,
        .eData = UART_DATA_8,
        .eParity = UART_PARITY_NONE,
        .eStop = UART_STOP_1,
        .bIntrLSEnable = True
    };

    eErr = HAL_UART_Init(UART_ID_2);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    eErr = HAL_UART_SetClkConfig(UART_ID_2, &tClkCfg);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    eErr = HAL_UART_SetConfig(UART_ID_2, &tCfg);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    eErr = HAL_UART_SetIRQ(UART_ID_2, UART_OPS_INTR, UART_IRQHandler_UART_ID_2, NULL, 3);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    return;
}


/* <<< End of Placeholder */

void PRV_UART_Init(void)
{

    Init_UART_ID_0();
    Init_UART_ID_1();
    Init_UART_ID_2();

    return;
}
