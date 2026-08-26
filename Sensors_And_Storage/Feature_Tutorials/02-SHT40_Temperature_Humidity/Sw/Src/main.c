/******************************************************************************
 * @file    main.c
 * @brief   SHT40 Temperature / Humidity -- minimal entry point.
 *
 * Same init sequence as 01-I2C_Bus_Bringup: the SHT40 still lives on
 * I2C_ID_2, so PRV_I2C_Init() is required here too. No new vendor call
 * beyond that tutorial -- this one is a protocol driver on a bus that's
 * already up.
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
