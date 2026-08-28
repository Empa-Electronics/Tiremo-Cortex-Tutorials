/******************************************************************************
 * @file    prv_user_code.c
 * @brief   BluFi cascade, Tiremo Cloud, live shadow, jobs, gated sensors.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "abov_config.h"
#include "../Board/board_config.h"
#include "../Common/Include/tiremo_systick.h"
#include "../Common/Include/tiremo_debug.h"
#include "../Common/Include/tiremo_banner.h"
#include "../Common/Include/tiremo_persist.h"
#include "../Common/Include/tiremo_clock.h"
#include "../Common/Include/tiremo_identity.h"
#include "../Common/Include/tiremo_provision.h"
#include "../Common/Include/tiremo_shadow.h"
#include "../Common/Include/tiremo_telemetry.h"
#include "../Common/Include/tiremo_jobs.h"
#include "../Drivers/esp32/Include/tiremo_esp32.h"
#include "../Drivers/button/Include/tiremo_button.h"
#include "../Drivers/sht40/Include/tiremo_sht40.h"
#include "../Drivers/lis2de12/Include/tiremo_lis2de12.h"
#include "aws_iot_config.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifndef TIREMO_AWS_ENDPOINT
#error "Copy Sw/aws_iot_config.h.example to Sw/aws_iot_config.h and fill in endpoint / template / product values"
#endif

#ifndef TIREMO_AWS_THING_NAME
#define TIREMO_AWS_THING_NAME ""
#endif

#define BLUFI_NAME             "Tiremo-Cortex"
#define SCAN_TIMEOUT_MS        (8000U)
#define JOIN_TIMEOUT_MS        (20000U)
#define SNTP_TIMEOUT_MS        (20000U)
#define MQTT_CONNECT_MS        (45000U)
#define RETRY_MAX              (3U)
#define RETRY_DELAY_MS         (1000U)
#define NVS_WAIT_MS            (15000U)
#define NVS_POLL_MS            (1000U)
#define POLL_MS                (20U)
#define ADVERTISE_LOG_MS       (5000U)
#define HEARTBEAT_MS           (10000U)
#define FACTORY_RESET_HOLD_MS  (10000U)

static void FactoryResetAndReboot(void)
{
    (void)TIREMO_Persist_Clear();
    (void)TIREMO_Provision_FactoryReset();
    TIREMO_LOG_INFO("factory reset: rebooting");
    NVIC_SystemReset();
}

static bool FactoryResetHoldReached(uint32_t now)
{
    static bool pressed = false;
    static bool announced = false;
    static uint32_t pressStartMs = 0U;
    bool rawPressed;

    rawPressed = TIREMO_Button_IsPressed(TIREMO_BUTTON_USER);
    if (!rawPressed)
    {
        pressed = false;
        announced = false;
        return false;
    }

    if (!pressed)
    {
        pressed = true;
        pressStartMs = now;
        TIREMO_LOG_WARN("USER button: hold %u ms to factory-reset",
                        FACTORY_RESET_HOLD_MS);
        return false;
    }

    if ((!announced) && ((now - pressStartMs) >= FACTORY_RESET_HOLD_MS))
    {
        announced = true;
        TIREMO_LOG_WARN("USER button: factory-reset hold reached");
        return true;
    }
    return false;
}

static void Hang(const char *pWhy)
{
    TIREMO_LOG_ERROR("%s", pWhy);
    TIREMO_LOG_INFO("hold USER %u ms to factory-reset and reboot",
                    FACTORY_RESET_HOLD_MS);
    for (;;)
    {
        if (FactoryResetHoldReached(TIREMO_SysTick_GetMs()))
        {
            FactoryResetAndReboot();
        }
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
        if (FactoryResetHoldReached(TIREMO_SysTick_GetMs()))
        {
            (void)TIREMO_ESP32_BlufiStop();
            FactoryResetAndReboot();
        }
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
        TIREMO_LOG_INFO("cascade: no +PASSWORD: URC, saving SSID only "
                        "(next boot uses NVS rejoin)");
    }

    return true;
}

static bool BringUpWifi(void)
{
    TIREMO_PersistRecord_t saved;
    TIREMO_PersistRecord_t harvest;
    bool online = false;

    if (TIREMO_Persist_Load(&saved) && TIREMO_Persist_HasSsid(&saved))
    {
        TIREMO_LOG_INFO("persist: loaded SSID \"%s\" password=%s",
                        saved.ssid,
                        TIREMO_Persist_HasPassword(&saved) ? "yes" : "no");

        if (TIREMO_Persist_HasPassword(&saved))
        {
            online = prvTryScanJoin(saved.ssid, saved.password);
        }
        else
        {
            online = prvTryNvsRejoin(saved.ssid);
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
            return false;
        }
        if (harvest.ssid[0] != '\0')
        {
            (void)SaveCredentials(harvest.ssid, harvest.password);
        }
        online = true;
    }

    (void)TIREMO_ESP32_EnableAutoConnect();
    return online;
}

static bool ConnectPermanent(const char *pClientId)
{
    (void)TIREMO_ESP32_MqttDisconnect();
    return TIREMO_ESP32_MqttUserConfigMtls(pClientId) &&
           TIREMO_ESP32_MqttSni(TIREMO_AWS_ENDPOINT) &&
           TIREMO_ESP32_MqttConnect(TIREMO_AWS_ENDPOINT, 8883U,
                                    MQTT_CONNECT_MS);
}

void PRV_USER_Code(void)
{
    uint8_t eui[6];
    char deviceId[TIREMO_IDENTITY_ID_SIZE];
    char clientId[64];
    const char *pThingName;
    uint32_t epoch = 0U;
    uint32_t lastBeatMs;

    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();
    TIREMO_Button_Init();
    TIREMO_Banner_Print("Sensor Telemetry and Shadow Control");
    TIREMO_LOG_INFO("Sensor_Telemetry_Shadow: boot");
    TIREMO_LOG_INFO("hold USER %u ms to factory-reset (WiFi CS01 + claim identity)",
                    FACTORY_RESET_HOLD_MS);

    if (!TIREMO_Persist_Init())
    {
        Hang("persist: flash init failed");
    }

    TIREMO_SHT40_Init();
    if (!TIREMO_LIS2DE12_Init())
    {
        TIREMO_LOG_WARN("LIS2DE12: init failed, accel samples will be empty");
    }
    else
    {
        (void)TIREMO_LIS2DE12_Configure(2U, 25U);
    }

    if (!TIREMO_ESP32_Init() || !TIREMO_ESP32_IsPresent() ||
        !TIREMO_ESP32_DisableEcho())
    {
        Hang("ESP32: transport startup failed");
    }
    if (!TIREMO_ESP32_SetStationMode())
    {
        Hang("ESP32: CWMODE=1 failed");
    }
    if (!TIREMO_ESP32_GetHwEui(eui) ||
        !TIREMO_Identity_FromEui(eui, deviceId, sizeof(deviceId)))
    {
        Hang("identity: station MAC read/format failed");
    }
    (void)snprintf(clientId, sizeof(clientId), "%s_%s",
                   TIREMO_AWS_PRODUCT_IDENTIFIER, deviceId);
    pThingName = (TIREMO_AWS_THING_NAME[0] != '\0') ? TIREMO_AWS_THING_NAME
                                                    : clientId;
    TIREMO_LOG_INFO("identity: deviceId=%s clientId=%s thingName=%s",
                    deviceId, clientId, pThingName);

    if (!BringUpWifi())
    {
        Hang("cascade: WiFi bring-up failed");
    }
    if (!TIREMO_ESP32_EnableSntp() ||
        !TIREMO_ESP32_WaitSntpEpoch(&epoch, SNTP_TIMEOUT_MS))
    {
        Hang("ESP32: SNTP sync failed");
    }
    TIREMO_Clock_SetEpoch(epoch);
    TIREMO_LOG_INFO("ESP32: online, SNTP epoch=%lu", (unsigned long)epoch);

    if (TIREMO_Provision_IsDone(deviceId))
    {
        TIREMO_LOG_INFO("provision: already done; using permanent credentials");
        if (!ConnectPermanent(clientId))
        {
            Hang("provision: stored permanent credential connect failed");
        }
    }
    else if (!TIREMO_Provision_Run(deviceId, clientId))
    {
        Hang("provision: fleet exchange failed");
    }

    if (!TIREMO_Shadow_GetApplyReport(pThingName))
    {
        Hang("shadow: get/apply/report failed");
    }

    if (!TIREMO_Telemetry_Init(TIREMO_AWS_PRODUCT_IDENTIFIER, deviceId))
    {
        Hang("telemetry: topic build failed");
    }

    if (!TIREMO_Jobs_Init(pThingName))
    {
        Hang("jobs: subscribe / start-next probe failed");
    }

    lastBeatMs = TIREMO_SysTick_GetMs();
    for (;;)
    {
        uint32_t now = TIREMO_SysTick_GetMs();

        if (FactoryResetHoldReached(now))
        {
            FactoryResetAndReboot();
        }
        TIREMO_Shadow_Poll();
        TIREMO_Jobs_Poll();
        TIREMO_Telemetry_Poll(now);
        if ((now - lastBeatMs) >= HEARTBEAT_MS)
        {
            TIREMO_LOG_INFO("ready thingName=%s ACC=%u GYRO=%u TEMP=%u HUM=%u intervalS=%lu",
                            pThingName,
                            TIREMO_Shadow_AccEn() ? 1U : 0U,
                            TIREMO_Shadow_GyroEn() ? 1U : 0U,
                            TIREMO_Shadow_TempEn() ? 1U : 0U,
                            TIREMO_Shadow_HumEn() ? 1U : 0U,
                            (unsigned long)TIREMO_Shadow_IntervalS());
            lastBeatMs = now;
        }
    }
}
