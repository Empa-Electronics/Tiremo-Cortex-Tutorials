<p class="sphinxhide" align="right"><a>English</a> | <a href="../../docs-tr/Getting_Started/03-Debug_UART_Logging/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../images/banner.png" width="100%">
      <h1>Debug UART Logging</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Debug UART Logging

***Board: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Section: Getting Started · Difficulty: Easy***

## Introduction

Every tutorial from here on will be debugged over a UART log, not a debugger breakpoint — that's how the reference firmware itself was built. Get `TIREMO_LOG_*` working now, on top of Tutorial 02's two-LED loop: every toggle gets narrated over UART instead of just happening silently.

## You Will Learn

- `TIREMO_LOG_ERROR/WARN/INFO/DEBUG` and the `TIREMO_DEBUG_LEVEL` compile-time ceiling
- Overriding verbosity with `-DTIREMO_DEBUG_LEVEL=N` at CMake configure time — and a preprocessor pitfall that can silently break that override
- Why runtime `TIREMO_Debug_SetLevel()` can only go quieter, never louder
- Why `main.c` finally changes in this tutorial, after staying identical through Tutorials 01 and 02

## Prerequisites

- [02-SysTick_Timing](../02-SysTick_Timing/) — this tutorial's LED loop is exactly Tutorial 02's, with logging added
- `arm-none-eabi-gcc`, `cmake` (3.20+), and `ninja` on `PATH` (see [01-Hello_Blink's Prerequisites](../01-Hello_Blink/#prerequisites) — identical here, including why Windows needs no Git Bash/WSL with this build system)
- A serial terminal at 115200 8N1 to see the log output: `tdf monitor` (see [Solution](#solution)), Tera Term, PuTTY, screen, minicom, or your IDE's built-in one. No separate USB-to-serial adapter is needed: the board's onboard CMSIS-DAP probe provides a serial port over the same USB cable (for example `USB Serial Device (COMx)` on Windows).
- A Tiremo Cortex board (ABOV A34G43A MCU — see [Hardware Overview](../../Hardware.md)) connected to your PC with a USB cable. The board's onboard CMSIS-DAP debug probe handles SWD flashing; no separate programmer is needed.

## A Preprocessor Pitfall: Why the Levels Are Also Macros

This tutorial's second "You Will Learn" bullet — rebuild at a lower
`TIREMO_DEBUG_LEVEL` and confirm verbosity changes — depends on a
detail in `tiremo_debug.h` that is easy to get wrong. A first, natural
version of the header looks like this, right after declaring the level
enum:

```c
typedef enum { TIREMO_DEBUG_LEVEL_NONE = 0, ..., TIREMO_DEBUG_LEVEL_DEBUG = 4 } TIREMO_Debug_Level_t;

#define TIREMO_DEBUG_LEVEL  TIREMO_DEBUG_LEVEL_INFO   /* <- unconditional */
#ifndef TIREMO_DEBUG_LEVEL
#define TIREMO_DEBUG_LEVEL           TIREMO_DEBUG_LEVEL_INFO
#endif
```

Two independent mistakes stack here. First, the unconditional `#define`
above the `#ifndef` guard always wins — the guard below it can never
fire, so a `-DTIREMO_DEBUG_LEVEL=2` passed at build time gets silently
overwritten back to `INFO` regardless. Fixing just that isn't enough,
though — the deeper issue is that `TIREMO_DEBUG_LEVEL_ERROR`,
`_WARN`, `_INFO`, `_DEBUG` are **enum constants, not preprocessor
macros**. The C standard says any identifier the preprocessor doesn't
recognize as a macro evaluates to `0` inside a `#if` expression — so
every guard in the file, like:

```c
#if (TIREMO_DEBUG_LEVEL >= TIREMO_DEBUG_LEVEL_DEBUG)
#define TIREMO_LOG_DEBUG(...) TIREMO_Debug_Log(TIREMO_DEBUG_LEVEL_DEBUG, __VA_ARGS__)
#else
#define TIREMO_LOG_DEBUG(...) ((void)0)
#endif
```

silently reduces to `0 >= 0` — always true — no matter what level is
set. You can see this with the preprocessor (`arm-none-eabi-gcc -E`):
`TIREMO_LOG_DEBUG(...)` expands to a real, live `TIREMO_Debug_Log()`
call even with `-DTIREMO_DEBUG_LEVEL=2` passed. Every log macro is
always compiled in; only the *runtime* check inside `TIREMO_Debug_Log()`
(real C, not preprocessor `#if`) actually filters anything.

**The solution used in this repo**, in [`Sw/Common/Include/tiremo_debug.h`](./Sw/Common/Include/tiremo_debug.h):
redeclare each level as a plain numeric macro immediately after the
enum, before anything references them in a `#if`:

```c
#define TIREMO_DEBUG_LEVEL_NONE     0
#define TIREMO_DEBUG_LEVEL_ERROR    1
#define TIREMO_DEBUG_LEVEL_WARN     2
#define TIREMO_DEBUG_LEVEL_INFO     3
#define TIREMO_DEBUG_LEVEL_DEBUG    4
```

This is safe specifically because it comes *after* the enum typedef —
the enum's own declaration is already parsed by the time these
`#define`s exist, so nothing corrupts it. From that point on, though,
every one of those names — in this header's own `#if` guards, and in
any `.c` file's calls like `TIREMO_Debug_Log(TIREMO_DEBUG_LEVEL_ERROR, ...)` —
gets replaced by the matching plain integer before the compiler ever
sees it, which is both what makes the `#if` guards work and completely
harmless to every existing call site, since the numbers match the
enum's own values exactly.

**How to check it yourself:** build this tutorial's `Sw/` twice, once
at the default level and once with `tdf build -DTIREMO_DEBUG_LEVEL=2`.
`strings build/DebugUartLogging | grep "toggle @"` (macOS/Linux) or
`Select-String -Path build\DebugUartLogging -Pattern "toggle @"`
(PowerShell) finds the INFO log format strings twice in the default
build and **zero** times at level 2 — genuinely compiled out, not just
runtime-suppressed. Binary `.text` size drops by about a quarter between
the two (for example from about 12.6 KB to about 9.3 KB with Arm GNU
Toolchain 14.2.1; exact numbers vary slightly with the toolchain version).

## Solution

[`Sw/`](./Sw/) has a real, buildable implementation. From the repo root:

```sh
cd Getting_Started/03-Debug_UART_Logging/Sw
tdf build
tdf flash
```

(See [01-Hello_Blink's Solution](../01-Hello_Blink/#solution) for what
`tdf` is and how to get it on `PATH` via `./install.sh` + `source
./export.sh` — on Windows, `install.ps1` + `. .\export.ps1`, or
`install.bat` + `export.bat`. On macOS/Linux, `tdf.py` also works as an
alias; on Windows, use `tdf`.)

Same shared vendor SDK as Tutorials 01/02 ([`Generation/`](../../Generation/)
at the repo root), same CMake build as every tutorial in this repo (see
[01-Hello_Blink's Solution](../01-Hello_Blink/#solution) for why CMake).
This is the first tutorial whose vendor source list grows noticeably:
bringing up a UART adds the vendor UART HAL (`hal_uart.c`) and the
template's UART setup code (`user_uart.c`, `user_uart_isr.c`), which in
turn pull in the DMA and NMI vendor HAL modules (`hpl_dma.c`,
`hpl_nmi.c`) — see [`Sw/CMakeLists.txt`](./Sw/CMakeLists.txt).

**Confirmed on real hardware:** `tdf build` compiles and links without
errors at every debug level on macOS and Windows 11 (the linker prints
the same expected, harmless `_close` / `_lseek` / `_read` / `_write is
not implemented` warnings explained in [01-Hello_Blink's Solution](../01-Hello_Blink/#solution)).
`tdf flash` completed a full program-and-verify cycle over SWD on a
real, connected Tiremo Cortex board (`** Verified OK **`), and the
flashed image streams leveled `[INFO] LED1 toggle @ ... ms` /
`LED2 toggle @ ... ms` lines continuously, matching the blink pattern
from Tutorial 02.

To watch the log, run:

```sh
tdf monitor
```

It resets the board over SWD and streams the log, color-coded by level
(Ctrl+C to exit). It needs `pyserial`, which `install.sh` / `install.ps1`
/ `install.bat` set up for you. Only one program can open the serial port
at a time, so close any other terminal (Tera Term, PuTTY, …) on that
port first. `tdf monitor` is covered in more detail in
[04-Button_Read_And_Debounce's Solution](../04-Button_Read_And_Debounce/#solution);
you don't need to finish Tutorial 04 to use it here.

## Walkthrough

### 1. Board config — unchanged from Tutorial 02

[`Sw/Board/board_config.h`](./Sw/Board/board_config.h) is identical to
Tutorial 02's (LED1 + LED2). The debug UART's physical port
(`TIREMO_HAL_UART_PORT_0`) is hardcoded inside `tiremo_debug.c` itself,
not something this tutorial's board config needs to name.

### 2. Entry point — the one that finally changes

[`Sw/Src/main.c`](./Sw/Src/main.c) adds one call between Tutorials 01/02's
`PRV_SCU_CLK_Init()` and `PRV_USER_Code()`:

```c
PRV_PORT_Init();
PRV_SCU_CLK_Init();
PRV_UART_Init();     /* new: brings up all 3 vendor UARTs at 115200 8N1 */
PRV_USER_Code();
```

`PRV_UART_Init()` is vendor-generated, unmodified — it initializes all
three physical UART peripherals with no-op placeholder IRQ handlers,
whether or not this tutorial's code ends up using them.
[`Common/Src/tiremo_debug.c`](./Sw/Common/Src/tiremo_debug.c) later
re-registers UART 0's interrupt handler with Tiremo's own real one
inside `TIREMO_Debug_Init()` — no conflict, just a second
`HAL_UART_SetIRQ()` call overwriting the vendor placeholder for that one
port. This is why `main.c` changes here and didn't in Tutorials 01/02:
GPIO and SysTick needed no vendor peripheral bring-up call at all; UART
genuinely does.

### 3. Logging the existing blink loop

[`Sw/Src/prv_user_code.c`](./Sw/Src/prv_user_code.c) is Tutorial 02's loop
with two additions: `TIREMO_Debug_Init()` at the top, and one
`TIREMO_LOG_INFO()` call per toggle:

```c
TIREMO_SysTick_Init();
TIREMO_Debug_Init();
TIREMO_LOG_INFO("Debug_UART_Logging: boot");

/* ... same LED setup as Tutorial 02 ... */

for (;;)
{
    now = TIREMO_SysTick_GetMs();
    if ((now - led1LastMs) >= LED1_INTERVAL_MS)
    {
        TIREMO_HAL_GPIO_TogglePin(TIREMO_BOARD_LED1_PORT, TIREMO_BOARD_LED1_PIN);
        TIREMO_LOG_INFO("LED1 toggle @ %lu ms", (unsigned long)now);
        led1LastMs = now;
    }
    /* ... LED2, same shape ... */
}
```

`TIREMO_LOG_INFO(...)` expands to `TIREMO_Debug_Log(TIREMO_DEBUG_LEVEL_INFO, ...)`
when compiled in, prints `[INFO] <message>\r\n`, and is blocking
(`prvDebugTransmitBlocking()` waits for the UART to actually send) — worth
knowing before adding logging to a tight timing loop later in the
curriculum, the same way Tutorial 01 flagged `TogglePin()`'s
read-modify-write as worth remembering later.

### 4. The level macros

Covered in full above — see **A Preprocessor Pitfall**. The short
version: five `#define`s (`TIREMO_DEBUG_LEVEL_NONE` … `_DEBUG`) in
[`Sw/Common/Include/tiremo_debug.h`](./Sw/Common/Include/tiremo_debug.h)
right after the enum, so the `#if` guards can see the level values.

### 5. Build and flash

```sh
tdf build             # default level: INFO and above
tdf flash

# WARN and above only -- INFO lines compiled out. TIREMO_DEBUG_LEVEL is a
# CMake *cache* variable, set at configure time: it persists across
# rebuilds in this build/ directory until you pass -D again with a
# different value (or use a separate build directory per level).
# tdf forwards -D straight to cmake configure:
tdf build -DTIREMO_DEBUG_LEVEL=2
tdf flash

# Levels: 0=NONE (all logs compiled out), 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG.
# Back to the default (INFO) -- pass an empty value:
tdf build -DTIREMO_DEBUG_LEVEL=
```

Same OS-detection, same bundled-OpenOCD flow as Tutorials 01/02, just
reached through `tdf` — see [01-Hello_Blink's Solution](../01-Hello_Blink/#solution)
for what it is. See **Solution** above for what's confirmed on real
hardware for this tutorial specifically.

## Done When

Boot log shows `[INFO]` lines for boot and every LED toggle over a
115200 8N1 serial connection, and reconfiguring with
`-DTIREMO_DEBUG_LEVEL=2` then rebuilding and flashing makes them
disappear entirely — check it with `tdf monitor` on the serial output,
and with the `strings` / binary-size check described above.

<p align="center"><img alt="" src="../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
