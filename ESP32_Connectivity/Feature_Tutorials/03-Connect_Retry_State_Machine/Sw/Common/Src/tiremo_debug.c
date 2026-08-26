/******************************************************************************
 * @file    tiremo_debug.c
 * @brief   Tiremo debug console implementation.
 *
 * Provides debug logging and data reception using the Tiremo UART hardware
 * abstraction.
 *
 * Copyright (c) 2026 Tiremo
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "../Include/tiremo_debug.h"
#include "../../Tiremo_Hal/Include/tiremo_hal_uart.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

#if (TIREMO_DEBUG_RX_RING_SIZE < 2U) || \
    ((TIREMO_DEBUG_RX_RING_SIZE & (TIREMO_DEBUG_RX_RING_SIZE - 1U)) != 0U)
#error "TIREMO_DEBUG_RX_RING_SIZE must be a power of two and at least two"
#endif

#define TIREMO_DEBUG_UART_PORT            TIREMO_HAL_UART_PORT_0
#define TIREMO_DEBUG_UART_IRQ_PRIORITY    (3U)
#define TIREMO_DEBUG_RX_MASK              (TIREMO_DEBUG_RX_RING_SIZE - 1U)

/******************************************************************************
 * Private Types
 *****************************************************************************/

typedef struct
{
    volatile uint32_t head;
    volatile uint32_t tail;
    uint8_t data[TIREMO_DEBUG_RX_RING_SIZE];
} TIREMO_Debug_RxRing_t;

/******************************************************************************
 * Private Variables
 *****************************************************************************/

static bool s_initialized = false;
static bool s_uartActive = false;
static volatile bool s_rxArmPending = false;
static TIREMO_Debug_RxCallback_t s_rxCallback = NULL;
static TIREMO_Debug_RxRing_t s_rxRing;
static uint8_t s_rxByte = 0U;
static TIREMO_Debug_Level_t s_logLevel = (TIREMO_Debug_Level_t)TIREMO_DEBUG_LEVEL;

/******************************************************************************
 * Private Function Prototypes
 *****************************************************************************/

static bool prvDebugRxRingIsFull(void);
static bool prvDebugRxRingIsEmpty(void);
static void prvDebugRxRingPush(uint8_t byte);
static bool prvDebugRxRingPop(uint8_t *pByte);
static bool prvDebugRxRingPeek(uint8_t *pByte);
static uint32_t prvDebugRxRingCount(void);
static void prvDebugUARTCallback(uint32_t event, void *pContext);
static void prvDebugArmRx(void);
static uint8_t prvDebugWaitByte(void);
static bool prvDebugStartUART(void);
static void prvDebugStopUART(void);
static void prvDebugTransmitBlocking(const uint8_t *pData, uint32_t length);
static void prvDebugSendByte(uint8_t data);

/******************************************************************************
 * Public Functions
 *****************************************************************************/

void TIREMO_Debug_SetRxCallback(TIREMO_Debug_RxCallback_t callback)
{
    s_rxCallback = callback;
}

bool TIREMO_Debug_Init(void)
{
    if (!prvDebugStartUART())
    {
        return false;
    }

    s_logLevel = (TIREMO_Debug_Level_t)TIREMO_DEBUG_LEVEL;
    s_initialized = true;
    return true;
}

void TIREMO_Debug_SetLevel(TIREMO_Debug_Level_t level)
{
    if ((int)level > (int)TIREMO_DEBUG_LEVEL)
    {
        level = (TIREMO_Debug_Level_t)TIREMO_DEBUG_LEVEL;
    }
    s_logLevel = level;
}

TIREMO_Debug_Level_t TIREMO_Debug_GetLevel(void)
{
    return s_logLevel;
}

void TIREMO_Debug_Log(TIREMO_Debug_Level_t level, const char *format, ...)
{
    char buffer[176];
    const char *pTag;
    int prefixLen;
    int length;
    va_list args;

    if ((format == NULL) || (!s_initialized) ||
        ((int)level <= (int)TIREMO_DEBUG_LEVEL_NONE) ||
        ((int)level > (int)s_logLevel))
    {
        return;
    }

    switch (level)
    {
    case TIREMO_DEBUG_LEVEL_ERROR:
        pTag = "[ERROR] ";
        break;
    case TIREMO_DEBUG_LEVEL_WARN:
        pTag = "[WARN] ";
        break;
    case TIREMO_DEBUG_LEVEL_DEBUG:
        pTag = "[DEBUG] ";
        break;
    case TIREMO_DEBUG_LEVEL_INFO:
    default:
        pTag = "[INFO] ";
        break;
    }

    prefixLen = (int)strlen(pTag);
    if (prefixLen >= (int)sizeof(buffer))
    {
        return;
    }

    (void)memcpy(buffer, pTag, (size_t)prefixLen);

    va_start(args, format);
    length = vsnprintf(buffer + prefixLen,
                       sizeof(buffer) - (size_t)prefixLen, format, args);
    va_end(args);

    if (length < 0)
    {
        return;
    }

    length += prefixLen;
    if (length >= (int)sizeof(buffer))
    {
        length = (int)sizeof(buffer) - 1;
    }

    if ((length + 2) < (int)sizeof(buffer))
    {
        buffer[length++] = '\r';
        buffer[length++] = '\n';
    }
    buffer[length] = '\0';

    prvDebugTransmitBlocking((const uint8_t *)buffer, (uint32_t)length);
}

