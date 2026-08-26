/******************************************************************************
 * @file    tiremo_mic_app.c
 * @brief   Tiremo microphone driver implementation — voice detection layer.
 *
 * Data flow: BSP DMA block -> HandleDmaBlock() -> per-sample DC-offset +
 * IIR high-pass filter -> circular buffer -> on buffer full, ProcessVoice()
 * (RMS envelope + hysteresis state machine) -> OnVoiceAlert() when the
 * voice state changes.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "../Include/tiremo_mic.h"
#include "tiremo_mic_bsp.h"

#include <stddef.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

#define TIREMO_MIC_DEFAULT_SAMPLE_RATE      (16000U)

/** @brief Input volume 0-100, scales the signal before the HP filter gain. */
#define TIREMO_MIC_DEFAULT_VOLUME           (80U)

/** @brief DC offset subtracted before the high-pass filter. */
#define TIREMO_MIC_ADC_OFFSET                (1705U)

/** @brief 12-bit ADC result mask. */
#define TIREMO_MIC_ADC_MASK                  (0x0FFFU)

/** @brief DMA DDR halfword -> 12-bit sample (hardware left-aligns in DDR). */
#define TIREMO_MIC_ADC_RAW_FROM_DDR(h) \
    ((uint16_t)(((uint16_t)(h) & 0xFFF0U) >> 4U))

#define TIREMO_MIC_ADC_CLAMP12(v)            ((uint16_t)((v) & TIREMO_MIC_ADC_MASK))

/** @brief Unsigned PCM centre: stored value = filtered_sample + this. */
#define TIREMO_MIC_PCM_CENTER                (32768U)

/** @brief Digital gain applied after volume scaling in the HP filter path. */
#define TIREMO_MIC_SIGNAL_GAIN               (8U)

#define TIREMO_MIC_HP_COEFF                  (0xF8)
#define TIREMO_MIC_HP_SHIFT                  (256)

#define TIREMO_MIC_CLAMP(v, lo, hi) \
    (((v) < (lo)) ? (lo) : (((v) > (hi)) ? (hi) : (v)))

/** @brief Circular capture buffer length (samples). */
#define TIREMO_MIC_BUFFER_SIZE               (1024U)

/** @brief Envelope attack/release divisors (smaller = faster). */
#define TIREMO_MIC_VOICE_LEVEL_ATTACK_DIV    (2U)
#define TIREMO_MIC_VOICE_LEVEL_RELEASE_DIV   (16U)

/** @brief Consecutive buffers before entering HIGH_VOICE / leaving it. */
#define TIREMO_MIC_VOICE_LOUD_BUFFERS        (2U)
#define TIREMO_MIC_VOICE_NORMAL_BUFFERS      (8U)

/** @brief Hysteresis enter/exit smoothed-RMS thresholds. */
#define TIREMO_MIC_VOICE_LOUD_ENTER          (1000U)
#define TIREMO_MIC_VOICE_LOUD_EXIT           (700U)
#define TIREMO_MIC_VOICE_SILENCE_ENTER       (180U)
#define TIREMO_MIC_VOICE_SILENCE_EXIT        (450U)

/******************************************************************************
 * Private Types
 *****************************************************************************/

typedef struct
{
    int32_t z;
    int32_t oldOut;
    int32_t oldIn;
} TIREMO_MIC_HpFilter_t;

typedef enum
{
    TIREMO_MIC_VOICE_NORMAL = 0,
    TIREMO_MIC_VOICE_HIGH,
    TIREMO_MIC_VOICE_SILENCE
} TIREMO_MIC_VoiceState_t;

/******************************************************************************
 * Private Variables
 *****************************************************************************/

static uint16_t s_captureBuffer[TIREMO_MIC_BUFFER_SIZE];

static TIREMO_MIC_State_t s_state = TIREMO_MIC_STATE_RESET;
static uint32_t s_writeIndex = 0U;
static TIREMO_MIC_HpFilter_t s_hpFilter;

static TIREMO_MIC_VoiceState_t s_voiceState = TIREMO_MIC_VOICE_NORMAL;
static uint32_t s_loudStreak = 0U;
static uint32_t s_quietStreak = 0U;
static uint32_t s_voiceLevel = 0U;
/** @brief Linear gain in Q10 (1024 = 0 dB) applied after volume. */
static int32_t s_gainLinQ10 = 1024;

/******************************************************************************
 * Private Function Prototypes
 *****************************************************************************/

static void prvMicResetVoiceState(void);
static uint32_t prvMicIsqrt64(uint64_t x);
static uint32_t prvMicBufferRms(const uint16_t *pBuffer, uint32_t size);
static uint32_t prvMicUpdateVoiceLevel(uint32_t rms);
static TIREMO_MIC_Alert_t prvMicStateToAlert(TIREMO_MIC_VoiceState_t state);
static TIREMO_MIC_Alert_t prvMicUpdateVoiceState(uint32_t level);
static TIREMO_MIC_VoiceResult_t prvMicProcessVoice(const uint16_t *pBuffer,
                                                    uint32_t size);
