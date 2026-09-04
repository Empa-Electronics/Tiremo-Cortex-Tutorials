/******************************************************************************
 * @file    tiremo_esp32.h
 * @brief   ESP32 AT-command transport: WiFi join, plain-TCP MQTT, mTLS
 *          credential storage, and a real Tiremo Cloud connection.
 *
 * Power sequencing, an interrupt-driven RX ring, and an expect/timeout
 * command primitive, plus station-mode join (CWMODE / CWJAP / CWSTATE),
 * MQTT (MQTTUSERCFG scheme 1 -- plain TCP, no TLS -- MQTTCONN / MQTTSUB /
 * MQTTPUB / MQTTPUBRAW, and a wait for the async +MQTTSUBRECV URC), the
 * three mTLS credential slots in the module's own manufacturing NVS
 * (AT+SYSMFG), and now a scheme-5 mutual-TLS MQTT identity plus
 * AT+MQTTSNI for a custom-domain endpoint. NTP / BluFi are ESP32
 * Connectivity.
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
#include <stddef.h>
#include <stdint.h>

/******************************************************************************
 * Public Types
 *****************************************************************************/

/**
 * @brief mTLS credential slots plus the fleet-provisioning marker.
 *
 * Each maps to one "AT+SYSMFG" namespace/key pair in the ESP32 module's
 * own manufacturing NVS. ESP-AT reserves a *separate* namespace set per
 * client feature -- "client_cert"/"client_key"/"client_ca" is the
 * generic SSL client's set, but the MQTT client (what AT+MQTTUSERCFG's
 * cert_key_ID/CA_ID actually read from) has its own:
 *
 *   TIREMO_ESP32_CREDSLOT_ROOT_CA     -> namespace "mqtt_ca"   (verifies the SERVER's cert)
 *   TIREMO_ESP32_CREDSLOT_CLIENT_CERT -> namespace "mqtt_cert" (this device's own cert)
 *   TIREMO_ESP32_CREDSLOT_CLIENT_KEY  -> namespace "mqtt_key"  (this device's own private key)
 *
 * Unlike the SSL client set (which stores multiple indexed sets as
 * "client_cert.0", "client_cert.1", ...), MQTT only supports one set,
 * and its key name equals its namespace name verbatim -- there is no
 * ".0" suffix here.
 *
 * "mqtt_ca" is ESP-AT's real, if slightly confusing, name for what
 * AWS IoT tutorials usually call the Root CA -- it's the CA the
 * *client* uses to verify the server, not a CA that belongs to the
 * client.
 */
typedef enum
{
    TIREMO_ESP32_CREDSLOT_ROOT_CA = 0,
    TIREMO_ESP32_CREDSLOT_CLIENT_CERT,
    TIREMO_ESP32_CREDSLOT_CLIENT_KEY,
    TIREMO_ESP32_CREDSLOT_PROV_MARKER,
    TIREMO_ESP32_CREDSLOT_MAX
} TIREMO_ESP32_CredSlot_t;

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
 * @brief Read and parse the six-byte station MAC ("AT+CIPSTAMAC?").
 *
 * @param[out] pEui  Six bytes, most-significant byte first.
 *
 * @return true when a complete quoted MAC was parsed.
 */
bool TIREMO_ESP32_GetHwEui(uint8_t pEui[6]);

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
 * @brief Enable SNTP with timezone 0 (UTC) ("AT+CIPSNTPCFG").
 *
 * Required before a scheme 3/5/8/10 AT+MQTTCONN: verifying the server
 * certificate means checking its validity dates, and the module has no
 * RTC at boot -- ESP-AT's own docs say the connect can fail until the
 * module has obtained the current time via SNTP.
 *
 * @return true when the module acknowledged with "OK".
 */
bool TIREMO_ESP32_EnableSntp(void);

/**
 * @brief Query "AT+CIPSNTPTIME?" once and parse asctime into a Unix epoch.
 *
 * Matches the terminating "OK", then parses "+CIPSNTPTIME:". Matching
 * the marker itself would return before the asctime bytes arrived.
 * Unsynced modules report 1 Jan 1970 -- that is rejected.
 *
 * @param[out] pEpoch  Unix epoch (UTC seconds) on success.
 *
 * @return true when a post-1970 time was parsed.
 */
bool TIREMO_ESP32_QuerySntpEpoch(uint32_t *pEpoch);

/**
 * @brief Poll QuerySntpEpoch until a valid time arrives or timeoutMs elapses.
 *
 * @param[out] pEpoch     Unix epoch (UTC seconds) on success.
 * @param[in]  timeoutMs  Maximum time to wait for a non-1970 sample.
 *
 * @return true when a valid epoch was obtained.
 */
