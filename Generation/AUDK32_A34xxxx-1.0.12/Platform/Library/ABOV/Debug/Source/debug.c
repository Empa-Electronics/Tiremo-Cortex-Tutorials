/**
 *******************************************************************************
 * @file        debug.c
 * @author      ABOV R&D Division
 * @brief       Common debug APIs for S/W
 *
 * Copyright 2022 ABOV Semiconductor Co.,Ltd. All rights reserved.
 *
 * This file is licensed under terms that are found in the LICENSE file
 * located at Document directory.
 * If this file is delivered or shared without applicable license terms,
 * the terms of the BSD-3-Clause license shall be applied.
 * Reference: https://opensource.org/licenses/BSD-3-Clause
 ******************************************************************************/

#include <stdint.h>
#include <stddef.h>

#include "debug.h"
#include "debug_serial.h"
#include "debug_log.h"
#if defined(CONFIG_DEBUG_MODULE_UART)
#include "hal_uart.h"
#elif defined(CONFIG_DEBUG_MODULE_USART)
#include "hal_usart.h"
#endif

#if (CONFIG_DEBUG == 1)

#if defined(CONFIG_DEBUG_MODULE_UART) || defined(CONFIG_DEBUG_MODULE_USART)
static void dbg_uart_irq_handler(uint32_t event, void *context)
{
#if defined(CONFIG_DEBUG_MODULE_UART)
    if (event & UART_EVENT_RX_DONE)
#elif defined(CONFIG_DEBUG_MODULE_USART)
    if (event & USART_EVENT_RX_DONE)
#endif
    {
        serial_continue_rx();
    }
#if defined(CONFIG_DEBUG_MODULE_UART)
    if (event & UART_EVENT_TX_DONE)
#elif defined(CONFIG_DEBUG_MODULE_USART)
    if (event & USART_EVENT_TX_DONE)
#endif
    {
        serial_continue_tx();
    }
}
#endif

static void dbg_serial_init(void)
{
#if defined(CONFIG_DEBUG_MODULE_UART)
    UART_CFG_t stCfg =
    {
        .un32BaudRate = APP_UART_BAUD,
        .eData      = UART_DATA_8,
        .eParity    = UART_PARITY_NONE,
        .eStop      = UART_STOP_1
    };

#if (DEBUG_UART_CLK == 1)
    UART_CLK_CFG_t stClkCfg =
    {
        .eClk = (UART_CLK_e)DEBUG_UART_CLK,
        .eMccr = (UART_CLK_MCCR_e)DEBUG_UART_CLK_MCCR,
        .un8MccrDiv = DEBUG_UART_CLK_MCCR_DIV
    };
#endif

    HAL_UART_Init((UART_ID_e)DEBUG_UART_ID);
#if (DEBUG_UART_CLK == 1)
    HAL_UART_SetClkConfig((UART_ID_e)DEBUG_UART_ID, &stClkCfg);
#endif
    HAL_UART_SetConfig((UART_ID_e)DEBUG_UART_ID, &stCfg);
    HAL_UART_SetIRQ((UART_ID_e)DEBUG_UART_ID, UART_OPS_INTR, dbg_uart_irq_handler, NULL, DRIVER_PRIORITY_LOWEST);
#elif defined(CONFIG_DEBUG_MODULE_USART)
    USART_CFG_t stCfg =
    {
        .eMode                 = USART_MODE_UART,
        .un32BaudRate          = APP_UART_BAUD,
        .tCfg.tUart.eData      = USART_DATA_8,
        .tCfg.tUart.eParity    = USART_PARITY_NONE,
        .tCfg.tUart.eStop      = USART_STOP_1
    };

    HAL_USART_Init((USART_ID_e)DEBUG_UART_ID);
    HAL_USART_SetConfig((USART_ID_e)DEBUG_UART_ID, &stCfg);
    HAL_USART_SetIRQ((USART_ID_e)DEBUG_UART_ID, USART_OPS_INTR, dbg_uart_irq_handler, NULL, DRIVER_PRIORITY_LOWEST);
#endif

}

