/******************************************************************************
 * @file    tiremo_identity.c
 * @brief   Device ID derived from a 48-bit hardware EUI (station MAC).
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "../Include/tiremo_identity.h"

#include <stddef.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

#define TIREMO_IDENTITY_MAC_TEXT_LEN  (17U)
#define TIREMO_IDENTITY_MAC_TEXT_SIZE (TIREMO_IDENTITY_MAC_TEXT_LEN + 1U)

/******************************************************************************
 * Private Functions
 *****************************************************************************/

static char prvNibbleToHex(uint8_t nibble)
{
    nibble &= 0x0FU;
    if (nibble < 10U)
    {
        return (char)('0' + nibble);
    }
    return (char)('a' + (nibble - 10U));
}

/******************************************************************************
 * Public Functions
 *****************************************************************************/

bool TIREMO_Identity_FromEui(const uint8_t pEui[6],
                             char *pOut,
                             uint32_t outSize)
{
    uint32_t i;

    if ((pEui == NULL) || (pOut == NULL) || (outSize < TIREMO_IDENTITY_ID_SIZE))
    {
        return false;
    }

    for (i = 0U; i < 6U; i++)
    {
        pOut[(i * 2U)] = prvNibbleToHex((uint8_t)(pEui[i] >> 4U));
        pOut[(i * 2U) + 1U] = prvNibbleToHex(pEui[i]);
    }
    pOut[TIREMO_IDENTITY_ID_LEN] = '\0';
    return true;
}

bool TIREMO_Identity_FormatMac(const uint8_t pEui[6],
                               char *pOut,
                               uint32_t outSize)
{
    uint32_t i;
    uint32_t cursor;

    if ((pEui == NULL) || (pOut == NULL) ||
        (outSize < TIREMO_IDENTITY_MAC_TEXT_SIZE))
    {
        return false;
    }

    cursor = 0U;
    for (i = 0U; i < 6U; i++)
    {
        if (i > 0U)
        {
            pOut[cursor++] = ':';
        }
        pOut[cursor++] = prvNibbleToHex((uint8_t)(pEui[i] >> 4U));
        pOut[cursor++] = prvNibbleToHex(pEui[i]);
    }
    pOut[cursor] = '\0';
    return true;
}
