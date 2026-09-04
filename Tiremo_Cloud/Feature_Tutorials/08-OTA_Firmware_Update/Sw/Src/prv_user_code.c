/******************************************************************************
 * @file    prv_user_code.c
 * @brief   Tutorial 07 ready loop plus a job-triggered A/B firmware update,
 *          with the ESP32 network bring-up retried until it succeeds.
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
#include "../Common/Include/tiremo_ota.h"
#include "../Common/Include/tiremo_version.h"
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

#define JOIN_TIMEOUT_MS        (100000U)
#define SNTP_TIMEOUT_MS        (20000U)
#define MQTT_CONNECT_MS        (45000U)
#define HEARTBEAT_MS           (10000U)
#define FACTORY_RESET_HOLD_MS  (10000U)

/* ESP-AT sometimes answers a join / connect with a framed \r\nERROR\r\n on
 * the first try and succeeds on the next -- retry the whole bring-up,
 * power-cycling the module between attempts, backing off 5 s -> 30 s. */
#define NET_RETRY_DELAY_MS      (5000U)
#define NET_RETRY_MAX_DELAY_MS  (30000U)

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

static bool ConnectPermanent(const char *pClientId)
{
    (void)TIREMO_ESP32_MqttDisconnect();
    return TIREMO_ESP32_MqttUserConfigMtls(pClientId) &&
           TIREMO_ESP32_MqttSni(TIREMO_AWS_ENDPOINT) &&
           TIREMO_ESP32_MqttConnect(TIREMO_AWS_ENDPOINT, 8883U,
                                    MQTT_CONNECT_MS);
}

/**
 * @brief Wait @p delayMs, still servicing the USER factory-reset hold.
 *
 * The network retry below never gives up on its own, so this keeps the
 * 10 s USER hold live during the wait -- a genuinely bad SSID/password
 * (a framed ERROR that will never clear) is still recoverable.
 */
static void RetryWait(uint32_t delayMs)
{
    uint32_t startMs = TIREMO_SysTick_GetMs();

    while ((TIREMO_SysTick_GetMs() - startMs) < delayMs)
    {
        if (FactoryResetHoldReached(TIREMO_SysTick_GetMs()))
        {
            FactoryResetAndReboot();
        }
        TIREMO_SysTick_DelayMs(20U);
    }
}

/**
 * @brief One full attempt: cold ESP32 -> WiFi -> SNTP -> Tiremo Cloud
 *        session -> shadow, telemetry, jobs.
 *
 * Every sub-step is a plain bool; the first failure aborts this attempt.
 * The caller powers the module down (TIREMO_ESP32_Deinit) before the next
 * call so a wedged module gets a real power cycle.
 *
 * @return true only when everything up to and including TIREMO_Jobs_Init
 *         succeeded.
 */
static bool NetworkBringUp(const char *pDeviceId, const char *pClientId,
                           const char *pThingName, uint32_t *pEpoch,
                           TIREMO_OtaBoot_t otaBoot)
{
    if (!TIREMO_ESP32_IsInitialized())
    {
        if (!TIREMO_ESP32_Init() || !TIREMO_ESP32_IsPresent() ||
            !TIREMO_ESP32_DisableEcho())
        {
            TIREMO_LOG_WARN("net: ESP32 transport restart failed");
            return false;
        }
    }

    (void)TIREMO_ESP32_DisableAutoConnect();
    (void)TIREMO_ESP32_Disconnect();
    if (!TIREMO_ESP32_SetStationMode() ||
        !TIREMO_ESP32_JoinAP(TIREMO_WIFI_SSID, TIREMO_WIFI_PASSWORD,
                             JOIN_TIMEOUT_MS))
    {
        TIREMO_LOG_WARN("net: WiFi join failed");
        return false;
    }
    if (!TIREMO_ESP32_EnableSntp() ||
        !TIREMO_ESP32_WaitSntpEpoch(pEpoch, SNTP_TIMEOUT_MS))
    {
        TIREMO_LOG_WARN("net: SNTP sync failed");
        return false;
    }
    TIREMO_LOG_INFO("net: online, SNTP epoch=%lu", (unsigned long)*pEpoch);

    if (TIREMO_Provision_IsDone(pDeviceId))
    {
        TIREMO_LOG_INFO("provision: already done; using permanent credentials");
        if (!ConnectPermanent(pClientId))
        {
            TIREMO_LOG_WARN("provision: permanent credential connect failed");
            return false;
        }
    }
    else if (!TIREMO_Provision_Run(pDeviceId, pClientId))
    {
        TIREMO_LOG_WARN("provision: fleet exchange failed");
        return false;
    }

    if (!TIREMO_Shadow_GetApplyReport(pThingName))
    {
        TIREMO_LOG_WARN("shadow: get/apply/report failed");
        return false;
    }
    if (!TIREMO_Telemetry_Init(TIREMO_AWS_PRODUCT_IDENTIFIER, pDeviceId))
    {
        TIREMO_LOG_WARN("telemetry: topic build failed");
        return false;
    }

    /* Close out a job that spanned the reboot -- BEFORE Jobs_Init(),
     * whose own boot start-next probe would otherwise find the same
     * still-IN_PROGRESS job and re-apply (re-download, re-swap, reboot)
     * it forever. The freshly-swapped image reports SUCCEEDED; a
     * rolled-back boot reports FAILED. Either way the hand-off record is
     * cleared so the running bank is marked good. Idempotent across a
     * retried NetworkBringUp -- ConfirmRunning() clears the pending id. */
    if (otaBoot == TIREMO_OTA_BOOT_CONFIRM_PENDING)
    {
        TIREMO_LOG_INFO("ota: reporting job %s SUCCEEDED (now fw=%s)",
                        TIREMO_OTA_PendingJobId(), TIREMO_FW_VERSION);
        TIREMO_Jobs_ReportResultDirect(pThingName, TIREMO_OTA_PendingJobId(),
                                       true, NULL);
        TIREMO_OTA_ConfirmRunning();
    }
    else if (otaBoot == TIREMO_OTA_BOOT_ROLLED_BACK)
    {
        TIREMO_LOG_WARN("ota: reporting job %s FAILED (%s)",
                        TIREMO_OTA_PendingJobId(),
                        TIREMO_OTA_RollbackReason());
        TIREMO_Jobs_ReportResultDirect(pThingName, TIREMO_OTA_PendingJobId(),
                                       false, TIREMO_OTA_RollbackReason());
        TIREMO_OTA_ConfirmRunning();
    }

    if (!TIREMO_Jobs_Init(pThingName))
    {
        TIREMO_LOG_WARN("jobs: subscribe / start-next probe failed");
        return false;
    }
    return true;
}

