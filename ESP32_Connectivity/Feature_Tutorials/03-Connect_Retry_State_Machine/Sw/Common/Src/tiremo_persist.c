/******************************************************************************
 * @file    tiremo_persist.c
 * @brief   Magic + CRC32 WiFi credential record in data-flash sector 0.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Include/tiremo_persist.h"
#include "../../Tiremo_Hal/Include/tiremo_hal_flash.h"

#include <stddef.h>
#include <string.h>

#define TIREMO_PERSIST_FLASH_OFFSET    (0U)

static uint32_t prvCrc32(const uint8_t *pData, uint32_t length)
{
    uint32_t crc = 0xFFFFFFFFUL;
    uint32_t i;
    uint32_t bit;

    for (i = 0U; i < length; i++)
    {
        crc ^= pData[i];
        for (bit = 0U; bit < 8U; bit++)
        {
            if ((crc & 1U) != 0U)
            {
                crc = (crc >> 1) ^ 0xEDB88320UL;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc ^ 0xFFFFFFFFUL;
}

static uint32_t prvRecordCrc(const TIREMO_PersistRecord_t *pRec)
{
    return prvCrc32((const uint8_t *)pRec,
                    (uint32_t)offsetof(TIREMO_PersistRecord_t, crc32));
}

bool TIREMO_Persist_Init(void)
{
    return TIREMO_HAL_FLASH_Init();
}

void TIREMO_Persist_Defaults(TIREMO_PersistRecord_t *pOut)
{
    if (pOut == NULL)
    {
        return;
    }

    memset(pOut, 0, sizeof(*pOut));
    pOut->magic = TIREMO_PERSIST_MAGIC;
    pOut->version = TIREMO_PERSIST_VERSION;
}

bool TIREMO_Persist_HasSsid(const TIREMO_PersistRecord_t *pRec)
{
    return ((pRec != NULL) && (pRec->ssid[0] != '\0'));
}

bool TIREMO_Persist_IsValid(const TIREMO_PersistRecord_t *pRec)
{
    if (pRec == NULL)
    {
        return false;
    }

    if ((pRec->magic != TIREMO_PERSIST_MAGIC) ||
        (pRec->version != TIREMO_PERSIST_VERSION))
    {
        return false;
    }

    return (pRec->crc32 == prvRecordCrc(pRec));
}

bool TIREMO_Persist_Load(TIREMO_PersistRecord_t *pOut)
{
    if (pOut == NULL)
    {
        return false;
    }

    if (TIREMO_HAL_FLASH_Read(TIREMO_PERSIST_FLASH_OFFSET,
                              (uint8_t *)pOut,
                              sizeof(*pOut)) != TIREMO_HAL_FLASH_STATUS_OK)
    {
        return false;
    }

    return TIREMO_Persist_IsValid(pOut);
}

bool TIREMO_Persist_Save(const TIREMO_PersistRecord_t *pIn)
{
    TIREMO_PersistRecord_t record;

    if (pIn == NULL)
    {
        return false;
    }

    record = *pIn;
    record.magic = TIREMO_PERSIST_MAGIC;
    record.version = TIREMO_PERSIST_VERSION;
    record.crc32 = prvRecordCrc(&record);

    if (TIREMO_HAL_FLASH_EraseSector(TIREMO_PERSIST_FLASH_OFFSET) !=
        TIREMO_HAL_FLASH_STATUS_OK)
    {
        return false;
    }

    return (TIREMO_HAL_FLASH_Write(TIREMO_PERSIST_FLASH_OFFSET,
                                   (const uint8_t *)&record,
                                   sizeof(record)) == TIREMO_HAL_FLASH_STATUS_OK);
}

bool TIREMO_Persist_Clear(void)
{
    TIREMO_PersistRecord_t empty;

    TIREMO_Persist_Defaults(&empty);
    return TIREMO_Persist_Save(&empty);
}
