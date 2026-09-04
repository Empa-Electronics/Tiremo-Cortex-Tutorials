/******************************************************************************
 * @file    tiremo_esp32.c
 * @brief   ESP32 AT-command transport, extended with a plain-TCP MQTT
 *          client (Cloud Messaging tutorial 01) and a raw TCP/SSL
 *          passthrough HTTP GET for OTA image downloads (Tiremo Cloud
 *          tutorial 08 -- see the HTTP download section near the bottom
 *          of this file).
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "../Include/tiremo_esp32.h"
#include "../../../Board/board_config.h"
#include "../../../Tiremo_Hal/Include/tiremo_hal_uart.h"
#include "../../../Tiremo_Hal/Include/tiremo_hal_gpio.h"
#include "../../../Common/Include/tiremo_systick.h"
#include "../../../Common/Include/tiremo_debug.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

#define TIREMO_ESP32_UART_PORT           TIREMO_BOARD_ESP32_UART_PORT
#define TIREMO_ESP32_UART_IRQ_PRIORITY   (3U)

#define TIREMO_ESP32_PWR_PORT            TIREMO_BOARD_ESP32_PWR_PORT
#define TIREMO_ESP32_PWR_PIN             TIREMO_BOARD_ESP32_PWR_PIN
#define TIREMO_ESP32_PWR_ON              TIREMO_BOARD_ESP32_PWR_ON
#define TIREMO_ESP32_PWR_OFF             TIREMO_BOARD_ESP32_PWR_OFF
#define TIREMO_ESP32_PWR_OFF_MS          TIREMO_BOARD_ESP32_PWR_OFF_MS
#define TIREMO_ESP32_PWR_BOOT_MS         TIREMO_BOARD_ESP32_PWR_BOOT_MS

/** @brief Must hold the complete create-certificate MQTT URC (PEM cert,
 * private key, and ownership token escaped inside JSON) plus a trailing
 * +MQTTPUB:OK that can arrive in the same burst. */
#define TIREMO_ESP32_RX_RING_SIZE        (8192U)
#define TIREMO_ESP32_RX_MASK             (TIREMO_ESP32_RX_RING_SIZE - 1U)

#define TIREMO_ESP32_RESPONSE_BUF_SIZE   (2048U)
/* Big enough for one whole FOTA execution document should the module
 * ever deliver it as a single +MQTTSUBRECV rather than ~1 KB fragments. */
#define TIREMO_ESP32_MQTT_RX_SIZE        (13312U)
#define TIREMO_ESP32_MQTT_PUB_TIMEOUT_MS    (10000U)
#define TIREMO_ESP32_DEFAULT_CMD_TIMEOUT_MS (1000U)

/* The '>' prompt after AT+MQTTPUBRAW can lag well past the 1 s default
 * command timeout when the module is mid-delivery of an inbound
 * +MQTTSUBRECV URC (the Jobs flow publishes from inside inbound
 * handling). Give it room, and retry the whole publish a few times --
 * a jobs/shadow status update is idempotent, so a duplicate is harmless. */
#define TIREMO_ESP32_MQTT_PROMPT_TIMEOUT_MS (3000U)
#define TIREMO_ESP32_MQTT_PUBRAW_RETRIES    (3U)
#define TIREMO_ESP32_MQTT_PUBRAW_RETRY_MS   (300U)
#define TIREMO_ESP32_STATUS_TIMEOUT_MS      (2000U)
#define TIREMO_ESP32_SSID_MAX_LEN           (33U)
#define TIREMO_ESP32_PASSWORD_MAX_LEN       (65U)
#define TIREMO_ESP32_CRED_TIMEOUT_MS        (5000U)

/** @brief Sanity floor for a parsed SNTP epoch -- 2020-01-01 UTC. Rejects
 *  the unsynced "Jan 1 1970" default the module reports before SNTP
 *  actually completes. */
#define TIREMO_ESP32_SNTP_MIN_EPOCH         (1577836800UL)
#define TIREMO_ESP32_SNTP_POLL_MS           (1000U)

/******************************************************************************
 * Private Types
 *****************************************************************************/

typedef struct
{
    volatile uint32_t head;
    volatile uint32_t tail;
    uint8_t data[TIREMO_ESP32_RX_RING_SIZE];
} TIREMO_ESP32_RxRing_t;

/******************************************************************************
 * Private Variables
 *****************************************************************************/

static bool s_initialized = false;
static volatile bool s_rxArmPending = false;
static TIREMO_ESP32_RxRing_t s_rxRing;
static uint8_t s_rxByte = 0U;
static char s_mqttRx[TIREMO_ESP32_MQTT_RX_SIZE];
static uint32_t s_mqttRxLen = 0U;

/******************************************************************************
 * Private Function Prototypes
 *****************************************************************************/

static bool prvESP32RxRingIsFull(void);
static bool prvESP32RxRingIsEmpty(void);
static void prvESP32RxRingPush(uint8_t byte);
static bool prvESP32RxRingPop(uint8_t *pByte);
static void prvESP32RxRingFlush(void);
static void prvESP32UARTCallback(uint32_t event, void *pContext);
static void prvESP32ArmRx(void);
static void prvESP32LogResponse(const char *pLabel,
                                const char *pResponse,
                                uint32_t length);
static void prvESP32PowerCycle(void);
static bool prvESP32SendCommandCapture(const char *pCommand,
                                       const char *pExpected,
                                       uint32_t timeoutMs,
                                       char *pResponseOut,
                                       uint32_t responseOutSize);
static bool prvESP32TryParseSubRecv(const char *pBuffer,
                                    uint32_t bufferLen,
                                    char *pTopicOut,
                                    uint32_t topicOutSize,
                                    uint8_t *pPayloadOut,
                                    uint32_t payloadOutSize,
                                    uint32_t *pPayloadLenOut,
                                    uint32_t *pConsumedOut);
static void prvESP32MqttHoldShift(uint32_t consumed);
static bool prvESP32CredSlotInfo(TIREMO_ESP32_CredSlot_t slot,
                                 const char **ppNamespace,
                                 const char **ppKey);
static bool prvESP32TryParseSysmfgValue(const char *pBuffer,
                                        uint32_t bufferLen,
                                        uint8_t *pOut,
                                        uint32_t outSize,
                                        uint32_t *pLengthOut);
static int prvMonthFromName(const char *pName);
static bool prvIsLeapYear(uint32_t year);
static bool prvCivilToEpoch(uint32_t year, uint32_t month, uint32_t day,
                            uint32_t hour, uint32_t minute, uint32_t second,
                            uint32_t *pEpoch);
static bool prvParseUint(const char **pp, int *pOut);
static bool prvParseToken(const char **pp, char *pOut, uint32_t outSize);
static bool prvParseSntpResponse(const char *pResponse, uint32_t *pEpoch);
static int32_t prvHexNibble(char c);
static bool prvParseMacQuoted(const char *pResponse, uint8_t pEui[6]);

/******************************************************************************
 * Public Functions
 *****************************************************************************/

bool TIREMO_ESP32_Init(void)
{
    TIREMO_HAL_UART_Status_t status;

    if (s_initialized)
    {
        return true;
    }

    (void)TIREMO_HAL_GPIO_SetMode(TIREMO_ESP32_PWR_PORT,
                                  TIREMO_ESP32_PWR_PIN,
                                  TIREMO_HAL_GPIO_MODE_OUTPUT_PP);

    s_rxRing.head = 0U;
    s_rxRing.tail = 0U;

    prvESP32PowerCycle();

    status = TIREMO_HAL_UART_SetInterruptHandler(TIREMO_ESP32_UART_PORT,
                                                 prvESP32UARTCallback,
                                                 NULL,
                                                 TIREMO_ESP32_UART_IRQ_PRIORITY);
    if (status != TIREMO_HAL_UART_STATUS_OK)
    {
        TIREMO_LOG_WARN("TIREMO_ESP32_Init: UART%d IRQ registration failed",
                        (int)TIREMO_ESP32_UART_PORT);
        return false;
    }

    s_initialized = true;
    prvESP32ArmRx();

    TIREMO_LOG_INFO("TIREMO_ESP32_Init: UART%d ready",
                    (int)TIREMO_ESP32_UART_PORT);

    return true;
}

