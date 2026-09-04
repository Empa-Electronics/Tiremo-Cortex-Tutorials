/******************************************************************************
 * @file    tiremo_ota_state.h
 * @brief   One magic + CRC32 record in DATA flash that survives an OTA.
 *
 * The OTA swaps the CODE-flash bank, so anything the post-swap image
 * needs to know -- which job to report, which version it should now be,
 * which bank to fall back to -- has to live somewhere the swap does not
 * touch. Data flash (DFMC) is a separate controller and region, so a
 * small record there rides through the reset untouched.
 *
 * Same layout idea as tiremo_persist from Sensors_And_Storage/06, with an
 * OTA-shaped payload. One 2 KB sector, no wear levelling -- an update is
 * a rare event.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_OTA_STATE_H
#define TIREMO_OTA_STATE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

/** @brief Marks a valid record ("OTA1"). */
#define TIREMO_OTA_STATE_MAGIC       (0x4F544131UL)
#define TIREMO_OTA_STATE_VERSION     (1U)
#define TIREMO_OTA_STATE_JOBID_LEN   (48U)
#define TIREMO_OTA_STATE_VER_LEN     (24U)

/** @brief Where the update is in the switch -> confirm -> done cycle. */
typedef enum
{
    TIREMO_OTA_PHASE_NONE = 0,       /**< no update in flight */
    TIREMO_OTA_PHASE_PENDING_CONFIRM = 1, /**< boot bank moved; new image must check in */
    TIREMO_OTA_PHASE_ROLLED_BACK = 2, /**< new image never checked in; boot bank moved back */
    TIREMO_OTA_PHASE_ARMED = 3       /**< multi-bank enable written, awaiting the reset that makes it real */
} TIREMO_OtaPhase_t;

typedef struct
{
    uint32_t magic;
    uint16_t version;
    uint16_t phase;                          /**< TIREMO_OtaPhase_t */
    char     jobId[TIREMO_OTA_STATE_JOBID_LEN];
    char     expectVersion[TIREMO_OTA_STATE_VER_LEN];
    uint32_t prevBank;                       /**< TIREMO_FlashAB_Bank_t before the switch */
    uint32_t bootCount;                      /**< boots seen since the switch */
    uint32_t reasonCode;                     /**< why a rollback happened (0 = none) */
    uint32_t crc32;                          /**< over the struct up to this field */
} TIREMO_OtaState_t;

/** @brief Bring up data flash. Call once before Load/Save/Clear. */
bool TIREMO_OtaState_Init(void);

/**
 * @brief Load the record.
 *
 * @param[out] pOut  Filled only on success.
 *
 * @return true when a magic- and CRC-valid record was read.
 */
bool TIREMO_OtaState_Load(TIREMO_OtaState_t *pOut);

/**
 * @brief Erase the sector and write @p pIn (magic/version/crc filled in here).
 */
bool TIREMO_OtaState_Save(TIREMO_OtaState_t *pIn);

/** @brief Erase the sector -- next Load() fails, i.e. phase NONE. */
bool TIREMO_OtaState_Clear(void);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_OTA_STATE_H */
