/******************************************************************************
 * @file    tiremo_esp32.h
 * @brief   ESP32 AT-command transport, extended with mTLS credential
 *          storage in the module's own manufacturing NVS
 *          (Cloud Messaging tutorial 02).
 *
 * Power sequencing, an interrupt-driven RX ring, and an expect/timeout
 * command primitive, plus write/read/erase for the three credential
 * slots ("AT+SYSMFG") a real mTLS connection needs. WiFi / NTP / BluFi
 * are ESP32 Connectivity; MQTT is Cloud Messaging tutorial 01; a real
 * AWS IoT Core mTLS connection using these slots is tutorial 03.
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
#include <stddef.h>

/******************************************************************************
 * Public Types
 *****************************************************************************/

/**
 * @brief The three mTLS credential slots a client connection needs.
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
 * @brief Write a credential slot ("AT+SYSMFG=2,...", type 8/binary).
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
 * Parses the module's own reported length out of the response
 * ("+SYSMFG:\"ns\",\"key\",<type>,<length>,<value>") and copies exactly
 * that many raw bytes -- correct for PEM data, which is text but still
 * read byte-exactly here, not line-parsed.
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

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_ESP32_H */
