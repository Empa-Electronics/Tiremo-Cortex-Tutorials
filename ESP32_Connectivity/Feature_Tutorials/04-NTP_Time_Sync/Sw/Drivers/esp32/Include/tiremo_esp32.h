/******************************************************************************
 * @file    tiremo_esp32.h
 * @brief   ESP32 AT-command transport (tutorial 01 subset).
 *
 * Power sequencing, an interrupt-driven RX ring, and an expect/timeout
 * command primitive, station join, scan/retry, and SNTP
 * (CIPSNTPCFG / CIPSNTPTIME). BluFi / MQTT live later.
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
 * Public Types
 *****************************************************************************/

/**
 * @brief Outcome of AT+CWJAP, distinguished so the retry loop can treat
 *        a definite ERROR (wrong password) differently from a timeout
 *        (AP busy / DHCP slow).
 */
typedef enum
{
    TIREMO_ESP32_JOIN_GOT_IP = 0,
    TIREMO_ESP32_JOIN_ERROR,
    TIREMO_ESP32_JOIN_TIMEOUT
} TIREMO_ESP32_JoinStatus_t;

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
 * @brief Scan for visible access points and check whether one SSID is
 *        among them ("AT+CWLAP").
 *
 * Matches the quoted SSID in the scan dump. An out-of-range AP fails
 * here in a few seconds instead of waiting out CWJAP's join timeout.
 *
 * @param[in] pSsid      SSID to look for.
 * @param[in] timeoutMs  Maximum time to wait for the scan to complete.
 *
 * @return true when pSsid appeared in the scan results.
 */
bool TIREMO_ESP32_ScanForSsid(const char *pSsid, uint32_t timeoutMs);

/**
 * @brief Join a WiFi access point ("AT+CWJAP").
 *
 * Waits for "WIFI GOT IP". A framed ERROR (wrong password) returns
 * TIREMO_ESP32_JOIN_ERROR without waiting out timeoutMs. Silence until
 * timeoutMs is TIREMO_ESP32_JOIN_TIMEOUT.
 *
 * @param[in] pSsid      Access point SSID.
 * @param[in] pPassword  Access point password.
 * @param[in] timeoutMs  Maximum time to wait for "WIFI GOT IP".
 */
TIREMO_ESP32_JoinStatus_t TIREMO_ESP32_JoinAP(const char *pSsid,
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
 * Timezone 0 makes CIPSNTPTIME asctime UTC, so the parsed value is a
 * Unix epoch. A non-zero timezone would be local civil time and would
 * silently skew TIREMO_Clock_SetEpoch().
 *
 * @return true when the module acknowledged with "OK".
 */
bool TIREMO_ESP32_EnableSntp(void);

/**
 * @brief Query "AT+CIPSNTPTIME?" once and parse asctime into a Unix epoch.
 *
 * Matches the terminating "OK", then parses "+CIPSNTPTIME:". Matching
 * the marker itself would return before the asctime bytes arrived.
 * Unsynced modules report 1 Jan 1970 — that is rejected.
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

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_ESP32_H */
