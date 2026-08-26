/******************************************************************************
 * @file    main.c
 * @brief   Standalone Environmental Logger -- vendor init for UART, I2C,
 *          ADC, and Timer1 (mic burst trigger).
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "abov_config.h"
#include "abov_module_config.h"

extern void PRV_SCU_CLK_Init(void);
extern void PRV_UART_Init(void);
extern void PRV_I2C_Init(void);
extern void PRV_ADC_Init(void);
extern void PRV_Timer1_Init(void);
extern void PRV_USER_Code(void);

int main(void)
{
    PRV_PORT_Init();
    PRV_SCU_CLK_Init();
    PRV_UART_Init();
    PRV_I2C_Init();
    PRV_ADC_Init();
    PRV_Timer1_Init();
    PRV_USER_Code();
    return 0;
}
