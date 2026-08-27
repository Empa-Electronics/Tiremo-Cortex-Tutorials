/******************************************************************************
 * @file    tiremo_button.c
 * @brief   Tiremo button driver implementation.
 *
 * Controls onboard buttons through the Tiremo GPIO hardware abstraction.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "../Include/tiremo_button.h"
#include "../../../Board/board_config.h"
#include "../../../Tiremo_Hal/Include/tiremo_hal_gpio.h"

#include <stddef.h>

/******************************************************************************
 * Private Types
 *****************************************************************************/

typedef struct
{
    TIREMO_HAL_GPIO_Port_t port;
    TIREMO_HAL_GPIO_Pin_t pin;
} TIREMO_Button_Pin_t;

/******************************************************************************
 * Private Variables
 *****************************************************************************/

static const TIREMO_Button_Pin_t s_buttonPinMap[TIREMO_BOARD_BUTTON_COUNT] =
{
    { TIREMO_BOARD_USER_BUTTON_PORT, TIREMO_BOARD_USER_BUTTON_PIN }
};

static TIREMO_Button_Callback_t s_callback = NULL;
static void *s_callbackContext = NULL;
static volatile uint8_t s_eventPending[TIREMO_BOARD_BUTTON_COUNT];
static volatile TIREMO_Button_State_t s_lastIntrState[TIREMO_BOARD_BUTTON_COUNT];
static bool s_portIrqRegistered = false;

/******************************************************************************
 * Private Function Prototypes
 *****************************************************************************/

static bool prvButtonIsValidId(TIREMO_Button_Id_t buttonId);
static TIREMO_HAL_GPIO_Level_t prvButtonReadLevel(TIREMO_Button_Id_t buttonId);
static void prvButtonPortIrqHandler(uint32_t event, void *pContext);

/******************************************************************************
 * Public Functions
 *****************************************************************************/

void TIREMO_Button_Init(void)
{
    uint32_t index;

    for (index = 0U; index < TIREMO_BOARD_BUTTON_COUNT; index++)
    {
        s_eventPending[index] = 0U;
        s_lastIntrState[index] = TIREMO_BUTTON_STATE_RELEASED;
    }
}

bool TIREMO_Button_IsPressed(TIREMO_Button_Id_t buttonId)
{
    TIREMO_HAL_GPIO_Level_t level;

    if (!prvButtonIsValidId(buttonId))
    {
        return false;
    }

    level = prvButtonReadLevel(buttonId);

#if (TIREMO_BOARD_BUTTON_ACTIVE_LOW == 1U)
    return (level == TIREMO_HAL_GPIO_LEVEL_LOW);
#else
    return (level == TIREMO_HAL_GPIO_LEVEL_HIGH);
#endif
}

bool TIREMO_Button_IsReleased(TIREMO_Button_Id_t buttonId)
{
    if (!prvButtonIsValidId(buttonId))
    {
        return false;
    }

    return !TIREMO_Button_IsPressed(buttonId);
}

TIREMO_Button_State_t TIREMO_Button_GetState(TIREMO_Button_Id_t buttonId)
{
    if (TIREMO_Button_IsPressed(buttonId))
    {
        return TIREMO_BUTTON_STATE_PRESSED;
    }

    return TIREMO_BUTTON_STATE_RELEASED;
}

void TIREMO_Button_IntrInit(TIREMO_Button_Id_t buttonId)
{
    const TIREMO_Button_Pin_t *pPin;

    if (!prvButtonIsValidId(buttonId))
    {
        return;
    }

    pPin = &s_buttonPinMap[buttonId];

    (void)TIREMO_HAL_GPIO_SetInterrupt(pPin->port,
                                       pPin->pin,
                                       TIREMO_HAL_GPIO_INTR_BOTH);

    if (!s_portIrqRegistered)
    {
        if (TIREMO_HAL_GPIO_SetPortIrqHandler(
                pPin->port,
                prvButtonPortIrqHandler,
                NULL,
                TIREMO_BOARD_USER_BUTTON_IRQ_PRIO) == TIREMO_HAL_GPIO_STATUS_OK)
        {
            s_portIrqRegistered = true;
        }
    }
}

void TIREMO_Button_IntrEnable(TIREMO_Button_Id_t buttonId)
{
    const TIREMO_Button_Pin_t *pPin;

    if (!prvButtonIsValidId(buttonId))
    {
        return;
    }

    pPin = &s_buttonPinMap[buttonId];
    (void)TIREMO_HAL_GPIO_SetInterrupt(pPin->port,
                                       pPin->pin,
                                       TIREMO_HAL_GPIO_INTR_BOTH);
}

void TIREMO_Button_IntrDisable(TIREMO_Button_Id_t buttonId)
{
    const TIREMO_Button_Pin_t *pPin;

    if (!prvButtonIsValidId(buttonId))
    {
        return;
    }

    pPin = &s_buttonPinMap[buttonId];
    (void)TIREMO_HAL_GPIO_SetInterrupt(pPin->port,
                                       pPin->pin,
                                       TIREMO_HAL_GPIO_INTR_DISABLE);
}

void TIREMO_Button_SetCallback(TIREMO_Button_Callback_t callback,
                               void *pContext)
{
    s_callback = callback;
    s_callbackContext = pContext;
}

bool TIREMO_Button_IsEventPending(TIREMO_Button_Id_t buttonId)
{
    if (!prvButtonIsValidId(buttonId))
    {
        return false;
    }

    return (s_eventPending[buttonId] != 0U);
}

void TIREMO_Button_ClearEvent(TIREMO_Button_Id_t buttonId)
{
    if (prvButtonIsValidId(buttonId))
    {
        s_eventPending[buttonId] = 0U;
    }
}

TIREMO_Button_State_t TIREMO_Button_GetLastIntrState(
    TIREMO_Button_Id_t buttonId)
{
    if (!prvButtonIsValidId(buttonId))
    {
        return TIREMO_BUTTON_STATE_RELEASED;
    }

    return s_lastIntrState[buttonId];
}

/******************************************************************************
 * Private Functions
 *****************************************************************************/

static bool prvButtonIsValidId(TIREMO_Button_Id_t buttonId)
{
    return ((uint32_t)buttonId < TIREMO_BOARD_BUTTON_COUNT);
}

static TIREMO_HAL_GPIO_Level_t prvButtonReadLevel(TIREMO_Button_Id_t buttonId)
{
    const TIREMO_Button_Pin_t *pPin = &s_buttonPinMap[buttonId];
    TIREMO_HAL_GPIO_Level_t level = TIREMO_HAL_GPIO_LEVEL_HIGH;

    (void)TIREMO_HAL_GPIO_ReadPin(pPin->port, pPin->pin, &level);
    return level;
}

static void prvButtonPortIrqHandler(uint32_t event, void *pContext)
{
    uint32_t index;
    uint32_t mask;

    (void)pContext;

    for (index = 0U; index < TIREMO_BOARD_BUTTON_COUNT; index++)
    {
        mask = TIREMO_HAL_GPIO_GetInterruptMask(s_buttonPinMap[index].pin);
        if ((event & mask) == 0U)
        {
            continue;
        }

        s_lastIntrState[index] = TIREMO_Button_GetState(
            (TIREMO_Button_Id_t)index);
        s_eventPending[index] = 1U;

        if (s_callback != NULL)
        {
            s_callback((TIREMO_Button_Id_t)index,
                       s_lastIntrState[index],
                       s_callbackContext);
        }
    }
}
