/**
 *******************************************************************************
 * @file        startup_a34xxxx.c
 * @author      ABOV R&D Division
 * @brief       CMSIS-Core(M) Device Startup File for a Cortex-M4F Device
 *
 * Copyright 2022 ABOV Semiconductor Co.,Ltd. All rights reserved.
 *
 * This file is licensed under terms that are found in the LICENSE file
 * located at Document directory.
 * If this file is delivered or shared without applicable license terms,
 * the terms of the BSD-3-Clause license shall be applied.
 * Reference: https://opensource.org/licenses/BSD-3-Clause
 ******************************************************************************/

#include "abov_config.h"

/*----------------------------------------------------------------------------
  Internal Macros
 *----------------------------------------------------------------------------*/
#if (CONFIG_EMUL_JTAG_CONNECTION == 1)
#define PRV_CHIPSET_REG_DHCSR  0xE000EDF0
#endif

/*----------------------------------------------------------------------------
  External References
 *----------------------------------------------------------------------------*/
extern uint32_t __INITIAL_SP;

#if (CONFIG_EMUL_JTAG_CONNECTION == 1)
int8_t g_bIsARMisDebugMode = 0;
int8_t g_bIsARMisDebugConnectionChecked = 0;
#endif

extern __NO_RETURN void __PROGRAM_START(void);

/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/
__NO_RETURN void Reset_Handler  (void);
            void Default_Handler(void);

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
/* Exceptions */
void NMI_Handler            (void) __attribute__ ((weak, alias("Default_Handler")));
void HardFault_Handler      (void) __attribute__ ((weak));
void MemManage_Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void BusFault_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void UsageFault_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void SVC_Handler            (void) __attribute__ ((weak, alias("Default_Handler")));
void DebugMon_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler         (void) __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));

void Interrupt0_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt1_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt2_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt3_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt4_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt5_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt6_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt7_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt8_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt9_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));

void Interrupt10_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt11_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt12_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt13_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt14_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt15_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt16_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt17_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt18_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt19_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));

void Interrupt20_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt21_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt22_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt23_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt24_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt25_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt26_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt27_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt28_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt29_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));

void Interrupt30_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt31_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt32_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt33_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt34_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt35_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt36_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt37_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt38_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt39_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));

void Interrupt40_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt41_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt42_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt43_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt44_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt45_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt46_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt47_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt48_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt49_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));

void Interrupt50_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt51_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt52_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt53_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt54_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt55_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt56_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt57_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt58_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt59_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));

void Interrupt60_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt61_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt62_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt63_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt64_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt65_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt66_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt67_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt68_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt69_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));

void Interrupt70_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt71_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt72_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt73_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt74_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt75_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt76_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt77_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt78_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt79_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));

void Interrupt80_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt81_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt82_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt83_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt84_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt85_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt86_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt87_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt88_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt89_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));

void Interrupt90_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt91_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt92_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt93_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt94_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt95_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt96_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt97_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt98_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt99_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));

void Interrupt100_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt101_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt102_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt103_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt104_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt105_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt106_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt107_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt108_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt109_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt110_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt111_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt112_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt113_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt114_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt115_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt116_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt117_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt118_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt119_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt120_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt121_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt122_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt123_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void Interrupt124_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));

/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/

#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

