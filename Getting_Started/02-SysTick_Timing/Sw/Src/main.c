/******************************************************************************
 * @file    main.c
 * @brief   SysTick Timing -- minimal entry point.
 *
 * Identical to Tutorial 01's main.c: port init, clock init, straight into
 * PRV_USER_Code(). SysTick bring-up belongs to this tutorial's own code
 * (prv_user_code.c), not the vendor entry point -- see that file.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "abov_config.h"
#include "abov_module_config.h"

extern void PRV_SCU_CLK_Init(void);
extern void PRV_USER_Code(void);

int main(void)
{
    /* PRV_PORT_Init() is a vendor-generated static inline in
     * startup_a34g43x.h (pulled in transitively via abov_config.h), so it
     * needs no extern declaration here. */
    PRV_PORT_Init();

    PRV_SCU_CLK_Init();

    PRV_USER_Code();

    return 0;
}
