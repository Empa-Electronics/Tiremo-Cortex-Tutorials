/******************************************************************************
 * @file    tiremo_json.c
 * @brief   Bounded JSON find/write helpers for known document shapes.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "../Include/tiremo_json.h"

#include <stddef.h>
#include <string.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

#define TIREMO_JSON_WRITER_MAX_DEPTH  (8U)

/******************************************************************************
 * Private Functions
 *****************************************************************************/

static const char *prvEnd(const char *p, uint32_t len)
{
    return p + len;
}

static const char *prvSkipWs(const char *p, const char *pEnd)
{
    while ((p < pEnd) &&
           ((*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n')))
    {
        p++;
    }
    return p;
}

static bool prvSkipString(const char **pp, const char *pEnd)
{
    const char *p = *pp;

    if ((p >= pEnd) || (*p != '"'))
    {
        return false;
    }
    p++;
    while (p < pEnd)
    {
        if (*p == '\\')
        {
            p++;
            if (p >= pEnd)
            {
                return false;
            }
            p++;
            continue;
        }
        if (*p == '"')
        {
            *pp = p + 1;
            return true;
        }
        p++;
    }
    return false;
}

static bool prvSkipNumber(const char **pp, const char *pEnd)
{
    const char *p = *pp;

    if ((p < pEnd) && (*p == '-'))
    {
        p++;
    }
    if ((p >= pEnd) || (*p < '0') || (*p > '9'))
    {
        return false;
    }
    while ((p < pEnd) && (*p >= '0') && (*p <= '9'))
    {
        p++;
    }
    if ((p < pEnd) && (*p == '.'))
    {
        p++;
        while ((p < pEnd) && (*p >= '0') && (*p <= '9'))
        {
            p++;
        }
    }
    if ((p < pEnd) && ((*p == 'e') || (*p == 'E')))
    {
        p++;
        if ((p < pEnd) && ((*p == '+') || (*p == '-')))
        {
            p++;
        }
        while ((p < pEnd) && (*p >= '0') && (*p <= '9'))
        {
            p++;
        }
    }
    *pp = p;
    return true;
}

static bool prvSkipLiteral(const char **pp, const char *pEnd, const char *pLit)
{
    const char *p = *pp;
    uint32_t i;

    for (i = 0U; pLit[i] != '\0'; i++)
    {
        if ((p >= pEnd) || (*p != pLit[i]))
        {
            return false;
        }
        p++;
    }
    *pp = p;
    return true;
}

static bool prvSkipValue(const char **pp, const char *pEnd)
{
    const char *p = prvSkipWs(*pp, pEnd);
    uint32_t depth;

    if (p >= pEnd)
    {
        return false;
    }

    if (*p == '"')
    {
        if (!prvSkipString(&p, pEnd))
        {
            return false;
        }
        *pp = p;
        return true;
    }
    if ((*p == '{') || (*p == '['))
    {
        char open = *p;
        char close = (open == '{') ? '}' : ']';
        depth = 1U;
        p++;
        while ((p < pEnd) && (depth > 0U))
        {
            if (*p == '"')
            {
                if (!prvSkipString(&p, pEnd))
                {
                    return false;
                }
                continue;
            }
            if (*p == open)
            {
                depth++;
            }
            else if (*p == close)
            {
                depth--;
            }
            p++;
        }
        if (depth != 0U)
        {
            return false;
        }
        *pp = p;
        return true;
    }
    if (*p == 't')
    {
        if (!prvSkipLiteral(&p, pEnd, "true"))
        {
            return false;
        }
        *pp = p;
        return true;
    }
    if (*p == 'f')
    {
        if (!prvSkipLiteral(&p, pEnd, "false"))
        {
            return false;
        }
        *pp = p;
        return true;
    }
    if (*p == 'n')
    {
        if (!prvSkipLiteral(&p, pEnd, "null"))
        {
            return false;
        }
        *pp = p;
        return true;
    }
    if (!prvSkipNumber(&p, pEnd))
    {
        return false;
    }
    *pp = p;
    return true;
}

static bool prvKeysEqual(const char *pKey, uint32_t keyLen, const char *pWant)
{
    uint32_t i;

    for (i = 0U; i < keyLen; i++)
    {
        if (pWant[i] != pKey[i])
        {
            return false;
        }
    }
    return (pWant[keyLen] == '\0');
}

static bool prvFindValue(const char *pJson,
                         uint32_t jsonLen,
                         const char *pKey,
                         const char **ppValue,
                         uint32_t *pValueLen)
{
    const char *pEnd = prvEnd(pJson, jsonLen);
    const char *p = prvSkipWs(pJson, pEnd);
    const char *pKeyStart;
    const char *pValStart;
    const char *pValEnd;
    uint32_t keyLen;

    if ((pJson == NULL) || (pKey == NULL) || (pKey[0] == '\0') ||
        (ppValue == NULL) || (pValueLen == NULL))
    {
        return false;
    }
    if ((p >= pEnd) || (*p != '{'))
    {
        return false;
    }
    p++;

    p = prvSkipWs(p, pEnd);
    if ((p < pEnd) && (*p == '}'))
    {
        return false;
    }

    for (;;)
    {
        p = prvSkipWs(p, pEnd);
        if ((p >= pEnd) || (*p != '"'))
        {
            return false;
        }
        pKeyStart = p + 1;
        if (!prvSkipString(&p, pEnd))
        {
            return false;
        }
        keyLen = (uint32_t)((p - pKeyStart) - 1);

        p = prvSkipWs(p, pEnd);
        if ((p >= pEnd) || (*p != ':'))
        {
            return false;
        }
        p++;
        p = prvSkipWs(p, pEnd);
        pValStart = p;
        if (!prvSkipValue(&p, pEnd))
        {
            return false;
        }
        pValEnd = p;

        if (prvKeysEqual(pKeyStart, keyLen, pKey))
        {
            *ppValue = pValStart;
            *pValueLen = (uint32_t)(pValEnd - pValStart);
            return true;
        }

        p = prvSkipWs(p, pEnd);
        if ((p < pEnd) && (*p == ','))
        {
            p++;
            continue;
        }
        return false;
    }
}

static bool prvUnescape(const char *pIn,
                        uint32_t inLen,
                        char *pOut,
                        uint32_t outSize)
{
    uint32_t i;
    uint32_t o = 0U;

    if ((pOut == NULL) || (outSize == 0U))
    {
        return false;
    }

    for (i = 0U; i < inLen; i++)
    {
        char c = pIn[i];
        if (c == '\\')
        {
            i++;
            if (i >= inLen)
            {
                return false;
            }
            switch (pIn[i])
            {
                case '"':
                case '\\':
                case '/':
                    c = pIn[i];
                    break;
                case 'n':
                    c = '\n';
                    break;
                case 'r':
                    c = '\r';
                    break;
                case 't':
                    c = '\t';
                    break;
                default:
                    return false;
            }
        }
        if ((o + 1U) >= outSize)
        {
            return false;
        }
        pOut[o++] = c;
    }
    pOut[o] = '\0';
    return true;
}

static bool prvWriterPut(TIREMO_JsonWriter_t *pW, char c)
{
    if ((pW == NULL) || (!pW->ok))
    {
        return false;
    }
    if ((pW->used + 1U) >= pW->size)
    {
        pW->ok = false;
        return false;
    }
    pW->pBuf[pW->used++] = c;
    return true;
}

static bool prvWriterPuts(TIREMO_JsonWriter_t *pW, const char *pS)
{
    while (*pS != '\0')
    {
        if (!prvWriterPut(pW, *pS))
        {
            return false;
        }
        pS++;
    }
    return true;
}

static bool prvWriterQuoted(TIREMO_JsonWriter_t *pW, const char *pS)
{
    if (!prvWriterPut(pW, '"'))
    {
        return false;
    }
    while (*pS != '\0')
    {
        char c = *pS++;
        if ((c == '"') || (c == '\\'))
        {
            if (!prvWriterPut(pW, '\\') || !prvWriterPut(pW, c))
            {
                return false;
            }
        }
        else if (c == '\n')
        {
            if (!prvWriterPuts(pW, "\\n"))
            {
                return false;
            }
        }
        else if (c == '\r')
        {
            if (!prvWriterPuts(pW, "\\r"))
            {
                return false;
            }
        }
        else if (c == '\t')
        {
            if (!prvWriterPuts(pW, "\\t"))
            {
                return false;
            }
        }
        else if (!prvWriterPut(pW, c))
        {
            return false;
        }
    }
    return prvWriterPut(pW, '"');
}

static bool prvWriterComma(TIREMO_JsonWriter_t *pW)
{
    if ((pW == NULL) || (!pW->ok) || (pW->depth == 0U))
    {
        if (pW != NULL)
        {
            pW->ok = false;
        }
        return false;
    }
    if (pW->needComma[pW->depth])
    {
        return prvWriterPut(pW, ',');
    }
    return true;
}

/******************************************************************************
 * Public Functions
 *****************************************************************************/

bool TIREMO_Json_FindObject(const char *pJson,
                            uint32_t jsonLen,
                            const char *pKey,
                            const char **ppOut,
                            uint32_t *pOutLen)
{
    const char *pVal;
    uint32_t valLen;
    const char *p;
    const char *pEnd;

    if (!prvFindValue(pJson, jsonLen, pKey, &pVal, &valLen))
    {
        return false;
    }
    pEnd = pVal + valLen;
    p = prvSkipWs(pVal, pEnd);
    if ((p >= pEnd) || (*p != '{'))
    {
        return false;
    }
    *ppOut = p;
    *pOutLen = (uint32_t)(pEnd - p);
    return true;
}

bool TIREMO_Json_FindBool(const char *pJson,
                          uint32_t jsonLen,
                          const char *pKey,
                          bool *pOut)
{
    const char *pVal;
    uint32_t valLen;
    const char *p;
    const char *pEnd;

    if ((pOut == NULL) ||
        !prvFindValue(pJson, jsonLen, pKey, &pVal, &valLen))
    {
        return false;
    }
    pEnd = pVal + valLen;
    p = prvSkipWs(pVal, pEnd);
    if (prvSkipLiteral(&p, pEnd, "true") && (prvSkipWs(p, pEnd) == pEnd))
    {
        *pOut = true;
        return true;
    }
    p = prvSkipWs(pVal, pEnd);
    if (prvSkipLiteral(&p, pEnd, "false") && (prvSkipWs(p, pEnd) == pEnd))
    {
        *pOut = false;
        return true;
    }
    return false;
}

bool TIREMO_Json_FindUint(const char *pJson,
                          uint32_t jsonLen,
                          const char *pKey,
                          uint32_t *pOut)
{
    const char *pVal;
    uint32_t valLen;
    const char *p;
    const char *pEnd;
    uint32_t acc = 0U;
    bool any = false;

    if ((pOut == NULL) ||
        !prvFindValue(pJson, jsonLen, pKey, &pVal, &valLen))
    {
        return false;
    }
    pEnd = pVal + valLen;
    p = prvSkipWs(pVal, pEnd);
    if ((p < pEnd) && (*p == '-'))
    {
        return false;
    }
    while ((p < pEnd) && (*p >= '0') && (*p <= '9'))
    {
        uint32_t digit = (uint32_t)(*p - '0');
        if (acc > (((uint32_t)0xFFFFFFFFUL - digit) / 10U))
        {
            return false;
        }
        acc = (acc * 10U) + digit;
        any = true;
        p++;
    }
    p = prvSkipWs(p, pEnd);
    if ((!any) || (p != pEnd))
    {
        return false;
    }
    *pOut = acc;
    return true;
}

bool TIREMO_Json_FindString(const char *pJson,
                            uint32_t jsonLen,
                            const char *pKey,
                            char *pOut,
                            uint32_t outSize)
{
    const char *pVal;
    uint32_t valLen;
    const char *p;
    const char *pEnd;
    const char *pContent;
    uint32_t contentLen;

    if (!prvFindValue(pJson, jsonLen, pKey, &pVal, &valLen))
    {
        return false;
    }
    pEnd = pVal + valLen;
    p = prvSkipWs(pVal, pEnd);
    if ((p >= pEnd) || (*p != '"'))
    {
        return false;
    }
    pContent = p + 1;
    if (!prvSkipString(&p, pEnd))
    {
        return false;
    }
    if (prvSkipWs(p, pEnd) != pEnd)
    {
        return false;
    }
    contentLen = (uint32_t)((p - pContent) - 1);
    return prvUnescape(pContent, contentLen, pOut, outSize);
}

void TIREMO_JsonWriter_Init(TIREMO_JsonWriter_t *pW, char *pBuf, uint32_t size)
{
    uint32_t i;

    if (pW == NULL)
    {
        return;
    }
    pW->pBuf = pBuf;
    pW->size = size;
    pW->used = 0U;
    pW->depth = 0U;
    pW->ok = (pBuf != NULL) && (size >= 2U);
    for (i = 0U; i < TIREMO_JSON_WRITER_MAX_DEPTH; i++)
    {
        pW->needComma[i] = false;
    }
}

bool TIREMO_JsonWriter_ObjectStart(TIREMO_JsonWriter_t *pW)
{
    if ((pW == NULL) || (!pW->ok))
    {
        return false;
    }
    if (pW->depth > 0U)
    {
        if (!prvWriterComma(pW))
        {
            return false;
        }
    }
    if ((pW->depth + 1U) >= TIREMO_JSON_WRITER_MAX_DEPTH)
    {
        pW->ok = false;
        return false;
    }
    if (!prvWriterPut(pW, '{'))
    {
        return false;
    }
    pW->depth++;
    pW->needComma[pW->depth] = false;
    return true;
}

bool TIREMO_JsonWriter_ObjectEnd(TIREMO_JsonWriter_t *pW)
{
    if ((pW == NULL) || (!pW->ok) || (pW->depth == 0U))
    {
        if (pW != NULL)
        {
            pW->ok = false;
        }
        return false;
    }
    if (!prvWriterPut(pW, '}'))
    {
        return false;
    }
    pW->depth--;
    if (pW->depth > 0U)
    {
        pW->needComma[pW->depth] = true;
    }
    return true;
}

bool TIREMO_JsonWriter_Key(TIREMO_JsonWriter_t *pW, const char *pKey)
{
    if ((pKey == NULL) || (pKey[0] == '\0'))
    {
        if (pW != NULL)
        {
            pW->ok = false;
        }
        return false;
    }
    if (!prvWriterComma(pW))
    {
        return false;
    }
    if (!prvWriterQuoted(pW, pKey) || !prvWriterPut(pW, ':'))
    {
        return false;
    }
    pW->needComma[pW->depth] = false;
    return true;
}

bool TIREMO_JsonWriter_Bool(TIREMO_JsonWriter_t *pW, bool value)
{
    if (!prvWriterComma(pW))
    {
        return false;
    }
    if (!prvWriterPuts(pW, value ? "true" : "false"))
    {
        return false;
    }
    pW->needComma[pW->depth] = true;
    return true;
}

bool TIREMO_JsonWriter_Uint(TIREMO_JsonWriter_t *pW, uint32_t value)
{
    char tmp[10];
    uint32_t n = 0U;
    uint32_t v = value;
    uint32_t i;

    if (!prvWriterComma(pW))
    {
        return false;
    }
    if (v == 0U)
    {
        tmp[n++] = '0';
    }
    else
    {
        while (v > 0U)
        {
            tmp[n++] = (char)('0' + (v % 10U));
            v /= 10U;
        }
    }
    for (i = n; i > 0U; i--)
    {
        if (!prvWriterPut(pW, tmp[i - 1U]))
        {
            return false;
        }
    }
    pW->needComma[pW->depth] = true;
    return true;
}

bool TIREMO_JsonWriter_String(TIREMO_JsonWriter_t *pW, const char *pValue)
{
    if (pValue == NULL)
    {
        if (pW != NULL)
        {
            pW->ok = false;
        }
        return false;
    }
    if (!prvWriterComma(pW))
    {
        return false;
    }
    if (!prvWriterQuoted(pW, pValue))
    {
        return false;
    }
    pW->needComma[pW->depth] = true;
    return true;
}

bool TIREMO_JsonWriter_Raw(TIREMO_JsonWriter_t *pW,
                           const char *pRaw,
                           uint32_t rawLen)
{
    uint32_t i;

    if ((pRaw == NULL) && (rawLen > 0U))
    {
        if (pW != NULL)
        {
            pW->ok = false;
        }
        return false;
    }
    if (!prvWriterComma(pW))
    {
        return false;
    }
    for (i = 0U; i < rawLen; i++)
    {
        if (!prvWriterPut(pW, pRaw[i]))
        {
            return false;
        }
    }
    pW->needComma[pW->depth] = true;
    return true;
}

uint32_t TIREMO_JsonWriter_Finish(TIREMO_JsonWriter_t *pW)
{
    if ((pW == NULL) || (!pW->ok) || (pW->depth != 0U) || (pW->pBuf == NULL))
    {
        return 0U;
    }
    pW->pBuf[pW->used] = '\0';
    return pW->used;
}