static int16_t prvMicApplyHpFilter(int32_t rawSample,
                                   TIREMO_MIC_HpFilter_t *pFilter);
static void prvMicStoreSample(uint16_t rawAdc);
static int32_t prvMicDbToLinQ10(int32_t gainDb);
static uint32_t prvMicFloorLog2(uint32_t value);
static void prvMicHandleDmaBlock(const uint16_t *pRawSamples,
                                 uint32_t sampleCount);

/******************************************************************************
 * Public Functions
 *****************************************************************************/

bool TIREMO_MIC_Init(void)
{
    if (s_state == TIREMO_MIC_STATE_RECORDING)
    {
        return false;
    }

    TIREMO_MIC_BSP_Init();
    TIREMO_MIC_BSP_SetDataHandler(prvMicHandleDmaBlock);

    s_writeIndex = 0U;
    s_hpFilter.z = 0;
    s_hpFilter.oldOut = 0;
    s_hpFilter.oldIn = 0;
    prvMicResetVoiceState();

    if (!TIREMO_MIC_BSP_SetSampleRate(TIREMO_MIC_DEFAULT_SAMPLE_RATE))
    {
        s_state = TIREMO_MIC_STATE_ERROR;
        return false;
    }

    s_state = TIREMO_MIC_STATE_READY;
    return true;
}

void TIREMO_MIC_Deinit(void)
{
    if (s_state == TIREMO_MIC_STATE_RECORDING)
    {
        TIREMO_MIC_Stop();
    }

    TIREMO_MIC_BSP_SetDataHandler(NULL);
    s_state = TIREMO_MIC_STATE_RESET;
}

bool TIREMO_MIC_IsInitialized(void)
{
    return (s_state != TIREMO_MIC_STATE_RESET);
}

bool TIREMO_MIC_Start(void)
{
    if (s_state != TIREMO_MIC_STATE_READY)
    {
        return false;
    }

    s_writeIndex = 0U;
    s_hpFilter.z = 0;
    s_hpFilter.oldOut = 0;
    s_hpFilter.oldIn = 0;
    prvMicResetVoiceState();

    s_state = TIREMO_MIC_STATE_RECORDING;

    if (!TIREMO_MIC_BSP_Start())
    {
        s_state = TIREMO_MIC_STATE_ERROR;
        return false;
    }

    return true;
}

void TIREMO_MIC_Stop(void)
{
    if (s_state != TIREMO_MIC_STATE_RECORDING)
    {
        return;
    }

    TIREMO_MIC_BSP_Stop();
    s_state = TIREMO_MIC_STATE_READY;
}

void TIREMO_MIC_Service(void)
{
    TIREMO_MIC_BSP_Service();
}

TIREMO_MIC_State_t TIREMO_MIC_GetState(void)
{
    return s_state;
}

uint32_t TIREMO_MIC_GetDmaBlockCount(void)
{
    return TIREMO_MIC_BSP_GetDmaBlockCount();
}

uint32_t TIREMO_MIC_GetVoiceLevel(void)
{
    return s_voiceLevel;
}

void TIREMO_MIC_SetGainDb(int32_t gainDb)
{
    if (gainDb < -24)
    {
        gainDb = -24;
    }
    if (gainDb > 24)
    {
        gainDb = 24;
    }
    s_gainLinQ10 = prvMicDbToLinQ10(gainDb);
}

int32_t TIREMO_MIC_RmsToDb(uint32_t rms)
{
    uint32_t exp2;
    uint32_t lo;
    uint32_t hi;
    uint32_t frac;
    uint32_t dbx1000;

    if (rms == 0U)
    {
        return 0;
    }

    /* 20*log10(rms) ≈ 6.0206 * log2(rms), with linear interp between 2^n. */
    exp2 = prvMicFloorLog2(rms);
    lo = 1U << exp2;
    hi = lo << 1;
    if (hi <= lo)
    {
        frac = 0U;
    }
    else
    {
        frac = ((rms - lo) * 1000U) / (hi - lo);
    }

    dbx1000 = (6020U * exp2) + ((6020U * frac) / 1000U);
    return (int32_t)(dbx1000 / 1000U);
}

int32_t TIREMO_MIC_GetVoiceLevelDb(void)
{
    return TIREMO_MIC_RmsToDb(s_voiceLevel);
}

__attribute__((weak)) void TIREMO_MIC_OnVoiceAlert(TIREMO_MIC_VoiceResult_t result)
{
    (void)result;
}

/******************************************************************************
 * Private Functions
 *****************************************************************************/