void TIREMO_ESP32_Deinit(void)
{
    if (!s_initialized)
    {
        return;
    }

    s_initialized = false;
    (void)TIREMO_HAL_UART_Abort(TIREMO_ESP32_UART_PORT);
}

bool TIREMO_ESP32_IsInitialized(void)
{
    return s_initialized;
}

bool TIREMO_ESP32_SendCommand(const char *pCommand,
                              const char *pExpected,
                              uint32_t timeoutMs)
{
    return prvESP32SendCommandCapture(pCommand, pExpected, timeoutMs,
                                      NULL, 0U);
}

bool TIREMO_ESP32_SendCommandCapture(const char *pCommand,
                                     const char *pExpected,
                                     uint32_t timeoutMs,
                                     char *pResponseOut,
                                     uint32_t responseOutSize)
{
    return prvESP32SendCommandCapture(pCommand, pExpected, timeoutMs,
                                      pResponseOut, responseOutSize);
}

bool TIREMO_ESP32_IsPresent(void)
{
    return TIREMO_ESP32_SendCommand("AT\r\n",
                                    "OK",
                                    TIREMO_ESP32_DEFAULT_CMD_TIMEOUT_MS);
}

bool TIREMO_ESP32_DisableEcho(void)
{
    bool ok = TIREMO_ESP32_SendCommand("ATE0\r\n",
                                       "OK",
                                       TIREMO_ESP32_DEFAULT_CMD_TIMEOUT_MS);

    if (ok)
    {
        TIREMO_LOG_INFO("ESP32: command echo disabled (ATE0)");
    }
    else
    {
        TIREMO_LOG_WARN("ESP32: ATE0 failed; command echo remains on");
    }

    return ok;
}

bool TIREMO_ESP32_GetHwEui(uint8_t pEui[6])
{
    char response[TIREMO_ESP32_RESPONSE_BUF_SIZE];

    if (pEui == NULL)
    {
        return false;
    }
    if (!TIREMO_ESP32_SendCommandCapture("AT+CIPSTAMAC?\r\n", "OK",
                                         TIREMO_ESP32_STATUS_TIMEOUT_MS,
                                         response, sizeof(response)))
    {
        return false;
    }
    return prvParseMacQuoted(response, pEui);
}

bool TIREMO_ESP32_SetStationMode(void)
{
    return TIREMO_ESP32_SendCommand("AT+CWMODE=1\r\n", "OK",
                                    TIREMO_ESP32_DEFAULT_CMD_TIMEOUT_MS);
}

bool TIREMO_ESP32_DisableAutoConnect(void)
{
    return TIREMO_ESP32_SendCommand("AT+CWAUTOCONN=0\r\n", "OK",
                                    TIREMO_ESP32_DEFAULT_CMD_TIMEOUT_MS);
}

bool TIREMO_ESP32_JoinAP(const char *pSsid,
                         const char *pPassword,
                         uint32_t timeoutMs)
{
    char command[TIREMO_ESP32_SSID_MAX_LEN + TIREMO_ESP32_PASSWORD_MAX_LEN +
                 24U];

    if ((pSsid == NULL) || (pPassword == NULL))
    {
        return false;
    }

    (void)snprintf(command, sizeof(command), "AT+CWJAP=\"%s\",\"%s\"\r\n",
                   pSsid, pPassword);

    /* "WIFI GOT IP", not "OK": the command can ACK while DHCP is still
     * running. Tutorial 01 taught substring match; this is why the
     * substring must be the event you actually care about. */
    return TIREMO_ESP32_SendCommand(command, "WIFI GOT IP", timeoutMs);
}

bool TIREMO_ESP32_Disconnect(void)
{
    return TIREMO_ESP32_SendCommand("AT+CWQAP\r\n", "OK",
                                    TIREMO_ESP32_DEFAULT_CMD_TIMEOUT_MS);
}

bool TIREMO_ESP32_IsConnected(void)
{
    /* "+CWSTATE:2" not "+CWSTATE:": the colon arrives before the digit,
     * and bytes are pushed one at a time. Matching the prefix returns
     * while the module is still in state 0/1/3/4. */
    return TIREMO_ESP32_SendCommand("AT+CWSTATE?\r\n",
                                    "+CWSTATE:2",
                                    TIREMO_ESP32_STATUS_TIMEOUT_MS);
}

bool TIREMO_ESP32_GetConnectedSsid(char *pOut, uint32_t outSize)
{
    char response[TIREMO_ESP32_RESPONSE_BUF_SIZE];
    const char *pMarker;
    const char *pStart;
    const char *pEnd;
    uint32_t copyLen;

    if ((pOut == NULL) || (outSize == 0U))
    {
        return false;
    }

    /* Match "OK", not "+CWJAP:". The quoted SSID is *after* that marker
     * on the same line, and RX is one byte at a time — returning at the
     * marker leaves pOut empty. */
    if (!TIREMO_ESP32_SendCommandCapture("AT+CWJAP?\r\n", "OK",
                                         TIREMO_ESP32_STATUS_TIMEOUT_MS,
                                         response, sizeof(response)))
    {
        return false;
    }

    pMarker = strstr(response, "+CWJAP:");
    if (pMarker == NULL)
    {
        return false;
    }

    pStart = strchr(pMarker, '"');
    if (pStart == NULL)
    {
        return false;
    }
    pStart++;

    pEnd = strchr(pStart, '"');
    if (pEnd == NULL)
    {
        return false;
    }

    copyLen = (uint32_t)(pEnd - pStart);
    if (copyLen >= outSize)
    {
        copyLen = outSize - 1U;
    }

    memcpy(pOut, pStart, copyLen);
    pOut[copyLen] = '\0';
    return (copyLen > 0U);
}

bool TIREMO_ESP32_EnableSntp(void)
{
    /* Timezone 0: asctime is UTC. Non-zero would skew the parsed epoch. */
    return TIREMO_ESP32_SendCommand(
        "AT+CIPSNTPCFG=1,0,\"pool.ntp.org\",\"time.google.com\"\r\n",
        "OK",
        TIREMO_ESP32_DEFAULT_CMD_TIMEOUT_MS);
}

bool TIREMO_ESP32_QuerySntpEpoch(uint32_t *pEpoch)
{
    char response[TIREMO_ESP32_RESPONSE_BUF_SIZE];

    if (pEpoch == NULL)
    {
        return false;
    }

    /* Match "OK", not "+CIPSNTPTIME:". The asctime sits after that
     * marker, one byte at a time -- returning at the colon leaves an
     * empty parse. */
    if (!TIREMO_ESP32_SendCommandCapture("AT+CIPSNTPTIME?\r\n", "OK",
                                         TIREMO_ESP32_STATUS_TIMEOUT_MS,
                                         response, sizeof(response)))
    {
        return false;
    }

    return prvParseSntpResponse(response, pEpoch);
}

bool TIREMO_ESP32_WaitSntpEpoch(uint32_t *pEpoch, uint32_t timeoutMs)
{
    uint32_t startMs;

    if (pEpoch == NULL)
    {
        return false;
    }

    startMs = TIREMO_SysTick_GetMs();
    while ((TIREMO_SysTick_GetMs() - startMs) < timeoutMs)
    {
        if (TIREMO_ESP32_QuerySntpEpoch(pEpoch))
        {
            return true;
        }
        TIREMO_SysTick_DelayMs(TIREMO_ESP32_SNTP_POLL_MS);
    }

    return false;
}