static void dbg_serial_reinit(uint16_t baudrate)
{
#if defined(CONFIG_DEBUG_MODULE_UART)
    UART_CFG_t stCfg =
    {
        .un32BaudRate = APP_UART_BAUD,
        .eData      = UART_DATA_8,
        .eParity    = UART_PARITY_NONE,
        .eStop      = UART_STOP_1
    };

#if (DEBUG_UART_CLK == 1)
    UART_CLK_CFG_t stClkCfg =
    {
        .eClk = (UART_CLK_e)DEBUG_UART_CLK,
        .eMccr = (UART_CLK_MCCR_e)DEBUG_UART_CLK_MCCR,
        .un8MccrDiv = DEBUG_UART_CLK_MCCR_DIV
    };
#endif

    if(baudrate != 0)
    {
        stCfg.un32BaudRate = baudrate;
    }

#if (DEBUG_UART_CLK == 1)
    HAL_UART_SetClkConfig((UART_ID_e)DEBUG_UART_ID, &stClkCfg);
#endif
    HAL_UART_SetConfig((UART_ID_e)DEBUG_UART_ID, &stCfg);
#elif defined(CONFIG_DEBUG_MODULE_USART)
    USART_CFG_t stCfg =
    {
        .eMode                 = USART_MODE_UART,
        .un32BaudRate          = APP_UART_BAUD,
        .tCfg.tUart.eData      = USART_DATA_8,
        .tCfg.tUart.eParity    = USART_PARITY_NONE,
        .tCfg.tUart.eStop      = USART_STOP_1
    };

    if(baudrate != 0)
    {
        stCfg.un32BaudRate = baudrate;
    }

    HAL_USART_SetIRQ((USART_ID_e)DEBUG_UART_ID, USART_OPS_POLL, dbg_uart_irq_handler, NULL, DRIVER_PRIORITY_LOWEST);
    HAL_USART_SetConfig((USART_ID_e)DEBUG_UART_ID, &stCfg);
    HAL_USART_SetIRQ((USART_ID_e)DEBUG_UART_ID, USART_OPS_INTR, dbg_uart_irq_handler, NULL, DRIVER_PRIORITY_LOWEST);
#endif

}

static void dbg_serial_abort(void)
{
#if defined(CONFIG_DEBUG_MODULE_UART)
    HAL_UART_Abort((UART_ID_e)DEBUG_UART_ID);
#elif defined(CONFIG_DEBUG_MODULE_USART)
    HAL_USART_Abort((USART_ID_e)DEBUG_UART_ID);
#endif
}

#if defined(CONFIG_DEBUG_MODULE_USART)
static void dbg_serial_abortRx(void)
{
    HAL_USART_AbortRx((USART_ID_e)DEBUG_UART_ID);
}
#endif

static void dbg_serial_rx(uint8_t *ch)
{
#if defined(CONFIG_DEBUG_MODULE_UART)
    HAL_UART_Receive((UART_ID_e)DEBUG_UART_ID, ch, 1, false);
#elif defined(CONFIG_DEBUG_MODULE_USART)
    HAL_USART_Receive((USART_ID_e)DEBUG_UART_ID, ch, 1, false);
#endif
}

static void dbg_serial_rx_poll(uint8_t *ch)
{
#if defined(CONFIG_DEBUG_MODULE_UART)
    HAL_UART_Receive((UART_ID_e)DEBUG_UART_ID, ch, 1, true);
#elif defined(CONFIG_DEBUG_MODULE_USART)
    HAL_USART_Receive((USART_ID_e)DEBUG_UART_ID, ch, 1, true);
#endif
}

static uint32_t dbg_serial_tx(uint8_t *ch)
{
#if defined(CONFIG_DEBUG_MODULE_UART)
    return (uint32_t)HAL_UART_Transmit((UART_ID_e)DEBUG_UART_ID, ch, 1, false);
#elif defined(CONFIG_DEBUG_MODULE_USART)
    return (uint32_t)HAL_USART_Transmit((USART_ID_e)DEBUG_UART_ID, ch, 1, false);
#else
    return 0;
#endif
}

static void dbg_serial_tx_poll(uint8_t *ch)
{
#if defined(CONFIG_DEBUG_MODULE_UART)
    HAL_UART_Transmit((UART_ID_e)DEBUG_UART_ID, ch, 1, true);
#elif defined(CONFIG_DEBUG_MODULE_USART)
    HAL_USART_Transmit((USART_ID_e)DEBUG_UART_ID, ch, 1, true);
#endif
}

struct serial_op s_op =
{
    .init = dbg_serial_init,
    .abort = dbg_serial_abort,
    .rx = dbg_serial_rx,
    .rx_poll = dbg_serial_rx_poll,
    .tx = dbg_serial_tx,
    .tx_poll = dbg_serial_tx_poll,
};

void Debug_Init(void)
{
    serial_init(&s_op);
}

void Debug_Reinit(uint16_t baudrate)
{
    __disable_irq();
#if defined(CONFIG_DEBUG_MODULE_UART)
    dbg_serial_abort();
#elif defined(CONFIG_DEBUG_MODULE_USART)
    dbg_serial_abort();
    dbg_serial_abortRx();
#endif
    dbg_serial_reinit(baudrate);
    __enable_irq();
}

void Assert(char *file, uint32_t line)
{
    LOG("ASSERT %s at %d\n", file, line);
    while (1)
    {
    }
}

#endif /* CONFIG_DEBUG */
