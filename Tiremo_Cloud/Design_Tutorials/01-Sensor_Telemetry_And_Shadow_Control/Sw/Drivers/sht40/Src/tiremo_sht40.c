/******************************************************************************
 * @file    tiremo_sht40.c
 * @brief   Tiremo SHT40 temperature / humidity driver implementation.
 *
 * Implements the Sensirion SHT4x protocol directly on Tiremo_Hal_I2C
 * rather than wrapping the Sensirion C library the production firmware
 * uses. This tutorial's learning goal is the command/response itself
 * (0xFD, 6-byte + CRC, milli-unit conversion), so the protocol stays
 * visible here instead of disappearing behind sht4x_measure_high_precision().
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "../Include/tiremo_sht40.h"
#include "../../../Board/board_config.h"
#include "../../../Common/Include/tiremo_systick.h"
#include "../../../Tiremo_Hal/Include/tiremo_hal_i2c.h"

#include <stddef.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/** @brief SHT4x "measure T & RH, high precision" command (datasheet 4.5). */
#define TIREMO_SHT40_CMD_MEASURE_HIGH_PRECISION    (0xFDU)

/**
 * @brief Worst-case high-precision conversion time, milliseconds.
 *
 * Datasheet typical is 6.9 ms, max 8.3 ms. 10 ms matches Sensirion's
 * own driver and leaves a little margin for clock error.
 */
#define TIREMO_SHT40_MEASURE_WAIT_MS               (10U)

/** @brief CRC-8 polynomial 0x31 (x^8 + x^5 + x^4 + 1), init 0xFF. */
#define TIREMO_SHT40_CRC8_POLY                     (0x31U)
#define TIREMO_SHT40_CRC8_INIT                     (0xFFU)

/******************************************************************************
 * Private Function Prototypes
 *****************************************************************************/

static uint8_t prvSht40Crc8(const uint8_t *pData, uint32_t length);
static int32_t prvSht40TicksToMilliC(uint16_t ticks);
static int32_t prvSht40TicksToMilliRh(uint16_t ticks);

/******************************************************************************
 * Public Functions
 *****************************************************************************/

void TIREMO_SHT40_Init(void)
{
    TIREMO_HAL_I2C_PrepareBus(TIREMO_BOARD_SHT40_I2C_BUS);
}

TIREMO_SHT40_Status_t TIREMO_SHT40_Read(TIREMO_SHT40_Reading_t *pReading)
{
    const uint8_t command = TIREMO_SHT40_CMD_MEASURE_HIGH_PRECISION;
    uint8_t response[6];
    uint16_t tempTicks;
    uint16_t rhTicks;
    int32_t humidityMilliRh;

    if (pReading == NULL)
    {
        return TIREMO_SHT40_STATUS_ERROR;
    }

    /* Write the command, then STOP. The SHT4x does *not* use a repeated
     * START (that's the LIS2DE12 tutorial). Conversion happens while
     * the bus is idle; a read before it's done NACKs. */
    if (TIREMO_HAL_I2C_Transmit(TIREMO_BOARD_SHT40_I2C_BUS,
                                TIREMO_BOARD_SHT40_I2C_ADDRESS,
                                &command,
                                1U,
                                true) != TIREMO_HAL_I2C_STATUS_OK)
    {
        return TIREMO_SHT40_STATUS_ERROR;
    }

    TIREMO_SysTick_DelayMs(TIREMO_SHT40_MEASURE_WAIT_MS);

    if (TIREMO_HAL_I2C_Receive(TIREMO_BOARD_SHT40_I2C_BUS,
                               TIREMO_BOARD_SHT40_I2C_ADDRESS,
                               response,
                               sizeof(response),
                               true) != TIREMO_HAL_I2C_STATUS_OK)
    {
        return TIREMO_SHT40_STATUS_ERROR;
    }

    /* [T_MSB, T_LSB, T_CRC, RH_MSB, RH_LSB, RH_CRC] */
    if ((prvSht40Crc8(&response[0], 2U) != response[2]) ||
        (prvSht40Crc8(&response[3], 2U) != response[5]))
    {
        return TIREMO_SHT40_STATUS_ERROR;
    }

    tempTicks = ((uint16_t)response[0] << 8) | (uint16_t)response[1];
    rhTicks = ((uint16_t)response[3] << 8) | (uint16_t)response[4];

    pReading->temperatureMilliC = prvSht40TicksToMilliC(tempTicks);

    humidityMilliRh = prvSht40TicksToMilliRh(rhTicks);
    if (humidityMilliRh < 0)
    {
        humidityMilliRh = 0;
    }
    else if (humidityMilliRh > 100000)
    {
        humidityMilliRh = 100000;
    }
    pReading->humidityMilliRh = humidityMilliRh;

    return TIREMO_SHT40_STATUS_OK;
}

/******************************************************************************
 * Private Functions
 *****************************************************************************/

static uint8_t prvSht40Crc8(const uint8_t *pData, uint32_t length)
{
    uint32_t i;
    uint8_t bit;
    uint8_t crc = TIREMO_SHT40_CRC8_INIT;

    for (i = 0U; i < length; i++)
    {
        crc ^= pData[i];
        for (bit = 0U; bit < 8U; bit++)
        {
            if ((crc & 0x80U) != 0U)
            {
                crc = (uint8_t)((crc << 1) ^ TIREMO_SHT40_CRC8_POLY);
            }
            else
            {
                crc = (uint8_t)(crc << 1);
            }
        }
    }

    return crc;
}

/*
 * Datasheet: T = -45 + 175 * ticks / 65535  (degC)
 *            RH = -6 + 125 * ticks / 65535  (%RH)
 *
 * Integer form used by Sensirion's own driver, so this stays float-free
 * and fits in 32 bits: 175000/65535 ~= 21875/2^13, 125000/65535 ~= 15625/2^13.
 */
static int32_t prvSht40TicksToMilliC(uint16_t ticks)
{
    return ((21875 * (int32_t)ticks) >> 13) - 45000;
}

static int32_t prvSht40TicksToMilliRh(uint16_t ticks)
{
    return ((15625 * (int32_t)ticks) >> 13) - 6000;
}
