/******************************************************************************
 * @file    prv_user_code.c
 * @brief   Join WiFi (tutorial 03 path), then SNTP into TIREMO_Clock.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Common/Include/tiremo_systick.h"
#include "../Common/Include/tiremo_debug.h"
#include "../Common/Include/tiremo_persist.h"
#include "../Common/Include/tiremo_clock.h"
#include "../Drivers/esp32/Include/tiremo_esp32.h"
#include "wifi_secrets.h"

#include <string.h>

#ifndef TIREMO_WIFI_SSID
#error "Copy Sw/wifi_secrets.h.example to Sw/wifi_secrets.h and set TIREMO_WIFI_SSID / TIREMO_WIFI_PASSWORD"
#endif

#define SCAN_TIMEOUT_MS   (8000U)
#define JOIN_TIMEOUT_MS   (20000U)
#define SNTP_TIMEOUT_MS   (20000U)
#define RETRY_MAX         (3U)
#define RETRY_DELAY_MS    (1000U)
#define HEARTBEAT_MS      (1000U)

static void Hang(const char *pWhy)
{
    TIREMO_LOG_ERROR("%s", pWhy);
    for (;;)
    {
    }
}

static void IdleDisconnected(void)
{
    for (;;)
    {
        TIREMO_SysTick_DelayMs(5000U);
        TIREMO_LOG_INFO("ESP32: idle, not connected");
    }
}

static bool SaveCredentials(const char *pSsid, const char *pPassword)
{
    TIREMO_PersistRecord_t rec;

    TIREMO_Persist_Defaults(&rec);
    (void)strncpy(rec.ssid, pSsid, sizeof(rec.ssid) - 1U);
    (void)strncpy(rec.password, pPassword, sizeof(rec.password) - 1U);

    if (!TIREMO_Persist_Save(&rec))
    {
        TIREMO_LOG_WARN("persist: save failed");
        return false;
    }

    TIREMO_LOG_INFO("persist: saved SSID \"%s\" (password not logged)", pSsid);
    return true;
}

static void ClearSaved(void)
{
    if (TIREMO_Persist_Clear())
    {
        TIREMO_LOG_INFO("persist: cleared last-good SSID");
    }
    else
    {
        TIREMO_LOG_WARN("persist: clear failed");
    }
}

void PRV_USER_Code(void)
{
    TIREMO_PersistRecord_t saved;
    const char *pSsid;
    const char *pPassword;
    bool usedSaved;
    uint32_t attempt;
    TIREMO_ESP32_JoinStatus_t join = TIREMO_ESP32_JOIN_TIMEOUT;
    uint32_t epoch = 0U;
    uint32_t bootMs;

    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();

    TIREMO_LOG_INFO("NTP_Time_Sync: boot");

    if (!TIREMO_Persist_Init())
    {
        Hang("persist: flash init failed");
    }

    usedSaved = TIREMO_Persist_Load(&saved) && TIREMO_Persist_HasSsid(&saved);
    if (usedSaved)
    {
        pSsid = saved.ssid;
        pPassword = saved.password;
        TIREMO_LOG_INFO("persist: loaded SSID \"%s\"", pSsid);
    }
    else
    {
        pSsid = TIREMO_WIFI_SSID;
        pPassword = TIREMO_WIFI_PASSWORD;
        TIREMO_LOG_INFO("persist: no saved SSID, using wifi_secrets.h \"%s\"",
                        pSsid);
    }

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

    (void)TIREMO_ESP32_DisableAutoConnect();
    (void)TIREMO_ESP32_Disconnect();
    if (!TIREMO_ESP32_SetStationMode())
    {
        Hang("ESP32: CWMODE=1 failed");
    }

    TIREMO_LOG_INFO("ESP32: scanning for \"%s\"", pSsid);
    if (!TIREMO_ESP32_ScanForSsid(pSsid, SCAN_TIMEOUT_MS))
    {
        TIREMO_LOG_WARN("ESP32: SSID not visible, skip join (fail fast)");
        if (usedSaved)
        {
            ClearSaved();
        }
        IdleDisconnected();
    }
    TIREMO_LOG_INFO("ESP32: SSID visible");

    for (attempt = 1U; attempt <= RETRY_MAX; attempt++)
    {
        TIREMO_LOG_INFO("ESP32: join attempt %u/%u",
                        (unsigned)attempt, (unsigned)RETRY_MAX);
        join = TIREMO_ESP32_JoinAP(pSsid, pPassword, JOIN_TIMEOUT_MS);

        if (join == TIREMO_ESP32_JOIN_GOT_IP)
        {
            TIREMO_LOG_INFO("ESP32: WIFI GOT IP");
            if (!TIREMO_ESP32_IsConnected())
            {
                TIREMO_LOG_WARN("ESP32: GOT IP but CWSTATE != 2");
            }
            else
            {
                TIREMO_LOG_INFO("ESP32: CWSTATE=2");
            }
            (void)SaveCredentials(pSsid, pPassword);
            break;
        }

        if (join == TIREMO_ESP32_JOIN_ERROR)
        {
            TIREMO_LOG_WARN("ESP32: CWJAP ERROR (definite fail, not a timeout)");
            ClearSaved();
            IdleDisconnected();
        }

        TIREMO_LOG_WARN("ESP32: join timeout, will retry");
        if (attempt < RETRY_MAX)
        {
            TIREMO_SysTick_DelayMs(RETRY_DELAY_MS);
        }
    }

    if (join != TIREMO_ESP32_JOIN_GOT_IP)
    {
        TIREMO_LOG_WARN("ESP32: %u timeouts, giving up", (unsigned)RETRY_MAX);
        ClearSaved();
        IdleDisconnected();
    }

    if (!TIREMO_ESP32_EnableSntp())
    {
        Hang("ESP32: CIPSNTPCFG failed");
    }
    TIREMO_LOG_INFO("ESP32: SNTP enabled (timezone 0 = UTC)");

    if (!TIREMO_ESP32_WaitSntpEpoch(&epoch, SNTP_TIMEOUT_MS))
    {
        Hang("ESP32: SNTP timeout (CIPSNTPTIME still 1970 or unparsable)");
    }

    TIREMO_Clock_SetEpoch(epoch);
    TIREMO_LOG_INFO("clock: SetEpoch %lu (from CIPSNTPTIME asctime)",
                    (unsigned long)epoch);

    bootMs = TIREMO_SysTick_GetMs();
    for (;;)
    {
        uint32_t now = 0U;

        TIREMO_SysTick_DelayMs(HEARTBEAT_MS);
        if (!TIREMO_Clock_GetEpoch(&now))
        {
            TIREMO_LOG_WARN("clock: GetEpoch failed (not synced)");
            continue;
        }

        TIREMO_LOG_INFO("clock: epoch=%lu  elapsed=%lu s",
                        (unsigned long)now,
                        (unsigned long)((TIREMO_SysTick_GetMs() - bootMs) /
                                        1000U));
    }
}
