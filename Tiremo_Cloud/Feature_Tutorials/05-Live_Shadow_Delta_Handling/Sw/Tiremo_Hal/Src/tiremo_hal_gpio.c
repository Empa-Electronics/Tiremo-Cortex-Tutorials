/******************************************************************************
 * @file    tiremo_hal_gpio.c
 * @brief   Tiremo GPIO hardware abstraction implementation.
 *
 * Adapts the Tiremo GPIO API to the ABOV PCU HAL.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "../Include/tiremo_hal_gpio.h"

#include "hal_pcu.h"

#include <stddef.h>

/******************************************************************************
 * Private Types
 *****************************************************************************/

typedef struct
{
    TIREMO_HAL_GPIO_IrqHandler_t handler;
    void *pContext;
} TIREMO_HAL_GPIO_IrqSlot_t;

/******************************************************************************
 * Private Variables
 *****************************************************************************/

static TIREMO_HAL_GPIO_IrqSlot_t s_irqSlots[TIREMO_HAL_GPIO_PORT_MAX];

/******************************************************************************
 * Private Function Prototypes
 *****************************************************************************/

static bool prvGPIOIsPortValid(TIREMO_HAL_GPIO_Port_t port);
static bool prvGPIOIsPinValid(TIREMO_HAL_GPIO_Pin_t pin);
static void prvGPIOVendorIrqHandler(uint32_t vendorEvent, void *pContext);

/******************************************************************************
 * Public Functions
 *****************************************************************************/

TIREMO_HAL_GPIO_Status_t TIREMO_HAL_GPIO_SetMode(TIREMO_HAL_GPIO_Port_t port,
                                                 TIREMO_HAL_GPIO_Pin_t pin,
                                                 TIREMO_HAL_GPIO_Mode_t mode)
{
    PCU_INOUT_e vendorMode;
    HAL_ERR_e vendorStatus;

    if ((!prvGPIOIsPortValid(port)) || (!prvGPIOIsPinValid(pin)))
    {
        return TIREMO_HAL_GPIO_STATUS_ERROR;
    }

    switch (mode)
    {
        case TIREMO_HAL_GPIO_MODE_INPUT:
            vendorMode = PCU_INOUT_INPUT;
            break;

        case TIREMO_HAL_GPIO_MODE_OUTPUT_PP:
            vendorMode = PCU_INOUT_OUTPUT_PUSH_PULL;
            break;

        case TIREMO_HAL_GPIO_MODE_OUTPUT_OD:
            vendorMode = PCU_INOUT_OUTPUT_OPEN_DRAIN;
            break;

        default:
            return TIREMO_HAL_GPIO_STATUS_ERROR;
    }

    vendorStatus = HAL_PCU_SetInOutMode((PCU_ID_e)port,
                                        (PCU_PIN_ID_e)pin,
                                        vendorMode);

    return (vendorStatus == HAL_ERR_OK) ? TIREMO_HAL_GPIO_STATUS_OK
                                        : TIREMO_HAL_GPIO_STATUS_ERROR;
}

TIREMO_HAL_GPIO_Status_t TIREMO_HAL_GPIO_SetPull(TIREMO_HAL_GPIO_Port_t port,
                                                 TIREMO_HAL_GPIO_Pin_t pin,
                                                 TIREMO_HAL_GPIO_Pull_t pull)
{
    PCU_PUPD_e vendorPull;
    HAL_ERR_e vendorStatus;

    if ((!prvGPIOIsPortValid(port)) || (!prvGPIOIsPinValid(pin)))
    {
        return TIREMO_HAL_GPIO_STATUS_ERROR;
    }

    switch (pull)
    {
        case TIREMO_HAL_GPIO_PULL_NONE:
            vendorPull = PCU_PUPD_DISABLED;
            break;

        case TIREMO_HAL_GPIO_PULL_UP:
            vendorPull = PCU_PUPD_UP;
            break;

        case TIREMO_HAL_GPIO_PULL_DOWN:
            vendorPull = PCU_PUPD_DOWN;
            break;

        default:
            return TIREMO_HAL_GPIO_STATUS_ERROR;
    }

    vendorStatus = HAL_PCU_SetPullUpDown((PCU_ID_e)port,
                                         (PCU_PIN_ID_e)pin,
                                         vendorPull);

    return (vendorStatus == HAL_ERR_OK) ? TIREMO_HAL_GPIO_STATUS_OK
                                        : TIREMO_HAL_GPIO_STATUS_ERROR;
}

