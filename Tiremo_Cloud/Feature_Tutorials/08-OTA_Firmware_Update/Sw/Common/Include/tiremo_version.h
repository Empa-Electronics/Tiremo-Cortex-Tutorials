/******************************************************************************
 * @file    tiremo_version.h
 * @brief   Compiled-in firmware version string.
 *
 * The default is what this source tree builds as. Build the "new" image
 * the FIRMWARE_UPDATE job points at from this same tree with
 * -D TIREMO_FW_VERSION=cortex-08b, so the two images are byte-different
 * and the version reported to Tiremo Cloud after a swap actually changes.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_VERSION_H
#define TIREMO_VERSION_H

#ifndef TIREMO_FW_VERSION
#define TIREMO_FW_VERSION "cortex-08b"
#endif

#endif /* TIREMO_VERSION_H */
