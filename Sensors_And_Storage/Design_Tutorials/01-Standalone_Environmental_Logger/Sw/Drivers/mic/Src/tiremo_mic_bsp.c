/******************************************************************************
 * @file    tiremo_mic_bsp.c
 * @brief   Private microphone BSP implementation.
 *
 * Capture sequence (Start): ADC/Timer1 runtime enable (register writes, not
 * HAL_ADC_SetConfig — that stays in user_adc.c) -> poll-mode IRQ setup ->
 * Timer1 start -> ADC start -> DMA arm. Runtime: TIREMO_MIC_BSP_Service()
 * polls the ADC SR.DMAF flag, dispatches the completed block to the
 * registered handler, and re-arms the DMA for the next block.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "tiremo_mic_bsp.h"
#include "tiremo_mic_adc_reg.h"
#include "../../../Board/board_config.h"

#include "hal_adc.h"
#include "hal_pcu.h"
#include "hal_timer1.h"
#include "hpl_dma.h"

#include <stddef.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

#define TIREMO_MIC_ADC_IRQ_PRIORITY    (3U)

/******************************************************************************
 * Private Variables
 *****************************************************************************/

extern void ADC_IRQHandler_ADC_ID_0(uint32_t un32Event, void *pContext);
extern uint32_t SystemCoreClock;

static uint16_t s_dmaBuffer[TIREMO_MIC_BSP_DMA_BLOCK_SIZE] __attribute__((aligned(4)));
static uint32_t s_dmaBlockCount = 0U;
static bool s_captureActive = false;
static bool s_dmaBackendReady = false;
static DMA_ID_e s_dmaCh = DMA_ID_MAX;
static TIREMO_MIC_BSP_DmaBlockFn_t s_pfnDmaBlock = NULL;

/******************************************************************************
 * Private Function Prototypes
 *****************************************************************************/

static uint16_t prvMicCalcGra(uint32_t sampleRateHz);
static bool prvMicDmaBackendInit(void);
static bool prvMicArmDma(void);
static void prvMicDmaTeardown(void);

/******************************************************************************
 * Public Functions
 *****************************************************************************/

void TIREMO_MIC_BSP_SetDataHandler(TIREMO_MIC_BSP_DmaBlockFn_t handler)
{
    s_pfnDmaBlock = handler;
}

void TIREMO_MIC_BSP_Init(void)
{
    (void)HAL_PCU_SetInOutMode(TIREMO_BOARD_MIC_GPIO_PORT,
                               TIREMO_BOARD_MIC_GPIO_PIN,
                               PCU_INOUT_ANG_INPUT);
    (void)HAL_PCU_SetAltMode(TIREMO_BOARD_MIC_GPIO_PORT,
                             TIREMO_BOARD_MIC_GPIO_PIN,
                             TIREMO_BOARD_MIC_GPIO_ALT);
}

bool TIREMO_MIC_BSP_SetSampleRate(uint32_t sampleRateHz)
{
    uint16_t gra;

    if (s_captureActive || (sampleRateHz == 0U))
    {
        return false;
    }

    gra = prvMicCalcGra(sampleRateHz);

    return (HAL_TIMER1_SetData(TIREMO_BOARD_MIC_TIMER_ID,
                               TIMER1_DATA_A,
                               (uint32_t)gra) == HAL_ERR_OK);
}

bool TIREMO_MIC_BSP_Start(void)
{
    HAL_ERR_e err;

    if (s_captureActive)
    {
        return true;
    }

    err = HAL_ADC_SetIRQ(TIREMO_BOARD_MIC_ADC_ID,
                         ADC_OPS_POLL,
                         ADC_IRQHandler_ADC_ID_0,
                         NULL,
                         TIREMO_MIC_ADC_IRQ_PRIORITY);
    if (err != HAL_ERR_OK)
    {
        return false;
    }

    prvMicAdcSetEnable(prvMicAdcReg(), true);
    prvMicAdcSetAutoRestart(prvMicAdcReg(), true);

    s_captureActive = true;
    s_dmaBlockCount = 0U;

    (void)HAL_TIMER1_Start(TIREMO_BOARD_MIC_TIMER_ID);

    err = HAL_ADC_Start(TIREMO_BOARD_MIC_ADC_ID);
    if (err != HAL_ERR_OK)
    {
        (void)HAL_TIMER1_Stop(TIREMO_BOARD_MIC_TIMER_ID);
        s_captureActive = false;
        return false;
    }

    if (!prvMicArmDma())
    {
        (void)HAL_TIMER1_Stop(TIREMO_BOARD_MIC_TIMER_ID);
        (void)HAL_ADC_Stop(TIREMO_BOARD_MIC_ADC_ID);
        s_captureActive = false;
        return false;
    }

    return true;
}

