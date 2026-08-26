/**
 *******************************************************************************
 * @file        debug_cmd.c
 * @author      ABOV R&D Division
 * @brief       Debugging with command line input
 *
 * Copyright 2022 ABOV Semiconductor Co.,Ltd. All rights reserved.
 *
 * This file is licensed under terms that are found in the LICENSE file
 * located at Document directory.
 * If this file is delivered or shared without applicable license terms,
 * the terms of the BSD-3-Clause license shall be applied.
 * Reference: https://opensource.org/licenses/BSD-3-Clause
 ******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "debug.h"
#include "debug_log.h"
#include "debug_cmd.h"
#include "debug_serial.h"

#if (CONFIG_DEBUG == 1)

#define MAX_CMD_NUM_GROUP 20
#define MAX_CMD_NUM_ARGS 30
#define MAX_CMD_LINE_LEN 96

static const struct debug_cmd *cmd_list[MAX_CMD_NUM_GROUP];
static uint8_t cmd_size[MAX_CMD_NUM_GROUP];
static char cmd_buf[MAX_CMD_LINE_LEN + 1];
static uint8_t cmd_buf_index;
static volatile uint8_t cmd_ready;
static uint8_t cmd_group_index = 0;
static uint8_t cmd_current_index = 0;
static bool bBdrChange = false;

static void convertToLower(char *str, char *lower)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        if(str[i] >= 'A' && str[i] <= 'Z')
        {
            lower[i] = str[i] + 32;
        }
        else
        {
            lower[i] = str[i];
        }
    }
}

static void tokenize(char *cmd_line, int *argc, char *argv[])
{
    int found_arg = 1;
    int argn = 0;

    /* parse input string as argc and argv */
    while (*cmd_line)
    {
        if (*cmd_line == '\n')
        {
            *cmd_line = '\0';
            break;
        }
        if ((*cmd_line == ' '))
        {
            found_arg = 1;
            *cmd_line = '\0';
        }
        else if (found_arg)
        {
            argv[argn++] = cmd_line;
            found_arg = 0;
        }

        cmd_line++;

        if (argn >= MAX_CMD_NUM_ARGS)
        {
            break;
        }
    }

    *argc = argn;
}

static void cmd_help(int index, int usage)
{
    LOG("%-5s: %s\n", cmd_list[cmd_current_index][index].cmd, cmd_list[cmd_current_index][index].desc);
    if (usage && cmd_list[cmd_current_index][index].usage)
    {
        LOG("%s\n", cmd_list[cmd_current_index][index].usage);
    }
}

static void show_cmd_list(void)
{
    LOG(" lm\t: show module list\n");
    LOG(" cm\t: change to specific module\n");
    LOG("    usage : cm [module] (one of module list by lm) \n");
    LOG(" help\t: show specific module command list\n");
}

static int execute(char *param)
{
    int argc = 0;
    char *argv[MAX_CMD_NUM_ARGS] = { NULL, };
    int ret = DEBUG_CMD_SUCCESS;
    int i;
    char lower1[16], lower2[16];

    tokenize(param, &argc, argv);

    if (argv[0] == NULL || argc == 0)
    {
        /* check for default handler */
        argc = 1;
        argv[0] = "";

    }
    else if (!strcmp(argv[0], "ls"))
    {
        show_cmd_list();
        goto out;
    }
    else if (!strcmp(argv[0], "cm"))
    {
        if (argc != 2)
        {
            show_cmd_list();
            goto out;
        }

        for(i = 0; i < cmd_group_index; i++)
        {
            if(argc == 2)
            {
                memset(lower1, 0x00, sizeof(lower1));
                memset(lower2, 0x00, sizeof(lower2));
                convertToLower(cmd_list[i][0].name, lower1);
                convertToLower(argv[1], lower2);
                if(!strcmp(lower1, lower2))
                {
                    cmd_current_index = i;
                    LOG("change to %s\n",cmd_list[i][0].name);
                    break;
                }
            }
        }

        if(i >= cmd_group_index)
        {
            LOG("%s is not available\n",argv[1]);
        }

        goto out;
    }
    else if (!strcmp(argv[0], "lm"))
    {
        LOG("\n = Executable module list = \n\n");

        for(i = 0; i < cmd_group_index; i++)
        {
            LOG("   %d : %s\n", i, cmd_list[i][0].name);
        }
        LOG("\n");

        goto out;
    }
    else if (!strcmp(argv[0], "help"))
    {

        /* check for help */
        for (i = 0; i < cmd_size[cmd_current_index]; i++)
        {
            /* specific command */
            if (argc == 2)
            {
                if (!strcmp(cmd_list[cmd_current_index][i].cmd, argv[1]))
                {
                    cmd_help(i, 1);
                    break;
                }
            }
            else
            {
                cmd_help(i, 0);
            }
        }

        goto out;
    }

    /* find the command from the list */
    for (i = 0; i < cmd_size[cmd_current_index]; i++)
    {
        if (!strcmp(argv[0], cmd_list[cmd_current_index][i].cmd))
        {
            ret = cmd_list[cmd_current_index][i].func(argc, argv);
            if (ret == DEBUG_CMD_INVALID)
            {
                cmd_help(i, 1);
            }
            goto out;
        }
    }

    /* end of command list */
    if (strlen(argv[0]) > 0)
    {
        LOG("\"%s\" not a valid command\n", argv[0]);
    }

out:
    /* print a prompt */
    if(bBdrChange == false)
    {
        LOG("<%s> # ",(cmd_list[cmd_current_index][0].name == NULL ? NULL : cmd_list[cmd_current_index][0].name));
    }

    return ret;
}

static void read_cmd_byte_cb(uint8_t ch)
{
    if (ch == '\r' || ch == '\n')
    {
        cmd_ready = 1;
        cmd_buf[cmd_buf_index++] = '\0';
    }
    else
    {
        /* only printable characters recognized for command */
        if (ch >= 0x20 && ch <= 0x7E && cmd_buf_index < (MAX_CMD_LINE_LEN - 1))
        {
            cmd_buf[cmd_buf_index++] = ch;
        }
        /* remove character by backspace or delete */
        if (ch == 0x08 || ch == 0x7F)
        {
            cmd_buf[cmd_buf_index--] = '\0';
        }
    }
    serial_getc(read_cmd_byte_cb);
}

void debug_cmd_init(const struct debug_cmd *list, uint8_t list_size)
{
    cmd_list[cmd_group_index] = list;
    cmd_size[cmd_group_index++] = list_size;

    /* receive a character one by one */
    cmd_buf_index = 0;
    cmd_ready = 0;
    serial_getc(read_cmd_byte_cb);
}

void debug_cmd_reinit(void)
{
    bBdrChange = true;
    serial_getc(read_cmd_byte_cb);
}

void debug_cmd_deinit(void)
{
}

void debug_cmd_process(void)
{
    /* execute the command when signaled it is ready */
    if (cmd_ready)
    {
        bBdrChange = false;
        execute(cmd_buf);
        cmd_buf_index = 0;
        cmd_ready = 0;
    }
}

void debug_cmd_execute(char *run)
{
    execute(run);
}

uint32_t debug_cmd_string2int(char *pn8String)
{
    uint32_t un32Result;

    if ((pn8String[0] == '0') && (pn8String[1] == 'x'))
    {
        un32Result = (uint32_t)strtoll(pn8String, NULL, 0);
    }
    else
    {
        un32Result = atoi(pn8String);
    }

    return un32Result;
}

#else

uint32_t debug_cmd_string2int(char *pn8String)
{
    (void)pn8String;
    return 0;
}

#endif /* CONFIG_DEBUG */

