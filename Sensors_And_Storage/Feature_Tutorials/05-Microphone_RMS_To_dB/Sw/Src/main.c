/******************************************************************************
 * @file    main.c
 * @brief   Microphone RMS to dB -- minimal entry point.
 *
 * PRV_ADC_Init() brings up ADC_ID_0 in burst mode, Timer1-triggered on
 * channel 0 (PA0). PRV_Timer1_Init() programs Timer1 as the ADC trigger
 * source. Pin mux for analog input is applied at runtime by the mic BSP.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "abov_config.h"
#include "abov_module_config.h"

extern void PRV_SCU_CLK_Init(void);
extern void PRV_UART_Init(void);
extern void PRV_ADC_Init(void);
extern void PRV_Timer1_Init(void);
extern void PRV_USER_Code(void);

int main(void)
{
    PRV_PORT_Init();
    PRV_SCU_CLK_Init();
    PRV_UART_Init();
    PRV_ADC_Init();
    PRV_Timer1_Init();
    PRV_USER_Code();
    return 0;
}
