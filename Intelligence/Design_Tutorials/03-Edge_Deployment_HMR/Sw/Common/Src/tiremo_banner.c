/******************************************************************************
 * @file    tiremo_banner.c
 * @brief   Tiremo Cortex boot banner implementation.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "../Include/tiremo_banner.h"
#include "../Include/tiremo_debug.h"

#include <stddef.h>

/******************************************************************************
 * Private Variables
 *****************************************************************************/

/*
 * The "tiremo cortex" wordmark from images/tiremo_sub_brand_1_mono_black.png,
 * traced at 100 columns. Needs a terminal at least 100 columns wide; narrower
 * ones wrap each row and the artwork falls apart.
 *
 * The two single-space rows are deliberate: tdf.py monitor discards empty
 * lines, so a truly blank row would collapse the gap between the two words.
 */
static const char *const s_logo[] = {
    "              ####",
    "   #####      ####",
    "   #####",
    "   #####",
    "############ #####   ###########   ###########     ############### ###########       ############",
    "############ #####   ########### ###############   ############################    ###############",
    "   #####     #####   ######     #####       #####  ######     #######     #####   ######      #####",
    "   #####     #####   #####      #################  #####       #####       ##### ######       ######",
    "   #####     #####   #####      #################  #####       #####       ##### #####         #####",
    "   #####     #####   #####      #####              #####       #####       #####  #####       ######",
    "   ######### #####   #####      ######    ######   #####       #####       #####  #######   #######",
    "    ######## #####   #####        #############    #####       #####       #####    #############",
    "      ###### #####   #####          #########      #####       #####       #####      #########",
    " ",
    " ",
    "                                                                    ###",
    "                                   ######       ######    ###  ## ########    #####    ####    #####",
    "                                 ##########  ###########  ####### ######## ###########  ####  ####",
    "                                ####         ###     #### ####      ###   ####     ####   #######",
    "                                ###         ####      ### ####      ###   #############    ####",
    "                                ####        ####      ### ####      ###   ####            #######",
    "                                 ##########  ###########  ####      #####  ###########  ####  ####",
    "                                   #######     #######    ####       ####    #######   ####    #####"
};

/******************************************************************************
 * Public Functions
 *****************************************************************************/

void TIREMO_Banner_Print(const char *pTitle)
{
    size_t index;

    if ((int)TIREMO_Debug_GetLevel() < (int)TIREMO_DEBUG_LEVEL_INFO)
    {
        return;
    }

    TIREMO_Debug_PutsLine(" ");

    for (index = 0U; index < (sizeof(s_logo) / sizeof(s_logo[0])); index++)
    {
        TIREMO_Debug_PutsLine(s_logo[index]);
    }

    TIREMO_Debug_PutsLine(" ");

    if (pTitle != NULL)
    {
        TIREMO_Debug_Puts("  ");
        TIREMO_Debug_PutsLine(pTitle);
    }

    TIREMO_Debug_PutsLine("  Design Tutorial - Empa Electronics");
    TIREMO_Debug_PutsLine(" ");
}
