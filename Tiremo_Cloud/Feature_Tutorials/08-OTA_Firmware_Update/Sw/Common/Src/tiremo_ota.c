/******************************************************************************
 * @file    tiremo_ota.c
 * @brief   Job-triggered A/B firmware update: download, verify, swap, confirm.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Include/tiremo_ota.h"
#include "../Include/tiremo_ota_state.h"
#include "../Include/tiremo_flash_ab.h"
#include "../Include/tiremo_crc32.h"
#include "../Include/tiremo_json.h"
#include "../Include/tiremo_systick.h"
#include "../Include/tiremo_debug.h"
#include "../Include/tiremo_version.h"
#include "../../Drivers/esp32/Include/tiremo_esp32.h"

#include "abov_config.h"   /* NVIC_SystemReset */
#include "hal_scu.h"       /* HAL_SCU_SetReset / HAL_SCU_SetSWReset */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OTA_RESET_DRAIN_MS       (20U)
#define OTA_MAX_UNCONFIRMED_BOOTS (1U)  /* new image gets this many boots to check in */
/* A pre-signed S3 URL from a Tiremo Cloud FOTA job (host + long key path
 * + a full set of X-Amz-* query params incl. an STS security token) runs
 * to several KB. */
#define OTA_URL_MAX              (6144U)
#define OTA_DL_TIMEOUT_MS        (120000U)  /* whole HTTP transfer budget */

/* Scratch for one code-flash sector while the download is reassembled
 * into bank-aligned writes, and the download URL. bss, not stack. */
static uint8_t s_otaSector[TIREMO_FLASHAB_SECTOR_SIZE];
static char s_otaUrl[OTA_URL_MAX];
/* First bytes of the download -- printed if the transfer is far too
 * small to be firmware (typically an S3 XML error body). */
static char s_otaHead[512];

typedef struct
{
    uint32_t sectorIndex;   /* next inactive-bank sector to program */
    uint32_t inSector;      /* bytes buffered in s_otaSector */
    uint32_t crc;           /* running CRC over the real (unpadded) bytes */
    uint32_t total;         /* bytes accepted so far */
    bool     ok;
} OtaDlCtx_t;

#define OTA_RC_NONE             (0U)
#define OTA_RC_NO_CONFIRM       (1U)
#define OTA_RC_SWITCH_FAILED    (2U)

static bool s_ready = false;
static char s_lastErr[24] = "";
static char s_pendingJobId[TIREMO_OTA_STATE_JOBID_LEN] = "";
static uint32_t s_rollbackReason = OTA_RC_NONE;

static void prvResetNow(void)
{
    TIREMO_SysTick_DelayMs(OTA_RESET_DRAIN_MS);

    /* The CFMC boot ROM only re-reads the bank-switch user-info command
     * on an SCU software reset (SCU_SRCR.SWRST) -- a CMSIS SYSRESETREQ
     * does NOT switch the bank. Enable the SW-reset source in SCU_RSER,
     * then pull the trigger; do not fall back to NVIC_SystemReset(),
     * which would reset without swapping. */
    (void)HAL_SCU_SetReset(SCU_RST_SW, true);
    HAL_SCU_SetSWReset();

    for (;;)
    {
        /* reset is imminent */
    }
}

static void prvSetErr(const char *pWhy)
{
    (void)snprintf(s_lastErr, sizeof(s_lastErr), "%s", pWhy);
}

static bool prvEnsureReady(void)
{
    if (s_ready)
    {
        return true;
    }
    if (!TIREMO_FlashAB_Init() || !TIREMO_OtaState_Init())
    {
        return false;
    }
    s_ready = true;
    return true;
}

const char *TIREMO_OTA_RunningVersion(void)
{
    return TIREMO_FW_VERSION;
}

const char *TIREMO_OTA_PendingJobId(void)
{
    return s_pendingJobId;
}

const char *TIREMO_OTA_RollbackReason(void)
{
    switch (s_rollbackReason)
    {
        case OTA_RC_NO_CONFIRM:    return "rollback_no_confirm";
        case OTA_RC_SWITCH_FAILED: return "rollback_switch_failed";
        default:                   return "rollback";
    }
}