bool TIREMO_ESP32_MqttUserConfig(const char *pClientId,
                                 const char *pUsername,
                                 const char *pPassword)
{
    char command[160];

    if (pClientId == NULL)
    {
        return false;
    }
    if (pUsername == NULL)
    {
        pUsername = "";
    }
    if (pPassword == NULL)
    {
        pPassword = "";
    }

    /* LinkID 0, scheme 1 (plain TCP, no TLS), no cert/CA (slot 0, path "").
     * ESP-AT's scheme is 1-indexed: 1=TCP, 2..5=TLS variants, 6..10=WSS
     * variants -- there is no scheme 0. Mutual TLS (tutorial 03) is 5. */
    (void)snprintf(command, sizeof(command),
                   "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",
                   pClientId, pUsername, pPassword);

    return TIREMO_ESP32_SendCommand(command, "OK",
                                    TIREMO_ESP32_DEFAULT_CMD_TIMEOUT_MS);
}

bool TIREMO_ESP32_MqttUserConfigMtls(const char *pClientId)
{
    char command[160];

    if (pClientId == NULL)
    {
        return false;
    }

    /* scheme 5: verify server certificate AND present a client
     * certificate (mutual TLS). cert_key_ID=0 / CA_ID=0 -- the module
     * reads those from whatever TIREMO_ESP32_CredWrite() last stored in
     * the CLIENT_CERT / CLIENT_KEY / ROOT_CA slots. Auth is by
     * certificate, not username/password, so both are sent empty. */
    (void)snprintf(command, sizeof(command),
                   "AT+MQTTUSERCFG=0,5,\"%s\",\"\",\"\",0,0,\"\"\r\n",
                   pClientId);

    return TIREMO_ESP32_SendCommand(command, "OK",
                                    TIREMO_ESP32_DEFAULT_CMD_TIMEOUT_MS);
}

bool TIREMO_ESP32_MqttSni(const char *pHost)
{
    char command[160];

    if (pHost == NULL)
    {
        return false;
    }

    /* TLS SNI -- required for a custom domain endpoint that may sit
     * behind shared TLS-terminating infrastructure routed by hostname.
     * Without it, the TLS/MQTT handshake can still complete (against
     * whatever the SNI-less default routes to) while the actual AWS
     * IoT backend never sees the connection -- publishes and
     * subscribes ack locally but nothing ever answers on the expected
     * topics. Must be sent between AT+MQTTUSERCFG and AT+MQTTCONN. */
    (void)snprintf(command, sizeof(command), "AT+MQTTSNI=0,\"%s\"\r\n",
                   pHost);

    return TIREMO_ESP32_SendCommand(command, "OK",
                                    TIREMO_ESP32_DEFAULT_CMD_TIMEOUT_MS);
}

bool TIREMO_ESP32_MqttConnect(const char *pHost, uint16_t port,
                              uint32_t timeoutMs)
{
    char command[96];

    if (pHost == NULL)
    {
        return false;
    }

    (void)snprintf(command, sizeof(command), "AT+MQTTCONN=0,\"%s\",%u,0\r\n",
                   pHost, (unsigned int)port);

    return TIREMO_ESP32_SendCommand(command, "OK", timeoutMs);
}

bool TIREMO_ESP32_MqttSubscribe(const char *pTopic, uint8_t qos)
{
    char command[160];

    if (pTopic == NULL)
    {
        return false;
    }

    (void)snprintf(command, sizeof(command), "AT+MQTTSUB=0,\"%s\",%u\r\n",
                   pTopic, (unsigned int)qos);

    return TIREMO_ESP32_SendCommand(command, "OK",
                                    TIREMO_ESP32_DEFAULT_CMD_TIMEOUT_MS);
}

bool TIREMO_ESP32_MqttPublish(const char *pTopic, const char *pPayload,
                              uint8_t qos, bool retain)
{
    char command[TIREMO_ESP32_RESPONSE_BUF_SIZE];

    if ((pTopic == NULL) || (pPayload == NULL))
    {
        return false;
    }

    (void)snprintf(command, sizeof(command),
                   "AT+MQTTPUB=0,\"%s\",\"%s\",%u,%u\r\n",
                   pTopic, pPayload, (unsigned int)qos, retain ? 1U : 0U);

    return TIREMO_ESP32_SendCommand(command, "OK",
                                    TIREMO_ESP32_DEFAULT_CMD_TIMEOUT_MS);
}

/**
 * @brief One AT+MQTTPUBRAW attempt: prompt -> raw payload -> +MQTTPUB:OK.
 *
 * If the '>' prompt arrives but the send then fails, the declared byte
 * count is padded out with filler so the module leaves raw-input mode --
 * otherwise the caller's retry command text would be swallowed as payload.
 */
static bool prvESP32MqttPublishRawOnce(const char *pTopic,
                                       const uint8_t *pPayload,
                                       uint32_t length, uint8_t qos,
                                       bool retain)
{
    char command[160];
    uint32_t startMs;
    uint8_t byte;
    bool foundOk = false;
    bool foundFail = false;

    (void)snprintf(command, sizeof(command),
                   "AT+MQTTPUBRAW=0,\"%s\",%lu,%u,%u\r\n",
                   pTopic, (unsigned long)length, (unsigned int)qos,
                   retain ? 1U : 0U);

    /* The module answers with a bare '>' prompt once it's ready for the
     * raw bytes -- not "OK" yet, that comes after the payload. */
    if (!TIREMO_ESP32_SendCommand(command, ">",
                                  TIREMO_ESP32_MQTT_PROMPT_TIMEOUT_MS))
    {
        /* No prompt -> the module never entered raw-input mode, so a
         * plain retry of the command is safe. */
        TIREMO_LOG_WARN("ESP32: MQTTPUBRAW prompt '>' never arrived");
        return false;
    }

    if (TIREMO_HAL_UART_Transmit(TIREMO_ESP32_UART_PORT, pPayload, length,
                                 true) != TIREMO_HAL_UART_STATUS_OK)
    {
        uint32_t i;

        TIREMO_LOG_WARN("ESP32: MQTTPUBRAW payload TX failed -- padding frame");
        for (i = 0U; i < length; ++i)
        {
            uint8_t pad = (uint8_t)' ';

            (void)TIREMO_HAL_UART_Transmit(TIREMO_ESP32_UART_PORT, &pad, 1U,
                                           true);
        }
        return false;
    }

    /* AWS can answer CreateKeysAndCertificate before +MQTTPUB:OK.
     * Accumulate that burst into the MQTT hold buffer instead of the
     * 2048-byte command capture, which would drop the PEM JSON and then
     * report a truncated "accepted" payload. Do not flush the ring. */
    s_mqttRxLen = 0U;
    s_mqttRx[0] = '\0';
    startMs = TIREMO_SysTick_GetMs();
    while ((!foundOk) && (!foundFail) &&
           ((TIREMO_SysTick_GetMs() - startMs) < TIREMO_ESP32_MQTT_PUB_TIMEOUT_MS))
    {
        while ((!foundOk) && (!foundFail) && prvESP32RxRingPop(&byte))
        {
            if (s_mqttRxLen < (TIREMO_ESP32_MQTT_RX_SIZE - 1U))
            {
                s_mqttRx[s_mqttRxLen++] = (char)byte;
                s_mqttRx[s_mqttRxLen] = '\0';
            }
            if (strstr(s_mqttRx, "+MQTTPUB:OK") != NULL)
            {
                foundOk = true;
            }
            else if (strstr(s_mqttRx, "+MQTTPUB:FAIL") != NULL)
            {
                foundFail = true;
            }
        }
    }

    if (!foundOk)
    {
        /* All declared bytes were sent, so the module is back in command
         * mode -- a retry (which re-publishes) is safe and idempotent. */
        TIREMO_LOG_WARN("ESP32: MQTTPUBRAW missing +MQTTPUB:OK (%lu hold bytes)",
                        (unsigned long)s_mqttRxLen);
        return false;
    }
    return true;
}

