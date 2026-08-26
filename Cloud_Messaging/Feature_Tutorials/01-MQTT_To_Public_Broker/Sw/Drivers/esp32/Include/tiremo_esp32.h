/******************************************************************************
 * @file    tiremo_esp32.h
 * @brief   ESP32 AT-command transport, extended with a plain-TCP MQTT
 *          client (Cloud Messaging tutorial 01).
 *
 * Power sequencing, an interrupt-driven RX ring, and an expect/timeout
 * command primitive, plus station-mode join (CWMODE / CWJAP / CWSTATE)
 * and now MQTT (MQTTUSERCFG scheme 1 -- plain TCP, no TLS -- MQTTCONN /
 * MQTTSUB / MQTTPUB / MQTTPUBRAW, and a wait for the async +MQTTSUBRECV
 * URC). mTLS credentials and a real AWS IoT Core connection are later
 * tutorials in this same section; NTP / BluFi are ESP32 Connectivity.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_ESP32_H
#define TIREMO_ESP32_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/******************************************************************************
 * Public Function Prototypes
 *****************************************************************************/

/**
 * @brief Initialize the ESP32 AT-command transport.
 *
 * Configures the supply-gate GPIO, power-cycles the module (off, then on
 * with a boot delay), registers the UART2 interrupt handler, and starts
 * the RX ring buffer.
 *
 * @return true when the transport was initialized successfully.
 *
 * @warning Call TIREMO_SysTick_Init() first; the boot delay depends on it.
 * @warning Call after PRV_UART_Init() so UART2 is already clocked.
 */
bool TIREMO_ESP32_Init(void);

/**
 * @brief Deinitialize the ESP32 AT-command transport.
 */
void TIREMO_ESP32_Deinit(void);

/**
 * @brief Check whether the ESP32 transport is initialized.
 *
 * @return true when TIREMO_ESP32_Init() succeeded.
 */
bool TIREMO_ESP32_IsInitialized(void);

/**
 * @brief Send an AT command and wait for an expected response substring.
 *
 * @param[in] pCommand   Command string to transmit as-is (include "\r\n").
 * @param[in] pExpected  Substring expected somewhere in the response.
 * @param[in] timeoutMs  Maximum time to wait for the expected substring.
 *
 * @return true when pExpected was seen in the response before timeoutMs
 *         elapsed.
 */
bool TIREMO_ESP32_SendCommand(const char *pCommand,
                              const char *pExpected,
                              uint32_t timeoutMs);

/**
 * @brief Send an AT command and also capture the raw response text.
 *
 * Same matching semantics as TIREMO_ESP32_SendCommand(), but also copies
 * whatever response bytes were accumulated into pResponseOut.
 *
 * @param[in]  pCommand        Command string to transmit as-is.
 * @param[in]  pExpected       Substring expected somewhere in the response.
 * @param[in]  timeoutMs       Maximum time to wait for the expected substring.
 * @param[out] pResponseOut    Destination buffer (NUL-terminated); may be NULL.
 * @param[in]  responseOutSize Size of pResponseOut, including the NUL.
 *
 * @return true when pExpected was seen in the response before timeoutMs
 *         elapsed.
 */
bool TIREMO_ESP32_SendCommandCapture(const char *pCommand,
                                     const char *pExpected,
                                     uint32_t timeoutMs,
                                     char *pResponseOut,
                                     uint32_t responseOutSize);

/**
 * @brief Probe whether the ESP32 module responds to a plain "AT" command.
 *
 * @return true when the module replied with "OK" within the default
 *         detection timeout.
 */
bool TIREMO_ESP32_IsPresent(void);

/**
 * @brief Disable AT command echo on the module ("ATE0").
 *
 * The module echoes every command by default, which mixes the echoed
 * command text into every response. Call this once after detection so
 * later command parsing only has to look at the actual response.
 *
 * @return true when the module acknowledged with "OK".
 */
bool TIREMO_ESP32_DisableEcho(void);

/**
 * @brief Put the module into WiFi station mode ("AT+CWMODE=1").
 *
 * @return true when the module acknowledged with "OK".
 */
bool TIREMO_ESP32_SetStationMode(void);

/**
 * @brief Disable the module's own connect-on-boot behavior
 *        ("AT+CWAUTOCONN=0").
 *
 * Without this, the ESP32 re-joins whatever AP is still in its NVS on
 * every power-up — independent of the SSID this tutorial hardcodes.
 *
 * @return true when the module acknowledged with "OK".
 */
bool TIREMO_ESP32_DisableAutoConnect(void);

/**
 * @brief Join a WiFi access point ("AT+CWJAP").
 *
 * Waits for the unsolicited "WIFI GOT IP" URC, not the command's "OK".
 * "OK" can arrive while association is still in progress.
 *
 * @param[in] pSsid      Access point SSID.
 * @param[in] pPassword  Access point password.
 * @param[in] timeoutMs  Maximum time to wait for "WIFI GOT IP".
 *
 * @return true when the module reported "WIFI GOT IP" before timeoutMs.
 */
bool TIREMO_ESP32_JoinAP(const char *pSsid,
                         const char *pPassword,
                         uint32_t timeoutMs);

/**
 * @brief Disconnect from the current access point ("AT+CWQAP").
 *
 * @return true when the module acknowledged with "OK".
 */
bool TIREMO_ESP32_Disconnect(void);

