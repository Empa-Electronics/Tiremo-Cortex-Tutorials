/******************************************************************************
 * @file    main.c
 * @brief   Debug UART Logging -- minimal entry point.
 *
 * Adds PRV_UART_Init() to Tutorial 01/02's port init + clock init +
 * PRV_USER_Code() shape. Unlike SysTick (a core Cortex-M4 peripheral
 * needing no vendor init call at all), the debug console sits on top of
 * a real ABOV UART peripheral that the vendor template brings up here,
 * before application code runs -- so this is the tutorial where main.c
 * finally changes from the first two.
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
    /* PRV_PORT_Init() is a vendor-generated static inline in
     * startup_a34g43x.h (pulled in transitively via abov_config.h), so it
     * needs no extern declaration here. */
    PRV_PORT_Init();

    PRV_SCU_CLK_Init();

    /* Vendor-generated: brings up all three UART peripherals (115200 8N1)
     * with no-op placeholder IRQ handlers. TIREMO_Debug_Init() re-arms
     * UART_ID_0's IRQ with the real Tiremo handler afterward -- see
     * prv_user_code.c. */
    PRV_UART_Init();

    PRV_USER_Code();

    return 0;
}