const char *TIREMO_OTA_LastError(void)
{
    return (s_lastErr[0] != '\0') ? s_lastErr : "ota_error";
}

TIREMO_OtaBoot_t TIREMO_OTA_Init(void)
{
    TIREMO_OtaState_t st;
    TIREMO_FlashAB_Bank_t active;
    bool justEnabled = false;

    if (!prvEnsureReady())
    {
        TIREMO_LOG_ERROR("ota: flash init failed");
        return TIREMO_OTA_BOOT_NORMAL;
    }

    /* A factory-fresh part is single-bank: enabling multi-bank mode writes
     * a CFMC user-info command that only takes effect on the next reset.
     * Arm it once (a DFMC marker stops a re-arm loop if it never sticks),
     * then reset -- all before any job runs. */
    if (TIREMO_FlashAB_ActiveBank() == TIREMO_FLASHAB_BANK_SINGLE)
    {
        TIREMO_OtaState_t armed;

        if (TIREMO_OtaState_Load(&armed) &&
            (armed.phase == (uint16_t)TIREMO_OTA_PHASE_ARMED))
        {
            TIREMO_LOG_ERROR("ota: multi-bank enable did not persist -- staying single-bank");
            (void)TIREMO_OtaState_Clear();
            return TIREMO_OTA_BOOT_NORMAL;
        }

        memset(&armed, 0, sizeof(armed));
        armed.phase = (uint16_t)TIREMO_OTA_PHASE_ARMED;
        (void)TIREMO_OtaState_Save(&armed);

        if (!TIREMO_FlashAB_EnableMultiBank(&justEnabled))
        {
            (void)TIREMO_OtaState_Clear();
            TIREMO_LOG_ERROR("ota: multi-bank enable failed");
            return TIREMO_OTA_BOOT_NORMAL;
        }
        TIREMO_LOG_INFO("ota: CFMC multi-bank armed, resetting to create banks");
        prvResetNow();
    }

    active = TIREMO_FlashAB_ActiveBank();

    /* multi-bank is live -- drop a stale arm marker from the boot before */
    {
        TIREMO_OtaState_t stArm;

        if (TIREMO_OtaState_Load(&stArm) &&
            (stArm.phase == (uint16_t)TIREMO_OTA_PHASE_ARMED))
        {
            (void)TIREMO_OtaState_Clear();
        }
    }
    TIREMO_LOG_INFO("ota: running %s from bank %s",
                    TIREMO_FW_VERSION,
                    (active == TIREMO_FLASHAB_BANK_A) ? "A" :
                    (active == TIREMO_FLASHAB_BANK_B) ? "B" : "?");

    if (!TIREMO_OtaState_Load(&st) ||
        (st.phase == (uint16_t)TIREMO_OTA_PHASE_NONE))
    {
        return TIREMO_OTA_BOOT_NORMAL;
    }

    (void)snprintf(s_pendingJobId, sizeof(s_pendingJobId), "%s", st.jobId);

    if (st.phase == (uint16_t)TIREMO_OTA_PHASE_ROLLED_BACK)
    {
        s_rollbackReason = st.reasonCode;
        TIREMO_LOG_WARN("ota: prior update rolled back (job %s)", st.jobId);
        return TIREMO_OTA_BOOT_ROLLED_BACK;
    }

    /* phase == PENDING_CONFIRM */
    if ((uint32_t)active == st.prevBank)
    {
        /* boot bank did not actually move -- treat as a failed switch */
        st.phase = (uint16_t)TIREMO_OTA_PHASE_ROLLED_BACK;
        st.reasonCode = OTA_RC_SWITCH_FAILED;
        (void)TIREMO_OtaState_Save(&st);
        s_rollbackReason = OTA_RC_SWITCH_FAILED;
        TIREMO_LOG_WARN("ota: boot bank never moved (job %s) -- FAILED", st.jobId);
        return TIREMO_OTA_BOOT_ROLLED_BACK;
    }

    st.bootCount++;
    if (st.bootCount > OTA_MAX_UNCONFIRMED_BOOTS)
    {
        /* the new image booted but never checked in -- roll the boot
         * bank back to where we came from and report FAILED after reset. */
        st.phase = (uint16_t)TIREMO_OTA_PHASE_ROLLED_BACK;
        st.reasonCode = OTA_RC_NO_CONFIRM;
        (void)TIREMO_OtaState_Save(&st);
        TIREMO_LOG_WARN("ota: new image did not check in -- rolling back to bank %s",
                        (st.prevBank == (uint32_t)TIREMO_FLASHAB_BANK_A) ? "A" : "B");
        (void)TIREMO_FlashAB_SetNextBoot((TIREMO_FlashAB_Bank_t)st.prevBank);
        prvResetNow();
    }

    (void)TIREMO_OtaState_Save(&st);   /* bootCount persisted */
    TIREMO_LOG_INFO("ota: this is the new image for job %s (expect %s) -- will confirm",
                    st.jobId, st.expectVersion);
    return TIREMO_OTA_BOOT_CONFIRM_PENDING;
}

