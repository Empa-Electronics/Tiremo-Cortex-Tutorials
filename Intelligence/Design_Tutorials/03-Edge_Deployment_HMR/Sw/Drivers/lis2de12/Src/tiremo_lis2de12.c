/******************************************************************************
 * @file    tiremo_lis2de12.c
 * @brief   Tiremo LIS2DE12 accelerometer driver implementation.
 *
 * Talks the register map directly. Multi-byte reads set bit 7 of the
 * sub-address (auto-increment). Writes use WriteRead with a dummy byte
 * afterwards so the ABOV I2C peripheral ends the frame cleanly -- same
 * trick as the reference I2C HAL adapter, not a datasheet requirement.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Include/tiremo_lis2de12.h"
#include "../../../Board/board_config.h"
#include "../../../Common/Include/tiremo_systick.h"
#include "../../../Tiremo_Hal/Include/tiremo_hal_i2c.h"

#include <stddef.h>

#define TIREMO_LIS2DE12_WHO_AM_I            (0x0FU)
#define TIREMO_LIS2DE12_CTRL_REG1           (0x20U)
#define TIREMO_LIS2DE12_CTRL_REG4           (0x23U)
#define TIREMO_LIS2DE12_STATUS_REG          (0x27U)
#define TIREMO_LIS2DE12_OUT_X_H             (0x29U)
#define TIREMO_LIS2DE12_ID                  (0x33U)

#define TIREMO_LIS2DE12_AUTO_INC            (0x80U)
#define TIREMO_LIS2DE12_CTRL1_AXES_LP       (0x0FU)
#define TIREMO_LIS2DE12_STATUS_ZYXDA        (0x08U)

#define TIREMO_LIS2DE12_OVERSAMPLE_CNT      (1U)
#define TIREMO_LIS2DE12_DRDY_TIMEOUT_MS     (200U)
#define TIREMO_LIS2DE12_WHOAMI_RETRIES      (10U)

static bool s_initialized = false;
static uint8_t s_whoAmI = 0U;
static uint16_t s_sensMg = 16U;

static bool prvReadRegs(uint8_t reg, uint8_t *pData, uint32_t length)
{
    uint8_t regAddr;

    if ((pData == NULL) || (length == 0U))
    {
        return false;
    }

    regAddr = (length > 1U) ? (uint8_t)(reg | TIREMO_LIS2DE12_AUTO_INC) : reg;

    return (TIREMO_HAL_I2C_WriteRead(TIREMO_BOARD_LIS2DE12_I2C_BUS,
                                     TIREMO_BOARD_LIS2DE12_I2C_ADDRESS,
                                     &regAddr,
                                     1U,
                                     pData,
                                     length) == TIREMO_HAL_I2C_STATUS_OK);
}

static bool prvWriteReg(uint8_t reg, uint8_t value)
{
    uint8_t tx[2];
    uint8_t dummy = 0U;

    tx[0] = reg;
    tx[1] = value;

    return (TIREMO_HAL_I2C_WriteRead(TIREMO_BOARD_LIS2DE12_I2C_BUS,
                                     TIREMO_BOARD_LIS2DE12_I2C_ADDRESS,
                                     tx,
                                     2U,
                                     &dummy,
                                     1U) == TIREMO_HAL_I2C_STATUS_OK);
}

static uint8_t prvScaleToFsBits(uint8_t scaleG, uint16_t *pSensMg)
{
    switch (scaleG)
    {
    case 4U:
        *pSensMg = 32U;
        return 0x10U;
    case 8U:
        *pSensMg = 64U;
        return 0x20U;
    case 16U:
        *pSensMg = 192U;
        return 0x30U;
    case 2U:
    default:
        *pSensMg = 16U;
        return 0x00U;
    }
}

static uint8_t prvHzToOdrNibble(uint16_t dataRateHz)
{
    if (dataRateHz == 0U)
    {
        return 0x00U;
    }
    if (dataRateHz <= 1U)
    {
        return 0x01U;
    }
    if (dataRateHz <= 10U)
    {
        return 0x02U;
    }
    if (dataRateHz <= 25U)
    {
        return 0x03U;
    }
    if (dataRateHz <= 50U)
    {
        return 0x04U;
    }
    if (dataRateHz <= 100U)
    {
        return 0x05U;
    }
    if (dataRateHz <= 200U)
    {
        return 0x06U;
    }
    return 0x07U;
}

bool TIREMO_LIS2DE12_Init(void)
{
    uint8_t whoAmI = 0U;
    uint8_t retry;

    if (s_initialized)
    {
        return true;
    }

    TIREMO_HAL_I2C_PrepareBus(TIREMO_BOARD_LIS2DE12_I2C_BUS);

    for (retry = 0U; retry < TIREMO_LIS2DE12_WHOAMI_RETRIES; retry++)
    {
        if (prvReadRegs(TIREMO_LIS2DE12_WHO_AM_I, &whoAmI, 1U) &&
            (whoAmI == TIREMO_LIS2DE12_ID))
        {
            break;
        }
        TIREMO_SysTick_DelayMs(10U);
    }

    if (whoAmI != TIREMO_LIS2DE12_ID)
    {
        return false;
    }

    s_whoAmI = whoAmI;
    s_initialized = true;

    if (!TIREMO_LIS2DE12_Configure(2U, 100U))
    {
        s_initialized = false;
        s_whoAmI = 0U;
        return false;
    }

    TIREMO_SysTick_DelayMs(20U);
    return true;
}

bool TIREMO_LIS2DE12_Configure(uint8_t scaleG, uint16_t dataRateHz)
{
    uint16_t sensMg = 16U;
    uint8_t fsBits;
    uint8_t ctrl1;
    uint8_t ctrl1Read = 0U;

    if (!s_initialized)
    {
        return false;
    }

    fsBits = prvScaleToFsBits(scaleG, &sensMg);
    if (!prvWriteReg(TIREMO_LIS2DE12_CTRL_REG4, fsBits))
    {
        return false;
    }

    ctrl1 = (uint8_t)((prvHzToOdrNibble(dataRateHz) << 4) |
                      TIREMO_LIS2DE12_CTRL1_AXES_LP);
    if (!prvWriteReg(TIREMO_LIS2DE12_CTRL_REG1, ctrl1))
    {
        return false;
    }

    TIREMO_SysTick_DelayMs(2U);
    if (!prvReadRegs(TIREMO_LIS2DE12_CTRL_REG1, &ctrl1Read, 1U) ||
        (ctrl1Read != ctrl1))
    {
        return false;
    }

    s_sensMg = sensMg;
    return true;
}

bool TIREMO_LIS2DE12_ReadWhoAmI(uint8_t *pWhoAmI)
{
    if ((!s_initialized) || (pWhoAmI == NULL))
    {
        return false;
    }

    *pWhoAmI = s_whoAmI;
    return true;
}

bool TIREMO_LIS2DE12_ReadAccel(int16_t *pAccelX_mg,
                               int16_t *pAccelY_mg,
                               int16_t *pAccelZ_mg)
{
    uint8_t raw[6];
    uint8_t statusReg = 0U;
    uint8_t sample;
    uint16_t timeoutMs;
    int32_t sumX = 0;
    int32_t sumY = 0;
    int32_t sumZ = 0;

    if ((!s_initialized) ||
        (pAccelX_mg == NULL) ||
        (pAccelY_mg == NULL) ||
        (pAccelZ_mg == NULL))
    {
        return false;
    }

    for (sample = 0U; sample < TIREMO_LIS2DE12_OVERSAMPLE_CNT; sample++)
    {
        timeoutMs = TIREMO_LIS2DE12_DRDY_TIMEOUT_MS;

        do
        {
            if (!prvReadRegs(TIREMO_LIS2DE12_STATUS_REG, &statusReg, 1U))
            {
                return false;
            }

            if ((statusReg & TIREMO_LIS2DE12_STATUS_ZYXDA) != 0U)
            {
                break;
            }

            TIREMO_SysTick_DelayMs(1U);
            timeoutMs--;
        } while (timeoutMs > 0U);

        if ((statusReg & TIREMO_LIS2DE12_STATUS_ZYXDA) == 0U)
        {
            return false;
        }

        if (!prvReadRegs(TIREMO_LIS2DE12_OUT_X_H, raw, 6U))
        {
            return false;
        }

        /* 8-bit samples are left-justified in a 16-bit word. */
        sumX += (int16_t)(((uint16_t)raw[0] << 8) | raw[1]);
        sumY += (int16_t)(((uint16_t)raw[2] << 8) | raw[3]);
        sumZ += (int16_t)(((uint16_t)raw[4] << 8) | raw[5]);
    }

    *pAccelX_mg = (int16_t)((sumX * (int32_t)s_sensMg) /
                            (TIREMO_LIS2DE12_OVERSAMPLE_CNT * 256));
    *pAccelY_mg = (int16_t)((sumY * (int32_t)s_sensMg) /
                            (TIREMO_LIS2DE12_OVERSAMPLE_CNT * 256));
    *pAccelZ_mg = (int16_t)((sumZ * (int32_t)s_sensMg) /
                            (TIREMO_LIS2DE12_OVERSAMPLE_CNT * 256));

    return true;
}
