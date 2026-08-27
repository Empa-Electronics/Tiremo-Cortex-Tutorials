/******************************************************************************
 * @file    prv_user_code.c
 * @brief   Device identity from the ESP32 station MAC.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Common/Include/tiremo_systick.h"
#include "../Common/Include/tiremo_debug.h"
#include "../Common/Include/tiremo_identity.h"
#include "../Drivers/esp32/Include/tiremo_esp32.h"

#include <string.h>

static void Hang(const char *pWhy)
{
    TIREMO_LOG_ERROR("%s", pWhy);
    for (;;)
    {
    }
}

void PRV_USER_Code(void)
{
    uint8_t eui[6];
    uint8_t again[6];
    char macText[18];
    char deviceId[TIREMO_IDENTITY_ID_SIZE];
    char checkId[TIREMO_IDENTITY_ID_SIZE];

    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();

    TIREMO_LOG_INFO("Device_Identity_From_MAC: boot");

    if (!TIREMO_ESP32_Init())
    {
        Hang("ESP32: transport init failed");
    }
    if (!TIREMO_ESP32_IsPresent())
    {
        Hang("ESP32: AT probe failed (no OK)");
    }
    if (!TIREMO_ESP32_DisableEcho())
    {
        Hang("ESP32: ATE0 failed");
    }

    if (!TIREMO_ESP32_GetHwEui(eui))
    {
        Hang("ESP32: CIPSTAMAC? failed");
    }
    if (!TIREMO_Identity_FormatMac(eui, macText, sizeof(macText)))
    {
        Hang("identity: MAC format failed");
    }
    if (!TIREMO_Identity_FromEui(eui, deviceId, sizeof(deviceId)))
    {
        Hang("identity: device ID format failed");
    }

    TIREMO_LOG_INFO("identity: station MAC %s", macText);
    TIREMO_LOG_INFO("identity: deviceId %s", deviceId);

    for (;;)
    {
        TIREMO_SysTick_DelayMs(5000U);

        if (!TIREMO_ESP32_GetHwEui(again))
        {
            TIREMO_LOG_WARN("identity: CIPSTAMAC? failed on heartbeat");
            continue;
        }
        if (!TIREMO_Identity_FromEui(again, checkId, sizeof(checkId)))
        {
            TIREMO_LOG_WARN("identity: format failed on heartbeat");
            continue;
        }
        if (memcmp(eui, again, sizeof(eui)) != 0)
        {
            TIREMO_LOG_ERROR("identity: MAC changed %s -> (re-read differs)",
                             deviceId);
            continue;
        }
        TIREMO_LOG_INFO("identity: still %s", checkId);
    }
}