bool TIREMO_ESP32_MqttPublishRaw(const char *pTopic, const uint8_t *pPayload,
                                 uint32_t length, uint8_t qos, bool retain)
{
    uint32_t attempt;

    if ((pTopic == NULL) || (pPayload == NULL) || (length == 0U))
    {
        return false;
    }

    for (attempt = 0U; attempt <= TIREMO_ESP32_MQTT_PUBRAW_RETRIES; ++attempt)
    {
        if (attempt > 0U)
        {
            /* Let the inbound URC that stole the last '>' finish, then
             * clear it so the retry's prompt is not mistaken for it. */
            TIREMO_SysTick_DelayMs(TIREMO_ESP32_MQTT_PUBRAW_RETRY_MS);
            prvESP32RxRingFlush();
            TIREMO_LOG_WARN("ESP32: MQTTPUBRAW retry %lu/%u topic=%s",
                            (unsigned long)attempt,
                            (unsigned int)TIREMO_ESP32_MQTT_PUBRAW_RETRIES,
                            pTopic);
        }

        if (prvESP32MqttPublishRawOnce(pTopic, pPayload, length, qos, retain))
        {
            return true;
        }
    }

    TIREMO_LOG_ERROR("ESP32: MQTTPUBRAW gave up after %u attempts topic=%s",
                     (unsigned int)(TIREMO_ESP32_MQTT_PUBRAW_RETRIES + 1U),
                     pTopic);
    return false;
}

bool TIREMO_ESP32_MqttWaitMessage(char *pTopicOut, uint32_t topicOutSize,
                                  uint8_t *pPayloadOut, uint32_t payloadOutSize,
                                  uint32_t *pPayloadLenOut, uint32_t timeoutMs)
{
    uint32_t startMs;
    uint32_t consumed = 0U;
    uint8_t byte;
    bool done = false;

    if ((pTopicOut == NULL) || (topicOutSize == 0U) ||
        (pPayloadOut == NULL) || (!s_initialized))
    {
        return false;
    }

    startMs = TIREMO_SysTick_GetMs();

    if (s_mqttRxLen > 0U)
    {
        done = prvESP32TryParseSubRecv(s_mqttRx, s_mqttRxLen, pTopicOut,
                                       topicOutSize, pPayloadOut,
                                       payloadOutSize, pPayloadLenOut,
                                       &consumed);
        if (done)
        {
            prvESP32MqttHoldShift(consumed);
            return true;
        }
    }

    while ((!done) && ((TIREMO_SysTick_GetMs() - startMs) < timeoutMs))
    {
        while ((!done) && prvESP32RxRingPop(&byte))
        {
            if (s_mqttRxLen < (TIREMO_ESP32_MQTT_RX_SIZE - 1U))
            {
                s_mqttRx[s_mqttRxLen++] = (char)byte;
                s_mqttRx[s_mqttRxLen] = '\0';
            }

            done = prvESP32TryParseSubRecv(s_mqttRx, s_mqttRxLen, pTopicOut,
                                           topicOutSize, pPayloadOut,
                                           payloadOutSize, pPayloadLenOut,
                                           &consumed);
        }
    }

    if (done)
    {
        prvESP32MqttHoldShift(consumed);
        return true;
    }

    /* Timeout is the normal idle return for a standing poll. Callers
     * that need a hard wait (boot get/update) log the miss themselves. */
    return false;
}

bool TIREMO_ESP32_MqttDisconnect(void)
{
    char response[128];

    s_mqttRxLen = 0U;
    s_mqttRx[0] = '\0';

    if (TIREMO_ESP32_SendCommandCapture("AT+MQTTCLEAN=0\r\n", "OK",
                                        TIREMO_ESP32_DEFAULT_CMD_TIMEOUT_MS,
                                        response, sizeof(response)))
    {
        return true;
    }
    /* Not connected: the module answers ERROR. That is success here. */
    return (strstr(response, "ERROR") != NULL);
}

bool TIREMO_ESP32_CredWrite(TIREMO_ESP32_CredSlot_t slot,
                            const uint8_t *pData, uint32_t length)
{
    const char *pNamespace;
    const char *pKey;
    char command[64];

    if (!prvESP32CredSlotInfo(slot, &pNamespace, &pKey))
    {
        return false;
    }
    if ((pData == NULL) || (length == 0U))
    {
        return false;
    }

    /* Type 8 = binary. PEM is text, but binary avoids any string
     * processing on the module's side -- see the ESP-AT SYSMFG docs'
     * own certificate examples, which all use type 8 too. */
    (void)snprintf(command, sizeof(command), "AT+SYSMFG=2,\"%s\",\"%s\",8,%lu\r\n",
                   pNamespace, pKey, (unsigned long)length);

    /* Module answers "OK" for the command line, then a bare ">" prompt
     * once it's ready for the raw bytes -- same two-stage shape as
     * MQTTPUBRAW. */
    if (!TIREMO_ESP32_SendCommand(command, ">", TIREMO_ESP32_CRED_TIMEOUT_MS))
    {
        TIREMO_LOG_WARN("ESP32: SYSMFG write prompt '>' never arrived");
        return false;
    }

    if (TIREMO_HAL_UART_Transmit(TIREMO_ESP32_UART_PORT, pData, length,
                                 true) != TIREMO_HAL_UART_STATUS_OK)
    {
        TIREMO_LOG_WARN("ESP32: SYSMFG write payload TX failed");
        return false;
    }

    return TIREMO_ESP32_SendCommand(NULL, "OK", TIREMO_ESP32_CRED_TIMEOUT_MS);
}

bool TIREMO_ESP32_CredRead(TIREMO_ESP32_CredSlot_t slot,
                           uint8_t *pOut, uint32_t outSize,
                           uint32_t *pLengthOut)
{
    const char *pNamespace;
    const char *pKey;
    char command[64];
    char buffer[TIREMO_ESP32_RESPONSE_BUF_SIZE];
    uint32_t length = 0U;
    uint32_t startMs;
    uint8_t byte;
    bool done = false;
    bool errored = false;

    if (!prvESP32CredSlotInfo(slot, &pNamespace, &pKey))
    {
        return false;
    }
    if ((pOut == NULL) || (!s_initialized))
    {
        return false;
    }

    (void)snprintf(command, sizeof(command), "AT+SYSMFG=1,\"%s\",\"%s\"\r\n",
                   pNamespace, pKey);

    prvESP32RxRingFlush();
    if (TIREMO_HAL_UART_Transmit(TIREMO_ESP32_UART_PORT,
                                 (const uint8_t *)command,
                                 (uint32_t)strlen(command),
                                 true) != TIREMO_HAL_UART_STATUS_OK)
    {
        TIREMO_LOG_WARN("ESP32: SYSMFG read command TX failed");
        return false;
    }

    buffer[0] = '\0';
    startMs = TIREMO_SysTick_GetMs();

    while ((!done) && (!errored) &&
           ((TIREMO_SysTick_GetMs() - startMs) < TIREMO_ESP32_CRED_TIMEOUT_MS))
    {
        while ((!done) && (!errored) && prvESP32RxRingPop(&byte))
        {
            if (length < (sizeof(buffer) - 1U))
            {
                buffer[length++] = (char)byte;
                buffer[length] = '\0';
            }

            if (strstr(buffer, "\r\nERROR\r\n") != NULL)
            {
                errored = true;
            }
            else
            {
                done = prvESP32TryParseSysmfgValue(buffer, length, pOut,
                                                   outSize, pLengthOut);
            }
        }
    }

    if (errored)
    {
        /* The real, expected way this fires: reading a slot that was
         * just erased (or never written). */
        TIREMO_LOG_WARN("ESP32: SYSMFG read \"%s\"/\"%s\": ERROR (empty slot?)",
                        pNamespace, pKey);
    }
    else if (!done)
    {
        TIREMO_LOG_WARN("ESP32: SYSMFG read \"%s\"/\"%s\": timed out",
                        pNamespace, pKey);
    }

    return done;
}

bool TIREMO_ESP32_CredErase(TIREMO_ESP32_CredSlot_t slot)
{
    const char *pNamespace;
    const char *pKey;
    char command[64];

    if (!prvESP32CredSlotInfo(slot, &pNamespace, &pKey))
    {
        return false;
    }

    (void)snprintf(command, sizeof(command), "AT+SYSMFG=0,\"%s\",\"%s\"\r\n",
                   pNamespace, pKey);

    return TIREMO_ESP32_SendCommand(command, "OK", TIREMO_ESP32_CRED_TIMEOUT_MS);
}