static int prvB64Val(char c)
{
    if ((c >= 'A') && (c <= 'Z')) { return (int)(c - 'A'); }
    if ((c >= 'a') && (c <= 'z')) { return (int)(c - 'a') + 26; }
    if ((c >= '0') && (c <= '9')) { return (int)(c - '0') + 52; }
    if (c == '+') { return 62; }
    if (c == '/') { return 63; }
    return -1;
}

/**
 * @brief Decode the first 4 bytes of a base64 string as a big-endian
 *        uint32 -- the encoding AWS/Tiremo Cloud use for a CRC32 checksum.
 */
static bool prvB64ToCrc32(const char *pB64, uint32_t *pOut)
{
    int v[6];
    int i;

    for (i = 0; i < 6; ++i)
    {
        v[i] = prvB64Val(pB64[i]);
        if (v[i] < 0)
        {
            return false;   /* '=' padding reached before 4 bytes -- malformed */
        }
    }
    *pOut = ((uint32_t)((v[0] << 2) | (v[1] >> 4)) << 24) |
            ((uint32_t)(((v[1] << 4) | (v[2] >> 2)) & 0xFF) << 16) |
            ((uint32_t)(((v[2] << 6) | v[3]) & 0xFF) << 8) |
            (uint32_t)(((v[4] << 2) | (v[5] >> 4)) & 0xFF);
    return true;
}

/**
 * @brief Read a CRC32 from an object: a nested {"function":"CRC32",
 *        "value":"<base64>"} (Tiremo Cloud), or a flat "crc32" number /
 *        0x-string (a hand-written job).
 */
static bool prvGetCrc32(const char *pObj, uint32_t objLen, uint32_t *pCrc)
{
    const char *pCk = NULL;
    uint32_t ckLen = 0U;
    char buf[24];

    if (TIREMO_Json_FindObject(pObj, objLen, "checksum", &pCk, &ckLen))
    {
        char fn[12];

        fn[0] = '\0';
        (void)TIREMO_Json_FindString(pCk, ckLen, "function", fn, sizeof(fn));
        if (TIREMO_Json_FindString(pCk, ckLen, "value", buf, sizeof(buf)) &&
            (buf[0] != '\0') &&
            ((fn[0] == '\0') || (strstr(fn, "CRC32") != NULL) ||
             (strstr(fn, "crc32") != NULL)))
        {
            return prvB64ToCrc32(buf, pCrc);
        }
    }

    if (TIREMO_Json_FindUint(pObj, objLen, "crc32", pCrc))
    {
        return true;
    }
    if (TIREMO_Json_FindString(pObj, objLen, "crc32", buf, sizeof(buf)) &&
        (buf[0] != '\0'))
    {
        *pCrc = (uint32_t)strtoul(buf, NULL, 0);
        return true;
    }
    return false;
}

