/******************************************************************************
 * @file    tiremo_flash_ab.c
 * @brief   Code-flash A/B bank access for OTA (ABOV CFMC multi-bank).
 *
 * The bank-swap sequence here is the same one the vendor cfmc_dual_bank
 * example uses -- unprotect, 4 KB-sector erase, program, then set the
 * boot bank in CFMC user info -- narrowed to the parts an OTA needs and
 * wrapped in a small API.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "../Include/tiremo_flash_ab.h"
#include "../Include/tiremo_crc32.h"
#include "../Include/tiremo_debug.h"

#include "abov_config.h"
#include "hal_fmc.h"

#define FLASHAB_SECTOR_SIZE   TIREMO_FLASHAB_SECTOR_SIZE

static bool s_ready = false;
static uint32_t s_bankSize = 0U;

bool TIREMO_FlashAB_Init(void)
{
    FMC_GEOMETRY_t geo;

    if (s_ready)
    {
        return true;
    }
    if (HAL_FMC_Init() != HAL_ERR_OK)
    {
        return false;
    }

    geo = HAL_FMC_GetGeometry();
    s_bankSize = geo.tCflash.un32Size >> 1;
    if (s_bankSize == 0U)
    {
        return false;
    }

    s_ready = true;
    return true;
}

uint32_t TIREMO_FlashAB_BankSize(void)
{
    return s_bankSize;
}

TIREMO_FlashAB_Bank_t TIREMO_FlashAB_ActiveBank(void)
{
    CFMC_BANK_ID_e eBoot;
    CFMC_BANK_ID_e eActive;
    bool bDone;

    if (HAL_CFMC_GetBankSwitchStatus(&eBoot, &bDone, &eActive) != HAL_ERR_OK)
    {
        return TIREMO_FLASHAB_BANK_UNKNOWN;
    }

    switch (eActive)
    {
        case CFMC_BANK_A:      return TIREMO_FLASHAB_BANK_A;
        case CFMC_BANK_B:      return TIREMO_FLASHAB_BANK_B;
        case CFMC_BANK_SINGLE: return TIREMO_FLASHAB_BANK_SINGLE;
        default:              return TIREMO_FLASHAB_BANK_UNKNOWN;
    }
}

TIREMO_FlashAB_Bank_t TIREMO_FlashAB_OtherBank(TIREMO_FlashAB_Bank_t bank)
{
    if (bank == TIREMO_FLASHAB_BANK_A)
    {
        return TIREMO_FLASHAB_BANK_B;
    }
    if (bank == TIREMO_FLASHAB_BANK_B)
    {
        return TIREMO_FLASHAB_BANK_A;
    }
    return TIREMO_FLASHAB_BANK_UNKNOWN;
}

bool TIREMO_FlashAB_EnableMultiBank(bool *pJustEnabled)
{
    if (pJustEnabled != NULL)
    {
        *pJustEnabled = false;
    }
    if (!s_ready)
    {
        return false;
    }

    if (TIREMO_FlashAB_ActiveBank() != TIREMO_FLASHAB_BANK_SINGLE)
    {
        return true;
    }

    if (HAL_CFMC_SetMultiBankConfig(true) != HAL_ERR_OK)
    {
        TIREMO_LOG_ERROR("flash_ab: enable multi-bank failed");
        return false;
    }
    if (pJustEnabled != NULL)
    {
        *pJustEnabled = true;
    }
    TIREMO_LOG_INFO("flash_ab: multi-bank enabled (reset required)");
    return true;
}

bool TIREMO_FlashAB_OtaBegin(uint32_t imageLen)
{
    TIREMO_FlashAB_Bank_t eNext;
    uint32_t un32Protect;
    uint32_t un32EraseLen;
    uint32_t off;

    if (!s_ready)
    {
        return false;
    }
    if ((imageLen != 0U) && (imageLen > s_bankSize))
    {
        TIREMO_LOG_ERROR("flash_ab: image %lu > bank %lu",
                         (unsigned long)imageLen, (unsigned long)s_bankSize);
        return false;
    }

    eNext = TIREMO_FlashAB_OtherBank(TIREMO_FlashAB_ActiveBank());
    un32Protect = (eNext == TIREMO_FLASHAB_BANK_A) ? 0U : s_bankSize;

    if (HAL_FMC_SetWriteProtect(FMC_ID_CFMC, un32Protect, s_bankSize, false) !=
        HAL_ERR_OK)
    {
        TIREMO_LOG_ERROR("flash_ab: unprotect failed");
        return false;
    }

    /* Erase the whole target region here, in one burst before any bytes
     * are streaming in -- so the per-sector work during the download is
     * just a (shorter) program, and the UART RX ring is less likely to
     * back up while flash is busy. Unknown size -> erase the full bank. */
    un32EraseLen = (imageLen != 0U)
                       ? ((imageLen + (FLASHAB_SECTOR_SIZE - 1U)) &
                          ~(FLASHAB_SECTOR_SIZE - 1U))
                       : s_bankSize;
    for (off = 0U; off < un32EraseLen; off += FLASHAB_SECTOR_SIZE)
    {
        if (HAL_FMC_Erase(FMC_ID_CFMC, s_bankSize + off,
                          FMC_4KB_ERASE_MODE) != HAL_ERR_OK)
        {
            TIREMO_LOG_ERROR("flash_ab: erase failed at +0x%lX",
                             (unsigned long)off);
            return false;
        }
    }
    TIREMO_LOG_INFO("flash_ab: inactive bank erased (%lu KB)",
                    (unsigned long)(un32EraseLen / 1024U));
    return true;
}