/******************************************************************************
 * Private Functions
 *****************************************************************************/

static bool prvESP32RxRingIsFull(void)
{
    uint32_t used = s_rxRing.head - s_rxRing.tail;
    return (used >= (TIREMO_ESP32_RX_RING_SIZE - 1U));
}

static bool prvESP32RxRingIsEmpty(void)
{
    return (s_rxRing.head == s_rxRing.tail);
}

static void prvESP32RxRingPush(uint8_t byte)
{
    uint32_t index;

    if (prvESP32RxRingIsFull())
    {
        return;
    }

    index = s_rxRing.head & TIREMO_ESP32_RX_MASK;
    s_rxRing.data[index] = byte;
    s_rxRing.head++;
}

static bool prvESP32RxRingPop(uint8_t *pByte)
{
    uint32_t index;

    if (prvESP32RxRingIsEmpty())
    {
        return false;
    }

    index = s_rxRing.tail & TIREMO_ESP32_RX_MASK;
    *pByte = s_rxRing.data[index];
    s_rxRing.tail++;
    return true;
}

static void prvESP32RxRingFlush(void)
{
    uint8_t byte;

    while (prvESP32RxRingPop(&byte))
    {
    }
}

/**
 * @brief Drop the parsed MQTT URC from the hold buffer, keep any bytes
 *        that arrived after it (typically +MQTTPUB:OK or the next URC).
 */
static void prvESP32MqttHoldShift(uint32_t consumed)
{
    if (consumed >= s_mqttRxLen)
    {
        s_mqttRxLen = 0U;
        s_mqttRx[0] = '\0';
        return;
    }
    s_mqttRxLen -= consumed;
    memmove(s_mqttRx, s_mqttRx + consumed, s_mqttRxLen);
    s_mqttRx[s_mqttRxLen] = '\0';
}

static void prvESP32UARTCallback(uint32_t event, void *pContext)
{
    (void)pContext;

    if (!s_initialized)
    {
        return;
    }

    if ((event & TIREMO_HAL_UART_EVENT_RX_DONE) != 0U)
    {
        prvESP32RxRingPush(s_rxByte);
        prvESP32ArmRx();
    }

    if ((event & TIREMO_HAL_UART_EVENT_LINE_ERROR) != 0U)
    {
        prvESP32ArmRx();
    }

    if (((event & TIREMO_HAL_UART_EVENT_TX_DONE) != 0U) && s_rxArmPending)
    {
        prvESP32ArmRx();
    }
}

static void prvESP32ArmRx(void)
{
    TIREMO_HAL_UART_Status_t status;

    if (!s_initialized)
    {
        return;
    }

    status = TIREMO_HAL_UART_Receive(TIREMO_ESP32_UART_PORT,
                                     &s_rxByte,
                                     1U,
                                     false);
    s_rxArmPending = (status != TIREMO_HAL_UART_STATUS_OK);
}

static void prvESP32LogResponse(const char *pLabel,
                                const char *pResponse,
                                uint32_t length)
{
#if (TIREMO_DEBUG_LEVEL < TIREMO_DEBUG_LEVEL_DEBUG)
    (void)pLabel;
    (void)pResponse;
    (void)length;
#else
    uint32_t i;

    if ((TIREMO_Debug_GetLevel() < TIREMO_DEBUG_LEVEL_DEBUG) ||
        (pResponse == NULL))
    {
        return;
    }

    if (pLabel != NULL)
    {
        TIREMO_Debug_Puts(pLabel);
    }
    TIREMO_Debug_Puts("{\"data\":\"");

    for (i = 0U; i < length; i++)
    {
        uint8_t ch = (uint8_t)pResponse[i];

        if (ch == (uint8_t)'"')
        {
            TIREMO_Debug_Puts("\\\"");
        }
        else if (ch == (uint8_t)'\\')
        {
            TIREMO_Debug_Puts("\\\\");
        }
        else if (ch == (uint8_t)'\n')
        {
            TIREMO_Debug_Puts("\\n");
        }
        else if (ch == (uint8_t)'\r')
        {
            TIREMO_Debug_Puts("\\r");
        }
        else if ((ch >= 0x20U) && (ch < 0x7FU))
        {
            TIREMO_Debug_PutChar(ch);
        }
        else
        {
            TIREMO_Debug_Puts("\\u00");
            TIREMO_Debug_PutHex(ch);
        }
    }

    TIREMO_Debug_PutsLine("\"}");
#endif
}

static void prvESP32PowerCycle(void)
{
    TIREMO_LOG_INFO("ESP32: power cycling module...");

    (void)TIREMO_HAL_GPIO_WritePin(TIREMO_ESP32_PWR_PORT,
                                   TIREMO_ESP32_PWR_PIN,
                                   TIREMO_ESP32_PWR_OFF);
    TIREMO_SysTick_DelayMs(TIREMO_ESP32_PWR_OFF_MS);

    (void)TIREMO_HAL_GPIO_WritePin(TIREMO_ESP32_PWR_PORT,
                                   TIREMO_ESP32_PWR_PIN,
                                   TIREMO_ESP32_PWR_ON);
    TIREMO_SysTick_DelayMs(TIREMO_ESP32_PWR_BOOT_MS);

    TIREMO_LOG_INFO("ESP32: power-on boot delay elapsed");
}

static bool prvESP32SendCommandCapture(const char *pCommand,
                                       const char *pExpected,
                                       uint32_t timeoutMs,
                                       char *pResponseOut,
                                       uint32_t responseOutSize)
{
    char response[TIREMO_ESP32_RESPONSE_BUF_SIZE];
    uint32_t length = 0U;
    uint32_t startMs;
    uint8_t byte;
    bool matched = false;
    bool errored = false;

    if ((!s_initialized) || (pExpected == NULL))
    {
        TIREMO_LOG_WARN("ESP32 SendCommand: not initialized or bad args");
        return false;
    }

    if (pCommand != NULL)
    {
        prvESP32RxRingFlush();

        TIREMO_LOG_DEBUG("ESP32 TX (%lu bytes): ",
                         (unsigned long)strlen(pCommand));
        prvESP32LogResponse("", pCommand, (uint32_t)strlen(pCommand));

        if (TIREMO_HAL_UART_Transmit(TIREMO_ESP32_UART_PORT,
                                     (const uint8_t *)pCommand,
                                     (uint32_t)strlen(pCommand),
                                     true) != TIREMO_HAL_UART_STATUS_OK)
        {
            TIREMO_LOG_WARN("ESP32 TX failed on UART2");
            return false;
        }
    }

    response[0] = '\0';
    startMs = TIREMO_SysTick_GetMs();

    while ((!matched) && (!errored) &&
           ((TIREMO_SysTick_GetMs() - startMs) < timeoutMs))
    {
        while ((!matched) && (!errored) && prvESP32RxRingPop(&byte))
        {
            if (length < (sizeof(response) - 1U))
            {
                response[length++] = (char)byte;
                response[length] = '\0';
            }

            if (strstr(response, pExpected) != NULL)
            {
                matched = true;
            }
            else if ((strcmp(pExpected, "\r\nERROR\r\n") != 0) &&
                     (strstr(response, "\r\nERROR\r\n") != NULL))
            {
                errored = true;
            }
        }
    }

    if (matched)
    {
        TIREMO_LOG_DEBUG("ESP32 RX matched \"%s\" after %lu bytes: ",
                         pExpected, (unsigned long)length);
        prvESP32LogResponse("", response, length);
    }
    else if (length == 0U)
    {
        TIREMO_LOG_WARN("ESP32 RX: no bytes received within %lu ms on UART%d "
                        "(check wiring/baud/power/EN pin)",
                        (unsigned long)timeoutMs, (int)TIREMO_ESP32_UART_PORT);
    }
    else if (errored)
    {
        TIREMO_LOG_WARN("ESP32 RX: \"%s\" rejected with ERROR after %lu bytes",
                        pExpected, (unsigned long)length);
        prvESP32LogResponse("", response, length);
    }
    else
    {
        TIREMO_LOG_WARN("ESP32 RX: timeout after %lu ms, %lu bytes without \"%s\"",
                        (unsigned long)timeoutMs, (unsigned long)length,
                        pExpected);
        prvESP32LogResponse("", response, length);
    }

    if ((pResponseOut != NULL) && (responseOutSize > 0U))
    {
        uint32_t copyLen = (length < (responseOutSize - 1U))
                               ? length
                               : (responseOutSize - 1U);
        memcpy(pResponseOut, response, copyLen);
        pResponseOut[copyLen] = '\0';
    }

    return matched;
}

