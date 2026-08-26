/******************************************************************************
 * @file    tiremo_battery.c
 * @brief   Tiremo battery (AVDD supply) driver implementation.
 *
 * AVDD (mV) = (VCORE_REF_MV * ADC_FULL_SCALE) / vcoreRaw
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "../Include/tiremo_battery.h"
#include "../../../Board/board_config.h"
#include "../../../Tiremo_Hal/Include/tiremo_hal_adc.h"

#include <stddef.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/** @brief Internal VCORE bandgap reference voltage, millivolts (~1.0V). */
#define TIREMO_BAT_VCORE_REF_MV         (1000U)

/** @brief ADC full scale for the AVDD formula (12-bit). */
#define TIREMO_BAT_ADC_FULL_SCALE       (4096U)

/** @brief Sequence sampling time for the VCORE channel read. */
#define TIREMO_BAT_ADC_SAMPLING_TIME    (10U)

/** @brief Busy-wait loops after Init() for VCORE reference stabilization. */
#define TIREMO_BAT_VCORE_STABILIZE_LOOPS (500000U)

/******************************************************************************
 * Private Variables
 *****************************************************************************/

static bool s_initialized = false;

/******************************************************************************
 * Public Functions
 *****************************************************************************/

bool TIREMO_BAT_Init(void)
{
    volatile uint32_t i;

    if (s_initialized)
    {
        return true;
    }

    for (i = 0U; i < TIREMO_BAT_VCORE_STABILIZE_LOOPS; i++)
    {
    }

    s_initialized = true;
    return true;
}

void TIREMO_BAT_Deinit(void)
{
    s_initialized = false;
}

bool TIREMO_BAT_IsInitialized(void)
{
    return s_initialized;
}

bool TIREMO_BAT_Read(uint32_t *pAvddMv, uint16_t *pVcoreRaw)
{
    uint16_t vcoreRaw;

    if ((!s_initialized) || (pAvddMv == NULL))
    {
        return false;
    }

    if (TIREMO_HAL_ADC_ReadChannel(TIREMO_BOARD_BATTERY_ADC_ID,
                                   TIREMO_BOARD_BATTERY_VCORE_CHANNEL,
                                   TIREMO_BAT_ADC_SAMPLING_TIME,
                                   &vcoreRaw) != TIREMO_HAL_ADC_STATUS_OK)
    {
        return false;
    }

    if (vcoreRaw == 0U)
    {
        return false;
    }

    *pAvddMv = ((uint32_t)TIREMO_BAT_VCORE_REF_MV * TIREMO_BAT_ADC_FULL_SCALE) /
               (uint32_t)vcoreRaw;

    if (pVcoreRaw != NULL)
    {
        *pVcoreRaw = vcoreRaw;
    }

    return true;
}
