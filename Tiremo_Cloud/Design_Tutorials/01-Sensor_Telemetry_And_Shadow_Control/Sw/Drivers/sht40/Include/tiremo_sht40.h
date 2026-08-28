/******************************************************************************
 * @file    tiremo_sht40.h
 * @brief   Tiremo SHT40 temperature / humidity driver API.
 *
 * Talks the Sensirion SHT4x command/response protocol over Tiremo_Hal_I2C:
 * one-byte measure command, wait, six-byte response with CRC, integer
 * conversion to milli-degC / milli-%RH. See the tutorial README's
 * "Understanding the SHT4x Protocol" section.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_SHT40_H
#define TIREMO_SHT40_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <stdint.h>

/******************************************************************************
 * Public Types
 *****************************************************************************/

typedef enum
{
    TIREMO_SHT40_STATUS_OK = 0,
    TIREMO_SHT40_STATUS_ERROR
} TIREMO_SHT40_Status_t;

typedef struct
{
    int32_t temperatureMilliC; /**< milli-degC, e.g. 23456 = 23.456 C */
    int32_t humidityMilliRh;   /**< milli-%RH, e.g. 45123 = 45.123 %RH */
} TIREMO_SHT40_Reading_t;

/******************************************************************************
 * Public Function Prototypes
 *****************************************************************************/

/**
 * @brief Prepare the I2C bus used by the SHT40.
 *
 * @warning Call PRV_I2C_Init() before this function.
 */
void TIREMO_SHT40_Init(void);

/**
 * @brief Take one high-precision T + RH measurement.
 *
 * Sends command 0xFD, waits the datasheet conversion time, reads six
 * bytes, checks both CRCs, and converts ticks to milli-units.
 *
 * @param[out] pReading  Destination for temperature and humidity.
 *
 * @return Driver status.
 *
 * @retval TIREMO_SHT40_STATUS_OK     Valid reading stored in pReading.
 * @retval TIREMO_SHT40_STATUS_ERROR  NULL pointer, I2C failure, or CRC mismatch.
 *
 * @warning Blocking for the conversion wait (~10 ms) plus two I2C
 *          transactions. Fine at 1 Hz; not for a tight inner loop.
 */
TIREMO_SHT40_Status_t TIREMO_SHT40_Read(TIREMO_SHT40_Reading_t *pReading);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_SHT40_H */
