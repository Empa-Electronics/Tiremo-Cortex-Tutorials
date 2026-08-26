/**
 *******************************************************************************
 * @file        prv_scu_clk.c
 * @author      ABOV R&D Division
 * @brief       Template User Application Main
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

#include "hal_scu_clk.h"

#ifndef True
#define True true
#endif

#ifndef False
#define False false
#endif

/**********************************************************************
 * @brief       PRV_SCU_CLK_Init
 * @param[in]   None
 * @return      None
 **********************************************************************/
void PRV_SCU_CLK_Init(void)
{
    bool bPllAcived = True;
    bool bPllConnectedToMclk = True;

    SCUCLK_MCLK_CFG_t tMClkCfg =
    {
        .eMClk = SCUCLK_SRC_LSI,
        .ePreMClkDiv = SCUCLK_DIV_NONE,
        .ePostMClkDiv = SCUCLK_DIV_NONE   // HCLK
    };

    SCUCLK_PLL_CFG_t tPllCfg =
    {
        .eSrc = SCUCLK_PLL_SRC_HSI,
        .eSrcDiv = SCUCLK_DIV_NONE ,
        .un8PreDiv = 7,
        .un8PostDiv1 = 179,
        .un8PostDiv2 = 0,
        .un8OutDiv = 1,
        .eCurOpt = SCUCLK_PLL_CTRLOPT_10UA,
        .eVcoBias = SCUCLK_PLL_VCOBIAS_NONE,
        .ePllMode = False
    };

    /* Internal/External Clock Source Enable */
    (void)HAL_SCU_CLK_SetSrcEnable(SCUCLK_SRC_LSI, True);
    (void)HAL_SCU_CLK_SetSrcEnable(SCUCLK_SRC_HSI, True);
    (void)HAL_SCU_CLK_SetSrcEnable(SCUCLK_SRC_HSE, True);
    (void)HAL_SCU_CLK_SetSrcEnable(SCUCLK_SRC_LSE, True);

    /* 1. Set default LSI clock source */
    HAL_SCU_CLK_SetMClk(&tMClkCfg);

    /* 2. Set PLL */
    HAL_SCU_CLK_SetPLLConfig(bPllAcived, &tPllCfg);

    /* 3. Pll is connected to MCLK or not */
    if (bPllConnectedToMclk == true)
    {
        tMClkCfg.eMClk = SCUCLK_SRC_PLL;
        tMClkCfg.ePreMClkDiv = SCUCLK_DIV_NONE;
        tMClkCfg.ePostMClkDiv = SCUCLK_DIV_NONE;
    }
    else
    {
        tMClkCfg.eMClk = SCUCLK_SRC_PLL;
        tMClkCfg.ePreMClkDiv = SCUCLK_DIV_NONE;
        tMClkCfg.ePostMClkDiv = SCUCLK_DIV_NONE;
    }

    /* 4. Set MCLK */
    HAL_SCU_CLK_SetMClk(&tMClkCfg);

    /* 5. Peri Clock Divider */
    HAL_SCU_CLK_SetPClkDiv(SCUCLK_DIV_NONE);

    /* 6. Clock Output Enable */
    HAL_SCU_CLK_SetOutput(SCUCLK_SRC_LSI, 15, False);

    /* Internal/External Clock Source Disable */
    (void)HAL_SCU_CLK_SetSrcEnable(SCUCLK_SRC_LSI, False);
    (void)HAL_SCU_CLK_SetSrcEnable(SCUCLK_SRC_HSE, False);
    (void)HAL_SCU_CLK_SetSrcEnable(SCUCLK_SRC_LSE, False);
}

