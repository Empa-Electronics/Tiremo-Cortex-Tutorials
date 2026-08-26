/******************************************************************************
 * @file    tiremo_led.h
 * @brief   Tiremo LED driver API.
 *
 * Controls onboard LEDs through the Tiremo GPIO hardware abstraction.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_LED_H
#define TIREMO_LED_H

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
 * Public Types
 *****************************************************************************/

typedef enum
{
    TIREMO_LED_1 = 0,
    TIREMO_LED_2,
    TIREMO_LED_3,
    TIREMO_LED_4,
    TIREMO_LED_5,
    TIREMO_LED_6,
    TIREMO_LED_7,
    TIREMO_LED_8,
    TIREMO_LED_9,
    TIREMO_LED_10,
    TIREMO_LED_ID_MAX
} TIREMO_LED_Id_t;

/******************************************************************************
 * Public Function Prototypes
 *****************************************************************************/

/**
 * @brief Initialize all onboard LEDs.
 *
 * Turns every LED off after initialization.
 *
 * @warning Call PRV_PORT_Init() before this function.
 */
void TIREMO_LED_Init(void);

/**
 * @brief Turn on the specified LED.
 *
 * @param[in] ledId  Target LED identifier.
 */
void TIREMO_LED_On(TIREMO_LED_Id_t ledId);

/**
 * @brief Turn off the specified LED.
 *
 * @param[in] ledId  Target LED identifier.
 */
void TIREMO_LED_Off(TIREMO_LED_Id_t ledId);

/**
 * @brief Toggle the specified LED.
 *
 * @param[in] ledId  Target LED identifier.
 */
void TIREMO_LED_Toggle(TIREMO_LED_Id_t ledId);

/**
 * @brief Set the logical state of the specified LED.
 *
 * @param[in] ledId  Target LED identifier.
 * @param[in] isOn   true to turn the LED on, false to turn it off.
 */
void TIREMO_LED_SetState(TIREMO_LED_Id_t ledId, bool isOn);

/**
 * @brief Get the logical state of the specified LED.
 *
 * @param[in] ledId  Target LED identifier.
 *
 * @return true when the LED is on.
 *
 * @retval true   LED is ON.
 * @retval false  LED is OFF or the identifier is invalid.
 */
bool TIREMO_LED_GetState(TIREMO_LED_Id_t ledId);

/**
 * @brief Turn on all onboard LEDs.
 */
void TIREMO_LED_AllOn(void);

/**
 * @brief Turn off all onboard LEDs.
 */
void TIREMO_LED_AllOff(void);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_LED_H */