extern const VECTOR_TABLE_Type __VECTOR_TABLE[240];
       const VECTOR_TABLE_Type __VECTOR_TABLE[240] __VECTOR_TABLE_ATTRIBUTE =
{
  (VECTOR_TABLE_Type)(&__INITIAL_SP),       /*     Initial Stack Pointer */
  Reset_Handler,                            /*     Reset Handler */
  NMI_Handler,                              /* -14 NMI Handler */
  HardFault_Handler,                        /* -13 Hard Fault Handler */
  MemManage_Handler,                        /* -12 MPU Fault Handler */
  BusFault_Handler,                         /* -11 Bus Fault Handler */
  UsageFault_Handler,                       /* -10 Usage Fault Handler */
  0,                                        /*     Reserved */
  0,                                        /*     Reserved */
  0,                                        /*     Reserved */
  0,                                        /*     Reserved */
  SVC_Handler,                              /*  -5 SVC Handler */
  DebugMon_Handler,                         /*  -4 Debug Monitor Handler */
  0,                                        /*     Reserved */
  PendSV_Handler,                           /*  -2 PendSV Handler */
  SysTick_Handler,                          /*  -1 SysTick Handler */

  /* Interrupts */
  Interrupt0_Handler,                       /*   Interrupt 0 */
  Interrupt1_Handler,                       /*   Interrupt 1 */
  Interrupt2_Handler,                       /*   Interrupt 2 */
  Interrupt3_Handler,                       /*   Interrupt 3 */
  Interrupt4_Handler,                       /*   Interrupt 4 */
  Interrupt5_Handler,                       /*   Interrupt 5 */
  Interrupt6_Handler,                       /*   Interrupt 6 */
  Interrupt7_Handler,                       /*   Interrupt 7 */
  Interrupt8_Handler,                       /*   Interrupt 8 */
  Interrupt9_Handler,                       /*   Interrupt 9 */
  Interrupt10_Handler,                      /*   Interrupt 10 */
  Interrupt11_Handler,                      /*   Interrupt 11 */
  Interrupt12_Handler,                      /*   Interrupt 12 */
  Interrupt13_Handler,                      /*   Interrupt 13 */
  Interrupt14_Handler,                      /*   Interrupt 14 */
  Interrupt15_Handler,                      /*   Interrupt 15 */
  Interrupt16_Handler,                      /*   Interrupt 16 */
  Interrupt17_Handler,                      /*   Interrupt 17 */
  Interrupt18_Handler,                      /*   Interrupt 18 */
  Interrupt19_Handler,                      /*   Interrupt 19 */
  Interrupt20_Handler,                      /*   Interrupt 20 */
  Interrupt21_Handler,                      /*   Interrupt 21 */
  Interrupt22_Handler,                      /*   Interrupt 22 */
  Interrupt23_Handler,                      /*   Interrupt 23 */
  Interrupt24_Handler,                      /*   Interrupt 24 */
  Interrupt25_Handler,                      /*   Interrupt 25 */
  Interrupt26_Handler,                      /*   Interrupt 26 */
  Interrupt27_Handler,                      /*   Interrupt 27 */
  Interrupt28_Handler,                      /*   Interrupt 28 */
  Interrupt29_Handler,                      /*   Interrupt 29 */
  Interrupt30_Handler,                      /*   Interrupt 30 */
  Interrupt31_Handler,                      /*   Interrupt 31 */
  Interrupt32_Handler,                      /*   Interrupt 32 */
  Interrupt33_Handler,                      /*   Interrupt 33 */
  Interrupt34_Handler,                      /*   Interrupt 34 */
  Interrupt35_Handler,                      /*   Interrupt 35 */
  Interrupt36_Handler,                      /*   Interrupt 36 */
  Interrupt37_Handler,                      /*   Interrupt 37 */
  Interrupt38_Handler,                      /*   Interrupt 38 */
  Interrupt39_Handler,                      /*   Interrupt 39 */
  Interrupt40_Handler,                      /*   Interrupt 40 */
  Interrupt41_Handler,                      /*   Interrupt 41 */
  Interrupt42_Handler,                      /*   Interrupt 42 */
  Interrupt43_Handler,                      /*   Interrupt 43 */
  Interrupt44_Handler,                      /*   Interrupt 44 */
  Interrupt45_Handler,                      /*   Interrupt 45 */
  Interrupt46_Handler,                      /*   Interrupt 46 */
  Interrupt47_Handler,                      /*   Interrupt 47 */
  Interrupt48_Handler,                      /*   Interrupt 48 */
  Interrupt49_Handler,                      /*   Interrupt 49 */
  Interrupt50_Handler,                      /*   Interrupt 50 */
  Interrupt51_Handler,                      /*   Interrupt 51 */
  Interrupt52_Handler,                      /*   Interrupt 52 */
  Interrupt53_Handler,                      /*   Interrupt 53 */
  Interrupt54_Handler,                      /*   Interrupt 54 */
  Interrupt55_Handler,                      /*   Interrupt 55 */
  Interrupt56_Handler,                      /*   Interrupt 56 */
  Interrupt57_Handler,                      /*   Interrupt 57 */
  Interrupt58_Handler,                      /*   Interrupt 58 */
  Interrupt59_Handler,                      /*   Interrupt 59 */
  Interrupt60_Handler,                      /*   Interrupt 60 */
  Interrupt61_Handler,                      /*   Interrupt 61 */
  Interrupt62_Handler,                      /*   Interrupt 62 */
  Interrupt63_Handler,                      /*   Interrupt 63 */
  Interrupt64_Handler,                      /*   Interrupt 64 */
  Interrupt65_Handler,                      /*   Interrupt 65 */
  Interrupt66_Handler,                      /*   Interrupt 66 */
  Interrupt67_Handler,                      /*   Interrupt 67 */
  Interrupt68_Handler,                      /*   Interrupt 68 */
  Interrupt69_Handler,                      /*   Interrupt 69 */
  Interrupt70_Handler,                      /*   Interrupt 70 */
  Interrupt71_Handler,                      /*   Interrupt 71 */
  Interrupt72_Handler,                      /*   Interrupt 72 */
  Interrupt73_Handler,                      /*   Interrupt 73 */
  Interrupt74_Handler,                      /*   Interrupt 74 */
  Interrupt75_Handler,                      /*   Interrupt 75 */
  Interrupt76_Handler,                      /*   Interrupt 76 */
  Interrupt77_Handler,                      /*   Interrupt 77 */
  Interrupt78_Handler,                      /*   Interrupt 78 */
  Interrupt79_Handler,                      /*   Interrupt 79 */
  Interrupt80_Handler,                      /*   Interrupt 80 */
  Interrupt81_Handler,                      /*   Interrupt 81 */
  Interrupt82_Handler,                      /*   Interrupt 82 */
  Interrupt83_Handler,                      /*   Interrupt 83 */
  Interrupt84_Handler,                      /*   Interrupt 84 */
  Interrupt85_Handler,                      /*   Interrupt 85 */
  Interrupt86_Handler,                      /*   Interrupt 86 */
  Interrupt87_Handler,                      /*   Interrupt 87 */
  Interrupt88_Handler,                      /*   Interrupt 88 */
  Interrupt89_Handler,                      /*   Interrupt 89 */
  Interrupt90_Handler,                      /*   Interrupt 90 */
  Interrupt91_Handler,                      /*   Interrupt 91 */
  Interrupt92_Handler,                      /*   Interrupt 92 */
  Interrupt93_Handler,                      /*   Interrupt 93 */
  Interrupt94_Handler,                      /*   Interrupt 94 */
  Interrupt95_Handler,                      /*   Interrupt 95 */
  Interrupt96_Handler,                      /*   Interrupt 96 */
  Interrupt97_Handler,                      /*   Interrupt 97 */
  Interrupt98_Handler,                      /*   Interrupt 98 */
  Interrupt99_Handler,                      /*   Interrupt 99 */
  Interrupt100_Handler,                     /*  Interrupt 100 */
  Interrupt101_Handler,                     /*  Interrupt 101 */
  Interrupt102_Handler,                     /*  Interrupt 102 */
  Interrupt103_Handler,                     /*  Interrupt 103 */
  Interrupt104_Handler,                     /*  Interrupt 104 */
  Interrupt105_Handler,                     /*  Interrupt 105 */
  Interrupt106_Handler,                     /*  Interrupt 106 */
  Interrupt107_Handler,                     /*  Interrupt 107 */
  Interrupt108_Handler,                     /*  Interrupt 108 */
  Interrupt109_Handler,                     /*  Interrupt 109 */
  Interrupt110_Handler,                     /*  Interrupt 110 */
  Interrupt111_Handler,                     /*  Interrupt 111 */
  Interrupt112_Handler,                     /*  Interrupt 112 */
  Interrupt113_Handler,                     /*  Interrupt 113 */
  Interrupt114_Handler,                     /*  Interrupt 114 */
  Interrupt115_Handler,                     /*  Interrupt 115 */
  Interrupt116_Handler,                     /*  Interrupt 116 */
  Interrupt117_Handler,                     /*  Interrupt 117 */
  Interrupt118_Handler,                     /*  Interrupt 118 */
  Interrupt119_Handler,                     /*  Interrupt 119 */
  Interrupt120_Handler,                     /*  Interrupt 120 */
  Interrupt121_Handler,                     /*  Interrupt 121 */
  Interrupt122_Handler,                     /*  Interrupt 122 */
  Interrupt123_Handler,                     /*  Interrupt 123 */
  Interrupt124_Handler,                     /*  Interrupt 124 */
                                            /* Interrupts 125 .. 223 are left out */
};

