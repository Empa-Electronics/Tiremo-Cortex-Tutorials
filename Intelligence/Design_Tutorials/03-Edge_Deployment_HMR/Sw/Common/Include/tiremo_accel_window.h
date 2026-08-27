/******************************************************************************
 * @file    tiremo_accel_window.h
 * @brief   Tumbling 8-sample x 3-axis accelerometer feature buffer.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_ACCEL_WINDOW_H
#define TIREMO_ACCEL_WINDOW_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../Board/board_config.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    float features[TIREMO_ACCEL_FEATURE_COUNT];
    uint8_t count;
} TIREMO_AccelWindow_t;

void TIREMO_AccelWindow_Reset(TIREMO_AccelWindow_t *pWindow);

/**
 * @brief Append one xyz sample as milli-g floats (no /1000).
 *
 * Fill order: features[i*3+0]=x, +1=y, +2=z for i = 0 .. 7.
 * Ignored when the window is already full.
 */
bool TIREMO_AccelWindow_Push(TIREMO_AccelWindow_t *pWindow,
                             float xMg,
                             float yMg,
                             float zMg);

bool TIREMO_AccelWindow_IsFull(const TIREMO_AccelWindow_t *pWindow);

const float *TIREMO_AccelWindow_Data(const TIREMO_AccelWindow_t *pWindow);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_ACCEL_WINDOW_H */