bool TIREMO_FlashAB_OtaWriteSector(uint32_t sectorIndex,
                                   const uint8_t *pSector)
{
    uint32_t off;

    if ((!s_ready) || (pSector == NULL))
    {
        return false;
    }
    off = sectorIndex * FLASHAB_SECTOR_SIZE;
    if ((off + FLASHAB_SECTOR_SIZE) > s_bankSize)
    {
        TIREMO_LOG_ERROR("flash_ab: sector %lu past end of bank",
                         (unsigned long)sectorIndex);
        return false;
    }

    /* Pre-erased in OtaBegin() -- program only. */
    if (HAL_FMC_Write(FMC_ID_CFMC, s_bankSize + off, (uint8_t *)pSector,
                      FLASHAB_SECTOR_SIZE) != HAL_ERR_OK)
    {
        TIREMO_LOG_ERROR("flash_ab: write failed at +0x%lX",
                         (unsigned long)off);
        return false;
    }
    return true;
}

uint32_t TIREMO_FlashAB_Crc32Other(uint32_t length)
{
    if ((!s_ready) || (length == 0U) || (length > s_bankSize))
    {
        return 0U;
    }
    return TIREMO_Crc32((const void *)(uintptr_t)s_bankSize, length);
}

bool TIREMO_FlashAB_SetNextBoot(TIREMO_FlashAB_Bank_t bank)
{
    CFMC_BANK_ID_e eBank;

    if (bank == TIREMO_FLASHAB_BANK_A)
    {
        eBank = CFMC_BANK_A;
    }
    else if (bank == TIREMO_FLASHAB_BANK_B)
    {
        eBank = CFMC_BANK_B;
    }
    else
    {
        return false;
    }

    if (HAL_CFMC_SetBootBankOnUserInfo(eBank) != HAL_ERR_OK)
    {
        TIREMO_LOG_ERROR("flash_ab: set boot bank failed");
        return false;
    }

    /* Read the CFMC bank-switch state back so the log shows what the boot
     * ROM will actually see on the next reset. */
    {
        CFMC_BANK_ID_e eBoot = CFMC_BANK_SINGLE;
        CFMC_BANK_ID_e eActive = CFMC_BANK_SINGLE;
        bool bDone = false;

        if (HAL_CFMC_GetBankSwitchStatus(&eBoot, &bDone, &eActive) ==
            HAL_ERR_OK)
        {
            TIREMO_LOG_INFO("flash_ab: user-info boot=%d active=%d done=%d "
                            "(want boot=%d)",
                            (int)eBoot, (int)eActive, (int)bDone, (int)eBank);
        }
    }
    return true;
}
