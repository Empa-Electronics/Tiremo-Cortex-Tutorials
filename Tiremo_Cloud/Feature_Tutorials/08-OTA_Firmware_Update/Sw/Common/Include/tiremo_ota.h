/******************************************************************************
 * @file    tiremo_ota.h
 * @brief   Job-triggered A/B firmware update: stage, verify, swap, confirm.
 *
 * TIREMO_OTA_Init() runs first thing in PRV_USER_Code(): it makes sure
 * the CFMC is in multi-bank mode and reads the DFMC hand-off record so a
 * freshly-swapped image knows it must report its job, and a swap that
 * never checked in gets rolled back.
 *
 * TIREMO_OTA_Apply() is called from the Jobs handler on a FIRMWARE_UPDATE
 * job: it HTTP-downloads the image named by the job document, streaming
 * it straight into the inactive bank, verifies it (against the job's
 * CRC-32 when it carries one, and always against a flash read-back),
 * records the hand-off, points the boot bank at it, and resets. It only
 * returns on failure.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_OTA_H
#define TIREMO_OTA_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

/** @brief What TIREMO_OTA_Init() found about the boot it is running in. */
typedef enum
{
    TIREMO_OTA_BOOT_NORMAL = 0,        /**< no update in flight */
    TIREMO_OTA_BOOT_CONFIRM_PENDING,   /**< this is the new image; report SUCCEEDED, then TIREMO_OTA_ConfirmRunning() */
    TIREMO_OTA_BOOT_ROLLED_BACK        /**< a prior swap failed; report FAILED, then TIREMO_OTA_ConfirmRunning() */
} TIREMO_OtaBoot_t;

/**
 * @brief Bring up flash, force multi-bank mode, read the hand-off record.
 *
 * May reset the MCU (once, on a factory-fresh single-bank part) before it
 * returns.
 *
 * @return How this boot relates to a pending update -- see TIREMO_OtaBoot_t.
 */
TIREMO_OtaBoot_t TIREMO_OTA_Init(void);

/** @brief The version string this running image was compiled with. */
const char *TIREMO_OTA_RunningVersion(void);

/**
 * @brief Job id tied to a CONFIRM_PENDING / ROLLED_BACK boot ("" otherwise).
 */
const char *TIREMO_OTA_PendingJobId(void);

/** @brief Short reason for a ROLLED_BACK boot (for the job's FAILED detail). */
const char *TIREMO_OTA_RollbackReason(void);

/**
 * @brief Download and apply the image named by a FIRMWARE_UPDATE job.
 *
 * The jobDocument must carry a download URL, and should carry a CRC-32
 * to authenticate the content: Tiremo Cloud's own shape --
 * fileInfo.s3Path (url), fileInfo.checksum {function:"CRC32",
 * value:"<base64>"}, fileInfo.fileSizeKB -- or a hand-written job's flat
 * "url" / "crc32" (decimal or 0x-hex) / "size" / "version" keys; both
 * are accepted. A job with no crc32 still applies -- only the
 * flash-write self-verify (download CRC vs. read-back-from-flash CRC)
 * runs, logged with a WARN.
 *
 * @param[in] pJobId    Job id to record for the post-swap report.
 * @param[in] pJobDoc   jobDocument object text (NULL/empty -> "no_url").
 * @param[in] docLen    Byte length of pJobDoc.
 *
 * @return false on a pre-swap failure (caller reports the job FAILED with
 *         TIREMO_OTA_LastError()); does not return on success -- it resets
 *         into the new bank.
 */
bool TIREMO_OTA_Apply(const char *pJobId, const char *pJobDoc, uint32_t docLen);

/** @brief Machine-readable reason the last TIREMO_OTA_Apply() returned false. */
const char *TIREMO_OTA_LastError(void);

/**
 * @brief Clear the hand-off record -- the running image is now confirmed
 *        good. Call once the CONFIRM_PENDING / ROLLED_BACK job result has
 *        been published.
 */
void TIREMO_OTA_ConfirmRunning(void);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_OTA_H */