void PRV_USER_Code(void)
{
    uint8_t eui[6];
    char deviceId[TIREMO_IDENTITY_ID_SIZE];
    char clientId[64];
    const char *pThingName;
    uint32_t epoch = 0U;
    uint32_t lastBeatMs;
    TIREMO_OtaBoot_t otaBoot;

    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();
    TIREMO_Button_Init();
    TIREMO_LOG_INFO("OTA_Firmware_Update: boot fw=%s", TIREMO_FW_VERSION);

    /* Runs before anything else touches the radio: forces CFMC multi-bank
     * mode (one-time reset on a fresh board) and reads the DFMC hand-off
     * record. A boot that came from a just-applied update, or one that
     * needs to roll a failed update back, is decided here. */
    otaBoot = TIREMO_OTA_Init();

    TIREMO_LOG_INFO("hold USER %u ms to factory-reset; tap USER to raise TEMP_HIGH; queue a FIRMWARE_UPDATE job in the Tiremo UI",
                    FACTORY_RESET_HOLD_MS);

    /* ESP32 up + station MAC -> identity. A power cycle clears most
     * first-try AT hiccups; only a truly dead module loops here forever
     * (the USER hold still reboots). */
    for (uint32_t attempt = 1U; ; attempt++)
    {
        if (TIREMO_ESP32_Init() && TIREMO_ESP32_IsPresent() &&
            TIREMO_ESP32_DisableEcho() &&
            TIREMO_ESP32_GetHwEui(eui) &&
            TIREMO_Identity_FromEui(eui, deviceId, sizeof(deviceId)))
        {
            break;
        }
        TIREMO_LOG_WARN("ESP32: startup/identity attempt %lu failed -- power-cycling",
                        (unsigned long)attempt);
        TIREMO_ESP32_Deinit();
        RetryWait(NET_RETRY_DELAY_MS);
    }

    (void)snprintf(clientId, sizeof(clientId), "%s_%s",
                   TIREMO_AWS_PRODUCT_IDENTIFIER, deviceId);
    pThingName = (TIREMO_AWS_THING_NAME[0] != '\0') ? TIREMO_AWS_THING_NAME
                                                    : clientId;
    TIREMO_LOG_INFO("identity: deviceId=%s clientId=%s thingName=%s",
                    deviceId, clientId, pThingName);

    /* Retry the full WiFi -> SNTP -> provision -> shadow -> jobs bring-up
     * until it comes up clean, power-cycling the ESP32 each round
     * (5 s -> 30 s backoff). */
    {
        uint32_t attempt = 0U;
        uint32_t delayMs = NET_RETRY_DELAY_MS;

        for (;;)
        {
            attempt++;
            if (NetworkBringUp(deviceId, clientId, pThingName, &epoch, otaBoot))
            {
                TIREMO_LOG_INFO("net: session up after %lu attempt(s)",
                                (unsigned long)attempt);
                break;
            }
            TIREMO_LOG_WARN("net: attempt %lu failed -- power-cycling ESP32, retry in %lu ms",
                            (unsigned long)attempt, (unsigned long)delayMs);
            TIREMO_ESP32_Deinit();
            RetryWait(delayMs);
            delayMs *= 2U;
            if (delayMs > NET_RETRY_MAX_DELAY_MS)
            {
                delayMs = NET_RETRY_MAX_DELAY_MS;
            }
        }
    }

    /* The pending OTA job (if any) was already reported and confirmed
     * inside NetworkBringUp(), before Jobs_Init()'s boot start-next probe
     * could re-claim it. */

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