/**
 * @brief Pull url / version / size / crc32 out of a jobDocument.
 *
 * Tiremo Cloud shape: url at fileInfo.s3Path, version at
 * operationInfo.firmwareVersion, size at fileInfo.fileSizeKB (a KB
 * string -- so *pSizeExact is false), crc32 at fileInfo.checksum. A
 * hand-written job may instead use flat "url" / "version" / "size" /
 * "crc32" keys (then *pSizeExact is true).
 */
static bool prvParseDoc(const char *pDoc, uint32_t docLen,
                        char *pUrlOut, uint32_t urlOutSz,
                        char *pVerOut, uint32_t verOutSz,
                        uint32_t *pSize, bool *pHaveSize, bool *pSizeExact,
                        uint32_t *pCrc, bool *pHaveCrc)
{
    const char *pObj = NULL;
    uint32_t objLen = 0U;
    char numBuf[16];

    pUrlOut[0] = '\0';
    pVerOut[0] = '\0';
    *pSize = 0U;
    *pHaveSize = false;
    *pSizeExact = false;
    *pCrc = 0U;
    *pHaveCrc = false;

    if ((pDoc == NULL) || (docLen == 0U))
    {
        return false;
    }

    if (TIREMO_Json_FindObject(pDoc, docLen, "fileInfo", &pObj, &objLen))
    {
        (void)TIREMO_Json_FindString(pObj, objLen, "s3Path", pUrlOut, urlOutSz);
        if (pUrlOut[0] == '\0')
        {
            (void)TIREMO_Json_FindString(pObj, objLen, "url", pUrlOut, urlOutSz);
        }
        if (TIREMO_Json_FindUint(pObj, objLen, "fileSize", pSize) ||
            TIREMO_Json_FindUint(pObj, objLen, "size", pSize))
        {
            *pHaveSize = true;
            *pSizeExact = true;
        }
        else if (TIREMO_Json_FindString(pObj, objLen, "fileSizeKB", numBuf,
                                        sizeof(numBuf)) && (numBuf[0] != '\0'))
        {
            uint32_t kb = (uint32_t)strtoul(numBuf, NULL, 10);

            if (kb > 0U)
            {
                *pSize = kb * 1024U;
                *pHaveSize = true;   /* KB-rounded -> erase bound only */
            }
        }
        *pHaveCrc = prvGetCrc32(pObj, objLen, pCrc);
    }
    if (pUrlOut[0] == '\0')
    {
        (void)TIREMO_Json_FindString(pDoc, docLen, "url", pUrlOut, urlOutSz);
    }

    if (TIREMO_Json_FindObject(pDoc, docLen, "operationInfo", &pObj, &objLen))
    {
        (void)TIREMO_Json_FindString(pObj, objLen, "firmwareVersion",
                                     pVerOut, verOutSz);
    }
    if (pVerOut[0] == '\0')
    {
        (void)TIREMO_Json_FindString(pDoc, docLen, "version", pVerOut, verOutSz);
    }

    if (!*pHaveSize && TIREMO_Json_FindUint(pDoc, docLen, "size", pSize))
    {
        *pHaveSize = true;
        *pSizeExact = true;
    }
    if (!*pHaveCrc)
    {
        *pHaveCrc = prvGetCrc32(pDoc, docLen, pCrc);
    }
    return true;
}

/**
 * @brief HTTP body sink: CRC each run, buffer it a sector at a time, and
 *        program full sectors into the inactive bank as they fill.
 */
