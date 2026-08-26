/******************************************************************************
 * @file    main.c
 * @brief   Connect/retry -- UART0 debug + UART2 ESP32 + DFMC persist.
 *
 * Same bring-up as tutorial 01. PRV_UART_Init() clocks UART0 and UART2;
 * TIREMO_Debug_Init() / TIREMO_ESP32_Init() steal the IRQs.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "abov_config.h"
#include "abov_module_config.h"

extern void PRV_SCU_CLK_Init(void);
extern void PRV_UART_Init(void);
extern void PRV_USER_Code(void);

int main(void)
{
    PRV_PORT_Init();
    PRV_SCU_CLK_Init();
    PRV_UART_Init();
    PRV_USER_Code();

    return 0;
}