TIREMO_HAL_GPIO_Status_t TIREMO_HAL_GPIO_WritePin(TIREMO_HAL_GPIO_Port_t port,
                                                  TIREMO_HAL_GPIO_Pin_t pin,
                                                  TIREMO_HAL_GPIO_Level_t level)
{
    PCU_OUTPUT_BIT_e vendorBit;
    HAL_ERR_e vendorStatus;

    if ((!prvGPIOIsPortValid(port)) || (!prvGPIOIsPinValid(pin)))
    {
        return TIREMO_HAL_GPIO_STATUS_ERROR;
    }

    vendorBit = (level == TIREMO_HAL_GPIO_LEVEL_HIGH) ? PCU_OUTPUT_BIT_SET
                                                      : PCU_OUTPUT_BIT_CLEAR;

    vendorStatus = HAL_PCU_SetOutputBit((PCU_ID_e)port,
                                        (PCU_PIN_ID_e)pin,
                                        vendorBit);

    return (vendorStatus == HAL_ERR_OK) ? TIREMO_HAL_GPIO_STATUS_OK
                                        : TIREMO_HAL_GPIO_STATUS_ERROR;
}

TIREMO_HAL_GPIO_Status_t TIREMO_HAL_GPIO_ReadPin(TIREMO_HAL_GPIO_Port_t port,
                                                 TIREMO_HAL_GPIO_Pin_t pin,
                                                 TIREMO_HAL_GPIO_Level_t *pLevel)
{
    PCU_PORT_e vendorLevel = PCU_PORT_LOW;
    HAL_ERR_e vendorStatus;

    if ((!prvGPIOIsPortValid(port)) ||
        (!prvGPIOIsPinValid(pin)) ||
        (pLevel == NULL))
    {
        return TIREMO_HAL_GPIO_STATUS_ERROR;
    }

    vendorStatus = HAL_PCU_GetInputValue((PCU_ID_e)port,
                                         (PCU_PIN_ID_e)pin,
                                         &vendorLevel);
    if (vendorStatus != HAL_ERR_OK)
    {
        return TIREMO_HAL_GPIO_STATUS_ERROR;
    }

    *pLevel = (vendorLevel == PCU_PORT_HIGH) ? TIREMO_HAL_GPIO_LEVEL_HIGH
                                             : TIREMO_HAL_GPIO_LEVEL_LOW;

    return TIREMO_HAL_GPIO_STATUS_OK;
}

TIREMO_HAL_GPIO_Status_t TIREMO_HAL_GPIO_TogglePin(TIREMO_HAL_GPIO_Port_t port,
                                                   TIREMO_HAL_GPIO_Pin_t pin)
{
    TIREMO_HAL_GPIO_Level_t level;

    if (TIREMO_HAL_GPIO_ReadPin(port, pin, &level) != TIREMO_HAL_GPIO_STATUS_OK)
    {
        return TIREMO_HAL_GPIO_STATUS_ERROR;
    }

    level = (level == TIREMO_HAL_GPIO_LEVEL_HIGH) ? TIREMO_HAL_GPIO_LEVEL_LOW
                                                  : TIREMO_HAL_GPIO_LEVEL_HIGH;

    return TIREMO_HAL_GPIO_WritePin(port, pin, level);
}

