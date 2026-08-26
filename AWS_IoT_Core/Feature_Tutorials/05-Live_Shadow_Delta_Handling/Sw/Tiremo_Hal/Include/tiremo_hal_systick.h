/******************************************************************************
 * @file    tiremo_hal_systick.h
 * @brief   Tiremo SysTick hardware abstraction API.
 *
 * Provides a vendor-independent one-millisecond system tick source.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_HAL_SYSTICK_H
#define TIREMO_HAL_SYSTICK_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <stdbool.h>

/******************************************************************************
 * Public Types
 *****************************************************************************/

typedef void (*TIREMO_HAL_SysTick_Callback_t)(void);

/******************************************************************************
 * Public Function Prototypes
 *****************************************************************************/

/**
 * @brief Configure the hardware SysTick for one-millisecond interrupts.
 *
 * @param[in] callback  Function invoked from every SysTick interrupt.
 *
 * @return true when the hardware timer was configured successfully.
 *
 * @warning The callback runs in interrupt context and must remain short.
 */
bool TIREMO_HAL_SysTick_Init1Ms(TIREMO_HAL_SysTick_Callback_t callback);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_HAL_SYSTICK_H */
