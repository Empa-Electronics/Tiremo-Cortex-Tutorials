/******************************************************************************
 * @file    tiremo_hal_i2c.h
 * @brief   Tiremo I2C hardware abstraction API.
 *
 * Provides a vendor-independent I2C interface over the ABOV I2C HAL.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_HAL_I2C_H
#define TIREMO_HAL_I2C_H

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
    TIREMO_HAL_I2C_BUS_0 = 0,
    TIREMO_HAL_I2C_BUS_1,
    TIREMO_HAL_I2C_BUS_2,
    TIREMO_HAL_I2C_BUS_3,
    TIREMO_HAL_I2C_BUS_MAX
} TIREMO_HAL_I2C_Bus_t;

typedef enum
{
    TIREMO_HAL_I2C_STATUS_OK = 0,
    TIREMO_HAL_I2C_STATUS_ERROR
} TIREMO_HAL_I2C_Status_t;

/******************************************************************************
 * Public Function Prototypes
 *****************************************************************************/

/**
 * @brief Transmit data to an I2C slave.
 *
 * @param[in] bus       Target I2C bus.
 * @param[in] address   7-bit slave address.
 * @param[in] pData     Data to transmit.
 * @param[in] length    Number of bytes to transmit.
 * @param[in] isPolling true to force blocking polling mode.
 *
 * @return I2C operation status.
 *
 * @retval TIREMO_HAL_I2C_STATUS_OK     Transmission completed.
 * @retval TIREMO_HAL_I2C_STATUS_ERROR  Invalid argument or vendor HAL failure.
 *
 * @warning Call PRV_I2C_Init() before using this function.
 */
TIREMO_HAL_I2C_Status_t TIREMO_HAL_I2C_Transmit(TIREMO_HAL_I2C_Bus_t bus,
                                                 uint8_t address,
                                                 const uint8_t *pData,
                                                 uint32_t length,
                                                 bool isPolling);

/**
 * @brief Receive data from an I2C slave.
 *
 * @param[in]  bus       Target I2C bus.
 * @param[in]  address   7-bit slave address.
 * @param[out] pData     Destination buffer.
 * @param[in]  length    Number of bytes to receive.
 * @param[in]  isPolling true to force blocking polling mode.
 *
 * @return I2C operation status.
 *
 * @retval TIREMO_HAL_I2C_STATUS_OK     Reception completed.
 * @retval TIREMO_HAL_I2C_STATUS_ERROR  Invalid argument or vendor HAL failure.
 *
 * @warning Call PRV_I2C_Init() before using this function.
 */
TIREMO_HAL_I2C_Status_t TIREMO_HAL_I2C_Receive(TIREMO_HAL_I2C_Bus_t bus,
                                                uint8_t address,
                                                uint8_t *pData,
                                                uint32_t length,
                                                bool isPolling);

/**
 * @brief Write then read from an I2C slave using a repeated START.
 *
 * Sends the write payload (typically a register sub-address), issues a
 * repeated START instead of a STOP, and clocks in the requested bytes. The
 * last received byte is NACKed before the closing STOP.
 *
 * @param[in]  bus       Target I2C bus.
 * @param[in]  address   7-bit slave address.
 * @param[in]  pTxData   Write payload.
 * @param[in]  txLength  Number of bytes to write.
 * @param[out] pRxData   Destination buffer.
 * @param[in]  rxLength  Number of bytes to read.
 *
 * @return I2C operation status.
 *
 * @retval TIREMO_HAL_I2C_STATUS_OK     Transaction completed.
 * @retval TIREMO_HAL_I2C_STATUS_ERROR  Invalid argument, NACK, or bus timeout.
 *
 * @note Required by slaves such as the LIS2DE12 that do not accept a STOP
 *       between the sub-address write and the data read.
 *
 * @warning This function is blocking and always runs in polling mode.
 * @warning Call PRV_I2C_Init() before using this function.
 */
TIREMO_HAL_I2C_Status_t TIREMO_HAL_I2C_WriteRead(TIREMO_HAL_I2C_Bus_t bus,
                                                  uint8_t address,
                                                  const uint8_t *pTxData,
                                                  uint32_t txLength,
                                                  uint8_t *pRxData,
                                                  uint32_t rxLength);

/**
 * @brief Release a bus left stuck by a slave after an MCU reset.
 *
 * Issues a burst of stop conditions so a slave still driving SDA low returns
 * the line to idle.
 *
 * @param[in] bus  Target I2C bus.
 */
void TIREMO_HAL_I2C_PrepareBus(TIREMO_HAL_I2C_Bus_t bus);

/**
 * @brief Re-initialize the I2C peripheral after repeated transfer failures.
 *
 * Releases the bus, restarts the vendor peripheral, and restores the default
 * master configuration.
 *
 * @param[in] bus  Target I2C bus.
 */
void TIREMO_HAL_I2C_Recover(TIREMO_HAL_I2C_Bus_t bus);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_HAL_I2C_H */
