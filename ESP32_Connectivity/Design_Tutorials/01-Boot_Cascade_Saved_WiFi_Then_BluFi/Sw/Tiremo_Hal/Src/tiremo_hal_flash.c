/******************************************************************************
 * @file    tiremo_hal_flash.c
 * @brief   Tiremo data-flash hardware abstraction implementation.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "../Include/tiremo_hal_flash.h"

#include "abov_config.h"
#include "hal_fmc.h"

#include <stddef.h>
#include <string.h>

/******************************************************************************
 * Private Variables
 *****************************************************************************/

static bool s_initialized = false;
static uint32_t s_dataFlashBase = 0U;
static uint32_t s_dataFlashSize = 0U;

/******************************************************************************
 * Public Functions
 *****************************************************************************/

bool TIREMO_HAL_FLASH_Init(void)
{
    FMC_GEOMETRY_t geometry;

    if (s_initialized)
    {
        return true;
    }

    if (HAL_FMC_Init() != HAL_ERR_OK)
    {
        return false;
    }

    geometry = HAL_FMC_GetGeometry();
    s_dataFlashBase = geometry.tDflash.un32BaseAddr;
    s_dataFlashSize = geometry.tDflash.un32Size;

    if ((s_dataFlashBase == 0U) || (s_dataFlashSize == 0U))
    {
        return false;
    }

    s_initialized = true;
    return true;
}

uint32_t TIREMO_HAL_FLASH_GetSize(void)
{
    return s_dataFlashSize;
}

TIREMO_HAL_FLASH_Status_t TIREMO_HAL_FLASH_EraseSector(uint32_t offset)
{
    if ((!s_initialized) ||
        ((offset % TIREMO_HAL_FLASH_SECTOR_SIZE) != 0U) ||
        (offset >= s_dataFlashSize))
    {
        return TIREMO_HAL_FLASH_STATUS_ERROR;
    }

    if (HAL_FMC_Erase(FMC_ID_DFMC, offset, FMC_2KB_ERASE_MODE) != HAL_ERR_OK)
    {
        return TIREMO_HAL_FLASH_STATUS_ERROR;
    }

    return TIREMO_HAL_FLASH_STATUS_OK;
}

TIREMO_HAL_FLASH_Status_t TIREMO_HAL_FLASH_Write(uint32_t offset,
                                                 const uint8_t *pData,
                                                 uint32_t length)
{
    if ((!s_initialized) || (pData == NULL) || (length == 0U) ||
        ((offset + length) > s_dataFlashSize))
    {
        return TIREMO_HAL_FLASH_STATUS_ERROR;
    }

    if (HAL_FMC_Write(FMC_ID_DFMC, offset, (uint8_t *)pData, length) !=
        HAL_ERR_OK)
    {
        return TIREMO_HAL_FLASH_STATUS_ERROR;
    }

    return TIREMO_HAL_FLASH_STATUS_OK;
}

TIREMO_HAL_FLASH_Status_t TIREMO_HAL_FLASH_Read(uint32_t offset,
                                                uint8_t *pData,
                                                uint32_t length)
{
    const uint8_t *pSrc;

    if ((!s_initialized) || (pData == NULL) || (length == 0U) ||
        ((offset + length) > s_dataFlashSize))
    {
        return TIREMO_HAL_FLASH_STATUS_ERROR;
    }

    pSrc = (const uint8_t *)(s_dataFlashBase + offset);
    memcpy(pData, pSrc, length);

    return TIREMO_HAL_FLASH_STATUS_OK;
}
