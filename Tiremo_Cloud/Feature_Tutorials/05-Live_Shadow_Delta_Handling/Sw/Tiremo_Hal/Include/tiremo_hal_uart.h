/******************************************************************************
 * @file    tiremo_hal_uart.h
 * @brief   Tiremo UART hardware abstraction API.
 *
 * Provides a vendor-independent UART interface for Tiremo common modules.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_HAL_UART_H
#define TIREMO_HAL_UART_H

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
 * Public Macros
 *****************************************************************************/

#define TIREMO_HAL_UART_EVENT_TX_DONE       (1UL << 0U)
#define TIREMO_HAL_UART_EVENT_RX_DONE       (1UL << 1U)
#define TIREMO_HAL_UART_EVENT_LINE_ERROR    (1UL << 2U)

/******************************************************************************
 * Public Types
 *****************************************************************************/

typedef enum
{
    TIREMO_HAL_UART_PORT_0 = 0,
    TIREMO_HAL_UART_PORT_1,
    TIREMO_HAL_UART_PORT_2,
    TIREMO_HAL_UART_PORT_MAX
} TIREMO_HAL_UART_Port_t;

typedef enum
{
    TIREMO_HAL_UART_STATUS_OK = 0,
    TIREMO_HAL_UART_STATUS_ERROR
} TIREMO_HAL_UART_Status_t;

typedef void (*TIREMO_HAL_UART_IrqHandler_t)(uint32_t event, void *pContext);

/******************************************************************************
 * Public Function Prototypes
 *****************************************************************************/

/**
 * @brief Configure interrupt-driven operation for a UART port.
 *
 * @param[in] port      Target UART port.
 * @param[in] handler   Function invoked for UART interrupt events.
 * @param[in] pContext  User context forwarded to the interrupt handler.
 * @param[in] priority  Interrupt priority.
 *
 * @return UART operation status.
 *
 * @retval TIREMO_HAL_UART_STATUS_OK     Interrupt operation was configured.
 * @retval TIREMO_HAL_UART_STATUS_ERROR  Invalid argument or vendor HAL failure.
 */
TIREMO_HAL_UART_Status_t TIREMO_HAL_UART_SetInterruptHandler(
    TIREMO_HAL_UART_Port_t port,
    TIREMO_HAL_UART_IrqHandler_t handler,
    void *pContext,
    uint32_t priority);

/**
 * @brief Transmit data through a UART port.
 *
 * @param[in] port      Target UART port.
 * @param[in] pData     Data to transmit.
 * @param[in] length    Number of bytes to transmit.
 * @param[in] isPolling true to use blocking polling mode.
 *
 * @return UART operation status.
 *
 * @retval TIREMO_HAL_UART_STATUS_OK     Transmission started or completed.
 * @retval TIREMO_HAL_UART_STATUS_ERROR  Invalid argument or vendor HAL failure.
 */
TIREMO_HAL_UART_Status_t TIREMO_HAL_UART_Transmit(TIREMO_HAL_UART_Port_t port,
                                                  const uint8_t *pData,
                                                  uint32_t length,
                                                  bool isPolling);

/**
 * @brief Receive data through a UART port.
 *
 * @param[in]  port      Target UART port.
 * @param[out] pData     Destination buffer.
 * @param[in]  length    Number of bytes to receive.
 * @param[in]  isPolling true to use blocking polling mode.
 *
 * @return UART operation status.
 *
 * @retval TIREMO_HAL_UART_STATUS_OK     Reception started or completed.
 * @retval TIREMO_HAL_UART_STATUS_ERROR  Invalid argument or vendor HAL failure.
 */
TIREMO_HAL_UART_Status_t TIREMO_HAL_UART_Receive(TIREMO_HAL_UART_Port_t port,
                                                 uint8_t *pData,
                                                 uint32_t length,
                                                 bool isPolling);

/**
 * @brief Abort active UART transfers.
 *
 * @param[in] port  Target UART port.
 *
 * @return UART operation status.
 *
 * @retval TIREMO_HAL_UART_STATUS_OK     Active transfers were aborted.
 * @retval TIREMO_HAL_UART_STATUS_ERROR  Invalid port or vendor HAL failure.
 */
TIREMO_HAL_UART_Status_t TIREMO_HAL_UART_Abort(TIREMO_HAL_UART_Port_t port);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_HAL_UART_H */
