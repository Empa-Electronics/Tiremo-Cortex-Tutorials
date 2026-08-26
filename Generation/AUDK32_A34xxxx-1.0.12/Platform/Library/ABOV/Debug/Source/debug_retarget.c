/**
 *******************************************************************************
 * @file        debug_retarget.c
 * @author      ABOV R&D Division
 * @brief       Debugging retarget, forward fgetc/fputc to serial
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
#include <stdint.h>

#include "debug.h"
#include "debug_serial.h"

#if (CONFIG_DEBUG == 1)

#if (__CC_ARM)

#pragma import(__use_no_semihosting_swi)
struct __FILE { int handle; /* Add whatever you need here */ };

FILE __stdout;
FILE __stdin;
FILE __stderr;

int fputc(int c, FILE *f)
{
    (void)f;

    return serial_putc((char)c);
}

int fgetc(FILE *f)
{
    int ch;

    (void)f;

    while (1)
    {
        ch = serial_getc(NULL);
        if (ch != EOF)
        {
            break;
        }
    }

    return ch;
}

int ferror(FILE *f)
{
    (void)f;

    return EOF;
}

void __ttywrch(int c)
{
    serial_putc((char)c);
}

#elif (__ARMCC_VERSION)

FILE __stdout;
FILE __stdin;
FILE __stderr;

int fputc(int c, FILE *f)
{
    (void)f;

    return serial_putc((char)c);
}

int fgetc(FILE *f)
{
    int ch;

    (void)f;

    while (1)
    {
        ch = serial_getc(NULL);
        if (ch != EOF)
        {
            break;
        }
    }

    return ch;
}

int ferror(FILE *f)
{
    (void)f;

    return EOF;
}

void __ttywrch(int c)
{
    serial_putc((char)c);
}

#elif (__GNUC__)

FILE __stdout;
FILE __stdin;
FILE __stderr;

int _write(int file, const char *p_char, int len)
{
    int i;

    (void)file;


    for (i = 0; i < len; i++)
    {
        serial_putc(p_char[i]);
    }

    return len;
}

int _read(int file, char *p_char, int len)
{
    int ch;

    (void)file;
    (void)len;

    while (1)
    {
        ch = serial_getc(NULL);
        if (ch != EOF)
        {
            break;
        }
    }
    *p_char = ch;

    return 1;
}

/* ARM GNU Toolchain version 12 or higher */
#if (__GNUC__ >= 12)

#include <sys/stat.h>

int _close(int file)
{
    (void)file;
    return 1;

}

int _fstat(int file, struct stat *st)
{
    (void)file;
    (void)stat;
    return 1;
}

int _isatty(int file)
{
    (void)file;
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}
#endif

#elif (__ICCARM__)

int __write(int file, const char *p_char, int len)
{
    int i;

    (void)file;

    for (i = 0; i < len; i++)
    {
        serial_putc(p_char[i]);
    }

    return len;
}

int __read(int file, char *p_char, int len)
{
    int ch;

    (void)file;
    (void)len;

    while (1)
    {
        ch = serial_getc(NULL);
        if (ch != EOF)
        {
            break;
        }
    }
    *p_char = ch;

    return 1;
}

#endif

void _sys_exit(int return_code)
{
label:  goto label;  /* endless loop */
}

#endif /* CONFIG_DEBUG */

