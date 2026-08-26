/******************************************************************************
 * @file    tiremo_led.c
 * @brief   Tiremo LED driver implementation.
 *
 * Controls onboard LEDs through the Tiremo GPIO hardware abstraction.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "../Include/tiremo_led.h"
#include "../../../Board/board_config.h"
#include "../../../Tiremo_Hal/Include/tiremo_hal_gpio.h"

/******************************************************************************
 * Private Types
 *****************************************************************************/

typedef struct
{
    TIREMO_HAL_GPIO_Port_t port;
    TIREMO_HAL_GPIO_Pin_t pin;
} TIREMO_LED_Pin_t;

/******************************************************************************
 * Private Variables
 *****************************************************************************/

static bool s_ledState[TIREMO_BOARD_LED_COUNT];

static const TIREMO_LED_Pin_t s_ledPinMap[TIREMO_BOARD_LED_COUNT] =
{
    { TIREMO_BOARD_LED1_PORT,  TIREMO_BOARD_LED1_PIN  },
    { TIREMO_BOARD_LED2_PORT,  TIREMO_BOARD_LED2_PIN  },
    { TIREMO_BOARD_LED3_PORT,  TIREMO_BOARD_LED3_PIN  },
    { TIREMO_BOARD_LED4_PORT,  TIREMO_BOARD_LED4_PIN  },
    { TIREMO_BOARD_LED5_PORT,  TIREMO_BOARD_LED5_PIN  },
    { TIREMO_BOARD_LED6_PORT,  TIREMO_BOARD_LED6_PIN  },
    { TIREMO_BOARD_LED7_PORT,  TIREMO_BOARD_LED7_PIN  },
    { TIREMO_BOARD_LED8_PORT,  TIREMO_BOARD_LED8_PIN  },
    { TIREMO_BOARD_LED9_PORT,  TIREMO_BOARD_LED9_PIN  },
    { TIREMO_BOARD_LED10_PORT, TIREMO_BOARD_LED10_PIN }
};

/******************************************************************************
 * Private Function Prototypes
 *****************************************************************************/

static bool prvLEDIsValidId(TIREMO_LED_Id_t ledId);
static void prvLEDWritePin(TIREMO_LED_Id_t ledId, bool isOn);

/******************************************************************************
 * Public Functions
 *****************************************************************************/

void TIREMO_LED_Init(void)
{
    uint32_t index;
    const TIREMO_LED_Pin_t *pPin;

    for (index = 0U; index < TIREMO_BOARD_LED_COUNT; index++)
    {
        pPin = &s_ledPinMap[index];
        (void)TIREMO_HAL_GPIO_SetMode(pPin->port, pPin->pin,
                                      TIREMO_HAL_GPIO_MODE_OUTPUT_PP);
        prvLEDWritePin((TIREMO_LED_Id_t)index, false);
    }
}

void TIREMO_LED_On(TIREMO_LED_Id_t ledId)
{
    if (prvLEDIsValidId(ledId))
    {
        prvLEDWritePin(ledId, true);
    }
}

void TIREMO_LED_Off(TIREMO_LED_Id_t ledId)
{
    if (prvLEDIsValidId(ledId))
    {
        prvLEDWritePin(ledId, false);
    }
}

void TIREMO_LED_Toggle(TIREMO_LED_Id_t ledId)
{
    if (prvLEDIsValidId(ledId))
    {
        prvLEDWritePin(ledId, !s_ledState[ledId]);
    }
}

void TIREMO_LED_SetState(TIREMO_LED_Id_t ledId, bool isOn)
{
    if (prvLEDIsValidId(ledId))
    {
        prvLEDWritePin(ledId, isOn);
    }
}

bool TIREMO_LED_GetState(TIREMO_LED_Id_t ledId)
{
    if (prvLEDIsValidId(ledId))
    {
        return s_ledState[ledId];
    }

    return false;
}

void TIREMO_LED_AllOn(void)
{
    uint32_t index;

    for (index = 0U; index < TIREMO_BOARD_LED_COUNT; index++)
    {
        prvLEDWritePin((TIREMO_LED_Id_t)index, true);
    }
}

void TIREMO_LED_AllOff(void)
{
    uint32_t index;

    for (index = 0U; index < TIREMO_BOARD_LED_COUNT; index++)
    {
        prvLEDWritePin((TIREMO_LED_Id_t)index, false);
    }
}

/******************************************************************************
 * Private Functions
 *****************************************************************************/

static bool prvLEDIsValidId(TIREMO_LED_Id_t ledId)
{
    return ((uint32_t)ledId < TIREMO_BOARD_LED_COUNT);
}

static void prvLEDWritePin(TIREMO_LED_Id_t ledId, bool isOn)
{
    const TIREMO_LED_Pin_t *pPin = &s_ledPinMap[ledId];
    TIREMO_HAL_GPIO_Level_t level;

#if (TIREMO_BOARD_LED_ACTIVE_LOW == 1U)
    level = isOn ? TIREMO_HAL_GPIO_LEVEL_LOW : TIREMO_HAL_GPIO_LEVEL_HIGH;
#else
    level = isOn ? TIREMO_HAL_GPIO_LEVEL_HIGH : TIREMO_HAL_GPIO_LEVEL_LOW;
#endif

    (void)TIREMO_HAL_GPIO_WritePin(pPin->port, pPin->pin, level);
    s_ledState[ledId] = isOn;
}