/**
 * @brief Check whether the module currently has an IPv4 WiFi connection
 *        ("AT+CWSTATE?").
 *
 * Matches "+CWSTATE:2" (connected with IPv4), not "+CWSTATE:" — that
 * prefix arrives one byte before the state digit, so matching it returns
 * true while the module is still associating.
 *
 * @return true when the reported state is 2.
 */
bool TIREMO_ESP32_IsConnected(void);

/**
 * @brief Get the SSID of the currently joined access point ("AT+CWJAP?").
 *
 * Matches on the terminating "OK", then parses the quoted SSID out of
 * the captured buffer. Matching "+CWJAP:" instead would return before
 * the quoted name had arrived (bytes come in one at a time).
 *
 * @param[out] pOut     Destination buffer for the SSID (NUL-terminated).
 * @param[in]  outSize  Size of pOut, including the NUL.
 *
 * @return true when a connected SSID was parsed out of the response.
 */
bool TIREMO_ESP32_GetConnectedSsid(char *pOut, uint32_t outSize);

/**
 * @brief Configure the MQTT client identity for a plain-TCP session
 *        ("AT+MQTTUSERCFG", scheme 1 -- no TLS, no certificate).
 *
 * @param[in] pClientId  MQTT client ID.
 * @param[in] pUsername  MQTT username, or "" if the broker needs none.
 * @param[in] pPassword  MQTT password, or "" if the broker needs none.
 *
 * @return true when the module acknowledged with "OK".
 */
bool TIREMO_ESP32_MqttUserConfig(const char *pClientId,
                                 const char *pUsername,
                                 const char *pPassword);

/**
 * @brief Connect to an MQTT broker ("AT+MQTTCONN").
 *
 * @param[in] pHost      Broker hostname.
 * @param[in] port       Broker port (1883 for plain TCP).
 * @param[in] timeoutMs  Maximum time to wait for "OK".
 *
 * @return true when the module acknowledged with "OK".
 */
bool TIREMO_ESP32_MqttConnect(const char *pHost, uint16_t port,
                              uint32_t timeoutMs);

/**
 * @brief Subscribe to a topic ("AT+MQTTSUB").
 *
 * @param[in] pTopic  Topic filter to subscribe to.
 * @param[in] qos     QoS level (0, 1, or 2).
 *
 * @return true when the module acknowledged with "OK".
 */
bool TIREMO_ESP32_MqttSubscribe(const char *pTopic, uint8_t qos);

/**
 * @brief Publish a NUL-terminated string payload ("AT+MQTTPUB").
 *
 * The payload is sent inside AT-command quotes, so it must not contain
 * `"`, `\r`, or `\n`. For anything that might, use
 * TIREMO_ESP32_MqttPublishRaw() instead.
 *
 * @param[in] pTopic    Topic to publish to.
 * @param[in] pPayload  NUL-terminated payload string.
 * @param[in] qos       QoS level (0, 1, or 2).
 * @param[in] retain    true to set the MQTT retain flag.
 *
 * @return true when the module acknowledged with "OK".
 */
bool TIREMO_ESP32_MqttPublish(const char *pTopic, const char *pPayload,
                              uint8_t qos, bool retain);

/**
 * @brief Publish a binary-safe payload ("AT+MQTTPUBRAW").
 *
 * Unlike TIREMO_ESP32_MqttPublish(), the payload is sent as raw,
 * length-prefixed bytes -- safe for embedded NULs, quotes, or any other
 * byte value a string-quoted command could not carry.
 *
 * @param[in] pTopic    Topic to publish to.
 * @param[in] pPayload  Raw payload bytes.
 * @param[in] length    Number of payload bytes.
 * @param[in] qos       QoS level (0, 1, or 2).
 * @param[in] retain    true to set the MQTT retain flag.
 *
 * @return true when the module accepted the payload and acknowledged
 *         with "OK".
 */
bool TIREMO_ESP32_MqttPublishRaw(const char *pTopic, const uint8_t *pPayload,
                                 uint32_t length, uint8_t qos, bool retain);

/**
 * @brief Block until the next inbound message arrives ("+MQTTSUBRECV"
 *        URC) or timeoutMs elapses.
 *
 * Parses the URC's own length field and reads exactly that many payload
 * bytes back out of the RX ring -- correct for a binary payload, as
 * long as the whole URC (header + payload) fits inside this driver's
 * internal capture buffer. See the tutorial README for that limit.
 *
 * @param[out] pTopicOut     Destination buffer for the topic (NUL-terminated).
 * @param[in]  topicOutSize  Size of pTopicOut, including the NUL.
 * @param[out] pPayloadOut   Destination buffer for the raw payload bytes.
 * @param[in]  payloadOutSize Size of pPayloadOut.
 * @param[out] pPayloadLenOut Actual payload length reported by the URC;
 *                             may exceed payloadOutSize (truncated copy).
 * @param[in]  timeoutMs     Maximum time to wait for a message.
 *
 * @return true when a message was received and parsed before timeoutMs.
 */
bool TIREMO_ESP32_MqttWaitMessage(char *pTopicOut, uint32_t topicOutSize,
                                  uint8_t *pPayloadOut, uint32_t payloadOutSize,
                                  uint32_t *pPayloadLenOut, uint32_t timeoutMs);

/**
 * @brief Disconnect the MQTT session ("AT+MQTTCLEAN").
 *
 * @return true when the module acknowledged with "OK".
 */
bool TIREMO_ESP32_MqttDisconnect(void);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_ESP32_H */
