/**
 *******************************************************************************
 * @file        main.c
 * @author      ABOV R&D Division
 * @brief       Template User Application Main
 *
 * Copyright 2024 ABOV Semiconductor Co.,Ltd. All rights reserved.
 *
 * This file is licensed under terms that are found in the LICENSE file
 * located at Document directory.
 * If this file is delivered or shared without applicable license terms,
 * the terms of the BSD-3-Clause license shall be applied.
 * Reference: https://opensource.org/licenses/BSD-3-Clause
 ******************************************************************************/

#include "abov_config.h"
#include "abov_module_config.h"
 
#if (CONFIG_DEBUG == 1)
#include "debug_log.h"
#include "debug.h"
#endif
 
#ifndef True
#define True true
#endif
 
#ifndef False
#define False false
#endif
 
extern void PRV_SCU_CLK_Init(void);
extern void PRV_USER_Code(void);
 
/* >>> Placeholder for External Functions */
extern void PRV_UART_Init(void);
extern void PRV_ADC_Init(void);
extern void PRV_Timer1_Init(void);
extern void PRV_I2C_Init(void);
#if (CONFIG_DEBUG == 1)
void GetChipSetInfo(void)
{
    int8_t *pn8ChipInfo = NULL;
    int8_t *pn8ChipCoreInfo = NULL;
    pn8ChipInfo = PRV_CHIPSET_GetInfo();
    pn8ChipCoreInfo = PRV_CHIPSET_GetCoreInfo();
    LOG("\n************************************************\n\r");
    LOG("- MCU - %s \n",pn8ChipInfo);
    LOG("- Core: ARM %s  \n",pn8ChipCoreInfo);
    LOG("- Communicate via: %s%d - %dbps\n",CONFIG_DEBUG_MODULE_STR,DEBUG_UART_ID,APP_UART_BAUD);
    LOG("- ARM System Core Clock = %d\n",SystemCoreClock);
    LOG("- CLI : 'ls' show command list \n");
    LOG("************************************************\n\r");
}
#endif
/* <<< End of Placeholder */
 
/**********************************************************************
 * @brief       Main program
 * @param[in]   None
 * @return      0 : No error, Non-Zero : Any error
 **********************************************************************/
int main(void)
{
    /* Initialize Port */
    PRV_PORT_Init();
 
    /* Initialize Clock System */
    PRV_SCU_CLK_Init();
 
    /* Initialize Serial Debug */
#if (CONFIG_DEBUG == 1)
    Debug_Init();
    GetChipSetInfo();
#endif
 
/* >>> Placeholder for Initialization of Peripherals */
    PRV_UART_Init();
    PRV_ADC_Init();
    PRV_Timer1_Init();
    PRV_I2C_Init();
/* <<< End of Placeholder */
 
    /* User Code Here */
    PRV_USER_Code();
}