TIREMO_HAL_GPIO_Status_t TIREMO_HAL_GPIO_SetInterrupt(
    TIREMO_HAL_GPIO_Port_t port,
    TIREMO_HAL_GPIO_Pin_t pin,
    TIREMO_HAL_GPIO_Intr_t intr)
{
    PCU_INTR_MODE_e vendorMode;
    PCU_INTR_TRG_e vendorTrigger;
    HAL_ERR_e vendorStatus;

    if ((!prvGPIOIsPortValid(port)) || (!prvGPIOIsPinValid(pin)))
    {
        return TIREMO_HAL_GPIO_STATUS_ERROR;
    }

    switch (intr)
    {
        case TIREMO_HAL_GPIO_INTR_DISABLE:
            vendorMode = PCU_INTR_MODE_DISABLE;
            vendorTrigger = PCU_INTR_TRG_DISABLE;
            break;

        case TIREMO_HAL_GPIO_INTR_RISING:
            vendorMode = PCU_INTR_MODE_EDGE;
            vendorTrigger = PCU_INTR_TRG_HIGH_RISING;
            break;

        case TIREMO_HAL_GPIO_INTR_FALLING:
            vendorMode = PCU_INTR_MODE_EDGE;
            vendorTrigger = PCU_INTR_TRG_LOW_FALLING;
            break;

        case TIREMO_HAL_GPIO_INTR_BOTH:
            vendorMode = PCU_INTR_MODE_EDGE;
            vendorTrigger = PCU_INTR_TRG_BOTH_LEVEL_EDGE;
            break;

        default:
            return TIREMO_HAL_GPIO_STATUS_ERROR;
    }

    vendorStatus = HAL_PCU_SetIntrPort((PCU_ID_e)port,
                                       (PCU_PIN_ID_e)pin,
                                       vendorMode,
                                       vendorTrigger,
                                       0U);

    return (vendorStatus == HAL_ERR_OK) ? TIREMO_HAL_GPIO_STATUS_OK
                                        : TIREMO_HAL_GPIO_STATUS_ERROR;
}

TIREMO_HAL_GPIO_Status_t TIREMO_HAL_GPIO_SetPortIrqHandler(
    TIREMO_HAL_GPIO_Port_t port,
    TIREMO_HAL_GPIO_IrqHandler_t handler,
    void *pContext,
    uint32_t priority)
{
    TIREMO_HAL_GPIO_IrqSlot_t *pSlot;
    PCU_IRQ_CFG_t irqCfg;
    HAL_ERR_e vendorStatus;

    if ((!prvGPIOIsPortValid(port)) || (handler == NULL))
    {
        return TIREMO_HAL_GPIO_STATUS_ERROR;
    }

    pSlot = &s_irqSlots[(uint32_t)port];
    pSlot->handler = handler;
    pSlot->pContext = pContext;

    irqCfg.eId = (PCU_ID_e)port;
    irqCfg.eOps = PCU_OPS_INTR;
    irqCfg.pfnHandler = prvGPIOVendorIrqHandler;
    irqCfg.pContext = pSlot;
    irqCfg.un32IRQPrio = priority;
    irqCfg.un8IntNum = 0U;

    vendorStatus = HAL_PCU_SetIRQ(&irqCfg);
    if (vendorStatus != HAL_ERR_OK)
    {
        pSlot->handler = NULL;
        pSlot->pContext = NULL;
        return TIREMO_HAL_GPIO_STATUS_ERROR;
    }

    return TIREMO_HAL_GPIO_STATUS_OK;
}

uint32_t TIREMO_HAL_GPIO_GetInterruptMask(TIREMO_HAL_GPIO_Pin_t pin)
{
    if (!prvGPIOIsPinValid(pin))
    {
        return 0U;
    }

    return (0x3UL << ((uint32_t)pin * 2U));
}

/******************************************************************************
 * Private Functions
 *****************************************************************************/

static bool prvGPIOIsPortValid(TIREMO_HAL_GPIO_Port_t port)
{
    return ((uint32_t)port < (uint32_t)TIREMO_HAL_GPIO_PORT_MAX);
}

static bool prvGPIOIsPinValid(TIREMO_HAL_GPIO_Pin_t pin)
{
    return ((uint32_t)pin < (uint32_t)TIREMO_HAL_GPIO_PIN_MAX);
}

static void prvGPIOVendorIrqHandler(uint32_t vendorEvent, void *pContext)
{
    TIREMO_HAL_GPIO_IrqSlot_t *pSlot = (TIREMO_HAL_GPIO_IrqSlot_t *)pContext;

    if ((pSlot == NULL) || (pSlot->handler == NULL))
    {
        return;
    }

    pSlot->handler(vendorEvent, pSlot->pContext);
}
