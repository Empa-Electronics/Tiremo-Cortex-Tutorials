/******************************************************************************
 * @file    tiremo_hal_i2c.c
 * @brief   Tiremo I2C hardware abstraction implementation.
 *
 * Adapts the Tiremo I2C API to the ABOV I2C HAL and provides a register level
 * repeated-START transaction for slaves that do not tolerate a stop between
 * the sub-address write and the data read.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "../Include/tiremo_hal_i2c.h"

#include "hal_i2c.h"

#include "hal_i2c_prv.h"

#include <stddef.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/** @brief Spin count used while waiting for an I2C interrupt flag. */
#define TIREMO_HAL_I2C_POLL_TIMEOUT     (1000000UL)

/** @brief Returned by prvI2CGetOpStatus() when the bus stops responding. */
#define TIREMO_HAL_I2C_OPSTATUS_TIMEOUT (0xFFFFFFFFUL)

/** @brief Upper bound on state machine iterations for one transaction. */
#define TIREMO_HAL_I2C_XFER_GUARD       (TIREMO_HAL_I2C_POLL_TIMEOUT)

/** @brief Consecutive failures before the peripheral is fully re-initialized. */
#define TIREMO_HAL_I2C_FAIL_LIMIT       (2U)

/** @brief Stop pulses issued to release a slave that holds SDA low. */
#define TIREMO_HAL_I2C_RELEASE_PULSES   (9U)

/** @brief Bus clock applied when the peripheral is re-initialized. */
#define TIREMO_HAL_I2C_RECOVER_FREQ_HZ  (100000UL)

/** @brief SDA hold time applied when the peripheral is re-initialized. */
#define TIREMO_HAL_I2C_RECOVER_SDA_HOLD (50U)

/** @brief IRQ priority applied when the peripheral is re-initialized. */
#define TIREMO_HAL_I2C_RECOVER_IRQ_PRIO (3U)

/******************************************************************************
 * Private Variables
 *****************************************************************************/

static uint8_t s_failStreak = 0U;

/******************************************************************************
 * Private Function Prototypes
 *****************************************************************************/

static bool prvI2CIsBusValid(TIREMO_HAL_I2C_Bus_t bus);
static uint32_t prvI2CGetOpStatus(I2C_Type *ptI2c);
static void prvI2CSetOpStop(I2C_Type *ptI2c);
static void prvI2CFlushFlags(I2C_Type *ptI2c);
static void prvI2CBusBegin(TIREMO_HAL_I2C_Bus_t bus, I2C_Type *ptI2c);
static void prvI2CBusEnd(TIREMO_HAL_I2C_Bus_t bus, I2C_Type *ptI2c);
static void prvI2CApplyConfig(TIREMO_HAL_I2C_Bus_t bus);
static void prvI2COnError(TIREMO_HAL_I2C_Bus_t bus);

/******************************************************************************
 * Public Functions
 *****************************************************************************/

TIREMO_HAL_I2C_Status_t TIREMO_HAL_I2C_Transmit(TIREMO_HAL_I2C_Bus_t bus,
                                                 uint8_t address,
                                                 const uint8_t *pData,
                                                 uint32_t length,
                                                 bool isPolling)
{
    HAL_ERR_e vendorStatus;

    if ((!prvI2CIsBusValid(bus)) || (pData == NULL) || (length == 0U))
    {
        return TIREMO_HAL_I2C_STATUS_ERROR;
    }

    vendorStatus = HAL_I2C_Transmit((I2C_ID_e)bus,
                                    address,
                                    (uint8_t *)pData,
                                    length,
                                    isPolling);

    return (vendorStatus == HAL_ERR_OK) ? TIREMO_HAL_I2C_STATUS_OK
                                        : TIREMO_HAL_I2C_STATUS_ERROR;
}

TIREMO_HAL_I2C_Status_t TIREMO_HAL_I2C_Receive(TIREMO_HAL_I2C_Bus_t bus,
                                                uint8_t address,
                                                uint8_t *pData,
                                                uint32_t length,
                                                bool isPolling)
{
    HAL_ERR_e vendorStatus;

    if ((!prvI2CIsBusValid(bus)) || (pData == NULL) || (length == 0U))
    {
        return TIREMO_HAL_I2C_STATUS_ERROR;
    }

    vendorStatus = HAL_I2C_Receive((I2C_ID_e)bus,
                                   address,
                                   pData,
                                   length,
                                   isPolling);

    return (vendorStatus == HAL_ERR_OK) ? TIREMO_HAL_I2C_STATUS_OK
                                        : TIREMO_HAL_I2C_STATUS_ERROR;
}

