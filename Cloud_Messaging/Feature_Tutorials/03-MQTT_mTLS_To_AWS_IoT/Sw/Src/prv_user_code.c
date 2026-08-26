/******************************************************************************
 * @file    prv_user_code.c
 * @brief   MQTT mTLS to AWS IoT Core -- load the three credential slots,
 *          configure a scheme-5 mutual-TLS MQTT identity, set SNI, and
 *          attempt the connection. No subscribe/publish yet -- this is
 *          just proving the mTLS handshake itself succeeds.
 *
 * WiFi credentials come from wifi_secrets.h (copy wifi_secrets.h.example),
 * same as every other Cloud Messaging / ESP32 Connectivity tutorial. The
 * three mTLS credentials come from certs/root_ca.pem (real, public --
 * Amazon's own Root CA 1) and certs/device_cert.pem / certs/device_key.pem
 * (YOUR real AWS IoT Thing's certificate and private key -- gitignored,
 * copy from the .example templates in certs/ and fill in your own). MQTT
 * endpoint and client ID are plain #defines below -- replace both with
 * your own AWS IoT Core endpoint and Thing name/client ID before building.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Common/Include/tiremo_systick.h"
#include "../Common/Include/tiremo_debug.h"
#include "../Drivers/esp32/Include/tiremo_esp32.h"
#include "wifi_secrets.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifndef TIREMO_WIFI_SSID
#error "Copy Sw/wifi_secrets.h.example to Sw/wifi_secrets.h and set TIREMO_WIFI_SSID / TIREMO_WIFI_PASSWORD"
#endif

/* Real AWS IoT credentials embedded from the certs/ folder's PEM files
 * at build time -- see cmake/embed_pem.cmake and this tutorial's
 * CMakeLists.txt. */
extern const unsigned char g_rootCaPem[];
extern const size_t g_rootCaPem_len;
extern const unsigned char g_deviceCertPem[];
extern const size_t g_deviceCertPem_len;
extern const unsigned char g_deviceKeyPem[];
extern const size_t g_deviceKeyPem_len;

#define JOIN_TIMEOUT_MS          (20000U)
#define SNTP_TIMEOUT_MS          (20000U)

/* Replace both with your own AWS IoT Core setup -- see this tutorial's
 * README for how to find your account's endpoint and register a Thing. */
#define MQTT_ENDPOINT             "iot.tiremo.ai"
#define MQTT_CLIENT_ID            "trackerfzfo5_TiremoCortex"

#define MQTT_PORT                 (8883U)
#define MQTT_CONNECT_TIMEOUT_MS   (45000U)

static void Hang(const char *pWhy)
{
    TIREMO_LOG_ERROR("%s", pWhy);
    for (;;)
    {
    }
}

/**
 * @brief Write one credential slot and log the outcome.
 */
static bool LoadCred(TIREMO_ESP32_CredSlot_t slot, const char *pLabel,
                     const unsigned char *pData, uint32_t length)
{
    TIREMO_LOG_INFO("%s: writing %lu bytes...", pLabel, (unsigned long)length);
    if (!TIREMO_ESP32_CredWrite(slot, pData, length))
    {
        TIREMO_LOG_ERROR("%s: SYSMFG write failed", pLabel);
        return false;
    }
    TIREMO_LOG_INFO("%s: stored", pLabel);
    return true;
}

void PRV_USER_Code(void)
{
    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();

    TIREMO_LOG_INFO("MQTT_mTLS_To_AWS_IoT: boot");

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

    if ((!LoadCred(TIREMO_ESP32_CREDSLOT_ROOT_CA, "ROOT_CA",
                  g_rootCaPem, (uint32_t)g_rootCaPem_len)) ||
        (!LoadCred(TIREMO_ESP32_CREDSLOT_CLIENT_CERT, "CLIENT_CERT",
                  g_deviceCertPem, (uint32_t)g_deviceCertPem_len)) ||
        (!LoadCred(TIREMO_ESP32_CREDSLOT_CLIENT_KEY, "CLIENT_KEY",
                  g_deviceKeyPem, (uint32_t)g_deviceKeyPem_len)))
    {
        Hang("Credentials: failed to load one or more slots");
    }
    TIREMO_LOG_INFO("Credentials: all three slots loaded");

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

    /* Scheme 5 verifies the server certificate, which means checking its
     * validity dates -- the module has no RTC at boot, so AT+MQTTCONN
     * fails until it has obtained the current time via SNTP. */
    if (!TIREMO_ESP32_EnableSntp())
    {
        Hang("ESP32: CIPSNTPCFG failed");
    }
    {
        uint32_t epoch = 0U;
        if (!TIREMO_ESP32_WaitSntpEpoch(&epoch, SNTP_TIMEOUT_MS))
        {
            Hang("ESP32: SNTP time never synced");
        }
        TIREMO_LOG_INFO("ESP32: SNTP synced (epoch %lu)", (unsigned long)epoch);
    }

    /* Proven order: USERCFG -> SNI -> CONN. */
    if (!TIREMO_ESP32_MqttUserConfigMtls(MQTT_CLIENT_ID))
    {
        Hang("MQTT: MQTTUSERCFG (scheme 5) failed");
    }
    TIREMO_LOG_INFO("MQTT: client id \"%s\" configured (scheme 5, mutual TLS)",
                    MQTT_CLIENT_ID);

    if (!TIREMO_ESP32_MqttSni(MQTT_ENDPOINT))
    {
        Hang("MQTT: MQTTSNI failed");
    }
    TIREMO_LOG_INFO("MQTT: SNI set to \"%s\"", MQTT_ENDPOINT);

    TIREMO_LOG_INFO("MQTT: connecting to %s:%u", MQTT_ENDPOINT, MQTT_PORT);
    if (!TIREMO_ESP32_MqttConnect(MQTT_ENDPOINT, MQTT_PORT,
                                  MQTT_CONNECT_TIMEOUT_MS))
    {
        Hang("MQTT: MQTTCONN failed");
    }
    TIREMO_LOG_INFO("MQTT: connected (mutual TLS) -- MQTTUSERCFG/MQTTSNI/MQTTCONN all OK");

    for (;;)
    {
        TIREMO_SysTick_DelayMs(10000U);
        TIREMO_LOG_INFO("MQTT_mTLS_To_AWS_IoT: idle (connect already succeeded once at boot, no subscribe/publish attempted)");
    }
}
