/******************************************************************************
 * @file    tiremo_envlog.h
 * @brief   Flash ring buffer of environmental samples.
 *
 * Last TIREMO_ENVLOG_CAPACITY readings in data-flash sector 0, magic +
 * CRC32 like tutorial 06. Replaces any previous occupant of that sector
 * (including the tutorial-06 bootCount record).
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_ENVLOG_H
#define TIREMO_ENVLOG_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

#define TIREMO_ENVLOG_CAPACITY    (8U)
#define TIREMO_ENVLOG_MAGIC       (0x454E5631UL) /* "ENV1" */
#define TIREMO_ENVLOG_VERSION     (1U)

typedef struct
{
    uint32_t seq;
    uint32_t uptimeMs;
    int32_t temperatureMilliC;
    int32_t humidityMilliRh;
    int16_t accelX_mg;
    int16_t accelY_mg;
    int16_t accelZ_mg;
    uint16_t flags; /* bit0 SHT40 ok, bit1 accel ok, bit2 battery ok */
    uint32_t avddMv;
    int32_t micDb;
} TIREMO_EnvSample_t;

bool TIREMO_EnvLog_Init(void);
uint32_t TIREMO_EnvLog_Count(void);
bool TIREMO_EnvLog_GetOldest(uint32_t indexFromOldest,
                             TIREMO_EnvSample_t *pOut);
bool TIREMO_EnvLog_Append(TIREMO_EnvSample_t *pSample);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_ENVLOG_H */
