/******************************************************************************
 * @file    prv_user_code.c
 * @brief   AT Command Transport -- power-cycle, AT/OK, ATE0.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Common/Include/tiremo_systick.h"
#include "../Common/Include/tiremo_debug.h"
#include "../Drivers/esp32/Include/tiremo_esp32.h"

#include <string.h>

static bool ResponseContainsEcho(const char *pResponse)
{
    const char *p;

    if (pResponse == NULL)
    {
        return false;
    }

    p = pResponse;
    while ((*p == '\r') || (*p == '\n'))
    {
        p++;
    }

    return (strncmp(p, "AT", 2U) == 0);
}

void PRV_USER_Code(void)
{
    char capture[96];
    bool present;
    bool echoOff;
    bool secondOk;

    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();

    TIREMO_LOG_INFO("AT_Command_Transport: boot");

    if (!TIREMO_ESP32_Init())
    {
        TIREMO_LOG_ERROR("ESP32: transport init failed");
        for (;;)
        {
        }
    }

    present = TIREMO_ESP32_IsPresent();
    if (!present)
    {
        TIREMO_LOG_ERROR("ESP32: AT probe failed (no OK)");
        for (;;)
        {
        }
    }
    TIREMO_LOG_INFO("ESP32: AT -> OK (module present)");

    capture[0] = '\0';
    (void)TIREMO_ESP32_SendCommandCapture("AT\r\n", "OK", 1000U,
                                          capture, sizeof(capture));
    TIREMO_LOG_INFO("ESP32: AT (echo still on) capture starts with echo=%s",
                    ResponseContainsEcho(capture) ? "yes" : "no");

    echoOff = TIREMO_ESP32_DisableEcho();
    if (!echoOff)
    {
        TIREMO_LOG_ERROR("ESP32: ATE0 failed");
        for (;;)
        {
        }
    }

    capture[0] = '\0';
    secondOk = TIREMO_ESP32_SendCommandCapture("AT\r\n", "OK", 1000U,
                                               capture, sizeof(capture));
    if (!secondOk)
    {
        TIREMO_LOG_ERROR("ESP32: AT after ATE0 failed");
        for (;;)
        {
        }
    }

    if (ResponseContainsEcho(capture))
    {
        TIREMO_LOG_WARN("ESP32: echoed 'AT' still in response after ATE0");
    }
    else
    {
        TIREMO_LOG_INFO("ESP32: AT after ATE0 is echo-free (OK only)");
    }

    for (;;)
    {
        TIREMO_SysTick_DelayMs(5000U);
        if (TIREMO_ESP32_IsPresent())
        {
            TIREMO_LOG_INFO("ESP32: heartbeat AT -> OK");
        }
        else
        {
            TIREMO_LOG_WARN("ESP32: heartbeat AT failed");
        }
    }
}
