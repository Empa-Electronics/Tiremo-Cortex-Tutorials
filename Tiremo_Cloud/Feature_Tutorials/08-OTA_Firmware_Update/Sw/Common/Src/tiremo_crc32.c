/******************************************************************************
 * @file    tiremo_crc32.c
 * @brief   Reflected CRC-32 (poly 0xEDB88320), bitwise -- no 1 KB table.
 *
 * The OTA image is a few tens of KB and CRC'd at most twice per update
 * (once over the staged bytes, once over the bank read-back), so the
 * bitwise inner loop costs a handful of milliseconds and is not worth a
 * lookup table in flash.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Include/tiremo_crc32.h"

#include <stddef.h>

uint32_t TIREMO_Crc32_Init(void)
{
    return 0xFFFFFFFFUL;
}

uint32_t TIREMO_Crc32_Update(uint32_t crc, const void *pData, uint32_t length)
{
    const uint8_t *p = (const uint8_t *)pData;
    uint32_t i;

    if (p == NULL)
    {
        return crc;
    }

    for (i = 0U; i < length; ++i)
    {
        uint32_t bit;

        crc ^= (uint32_t)p[i];
        for (bit = 0U; bit < 8U; ++bit)
        {
            uint32_t mask = (uint32_t)-(int32_t)(crc & 1U);

            crc = (crc >> 1) ^ (0xEDB88320UL & mask);
        }
    }
    return crc;
}

uint32_t TIREMO_Crc32_Final(uint32_t crc)
{
    return crc ^ 0xFFFFFFFFUL;
}

uint32_t TIREMO_Crc32(const void *pData, uint32_t length)
{
    return TIREMO_Crc32_Final(
        TIREMO_Crc32_Update(TIREMO_Crc32_Init(), pData, length));
}
