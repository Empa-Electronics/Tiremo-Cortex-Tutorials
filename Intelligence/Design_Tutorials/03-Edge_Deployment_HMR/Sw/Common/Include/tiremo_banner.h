/******************************************************************************
 * @file    tiremo_banner.h
 * @brief   Tiremo Cortex boot banner.
 *
 * Design Tutorials print the Tiremo Cortex wordmark once at boot so a bare
 * terminal identifies the board and the running application. Feature
 * Tutorials deliberately do not -- they stay minimal.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_BANNER_H
#define TIREMO_BANNER_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * Public Function Prototypes
 *****************************************************************************/

/**
 * @brief Print the Tiremo Cortex logo followed by an application title.
 *
 * Written straight to the UART without a `[INFO]` prefix so the artwork is
 * not broken up. Suppressed when the runtime log level is below INFO.
 *
 * @param[in] pTitle  Application name, or NULL to print the logo alone.
 *
 * @note The artwork is 100 columns wide; narrower terminals wrap it.
 * @warning Call after TIREMO_Debug_Init().
 */
void TIREMO_Banner_Print(const char *pTitle);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_BANNER_H */