/**
 * @brief Try to parse a complete "+MQTTSUBRECV:<id>,\"<topic>\",<len>,<data>"
 *        URC out of the accumulated capture buffer.
 *
 * Returns false (not an error, just "not yet") until the buffer holds the
 * marker, a complete quoted topic, a complete decimal length field, AND
 * at least that many payload bytes past it -- so this is safe to call
 * again as each new byte is appended.
 */
static bool prvESP32TryParseSubRecv(const char *pBuffer,
                                    uint32_t bufferLen,
                                    char *pTopicOut,
                                    uint32_t topicOutSize,
                                    uint8_t *pPayloadOut,
                                    uint32_t payloadOutSize,
                                    uint32_t *pPayloadLenOut,
                                    uint32_t *pConsumedOut)
{
    const char *pMarker;
    const char *p;
    const char *pTopicStart;
    const char *pTopicEnd;
    const char *pEnd;
    char *pDigitsEnd = NULL;
    long payloadLen;
    uint32_t haveBytes;
    uint32_t topicLen;
    uint32_t copyLen;
    uint32_t consumed;

    pMarker = strstr(pBuffer, "+MQTTSUBRECV:");
    if (pMarker == NULL)
    {
        return false;
    }

    p = strchr(pMarker, ',');   /* end of <LinkID> */
    if (p == NULL)
    {
        return false;
    }
    p++;

    if (*p != '"')
    {
        return false;
    }
    pTopicStart = p + 1;
    pTopicEnd = strchr(pTopicStart, '"');
    if (pTopicEnd == NULL)
    {
        return false;
    }

    p = pTopicEnd + 1;
    if (*p != ',')
    {
        return false;
    }
    p++;

    if ((*p < '0') || (*p > '9'))
    {
        return false;
    }
    /* Require the comma immediately after the digits -- strchr would
     * otherwise match a comma inside JSON if the length field was still
     * incomplete. */
    payloadLen = strtol(p, &pDigitsEnd, 10);
    if ((pDigitsEnd == NULL) || (pDigitsEnd == p) || (*pDigitsEnd != ',') ||
        (payloadLen < 0L) ||
        (payloadLen > (long)TIREMO_ESP32_MQTT_RX_SIZE))
    {
        return false;
    }
    p = pDigitsEnd + 1;

    haveBytes = (uint32_t)((pBuffer + bufferLen) - p);
    if (haveBytes < (uint32_t)payloadLen)
    {
        return false;   /* header is complete; payload is still arriving */
    }

    topicLen = (uint32_t)(pTopicEnd - pTopicStart);
    if (topicLen >= topicOutSize)
    {
        topicLen = topicOutSize - 1U;
    }
    memcpy(pTopicOut, pTopicStart, topicLen);
    pTopicOut[topicLen] = '\0';

    copyLen = (uint32_t)payloadLen;
    if (copyLen > payloadOutSize)
    {
        copyLen = payloadOutSize;
    }
    memcpy(pPayloadOut, p, copyLen);

    if (pPayloadLenOut != NULL)
    {
        *pPayloadLenOut = (uint32_t)payloadLen;
    }

    pEnd = p + (uint32_t)payloadLen;
    if (((pEnd + 1) <= (pBuffer + bufferLen)) &&
        (pEnd[0] == '\r') && (pEnd[1] == '\n'))
    {
        pEnd += 2;
    }
    consumed = (uint32_t)(pEnd - pBuffer);
    if (pConsumedOut != NULL)
    {
        *pConsumedOut = consumed;
    }

    return true;
}

/**
 * @brief Map a credential slot to its ESP-AT "AT+SYSMFG" namespace/key.
 *
 * The namespace strings are ESP-AT's own reserved names for the MQTT
 * client feature specifically (not the generic SSL client's
 * "client_cert"/"client_key"/"client_ca" set) -- "mqtt_ca" really is
 * what stores the server-verifying root CA. See the enum's own doc
 * comment in the header for why that name looks backwards.
 */
static bool prvESP32CredSlotInfo(TIREMO_ESP32_CredSlot_t slot,
                                 const char **ppNamespace,
                                 const char **ppKey)
{
    switch (slot)
    {
    case TIREMO_ESP32_CREDSLOT_ROOT_CA:
        *ppNamespace = "mqtt_ca";
        *ppKey = "mqtt_ca";
        return true;
    case TIREMO_ESP32_CREDSLOT_CLIENT_CERT:
        *ppNamespace = "mqtt_cert";
        *ppKey = "mqtt_cert";
        return true;
    case TIREMO_ESP32_CREDSLOT_CLIENT_KEY:
        *ppNamespace = "mqtt_key";
        *ppKey = "mqtt_key";
        return true;
    case TIREMO_ESP32_CREDSLOT_PROV_MARKER:
        *ppNamespace = "tiremo";
        *ppKey = "provisioned";
        return true;
    default:
        return false;
    }
}

/**
 * @brief Try to parse a complete
 *        "+SYSMFG:\"<ns>\",\"<key>\",<type>,<length>,<value>" response
 *        out of the accumulated capture buffer.
 *
 * Same shape as the MQTTSUBRECV parser above: returns false (not an
 * error, just "not yet") until the header is fully readable AND the
 * buffer already holds <length> bytes past it -- a genuine
 * length-prefixed parse, not a text heuristic, so it's correct for the
 * multi-line PEM value even though the value itself is never quoted.
 */
static bool prvESP32TryParseSysmfgValue(const char *pBuffer,
                                        uint32_t bufferLen,
                                        uint8_t *pOut,
                                        uint32_t outSize,
                                        uint32_t *pLengthOut)
{
    const char *pMarker;
    const char *p;
    long valueLen;
    uint32_t haveBytes;
    uint32_t copyLen;

    pMarker = strstr(pBuffer, "+SYSMFG:");
    if (pMarker == NULL)
    {
        return false;
    }
    p = pMarker + strlen("+SYSMFG:");

    if (*p != '"')   /* quoted namespace, echoed back -- skip it */
    {
        return false;
    }
    p = strchr(p + 1, '"');
    if (p == NULL)
    {
        return false;
    }
    p++;
    if (*p != ',')
    {
        return false;
    }
    p++;

    if (*p != '"')   /* quoted key, echoed back -- skip it */
    {
        return false;
    }
    p = strchr(p + 1, '"');
    if (p == NULL)
    {
        return false;
    }
    p++;
    if (*p != ',')
    {
        return false;
    }
    p++;

    if ((*p < '0') || (*p > '9'))   /* type digit(s) -- skip, don't need it */
    {
        return false;
    }
    p = strchr(p, ',');
    if (p == NULL)
    {
        return false;
    }
    p++;

    if ((*p < '0') || (*p > '9'))   /* decimal length field */
    {
        return false;
    }
    valueLen = strtol(p, NULL, 10);

    p = strchr(p, ',');   /* comma right after the length digits */
    if (p == NULL)
    {
        return false;
    }
    p++;

    haveBytes = (uint32_t)((pBuffer + bufferLen) - p);
    if (haveBytes < (uint32_t)valueLen)
    {
        return false;   /* header parsed, but the value is still arriving */
    }

    copyLen = (uint32_t)valueLen;
    if (copyLen > outSize)
    {
        copyLen = outSize;
    }
    memcpy(pOut, p, copyLen);

    if (pLengthOut != NULL)
    {
        *pLengthOut = (uint32_t)valueLen;
    }

    return true;
}