TIREMO_HAL_I2C_Status_t TIREMO_HAL_I2C_WriteRead(TIREMO_HAL_I2C_Bus_t bus,
                                                  uint8_t address,
                                                  const uint8_t *pTxData,
                                                  uint32_t txLength,
                                                  uint8_t *pRxData,
                                                  uint32_t rxLength)
{
    I2C_Type *ptI2c;
    uint32_t status;
    uint32_t guard = TIREMO_HAL_I2C_XFER_GUARD;
    uint32_t txCount = 0U;
    uint32_t rxCount = 0U;
    uint8_t phase = 0U;
    TIREMO_HAL_I2C_Status_t result = TIREMO_HAL_I2C_STATUS_ERROR;
    bool isDone = false;

    if (!prvI2CIsBusValid(bus))
    {
        return TIREMO_HAL_I2C_STATUS_ERROR;
    }

    if ((txLength == 0U) && (rxLength == 0U))
    {
        return TIREMO_HAL_I2C_STATUS_OK;
    }

    if (((txLength > 0U) && (pTxData == NULL)) ||
        ((rxLength > 0U) && (pRxData == NULL)))
    {
        return TIREMO_HAL_I2C_STATUS_ERROR;
    }

    ptI2c = I2C_GetReg((P_I2C_ID_e)bus);
    prvI2CBusBegin(bus, ptI2c);

    SET_I2C_CR_ACK(ptI2c, true);
    SET_I2C_DR_TX(ptI2c, (uint8_t)(address << 1));
    SET_I2C_CR_START(ptI2c, true);

    while ((!isDone) && (guard > 0U))
    {
        guard--;

        status = prvI2CGetOpStatus(ptI2c);
        if (status == TIREMO_HAL_I2C_OPSTATUS_TIMEOUT)
        {
            break;
        }

        if (phase == 0U)
        {
            if ((status != I2C_MASTER_TX_ADDR_ACK) &&
                (status != I2C_TX_DATA_ACK))
            {
                break;
            }

            if (txCount < txLength)
            {
                SET_I2C_DR_TX(ptI2c, pTxData[txCount]);
                txCount++;
                SET_I2C_SR_CLEAR(ptI2c);
            }
            else if (rxLength > 0U)
            {
                /* Restart into read mode without releasing the bus. */
                phase = 1U;
                SET_I2C_CR_ACK(ptI2c, true);
                SET_I2C_DR_TX(ptI2c, (uint8_t)((address << 1) | 0x01U));
                SET_I2C_CR_START(ptI2c, true);
                SET_I2C_SR_CLEAR(ptI2c);
            }
            else
            {
                result = TIREMO_HAL_I2C_STATUS_OK;
                isDone = true;
            }
        }
        else if (status == I2C_MASTER_RX_ADDR_ACK)
        {
            SET_I2C_SR_CLEAR(ptI2c);
        }
        else if (status == I2C_RX_DATA_ACK)
        {
            if (rxCount >= rxLength)
            {
                result = TIREMO_HAL_I2C_STATUS_OK;
                isDone = true;
            }
            else
            {
                pRxData[rxCount] = (uint8_t)GET_I2C_DR_RX(ptI2c);
                rxCount++;

                if (rxCount == rxLength)
                {
                    SET_I2C_CR_ACK(ptI2c, false);
                }
                else
                {
                    SET_I2C_SR_CLEAR(ptI2c);
                }
            }
        }
        else if (status == I2C_MASTER_RX_DATA_NOACK)
        {
            if (rxCount == rxLength)
            {
                result = TIREMO_HAL_I2C_STATUS_OK;
            }

            isDone = true;
        }
        else
        {
            break;
        }
    }

    prvI2CSetOpStop(ptI2c);
    SET_I2C_SR_CLEAR(ptI2c);
    prvI2CBusEnd(bus, ptI2c);

    if (result == TIREMO_HAL_I2C_STATUS_OK)
    {
        s_failStreak = 0U;
    }
    else
    {
        prvI2COnError(bus);
    }

    return result;
}

