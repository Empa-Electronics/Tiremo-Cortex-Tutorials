/******************************************************************************
 * @file    tiremo_debug.h
 * @brief   Tiremo debug console API.
 *
 * Provides polling and interrupt-driven debug communication over the Tiremo
 * UART hardware abstraction.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef TIREMO_DEBUG_H
#define TIREMO_DEBUG_H

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
 * Public Macros
 *****************************************************************************/

#define TIREMO_DEBUG_RX_RING_SIZE    (128U)

/******************************************************************************
 * Public Types
 *****************************************************************************/

/**
 * @brief Log severity (higher value = more verbose).
 */
typedef enum
{
    TIREMO_DEBUG_LEVEL_NONE = 0,
    TIREMO_DEBUG_LEVEL_ERROR = 1,
    TIREMO_DEBUG_LEVEL_WARN = 2,
    TIREMO_DEBUG_LEVEL_INFO = 3,
    TIREMO_DEBUG_LEVEL_DEBUG = 4
} TIREMO_Debug_Level_t;

/*
 * Bug fix (see this tutorial's README "A Real Bug" section): the enum
 * values above are invisible to the preprocessor -- #if can only see
 * macros, and an undefined identifier in a #if expression is just 0 per
 * the C standard, not its enum value. Without these, every #if below
 * silently reduced to "0 >= 0" (always true) regardless of
 * TIREMO_DEBUG_LEVEL, so -DTIREMO_DEBUG_LEVEL=N never compiled anything
 * out. Defined after the enum (not before), so the enum's own
 * declaration is untouched -- these just shadow the same names, at the
 * same values, for the preprocessor's benefit from here down.
 */
#define TIREMO_DEBUG_LEVEL_NONE     0
#define TIREMO_DEBUG_LEVEL_ERROR    1
#define TIREMO_DEBUG_LEVEL_WARN     2
#define TIREMO_DEBUG_LEVEL_INFO     3
#define TIREMO_DEBUG_LEVEL_DEBUG    4

/**
 * @brief Compile-time maximum log verbosity.
 *
 * Messages above this level are compiled out. Runtime
 * TIREMO_Debug_SetLevel() can only go equal or quieter.
 *
 * Override from the Makefile, e.g. `-DTIREMO_DEBUG_LEVEL=2` (WARN).
 */
#ifndef TIREMO_DEBUG_LEVEL
#define TIREMO_DEBUG_LEVEL    TIREMO_DEBUG_LEVEL_INFO
#endif

/**
 * @brief Define the RX byte callback type.
 *
 * @param[in] byte  Received byte.
 *
 * @warning The callback runs in interrupt context and must remain short.
 */
typedef void (*TIREMO_Debug_RxCallback_t)(uint8_t byte);

/******************************************************************************
 * Public Function Prototypes
 *****************************************************************************/

/**
 * @brief Initialize the debug console.
 *
 * @return true when the debug console was initialized successfully.
 *
 * @warning Initialize the board UART before calling this function.
 */
bool TIREMO_Debug_Init(void);

/**
 * @brief Set the runtime log level (clamped to TIREMO_DEBUG_LEVEL).
 */
void TIREMO_Debug_SetLevel(TIREMO_Debug_Level_t level);

/**
 * @brief Get the current runtime log level.
 */
TIREMO_Debug_Level_t TIREMO_Debug_GetLevel(void);

/**
 * @brief Print a leveled log line: `[TAG] <fmt...>\r\n`.
 *
 * Prefer the TIREMO_LOG_* macros so compile-time filtering applies.
 */
void TIREMO_Debug_Log(TIREMO_Debug_Level_t level, const char *format, ...);

#if (TIREMO_DEBUG_LEVEL >= TIREMO_DEBUG_LEVEL_ERROR)
#define TIREMO_LOG_ERROR(...) \
    TIREMO_Debug_Log(TIREMO_DEBUG_LEVEL_ERROR, __VA_ARGS__)
#else
#define TIREMO_LOG_ERROR(...) ((void)0)
#endif

#if (TIREMO_DEBUG_LEVEL >= TIREMO_DEBUG_LEVEL_WARN)
#define TIREMO_LOG_WARN(...) \
    TIREMO_Debug_Log(TIREMO_DEBUG_LEVEL_WARN, __VA_ARGS__)
#else
#define TIREMO_LOG_WARN(...) ((void)0)
#endif

#if (TIREMO_DEBUG_LEVEL >= TIREMO_DEBUG_LEVEL_INFO)
#define TIREMO_LOG_INFO(...) \
    TIREMO_Debug_Log(TIREMO_DEBUG_LEVEL_INFO, __VA_ARGS__)
