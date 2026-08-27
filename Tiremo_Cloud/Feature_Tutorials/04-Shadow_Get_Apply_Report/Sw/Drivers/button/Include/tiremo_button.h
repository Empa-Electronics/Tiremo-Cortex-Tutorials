/******************************************************************************
 * @file    tiremo_button.h
 * @brief   Tiremo button driver API.
 *
 * Controls onboard buttons through the Tiremo GPIO hardware abstraction.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_BUTTON_H
#define TIREMO_BUTTON_H

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
    TIREMO_BUTTON_USER = 0,
    TIREMO_BUTTON_ID_MAX
} TIREMO_Button_Id_t;

typedef enum
{
    TIREMO_BUTTON_STATE_RELEASED = 0,
    TIREMO_BUTTON_STATE_PRESSED
} TIREMO_Button_State_t;

/**
 * @brief Button interrupt callback type.
 *
 * @param[in] buttonId  Button that generated the interrupt.
 * @param[in] state     Logical button state after the edge event.
 * @param[in] pContext  User context supplied at registration.
 *
 * @warning The callback runs in interrupt context and must remain short.
 */
typedef void (*TIREMO_Button_Callback_t)(TIREMO_Button_Id_t buttonId,
                                         TIREMO_Button_State_t state,
                                         void *pContext);

/******************************************************************************
 * Public Function Prototypes
 *****************************************************************************/

/**
 * @brief Initialize the button driver.
 *
 * @warning Call PRV_PORT_Init() before this function.
 */
void TIREMO_Button_Init(void);

/**
 * @brief Check whether a button is currently pressed.
 *
 * @param[in] buttonId  Target button identifier.
 *
 * @return true when the button is pressed.
 */
bool TIREMO_Button_IsPressed(TIREMO_Button_Id_t buttonId);

/**
 * @brief Check whether a button is currently released.
 *
 * @param[in] buttonId  Target button identifier.
 *
 * @return true when the button is released.
 */
bool TIREMO_Button_IsReleased(TIREMO_Button_Id_t buttonId);

/**
 * @brief Get the logical pressed or released state of a button.
 *
 * @param[in] buttonId  Target button identifier.
 *
 * @return Current logical button state.
 */
TIREMO_Button_State_t TIREMO_Button_GetState(TIREMO_Button_Id_t buttonId);

/**
 * @brief Configure edge interrupt support for a button.
 *
 * @param[in] buttonId  Target button identifier.
 */
void TIREMO_Button_IntrInit(TIREMO_Button_Id_t buttonId);

/**
 * @brief Enable the interrupt for a button.
 *
 * @param[in] buttonId  Target button identifier.
 */
void TIREMO_Button_IntrEnable(TIREMO_Button_Id_t buttonId);

/**
 * @brief Disable the interrupt for a button.
 *
 * @param[in] buttonId  Target button identifier.
 */
void TIREMO_Button_IntrDisable(TIREMO_Button_Id_t buttonId);

/**
 * @brief Register a button interrupt callback.
 *
 * @param[in] callback  Callback function, or NULL to unregister.
 * @param[in] pContext  User context forwarded to the callback.
 */
void TIREMO_Button_SetCallback(TIREMO_Button_Callback_t callback,
                               void *pContext);

/**
 * @brief Check whether a button interrupt event is pending.
 *
 * @param[in] buttonId  Target button identifier.
 *
 * @return true when an unprocessed interrupt event exists.
 */
bool TIREMO_Button_IsEventPending(TIREMO_Button_Id_t buttonId);

/**
 * @brief Clear the pending interrupt event for a button.
 *
 * @param[in] buttonId  Target button identifier.
 */
void TIREMO_Button_ClearEvent(TIREMO_Button_Id_t buttonId);

/**
 * @brief Get the last state captured by the interrupt handler.
 *
 * @param[in] buttonId  Target button identifier.
 *
 * @return Last captured button state.
 */
TIREMO_Button_State_t TIREMO_Button_GetLastIntrState(
    TIREMO_Button_Id_t buttonId);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_BUTTON_H */
