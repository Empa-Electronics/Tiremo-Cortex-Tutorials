/******************************************************************************
 * @file    main.c
 * @brief   Battery Voltage via ADC -- minimal entry point.
 *
 * Adds PRV_ADC_Init() (vendor-generated): ADC_ID_1 in single / software-
 * trigger mode for the VCORE bandgap read, plus ADC_ID_0 which this
 * tutorial does not use (the mic tutorial will).
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "abov_config.h"
#include "abov_module_config.h"

extern void PRV_SCU_CLK_Init(void);
extern void PRV_UART_Init(void);
extern void PRV_ADC_Init(void);
extern void PRV_USER_Code(void);

int main(void)
{
    PRV_PORT_Init();
    PRV_SCU_CLK_Init();
    PRV_UART_Init();
    PRV_ADC_Init();
    PRV_USER_Code();
    return 0;
}