void TIREMO_MIC_BSP_Stop(void)
{
    if (!s_captureActive)
    {
        return;
    }

    prvMicDmaTeardown();
    (void)HAL_TIMER1_Stop(TIREMO_BOARD_MIC_TIMER_ID);
    (void)HAL_ADC_Stop(TIREMO_BOARD_MIC_ADC_ID);
    s_captureActive = false;
}

void TIREMO_MIC_BSP_Service(void)
{
    TIREMO_MIC_AdcReg_t *pAdc;

    if (!s_captureActive)
    {
        return;
    }

    pAdc = prvMicAdcReg();
    if (!prvMicAdcIsDmaFlagSet(pAdc))
    {
        return;
    }

    prvMicAdcClearDmaFlag(pAdc);
    s_dmaBlockCount++;

    if (s_pfnDmaBlock != NULL)
    {
        s_pfnDmaBlock(s_dmaBuffer, TIREMO_MIC_BSP_DMA_BLOCK_SIZE);
    }

    if (s_captureActive)
    {
        (void)prvMicArmDma();
    }
}

uint32_t TIREMO_MIC_BSP_GetDmaBlockCount(void)
{
    return s_dmaBlockCount;
}

/******************************************************************************
 * Private Functions
 *****************************************************************************/

static uint16_t prvMicCalcGra(uint32_t sampleRateHz)
{
    uint32_t timerClk = SystemCoreClock / 2U;
    uint32_t period = timerClk / sampleRateHz;

    if (period > 0U)
    {
        period--;
    }
    if (period > 0xFFFFU)
    {
        period = 0xFFFFU;
    }

    return (uint16_t)period;
}

static bool prvMicDmaBackendInit(void)
{
    HAL_ERR_e err;
    DMA_ID_e id;

    if (s_dmaBackendReady)
    {
        return true;
    }

    err = HPL_DMA_Init(DMA_PERI_ADC, (uint8_t)TIREMO_BOARD_MIC_ADC_ID);
    if (err != HAL_ERR_OK)
    {
        return false;
    }

    err = HPL_DMA_GetAvailChannel(&id);
    if (err != HAL_ERR_OK)
    {
        return false;
    }

    s_dmaCh = id;
    s_dmaBackendReady = true;
    return true;
}

static bool prvMicArmDma(void)
{
    HAL_ERR_e err;
    TIREMO_MIC_AdcReg_t *pAdc;
    DMA_CFG_t dmaCfg =
    {
        .ePeri = DMA_PERI_ADC,
        .ePeriId = (DMA_PERI_ID_e)TIREMO_BOARD_MIC_ADC_ID,
        .un32PeriNum = 0U,
        .eSize = DMA_BUS_SIZE_HALFWORD,
        .eDir = DMA_DIR_PERI_TO_MEM
    };

    if (!prvMicDmaBackendInit())
    {
        return false;
    }

    pAdc = prvMicAdcReg();

    err = (HAL_ERR_e)HPL_DMA_GetPeriSelectNumber(
        DMA_PERI_ADC,
        (DMA_PERI_ID_e)TIREMO_BOARD_MIC_ADC_ID,
        DMA_PERI_COM_RX,
        &dmaCfg.un32PeriNum);
    if (err != HAL_ERR_OK)
    {
        return false;
    }

    (void)HPL_DMA_Stop(s_dmaCh);

    err = (HAL_ERR_e)HPL_DMA_SetConfig(s_dmaCh, &dmaCfg);
    if (err != HAL_ERR_OK)
    {
        return false;
    }

    prvMicAdcSetDmaEn(pAdc, true);
    prvMicAdcSetDmaIe(pAdc, true);

    if (prvMicAdcIsDmaFlagSet(pAdc))
    {
        prvMicAdcClearDmaFlag(pAdc);
    }

    err = (HAL_ERR_e)HPL_DMA_Start(s_dmaCh,
                                   prvMicAdcGetDdrAddr(pAdc),
                                   (uint32_t)s_dmaBuffer,
                                   TIREMO_MIC_BSP_DMA_BLOCK_SIZE);
    return (err == HAL_ERR_OK);
}

static void prvMicDmaTeardown(void)
{
    TIREMO_MIC_AdcReg_t *pAdc;

    if (!s_dmaBackendReady)
    {
        return;
    }

    pAdc = prvMicAdcReg();
    prvMicAdcSetDmaEn(pAdc, false);
    prvMicAdcSetDmaIe(pAdc, false);
    (void)HPL_DMA_Stop(s_dmaCh);
}