#else
#define TIREMO_LOG_INFO(...) ((void)0)
#endif

#if (TIREMO_DEBUG_LEVEL >= TIREMO_DEBUG_LEVEL_DEBUG)
#define TIREMO_LOG_DEBUG(...) \
    TIREMO_Debug_Log(TIREMO_DEBUG_LEVEL_DEBUG, __VA_ARGS__)
#else
#define TIREMO_LOG_DEBUG(...) ((void)0)
#endif

/**
 * @brief Uninitialize the debug console.
 */
void TIREMO_Debug_Uninit(void);

/**
 * @brief Register an RX callback for received bytes.
 *
 * Received bytes are always stored in the RX ring, whether a callback is
 * registered or not.
 *
 * @param[in] callback  RX callback, or NULL to unregister.
 */
void TIREMO_Debug_SetRxCallback(TIREMO_Debug_RxCallback_t callback);

/**
 * @brief Discard all unread bytes from the RX ring.
 */
void TIREMO_Debug_FlushRx(void);

/**
 * @brief Transmit one character.
 *
 * @param[in] ch  Character to transmit.
 */
void TIREMO_Debug_PutChar(uint8_t ch);

/**
 * @brief Transmit a null-terminated string.
 *
 * @param[in] str  String to transmit.
 */
void TIREMO_Debug_Puts(const char *str);

/**
 * @brief Transmit a null-terminated string followed by CRLF.
 *
 * @param[in] str  String to transmit.
 */
void TIREMO_Debug_PutsLine(const char *str);

/**
 * @brief Transmit an 8-bit unsigned decimal value from 000 to 255.
 *
 * @param[in] num  Value to print.
 */
void TIREMO_Debug_PutDec(uint8_t num);

/**
 * @brief Transmit a 16-bit unsigned decimal value.
 *
 * @param[in] num  Value to print.
 */
void TIREMO_Debug_PutDec16(uint16_t num);

/**
 * @brief Transmit a 32-bit unsigned decimal value.
 *
 * @param[in] num  Value to print.
 */
void TIREMO_Debug_PutDec32(uint32_t num);

/**
 * @brief Transmit an 8-bit value as two hexadecimal digits.
 *
 * @param[in] num  Value to print.
 */
void TIREMO_Debug_PutHex(uint8_t num);

/**
 * @brief Transmit a 16-bit value as four hexadecimal digits.
 *
 * @param[in] num  Value to print.
 */
void TIREMO_Debug_PutHex16(uint16_t num);

/**
 * @brief Transmit a 32-bit value as eight hexadecimal digits.
 *
 * @param[in] num  Value to print.
 */
void TIREMO_Debug_PutHex32(uint32_t num);

/**
 * @brief Transmit a printf-style formatted string.
 *
 * @param[in] format  Format string.
 * @param[in] ...     Format arguments.
 */
void TIREMO_Debug_Printf(const char *format, ...);

/**
 * @brief Wait until one byte is available, then return it.
 *
 * @return Received byte.
 *
 * @note This function blocks until a byte is received.
 * @warning Do not call this function from interrupt context.
 */
uint8_t TIREMO_Debug_WaitChar(void);

/**
 * @brief Wait until one byte is available, then return it.
 *
 * @return Received byte.
 *
 * @note This function blocks until a byte is received.
 * @warning Do not call this function from interrupt context.
 */
uint8_t TIREMO_Debug_GetChar(void);

/**
 * @brief Read one line into a buffer.
 *
 * Blocks until a newline (`\\r` or `\\n`) is received, or until
 * `maxLength - 1` payload bytes are stored. The newline is consumed and is
 * not written to the buffer. The buffer is always null-terminated when
 * `maxLength` is greater than zero.
 *
 * @param[out] pBuffer    Destination buffer.
 * @param[in]  maxLength  Buffer size, including the null terminator.
 *
 * @return Number of payload bytes stored, excluding the null terminator.
 *
 * @warning Do not call this function from interrupt context.
 */
uint32_t TIREMO_Debug_Read(uint8_t *pBuffer, uint32_t maxLength);

/**
 * @brief Check whether the RX ring contains unread data.
 *
 * @return true when unread data is available.
 */
bool TIREMO_Debug_IsDataAvailable(void);

/**
 * @brief Check whether the debug console is initialized.
 *
 * @return true when TIREMO_Debug_Init() succeeded.
 */
bool TIREMO_Debug_IsInitialized(void);

#ifdef __cplusplus
}
#endif

#endif /* TIREMO_DEBUG_H */
