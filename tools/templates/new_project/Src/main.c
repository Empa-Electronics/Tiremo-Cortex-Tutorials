/******************************************************************************
 * @file    main.c
 * @brief   Minimal entry point -- port init, clock init, user code.
 *
 * Trimmed from the vendor-generated TmplUserApp/main.c: only port init,
 * clock init, and the user code call. Add PRV_UART_Init() or other
 * vendor peripheral bring-up calls here if your application needs them
 * (see Getting_Started/03-Debug_UART_Logging in the tutorials repo for
 * an example of when and why a peripheral needs one).
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
