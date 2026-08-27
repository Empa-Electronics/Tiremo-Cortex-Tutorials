/******************************************************************************
 * @file    tiremo_hal_uart.c
 * @brief   Tiremo UART hardware abstraction implementation.
 *
 * Adapts the Tiremo UART API to the ABOV UART HAL.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "../Include/tiremo_hal_uart.h"

#include "hal_uart.h"

#include <stddef.h>

/******************************************************************************
 * Private Types
 *****************************************************************************/

typedef struct
{
    TIREMO_HAL_UART_IrqHandler_t handler;
    void *pContext;
} TIREMO_HAL_UART_IrqSlot_t;

/******************************************************************************
 * Private Variables
 *****************************************************************************/

static TIREMO_HAL_UART_IrqSlot_t s_irqSlots[TIREMO_HAL_UART_PORT_MAX];

/******************************************************************************
 * Private Function Prototypes
 *****************************************************************************/

static bool prvUARTIsPortValid(TIREMO_HAL_UART_Port_t port);
static void prvUARTVendorIrqHandler(uint32_t vendorEvent, void *pContext);
static uint32_t prvUARTConvertVendorEvent(uint32_t vendorEvent);

/******************************************************************************
 * Public Functions
 *****************************************************************************/

TIREMO_HAL_UART_Status_t TIREMO_HAL_UART_SetInterruptHandler(
    TIREMO_HAL_UART_Port_t port,
    TIREMO_HAL_UART_IrqHandler_t handler,
    void *pContext,
    uint32_t priority)
{
    HAL_ERR_e vendorStatus;
    TIREMO_HAL_UART_IrqSlot_t *pSlot;

    if ((!prvUARTIsPortValid(port)) || (handler == NULL))
    {
        return TIREMO_HAL_UART_STATUS_ERROR;
    }

    pSlot = &s_irqSlots[(uint32_t)port];
    pSlot->handler = handler;
    pSlot->pContext = pContext;

    vendorStatus = HAL_UART_SetIRQ((UART_ID_e)port,
                                   UART_OPS_INTR,
                                   prvUARTVendorIrqHandler,
                                   pSlot,
                                   priority);
    if (vendorStatus != HAL_ERR_OK)
    {
        pSlot->handler = NULL;
        pSlot->pContext = NULL;
        return TIREMO_HAL_UART_STATUS_ERROR;
    }

    return TIREMO_HAL_UART_STATUS_OK;
}

TIREMO_HAL_UART_Status_t TIREMO_HAL_UART_Transmit(TIREMO_HAL_UART_Port_t port,
                                                  const uint8_t *pData,
                                                  uint32_t length,
                                                  bool isPolling)
{
    HAL_ERR_e vendorStatus;

    if ((!prvUARTIsPortValid(port)) || (pData == NULL) || (length == 0U))
    {
        return TIREMO_HAL_UART_STATUS_ERROR;
    }

    vendorStatus = HAL_UART_Transmit((UART_ID_e)port,
                                     (uint8_t *)pData,
                                     length,
                                     isPolling);

    return (vendorStatus == HAL_ERR_OK) ? TIREMO_HAL_UART_STATUS_OK
                                        : TIREMO_HAL_UART_STATUS_ERROR;
}

TIREMO_HAL_UART_Status_t TIREMO_HAL_UART_Receive(TIREMO_HAL_UART_Port_t port,
                                                 uint8_t *pData,
                                                 uint32_t length,
                                                 bool isPolling)
{
    HAL_ERR_e vendorStatus;

    if ((!prvUARTIsPortValid(port)) || (pData == NULL) || (length == 0U))
    {
        return TIREMO_HAL_UART_STATUS_ERROR;
    }

    vendorStatus = HAL_UART_Receive((UART_ID_e)port,
                                    pData,
                                    length,
                                    isPolling);

    return (vendorStatus == HAL_ERR_OK) ? TIREMO_HAL_UART_STATUS_OK
                                        : TIREMO_HAL_UART_STATUS_ERROR;
}

TIREMO_HAL_UART_Status_t TIREMO_HAL_UART_Abort(TIREMO_HAL_UART_Port_t port)
{
    HAL_ERR_e vendorStatus;

    if (!prvUARTIsPortValid(port))
    {
        return TIREMO_HAL_UART_STATUS_ERROR;
    }

    vendorStatus = HAL_UART_Abort((UART_ID_e)port);

    return (vendorStatus == HAL_ERR_OK) ? TIREMO_HAL_UART_STATUS_OK
                                        : TIREMO_HAL_UART_STATUS_ERROR;
}

/******************************************************************************
 * Private Functions
 *****************************************************************************/

static bool prvUARTIsPortValid(TIREMO_HAL_UART_Port_t port)
{
    return ((uint32_t)port < (uint32_t)TIREMO_HAL_UART_PORT_MAX);
}

static void prvUARTVendorIrqHandler(uint32_t vendorEvent, void *pContext)
{
    TIREMO_HAL_UART_IrqSlot_t *pSlot =
        (TIREMO_HAL_UART_IrqSlot_t *)pContext;
    uint32_t event;

    if ((pSlot == NULL) || (pSlot->handler == NULL))
    {
        return;
    }

    event = prvUARTConvertVendorEvent(vendorEvent);
    if (event != 0U)
    {
        pSlot->handler(event, pSlot->pContext);
    }
}

static uint32_t prvUARTConvertVendorEvent(uint32_t vendorEvent)
{
    uint32_t event = 0U;
    const uint32_t vendorErrorEvents =
        (uint32_t)UART_EVENT_LINE_ERROR |
        (uint32_t)UART_EVENT_BREAK_ERROR |
        (uint32_t)UART_EVENT_FRAME_ERROR |
        (uint32_t)UART_EVENT_PARITY_ERROR |
        (uint32_t)UART_EVENT_OVERFLOW_ERROR;

    if ((vendorEvent & (uint32_t)UART_EVENT_TX_DONE) != 0U)
    {
        event |= TIREMO_HAL_UART_EVENT_TX_DONE;
    }

    if ((vendorEvent & (uint32_t)UART_EVENT_RX_DONE) != 0U)
    {
        event |= TIREMO_HAL_UART_EVENT_RX_DONE;
    }

    if ((vendorEvent & vendorErrorEvents) != 0U)
    {
        event |= TIREMO_HAL_UART_EVENT_LINE_ERROR;
    }

    return event;
}