static int prvMonthFromName(const char *pName)
{
    static const char *kMonths[12] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    int i;

    if (pName == NULL)
    {
        return 0;
    }

    for (i = 0; i < 12; i++)
    {
        if (strncmp(pName, kMonths[i], 3) == 0)
        {
            return i + 1;
        }
    }

    return 0;
}

static bool prvIsLeapYear(uint32_t year)
{
    return (((year % 4U) == 0U) &&
            (((year % 100U) != 0U) || ((year % 400U) == 0U)));
}

static bool prvCivilToEpoch(uint32_t year,
                            uint32_t month,
                            uint32_t day,
                            uint32_t hour,
                            uint32_t minute,
                            uint32_t second,
                            uint32_t *pEpoch)
{
    static const uint8_t kDim[12] = {
        31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U
    };
    uint32_t y;
    uint32_t m;
    uint32_t days = 0U;

    if ((pEpoch == NULL) || (year < 1970U) || (month < 1U) || (month > 12U) ||
        (day < 1U) || (hour > 23U) || (minute > 59U) || (second > 60U))
    {
        return false;
    }

    for (y = 1970U; y < year; y++)
    {
        days += prvIsLeapYear(y) ? 366U : 365U;
    }

    for (m = 1U; m < month; m++)
    {
        days += kDim[m - 1U];
        if ((m == 2U) && prvIsLeapYear(year))
        {
            days += 1U;
        }
    }

    days += (day - 1U);
    *pEpoch = (days * 86400U) + (hour * 3600U) + (minute * 60U) + second;
    return true;
}

static bool prvParseUint(const char **pp, int *pOut)
{
    int value = 0;
    bool any = false;

    if ((pp == NULL) || (*pp == NULL) || (pOut == NULL))
    {
        return false;
    }

    while ((**pp == ' ') || (**pp == '\t'))
    {
        (*pp)++;
    }

    while ((**pp >= '0') && (**pp <= '9'))
    {
        value = (value * 10) + (int)(**pp - '0');
        (*pp)++;
        any = true;
    }

    *pOut = value;
    return any;
}

static bool prvParseToken(const char **pp, char *pOut, uint32_t outSize)
{
    uint32_t n = 0U;

    if ((pp == NULL) || (*pp == NULL) || (pOut == NULL) || (outSize == 0U))
    {
        return false;
    }

    while ((**pp == ' ') || (**pp == '\t'))
    {
        (*pp)++;
    }

    while ((**pp != '\0') && (**pp != ' ') && (**pp != '\t') &&
           (**pp != '\r') && (**pp != '\n') && (**pp != ':') &&
           ((n + 1U) < outSize))
    {
        pOut[n] = **pp;
        n++;
        (*pp)++;
    }

    pOut[n] = '\0';
    return (n > 0U);
}

static bool prvParseSntpResponse(const char *pResponse, uint32_t *pEpoch)
{
    const char *pLine;
    char weekday[8];
    char monthName[8];
    int day;
    int hour;
    int minute;
    int second;
    int year;
    int month;

    if ((pResponse == NULL) || (pEpoch == NULL))
    {
        return false;
    }

    pLine = strstr(pResponse, "+CIPSNTPTIME:");
    if (pLine == NULL)
    {
        return false;
    }
    pLine += (sizeof("+CIPSNTPTIME:") - 1U);

    /* asctime: "Tue Oct 19 17:47:56 2021" (extra space before day 1-9). */
    if (!prvParseToken(&pLine, weekday, sizeof(weekday)))
    {
        return false;
    }
    if (!prvParseToken(&pLine, monthName, sizeof(monthName)))
    {
        return false;
    }
    if (!prvParseUint(&pLine, &day))
    {
        return false;
    }
    if (!prvParseUint(&pLine, &hour) || (*pLine != ':'))
    {
        return false;
    }
    pLine++;
    if (!prvParseUint(&pLine, &minute) || (*pLine != ':'))
    {
        return false;
    }
    pLine++;
    if (!prvParseUint(&pLine, &second))
    {
        return false;
    }
    if (!prvParseUint(&pLine, &year))
    {
        return false;
    }

    (void)weekday;
    month = prvMonthFromName(monthName);
    if (month == 0)
    {
        return false;
    }

    if (!prvCivilToEpoch((uint32_t)year, (uint32_t)month, (uint32_t)day,
                         (uint32_t)hour, (uint32_t)minute, (uint32_t)second,
                         pEpoch))
    {
        return false;
    }

    if (*pEpoch < TIREMO_ESP32_SNTP_MIN_EPOCH)
    {
        return false;
    }

    return true;
}

static int32_t prvHexNibble(char c)
{
    if ((c >= '0') && (c <= '9'))
    {
        return (int32_t)(c - '0');
    }
    if ((c >= 'a') && (c <= 'f'))
    {
        return (int32_t)(10 + (c - 'a'));
    }
    if ((c >= 'A') && (c <= 'F'))
    {
        return (int32_t)(10 + (c - 'A'));
    }
    return -1;
}

static bool prvParseMacQuoted(const char *pResponse, uint8_t pEui[6])
{
    const char *p;
    uint32_t i;
    int32_t hi;
    int32_t lo;

    p = strstr(pResponse, "+CIPSTAMAC:");
    if (p == NULL)
    {
        return false;
    }
    p = strchr(p, '"');
    if (p == NULL)
    {
        return false;
    }
    p++;

    for (i = 0U; i < 6U; i++)
    {
        if (i > 0U)
        {
            if (*p != ':')
            {
                return false;
            }
            p++;
        }
        hi = prvHexNibble(*p++);
        lo = prvHexNibble(*p++);
        if ((hi < 0) || (lo < 0))
        {
            return false;
        }
        pEui[i] = (uint8_t)(((uint32_t)hi << 4U) | (uint32_t)lo);
    }
    return (*p == '"');
}

/******************************************************************************
 * HTTP download -- raw TCP/SSL passthrough (AT+CIPMODE=1)
 *
 * The AT HTTP client cannot take a multi-KB pre-signed URL (command-line
 * limit) and its TLS collides with the MQTT session. Instead: open a
 * socket, drop into transparent mode, push a raw HTTP GET (the long path
 * goes down the tunnel, not on a command line), and stream the response
 * body straight to the sink.
 *****************************************************************************/

#define TIREMO_ESP32_HTTP_HDR_MAX     (1400U)  /* HTTP response header block */
#define TIREMO_ESP32_HTTP_RUN         (256U)   /* sink call granularity */
#define TIREMO_ESP32_HTTP_STALL_MS    (20000U) /* no byte this long -> abort */
#define TIREMO_ESP32_HTTP_HOST_MAX    (128U)

/**
 * @brief Split "http(s)://host[:port]/path..." into host, port, and a
 *        pointer into @p pUrl at the path ('/' if none).
 */
static bool prvEspSplitUrl(const char *pUrl, char *pHost, uint32_t hostSz,
                           uint16_t *pPort, const char **ppPath, bool *pSsl)
{
    const char *p;
    const char *pHostStart;
    const char *pHostEnd;
    uint32_t n;

    if (strncmp(pUrl, "https://", 8) == 0)
    {
        *pSsl = true;
        *pPort = 443U;
        p = pUrl + 8;
    }
    else if (strncmp(pUrl, "http://", 7) == 0)
    {
        *pSsl = false;
        *pPort = 80U;
        p = pUrl + 7;
    }
    else
    {
        return false;
    }

    pHostStart = p;
    while ((*p != '\0') && (*p != '/') && (*p != ':'))
    {
        p++;
    }
    pHostEnd = p;

    if (*p == ':')
    {
        *pPort = (uint16_t)strtoul(p + 1, NULL, 10);
        while ((*p != '\0') && (*p != '/'))
        {
            p++;
        }
    }

    n = (uint32_t)(pHostEnd - pHostStart);
    if ((n == 0U) || (n >= hostSz))
    {
        return false;
    }
    memcpy(pHost, pHostStart, n);
    pHost[n] = '\0';
    *ppPath = (*p == '/') ? p : "/";
    return true;
}

