/******************************************************************************
 * @file    tiremo_telemetry.h
 * @brief   Data-plane MQTT: shadow-gated LIS2DE12 / SHT40 samples.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_TELEMETRY_H
#define TIREMO_TELEMETRY_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Build Tiremo data-plane topics (legacy path, no principal).
 *
 *   pub/{productIdentifier}/{deviceIdentifier}/telemetry
 *
 * @param[in] pProductId  TIREMO_AWS_PRODUCT_IDENTIFIER.
 * @param[in] pDeviceId   12-hex station MAC from tutorial 01.
 */
bool TIREMO_Telemetry_Init(const char *pProductId, const char *pDeviceId);

/**
 * @brief Sample enabled sensors and maybe publish telemetry.
 *
 * Shadow flags `ACC_en` / `GYRO_en` / `TEMP_en` / `HUM_en` (`true`/`false`
 * or `0`/`1`) select which keys go in the payload. GYRO_en is accepted
 * and reported so delta clears; Cortex has no gyro, so no gyro fields
 * are published. Interval is `reporting.intervalS` when present, else 60 s.
 *
 * @param[in] nowMs  TIREMO_SysTick_GetMs().
 */
void TIREMO_Telemetry_Poll(uint32_t nowMs);

/** @brief Publish on the next Poll (after a live shadow flag change). */
void TIREMO_Telemetry_Kick(void);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_TELEMETRY_H */
