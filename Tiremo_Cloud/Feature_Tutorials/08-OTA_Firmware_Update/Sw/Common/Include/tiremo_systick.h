/******************************************************************************
 * @file    tiremo_systick.h
 * @brief   Tiremo system tick timing API.
 *
 * Provides millisecond timing services using the Tiremo SysTick HAL.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_SYSTICK_H
#define TIREMO_SYSTICK_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/******************************************************************************
 * Public Function Prototypes
 *****************************************************************************/

/**
 * @brief Initialize the one-millisecond system tick.
 *
 * @return true when the system tick was initialized successfully.
 */
bool TIREMO_SysTick_Init(void);

/**
 * @brief Wait for a blocking millisecond delay.
 *
 * @param[in] delayMs  Delay duration in milliseconds.
 *
 * @note The function returns immediately when SysTick is not initialized.
 * @warning Do not call this function from interrupt context.
 */
void TIREMO_SysTick_DelayMs(uint32_t delayMs);

/**
 * @brief Get the free-running millisecond counter.
 *
 * @return Milliseconds elapsed since TIREMO_SysTick_Init().
 *
 * @note The counter wraps at UINT32_MAX.
 */
uint32_t TIREMO_SysTick_GetMs(void);

/**
 * @brief Run optional application work on every millisecond tick.
 *
 * Override the weak default implementation when an application tick hook is
 * required.
 *
 * @warning This function runs in interrupt context and must remain short.
 */
void TIREMO_SysTick_OnTick1ms(void);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_SYSTICK_H */
