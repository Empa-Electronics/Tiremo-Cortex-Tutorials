/**
 *******************************************************************************
 * @file        system_a34xxxx.c
 * @author      ABOV R&D Division
 * @brief       CMSIS Cortex-M4F Device Peripheral Access Layer for A34xxxx
 *
 * Copyright 2022 ABOV Semiconductor Co.,Ltd. All rights reserved.
 *
 * This file is licensed under terms that are found in the LICENSE file
 * located at Document directory.
 * If this file is delivered or shared without applicable license terms,
 * the terms of the BSD-3-Clause license shall be applied.
 * Reference: https://opensource.org/licenses/BSD-3-Clause
 ******************************************************************************/

#include <stdint.h>
#include "abov_config.h"

extern const VECTOR_TABLE_Type __VECTOR_TABLE[240];

void SystemInit(void)
{
    __disable_irq();

#if !defined (EXTRN_FPA_CFMC) && !defined (EXTRN_FPA_DFMC)
    SCB->VTOR = (uint32_t)(&__VECTOR_TABLE[0]);
#endif

#if defined (EXTRN_FPU_USED)
    SCB->CPACR |= ((3U << 10U*2U) |           /* enable CP10 Full Access */
                   (3U << 11U*2U)  );         /* enable CP11 Full Access */
#endif

    PRV_CHIPSET_Init();

    __enable_irq();
}

void SystemDelayMS(uint32_t ms)
{
    uint32_t i;
    uint32_t Delay = (SystemDelayMSCount) * ms;
    for (i=0; i < Delay; i++)
    {
        __NOP();
    }
}

void SystemDelayUS(uint32_t us)
{
    uint32_t i;
    uint32_t Delay = (SystemDelayUSCount) * us;
    for (i=0; i < Delay; i++)
    {
        __NOP();
    }
}
