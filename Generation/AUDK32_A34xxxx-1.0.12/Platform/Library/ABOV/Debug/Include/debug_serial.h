/**
 *******************************************************************************
 * @file        debug_serial.h
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

/**
 * @addtogroup  Framework
 * @{
 * @defgroup    Debug Debug
 * @{
 * @defgroup    Serial Serial
 * @{
 * @brief       Serial is composed of API related to physical serial driver.
 */

#ifndef _DEBUG_SERIAL_H_
#define _DEBUG_SERIAL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <string.h>

/**
 * @brief Prototype for debug initialization function
 */
typedef void (*serial_op_init_t)(void);

/**
 * @brief Prototype for debug abort function 
 */
typedef void (*serial_op_abort_t)(void);

/**
 * @brief Prototype for debug rx non-blocking function 
 */
typedef void (*serial_op_rx_t)(uint8_t *ch);

/**
 * @brief Prototype for debug rx blocking function 
 */
typedef void (*serial_op_rx_poll_t)(uint8_t *ch);

/**
 * @brief Prototype for debug tx non-blocking function 
 */
typedef uint32_t (*serial_op_tx_t)(uint8_t *ch);

/**
 * @brief Prototype for debug tx blocking function 
 */
typedef void (*serial_op_tx_poll_t)(uint8_t *ch);

/**
 * @brief Serial operation for debugging
 */
struct serial_op
{
    serial_op_init_t    init;           /**< initialization function */
    serial_op_abort_t   abort;          /**< abort function */
    serial_op_rx_t      rx;             /**< rx non-blocking function */
    serial_op_rx_poll_t rx_poll;        /**< rx blocking function */
    serial_op_tx_t      tx;             /**< tx non-blocking function */
    serial_op_tx_poll_t tx_poll;        /**< tx blocking function */
};

#if (CONFIG_DEBUG == 1)

/**
 *******************************************************************************
 * @brief       This function initializes the serial port with fifo
 * @param[in]   *op : serial operation
 * @return      int : a non-zero on error, otherwise 0
 ******************************************************************************/
int serial_init(struct serial_op *op);

/**
 *******************************************************************************
 * @brief       This function puts a character to the fifo, which will be
 *              transmitted automatically
 * @param[in]   c : character to put
 * @return      int : character written as int type
 ******************************************************************************/
int serial_putc(char c);

/**
 *******************************************************************************
 * @brief       This function puts a character, which will be
 *              transmitted automatically. \n
                If transmit error, abort Tx and retry it, forcily.
 * @param[in]   c : character to put
 * @return      int : character written as int type
 ******************************************************************************/
int serial_abort_putc(char c);

/**
 *******************************************************************************
 * @brief       This function gets a character from the fifo.
 *              If receive callback is specified, the function returns EOF always,
 *              and callback is invoked when a character is received.
 * @param[in]   *rx_cb : receive callback function.
 * @return      int : character received or EOF
 ******************************************************************************/
int serial_getc(void (*rx_cb)(uint8_t));

/**
 *******************************************************************************
 * @brief       This function continues transmission
 * @return      void : None
 ******************************************************************************/
void serial_continue_tx(void);

/**
 *******************************************************************************
 * @brief       This function continues reception
 * @return      void : None
 ******************************************************************************/
void serial_continue_rx(void);

#else

#define serial_init(...)
#define serial_putc(...)
#define serial_getc(...)
#define serial_continue_tx(...)
#define serial_continue_rx(...)

#endif

#ifdef __cplusplus
}
#endif

#endif /* _DEBUG_SERIAL_H_ */

/** @} */
/** @} */
/** @} */
