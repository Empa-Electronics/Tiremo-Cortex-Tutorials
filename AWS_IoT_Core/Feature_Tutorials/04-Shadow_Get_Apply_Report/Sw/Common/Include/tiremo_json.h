/******************************************************************************
 * @file    tiremo_json.h
 * @brief   Bounded JSON find/write helpers for known document shapes.
 *
 * Not a general parser. Keys are matched at the current object depth
 * only; nest by feeding a FindObject slice back in. Strings with
 * colons or braces do not confuse the scan.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_JSON_H
#define TIREMO_JSON_H

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
 * Public Types
 *****************************************************************************/

typedef struct
{
    char *pBuf;
    uint32_t size;
    uint32_t used;
    uint8_t depth;
    bool needComma[8];
    bool ok;
} TIREMO_JsonWriter_t;

/******************************************************************************
 * Public Function Prototypes
 *****************************************************************************/

/**
 * @brief Locate a sibling object's value, including its braces.
 *
 * @param[in]  pJson    Object text (must start with `{` after whitespace).
 * @param[in]  jsonLen  Byte length of pJson.
 * @param[in]  pKey     Unquoted key name.
 * @param[out] ppOut    Pointer into pJson at the `{`.
 * @param[out] pOutLen  Length of the object, braces included.
 */
bool TIREMO_Json_FindObject(const char *pJson,
                            uint32_t jsonLen,
                            const char *pKey,
                            const char **ppOut,
                            uint32_t *pOutLen);

bool TIREMO_Json_FindBool(const char *pJson,
                          uint32_t jsonLen,
                          const char *pKey,
                          bool *pOut);

bool TIREMO_Json_FindUint(const char *pJson,
                          uint32_t jsonLen,
                          const char *pKey,
                          uint32_t *pOut);

/**
 * @brief Copy an unquoted string value (simple `\\` / `\"` / `\/` / `\\n`
 *        / `\\r` / `\\t` escapes only).
 */
bool TIREMO_Json_FindString(const char *pJson,
                            uint32_t jsonLen,
                            const char *pKey,
                            char *pOut,
                            uint32_t outSize);

void TIREMO_JsonWriter_Init(TIREMO_JsonWriter_t *pW,
                            char *pBuf,
                            uint32_t size);

bool TIREMO_JsonWriter_ObjectStart(TIREMO_JsonWriter_t *pW);
bool TIREMO_JsonWriter_ObjectEnd(TIREMO_JsonWriter_t *pW);
bool TIREMO_JsonWriter_Key(TIREMO_JsonWriter_t *pW, const char *pKey);
bool TIREMO_JsonWriter_Bool(TIREMO_JsonWriter_t *pW, bool value);
bool TIREMO_JsonWriter_Uint(TIREMO_JsonWriter_t *pW, uint32_t value);
bool TIREMO_JsonWriter_String(TIREMO_JsonWriter_t *pW, const char *pValue);

/**
 * @brief Insert a complete JSON value that is already serialized.
 *
 * Used to echo `state.desired` into `state.reported` without rebuilding
 * nested objects key by key -- the shadow delta only clears when the
 * two objects match.
 */
bool TIREMO_JsonWriter_Raw(TIREMO_JsonWriter_t *pW,
                           const char *pRaw,
                           uint32_t rawLen);

/** @brief NUL-terminate and return the written length, or 0 on failure. */
uint32_t TIREMO_JsonWriter_Finish(TIREMO_JsonWriter_t *pW);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_JSON_H */
