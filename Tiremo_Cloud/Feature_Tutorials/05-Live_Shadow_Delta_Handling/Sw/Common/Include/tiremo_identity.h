/******************************************************************************
 * @file    tiremo_identity.h
 * @brief   Device ID derived from a 48-bit hardware EUI (station MAC).
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_IDENTITY_H
#define TIREMO_IDENTITY_H

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
 * Public Macros
 *****************************************************************************/

/** @brief 12 lowercase hex digits, no separators. */
#define TIREMO_IDENTITY_ID_LEN   (12U)
/** @brief Buffer size including the terminating NUL. */
#define TIREMO_IDENTITY_ID_SIZE  (TIREMO_IDENTITY_ID_LEN + 1U)

/******************************************************************************
 * Public Function Prototypes
 *****************************************************************************/

/**
 * @brief Format a 48-bit EUI as a 12-digit lowercase hex device ID.
 *
 * No colons, no `0x`, no uppercase. Later tutorials use this string as
 * the AWS IoT Thing name and MQTT client ID so identity is hardware,
 * not a constant baked into firmware.
 *
 * @param[in]  pEui     Six-byte MAC from TIREMO_ESP32_GetHwEui().
 * @param[out] pOut     Destination; must hold TIREMO_IDENTITY_ID_SIZE.
 * @param[in]  outSize  Size of pOut, including the NUL.
 *
 * @return true when pOut was filled.
 */
bool TIREMO_Identity_FromEui(const uint8_t pEui[6],
                             char *pOut,
                             uint32_t outSize);

/**
 * @brief Format a 48-bit EUI as `aa:bb:cc:dd:ee:ff` for human logs.
 *
 * @param[in]  pEui     Six-byte MAC.
 * @param[out] pOut     Destination; 18 bytes including NUL.
 * @param[in]  outSize  Size of pOut, including the NUL.
 *
 * @return true when pOut was filled.
 */
bool TIREMO_Identity_FormatMac(const uint8_t pEui[6],
                               char *pOut,
                               uint32_t outSize);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_IDENTITY_H */
