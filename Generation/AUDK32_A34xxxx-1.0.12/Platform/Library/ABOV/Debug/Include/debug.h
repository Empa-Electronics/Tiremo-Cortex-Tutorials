/**
 *******************************************************************************
 * @file        debug.h
 * @author      ABOV R&D Division
 * @brief       Common debug APIs for S/W
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
 * @brief       Framework is composed of two categories roughly. \n
 *              External S/W framework like CMSIS and internal (ABOV) framework like a debug APIs 
 *              which are used with any S/W module.
 * @defgroup    Debug Debug
 * @{
 * @brief       Debug is software debugging tool based on UART serial communication.
 * @defgroup    Common Common
 * @{
 * @brief       Common is composed of public debug APIs.
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "abov_config.h"

/**
 *******************************************************************************
 * @brief       Initialize Debug.
 * @return      void : None
 ******************************************************************************/
void Debug_Init(void);

/**
 *******************************************************************************
 * @brief       Re-initialize Debug. (Re-configure system clock source)
 * @param[in]   baudrate : Baudrate
 * @return      void : None
 ******************************************************************************/
void Debug_Reinit(uint16_t baudrate);

/**
 *******************************************************************************
 * @brief       Assertion.
 * @param[in]   *file : .
 * @param[in]   line : .
 * @return      void : None
 ******************************************************************************/
void Assert(char *file, uint32_t line);

#ifdef __cplusplus
}
#endif

#endif /* _DEBUG_H_ */

/** @} */
/** @} */
/** @} */
