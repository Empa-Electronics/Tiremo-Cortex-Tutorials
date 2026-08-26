/******************************************************************************
 * @file    prv_user_code.c
 * @brief   BluFi provision (no hardcoded SSID); persist last-good BL01.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Common/Include/tiremo_systick.h"
#include "../Common/Include/tiremo_debug.h"
#include "../Common/Include/tiremo_persist.h"
#include "../Drivers/esp32/Include/tiremo_esp32.h"

#include <string.h>

#define BLUFI_NAME        "Tiremo-Cortex"
#define SCAN_TIMEOUT_MS   (8000U)
#define JOIN_TIMEOUT_MS   (20000U)
#define RETRY_MAX         (3U)
#define RETRY_DELAY_MS    (1000U)
#define POLL_MS           (20U)
#define ADVERTISE_LOG_MS  (5000U)
#define HEARTBEAT_MS      (5000U)

static void Hang(const char *pWhy)
{
    TIREMO_LOG_ERROR("%s", pWhy);
    for (;;)
    {
    }
}

static void HeartbeatConnected(void)
{
    for (;;)
    {
        TIREMO_SysTick_DelayMs(HEARTBEAT_MS);
        if (TIREMO_ESP32_IsConnected())
        {
            TIREMO_LOG_INFO("ESP32: heartbeat still connected");
        }
        else
        {
            TIREMO_LOG_WARN("ESP32: heartbeat lost IPv4");
        }
    }
}

static bool SaveCredentials(const char *pSsid, const char *pPassword)
{
    TIREMO_PersistRecord_t rec;

    TIREMO_Persist_Defaults(&rec);
    (void)strncpy(rec.ssid, pSsid, sizeof(rec.ssid) - 1U);
    if (pPassword != NULL)
    {
        (void)strncpy(rec.password, pPassword, sizeof(rec.password) - 1U);
    }

    if (!TIREMO_Persist_Save(&rec))
    {
        TIREMO_LOG_WARN("persist: save failed");
        return false;
    }

    TIREMO_LOG_INFO("persist: saved SSID \"%s\" (password not logged)", pSsid);
    return true;
}

static bool JoinSaved(const char *pSsid, const char *pPassword)
{
    uint32_t attempt;
    TIREMO_ESP32_JoinStatus_t join = TIREMO_ESP32_JOIN_TIMEOUT;

    TIREMO_LOG_INFO("ESP32: scanning for saved SSID \"%s\"", pSsid);
    if (!TIREMO_ESP32_ScanForSsid(pSsid, SCAN_TIMEOUT_MS))
    {
        TIREMO_LOG_WARN("ESP32: saved SSID not visible");
        return false;
    }

    for (attempt = 1U; attempt <= RETRY_MAX; attempt++)
    {
        TIREMO_LOG_INFO("ESP32: join attempt %u/%u",
                        (unsigned)attempt, (unsigned)RETRY_MAX);
        join = TIREMO_ESP32_JoinAP(pSsid, pPassword, JOIN_TIMEOUT_MS);
        if (join == TIREMO_ESP32_JOIN_GOT_IP)
        {
            TIREMO_LOG_INFO("ESP32: WIFI GOT IP (saved credentials)");
            return true;
        }
        if (join == TIREMO_ESP32_JOIN_ERROR)
        {
            TIREMO_LOG_WARN("ESP32: saved credentials rejected");
            return false;
        }
        if (attempt < RETRY_MAX)
        {
            TIREMO_SysTick_DelayMs(RETRY_DELAY_MS);
        }
    }

    return false;
}

static void WaitBluFiThenPersist(void)
{
    TIREMO_ESP32_BlufiStatus_t st;
    uint32_t lastLogMs;

    if (!TIREMO_ESP32_BlufiStart(BLUFI_NAME))
    {
        Hang("ESP32: BluFi start failed (is BluFi compiled into ESP-AT?)");
    }

    TIREMO_LOG_INFO("BluFi: open EspBluFi, connect to \"%s\", Configure",
                    BLUFI_NAME);
    lastLogMs = TIREMO_SysTick_GetMs();

    for (;;)
    {
        /* Sole consumer of UART2. Do not call IsConnected / SendCommand
         * here — those flush the RX ring and can eat +SSID:/+PASSWORD:. */
        TIREMO_ESP32_BlufiPoll(&st);

        if (st.gotIp)
        {
            break;
        }

        if ((TIREMO_SysTick_GetMs() - lastLogMs) >= ADVERTISE_LOG_MS)
        {
            lastLogMs = TIREMO_SysTick_GetMs();
            TIREMO_LOG_INFO("BluFi: still advertising as \"%s\"", BLUFI_NAME);
        }

        TIREMO_SysTick_DelayMs(POLL_MS);
    }

    /* Snapshot before BlufiStop(), which sends AT and flushes the ring. */
    if (!st.gotSsid)
    {
        TIREMO_LOG_WARN("ESP32: GOT IP without +SSID: URC");
    }
    if (!st.gotPassword)
    {
        TIREMO_LOG_WARN("ESP32: GOT IP without +PASSWORD: URC "
                        "(module NVS keeps the only copy)");
    }

    /* Turned on before BlufiStop() can fail: with no +PASSWORD: URC the
     * module's own NVS copy is the only way back onto this AP. */
    (void)TIREMO_ESP32_EnableAutoConnect();
    (void)TIREMO_ESP32_BlufiStop();

    if (!st.gotSsid)
    {
        char queried[33];
        if (TIREMO_ESP32_GetConnectedSsid(queried, sizeof(queried)))
        {
            (void)strncpy(st.ssid, queried, sizeof(st.ssid) - 1U);
            st.gotSsid = true;
            TIREMO_LOG_INFO("ESP32: SSID recovered from CWJAP? \"%s\"",
                            st.ssid);
        }
    }

    if (st.gotSsid)
    {
        (void)SaveCredentials(st.ssid, st.password);
    }
    else
    {
        TIREMO_LOG_WARN("persist: no SSID to save (NVS auto-connect still on)");
    }
}

void PRV_USER_Code(void)
{
    TIREMO_PersistRecord_t saved;
    bool usedSaved;

    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();

    TIREMO_LOG_INFO("BluFi_Provisioning: boot");

    if (!TIREMO_Persist_Init())
    {
        Hang("persist: flash init failed");
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

    usedSaved = TIREMO_Persist_Load(&saved) && TIREMO_Persist_HasSsid(&saved);
    if (usedSaved)
    {
        TIREMO_LOG_INFO("persist: loaded SSID \"%s\"", saved.ssid);
        if (JoinSaved(saved.ssid, saved.password))
        {
            HeartbeatConnected();
        }
        TIREMO_LOG_WARN("persist: saved join failed, falling back to BluFi");
        (void)TIREMO_Persist_Clear();
    }
    else
    {
        TIREMO_LOG_INFO("persist: no saved SSID (no wifi_secrets.h in this tutorial)");
    }

    WaitBluFiThenPersist();
    HeartbeatConnected();
}
