/******************************************************************************
 * @file    tiremo_crc32.h
 * @brief   Reflected CRC-32 (poly 0xEDB88320), the zlib/PNG variant.
 *
 * Used to gate the OTA image: the value the Tiremo Job document carries
 * must match a fresh CRC over the staged bytes, and a fresh CRC over the
 * bytes read back from the inactive bank must match that same value.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_CRC32_H
#define TIREMO_CRC32_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/**
 * @brief One-shot CRC-32 over a buffer.
 *
 * Equivalent to Init() -> Update() -> Final().
 */
uint32_t TIREMO_Crc32(const void *pData, uint32_t length);

/** @brief Seed value for a streaming CRC (before the first Update). */
uint32_t TIREMO_Crc32_Init(void);

/**
 * @brief Fold @p length more bytes into a running CRC.
 *
 * @param[in] crc     Running value from Init() or a previous Update().
 * @param[in] pData    Bytes to fold in.
 * @param[in] length   Number of bytes.
 *
 * @return Updated running value (not yet finalized).
 */
uint32_t TIREMO_Crc32_Update(uint32_t crc, const void *pData, uint32_t length);

/** @brief Finalize a running CRC into its reported value. */
uint32_t TIREMO_Crc32_Final(uint32_t crc);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_CRC32_H */
