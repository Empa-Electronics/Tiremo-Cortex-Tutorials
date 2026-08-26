/******************************************************************************
 * @file    tiremo_persist.h
 * @brief   Last-good WiFi credentials in data flash (magic + CRC32).
 *
 * Same CRC and sector-0 layout as Sensors tutorial 06, different magic
 * ("WF01") so this record does not collide with TRM1/ENV1.
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

/** @brief Marks a valid WiFi persist record ("WF01"). */
#define TIREMO_PERSIST_MAGIC         (0x57463031UL)
#define TIREMO_PERSIST_VERSION       (1U)
#define TIREMO_PERSIST_SSID_LEN      (33U)
#define TIREMO_PERSIST_PASSWORD_LEN  (65U)

typedef struct
{
    uint32_t magic;
    uint16_t version;
    uint16_t reserved;
    char ssid[TIREMO_PERSIST_SSID_LEN];
    char password[TIREMO_PERSIST_PASSWORD_LEN];
    uint32_t crc32;
} TIREMO_PersistRecord_t;

bool TIREMO_Persist_Init(void);
void TIREMO_Persist_Defaults(TIREMO_PersistRecord_t *pOut);
bool TIREMO_Persist_IsValid(const TIREMO_PersistRecord_t *pRec);
bool TIREMO_Persist_Load(TIREMO_PersistRecord_t *pOut);
bool TIREMO_Persist_Save(const TIREMO_PersistRecord_t *pIn);
bool TIREMO_Persist_Clear(void);
bool TIREMO_Persist_HasSsid(const TIREMO_PersistRecord_t *pRec);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_PERSIST_H */