static bool prvEspTunnelPuts(const char *pStr)
{
    return TIREMO_HAL_UART_Transmit(TIREMO_ESP32_UART_PORT,
                                    (const uint8_t *)pStr,
                                    (uint32_t)strlen(pStr), true) ==
           TIREMO_HAL_UART_STATUS_OK;
}

/** @brief Leave transparent mode: "+++" needs >=1 s silence either side. */
static void prvEspTunnelExit(void)
{
    TIREMO_SysTick_DelayMs(1100U);
    (void)TIREMO_HAL_UART_Transmit(TIREMO_ESP32_UART_PORT,
                                   (const uint8_t *)"+++", 3U, true);
    TIREMO_SysTick_DelayMs(1100U);
    (void)TIREMO_ESP32_SendCommand("AT+CIPMODE=0\r\n", "OK", 2000U);
}

bool TIREMO_ESP32_HttpGetToSink(const char *pUrl,
                                TIREMO_ESP32_HttpSink_t pSink, void *pUser,
                                uint32_t *pTotalOut, uint32_t timeoutMs)
{
    char host[TIREMO_ESP32_HTTP_HOST_MAX];
    char cmd[TIREMO_ESP32_HTTP_HOST_MAX + 48U];
    char hdr[TIREMO_ESP32_HTTP_HDR_MAX];
    const char *pPath;
    uint16_t port = 0U;
    bool ssl = false;
    uint32_t hdrLen = 0U;
    uint32_t bodyLen = 0U;    /* Content-Length, 0 = unknown */
    uint32_t bodyGot = 0U;
    bool headersDone = false;
    bool ok = false;
    uint32_t startMs;
    uint32_t lastByteMs;
    uint8_t byte;

    if ((pUrl == NULL) || (pSink == NULL) || (!s_initialized))
    {
        return false;
    }
    if (pTotalOut != NULL)
    {
        *pTotalOut = 0U;
    }
    if (!prvEspSplitUrl(pUrl, host, sizeof(host), &port, &pPath, &ssl))
    {
        TIREMO_LOG_WARN("http: unsupported URL scheme");
        return false;
    }
    TIREMO_LOG_INFO("http: %s %s:%u  path %lu bytes",
                    ssl ? "SSL" : "TCP", host, (unsigned)port,
                    (unsigned long)strlen(pPath));

    if (ssl)
    {
        /* One TLS context, and MQTT holds it -- free it or the SSL socket
         * open fails / wedges the module. MQTT is re-established on the
         * next boot (after a successful swap) or by the ready loop. */
        TIREMO_LOG_WARN("http: dropping MQTT to free the TLS context for SSL");
        (void)TIREMO_ESP32_SendCommand("AT+MQTTCLEAN=0\r\n", "OK", 5000U);
        (void)TIREMO_ESP32_SendCommand("AT+CIPSSLCCONF=0\r\n", "OK", 2000U);
    }

    (void)TIREMO_ESP32_SendCommand("AT+CIPMUX=0\r\n", "OK", 2000U);
    (void)TIREMO_ESP32_SendCommand("AT+CIPMODE=1\r\n", "OK", 2000U);

    (void)snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"%s\",\"%s\",%u\r\n",
                   ssl ? "SSL" : "TCP", host, (unsigned)port);
    prvESP32RxRingFlush();
    if (!TIREMO_ESP32_SendCommand(cmd, "CONNECT", 20000U))
    {
        TIREMO_LOG_WARN("http: CIPSTART failed");
        (void)TIREMO_ESP32_SendCommand("AT+CIPMODE=0\r\n", "OK", 2000U);
        return false;
    }

    prvESP32RxRingFlush();
    if (!TIREMO_ESP32_SendCommand("AT+CIPSEND\r\n", ">", 5000U))
    {
        TIREMO_LOG_WARN("http: CIPSEND prompt failed");
        prvEspTunnelExit();
        (void)TIREMO_ESP32_SendCommand("AT+CIPCLOSE\r\n", "OK", 3000U);
        return false;
    }

    /* --- in the tunnel: raw request --- */
    prvESP32RxRingFlush();
    (void)prvEspTunnelPuts("GET ");
    (void)prvEspTunnelPuts(pPath);
    (void)prvEspTunnelPuts(" HTTP/1.1\r\nHost: ");
    (void)prvEspTunnelPuts(host);
    (void)prvEspTunnelPuts("\r\nUser-Agent: tiremo-cortex\r\n"
                           "Accept: */*\r\nConnection: close\r\n\r\n");

    /* --- read the response --- */
    startMs = TIREMO_SysTick_GetMs();
    lastByteMs = startMs;
    hdr[0] = '\0';

    for (;;)
    {
        uint32_t nowMs = TIREMO_SysTick_GetMs();

        if ((nowMs - startMs) >= timeoutMs)
        {
            TIREMO_LOG_WARN("http: timeout (%lu body bytes)",
                            (unsigned long)bodyGot);
            break;
        }
        if (!prvESP32RxRingPop(&byte))
        {
            if ((nowMs - lastByteMs) >= TIREMO_ESP32_HTTP_STALL_MS)
            {
                TIREMO_LOG_WARN("http: stall (%lu body bytes)",
                                (unsigned long)bodyGot);
                break;
            }
            continue;
        }
        lastByteMs = nowMs;

        if (!headersDone)
        {
            if (hdrLen >= (sizeof(hdr) - 1U))
            {
                TIREMO_LOG_WARN("http: response headers too large");
                break;
            }
            hdr[hdrLen++] = (char)byte;
            hdr[hdrLen] = '\0';

            if ((hdrLen >= 4U) &&
                (hdr[hdrLen - 4U] == '\r') && (hdr[hdrLen - 3U] == '\n') &&
                (hdr[hdrLen - 2U] == '\r') && (hdr[hdrLen - 1U] == '\n'))
            {
                char *pCl;

                headersDone = true;
                if ((hdrLen < 12U) || (strncmp(hdr, "HTTP/1.", 7) != 0) ||
                    (hdr[9] != '2'))
                {
                    hdr[hdrLen - 2U] = '\0';
                    TIREMO_LOG_WARN("http: non-2xx response:");
                    TIREMO_Debug_PutsLine(hdr);
                    break;
                }
                pCl = strstr(hdr, "Content-Length:");
                if (pCl == NULL)
                {
                    pCl = strstr(hdr, "content-length:");
                }
                if (pCl != NULL)
                {
                    bodyLen = (uint32_t)strtoul(pCl + 15, NULL, 10);
                }
                TIREMO_LOG_INFO("http: 2xx, Content-Length=%lu",
                                (unsigned long)bodyLen);
            }
            continue;
        }

        {
            uint8_t run[TIREMO_ESP32_HTTP_RUN];
            uint32_t n = 0U;

            run[n++] = byte;
            bodyGot++;
            while ((n < sizeof(run)) &&
                   ((bodyLen == 0U) || (bodyGot < bodyLen)) &&
                   prvESP32RxRingPop(&byte))
            {
                run[n++] = byte;
                bodyGot++;
            }
            lastByteMs = TIREMO_SysTick_GetMs();
            if (!pSink(bodyGot - n, run, n, pUser))
            {
                TIREMO_LOG_WARN("http: sink rejected at %lu",
                                (unsigned long)(bodyGot - n));
                break;
            }
        }

        if ((bodyLen != 0U) && (bodyGot >= bodyLen))
        {
            ok = true;
            break;
        }
    }

    prvEspTunnelExit();
    (void)TIREMO_ESP32_SendCommand("AT+CIPCLOSE\r\n", "OK", 3000U);

    if (pTotalOut != NULL)
    {
        *pTotalOut = bodyGot;
    }
    if ((!ok) && headersDone && (bodyLen == 0U) && (bodyGot > 0U))
    {
        ok = true;   /* no Content-Length, but a body arrived and drained */
    }
    if (ok)
    {
        TIREMO_LOG_INFO("http: got %lu bytes", (unsigned long)bodyGot);
    }
    return ok && (bodyGot > 0U);
}
