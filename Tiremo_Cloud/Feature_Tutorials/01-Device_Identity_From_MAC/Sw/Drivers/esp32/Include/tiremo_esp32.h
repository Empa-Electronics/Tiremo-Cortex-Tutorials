/******************************************************************************
 * @file    tiremo_esp32.h
 * @brief   ESP32 AT-command transport plus station MAC (CIPSTAMAC).
 *
 * Power sequencing, an interrupt-driven RX ring, an expect/timeout
 * command primitive, and GetHwEui(). WiFi / NTP / BluFi / MQTT live
 * in other tutorials.
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
 * @brief Read the ESP32 station MAC ("AT+CIPSTAMAC?").
 *
 * The module answers `+CIPSTAMAC:"aa:bb:cc:dd:ee:ff"` then `OK`. This
 * waits for the terminating `OK` (the quoted address sits *after*
 * `+CIPSTAMAC:` on the same line, and RX is one byte at a time) then
 * parses six hex bytes.
 *
 * WiFi does not need to be joined. The factory MAC is unique per
 * module and stable across power cycles.
 *
 * @param[out] pEui  Six-byte IEEE 48-bit address, MSB first.
 *
 * @return true when six bytes were parsed from a well-formed reply.
 */
bool TIREMO_ESP32_GetHwEui(uint8_t pEui[6]);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_ESP32_H */