void TIREMO_Debug_Uninit(void)
{
    prvDebugStopUART();
    s_rxCallback = NULL;
    s_rxArmPending = false;
    s_initialized = false;
}

void TIREMO_Debug_FlushRx(void)
{
    uint8_t byte;

    while (prvDebugRxRingPop(&byte))
    {
    }
}

void TIREMO_Debug_PutChar(uint8_t ch)
{
    prvDebugSendByte(ch);
}

void TIREMO_Debug_Puts(const char *str)
{
    uint32_t length;

    if ((str == NULL) || (!s_initialized))
    {
        return;
    }

    length = (uint32_t)strlen(str);
    if (length == 0U)
    {
        return;
    }

    prvDebugTransmitBlocking((const uint8_t *)str, length);
}

void TIREMO_Debug_PutsLine(const char *str)
{
    TIREMO_Debug_Puts(str);
    TIREMO_Debug_Puts("\r\n");
}

void TIREMO_Debug_PutDec(uint8_t num)
{
    uint8_t digit1 = (uint8_t)(num % 10U);
    uint8_t digit2 = (uint8_t)((num / 10U) % 10U);
    uint8_t digit3 = (uint8_t)((num / 100U) % 10U);

    prvDebugSendByte((uint8_t)('0' + digit3));
    prvDebugSendByte((uint8_t)('0' + digit2));
    prvDebugSendByte((uint8_t)('0' + digit1));
}

void TIREMO_Debug_PutDec16(uint16_t num)
{
    char buffer[6];
    int index = 0;

    if (num == 0U)
    {
        prvDebugSendByte((uint8_t)'0');
        return;
    }

    while ((num > 0U) && (index < 5))
    {
        buffer[index++] = (char)('0' + (num % 10U));
        num = (uint16_t)(num / 10U);
    }

    while (index > 0)
    {
        prvDebugSendByte((uint8_t)buffer[--index]);
    }
}

void TIREMO_Debug_PutDec32(uint32_t num)
{
    char buffer[11];
    int index = 0;

    if (num == 0UL)
    {
        prvDebugSendByte((uint8_t)'0');
        return;
    }

    while ((num > 0UL) && (index < 10))
    {
        buffer[index++] = (char)('0' + (num % 10UL));
        num /= 10UL;
    }

    while (index > 0)
    {
        prvDebugSendByte((uint8_t)buffer[--index]);
    }
}

void TIREMO_Debug_PutHex(uint8_t num)
{
    static const char hexDigits[] = "0123456789ABCDEF";

    prvDebugSendByte((uint8_t)hexDigits[(num >> 4) & 0x0FU]);
    prvDebugSendByte((uint8_t)hexDigits[num & 0x0FU]);
}

void TIREMO_Debug_PutHex16(uint16_t num)
{
    TIREMO_Debug_PutHex((uint8_t)(num >> 8));
    TIREMO_Debug_PutHex((uint8_t)(num & 0xFFU));
}

void TIREMO_Debug_PutHex32(uint32_t num)
{
    TIREMO_Debug_PutHex16((uint16_t)(num >> 16));
    TIREMO_Debug_PutHex16((uint16_t)(num & 0xFFFFUL));
}

void TIREMO_Debug_Printf(const char *format, ...)
{
    char buffer[160];
    int length;
    va_list args;

    if ((format == NULL) || (!s_initialized))
    {
        return;
    }

    va_start(args, format);
    length = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (length <= 0)
    {
        return;
    }

    if (length >= (int)sizeof(buffer))
    {
        length = (int)sizeof(buffer) - 1;
    }

    prvDebugTransmitBlocking((const uint8_t *)buffer, (uint32_t)length);
}

uint8_t TIREMO_Debug_WaitChar(void)
{
    return TIREMO_Debug_GetChar();
}

uint8_t TIREMO_Debug_GetChar(void)
{
    if (!s_initialized)
    {
        return 0U;
    }

    return prvDebugWaitByte();
}

uint32_t TIREMO_Debug_Read(uint8_t *pBuffer, uint32_t maxLength)
{
    uint32_t count = 0U;
    uint32_t payloadLimit;
    uint8_t byte;
    uint8_t next;

    if ((!s_initialized) || (pBuffer == NULL) || (maxLength == 0U))
    {
        return 0U;
    }

    payloadLimit = maxLength - 1U;

    while (count < payloadLimit)
    {
        byte = prvDebugWaitByte();

        if ((byte == (uint8_t)'\r') || (byte == (uint8_t)'\n'))
        {
            if ((byte == (uint8_t)'\r') &&
                prvDebugRxRingPeek(&next) &&
                (next == (uint8_t)'\n'))
            {
                (void)prvDebugRxRingPop(&next);
            }

            break;
        }

        pBuffer[count] = byte;
        count++;
    }

    pBuffer[count] = 0U;
    return count;
}

