/**
 *******************************************************************************
 * @file        debug_log.h
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

/**
 * @addtogroup  Framework
 * @{
 * @defgroup    Debug Debug
 * @{
 * @defgroup    Log Log
 * @{
 * @brief       Log is composed of API related to serial debug log.
 */

#ifndef _DEBUG_LOG_H_
#define _DEBUG_LOG_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdio.h>

#if (CONFIG_DEBUG == 1)

/**
 *******************************************************************************
 * @brief       print log
 * @param[in]   *fmt : printf style variable arguments.
 * @return      int : 0 (does not return the number of characters)
 ******************************************************************************/
int debug_log(const char *fmt, ...);

/**
 *******************************************************************************
 * @brief       print hex formatted string
 * @param[in]   *data : data to print
 * @param[in]   len : length of the data
 * @return      void : None
 ******************************************************************************/
void debug_log_hex(void *data, uint16_t len);

/**
 * Logging MACRO similar to the printf style
 */
#define LOG             debug_log

/**
 * Logging MACRO to print starting part of the buffer
 */
#define LOG_HEX         debug_log_hex

#else /* no logging */

/**
 * Logging MACRO similar to the printf style
 */
#define LOG(...)

/**
 * Logging MACRO to print starting part of the buffer
 */
#define LOG_HEX(...)

#endif

#ifdef __cplusplus
}
#endif

#endif /* _DEBUG_LOG_H_ */

/** @} */
/** @} */
/** @} */
