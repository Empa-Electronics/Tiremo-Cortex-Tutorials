/******************************************************************************
 * @file    tiremo_shadow.h
 * @brief   One-shot Device Shadow get / apply / report (no live delta).
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_SHADOW_H
#define TIREMO_SHADOW_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>

/**
 * @brief Fetch the classic shadow, apply known fields, echo reported.
 *
 * Subscribes to get/accepted and get/rejected (not update/delta),
 * publishes an empty get, applies `reporting.enabled`,
 * `reporting.intervalS`, and `led.on` locally, then publishes
 * `state.reported` as a full copy of `state.desired` so AWS clears
 * the delta. A second get confirms `state.delta` is absent.
 *
 * @param[in] pThingName  AWS IoT Thing name used in the shadow topics.
 */
bool TIREMO_Shadow_GetApplyReport(const char *pThingName);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_SHADOW_H */
