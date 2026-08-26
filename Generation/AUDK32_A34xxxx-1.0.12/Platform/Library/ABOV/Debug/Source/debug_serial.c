/**
 *******************************************************************************
 * @file        debug_serial.c
 * @author      ABOV R&D Division
 * @brief       Serial API for debugging
 *
 * Copyright 2022 ABOV Semiconductor Co.,Ltd. All rights reserved.
 *
 * This file is licensed under terms that are found in the LICENSE file
 * located at Document directory.
 * If this file is delivered or shared without applicable license terms,
 * the terms of the BSD-3-Clause license shall be applied.
 * Reference: https://opensource.org/licenses/BSD-3-Clause
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <cmsis_compiler.h>

#include "debug.h"
#include "debug_serial.h"

#if (CONFIG_DEBUG == 1)

static uint8_t serial_tx;
static uint8_t serial_rx;
static void (*serial_rx_cb)(uint8_t);

static struct serial_op *s_op;

#if (CONFIG_DEBUG_SERIAL_FIFO == 1)

#if !defined(CONFIG_DEBUG_SERIAL_RX_FIFO_SIZE)
#define CONFIG_DEBUG_SERIAL_RX_FIFO_SIZE    16
#endif
#if !defined(CONFIG_DEBUG_SERIAL_TX_FIFO_SIZE)
#define CONFIG_DEBUG_SERIAL_TX_FIFO_SIZE          512
#define CONFIG_DEBUG_SERIAL_TX_FIFO_FLUSH_SIZE    256
#endif

struct serial_fifo
{
    uint8_t *           data;
    uint16_t            mask;
    volatile uint8_t    active;
    volatile uint32_t   in_pos;
    volatile uint32_t   out_pos;
};

static uint8_t rx_fifo_buf[CONFIG_DEBUG_SERIAL_RX_FIFO_SIZE];
static uint8_t tx_fifo_buf[CONFIG_DEBUG_SERIAL_TX_FIFO_SIZE];
static struct serial_fifo rx_fifo;
static struct serial_fifo tx_fifo;

static void serial_fifo_init(struct serial_fifo *fifo, uint8_t *data, uint32_t len)
{
    fifo->data = data;
    fifo->active = 0;
    fifo->mask = len - 1;
    fifo->in_pos = 0;
    fifo->out_pos = 0;
}

/* check if fifo is empty */
static __INLINE uint8_t serial_fifo_empty(struct serial_fifo *fifo)
{
    uint32_t in_pos;
    uint32_t out_pos;

    in_pos = fifo->in_pos;
    out_pos = fifo->out_pos;

    return (in_pos == out_pos) ? 1 : 0;
}

/* check fifo is active */
static __INLINE uint8_t serial_fifo_is_active(struct serial_fifo *fifo)
{
    return fifo->active;
}

/* set fifo active */
static __INLINE void serial_fifo_set_active(struct serial_fifo *fifo, uint8_t active)
{
    fifo->active = active;
}

/* put fifo: check for fifo available before call to this function */
static __INLINE void serial_fifo_put(struct serial_fifo *fifo, uint8_t c)
{
    fifo->data[fifo->in_pos & fifo->mask] = c;
    fifo->in_pos++;
}

/* get fifo: check for fifo not empty before call to this function */
static __INLINE void serial_fifo_get(struct serial_fifo *fifo, uint8_t *c)
{
    *c = fifo->data[fifo->out_pos & fifo->mask];
    fifo->out_pos++;
}

/* get fifo: check for fifo not empty before call to this function */
static __INLINE uint16_t serial_fifo_get_gap(struct serial_fifo *fifo)
{
   uint32_t in_pos;
   uint32_t out_pos;
   uint16_t mask;

   mask = fifo->mask;
   in_pos = (fifo->in_pos & mask);
   out_pos = (fifo->out_pos & mask);

   if(in_pos < out_pos)
   {
       out_pos = CONFIG_DEBUG_SERIAL_TX_FIFO_SIZE - out_pos;
       return (out_pos + in_pos);
   }
   else
   {
       return (in_pos - out_pos);
   }
}

static void serial_fifo_tx_flush(void)
{
#if (CONFIG_DEBUG_SERIAL_FIFO == 1)

    __disable_irq();

    uint16_t diff = serial_fifo_get_gap(&tx_fifo);
    for(int i = 0; i < diff; i++)
    { 
         serial_fifo_get(&tx_fifo, &serial_tx);
         s_op->tx_poll(&serial_tx);
    }
    serial_fifo_set_active(&tx_fifo, 0);

    __enable_irq();
#endif
}
#endif

