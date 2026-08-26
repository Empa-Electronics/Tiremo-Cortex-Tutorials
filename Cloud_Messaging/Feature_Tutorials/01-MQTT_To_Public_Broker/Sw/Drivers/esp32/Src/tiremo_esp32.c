/******************************************************************************
 * @file    tiremo_esp32.c
 * @brief   ESP32 AT-command transport, extended with a plain-TCP MQTT
 *          client (Cloud Messaging tutorial 01).
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

/** @brief RX ring size; must stay a power of two. A +MQTTSUBRECV URC
 *  (topic + length header + payload) needs more room than a bare AT/OK. */
#define TIREMO_ESP32_RX_RING_SIZE        (512U)
#define TIREMO_ESP32_RX_MASK             (TIREMO_ESP32_RX_RING_SIZE - 1U)

#define TIREMO_ESP32_RESPONSE_BUF_SIZE   (512U)
#define TIREMO_ESP32_DEFAULT_CMD_TIMEOUT_MS (1000U)
#define TIREMO_ESP32_STATUS_TIMEOUT_MS      (2000U)
#define TIREMO_ESP32_SSID_MAX_LEN           (33U)
#define TIREMO_ESP32_PASSWORD_MAX_LEN       (65U)

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
                                    uint32_t *pPayloadLenOut);

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
    char command[96];

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

bool TIREMO_ESP32_MqttPublishRaw(const char *pTopic, const uint8_t *pPayload,
                                 uint32_t length, uint8_t qos, bool retain)
{
    char command[96];

    if ((pTopic == NULL) || (pPayload == NULL) || (length == 0U))
    {
        return false;
    }

    (void)snprintf(command, sizeof(command),
                   "AT+MQTTPUBRAW=0,\"%s\",%lu,%u,%u\r\n",
                   pTopic, (unsigned long)length, (unsigned int)qos,
                   retain ? 1U : 0U);

    /* The module answers with a bare '>' prompt once it's ready for the
     * raw bytes -- not "OK" yet, that comes after the payload. */
    if (!TIREMO_ESP32_SendCommand(command, ">",
                                  TIREMO_ESP32_DEFAULT_CMD_TIMEOUT_MS))
    {
        TIREMO_LOG_WARN("ESP32: MQTTPUBRAW prompt '>' never arrived");
        return false;
    }

    if (TIREMO_HAL_UART_Transmit(TIREMO_ESP32_UART_PORT, pPayload, length,
                                 true) != TIREMO_HAL_UART_STATUS_OK)
    {
        TIREMO_LOG_WARN("ESP32: MQTTPUBRAW payload TX failed");
        return false;
    }

    /* pCommand = NULL: just wait, do not flush/transmit again -- the
     * command line and prompt were already handled above. Completion is
     * "+MQTTPUB:OK" or "+MQTTPUB:FAIL", not a bare "OK". */
    return TIREMO_ESP32_SendCommand(NULL, "+MQTTPUB:OK",
                                    TIREMO_ESP32_DEFAULT_CMD_TIMEOUT_MS);
}

bool TIREMO_ESP32_MqttWaitMessage(char *pTopicOut, uint32_t topicOutSize,
                                  uint8_t *pPayloadOut, uint32_t payloadOutSize,
                                  uint32_t *pPayloadLenOut, uint32_t timeoutMs)
{
    char buffer[TIREMO_ESP32_RESPONSE_BUF_SIZE];
    uint32_t length = 0U;
    uint32_t startMs;
    uint8_t byte;
    bool done = false;

    if ((pTopicOut == NULL) || (topicOutSize == 0U) ||
        (pPayloadOut == NULL) || (!s_initialized))
    {
        return false;
    }

    buffer[0] = '\0';
    startMs = TIREMO_SysTick_GetMs();

    while ((!done) && ((TIREMO_SysTick_GetMs() - startMs) < timeoutMs))
    {
        while ((!done) && prvESP32RxRingPop(&byte))
        {
            if (length < (sizeof(buffer) - 1U))
            {
                buffer[length++] = (char)byte;
                buffer[length] = '\0';
            }

            done = prvESP32TryParseSubRecv(buffer, length, pTopicOut,
                                           topicOutSize, pPayloadOut,
                                           payloadOutSize, pPayloadLenOut);
        }
    }

    if (!done)
    {
        TIREMO_LOG_WARN("ESP32: MQTTSUBRECV wait timed out after %lu ms",
                        (unsigned long)timeoutMs);
    }

    return done;
}

bool TIREMO_ESP32_MqttDisconnect(void)
{
    return TIREMO_ESP32_SendCommand("AT+MQTTCLEAN=0\r\n", "OK",
                                    TIREMO_ESP32_DEFAULT_CMD_TIMEOUT_MS);
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
                                    uint32_t *pPayloadLenOut)
{
    const char *pMarker;
    const char *p;
    const char *pTopicStart;
    const char *pTopicEnd;
    long payloadLen;
    uint32_t haveBytes;
    uint32_t topicLen;
    uint32_t copyLen;

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
    payloadLen = strtol(p, NULL, 10);

    p = strchr(p, ',');   /* comma right after the length digits */
    if (p == NULL)
    {
        return false;
    }
    p++;

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

    return true;
}
