/******************************************************************************
 * @file    tiremo_ota_state.c
 * @brief   OTA hand-off record in DFMC sector 0.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Include/tiremo_ota_state.h"
#include "../Include/tiremo_crc32.h"
#include "../../Tiremo_Hal/Include/tiremo_hal_flash.h"

#include <string.h>

#define OTA_STATE_OFFSET   (0U)   /* first data-flash sector */

/* CRC covers every byte up to (not including) the trailing crc32 field. */
#define OTA_STATE_CRC_LEN  (offsetof(TIREMO_OtaState_t, crc32))

bool TIREMO_OtaState_Init(void)
{
    return TIREMO_HAL_FLASH_Init();
}

bool TIREMO_OtaState_Load(TIREMO_OtaState_t *pOut)
{
    TIREMO_OtaState_t rec;

    if (pOut == NULL)
    {
        return false;
    }
    if (TIREMO_HAL_FLASH_Read(OTA_STATE_OFFSET, (uint8_t *)&rec,
                              sizeof(rec)) != TIREMO_HAL_FLASH_STATUS_OK)
    {
        return false;
    }
    if (rec.magic != TIREMO_OTA_STATE_MAGIC)
    {
        return false;
    }
    if (rec.crc32 != TIREMO_Crc32(&rec, OTA_STATE_CRC_LEN))
    {
        return false;
    }

    *pOut = rec;
    return true;
}

bool TIREMO_OtaState_Save(TIREMO_OtaState_t *pIn)
{
    if (pIn == NULL)
    {
        return false;
    }

    pIn->magic = TIREMO_OTA_STATE_MAGIC;
    pIn->version = (uint16_t)TIREMO_OTA_STATE_VERSION;
    pIn->crc32 = TIREMO_Crc32(pIn, OTA_STATE_CRC_LEN);

    if (TIREMO_HAL_FLASH_EraseSector(OTA_STATE_OFFSET) !=
        TIREMO_HAL_FLASH_STATUS_OK)
    {
        return false;
    }
    return (TIREMO_HAL_FLASH_Write(OTA_STATE_OFFSET, (const uint8_t *)pIn,
                                   sizeof(*pIn)) ==
            TIREMO_HAL_FLASH_STATUS_OK);
}

bool TIREMO_OtaState_Clear(void)
{
    return (TIREMO_HAL_FLASH_EraseSector(OTA_STATE_OFFSET) ==
            TIREMO_HAL_FLASH_STATUS_OK);
}