void serial_continue_rx(void)
{
    if (serial_rx == 0x08)
    {
        serial_putc(serial_rx);
        serial_putc(' ');
        serial_putc(0x08);
    }
    else
    {
#if defined(CONFIG_DEBUG_MODULE_USART)
        serial_abort_putc(serial_rx);
#else
        serial_putc(serial_rx);
#endif
    }

    if (serial_rx_cb)
    {
        serial_rx_cb(serial_rx);
    }
    else
    {
#if (CONFIG_DEBUG_SERIAL_FIFO == 1)
        serial_fifo_put(&rx_fifo, serial_rx);
        s_op->rx(&serial_rx);
#endif
    }
}

void serial_continue_tx(void)
{
#if (CONFIG_DEBUG_SERIAL_FIFO == 1)
    uint16_t diff = 0;
    if (serial_fifo_empty(&tx_fifo))
    {
        serial_fifo_set_active(&tx_fifo, 0);
    }
    else
    {
      
        /* check in/out fifo position gap */
        diff = serial_fifo_get_gap(&tx_fifo);
        if(diff < CONFIG_DEBUG_SERIAL_TX_FIFO_FLUSH_SIZE)
        {
            serial_fifo_get(&tx_fifo, &serial_tx);
            (void)s_op->tx(&serial_tx);
        }
        else
        {
            /* flushing tx fifo data by polling mode instead of isr */
            serial_fifo_tx_flush();
        }
    }
#endif
}

int serial_init(struct serial_op *op)
{
    s_op = op;

    s_op->init();

#if (CONFIG_DEBUG_SERIAL_FIFO == 1)
    serial_fifo_init(&rx_fifo, rx_fifo_buf, sizeof(rx_fifo_buf));
    serial_fifo_init(&tx_fifo, tx_fifo_buf, sizeof(tx_fifo_buf));
#endif

#if (__GNUC__)
    setvbuf(stdout, NULL, _IONBF, 0);
#endif

    return 0;
}

int serial_abort_putc(char c)
{
#if (CONFIG_DEBUG_SERIAL_FIFO == 1)
    int32_t ret = 0;

    __disable_irq();

    serial_fifo_set_active(&tx_fifo, 1);
    serial_tx = c;
    ret = s_op->tx(&serial_tx);
    if(ret != 0)
    {
        s_op->abort();
        s_op->tx(&serial_tx);
    }
    __enable_irq();

#else
    serial_tx = c;
    s_op->tx_poll(&serial_tx);
#endif

    return c;
}

int serial_putc(char c)
{
#if (CONFIG_DEBUG_SERIAL_FIFO == 1)
    int32_t ret = 0;

    __disable_irq();

    if (serial_fifo_is_active(&tx_fifo) == 0)
    {
        /* send directly if fifo is not active */
        serial_fifo_set_active(&tx_fifo, 1);
        serial_tx = c;
        ret = s_op->tx(&serial_tx);
        if(ret != 0)
        {
            serial_fifo_put(&tx_fifo, c);
        }
    }
    else
    {
        serial_fifo_put(&tx_fifo, c);
    }

    __enable_irq();

#else
    serial_tx = c;
    s_op->tx_poll(&serial_tx);
#endif

    return c;
}

int serial_getc(void (*rx_cb)(uint8_t))
{
    serial_rx_cb = rx_cb;

    if (serial_rx_cb)
    {
        /* use callback if it is set */
        s_op->rx(&serial_rx);
    }
    else
    {
#if (CONFIG_DEBUG_SERIAL_FIFO == 1)
        /* callback is not set, return synchronously */
        if (serial_fifo_is_active(&rx_fifo) == 0)
        {
            serial_fifo_set_active(&rx_fifo, 1);
            s_op->rx(&serial_rx);
        }

        if (serial_fifo_empty(&rx_fifo) == 0)
        {
            uint8_t c;
            serial_fifo_get(&rx_fifo, &c);
            return c;
        }
#endif
    }

    return EOF;
}

void serial_hardfault_abort(char c)
{
    s_op->abort();
}

void serial_hardfault_putc(char c)
{
    s_op->tx_poll((uint8_t *)&c);
}

#endif /* CONFIG_DEBUG */

