/******************************************************************************
 * @file    tiremo_hal_flash.h
 * @brief   Tiremo data-flash hardware abstraction API.
 *
 * Provides raw erase/write/read access to the chip's Data Flash Memory
 * Controller (DFMC) region, used to persist small config blobs. This is a
 * single-sector primitive (no wear leveling, no filesystem) — callers own
 * the layout of what they store.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_HAL_FLASH_H
#define TIREMO_HAL_FLASH_H

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

/** @brief Erase/write granularity of the DFMC region on this chip. */
#define TIREMO_HAL_FLASH_SECTOR_SIZE    (2048U)

/******************************************************************************
 * Public Types
 *****************************************************************************/

typedef enum
{
    TIREMO_HAL_FLASH_STATUS_OK = 0,
    TIREMO_HAL_FLASH_STATUS_ERROR
} TIREMO_HAL_FLASH_Status_t;

/******************************************************************************
 * Public Function Prototypes
 *****************************************************************************/

/**
 * @brief Initialize the flash controller and cache the data-flash geometry.
 *
 * @return true when initialized successfully.
 */
bool TIREMO_HAL_FLASH_Init(void);

/**
 * @brief Get the usable size of the data-flash region, in bytes.
 */
uint32_t TIREMO_HAL_FLASH_GetSize(void);

/**
 * @brief Erase one TIREMO_HAL_FLASH_SECTOR_SIZE-aligned sector.
 *
 * @param[in] offset  Byte offset from the start of the data-flash region;
 *                     must be a multiple of TIREMO_HAL_FLASH_SECTOR_SIZE.
 *
 * @return TIREMO_HAL_FLASH_STATUS_OK on success.
 */
TIREMO_HAL_FLASH_Status_t TIREMO_HAL_FLASH_EraseSector(uint32_t offset);

/**
 * @brief Write data into a previously erased region.
 *
 * @param[in] offset  Byte offset from the start of the data-flash region.
 * @param[in] pData   Data to write.
 * @param[in] length  Number of bytes to write.
 *
 * @return TIREMO_HAL_FLASH_STATUS_OK on success.
 */
TIREMO_HAL_FLASH_Status_t TIREMO_HAL_FLASH_Write(uint32_t offset,
                                                 const uint8_t *pData,
                                                 uint32_t length);

/**
 * @brief Read data back from the data-flash region.
 *
 * Flash is memory-mapped, so this is a direct copy — always succeeds for an
 * in-range offset/length.
 *
 * @param[in]  offset  Byte offset from the start of the data-flash region.
 * @param[out] pData   Destination buffer.
 * @param[in]  length  Number of bytes to read.
 *
 * @return TIREMO_HAL_FLASH_STATUS_OK on success.
 */
TIREMO_HAL_FLASH_Status_t TIREMO_HAL_FLASH_Read(uint32_t offset,
                                                uint8_t *pData,
                                                uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_HAL_FLASH_H */
