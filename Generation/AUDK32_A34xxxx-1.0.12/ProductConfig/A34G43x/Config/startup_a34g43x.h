/**
 *******************************************************************************
 * @file        startup_a34g43x.h
 * @author      ABOV R&D Division
 * @brief       Startup APIs for A34L71x
 *
 * Copyright 2022 ABOV Semiconductor Co.,Ltd. All rights reserved.
 *
 * This file is licensed under terms that are found in the LICENSE file
 * located at Document directory.
 * If this file is delivered or shared without applicable license terms,
 * the terms of the BSD-3-Clause license shall be applied.
 * Reference: https://opensource.org/licenses/BSD-3-Clause
 ******************************************************************************/

#ifndef _STARTUP_A34G43X_H_
#define _STARTUP_A34G43X_H_

/* place a function into a ram section macro */
#if defined ( NO_RAMFUNCS )
#define RAMFUNC

#elif defined (__ICCARM__)
#define RAMFUNC                                       __ramfunc
#ifndef __inline
#define __inline                                      inline
#endif

#elif defined (__CC_ARM) || (__ARMCC_VERSION)
#define RAMFUNC                                       __attribute__ ((section ("code_section_on_ram")))

#elif defined (__GNUC__)
#define RAMFUNC                                       __attribute__ ((long_call, section (".code_section_on_ram")))

#endif

/* The clock speed of Internal Low Speed Oscillator */
#define LSI_CLOCK   (500000)

/* The clock speed of External Low Speed Oscillator (SXTAL) */
#define LSE_CLOCK   (32768)

/* The clock speed of Internal High Speed Oscillator */
#define HSI_CLOCK   (16000000)

/* The clock speed of External High Speed Oscillator(XTAL) */
#define HSE_CLOCK   (4000000)

extern void SystemClock_Config(void);

static __inline int8_t *PRV_CHIPSET_GetCoreInfo(void)
{
    if(((CHIPCONFIG->CHIPID & 0x00FF0000) >> 16) == 0x34)
    {
        return (int8_t *)"Cortex-M4";
    }
    else
    {
        return (int8_t *)"Unknown";
    }
}

static __inline int8_t *PRV_CHIPSET_GetInfo(void)
{
    if(CHIPCONFIG->CHIPID == 0x473444A0)
    {
        return (int8_t *)"A34G43A : Flash 2048KB / SRAM 256KB";
    }
    else if(CHIPCONFIG->CHIPID == 0x473444A1)
    {
        return (int8_t *)"A34G430 : Flash 1024KB / SRAM 256KB";
    }
    else
    {
        return (int8_t *)"Unknown";
    }
}

/* Port flash geometry here */
#define STARTUP_FLASH_BASE_ADDR             0x00000000
#define STARTUP_CODE_FLASH_BASE_OFFSET      0x00000000
#define STARTUP_DATA_FLASH_BASE_OFFSET      0x0E000000
#define STARTUP_SYSTEM_FLASH_BASE_OFFSET    0x0F000000
#define STARTUP_SYSTEM_FLASH_OFFSET_MASK    0x00FFFFFF
#define STARTUP_PAGE_SIZE                   0x200
#define STARTUP_DATA_PAGE_SIZE              0x200

#define CONFIG_MAX_WPROT_BLOCKS             3

typedef struct
{
    uint32_t    un32WProtectedSegStartOffset;
    uint32_t    un32WProtectedSegSize;
    uint8_t     un8NumWProtectedSegs;
} CONFIG_WPROT_BLOCKS_t;

typedef struct
{
    uint32_t    un32Size;
    uint32_t    un32BaseAddr;
    uint8_t     un8NumWProtectedBlocks;
    CONFIG_WPROT_BLOCKS_t tWProtectedBlocks[CONFIG_MAX_WPROT_BLOCKS];
} CONFIG_FLASH_GEOMETRY_t;

/* General purpose system flash area only */
typedef struct
{
    uint32_t    un32Size;
    uint32_t    un32BaseAddr;
    uint32_t    un32SysSegSize;
    uint8_t     un8NumSysSegs;
} CONFIG_SYSFLASH_GEOMETRY_t;

typedef struct
{
    CONFIG_FLASH_GEOMETRY_t    tCflash;
    CONFIG_FLASH_GEOMETRY_t    tDflash;
    CONFIG_SYSFLASH_GEOMETRY_t tSysflash;
} CONFIG_FLASH_INFO_t;

