/******************************************************************************
 * @file    tiremo_hal_gpio.h
 * @brief   Tiremo GPIO hardware abstraction API.
 *
 * Provides a vendor-independent GPIO interface over the ABOV PCU.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_HAL_GPIO_H
#define TIREMO_HAL_GPIO_H

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
    TIREMO_HAL_GPIO_PORT_A = 0,
    TIREMO_HAL_GPIO_PORT_B,
    TIREMO_HAL_GPIO_PORT_C,
    TIREMO_HAL_GPIO_PORT_D,
    TIREMO_HAL_GPIO_PORT_E,
    TIREMO_HAL_GPIO_PORT_F,
    TIREMO_HAL_GPIO_PORT_G,
    TIREMO_HAL_GPIO_PORT_MAX
} TIREMO_HAL_GPIO_Port_t;

typedef enum
{
    TIREMO_HAL_GPIO_PIN_0 = 0,
    TIREMO_HAL_GPIO_PIN_1,
    TIREMO_HAL_GPIO_PIN_2,
    TIREMO_HAL_GPIO_PIN_3,
    TIREMO_HAL_GPIO_PIN_4,
    TIREMO_HAL_GPIO_PIN_5,
    TIREMO_HAL_GPIO_PIN_6,
    TIREMO_HAL_GPIO_PIN_7,
    TIREMO_HAL_GPIO_PIN_8,
    TIREMO_HAL_GPIO_PIN_9,
    TIREMO_HAL_GPIO_PIN_10,
    TIREMO_HAL_GPIO_PIN_11,
    TIREMO_HAL_GPIO_PIN_12,
    TIREMO_HAL_GPIO_PIN_13,
    TIREMO_HAL_GPIO_PIN_14,
    TIREMO_HAL_GPIO_PIN_15,
    TIREMO_HAL_GPIO_PIN_MAX
} TIREMO_HAL_GPIO_Pin_t;

typedef enum
{
    TIREMO_HAL_GPIO_LEVEL_LOW = 0,
    TIREMO_HAL_GPIO_LEVEL_HIGH
} TIREMO_HAL_GPIO_Level_t;

typedef enum
{
    TIREMO_HAL_GPIO_MODE_INPUT = 0,
    TIREMO_HAL_GPIO_MODE_OUTPUT_PP,
    TIREMO_HAL_GPIO_MODE_OUTPUT_OD
} TIREMO_HAL_GPIO_Mode_t;

typedef enum
{
    TIREMO_HAL_GPIO_PULL_NONE = 0,
    TIREMO_HAL_GPIO_PULL_UP,
    TIREMO_HAL_GPIO_PULL_DOWN
} TIREMO_HAL_GPIO_Pull_t;

typedef enum
{
    TIREMO_HAL_GPIO_INTR_DISABLE = 0,
    TIREMO_HAL_GPIO_INTR_RISING,
    TIREMO_HAL_GPIO_INTR_FALLING,
    TIREMO_HAL_GPIO_INTR_BOTH
} TIREMO_HAL_GPIO_Intr_t;

typedef enum
{
    TIREMO_HAL_GPIO_STATUS_OK = 0,
    TIREMO_HAL_GPIO_STATUS_ERROR
} TIREMO_HAL_GPIO_Status_t;

typedef void (*TIREMO_HAL_GPIO_IrqHandler_t)(uint32_t event, void *pContext);

/******************************************************************************
 * Public Function Prototypes
 *****************************************************************************/

/**
 * @brief Configure the direction of a GPIO pin.
 *
 * @param[in] port  Target GPIO port.
 * @param[in] pin   Target GPIO pin.
 * @param[in] mode  Pin direction mode.
 *
 * @return GPIO operation status.
 */
TIREMO_HAL_GPIO_Status_t TIREMO_HAL_GPIO_SetMode(TIREMO_HAL_GPIO_Port_t port,
                                                 TIREMO_HAL_GPIO_Pin_t pin,
                                                 TIREMO_HAL_GPIO_Mode_t mode);

/**
 * @brief Configure the pull resistor of a GPIO pin.
 *
 * @param[in] port  Target GPIO port.
 * @param[in] pin   Target GPIO pin.
 * @param[in] pull  Pull configuration.
 *
 * @return GPIO operation status.
 */
TIREMO_HAL_GPIO_Status_t TIREMO_HAL_GPIO_SetPull(TIREMO_HAL_GPIO_Port_t port,
                                                 TIREMO_HAL_GPIO_Pin_t pin,
                                                 TIREMO_HAL_GPIO_Pull_t pull);

/**
 * @brief Write the output level of a GPIO pin.
 *
 * @param[in] port   Target GPIO port.
 * @param[in] pin    Target GPIO pin.
 * @param[in] level  Output level.
 *
 * @return GPIO operation status.
 */
TIREMO_HAL_GPIO_Status_t TIREMO_HAL_GPIO_WritePin(TIREMO_HAL_GPIO_Port_t port,
                                                  TIREMO_HAL_GPIO_Pin_t pin,
                                                  TIREMO_HAL_GPIO_Level_t level);

/**
 * @brief Read the input level of a GPIO pin.
 *
 * @param[in]  port    Target GPIO port.
 * @param[in]  pin     Target GPIO pin.
 * @param[out] pLevel  Destination for the pin level.
 *
 * @return GPIO operation status.
 */
TIREMO_HAL_GPIO_Status_t TIREMO_HAL_GPIO_ReadPin(TIREMO_HAL_GPIO_Port_t port,
                                                 TIREMO_HAL_GPIO_Pin_t pin,
                                                 TIREMO_HAL_GPIO_Level_t *pLevel);

/**
 * @brief Toggle the output level of a GPIO pin.
 *
 * @param[in] port  Target GPIO port.
 * @param[in] pin   Target GPIO pin.
 *
 * @return GPIO operation status.
 */
TIREMO_HAL_GPIO_Status_t TIREMO_HAL_GPIO_TogglePin(TIREMO_HAL_GPIO_Port_t port,
                                                   TIREMO_HAL_GPIO_Pin_t pin);

/**
 * @brief Configure an edge interrupt on a GPIO pin.
 *
 * @param[in] port   Target GPIO port.
 * @param[in] pin    Target GPIO pin.
 * @param[in] intr   Interrupt edge configuration.
 *
 * @return GPIO operation status.
 */
TIREMO_HAL_GPIO_Status_t TIREMO_HAL_GPIO_SetInterrupt(
    TIREMO_HAL_GPIO_Port_t port,
    TIREMO_HAL_GPIO_Pin_t pin,
    TIREMO_HAL_GPIO_Intr_t intr);

/**
 * @brief Register a port interrupt handler.
 *
 * @param[in] port      Target GPIO port.
 * @param[in] handler   Interrupt callback.
 * @param[in] pContext  User context forwarded to the callback.
 * @param[in] priority  Interrupt priority.
 *
 * @return GPIO operation status.
 */
TIREMO_HAL_GPIO_Status_t TIREMO_HAL_GPIO_SetPortIrqHandler(
    TIREMO_HAL_GPIO_Port_t port,
    TIREMO_HAL_GPIO_IrqHandler_t handler,
    void *pContext,
    uint32_t priority);

/**
 * @brief Build the interrupt event mask for a pin.
 *
 * @param[in] pin  Target GPIO pin.
 *
 * @return Interrupt event mask used by the PCU IRQ callback.
 */
uint32_t TIREMO_HAL_GPIO_GetInterruptMask(TIREMO_HAL_GPIO_Pin_t pin);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_HAL_GPIO_H */
