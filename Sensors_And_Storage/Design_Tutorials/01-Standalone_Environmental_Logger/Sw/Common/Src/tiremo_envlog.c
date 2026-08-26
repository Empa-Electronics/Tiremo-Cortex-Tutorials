/******************************************************************************
 * @file    tiremo_envlog.c
 * @brief   Flash ring buffer of environmental samples.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Include/tiremo_envlog.h"
#include "../../Tiremo_Hal/Include/tiremo_hal_flash.h"

#include <stddef.h>
#include <string.h>

#define TIREMO_ENVLOG_FLASH_OFFSET    (0U)

typedef struct
{
    uint32_t magic;
    uint16_t version;
    uint16_t count;
    uint32_t nextSeq;
    uint32_t writeIndex;
    TIREMO_EnvSample_t samples[TIREMO_ENVLOG_CAPACITY];
    uint32_t crc32;
} TIREMO_EnvLogRecord_t;

static TIREMO_EnvLogRecord_t s_log;
static bool s_ready = false;

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

static uint32_t prvRecordCrc(const TIREMO_EnvLogRecord_t *pRec)
{
    return prvCrc32((const uint8_t *)pRec,
                    (uint32_t)offsetof(TIREMO_EnvLogRecord_t, crc32));
}

static bool prvIsValid(const TIREMO_EnvLogRecord_t *pRec)
{
    if ((pRec->magic != TIREMO_ENVLOG_MAGIC) ||
        (pRec->version != TIREMO_ENVLOG_VERSION) ||
        (pRec->count > TIREMO_ENVLOG_CAPACITY) ||
        (pRec->writeIndex >= TIREMO_ENVLOG_CAPACITY))
    {
        return false;
    }

    return (pRec->crc32 == prvRecordCrc(pRec));
}

static bool prvSave(void)
{
    s_log.magic = TIREMO_ENVLOG_MAGIC;
    s_log.version = TIREMO_ENVLOG_VERSION;
    s_log.crc32 = prvRecordCrc(&s_log);

    if (TIREMO_HAL_FLASH_EraseSector(TIREMO_ENVLOG_FLASH_OFFSET) !=
        TIREMO_HAL_FLASH_STATUS_OK)
    {
        return false;
    }

    return (TIREMO_HAL_FLASH_Write(TIREMO_ENVLOG_FLASH_OFFSET,
                                   (const uint8_t *)&s_log,
                                   sizeof(s_log)) == TIREMO_HAL_FLASH_STATUS_OK);
}

bool TIREMO_EnvLog_Init(void)
{
    TIREMO_EnvLogRecord_t loaded;

    if (!TIREMO_HAL_FLASH_Init())
    {
        return false;
    }

    memset(&s_log, 0, sizeof(s_log));
    s_log.magic = TIREMO_ENVLOG_MAGIC;
    s_log.version = TIREMO_ENVLOG_VERSION;

    if ((TIREMO_HAL_FLASH_Read(TIREMO_ENVLOG_FLASH_OFFSET,
                               (uint8_t *)&loaded,
                               sizeof(loaded)) == TIREMO_HAL_FLASH_STATUS_OK) &&
        prvIsValid(&loaded))
    {
        s_log = loaded;
    }

    s_ready = true;
    return true;
}

uint32_t TIREMO_EnvLog_Count(void)
{
    return s_ready ? (uint32_t)s_log.count : 0U;
}

bool TIREMO_EnvLog_GetOldest(uint32_t indexFromOldest, TIREMO_EnvSample_t *pOut)
{
    uint32_t oldest;

    if ((!s_ready) || (pOut == NULL) || (indexFromOldest >= s_log.count))
    {
        return false;
    }

    if (s_log.count < TIREMO_ENVLOG_CAPACITY)
    {
        oldest = 0U;
    }
    else
    {
        oldest = s_log.writeIndex;
    }

    *pOut = s_log.samples[(oldest + indexFromOldest) % TIREMO_ENVLOG_CAPACITY];
    return true;
}

bool TIREMO_EnvLog_Append(TIREMO_EnvSample_t *pSample)
{
    if ((!s_ready) || (pSample == NULL))
    {
        return false;
    }

    pSample->seq = s_log.nextSeq;
    s_log.nextSeq++;
    s_log.samples[s_log.writeIndex] = *pSample;
    s_log.writeIndex = (s_log.writeIndex + 1U) % TIREMO_ENVLOG_CAPACITY;
    if (s_log.count < TIREMO_ENVLOG_CAPACITY)
    {
        s_log.count++;
    }

    return prvSave();
}
