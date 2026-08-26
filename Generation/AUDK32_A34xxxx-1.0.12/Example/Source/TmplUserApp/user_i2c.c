/**
 *******************************************************************************
 * @file        user_i2c.c
 * @author      ABOV R&D Division
 * @brief       Template User Application Peripheral I2C V1x
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
#include "hal_i2c.h"

#ifndef True
#define True true
#endif

#ifndef False
#define False false
#endif

/* Generated Code */
/* >>> Placeholder for code generation */

extern void I2C_IRQHandler_I2C_ID_2(uint32_t un32Event, void *pContext);
static void Init_I2C_ID_2(void)
{
    HAL_ERR_e eErr = HAL_ERR_OK;

    I2C_CFG_t tCfg =
    {
        .eMode = I2C_MODE_MASTER,
        .uPeriod.tFreq.un32Freq = 100000,
        .tSdht.bEnable = True,
        .tSdht.un16Hold = 50,
    };

    eErr = HAL_I2C_Init(I2C_ID_2);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    eErr = HAL_I2C_SetConfig(I2C_ID_2, &tCfg);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    eErr = HAL_I2C_SetIRQ(I2C_ID_2, I2C_OPS_POLL, I2C_IRQHandler_I2C_ID_2, NULL, 3);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    return;
}


/* <<< End of Placeholder */

void PRV_I2C_Init(void)
{

    Init_I2C_ID_2();

    return;
}