#if defined ( __GNUC__ )
#if 0
#pragma GCC diagnostic pop
#endif
#endif

/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
__NO_RETURN void Reset_Handler(void)
{
  SystemInit();                             /* CMSIS System Initialization */
  __PROGRAM_START();                        /* Enter PreMain (C library entry point) */
}


#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#if 0
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wmissing-noreturn"
#endif
#endif

/*----------------------------------------------------------------------------
  Hard Fault Handler
 *----------------------------------------------------------------------------*/
void HardFault_Handler(void)
{
#if (CONFIG_EMUL_JTAG_CONNECTION == 1)
    int* pn32Reg = (int*)PRV_CHIPSET_REG_DHCSR;

    /* Get Debugger connection */
    if (g_bIsARMisDebugConnectionChecked)
    {
       if (*(pn32Reg) & 0x01)
       {
          g_bIsARMisDebugMode = 1;
       }

       g_bIsARMisDebugConnectionChecked = 0;
       return;
    }
#endif

  while(1);
}

/*----------------------------------------------------------------------------
  Default Handler for Exceptions / Interrupts
 *----------------------------------------------------------------------------*/
void Default_Handler(void)
{
  while(1);
}

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#if 0
  #pragma clang diagnostic pop
#endif
#endif

/*----------------------------------------------------------------------------
  Private API to detect debugger connection
 *----------------------------------------------------------------------------*/
#define PRV_CHIPSET_INVALID_ADD   0x99999999UL
#define PRV_CHIPSET_INVALID_DATA  0x50

#if (CONFIG_EMUL_JTAG_CONNECTION == 1)
void PRV_CHIPSET_SetDebuggerConnectionStatus (void)
{
    uint8_t *un8pAddr = (uint8_t *)PRV_CHIPSET_INVALID_ADD;

    g_bIsARMisDebugConnectionChecked = 1;

    /* Fire Hard-Fault */
    *un8pAddr = PRV_CHIPSET_INVALID_DATA;
    return;
}

int8_t PRV_CHIPSET_GetDebuggerConnectionStatus (void)
{
    return g_bIsARMisDebugMode;
}
#endif

