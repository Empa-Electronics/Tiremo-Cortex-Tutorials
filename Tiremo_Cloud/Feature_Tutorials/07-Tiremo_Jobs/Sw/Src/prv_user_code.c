/******************************************************************************
 * @file    prv_user_code.c
 * @brief   Tutorial 06 ready loop plus Tiremo Jobs health-check.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "abov_config.h"
#include "../Board/board_config.h"
#include "../Common/Include/tiremo_systick.h"
#include "../Common/Include/tiremo_debug.h"
#include "../Common/Include/tiremo_identity.h"
#include "../Common/Include/tiremo_provision.h"
#include "../Common/Include/tiremo_shadow.h"
#include "../Common/Include/tiremo_telemetry.h"
#include "../Common/Include/tiremo_jobs.h"
#include "../Drivers/esp32/Include/tiremo_esp32.h"
#include "../Drivers/button/Include/tiremo_button.h"
#include "wifi_secrets.h"
#include "aws_iot_config.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef TIREMO_WIFI_SSID
#error "Copy Sw/wifi_secrets.h.example to Sw/wifi_secrets.h and set TIREMO_WIFI_SSID / TIREMO_WIFI_PASSWORD"
#endif

#ifndef TIREMO_AWS_ENDPOINT
#error "Copy Sw/aws_iot_config.h.example to Sw/aws_iot_config.h and fill in endpoint / template / product values"
#endif

#ifndef TIREMO_AWS_THING_NAME
#define TIREMO_AWS_THING_NAME ""
#endif

#define JOIN_TIMEOUT_MS        (20000U)
#define SNTP_TIMEOUT_MS        (20000U)
#define MQTT_CONNECT_MS        (45000U)
#define HEARTBEAT_MS           (10000U)
#define FACTORY_RESET_HOLD_MS  (10000U)

static void FactoryResetAndReboot(void)
{
    (void)TIREMO_Provision_FactoryReset();
    TIREMO_LOG_INFO("factory reset: rebooting");
    NVIC_SystemReset();
}

/**
 * @brief Same shape as production factory-reset hold: raw press, 10 s
 *        continuous, fire at the threshold without waiting for release.
 *        Bounce is irrelevant at this timescale (Getting Started 04).
 */
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
    TIREMO_LOG_INFO("Tiremo_Jobs: boot");
    TIREMO_LOG_INFO("hold USER %u ms to factory-reset; tap USER to raise TEMP_HIGH; queue a health-check job in the Tiremo UI",
                    FACTORY_RESET_HOLD_MS);

    if (!TIREMO_ESP32_Init() || !TIREMO_ESP32_IsPresent() ||
        !TIREMO_ESP32_DisableEcho())
    {
        Hang("ESP32: transport startup failed");
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

    (void)TIREMO_ESP32_DisableAutoConnect();
    (void)TIREMO_ESP32_Disconnect();
    if (!TIREMO_ESP32_SetStationMode() ||
        !TIREMO_ESP32_JoinAP(TIREMO_WIFI_SSID, TIREMO_WIFI_PASSWORD,
                             JOIN_TIMEOUT_MS))
    {
        Hang("ESP32: WiFi join failed");
    }
    if (!TIREMO_ESP32_EnableSntp() ||
        !TIREMO_ESP32_WaitSntpEpoch(&epoch, SNTP_TIMEOUT_MS))
    {
        Hang("ESP32: SNTP sync failed");
    }
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
        TIREMO_Telemetry_Poll(now, TIREMO_Button_IsPressed(TIREMO_BUTTON_USER));
        if ((now - lastBeatMs) >= HEARTBEAT_MS)
        {
            TIREMO_LOG_INFO("ready thingName=%s telemetry=%s intervalS=%lu",
                            pThingName,
                            TIREMO_Shadow_ReportingEnabled() ? "on" : "off",
                            (unsigned long)TIREMO_Shadow_IntervalS());
            lastBeatMs = now;
        }
    }
}
