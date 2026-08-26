/******************************************************************************
 * @file    prv_user_code.c
 * @brief   Boot cascade: saved WiFi, else NVS SSID-only, else BluFi, then NTP.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Common/Include/tiremo_systick.h"
#include "../Common/Include/tiremo_debug.h"
#include "../Common/Include/tiremo_banner.h"
#include "../Common/Include/tiremo_persist.h"
#include "../Common/Include/tiremo_clock.h"
#include "../Drivers/esp32/Include/tiremo_esp32.h"

#include <string.h>

#define BLUFI_NAME        "Tiremo-Cortex"
#define SCAN_TIMEOUT_MS   (8000U)
#define JOIN_TIMEOUT_MS   (20000U)
#define SNTP_TIMEOUT_MS   (20000U)
#define RETRY_MAX         (3U)
#define RETRY_DELAY_MS    (1000U)
#define NVS_WAIT_MS       (15000U)
#define NVS_POLL_MS       (1000U)
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

static bool SaveCredentials(const char *pSsid, const char *pPassword)
{
    TIREMO_PersistRecord_t rec;

    if ((pSsid == NULL) || (pSsid[0] == '\0'))
    {
        return false;
    }

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

static bool prvTryScanJoin(const char *pSsid, const char *pPassword)
{
    uint32_t attempt;
    TIREMO_ESP32_JoinStatus_t join = TIREMO_ESP32_JOIN_TIMEOUT;

    TIREMO_LOG_INFO("cascade: try saved SSID+password");
    (void)TIREMO_ESP32_DisableAutoConnect();
    (void)TIREMO_ESP32_Disconnect();

    TIREMO_LOG_INFO("ESP32: scanning for \"%s\"", pSsid);
    if (!TIREMO_ESP32_ScanForSsid(pSsid, SCAN_TIMEOUT_MS))
    {
        TIREMO_LOG_WARN("cascade: saved SSID not visible");
        return false;
    }

    for (attempt = 1U; attempt <= RETRY_MAX; attempt++)
    {
        TIREMO_LOG_INFO("ESP32: join attempt %u/%u",
                        (unsigned)attempt, (unsigned)RETRY_MAX);
        join = TIREMO_ESP32_JoinAP(pSsid, pPassword, JOIN_TIMEOUT_MS);
        if (join == TIREMO_ESP32_JOIN_GOT_IP)
        {
            TIREMO_LOG_INFO("cascade: saved join got IP");
            return true;
        }
        if (join == TIREMO_ESP32_JOIN_ERROR)
        {
            TIREMO_LOG_WARN("cascade: saved join ERROR");
            return false;
        }
        if (attempt < RETRY_MAX)
        {
            TIREMO_SysTick_DelayMs(RETRY_DELAY_MS);
        }
    }

    return false;
}

static bool prvTryNvsRejoin(const char *pSsid)
{
    TIREMO_ESP32_JoinStatus_t join;
    uint32_t startMs;

    TIREMO_LOG_INFO("cascade: try NVS auto-rejoin (SSID only, no MCU password)");
    (void)TIREMO_ESP32_EnableAutoConnect();

    /* With CWAUTOCONN=1 the module rejoins by itself after the power
     * cycle in Init(). Watch CWSTATE before spending a CWJAP timeout. */
    startMs = TIREMO_SysTick_GetMs();
    while ((TIREMO_SysTick_GetMs() - startMs) < NVS_WAIT_MS)
    {
        if (TIREMO_ESP32_IsConnected())
        {
            TIREMO_LOG_INFO("cascade: NVS rejoin got IP");
            return true;
        }
        TIREMO_SysTick_DelayMs(NVS_POLL_MS);
    }

    join = TIREMO_ESP32_JoinStoredSsid(pSsid, JOIN_TIMEOUT_MS);
    if (join == TIREMO_ESP32_JOIN_GOT_IP)
    {
        TIREMO_LOG_INFO("cascade: NVS rejoin got IP (explicit CWJAP)");
        return true;
    }

    TIREMO_LOG_WARN("cascade: NVS rejoin failed");
    return false;
}

