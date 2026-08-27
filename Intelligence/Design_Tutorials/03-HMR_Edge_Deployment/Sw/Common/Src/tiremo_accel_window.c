/******************************************************************************
 * @file    tiremo_accel_window.c
 * @brief   Tumbling 8-sample x 3-axis accelerometer feature buffer.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Include/tiremo_accel_window.h"

#include <stddef.h>

void TIREMO_AccelWindow_Reset(TIREMO_AccelWindow_t *pWindow)
{
    uint8_t i;

    if (pWindow == NULL)
    {
        return;
    }

    for (i = 0U; i < TIREMO_ACCEL_FEATURE_COUNT; i++)
    {
        pWindow->features[i] = 0.0f;
    }
    pWindow->count = 0U;
}

bool TIREMO_AccelWindow_Push(TIREMO_AccelWindow_t *pWindow,
                             float xMg,
                             float yMg,
                             float zMg)
{
    uint8_t base;

    if ((pWindow == NULL) || TIREMO_AccelWindow_IsFull(pWindow))
    {
        return false;
    }

    base = (uint8_t)(pWindow->count * TIREMO_ACCEL_NUM_AXES);
    pWindow->features[base + 0U] = xMg;
    pWindow->features[base + 1U] = yMg;
    pWindow->features[base + 2U] = zMg;
    pWindow->count++;
    return true;
}

bool TIREMO_AccelWindow_IsFull(const TIREMO_AccelWindow_t *pWindow)
{
    if (pWindow == NULL)
    {
        return false;
    }

    return (pWindow->count >= TIREMO_ACCEL_WINDOW_SAMPLES);
}

const float *TIREMO_AccelWindow_Data(const TIREMO_AccelWindow_t *pWindow)
{
    if (pWindow == NULL)
    {
        return NULL;
    }

    return pWindow->features;
}