static void prvMicResetVoiceState(void)
{
    s_voiceState = TIREMO_MIC_VOICE_NORMAL;
    s_loudStreak = 0U;
    s_quietStreak = 0U;
    s_voiceLevel = 0U;
}

static uint32_t prvMicIsqrt64(uint64_t x)
{
    uint64_t op = x;
    uint64_t res = 0U;
    uint64_t one = 1ULL << 62;

    if (x == 0U)
    {
        return 0U;
    }

    while (one > op)
    {
        one >>= 2;
    }

    while (one != 0U)
    {
        if (op >= (res + one))
        {
            op -= (res + one);
            res = (res >> 1) + one;
        }
        else
        {
            res >>= 1;
        }
        one >>= 2;
    }

    return (uint32_t)res;
}

static uint32_t prvMicBufferRms(const uint16_t *pBuffer, uint32_t size)
{
    uint32_t i;
    uint64_t sumSq = 0U;

    if ((pBuffer == NULL) || (size == 0U))
    {
        return 0U;
    }

    for (i = 0U; i < size; i++)
    {
        int32_t x = (int32_t)pBuffer[i] - (int32_t)TIREMO_MIC_PCM_CENTER;
        int64_t sq = (int64_t)x * (int64_t)x;

        sumSq += (uint64_t)sq;
    }

    return prvMicIsqrt64(sumSq / (uint64_t)size);
}

static uint32_t prvMicUpdateVoiceLevel(uint32_t rms)
{
    if (rms > s_voiceLevel)
    {
        uint32_t rise = (rms - s_voiceLevel) / TIREMO_MIC_VOICE_LEVEL_ATTACK_DIV;

        if (rise == 0U)
        {
            rise = 1U;
        }
        s_voiceLevel += rise;
        if (s_voiceLevel > rms)
        {
            s_voiceLevel = rms;
        }
    }
    else if (s_voiceLevel > rms)
    {
        uint32_t fall = (s_voiceLevel - rms) / TIREMO_MIC_VOICE_LEVEL_RELEASE_DIV;

        if (fall == 0U)
        {
            fall = 1U;
        }
        if (fall > s_voiceLevel)
        {
            s_voiceLevel = 0U;
        }
        else
        {
            s_voiceLevel -= fall;
        }
    }

    return s_voiceLevel;
}

static TIREMO_MIC_Alert_t prvMicStateToAlert(TIREMO_MIC_VoiceState_t state)
{
    switch (state)
    {
    case TIREMO_MIC_VOICE_HIGH:
        return TIREMO_MIC_ALERT_HIGH_VOICE;

    case TIREMO_MIC_VOICE_SILENCE:
        return TIREMO_MIC_ALERT_SILENCE;

    case TIREMO_MIC_VOICE_NORMAL:
    default:
        return TIREMO_MIC_ALERT_NORMAL;
    }
}

static TIREMO_MIC_Alert_t prvMicUpdateVoiceState(uint32_t level)
{
    switch (s_voiceState)
    {
    case TIREMO_MIC_VOICE_HIGH:
        if (level >= TIREMO_MIC_VOICE_LOUD_EXIT)
        {
            s_quietStreak = 0U;
            return TIREMO_MIC_ALERT_NONE;
        }

        s_quietStreak++;
        if (s_quietStreak < TIREMO_MIC_VOICE_NORMAL_BUFFERS)
        {
            return TIREMO_MIC_ALERT_NONE;
        }

        s_quietStreak = 0U;
        s_voiceState = (level <= TIREMO_MIC_VOICE_SILENCE_ENTER)
                           ? TIREMO_MIC_VOICE_SILENCE
                           : TIREMO_MIC_VOICE_NORMAL;
        return prvMicStateToAlert(s_voiceState);

    case TIREMO_MIC_VOICE_SILENCE:
        if (level <= TIREMO_MIC_VOICE_SILENCE_ENTER)
        {
            s_loudStreak = 0U;
            return TIREMO_MIC_ALERT_NONE;
        }

        if (level >= TIREMO_MIC_VOICE_LOUD_ENTER)
        {
            s_loudStreak++;
            if (s_loudStreak >= TIREMO_MIC_VOICE_LOUD_BUFFERS)
            {
                s_loudStreak = 0U;
                s_voiceState = TIREMO_MIC_VOICE_HIGH;
                return TIREMO_MIC_ALERT_HIGH_VOICE;
            }
            return TIREMO_MIC_ALERT_NONE;
        }

        if (level > TIREMO_MIC_VOICE_SILENCE_EXIT)
        {
            s_loudStreak = 0U;
            s_voiceState = TIREMO_MIC_VOICE_NORMAL;
            return TIREMO_MIC_ALERT_NORMAL;
        }
        return TIREMO_MIC_ALERT_NONE;

    case TIREMO_MIC_VOICE_NORMAL:
    default:
        if (level >= TIREMO_MIC_VOICE_LOUD_ENTER)
        {
            s_loudStreak++;
            s_quietStreak = 0U;

            if (s_loudStreak >= TIREMO_MIC_VOICE_LOUD_BUFFERS)
            {
                s_loudStreak = 0U;
                s_voiceState = TIREMO_MIC_VOICE_HIGH;
                return TIREMO_MIC_ALERT_HIGH_VOICE;
            }
        }
        else if (level <= TIREMO_MIC_VOICE_SILENCE_ENTER)
        {
            s_loudStreak = 0U;
            s_quietStreak++;

            if (s_quietStreak >= TIREMO_MIC_VOICE_NORMAL_BUFFERS)
            {
                s_quietStreak = 0U;
                s_voiceState = TIREMO_MIC_VOICE_SILENCE;
                return TIREMO_MIC_ALERT_SILENCE;
            }
        }
        else
        {
            s_loudStreak = 0U;
            s_quietStreak = 0U;
        }
        return TIREMO_MIC_ALERT_NONE;
    }
}

