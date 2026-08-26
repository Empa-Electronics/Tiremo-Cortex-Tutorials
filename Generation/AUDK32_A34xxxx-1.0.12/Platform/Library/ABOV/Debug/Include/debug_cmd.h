/**
 *******************************************************************************
 * @file        debug_cmd.h
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

/**
 * @addtogroup  Framework
 * @{
 * @defgroup    Debug Debug
 * @{
 * @defgroup    Command Command
 * @{
 * @brief       Command is composed of API related Command-line input.
 */

#ifndef _DEBUG_CMD_H_
#define _DEBUG_CMD_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/**
 * @brief Command Status
 */
enum debug_cmd_status
{
    DEBUG_CMD_SUCCESS,      /**< command function succeeded */
    DEBUG_CMD_INVALID,      /**< command parameters are invalid, help displayed */
    DEBUG_CMD_FAILED,       /**< command function failed */
};

/**
 * @brief Command Function type
 */
typedef enum debug_cmd_status (*debug_cmd_func_t)(int32_t argc, char *argv[]);

/**
 * @brief command, function, description, argument information
 */
struct debug_cmd
{
    char *              name;   /**< command group name */
    char *              cmd;    /**< command name */
    debug_cmd_func_t    func;   /**< function to be executed */
    const char *        desc;   /**< description of the command */
    const char *        usage;  /**< description of the command argument */
};

/**
 * macro to count number of command list
 */
#define DEBUG_CMD_LIST_COUNT(list) (sizeof(list) / sizeof(struct debug_cmd))


#if (CONFIG_DEBUG == 1)

/**
 *******************************************************************************
 * @brief       Initialize command line input processing.
 * @param[in]   *list : list of commands.
 * @param[in]   *list_size : list_size size of the list.
 * @return      void : None
 ******************************************************************************/
void debug_cmd_init(const struct debug_cmd *list, uint8_t list_size);

/**
 *******************************************************************************
 * @brief       Deinitialize command line input processing.
 * @return      void : None
 ******************************************************************************/
void debug_cmd_deinit(void);

/**
 *******************************************************************************
 * @brief       Processs command line input. Should be called after an (UART)
 *              interrupt to check if there is a carriage return entered
 *              from the user, and process the string if so.
 * @return      void : None
 ******************************************************************************/
void debug_cmd_process(void);


/**
 *******************************************************************************
 * @brief       Execute command line input.
 * @param[in]   *run : string command.
 * @return      void : None
 ******************************************************************************/
void debug_cmd_execute(char *run);

/**
 *******************************************************************************
 * @brief       Re-init command line input.
 * @return      void : None
 ******************************************************************************/
void debug_cmd_reinit(void);

#else

#define debug_cmd_init(list, list_size)  do { \
        (void)list;                           \
        (void)list_size;                      \
} while (0)
#define debug_cmd_deinit()
#define debug_cmd_process()
#define debug_cmd_execute(run)           do { \
        (void)run;                            \
} while (0)
#define debug_cmd_reinit()

#endif /* CONFIG_DEBUG */

/**
 *******************************************************************************
 * @brief       Convert hex-string or integer-string to integer value.
 * @param[in]   *pnString : string to int convert.
 * @return      uin32_t : Integer from string
 ******************************************************************************/
uint32_t debug_cmd_string2int(char *pn8String);

#ifdef __cplusplus
}
#endif

#endif /* _DEBUG_CMD_H_ */

/** @} */
/** @} */
/** @} */
