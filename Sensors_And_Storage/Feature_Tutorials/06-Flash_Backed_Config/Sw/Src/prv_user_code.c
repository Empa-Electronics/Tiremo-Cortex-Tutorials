/******************************************************************************
 * @file    prv_user_code.c
 * @brief   Flash-Backed Config -- load, increment bootCount, save, demo CRC.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Common/Include/tiremo_systick.h"
#include "../Common/Include/tiremo_debug.h"
#include "../Common/Include/tiremo_persist.h"

#include <string.h>

void PRV_USER_Code(void)
{
    TIREMO_PersistRecord_t record;
    TIREMO_PersistRecord_t corrupt;

    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();

    TIREMO_LOG_INFO("Flash_Backed_Config: boot");

    if (!TIREMO_Persist_Init())
    {
        TIREMO_LOG_ERROR("persist: flash init failed");
        for (;;)
        {
        }
    }

    if (TIREMO_Persist_Load(&record))
    {
        TIREMO_LOG_INFO("persist: loaded bootCount=%u label=%s",
                        (unsigned)record.bootCount, record.label);
    }
    else
    {
        TIREMO_LOG_INFO("persist: no valid record, using defaults");
        TIREMO_Persist_Defaults(&record);
    }

    record.bootCount++;
    memcpy(record.label, "tutorial", 9);

    if (!TIREMO_Persist_Save(&record))
    {
        TIREMO_LOG_WARN("persist: save failed");
    }
    else
    {
        TIREMO_LOG_INFO("persist: saved bootCount=%u (survives reset)",
                        (unsigned)record.bootCount);
    }

    corrupt = record;
    corrupt.crc32 ^= 1U;
    if (TIREMO_Persist_IsValid(&corrupt))
    {
        TIREMO_LOG_WARN("persist: corrupt demo unexpectedly valid");
    }
    else
    {
        TIREMO_LOG_INFO("persist: corrupt CRC rejected (expected)");
    }

    for (;;)
    {
    }
}