bool TIREMO_ESP32_WaitSntpEpoch(uint32_t *pEpoch, uint32_t timeoutMs);

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
 * @brief Configure the MQTT client identity for a mutual-TLS session
 *        ("AT+MQTTUSERCFG", scheme 5 -- verify the server AND present a
 *        client certificate).
 *
 * Auth is by certificate, not username/password, so both fields are
 * sent empty. cert_key_ID=0 / CA_ID=0 -- the module reads those from
 * whatever TIREMO_ESP32_CredWrite() last stored in the CLIENT_CERT /
 * CLIENT_KEY / ROOT_CA slots (see TIREMO_ESP32_CredSlot_t).
 *
 * @param[in] pClientId  MQTT client ID (your AWS IoT Thing name, or
 *                        whatever client ID your Thing's policy allows).
 *
 * @return true when the module acknowledged with "OK".
 */
bool TIREMO_ESP32_MqttUserConfigMtls(const char *pClientId);

/**
 * @brief Set the TLS SNI hostname for the MQTT connection ("AT+MQTTSNI").
 *
 * Required for a custom domain endpoint that may sit behind SNI-routed
 * TLS-terminating infrastructure -- without it, the TLS/MQTT handshake
 * can still complete while the connection never actually reaches the
 * intended backend. Must be called after
 * TIREMO_ESP32_MqttUserConfigMtls() and before
 * TIREMO_ESP32_MqttConnect().
 *
 * @param[in] pHost  SNI hostname; normally the same as the broker host
 *                    passed to TIREMO_ESP32_MqttConnect().
 *
 * @return true when the module acknowledged with "OK".
 */
bool TIREMO_ESP32_MqttSni(const char *pHost);

/**
 * @brief Connect to an MQTT broker ("AT+MQTTCONN").
 *
 * @param[in] pHost      Broker hostname.
 * @param[in] port       Broker port (1883 for plain TCP, 8883 for TLS).
 * @param[in] timeoutMs  Maximum time to wait for "OK".
 *
 * @return true when the module acknowledged with "OK".
 */
bool TIREMO_ESP32_MqttConnect(const char *pHost, uint16_t port,
                              uint32_t timeoutMs);

/**
 * @brief Write a credential into one of the three mTLS slots
 *        ("AT+SYSMFG=2,...").
 *
 * Overwrites whatever was in that slot before. The module answers "OK"
 * for the command line, then a bare "\r\n" and a ">" prompt once it's
 * ready for the raw bytes; a final "OK" follows once all @p length
 * bytes have been received.
 *
 * @param[in] slot     Target credential slot.
 * @param[in] pData    Raw PEM bytes to store (no NUL needed/expected).
 * @param[in] length   Number of bytes to write.
 *
 * @return true when the module accepted the data and acknowledged with
 *         the final "OK".
 */
bool TIREMO_ESP32_CredWrite(TIREMO_ESP32_CredSlot_t slot,
                            const uint8_t *pData, uint32_t length);

/**
 * @brief Read a credential slot back ("AT+SYSMFG=1,...").
 *
 * @param[out] pOut        Destination buffer for the raw bytes.
 * @param[in]  outSize     Size of pOut.
 * @param[out] pLengthOut  Length the module reported; may exceed
 *                         outSize (copy is then truncated). May be NULL.
 * @param[in]  slot        Target credential slot.
 *
 * @return true when the slot held a value and it was parsed
 *         successfully; false on a timeout, a framed ERROR (e.g. the
 *         slot is empty/was erased), or a malformed response.
 */
bool TIREMO_ESP32_CredRead(TIREMO_ESP32_CredSlot_t slot,
                           uint8_t *pOut, uint32_t outSize,
                           uint32_t *pLengthOut);

/**
 * @brief Erase a credential slot ("AT+SYSMFG=0,...").
 *
 * @param[in] slot  Target credential slot.
 *
 * @return true when the module acknowledged with "OK".
 */
bool TIREMO_ESP32_CredErase(TIREMO_ESP32_CredSlot_t slot);

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

/**
 * @brief Sink for a streamed HTTP body.
 *
 * Called in order for each run of downloaded bytes. Return false to abort
 * the transfer (e.g. the destination is full or a write failed).
 *
 * @param[in] offset  Byte offset of this run from the start of the body.
 * @param[in] pData    Body bytes.
 * @param[in] length   Number of bytes.
 * @param[in] pUser    Opaque pointer passed to TIREMO_ESP32_HttpGetToSink().
 */
typedef bool (*TIREMO_ESP32_HttpSink_t)(uint32_t offset, const uint8_t *pData,
                                        uint32_t length, void *pUser);

/**
 * @brief HTTP(S) GET a URL, streaming the response body to a sink.
 *
 * The AT HTTP client (AT+HTTPCGET / AT+HTTPCLIENT) cannot take a
 * multi-KB pre-signed URL -- it does not fit the AT command line -- and
 * opening its own HTTPS session collides with an already-connected MQTT
 * session (one TLS context). Instead this drops into raw socket
 * passthrough: AT+CIPMODE=1, AT+CIPSTART, AT+CIPSEND, then a hand-built
 * "GET <path> HTTP/1.1" request is pushed straight down the UART tunnel
 * (the long path/query string never touches a command line) and the
 * response is read byte-by-byte off the same tunnel -- past the
 * "\r\n\r\n" header terminator, then Content-Length body bytes straight
 * to @p pSink, then "+++" to leave passthrough.
 *
 * For https://, the MQTT session is disconnected first to free the
 * single TLS context the ESP32 AT firmware supports; the ready loop /
 * next boot reconnects it.
 *
 * @param[in]  pUrl        Full URL, http:// or https://.
 * @param[in]  pSink       Called for each body run; return false to abort.
 * @param[in]  pUser       Passed through to @p pSink.
 * @param[out] pTotalOut   Total bytes delivered (may be NULL).
 * @param[in]  timeoutMs   Overall wall-clock budget for the whole transfer.
 *
 * @return true only when at least one body byte arrived and the transfer
 *         ended cleanly (Content-Length reached, or the server closed
 *         the connection after a body with no Content-Length).
 */
bool TIREMO_ESP32_HttpGetToSink(const char *pUrl,
                                TIREMO_ESP32_HttpSink_t pSink, void *pUser,
                                uint32_t *pTotalOut, uint32_t timeoutMs);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_ESP32_H */