static TIREMO_MIC_VoiceResult_t prvMicProcessVoice(const uint16_t *pBuffer,
                                                    uint32_t size)
{
    TIREMO_MIC_VoiceResult_t result = { TIREMO_MIC_ALERT_NONE, 0U };
    uint32_t rms;

    if ((pBuffer == NULL) || (size == 0U))
    {
        return result;
    }

    rms = prvMicBufferRms(pBuffer, size);
    result.level = prvMicUpdateVoiceLevel(rms);
    result.alert = prvMicUpdateVoiceState(result.level);

    return result;
}

static int32_t prvMicDbToLinQ10(int32_t gainDb)
{
    int32_t steps;
    int32_t rem;
    int32_t v = 1024;

    steps = gainDb / 6;
    rem = gainDb - (steps * 6);

    while (steps > 0)
    {
        v *= 2;
        steps--;
    }
    while (steps < 0)
    {
        v /= 2;
        steps++;
    }

    /* ~1.122x per +1 dB residual (12.2%). */
    if (rem != 0)
    {
        v += (v * rem * 122) / 1000;
    }
    if (v < 1)
    {
        v = 1;
    }
    return v;
}

static uint32_t prvMicFloorLog2(uint32_t value)
{
    uint32_t r = 0U;

    while (value > 1U)
    {
        value >>= 1;
        r++;
    }
    return r;
}

static int16_t prvMicApplyHpFilter(int32_t rawSample, TIREMO_MIC_HpFilter_t *pFilter)
{
    int32_t out;
    int32_t signedSample;

    signedSample = rawSample - (int32_t)TIREMO_MIC_ADC_OFFSET;
    pFilter->z = (signedSample * (int32_t)TIREMO_MIC_DEFAULT_VOLUME) / 100;
    pFilter->z *= (int32_t)TIREMO_MIC_SIGNAL_GAIN;
    pFilter->z = (pFilter->z * s_gainLinQ10) / 1024;

    pFilter->oldOut = (TIREMO_MIC_HP_COEFF *
                       (pFilter->oldOut + pFilter->z - pFilter->oldIn)) /
                      TIREMO_MIC_HP_SHIFT;
    pFilter->oldIn = pFilter->z;

    out = TIREMO_MIC_CLAMP(pFilter->oldOut, -32760, 32760);
    return (int16_t)out;
}

static void prvMicStoreSample(uint16_t rawAdc)
{
    uint16_t raw12 = TIREMO_MIC_ADC_CLAMP12(rawAdc);
    int16_t filtered = prvMicApplyHpFilter((int32_t)raw12, &s_hpFilter);

    s_captureBuffer[s_writeIndex] = (uint16_t)((int32_t)filtered + 32768);
    s_writeIndex++;

    if (s_writeIndex >= TIREMO_MIC_BUFFER_SIZE)
    {
        TIREMO_MIC_VoiceResult_t result;

        s_writeIndex = 0U;
        result = prvMicProcessVoice(s_captureBuffer, TIREMO_MIC_BUFFER_SIZE);
        if (result.alert != TIREMO_MIC_ALERT_NONE)
        {
            TIREMO_MIC_OnVoiceAlert(result);
        }
    }
}

static void prvMicHandleDmaBlock(const uint16_t *pRawSamples, uint32_t sampleCount)
{
    uint32_t i;

    if ((s_state != TIREMO_MIC_STATE_RECORDING) || (pRawSamples == NULL))
    {
        return;
    }

    for (i = 0U; i < sampleCount; i++)
    {
        prvMicStoreSample(TIREMO_MIC_ADC_RAW_FROM_DDR(pRawSamples[i]));
    }
}
