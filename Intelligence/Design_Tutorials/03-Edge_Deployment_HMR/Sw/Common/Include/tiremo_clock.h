/******************************************************************************
 * @file    tiremo_clock.h
 * @brief   Tiremo software wall-clock API.
 *
 * The A34G43x has no RTC peripheral, so there is no battery-backed
 * hardware clock available. This keeps a RAM-resident Unix epoch, set once
 * from an NTP sync and advanced using the existing SysTick millisecond
 * counter. It is genuinely synced wall-clock time — it is just lost on
 * reset/power-loss instead of persisting, which is the closest this
 * hardware can get to a real clock.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_CLOCK_H
#define TIREMO_CLOCK_H

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
 * @brief Set the current time, e.g. right after parsing an NTP response.
 *
 * @param[in] epochSeconds  Current time as a Unix epoch (UTC seconds).
 *
 * @warning Call TIREMO_SysTick_Init() before this function; elapsed time is
 *          measured against TIREMO_SysTick_GetMs().
 */
void TIREMO_Clock_SetEpoch(uint32_t epochSeconds);

/**
 * @brief Get the current time.
 *
 * @param[out] pEpochSeconds  Current time as a Unix epoch (UTC seconds).
 *
 * @return true when the clock has been synced at least once.
 */
bool TIREMO_Clock_GetEpoch(uint32_t *pEpochSeconds);

/**
 * @brief Check whether the clock has been synced at least once.
 */
bool TIREMO_Clock_IsSynced(void);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_CLOCK_H */
