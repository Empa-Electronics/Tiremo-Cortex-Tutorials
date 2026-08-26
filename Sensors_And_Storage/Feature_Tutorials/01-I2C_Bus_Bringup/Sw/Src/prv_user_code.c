/******************************************************************************
 * @file    prv_user_code.c
 * @brief   I2C Bus Bring-up -- probe both sensor addresses on I2C2 and
 *          log ACK/NACK for each, once at boot and then every 5 s.
 *
 * TIREMO_HAL_I2C has no explicit "probe" function of its own -- this
 * tutorial's own ProbeAndLog() builds one from TIREMO_HAL_I2C_Receive():
 * read exactly one byte and discard it, so the transaction never writes
 * anything to the sensor's internal state, and check the return status
 * for ACK/NACK. See the README's "Understanding the Probe" section for
 * why a read, not a write, and why this is the right level of protocol
 * knowledge for a bus bring-up tutorial (none at all -- just "does this
 * address answer").
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Board/board_config.h"
#include "../Common/Include/tiremo_systick.h"
#include "../Common/Include/tiremo_debug.h"
#include "../Tiremo_Hal/Include/tiremo_hal_i2c.h"

#include <stdbool.h>
#include <stdint.h>

/** @brief How often the bus is re-probed after the initial boot-time probe. */
#define PROBE_INTERVAL_MS (5000U)

static void ProbeAndLog(const char *pName, TIREMO_HAL_I2C_Bus_t bus, uint8_t address)
{
    uint8_t dummy;
    bool acked;

    acked = (TIREMO_HAL_I2C_Receive(bus, address, &dummy, 1U, true) ==
             TIREMO_HAL_I2C_STATUS_OK);

    if (acked)
    {
        TIREMO_LOG_INFO("%s (0x%02X): ACK", pName, address);
    }
    else
    {
        TIREMO_LOG_WARN("%s (0x%02X): NACK -- not responding", pName, address);
    }
}

static void ProbeBoth(void)
{
    ProbeAndLog("SHT40", TIREMO_BOARD_SHT40_I2C_BUS, TIREMO_BOARD_SHT40_I2C_ADDRESS);
    ProbeAndLog("LIS2DE12", TIREMO_BOARD_LIS2DE12_I2C_BUS, TIREMO_BOARD_LIS2DE12_I2C_ADDRESS);
}

void PRV_USER_Code(void)
{
    uint32_t now;
    uint32_t lastProbeMs;

    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();

    TIREMO_LOG_INFO("I2C_Bus_Bringup: boot");

    ProbeBoth();
    lastProbeMs = TIREMO_SysTick_GetMs();

    for (;;)
    {
        now = TIREMO_SysTick_GetMs();

        if ((now - lastProbeMs) >= PROBE_INTERVAL_MS)
        {
            ProbeBoth();
            lastProbeMs = now;
        }
    }
}
