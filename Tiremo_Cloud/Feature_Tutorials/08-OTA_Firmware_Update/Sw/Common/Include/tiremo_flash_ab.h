/******************************************************************************
 * @file    tiremo_flash_ab.h
 * @brief   Code-flash A/B bank access for OTA (ABOV CFMC multi-bank).
 *
 * The A34G43x code flash is one linear 2 MB region that the CFMC can run
 * as two equal banks. Whichever bank booted is mapped at 0x0; the other
 * bank is always visible at BankSize() from the running image's point of
 * view, which is where a new image is staged. The boot-bank selection
 * lives in CFMC user info and only takes effect after a reset -- so an
 * update is: write the other bank, verify it, point boot at it, reset.
 *
 * The image is identical for either bank (the CFMC remaps the active
 * bank to 0x0), so there is no second linker script and no
 * position-independent code.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_FLASH_AB_H
#define TIREMO_FLASH_AB_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

/** @brief Code-flash erase/program sector on this part. */
#define TIREMO_FLASHAB_SECTOR_SIZE   (0x1000U)   /* 4 KB */

/**
 * @brief Bank identifier. Values match the vendor CFMC_BANK_ID_e so a
 *        cast is enough and the number is safe to persist.
 */
typedef enum
{
    TIREMO_FLASHAB_BANK_SINGLE = 0,
    TIREMO_FLASHAB_BANK_A = 1,
    TIREMO_FLASHAB_BANK_B = 2,
    TIREMO_FLASHAB_BANK_UNKNOWN = 3
} TIREMO_FlashAB_Bank_t;

/**
 * @brief Bring up the code-flash controller and cache the bank geometry.
 *
 * @return true on success.
 */
bool TIREMO_FlashAB_Init(void);

/**
 * @brief Size of one bank in bytes (code-flash size / 2).
 */
uint32_t TIREMO_FlashAB_BankSize(void);

/**
 * @brief The bank the running image booted from.
 *
 * @return TIREMO_FLASHAB_BANK_A / _B, or _SINGLE when multi-bank mode is
 *         not enabled yet, or _UNKNOWN on a HAL error.
 */
TIREMO_FlashAB_Bank_t TIREMO_FlashAB_ActiveBank(void);

/**
 * @brief The other bank relative to @p bank (A<->B; anything else -> _UNKNOWN).
 */
TIREMO_FlashAB_Bank_t TIREMO_FlashAB_OtherBank(TIREMO_FlashAB_Bank_t bank);

/**
 * @brief Enable CFMC multi-bank mode if the part is still single-bank.
 *
 * @param[out] pJustEnabled  Set true when this call switched the mode on,
 *                            meaning the caller must reset before the two
 *                            banks exist. Set false when it was already on.
 *
 * @return true on success (already-on or just-enabled).
 */
bool TIREMO_FlashAB_EnableMultiBank(bool *pJustEnabled);

/**
 * @brief Unprotect the inactive bank, ready for streamed sector writes.
 *
 * @param[in] imageLen  Expected total image size, or 0 if not yet known;
 *                       used only to reject an image larger than a bank.
 *
 * @return true on success.
 */
bool TIREMO_FlashAB_OtaBegin(uint32_t imageLen);

/**
 * @brief Erase and program one 4 KB sector of the inactive bank.
 *
 * @param[in] sectorIndex  Sector number from the start of the bank.
 * @param[in] pSector       Exactly TIREMO_FLASHAB_SECTOR_SIZE bytes; a
 *                          short final sector is padded by the caller.
 *
 * @return true when the erase and the write both succeeded.
 */
bool TIREMO_FlashAB_OtaWriteSector(uint32_t sectorIndex,
                                   const uint8_t *pSector);

/**
 * @brief CRC-32 of the first @p length bytes of the inactive bank, read
 *        straight through its memory-mapped window.
 */
uint32_t TIREMO_FlashAB_Crc32Other(uint32_t length);

/**
 * @brief Point the next boot at @p bank (CFMC user info; needs a reset).
 *
 * @return true when the HAL accepted the request.
 */
bool TIREMO_FlashAB_SetNextBoot(TIREMO_FlashAB_Bank_t bank);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_FLASH_AB_H */
