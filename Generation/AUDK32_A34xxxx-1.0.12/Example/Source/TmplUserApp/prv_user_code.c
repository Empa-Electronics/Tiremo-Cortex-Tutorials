/**
 *******************************************************************************
 * @file        prv_user_code.c
 * @author      ABOV R&D Division
 * @brief       Template User Application glue into the Tiremo_App layer
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

#include "tiremo_app.h"

/**********************************************************************
 * @brief       User Code Here
 * @param[in]   None
 * @return      None
 *
 * @note        This vendor-generated file is only a call-through into the
 *              Tiremo_App boot flow (Tiremo_Lib/Tiremo_Core/Tiremo_App).
 *              Application logic must not be added here.
 **********************************************************************/
void PRV_USER_Code(void)
{
    TIREMO_APP_ProbeAndInitialize();
    TIREMO_APP_Run();
}
