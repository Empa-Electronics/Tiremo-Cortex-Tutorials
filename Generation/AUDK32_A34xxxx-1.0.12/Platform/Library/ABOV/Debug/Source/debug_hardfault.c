/**
 *******************************************************************************
 * @file        debug_hardfault.c
 * @author      ABOV R&D Division
 * @brief       Debugging hardfault, prints register values
 *
 * Copyright 2022 ABOV Semiconductor Co.,Ltd. All rights reserved.
 *
 * This file is licensed under terms that are found in the LICENSE file
 * located at Document directory.
 * If this file is delivered or shared without applicable license terms,
 * the terms of the BSD-3-Clause license shall be applied.
 * Reference: https://opensource.org/licenses/BSD-3-Clause
 ******************************************************************************/

#include <stdint.h>
#include "debug.h"

#if (CONFIG_DEBUG == 1)

extern void serial_hardfault_abort(void);
extern void serial_hardfault_putc(char c);

#define REG_COUNT       8
#define REG_NAME_LEN    5

static const char reg_name[REG_COUNT][REG_NAME_LEN] =
{
    "R0  :",
    "R1  :",
    "R2  :",
    "R3  :",
    "R12 :",
    "LR  :",
    "PC  :",
    "xPSR:",
};

static void hardfault_put32(unsigned long hex)
{
    uint8_t start = 28;
    uint8_t i;
    char val;

    serial_hardfault_putc('0');
    serial_hardfault_putc('x');

    for (i = 0; i < 8; i++)
    {
        val = ((hex >> start) & 0x0F);
        if (val <= 9)
        {
            serial_hardfault_putc(val + 48);    /* ascii '0' */
        }
        else
        {
            serial_hardfault_putc(val + 87);    /* ascii 'a' - 10 */
        }
        start -= 4;
    }
}

void HardFault_HandlerC(unsigned long *hardfault_args)
{
    int i, j;

    serial_hardfault_abort();
    serial_hardfault_putc('H');
    serial_hardfault_putc('a');
    serial_hardfault_putc('r');
    serial_hardfault_putc('d');
    serial_hardfault_putc('F');
    serial_hardfault_putc('a');
    serial_hardfault_putc('u');
    serial_hardfault_putc('l');
    serial_hardfault_putc('t');
    serial_hardfault_putc(0x0D);
    serial_hardfault_putc(0x0A);

    for (i = 0; i < REG_COUNT; i++)
    {
        serial_hardfault_putc(' ');
        for (j = 0; j < REG_NAME_LEN; j++)
        {
            serial_hardfault_putc(reg_name[i][j]);
        }
        hardfault_put32(hardfault_args[i]);
        serial_hardfault_putc(0x0D);
        serial_hardfault_putc(0x0A);
    }

    /* halt the system, or reset */
    while (1)
    {
    }
}
#endif /* CONFIG_DEBUG */

