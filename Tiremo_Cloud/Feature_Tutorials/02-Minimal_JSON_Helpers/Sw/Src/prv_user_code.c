/******************************************************************************
 * @file    prv_user_code.c
 * @brief   Minimal JSON helpers -- parse a sample shadow, write reported,
 *          re-parse.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Common/Include/tiremo_systick.h"
#include "../Common/Include/tiremo_debug.h"
#include "../Common/Include/tiremo_json.h"

#include <string.h>

static const char kShadow[] =
    "{"
    "\"state\":{"
    "\"desired\":{"
    "\"reporting\":{\"enabled\":true,\"intervalS\":60},"
    "\"led\":{\"on\":false},"
    "\"note\":\"colon:not-a-key\""
    "},"
    "\"reported\":{"
    "\"reporting\":{\"enabled\":false,\"intervalS\":30}"
    "}"
    "},"
    "\"version\":7"
    "}";

static uint32_t s_fails;

static void Expect(bool ok, const char *pWhat)
{
    if (ok)
    {
        TIREMO_LOG_INFO("json: %s", pWhat);
    }
    else
    {
        TIREMO_LOG_ERROR("json: FAIL %s", pWhat);
        s_fails++;
    }
}

static void Hang(const char *pWhy)
{
    TIREMO_LOG_ERROR("%s", pWhy);
    for (;;)
    {
    }
}

void PRV_USER_Code(void)
{
    const char *pState;
    const char *pDesired;
    const char *pReporting;
    const char *pLed;
    uint32_t stateLen;
    uint32_t desiredLen;
    uint32_t reportingLen;
    uint32_t ledLen;
    bool enabled = false;
    bool ledOn = true;
    uint32_t intervalS = 0U;
    uint32_t version = 0U;
    char note[24];
    TIREMO_JsonWriter_t writer;
    char reported[160];
    uint32_t written;
    const char *pRepReporting;
    uint32_t pRepReportingLen;
    bool echoEnabled = false;
    uint32_t echoInterval = 0U;

    (void)TIREMO_SysTick_Init();
    (void)TIREMO_Debug_Init();

    TIREMO_LOG_INFO("Minimal_JSON_Helpers: boot");
    s_fails = 0U;

    Expect(TIREMO_Json_FindObject(kShadow, (uint32_t)strlen(kShadow),
                                  "state", &pState, &stateLen),
           "FindObject state");
    Expect(TIREMO_Json_FindObject(pState, stateLen, "desired",
                                  &pDesired, &desiredLen),
           "FindObject desired");
    Expect(TIREMO_Json_FindObject(pDesired, desiredLen, "reporting",
                                  &pReporting, &reportingLen),
           "FindObject reporting");
    Expect(TIREMO_Json_FindBool(pReporting, reportingLen, "enabled", &enabled) &&
               (enabled == true),
           "FindBool reporting.enabled == true");
    Expect(TIREMO_Json_FindUint(pReporting, reportingLen, "intervalS",
                                &intervalS) &&
               (intervalS == 60U),
           "FindUint reporting.intervalS == 60");
    Expect(TIREMO_Json_FindObject(pDesired, desiredLen, "led", &pLed, &ledLen),
           "FindObject led");
    Expect(TIREMO_Json_FindBool(pLed, ledLen, "on", &ledOn) && (ledOn == false),
           "FindBool led.on == false");
    Expect(TIREMO_Json_FindString(pDesired, desiredLen, "note",
                                  note, sizeof(note)) &&
               (strcmp(note, "colon:not-a-key") == 0),
           "FindString note (colon inside string)");
    Expect(TIREMO_Json_FindUint(kShadow, (uint32_t)strlen(kShadow),
                                "version", &version) &&
               (version == 7U),
           "FindUint version == 7");
    Expect(!TIREMO_Json_FindBool(kShadow, (uint32_t)strlen(kShadow),
                                 "enabled", &enabled),
           "root has no sibling key enabled");

    TIREMO_JsonWriter_Init(&writer, reported, sizeof(reported));
    (void)TIREMO_JsonWriter_ObjectStart(&writer);
    (void)TIREMO_JsonWriter_Key(&writer, "state");
    (void)TIREMO_JsonWriter_ObjectStart(&writer);
    (void)TIREMO_JsonWriter_Key(&writer, "reported");
    (void)TIREMO_JsonWriter_ObjectStart(&writer);
    (void)TIREMO_JsonWriter_Key(&writer, "reporting");
    (void)TIREMO_JsonWriter_ObjectStart(&writer);
    (void)TIREMO_JsonWriter_Key(&writer, "enabled");
    (void)TIREMO_JsonWriter_Bool(&writer, enabled);
    (void)TIREMO_JsonWriter_Key(&writer, "intervalS");
    (void)TIREMO_JsonWriter_Uint(&writer, intervalS);
    (void)TIREMO_JsonWriter_ObjectEnd(&writer);
    (void)TIREMO_JsonWriter_Key(&writer, "led");
    (void)TIREMO_JsonWriter_ObjectStart(&writer);
    (void)TIREMO_JsonWriter_Key(&writer, "on");
    (void)TIREMO_JsonWriter_Bool(&writer, ledOn);
    (void)TIREMO_JsonWriter_ObjectEnd(&writer);
    (void)TIREMO_JsonWriter_Key(&writer, "note");
    (void)TIREMO_JsonWriter_String(&writer, note);
    (void)TIREMO_JsonWriter_ObjectEnd(&writer);
    (void)TIREMO_JsonWriter_ObjectEnd(&writer);
    (void)TIREMO_JsonWriter_ObjectEnd(&writer);
    written = TIREMO_JsonWriter_Finish(&writer);
    Expect(written > 0U, "writer finish");

    if (written > 0U)
    {
        TIREMO_LOG_INFO("json: wrote %s", reported);
        Expect(TIREMO_Json_FindObject(reported, written, "state",
                                      &pState, &stateLen),
               "re-parse state");
        Expect(TIREMO_Json_FindObject(pState, stateLen, "reported",
                                      &pDesired, &desiredLen),
               "re-parse reported");
        Expect(TIREMO_Json_FindObject(pDesired, desiredLen, "reporting",
                                      &pRepReporting, &pRepReportingLen),
               "re-parse reporting");
        Expect(TIREMO_Json_FindBool(pRepReporting, pRepReportingLen,
                                    "enabled", &echoEnabled) &&
                   (echoEnabled == true),
               "echo enabled == true");
        Expect(TIREMO_Json_FindUint(pRepReporting, pRepReportingLen,
                                    "intervalS", &echoInterval) &&
                   (echoInterval == 60U),
               "echo intervalS == 60");
    }

    if (s_fails != 0U)
    {
        Hang("json: one or more checks failed");
    }

    TIREMO_LOG_INFO("json: round-trip ok");

    for (;;)
    {
        TIREMO_SysTick_DelayMs(5000U);
        TIREMO_LOG_INFO("json: still ok");
    }
}