static bool prvTryBlufi(TIREMO_PersistRecord_t *pHarvest)
{
    TIREMO_ESP32_BlufiStatus_t st;
    uint32_t lastLogMs;

    TIREMO_LOG_INFO("cascade: BluFi fallback");
    (void)TIREMO_ESP32_DisableAutoConnect();
    (void)TIREMO_ESP32_Disconnect();

    if (!TIREMO_ESP32_BlufiStart(BLUFI_NAME))
    {
        TIREMO_LOG_ERROR("cascade: BluFi start failed");
        return false;
    }

    TIREMO_LOG_INFO("BluFi: EspBluFi → \"%s\" → Configure", BLUFI_NAME);
    lastLogMs = TIREMO_SysTick_GetMs();

    for (;;)
    {
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

    /* Re-enable NVS auto-connect before anything else can fail: it is the
     * only copy of the password when the module does not print +PASSWORD:. */
    (void)TIREMO_ESP32_EnableAutoConnect();
    (void)TIREMO_ESP32_BlufiStop();

    TIREMO_Persist_Defaults(pHarvest);
    if (st.gotSsid)
    {
        (void)strncpy(pHarvest->ssid, st.ssid, sizeof(pHarvest->ssid) - 1U);
    }
    else if (TIREMO_ESP32_GetConnectedSsid(pHarvest->ssid,
                                           sizeof(pHarvest->ssid)))
    {
        TIREMO_LOG_INFO("cascade: SSID from CWJAP? \"%s\"", pHarvest->ssid);
    }
    else
    {
        TIREMO_LOG_WARN("cascade: BluFi GOT IP but no SSID to persist");
        return true;
    }

    if (st.gotPassword)
    {
        (void)strncpy(pHarvest->password, st.password,
                      sizeof(pHarvest->password) - 1U);
    }
    else
    {
        /* No +PASSWORD: URC from this AT build. Persist SSID only; the
         * next boot takes the NVS rejoin branch, where the module
         * supplies the password it kept itself. */
        TIREMO_LOG_INFO("cascade: no +PASSWORD: URC, saving SSID only "
                        "(next boot uses NVS rejoin)");
    }

    return true;
}

static void prvSyncNtpThenHeartbeat(const char *pPath)
{
    uint32_t epoch = 0U;

    (void)TIREMO_ESP32_EnableAutoConnect();

    if (!TIREMO_ESP32_EnableSntp())
    {
        Hang("ESP32: CIPSNTPCFG failed");
    }
    TIREMO_LOG_INFO("ESP32: SNTP enabled (timezone 0 = UTC)");

    if (!TIREMO_ESP32_WaitSntpEpoch(&epoch, SNTP_TIMEOUT_MS))
    {
        Hang("ESP32: SNTP timeout");
    }
    TIREMO_Clock_SetEpoch(epoch);
    TIREMO_LOG_INFO("clock: SetEpoch %lu  path=%s",
                    (unsigned long)epoch, pPath);

    for (;;)
    {
        uint32_t now = 0U;

        TIREMO_SysTick_DelayMs(HEARTBEAT_MS);
        if (!TIREMO_Clock_GetEpoch(&now))
        {
            TIREMO_LOG_WARN("clock: not synced");
            continue;
        }
        if (TIREMO_ESP32_IsConnected())
        {
            TIREMO_LOG_INFO("cascade: online path=%s epoch=%lu",
                            pPath, (unsigned long)now);
        }
        else
        {
            TIREMO_LOG_WARN("cascade: lost IPv4 path=%s epoch=%lu",
                            pPath, (unsigned long)now);
        }
    }
}

void PRV_USER_Code(void)
{
    TIREMO_PersistRecord_t saved;
    TIREMO_PersistRecord_t harvest;
    bool online = false;
    const char *pPath = "none";

    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();
    TIREMO_Banner_Print("Boot Cascade: Saved WiFi then BluFi");

    TIREMO_LOG_INFO("Boot_Cascade: boot");

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
    if (!TIREMO_ESP32_SetStationMode())
    {
        Hang("ESP32: CWMODE=1 failed");
    }

    if (TIREMO_Persist_Load(&saved) && TIREMO_Persist_HasSsid(&saved))
    {
        TIREMO_LOG_INFO("persist: loaded SSID \"%s\" password=%s",
                        saved.ssid,
                        TIREMO_Persist_HasPassword(&saved) ? "yes" : "no");

        if (TIREMO_Persist_HasPassword(&saved))
        {
            online = prvTryScanJoin(saved.ssid, saved.password);
            if (online)
            {
                pPath = "saved";
            }
        }
        else
        {
            online = prvTryNvsRejoin(saved.ssid);
            if (online)
            {
                pPath = "nvs";
            }
        }
    }
    else
    {
        TIREMO_LOG_INFO("persist: no CS01 record (factory-fresh for this app)");
    }

    if (!online)
    {
        if (!prvTryBlufi(&harvest))
        {
            Hang("cascade: BluFi failed");
        }
        pPath = "blufi";
        if (harvest.ssid[0] != '\0')
        {
            (void)SaveCredentials(harvest.ssid, harvest.password);
        }
        online = true;
    }

    (void)online;
    prvSyncNtpThenHeartbeat(pPath);
}
