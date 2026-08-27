/******************************************************************************
 * @file    prv_user_code.c
 * @brief   Windowed LIS2DE12 inference published on MQTT after session.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "abov_config.h"
#include "../Board/board_config.h"
#include "../Common/Include/tiremo_accel_window.h"
#include "../Common/Include/tiremo_banner.h"
#include "../Common/Include/tiremo_clock.h"
#include "../Common/Include/tiremo_debug.h"
#include "../Common/Include/tiremo_identity.h"
#include "../Common/Include/tiremo_provision.h"
#include "../Common/Include/tiremo_shadow.h"
#include "../Common/Include/tiremo_systick.h"
#include "../Drivers/button/Include/tiremo_button.h"
#include "../Drivers/esp32/Include/tiremo_esp32.h"
#include "../Drivers/lis2de12/Include/tiremo_lis2de12.h"
#include "tiremo_api.h"
#include "wifi_secrets.h"
#include "aws_iot_config.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

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
#define FACTORY_RESET_HOLD_MS  (10000U)

static void FactoryResetAndReboot(void)
{
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

static bool ConnectPermanent(const char *pClientId)
{
    (void)TIREMO_ESP32_MqttDisconnect();
    return TIREMO_ESP32_MqttUserConfigMtls(pClientId) &&
           TIREMO_ESP32_MqttSni(TIREMO_AWS_ENDPOINT) &&
           TIREMO_ESP32_MqttConnect(TIREMO_AWS_ENDPOINT, 8883U,
                                    MQTT_CONNECT_MS);
}

static bool InitAccel(void)
{
    uint8_t whoAmI = 0U;

    if (!TIREMO_LIS2DE12_Init())
    {
        return false;
    }

    if (!TIREMO_LIS2DE12_Configure(TIREMO_ACCEL_SCALE_G, TIREMO_ACCEL_HW_ODR_HZ))
    {
        return false;
    }

    (void)TIREMO_LIS2DE12_ReadWhoAmI(&whoAmI);
    TIREMO_LOG_INFO("LIS2DE12: WHO_AM_I=0x%02X  FS=%u g  ODR=%u Hz  pace=%u ms",
                    whoAmI,
                    (unsigned)TIREMO_ACCEL_SCALE_G,
                    (unsigned)TIREMO_ACCEL_HW_ODR_HZ,
                    (unsigned)TIREMO_ACCEL_SAMPLE_INTERVAL_MS);
    return true;
}

static int64_t UnixMsNow(void)
{
    uint32_t epoch = 0U;
    uint32_t now = TIREMO_SysTick_GetMs();

    (void)TIREMO_Clock_GetEpoch(&epoch);
    return ((int64_t)epoch * 1000LL) + (int64_t)(now % 1000U);
}

static bool RunWindow(const char *pTopic,
                      const TIREMO_AccelWindow_t *pWindow,
                      int64_t windowStartMs,
                      int64_t windowFinishMs)
{
    TiremoInferenceResult result;
    char json[1024];
    uint32_t t0;
    uint32_t inferMs;
    int status;
    int jsonStatus;

    t0 = TIREMO_SysTick_GetMs();
    status = tiremo_ai_run(TIREMO_AccelWindow_Data(pWindow), &result);
    inferMs = TIREMO_SysTick_GetMs() - t0;

    if (status != TIREMO_OK)
    {
        TIREMO_LOG_WARN("inference: tiremo_ai_run failed (%d) %s",
                        status, tiremo_ai_strerror(status));
        return false;
    }

    TIREMO_LOG_INFO("predicted_class_label=%s infer_ms=%u",
                    (result.predicted_class_label != NULL)
                        ? result.predicted_class_label
                        : "?",
                    (unsigned)inferMs);

    jsonStatus = tiremo_ai_format_mqtt_json(&result,
                                            windowStartMs,
                                            windowFinishMs,
                                            (int32_t)inferMs,
                                            json,
                                            sizeof(json));
    if (jsonStatus != TIREMO_OK)
    {
        TIREMO_LOG_WARN("inference: format json failed (%d)", jsonStatus);
        return false;
    }

    return TIREMO_ESP32_MqttPublishRaw(pTopic,
                                       (const uint8_t *)json,
                                       (uint32_t)strlen(json),
                                       0U,
                                       false);
}

void PRV_USER_Code(void)
{
    uint8_t eui[6];
    char deviceId[TIREMO_IDENTITY_ID_SIZE];
    char clientId[64];
    char topic[96];
    const char *pThingName;
    uint32_t epoch = 0U;
    uint32_t lastSampleMs;
    TIREMO_AccelWindow_t window;
    int64_t windowStartMs = 0;
    int n;

    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();
    TIREMO_Button_Init();
    TIREMO_Banner_Print("HMR Edge Deployment");
    TIREMO_LOG_INFO("hold USER %u ms to factory-reset (erase issued cert + marker)",
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

    if (!InitAccel())
    {
        Hang("LIS2DE12: init or Configure failed");
    }

    n = snprintf(topic, sizeof(topic), "pub/%s/%s/%s",
                 TIREMO_AWS_PRODUCT_IDENTIFIER, deviceId,
                 TIREMO_MQTT_TOPIC_SUFFIX);
    if ((n < 0) || ((uint32_t)n >= sizeof(topic)))
    {
        Hang("inference: topic overflow");
    }
    TIREMO_LOG_INFO("inference: topic=%s", topic);

    TIREMO_AccelWindow_Reset(&window);
    lastSampleMs = TIREMO_SysTick_GetMs();
    for (;;)
    {
        uint32_t now = TIREMO_SysTick_GetMs();
        int16_t xMg;
        int16_t yMg;
        int16_t zMg;

        if (FactoryResetHoldReached(now))
        {
            FactoryResetAndReboot();
        }
        TIREMO_Shadow_Poll();

        if ((now - lastSampleMs) >= TIREMO_ACCEL_SAMPLE_INTERVAL_MS)
        {
            lastSampleMs = now;
            if (!TIREMO_LIS2DE12_ReadAccel(&xMg, &yMg, &zMg))
            {
                TIREMO_LOG_WARN("LIS2DE12: read failed");
            }
            else
            {
                if (window.count == 0U)
                {
                    windowStartMs = UnixMsNow();
                }

                if (TIREMO_AccelWindow_Push(&window,
                                            (float)xMg,
                                            (float)yMg,
                                            (float)zMg) &&
                    TIREMO_AccelWindow_IsFull(&window))
                {
                    if (!RunWindow(topic, &window, windowStartMs, UnixMsNow()))
                    {
                        TIREMO_LOG_WARN("inference: publish failed");
                    }
                    TIREMO_AccelWindow_Reset(&window);
                }
            }
        }
    }
}
