/******************************************************************************
 * @file    prv_user_code.c
 * @brief   Microphone RMS to dB -- capture loop, log RMS and dB once a second.
 *
 * The driver is copied from the reference firmware (BSP + app). This file
 * is the tutorial policy: poll Service() and print GetVoiceLevel() /
 * GetVoiceLevelDb(). Talking near the mic should raise the dB figure.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Common/Include/tiremo_systick.h"
#include "../Common/Include/tiremo_debug.h"
#include "../Drivers/mic/Include/tiremo_mic.h"

#include <stdint.h>

#define LOG_INTERVAL_MS (1000U)

void TIREMO_MIC_OnVoiceAlert(TIREMO_MIC_VoiceResult_t result)
{
    const char *pName = "NORMAL";

    if (result.alert == TIREMO_MIC_ALERT_HIGH_VOICE)
    {
        pName = "HIGH_VOICE";
    }
    else if (result.alert == TIREMO_MIC_ALERT_SILENCE)
    {
        pName = "SILENCE";
    }

    TIREMO_LOG_INFO("MIC alert: %s  rms=%u  dB=%d",
                    pName,
                    (unsigned)result.level,
                    (int)TIREMO_MIC_RmsToDb(result.level));
}

void PRV_USER_Code(void)
{
    uint32_t now;
    uint32_t lastLogMs;

    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();

    TIREMO_LOG_INFO("Microphone_RMS_To_dB: boot");

    if (!TIREMO_MIC_Init())
    {
        TIREMO_LOG_ERROR("MIC: init failed");
        for (;;)
        {
        }
    }

    if (!TIREMO_MIC_Start())
    {
        TIREMO_LOG_ERROR("MIC: start failed");
        for (;;)
        {
        }
    }

    TIREMO_LOG_INFO("MIC: capturing (talk near the mic vs silence)");
    lastLogMs = TIREMO_SysTick_GetMs();

    for (;;)
    {
        TIREMO_MIC_Service();

        now = TIREMO_SysTick_GetMs();
        if ((now - lastLogMs) >= LOG_INTERVAL_MS)
        {
            uint32_t rms = TIREMO_MIC_GetVoiceLevel();

            TIREMO_LOG_INFO("MIC: blocks=%u  rms=%u  dB=%d",
                            (unsigned)TIREMO_MIC_GetDmaBlockCount(),
                            (unsigned)rms,
                            (int)TIREMO_MIC_GetVoiceLevelDb());
            lastLogMs = now;
        }
    }
}
