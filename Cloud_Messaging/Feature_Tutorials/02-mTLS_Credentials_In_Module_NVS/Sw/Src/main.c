/******************************************************************************
 * @file    main.c
 * @brief   mTLS Credentials in Module NVS -- UART0 debug + UART2 ESP32.
 *
 * Unchanged from AT Command Transport. PRV_UART_Init() brings up
 * UART_ID_0 (debug console), UART_ID_1, and UART_ID_2 (ESP32 AT).
 * TIREMO_Debug_Init() re-arms UART0; TIREMO_ESP32_Init() re-arms UART2.
 * AT+SYSMFG needs no WiFi and no new vendor peripheral bring-up -- it's
 * pure local NVS storage on the module, reached over the same AT
 * transport.
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
