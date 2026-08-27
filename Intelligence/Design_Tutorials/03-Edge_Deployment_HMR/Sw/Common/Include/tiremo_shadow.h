/******************************************************************************
 * @file    tiremo_shadow.h
 * @brief   Device Shadow get / apply / report plus live update/delta.
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
 * @brief Boot GET / apply / report, then subscribe update/delta.
 *
 * Same one-shot exchange as tutorial 04. After the second get confirms
 * no leftover delta, subscribes `$aws/things/<thing>/shadow/update/delta`
 * so later desired edits arrive without another GET. Each delta is
 * merged into the last full desired object and echoed as reported.
 *
 * @param[in] pThingName  AWS IoT Thing name used in the shadow topics.
 */
bool TIREMO_Shadow_GetApplyReport(const char *pThingName);

/**
 * @brief Poll one inbound MQTT message (short timeout).
 *
 * On `update/delta`, overlay known fields, merge the slice into the
 * last full desired object, and echo that as reported. Other topics
 * (get/accepted, update/accepted) are ignored.
 */
void TIREMO_Shadow_Poll(void);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_SHADOW_H */
