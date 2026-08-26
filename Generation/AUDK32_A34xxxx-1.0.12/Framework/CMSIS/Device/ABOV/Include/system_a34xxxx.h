/**
 *******************************************************************************
 * @file        system_a34xxxx.h
 * @author      ABOV R&D Division
 * @brief       CMSIS Cortex-M4F Device Peripheral Access Layer for A34xxxx
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
 * @addtogroup device
 * @{
 * @defgroup device_system System
 * @{
 * @brief System configuration
 */

#ifndef _SYSTEM_A34XXXX_H_
#define _SYSTEM_A34XXXX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef void(*VECTOR_TABLE_Type)(void);

extern uint32_t SystemCoreClock;        /**< System Clock Frequency */
extern uint32_t SystemPeriClock;        /**< System Peripheral clock Frequency */
extern uint32_t SystemDelayMSCount;     /**< System Delay Milli-second Count */
extern uint32_t SystemDelayUSCount;     /**< System Delay Micro-second Count */

/**
 *******************************************************************************
 * @brief       Setup the microcontroller system.
 *              Initialize the System.
 *              This function is normally called from the startup before
 *              program jumps to the application's main function.
 * @return      none
 ******************************************************************************/
void SystemInit(void);

/**
 *******************************************************************************
 * @brief       Delay using loop
 *              This function is based on _NOP instruction and does not provide
 *              accurate delay.
 * @param[in]   ms delay in millisecond
 * @return      none
 ******************************************************************************/
extern void SystemDelayMS(uint32_t ms);


/**
 *******************************************************************************
 * @brief       Delay using loop
 *              This function is based on _NOP instruction and does not provide
 *              accurate delay.
 * @param[in]   ms delay in microsecond
 * @return      none
 ******************************************************************************/
extern void SystemDelayUS(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif /* _SYSTEM_A34XXXX_H_ */

/** @} */
/** @} */

