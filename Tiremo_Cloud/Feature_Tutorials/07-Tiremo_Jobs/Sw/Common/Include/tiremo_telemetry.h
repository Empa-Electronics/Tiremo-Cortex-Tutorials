/******************************************************************************
 * @file    tiremo_telemetry.h
 * @brief   Data-plane MQTT: periodic telemetry + one hysteresis alarm.
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
 *   pub/{productIdentifier}/{deviceIdentifier}/alarm
 *
 * Same strings as
 * https://docs.tiremo.ai/device-provisioning/third-phase
 * ("Without a principal, devices may use the legacy path").
 * Call after the shadow boot GET so intervalS / enabled are already
 * applied. Does not subscribe — these topics are publish-only.
 *
 * @param[in] pProductId  TIREMO_AWS_PRODUCT_IDENTIFIER.
 * @param[in] pDeviceId   12-hex station MAC from tutorial 01.
 */
bool TIREMO_Telemetry_Init(const char *pProductId, const char *pDeviceId);

/**
 * @brief Sample, evaluate the temperature alarm, maybe publish telemetry.
 *
 * A short USER hold is a *demo sensor*: 40 C while pressed, 22 C when
 * released. That is enough to cross 35 C (raise) and drop below 30 C
 * (clear) without I2C. The 10 s factory-reset hold is handled by the
 * caller; this function only needs the current pressed level.
 *
 * Telemetry publishes on `reporting.intervalS` while
 * `reporting.enabled` is true. Alarms always evaluate so a tap still
 * produces a raise/clear pair.
 *
 * @param[in] nowMs       TIREMO_SysTick_GetMs().
 * @param[in] userHeld    Raw USER pressed (no debounce).
 */
void TIREMO_Telemetry_Poll(uint32_t nowMs, bool userHeld);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_TELEMETRY_H */
