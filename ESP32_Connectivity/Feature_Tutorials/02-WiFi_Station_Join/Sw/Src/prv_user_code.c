/******************************************************************************
 * @file    prv_user_code.c
 * @brief   WiFi Station Join -- CWMODE, one CWJAP, confirm CWSTATE:2.
 *
 * Credentials come from wifi_secrets.h (copy wifi_secrets.h.example).
 * That file is gitignored so a real password never lands in the repo.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Common/Include/tiremo_systick.h"
#include "../Common/Include/tiremo_debug.h"
#include "../Drivers/esp32/Include/tiremo_esp32.h"
#include "wifi_secrets.h"

#ifndef TIREMO_WIFI_SSID
#error "Copy Sw/wifi_secrets.h.example to Sw/wifi_secrets.h and set TIREMO_WIFI_SSID / TIREMO_WIFI_PASSWORD"
#endif

#define JOIN_TIMEOUT_MS (20000U)
#define HEARTBEAT_MS    (5000U)

static void Hang(const char *pWhy)
{
    TIREMO_LOG_ERROR("%s", pWhy);
    for (;;)
    {
    }
}

void PRV_USER_Code(void)
{
    char ssid[33];

    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();

    TIREMO_LOG_INFO("WiFi_Station_Join: boot");

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

    /* Drop whatever AP the module remembered in its own NVS, then join
     * only the hardcoded network. Tutorial 03 is where persistence
     * belongs. */
    (void)TIREMO_ESP32_DisableAutoConnect();
    (void)TIREMO_ESP32_Disconnect();

    if (!TIREMO_ESP32_SetStationMode())
    {
        Hang("ESP32: CWMODE=1 failed");
    }
    TIREMO_LOG_INFO("ESP32: station mode");

    TIREMO_LOG_INFO("ESP32: joining SSID \"%s\"", TIREMO_WIFI_SSID);
    if (!TIREMO_ESP32_JoinAP(TIREMO_WIFI_SSID, TIREMO_WIFI_PASSWORD,
                             JOIN_TIMEOUT_MS))
    {
        Hang("ESP32: CWJAP did not report WIFI GOT IP");
    }
    TIREMO_LOG_INFO("ESP32: WIFI GOT IP");

    if (!TIREMO_ESP32_IsConnected())
    {
        Hang("ESP32: CWSTATE is not 2 after join");
    }
    TIREMO_LOG_INFO("ESP32: CWSTATE=2 (connected, IPv4)");

    ssid[0] = '\0';
    if (TIREMO_ESP32_GetConnectedSsid(ssid, sizeof(ssid)))
    {
        TIREMO_LOG_INFO("ESP32: CWJAP? SSID=\"%s\"", ssid);
    }
    else
    {
        TIREMO_LOG_WARN("ESP32: CWJAP? did not parse an SSID");
    }

    for (;;)
    {
        TIREMO_SysTick_DelayMs(HEARTBEAT_MS);
        if (TIREMO_ESP32_IsConnected())
        {
            TIREMO_LOG_INFO("ESP32: heartbeat still connected");
        }
        else
        {
            TIREMO_LOG_WARN("ESP32: heartbeat lost IPv4 (CWSTATE != 2)");
        }
    }
}