static bool prvOtaSink(uint32_t offset, const uint8_t *pData, uint32_t len,
                       void *pUser)
{
    OtaDlCtx_t *pCtx = (OtaDlCtx_t *)pUser;

    (void)offset;   /* the sink tracks its own running total */

    if ((pCtx->total + len) > TIREMO_FlashAB_BankSize())
    {
        TIREMO_LOG_ERROR("ota: download exceeds bank at %lu bytes",
                         (unsigned long)(pCtx->total + len));
        pCtx->ok = false;
        return false;
    }

    if (pCtx->total < (sizeof(s_otaHead) - 1U))
    {
        uint32_t room = (uint32_t)sizeof(s_otaHead) - 1U - pCtx->total;
        uint32_t n = (len < room) ? len : room;

        memcpy(&s_otaHead[pCtx->total], pData, n);
        s_otaHead[pCtx->total + n] = '\0';
    }

    pCtx->crc = TIREMO_Crc32_Update(pCtx->crc, pData, len);

    while (len > 0U)
    {
        uint32_t space = TIREMO_FLASHAB_SECTOR_SIZE - pCtx->inSector;
        uint32_t take = (len < space) ? len : space;

        memcpy(&s_otaSector[pCtx->inSector], pData, take);
        pCtx->inSector += take;
        pData += take;
        len -= take;
        pCtx->total += take;

        if (pCtx->inSector == TIREMO_FLASHAB_SECTOR_SIZE)
        {
            if (!TIREMO_FlashAB_OtaWriteSector(pCtx->sectorIndex, s_otaSector))
            {
                pCtx->ok = false;
                return false;
            }
            pCtx->sectorIndex++;
            pCtx->inSector = 0U;
        }
    }
    return true;
}

