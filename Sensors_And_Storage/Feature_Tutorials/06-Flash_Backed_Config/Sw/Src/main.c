/******************************************************************************
 * @file    main.c
 * @brief   Flash-Backed Config -- minimal entry point.
 *
 * No extra vendor init: HAL_FMC_Init() is called from TIREMO_HAL_FLASH_Init().
 * CONFIG_HAL_FMC is already 1 in this SDK's abov_module_config.h.
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