bool TIREMO_Debug_IsDataAvailable(void)
{
    if (!s_initialized)
    {
        return false;
    }

    return (prvDebugRxRingCount() > 0U);
}

bool TIREMO_Debug_IsInitialized(void)
{
    return s_initialized;
}

/******************************************************************************
 * Private Functions
 *****************************************************************************/

static bool prvDebugRxRingIsFull(void)
{
    uint32_t used;

    used = s_rxRing.head - s_rxRing.tail;
    return (used >= (TIREMO_DEBUG_RX_RING_SIZE - 1U));
}

static bool prvDebugRxRingIsEmpty(void)
{
    return (s_rxRing.head == s_rxRing.tail);
}

static void prvDebugRxRingPush(uint8_t byte)
{
    uint32_t index;

    if (prvDebugRxRingIsFull())
    {
        return;
    }

    index = s_rxRing.head & TIREMO_DEBUG_RX_MASK;
    s_rxRing.data[index] = byte;
    s_rxRing.head++;
}

static bool prvDebugRxRingPop(uint8_t *pByte)
{
    uint32_t index;

    if (prvDebugRxRingIsEmpty())
    {
        return false;
    }

    index = s_rxRing.tail & TIREMO_DEBUG_RX_MASK;
    *pByte = s_rxRing.data[index];
    s_rxRing.tail++;
    return true;
}

static bool prvDebugRxRingPeek(uint8_t *pByte)
{
    uint32_t index;

    if (prvDebugRxRingIsEmpty())
    {
        return false;
    }

    index = s_rxRing.tail & TIREMO_DEBUG_RX_MASK;
    *pByte = s_rxRing.data[index];
    return true;
}

static uint32_t prvDebugRxRingCount(void)
{
    return (s_rxRing.head - s_rxRing.tail);
}

static void prvDebugUARTCallback(uint32_t event, void *pContext)
{
    (void)pContext;

    if (!s_uartActive)
    {
        return;
    }

    if ((event & TIREMO_HAL_UART_EVENT_RX_DONE) != 0U)
    {
        prvDebugRxRingPush(s_rxByte);

        if (s_rxCallback != NULL)
        {
            s_rxCallback(s_rxByte);
        }

        prvDebugArmRx();
    }

    if ((event & TIREMO_HAL_UART_EVENT_LINE_ERROR) != 0U)
    {
        prvDebugArmRx();
    }

    if (((event & TIREMO_HAL_UART_EVENT_TX_DONE) != 0U) &&
        s_rxArmPending)
    {
        prvDebugArmRx();
    }
}

static void prvDebugArmRx(void)
{
    TIREMO_HAL_UART_Status_t status;

    if (!s_uartActive)
    {
        return;
    }

    status = TIREMO_HAL_UART_Receive(TIREMO_DEBUG_UART_PORT,
                                     &s_rxByte,
                                     1U,
                                     false);
    s_rxArmPending = (status != TIREMO_HAL_UART_STATUS_OK);
}

static uint8_t prvDebugWaitByte(void)
{
    uint8_t byte = 0U;

    while (!prvDebugRxRingPop(&byte))
    {
        /* The interrupt handler stores received bytes in the RX ring. */
    }

    return byte;
}

static bool prvDebugStartUART(void)
{
    TIREMO_HAL_UART_Status_t status;

    if (s_uartActive)
    {
        return true;
    }

    s_rxRing.head = 0U;
    s_rxRing.tail = 0U;

    status = TIREMO_HAL_UART_SetInterruptHandler(
        TIREMO_DEBUG_UART_PORT,
        prvDebugUARTCallback,
        NULL,
        TIREMO_DEBUG_UART_IRQ_PRIORITY);
    if (status != TIREMO_HAL_UART_STATUS_OK)
    {
        return false;
    }

    s_uartActive = true;
    prvDebugArmRx();

    return true;
}

static void prvDebugStopUART(void)
{
    s_uartActive = false;
    (void)TIREMO_HAL_UART_Abort(TIREMO_DEBUG_UART_PORT);
}

static void prvDebugTransmitBlocking(const uint8_t *pData, uint32_t length)
{
    if ((!s_initialized) || (pData == NULL) || (length == 0U))
    {
        return;
    }

    (void)TIREMO_HAL_UART_Transmit(TIREMO_DEBUG_UART_PORT,
                                   pData,
                                   length,
                                   true);
}

static void prvDebugSendByte(uint8_t data)
{
    prvDebugTransmitBlocking(&data, 1U);
}