static __inline CONFIG_FLASH_INFO_t PRV_CHIPSET_GetFlashGeometricInfo(void);

/* Be careful:Number of Blocks should not exeed to CONFIG_MAX_WPROT_BLOCKS */
RAMFUNC static __inline CONFIG_FLASH_INFO_t PRV_CHIPSET_GetFlashGeometricInfo(void)
{
    CONFIG_FLASH_INFO_t tFlashGeometricInfo;

    tFlashGeometricInfo.tCflash.un32BaseAddr = STARTUP_CODE_FLASH_BASE_OFFSET | STARTUP_FLASH_BASE_ADDR;
    tFlashGeometricInfo.tCflash.un8NumWProtectedBlocks = 1;
    tFlashGeometricInfo.tCflash.tWProtectedBlocks[0].un32WProtectedSegStartOffset = 0;
    tFlashGeometricInfo.tCflash.tWProtectedBlocks[0].un32WProtectedSegSize = 0x8000;

    tFlashGeometricInfo.tDflash.un32BaseAddr = STARTUP_DATA_FLASH_BASE_OFFSET | STARTUP_FLASH_BASE_ADDR;
    tFlashGeometricInfo.tDflash.un32Size = 0x8000;
    tFlashGeometricInfo.tDflash.un8NumWProtectedBlocks = 1;
    tFlashGeometricInfo.tDflash.tWProtectedBlocks[0].un32WProtectedSegStartOffset = 0;
    tFlashGeometricInfo.tDflash.tWProtectedBlocks[0].un32WProtectedSegSize = 0x800;
    tFlashGeometricInfo.tDflash.tWProtectedBlocks[0].un8NumWProtectedSegs = 16;

    /* If FMC supports data flash, System flash should not be accessed by user */
    tFlashGeometricInfo.tSysflash.un32BaseAddr = STARTUP_SYSTEM_FLASH_BASE_OFFSET | STARTUP_FLASH_BASE_ADDR;
    tFlashGeometricInfo.tSysflash.un32Size = 0;
    tFlashGeometricInfo.tSysflash.un32SysSegSize = 0;
    tFlashGeometricInfo.tSysflash.un8NumSysSegs = 0;

    switch (CHIPCONFIG->CHIPID)
    {
        case 0x473444A0: /* A34G43A */
            tFlashGeometricInfo.tCflash.un32Size = 0x200000;
            tFlashGeometricInfo.tCflash.tWProtectedBlocks[0].un8NumWProtectedSegs = 16;
            tFlashGeometricInfo.tCflash.tWProtectedBlocks[0].un32WProtectedSegSize = 0x10000;
            break;
        case 0x473444A1: /* A34G430 */
            tFlashGeometricInfo.tCflash.un32Size = 0x100000;
            tFlashGeometricInfo.tCflash.tWProtectedBlocks[0].un8NumWProtectedSegs = 16;
            break;
        default:
            tFlashGeometricInfo.tCflash.un32Size = 0;
            tFlashGeometricInfo.tDflash.un32Size = 0;
            tFlashGeometricInfo.tSysflash.un32Size = 0;
            break;
    }
    return tFlashGeometricInfo;
}

static __inline void PRV_CHIPSET_Init(void)
{

    /* WDT Access Enable */
    WDT->AEN = 0xA55A;
    /* WDT Disable */
    WDT->CON = 0;


    /* SCU Access Enable */
    SCU->SYSTEN = 0x57;
    SCU->SYSTEN = 0x75;

    /* GPIO Access Enable */
    PORTEN->EN = 0x15;
    PORTEN->EN = 0x51;
}