void TIREMO_HAL_I2C_PrepareBus(TIREMO_HAL_I2C_Bus_t bus)
{
    I2C_Type *ptI2c;
    uint32_t index;

    if (!prvI2CIsBusValid(bus))
    {
        return;
    }

    ptI2c = I2C_GetReg((P_I2C_ID_e)bus);
    prvI2CBusBegin(bus, ptI2c);

    for (index = 0U; index < TIREMO_HAL_I2C_RELEASE_PULSES; index++)
    {
        prvI2CSetOpStop(ptI2c);
        SET_I2C_SR_CLEAR(ptI2c);
    }

    prvI2CBusEnd(bus, ptI2c);
}

void TIREMO_HAL_I2C_Recover(TIREMO_HAL_I2C_Bus_t bus)
{
    if (!prvI2CIsBusValid(bus))
    {
        return;
    }

    TIREMO_HAL_I2C_PrepareBus(bus);
    (void)HAL_I2C_Uninit((I2C_ID_e)bus);
    (void)HAL_I2C_Init((I2C_ID_e)bus);
    prvI2CApplyConfig(bus);

    s_failStreak = 0U;
}

/******************************************************************************
 * Private Functions
 *****************************************************************************/

static bool prvI2CIsBusValid(TIREMO_HAL_I2C_Bus_t bus)
{
    return ((uint32_t)bus < (uint32_t)TIREMO_HAL_I2C_BUS_MAX);
}

static uint32_t prvI2CGetOpStatus(I2C_Type *ptI2c)
{
    uint32_t timeout = TIREMO_HAL_I2C_POLL_TIMEOUT;

    while (!GET_I2C_IER_FLAG(ptI2c))
    {
        if (timeout == 0U)
        {
            return TIREMO_HAL_I2C_OPSTATUS_TIMEOUT;
        }

        timeout--;
    }

    return GET_I2C_SR(ptI2c);
}

static void prvI2CSetOpStop(I2C_Type *ptI2c)
{
    uint32_t timeout = TIREMO_HAL_I2C_POLL_TIMEOUT;

    SET_I2C_CR_STOP(ptI2c, true);

    while (!GET_I2C_IER_FLAG(ptI2c))
    {
        if (timeout == 0U)
        {
            break;
        }

        timeout--;
    }

    SET_I2C_CR_STOP(ptI2c, false);
}

static void prvI2CFlushFlags(I2C_Type *ptI2c)
{
    uint32_t timeout = 10000U;

    while ((GET_I2C_IER_FLAG(ptI2c) != 0U) && (timeout > 0U))
    {
        SET_I2C_SR_CLEAR(ptI2c);
        timeout--;
    }
}

static void prvI2CBusBegin(TIREMO_HAL_I2C_Bus_t bus, I2C_Type *ptI2c)
{
    IRQn_Type irq = I2C_GetIRQNum((P_I2C_ID_e)bus);

    NVIC_DisableIRQ(irq);
    SET_I2C_IER_EN(ptI2c, false);
    prvI2CFlushFlags(ptI2c);
}

static void prvI2CBusEnd(TIREMO_HAL_I2C_Bus_t bus, I2C_Type *ptI2c)
{
    IRQn_Type irq = I2C_GetIRQNum((P_I2C_ID_e)bus);

    prvI2CFlushFlags(ptI2c);
    SET_I2C_IER_EN(ptI2c, false);
    NVIC_ClearPendingIRQ(irq);
    NVIC_DisableIRQ(irq);
}

static void prvI2CApplyConfig(TIREMO_HAL_I2C_Bus_t bus)
{
    I2C_CFG_t config =
    {
        .eMode = I2C_MODE_MASTER,
        .uPeriod.tFreq.un32Freq = TIREMO_HAL_I2C_RECOVER_FREQ_HZ,
        .tSdht.bEnable = true,
        .tSdht.un16Hold = TIREMO_HAL_I2C_RECOVER_SDA_HOLD,
    };

    (void)HAL_I2C_SetConfig((I2C_ID_e)bus, &config);
    (void)HAL_I2C_SetIRQ((I2C_ID_e)bus,
                         I2C_OPS_POLL,
                         NULL,
                         NULL,
                         TIREMO_HAL_I2C_RECOVER_IRQ_PRIO);
}

static void prvI2COnError(TIREMO_HAL_I2C_Bus_t bus)
{
    s_failStreak++;

    if (s_failStreak >= TIREMO_HAL_I2C_FAIL_LIMIT)
    {
        TIREMO_HAL_I2C_Recover(bus);
    }
    else
    {
        TIREMO_HAL_I2C_PrepareBus(bus);
    }
}
