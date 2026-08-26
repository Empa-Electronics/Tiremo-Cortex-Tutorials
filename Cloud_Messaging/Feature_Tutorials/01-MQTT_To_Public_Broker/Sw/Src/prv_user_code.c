/******************************************************************************
 * @file    prv_user_code.c
 * @brief   MQTT to a Public Broker -- connect, subscribe, publish (both
 *          string and binary-safe), and confirm the message echoes back.
 *
 * WiFi credentials come from wifi_secrets.h (copy wifi_secrets.h.example),
 * same as ESP32 Connectivity. MQTT identity/broker are plain #defines
 * below, not board_config.h -- they're application/network parameters,
 * not board wiring.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Common/Include/tiremo_systick.h"
#include "../Common/Include/tiremo_debug.h"
#include "../Drivers/esp32/Include/tiremo_esp32.h"
#include "wifi_secrets.h"

#include <stdint.h>
#include <stdio.h>

#ifndef TIREMO_WIFI_SSID
#error "Copy Sw/wifi_secrets.h.example to Sw/wifi_secrets.h and set TIREMO_WIFI_SSID / TIREMO_WIFI_PASSWORD"
#endif

#define JOIN_TIMEOUT_MS      (20000U)
#define MQTT_CLIENT_ID       "tiremo-cortex-01"
#define MQTT_BROKER_HOST     "broker.emqx.io"
#define MQTT_BROKER_PORT     (1883U)
#define MQTT_CONNECT_TIMEOUT_MS (10000U)
#define MQTT_TOPIC           "tiremo-cortex/mqtt-demo"
#define MQTT_WAIT_TIMEOUT_MS (5000U)
#define PUBLISH_INTERVAL_MS  (5000U)

static void Hang(const char *pWhy)
{
    TIREMO_LOG_ERROR("%s", pWhy);
    for (;;)
    {
    }
}

/**
 * @brief Render payload bytes as printable ASCII (non-printable -> '.')
 *        so both the text and binary-safe demo messages log readably.
 */
static void LogPayload(const char *pPrefix, const uint8_t *pPayload,
                       uint32_t length)
{
    char buf[80];
    uint32_t n = (length < (sizeof(buf) - 1U)) ? length : (sizeof(buf) - 1U);
    uint32_t i;

    for (i = 0U; i < n; i++)
    {
        uint8_t ch = pPayload[i];
        buf[i] = ((ch >= 0x20U) && (ch < 0x7FU)) ? (char)ch : '.';
    }
    buf[n] = '\0';

    TIREMO_LOG_INFO("%s: %lu bytes \"%s\"", pPrefix, (unsigned long)length, buf);
}

void PRV_USER_Code(void)
{
    char textPayload[48];
    char topic[64];
    uint8_t rxPayload[64];
    uint8_t rawPayload[4];
    uint32_t rxLen;
    uint32_t now;
    uint32_t lastPublishMs;
    uint32_t counter = 0U;

    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();

    TIREMO_LOG_INFO("MQTT_To_Public_Broker: boot");

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

    TIREMO_LOG_INFO("ESP32: joining SSID \"%s\"", TIREMO_WIFI_SSID);
    if (!TIREMO_ESP32_JoinAP(TIREMO_WIFI_SSID, TIREMO_WIFI_PASSWORD,
                             JOIN_TIMEOUT_MS))
    {
        Hang("ESP32: CWJAP did not report WIFI GOT IP");
    }
    TIREMO_LOG_INFO("ESP32: WIFI GOT IP");

    if (!TIREMO_ESP32_MqttUserConfig(MQTT_CLIENT_ID, "", ""))
    {
        Hang("MQTT: MQTTUSERCFG failed");
    }
    TIREMO_LOG_INFO("MQTT: client id \"%s\" configured (scheme 1, no TLS)",
                    MQTT_CLIENT_ID);

    TIREMO_LOG_INFO("MQTT: connecting to %s:%u", MQTT_BROKER_HOST,
                    MQTT_BROKER_PORT);
    if (!TIREMO_ESP32_MqttConnect(MQTT_BROKER_HOST, MQTT_BROKER_PORT,
                                  MQTT_CONNECT_TIMEOUT_MS))
    {
        Hang("MQTT: MQTTCONN failed");
    }
    TIREMO_LOG_INFO("MQTT: connected");

    if (!TIREMO_ESP32_MqttSubscribe(MQTT_TOPIC, 0U))
    {
        Hang("MQTT: MQTTSUB failed");
    }
    TIREMO_LOG_INFO("MQTT: subscribed to \"%s\"", MQTT_TOPIC);

    lastPublishMs = TIREMO_SysTick_GetMs() - PUBLISH_INTERVAL_MS;

    for (;;)
    {
        now = TIREMO_SysTick_GetMs();

        if ((now - lastPublishMs) < PUBLISH_INTERVAL_MS)
        {
            continue;
        }
        lastPublishMs = now;

        if ((counter % 2U) == 0U)
        {
            (void)snprintf(textPayload, sizeof(textPayload),
                           "hello from Tiremo Cortex #%lu",
                           (unsigned long)counter);
            TIREMO_LOG_INFO("MQTT: publishing (string) \"%s\"", textPayload);
            if (!TIREMO_ESP32_MqttPublish(MQTT_TOPIC, textPayload, 0U, false))
            {
                TIREMO_LOG_WARN("MQTT: MqttPublish failed");
            }
        }
        else
        {
            rawPayload[0] = (uint8_t)((counter >> 24) & 0xFFU);
            rawPayload[1] = (uint8_t)((counter >> 16) & 0xFFU);
            rawPayload[2] = (uint8_t)((counter >> 8) & 0xFFU);
            rawPayload[3] = (uint8_t)(counter & 0xFFU);
            TIREMO_LOG_INFO("MQTT: publishing (raw) counter=%lu as 4 bytes",
                            (unsigned long)counter);
            if (!TIREMO_ESP32_MqttPublishRaw(MQTT_TOPIC, rawPayload,
                                             sizeof(rawPayload), 0U, false))
            {
                TIREMO_LOG_WARN("MQTT: MqttPublishRaw failed");
            }
        }
        counter++;

        rxLen = 0U;
        if (TIREMO_ESP32_MqttWaitMessage(topic, sizeof(topic), rxPayload,
                                         sizeof(rxPayload), &rxLen,
                                         MQTT_WAIT_TIMEOUT_MS))
        {
            TIREMO_LOG_INFO("MQTT: echo received on \"%s\"", topic);
            LogPayload("MQTT: echo payload", rxPayload, rxLen);
        }
        else
        {
            TIREMO_LOG_WARN("MQTT: no echo received within %u ms",
                            MQTT_WAIT_TIMEOUT_MS);
        }
    }
}
