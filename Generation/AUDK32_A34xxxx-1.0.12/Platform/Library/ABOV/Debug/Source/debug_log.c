/**
 *******************************************************************************
 * @file        debug_log.c
 * @author      ABOV R&D Division
 * @brief       Debugging with log (using printf)
 *
 * Copyright 2022 ABOV Semiconductor Co.,Ltd. All rights reserved.
 *
 * This file is licensed under terms that are found in the LICENSE file
 * located at Document directory.
 * If this file is delivered or shared without applicable license terms,
 * the terms of the BSD-3-Clause license shall be applied.
 * Reference: https://opensource.org/licenses/BSD-3-Clause
 ******************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

#include "debug.h"
#include "debug_log.h"

#if (CONFIG_DEBUG == 1)

#define LOG_BUFFER_SIZE 128

static char debug_log_buf[LOG_BUFFER_SIZE];

int debug_log(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(debug_log_buf, LOG_BUFFER_SIZE, fmt, ap);
    va_end(ap);

    printf("%s", debug_log_buf);

    return 0;
}


void debug_log_hex(void *data, uint16_t len)
{
    uint16_t i;
    uint16_t offset = 0;

    for (i = 0; i < len; i++)
    {
        if (offset >= (LOG_BUFFER_SIZE - 8))
        {
            offset += sprintf(debug_log_buf + offset, "...");
            break;
        }
        offset += sprintf(debug_log_buf + offset, "%02X ", ((uint8_t *)data)[i]);
    }

    sprintf(debug_log_buf + offset, "\n");
    printf("%s", debug_log_buf);
}

#endif /* CONFIG_DEBUG */

