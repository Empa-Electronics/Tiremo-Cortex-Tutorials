/******************************************************************************
 * @file    tiremo_persist.h
 * @brief   Magic + CRC32 record in data flash (tutorial-sized config).
 *
 * Same layout idea as the production TIREMO_APP persisted config, stripped
 * to a boot counter and a short label so this tutorial stays one sector.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_PERSIST_H
#define TIREMO_PERSIST_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

/** @brief Marks a valid persisted record ("TRM1"). */
#define TIREMO_PERSIST_MAGIC      (0x54524D31UL)
#define TIREMO_PERSIST_VERSION    (1U)
#define TIREMO_PERSIST_LABEL_LEN  (16U)

typedef struct
{
    uint32_t magic;
    uint16_t version;
    uint16_t reserved;
    uint32_t bootCount;
    char label[TIREMO_PERSIST_LABEL_LEN];
    uint32_t crc32;
} TIREMO_PersistRecord_t;

bool TIREMO_Persist_Init(void);
void TIREMO_Persist_Defaults(TIREMO_PersistRecord_t *pOut);
bool TIREMO_Persist_IsValid(const TIREMO_PersistRecord_t *pRec);
bool TIREMO_Persist_Load(TIREMO_PersistRecord_t *pOut);
bool TIREMO_Persist_Save(const TIREMO_PersistRecord_t *pIn);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_PERSIST_H */