static __inline void PRV_PORT_Init(void)
{
    /* Peripheral Enable Register 1  0:Disable 1:Enable */
    SCU->PER1 |= 0x00UL
            | (0x01UL << SCU_PER1_GPIOG_Pos)    /* GPIO G */
            | (0x01UL << SCU_PER1_GPIOF_Pos)    /* GPIO F */
            | (0x01UL << SCU_PER1_GPIOE_Pos)    /* GPIO E */
            | (0x01UL << SCU_PER1_GPIOD_Pos)    /* GPIO D */
            | (0x01UL << SCU_PER1_GPIOC_Pos)    /* GPIO C */
            | (0x01UL << SCU_PER1_GPIOB_Pos)    /* GPIO B */
            | (0x01UL << SCU_PER1_GPIOA_Pos);   /* GPIO A */

    /* Peripheral Clock Enable Register 1 0:Disable 1:Enable */
    SCU->PCER1 |= 0x00UL
            | (0x01UL << SCU_PCER1_GPIOG_Pos)   /* GPIO G */
            | (0x01UL << SCU_PCER1_GPIOF_Pos)   /* GPIO F */
            | (0x01UL << SCU_PCER1_GPIOE_Pos)   /* GPIO E */
            | (0x01UL << SCU_PCER1_GPIOD_Pos)   /* GPIO D */
            | (0x01UL << SCU_PCER1_GPIOC_Pos)   /* GPIO C */
            | (0x01UL << SCU_PCER1_GPIOB_Pos)   /* GPIO B */
            | (0x01UL << SCU_PCER1_GPIOA_Pos);  /* GPIO A */

    /* Debouncing port clock setting */
    SCU->MCCR4 |= 0x00UL
            | (0x0 << SCU_MCCR4_PGADCDIV_Pos)
            | (0x4 << SCU_MCCR4_PGADCSEL_Pos);

    SCU->MCCR5 |= 0x00UL
            | (0xa << SCU_MCCR5_PGBDCDIV_Pos)
            | (0x4 << SCU_MCCR5_PGBDCSEL_Pos)
            | (0x0 << SCU_MCCR5_PGCDCDIV_Pos)
            | (0x4 << SCU_MCCR5_PGCDCSEL_Pos);

    /* enable writing permittion of ALL PCU register */
    PORTEN->EN = 0x15;
    PORTEN->EN = 0x51;

    /*--------------------------------------------------------------
     *    PORT INIT
     *        PA PB PC PD PE PF PG
     *-------------------------------------------------------------*/
    /* PORT - A */
    PA->DER = 0x0;
    PA->ODR = 0x0;
    PA->BSR = 0x0;
    PA->BCR = 0x0;
    PA->STR = 0x0;
    PA->MR1 = 0xff88ffff;
    PA->MR2 = 0xfff89999;
    PA->CR = 0xfcffffff;
    PA->PRCR = 0x0;

    /* PORT - B */
    PB->DER = 0x0;
    PB->ODR = 0xfe30;
    PB->BSR = 0xfe20;
    PB->BCR = 0x0;
    PB->STR = 0x0;
    PB->MR1 = 0x9988ffff;
    PB->MR2 = 0x8888888f;
    PB->CR = 0x3f0ff;
    PB->PRCR = 0x0;

    /* PORT - C */
    PC->DER = 0x200;
    PC->ODR = 0x0;
    PC->BSR = 0x0;
    PC->BCR = 0x0;
    PC->STR = 0x0;
    PC->MR1 = 0xfffbbbbb;
    PC->MR2 = 0xbbffbb8f;
    PC->CR = 0xffffffff;
    PC->PRCR = 0xa0028a;

    /* PORT - D */
    PD->DER = 0x0;
    PD->ODR = 0x0;
    PD->BSR = 0x0;
    PD->BCR = 0x0;
    PD->STR = 0x0;
    PD->MR1 = 0xffffffff;
    PD->MR2 = 0xffffffff;
    PD->CR = 0xffffffff;
    PD->PRCR = 0x0;

    /* PORT - E */
    PE->DER = 0x0;
    PE->ODR = 0x0;
    PE->BSR = 0x0;
    PE->BCR = 0x0;
    PE->STR = 0x0;
    PE->MR1 = 0xffffffff;
    PE->MR2 = 0xffffffff;
    PE->CR = 0xffffffff;
    PE->PRCR = 0x0;

    /* PORT - F */
    PF->DER = 0x0;
    PF->ODR = 0x80;
    PF->BSR = 0x80;
    PF->BCR = 0x0;
    PF->STR = 0x0;
    PF->MR1 = 0x8fffffff;
    PF->MR2 = 0xffffffff;
    PF->CR = 0xffff3fff;
    PF->PRCR = 0x0;

    /* PORT - G */
    PG->DER = 0x0;
    PG->ODR = 0x0;
    PG->BSR = 0x0;
    PG->BCR = 0x0;
    PG->STR = 0x0;
    PG->MR1 = 0xffffffff;
    PG->MR2 = 0xffffffff;
    PG->CR = 0xffffffff;
    PG->PRCR = 0x0;

    /* disable writing permittion of ALL PCU register */
    PORTEN->EN = 0x00;
}

#endif /* _STARTUP_A34G43X_H_ */
