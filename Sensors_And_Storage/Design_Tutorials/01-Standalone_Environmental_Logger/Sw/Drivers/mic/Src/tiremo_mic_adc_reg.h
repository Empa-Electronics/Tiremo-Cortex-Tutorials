/******************************************************************************
 * @file    tiremo_mic_adc_reg.h
 * @brief   Private CMSIS ADC0 register helpers for the mic BSP.
 *
 * hal_adc.h's public API does not expose the DMA-enable bit, the DMA-done
 * status flag, or MR.ADEN/MR.ARST outside of HAL_ADC_SetConfig() (which
 * would fight the vendor-generated burst-mode config already applied by
 * PRV_ADC_Init()/user_adc.c). These inline helpers access the CMSIS ADC0
 * register block directly for exactly those bits, mirroring what a vendor
 * hal_adc_prv.h would provide.
 *
 * Hardcoded to ADC0 to match TIREMO_BOARD_MIC_ADC_ID (board_config.h); the
 * ADC_ID_e enum used elsewhere is a plain C enum, not preprocessor-visible,
 * so it cannot drive a compile-time peripheral selection here.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_MIC_ADC_REG_H
#define TIREMO_MIC_ADC_REG_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "abov_config.h"

#include <stdbool.h>
#include <stdint.h>

/******************************************************************************
 * Private Types
 *****************************************************************************/

typedef ADC0_Type TIREMO_MIC_AdcReg_t;

/******************************************************************************
 * Private Functions
 *****************************************************************************/

static inline TIREMO_MIC_AdcReg_t *prvMicAdcReg(void)
{
    return ADC0;
}

static inline IRQn_Type prvMicAdcIrq(void)
{
    return ADC0_IRQn;
}

static inline void prvMicAdcSetEnable(TIREMO_MIC_AdcReg_t *pAdc, bool enable)
{
    if (enable)
    {
        pAdc->MR |= ADC_MR_ADEN_Msk;
    }
    else
    {
        pAdc->MR &= ~ADC_MR_ADEN_Msk;
    }
}

static inline void prvMicAdcSetAutoRestart(TIREMO_MIC_AdcReg_t *pAdc, bool enable)
{
    if (enable)
    {
        pAdc->MR |= ADC_MR_ARST_Msk;
    }
    else
    {
        pAdc->MR &= ~ADC_MR_ARST_Msk;
    }
}

static inline void prvMicAdcSetDmaEn(TIREMO_MIC_AdcReg_t *pAdc, bool enable)
{
    if (enable)
    {
        pAdc->MR |= ADC_MR_DMAEN_Msk;
    }
    else
    {
        pAdc->MR &= ~ADC_MR_DMAEN_Msk;
    }
}

static inline void prvMicAdcSetDmaIe(TIREMO_MIC_AdcReg_t *pAdc, bool enable)
{
    if (enable)
    {
        pAdc->IER |= ADC_IER_DMAIE_Msk;
    }
    else
    {
        pAdc->IER &= ~ADC_IER_DMAIE_Msk;
    }
}

static inline uint32_t prvMicAdcGetDdrAddr(const TIREMO_MIC_AdcReg_t *pAdc)
{
    return (uint32_t)&pAdc->DDR;
}

static inline bool prvMicAdcIsDmaFlagSet(const TIREMO_MIC_AdcReg_t *pAdc)
{
    return ((pAdc->SR & ADC_SR_DMAF_Msk) != 0U);
}

static inline void prvMicAdcClearDmaFlag(TIREMO_MIC_AdcReg_t *pAdc)
{
    pAdc->SR = ADC_SR_DMAF_Msk;
}

static inline uint32_t prvMicAdcGetSr(const TIREMO_MIC_AdcReg_t *pAdc)
{
    return pAdc->SR;
}

static inline bool prvMicAdcIsSeqFlag(const TIREMO_MIC_AdcReg_t *pAdc)
{
    return ((pAdc->SR & ADC_SR_EOSIF_Msk) != 0U);
}

static inline void prvMicAdcAckSeqFlag(TIREMO_MIC_AdcReg_t *pAdc)
{
    pAdc->SR = ADC_SR_EOSIF_Msk;
}

static inline bool prvMicAdcIsSingleFlag(const TIREMO_MIC_AdcReg_t *pAdc)
{
    return ((pAdc->SR & ADC_SR_EOCIF_Msk) != 0U);
}

static inline void prvMicAdcAckSingleFlag(TIREMO_MIC_AdcReg_t *pAdc, bool data)
{
    pAdc->SR = ((uint32_t)data << ADC_SR_EOCIF_Pos);
}

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_MIC_ADC_REG_H */