bool TIREMO_OTA_Apply(const char *pJobId, const char *pJobDoc, uint32_t docLen)
{
    TIREMO_OtaState_t st;
    TIREMO_FlashAB_Bank_t active;
    TIREMO_FlashAB_Bank_t next;
    OtaDlCtx_t ctx;
    char wantVer[TIREMO_OTA_STATE_VER_LEN];
    uint32_t wantSize;
    uint32_t wantCrc;
    uint32_t crcDl;
    uint32_t crcBank;
    uint32_t dlTotal = 0U;
    bool haveSize;
    bool sizeExact;
    bool haveCrc;

    s_lastErr[0] = '\0';

    if ((pJobId == NULL) || (pJobId[0] == '\0'))
    {
        prvSetErr("no_job_id");
        return false;
    }
    if (!prvEnsureReady())
    {
        prvSetErr("flash_init");
        return false;
    }

    (void)prvParseDoc(pJobDoc, docLen, s_otaUrl, sizeof(s_otaUrl),
                      wantVer, sizeof(wantVer),
                      &wantSize, &haveSize, &sizeExact, &wantCrc, &haveCrc);

    if (s_otaUrl[0] == '\0')
    {
        prvSetErr("no_url");
        return false;
    }
    if (haveSize && (wantSize > TIREMO_FlashAB_BankSize()))
    {
        prvSetErr("image_too_big");
        return false;
    }
    if (haveCrc)
    {
        TIREMO_LOG_INFO("ota: job crc32=%08lX size%s%lu",
                        (unsigned long)wantCrc, sizeExact ? "=" : "~",
                        (unsigned long)wantSize);
    }
    else
    {
        /* The job's provenance is still authenticated -- the pre-signed
         * URL arrived over the mTLS MQTT channel -- but without a
         * content checksum only the flash-write self-verify runs. */
        TIREMO_LOG_WARN("ota: job carries no crc32 -- flash self-verify only");
    }

    active = TIREMO_FlashAB_ActiveBank();
    next = TIREMO_FlashAB_OtherBank(active);
    if (next == TIREMO_FLASHAB_BANK_UNKNOWN)
    {
        prvSetErr("bank_state");
        return false;
    }

    TIREMO_LOG_INFO("ota: downloading %s -> bank %s",
                    (wantVer[0] != '\0') ? wantVer : "update",
                    (next == TIREMO_FLASHAB_BANK_A) ? "A" : "B");
    TIREMO_Debug_Puts("[INFO] ota: url = ");
    TIREMO_Debug_PutsLine(s_otaUrl);

    if (!TIREMO_FlashAB_OtaBegin(haveSize ? wantSize : 0U))
    {
        prvSetErr("flash_begin");
        return false;
    }

    memset(&ctx, 0, sizeof(ctx));
    ctx.crc = TIREMO_Crc32_Init();
    ctx.ok = true;
    s_otaHead[0] = '\0';

    if (!TIREMO_ESP32_HttpGetToSink(s_otaUrl, prvOtaSink, &ctx, &dlTotal,
                                    OTA_DL_TIMEOUT_MS) || (!ctx.ok))
    {
        prvSetErr("download_failed");
        return false;
    }

    /* Far too small to be a firmware image -- almost always an S3 XML
     * error body. Print it; it names the real cause. */
    if (ctx.total < 4096U)
    {
        TIREMO_LOG_WARN("ota: download only %lu bytes -- server response:",
                        (unsigned long)ctx.total);
        TIREMO_Debug_PutsLine(s_otaHead);
    }

    /* Flush the short final sector (0xFF-padded to the erase granularity). */
    if (ctx.inSector > 0U)
    {
        memset(&s_otaSector[ctx.inSector], 0xFF,
               TIREMO_FLASHAB_SECTOR_SIZE - ctx.inSector);
        if (!TIREMO_FlashAB_OtaWriteSector(ctx.sectorIndex, s_otaSector))
        {
            prvSetErr("flash_tail");
            return false;
        }
    }

    if (ctx.total == 0U)
    {
        prvSetErr("empty_download");
        return false;
    }
    if (haveSize && sizeExact && (ctx.total != wantSize))
    {
        TIREMO_LOG_ERROR("ota: got %lu bytes, job said %lu",
                         (unsigned long)ctx.total, (unsigned long)wantSize);
        prvSetErr("size_mismatch");
        return false;
    }
    if (haveSize && !sizeExact &&
        ((ctx.total > wantSize) || (ctx.total + 1024U <= wantSize)))
    {
        /* fileSizeKB is KB-rounded: real size is (wantSize-1024, wantSize]. */
        TIREMO_LOG_ERROR("ota: got %lu bytes, outside job's ~%lu",
                         (unsigned long)ctx.total, (unsigned long)wantSize);
        prvSetErr("size_mismatch");
        return false;
    }

    crcDl = TIREMO_Crc32_Final(ctx.crc);
    if (haveCrc && (crcDl != wantCrc))
    {
        TIREMO_LOG_ERROR("ota: download crc %08lX != job %08lX",
                         (unsigned long)crcDl, (unsigned long)wantCrc);
        prvSetErr("crc_mismatch");
        return false;
    }

    crcBank = TIREMO_FlashAB_Crc32Other(ctx.total);
    if (crcBank != crcDl)
    {
        TIREMO_LOG_ERROR("ota: bank verify %08lX != %08lX",
                         (unsigned long)crcBank, (unsigned long)crcDl);
        prvSetErr("verify_mismatch");
        return false;
    }
    TIREMO_LOG_INFO("ota: bank %s verified -- %lu bytes, crc %08lX",
                    (next == TIREMO_FLASHAB_BANK_A) ? "A" : "B",
                    (unsigned long)ctx.total, (unsigned long)crcBank);

    memset(&st, 0, sizeof(st));
    st.phase = (uint16_t)TIREMO_OTA_PHASE_PENDING_CONFIRM;
    (void)snprintf(st.jobId, sizeof(st.jobId), "%s", pJobId);
    (void)snprintf(st.expectVersion, sizeof(st.expectVersion), "%s",
                   (wantVer[0] != '\0') ? wantVer : "downloaded");
    st.prevBank = (uint32_t)active;
    st.bootCount = 0U;
    st.reasonCode = OTA_RC_NONE;

    if (!TIREMO_OtaState_Save(&st))
    {
        prvSetErr("state_save");
        return false;
    }
    if (!TIREMO_FlashAB_SetNextBoot(next))
    {
        prvSetErr("set_boot");
        return false;
    }

    TIREMO_LOG_INFO("ota: boot bank -> %s, resetting into the new image",
                    (next == TIREMO_FLASHAB_BANK_A) ? "A" : "B");
    prvResetNow();
    return true;   /* not reached */
}

void TIREMO_OTA_ConfirmRunning(void)
{
    (void)TIREMO_OtaState_Clear();
    s_pendingJobId[0] = '\0';
    s_rollbackReason = OTA_RC_NONE;
    TIREMO_LOG_INFO("ota: confirmed running %s (bank marked good)",
                    TIREMO_FW_VERSION);
}
