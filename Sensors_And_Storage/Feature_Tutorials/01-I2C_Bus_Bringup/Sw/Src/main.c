/******************************************************************************
 * @file    main.c
 * @brief   I2C Bus Bring-up -- minimal entry point.
 *
 * Adds PRV_I2C_Init() to the port init + clock init + UART init +
 * PRV_USER_Code() shape established in Getting_Started. PRV_I2C_Init()
 * is vendor-generated, unmodified: it brings up I2C_ID_2 (master,
 * 100 kHz, SDA hold enabled) -- exactly TIREMO_HAL_I2C_BUS_2, the bus
 * this tutorial probes. SCL/SDA pin muxing is already applied by the
 * generated pin config, so no runtime GPIO call is needed for it.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "abov_config.h"
#include "abov_module_config.h"

extern void PRV_SCU_CLK_Init(void);
extern void PRV_UART_Init(void);
extern void PRV_I2C_Init(void);
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

    /* Vendor-generated: brings up I2C_ID_2 in master mode at 100 kHz. */
    PRV_I2C_Init();

    PRV_USER_Code();

    return 0;
}
