/******************************************************************************
 * @file    tiremo_mic.h
 * @brief   Tiremo microphone driver API.
 *
 * Analog MEMS microphone captured via ADC burst + DMA, filtered and
 * analyzed into a NORMAL / HIGH_VOICE / SILENCE voice level state. Capture
 * is bounded: Start() then repeated Service() calls while polling for
 * progress (see TIREMO_MIC_GetDmaBlockCount()), then Stop() — there is no
 * persistent background service loop.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_MIC_H
#define TIREMO_MIC_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/******************************************************************************
 * Public Types
 *****************************************************************************/

typedef enum
{
    TIREMO_MIC_STATE_RESET = 0,
    TIREMO_MIC_STATE_READY,
    TIREMO_MIC_STATE_RECORDING,
    TIREMO_MIC_STATE_ERROR
} TIREMO_MIC_State_t;

typedef enum
{
    TIREMO_MIC_ALERT_NONE = 0,
    TIREMO_MIC_ALERT_NORMAL,
    TIREMO_MIC_ALERT_HIGH_VOICE,
    TIREMO_MIC_ALERT_SILENCE
} TIREMO_MIC_Alert_t;

/** @brief Result of one buffer's voice analysis. */
typedef struct
{
    TIREMO_MIC_Alert_t alert;
    uint32_t level;
} TIREMO_MIC_VoiceResult_t;

/******************************************************************************
 * Public Function Prototypes
 *****************************************************************************/

/**
 * @brief Initialize the microphone driver with default sample rate/buffer.
 *
 * Configures PA0 as an analog input and sets the Timer1 sample-rate divider.
 * Does not start capture.
 *
 * @return true when initialized successfully.
 *
 * @warning Call PRV_ADC_Init() and TIREMO_SysTick_Init() before this
 *          function.
 */
bool TIREMO_MIC_Init(void);

/**
 * @brief Deinitialize the microphone driver, stopping capture if active.
 */
void TIREMO_MIC_Deinit(void);

/**
 * @brief Check whether the microphone driver is initialized.
 */
bool TIREMO_MIC_IsInitialized(void);

/**
 * @brief Start burst capture.
 *
 * @return true on success.
 */
bool TIREMO_MIC_Start(void);

/**
 * @brief Stop capture; state returns to READY.
 */
void TIREMO_MIC_Stop(void);

/**
 * @brief Service pending capture data.
 *
 * @note Call frequently (e.g. in a bounded polling loop) while capturing —
 *       there is no interrupt- or task-driven background service.
 */
void TIREMO_MIC_Service(void);

/**
 * @brief Get the current driver state.
 */
TIREMO_MIC_State_t TIREMO_MIC_GetState(void);

/**
 * @brief Number of DMA capture blocks completed since the last Start().
 *
 * Useful as a bring-up "is data actually arriving" check.
 */
uint32_t TIREMO_MIC_GetDmaBlockCount(void);

/**
 * @brief Smoothed RMS voice level from the most recently processed buffer.
 */
uint32_t TIREMO_MIC_GetVoiceLevel(void);

/**
 * @brief Set digital gain in dB applied in the capture filter path.
 *
 * @param[in] gainDb  Typically -12..+24; 0 leaves the default path gain.
 */
void TIREMO_MIC_SetGainDb(int32_t gainDb);

/**
 * @brief Convert an RMS level to approximate dB (20*log10(rms)).
 *
 * @return dB value in 0..120 (0 when rms is 0).
 */
int32_t TIREMO_MIC_RmsToDb(uint32_t rms);

/**
 * @brief Smoothed voice level expressed in dB.
 */
int32_t TIREMO_MIC_GetVoiceLevelDb(void);

/**
 * @brief Called when the voice state changes (HIGH_VOICE / NORMAL / SILENCE).
 *
 * @details Weak stub — override elsewhere (e.g. app-layer alerting) for
 *          UART, LED, or buzzer notification.
 *
 * @param[in] result  .alert != TIREMO_MIC_ALERT_NONE; .level = smoothed RMS.
 */
void TIREMO_MIC_OnVoiceAlert(TIREMO_MIC_VoiceResult_t result);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_MIC_H */
