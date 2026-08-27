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

***Board: Tiremo Cortex (ESP32) · Section: Getting Started / Getting Started · Difficulty: Easy***

## Introduction

Every tutorial from here on will be debugged over a UART log, not a debugger breakpoint — that's how the reference firmware itself was built. Get `TIREMO_LOG_*` working now, on top of Tutorial 02's two-LED loop: every toggle gets narrated over UART instead of just happening silently.

## You Will Learn

- `TIREMO_LOG_ERROR/WARN/INFO/DEBUG` and the `TIREMO_DEBUG_LEVEL` compile-time ceiling
- Overriding verbosity with `-DTIREMO_DEBUG_LEVEL=N` at CMake configure time — and a real bug found while getting that override to actually work
- Why runtime `TIREMO_Debug_SetLevel()` can only go quieter, never louder
- Why `main.c` finally changes in this tutorial, after staying identical through Tutorials 01 and 02

## Prerequisites

- [02-SysTick_Timing](../02-SysTick_Timing/) — this tutorial's LED loop is exactly Tutorial 02's, with logging added
- `arm-none-eabi-gcc`, `cmake` (3.20+), and `ninja` on `PATH` (see [01-Hello_Blink's Prerequisites](../01-Hello_Blink/#prerequisites) — identical here, including why Windows needs no Git Bash/WSL with this build system)
- A USB-to-serial terminal (screen, PuTTY, minicom, or your IDE's built-in one) at 115200 8N1, to actually see the log output
- An ABOV A34G43A Starter Kit connected over its onboard SWD debugger

## A Real Bug: the Compile-Time Level Wasn't Compiling Anything Out

This tutorial's third "You Will Learn" bullet — rebuild at a lower
`TIREMO_DEBUG_LEVEL` and confirm verbosity changes — is also the test
that exposed a real bug in `tiremo_debug.h`. The original header has
this, right after declaring the level enum:

```c
typedef enum { TIREMO_DEBUG_LEVEL_NONE = 0, ..., TIREMO_DEBUG_LEVEL_DEBUG = 4 } TIREMO_Debug_Level_t;

 #define TIREMO_DEBUG_LEVEL  TIREMO_DEBUG_LEVEL_INFO   /* <- unconditional */
#ifndef TIREMO_DEBUG_LEVEL
#define TIREMO_DEBUG_LEVEL           TIREMO_DEBUG_LEVEL_INFO
#endif
```

Two independent bugs stack here. First, the unconditional `#define`
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
set. Verified directly with the preprocessor (`arm-none-eabi-gcc -E`):
`TIREMO_LOG_DEBUG(...)` expanded to a real, live `TIREMO_Debug_Log()`
call even with `-DTIREMO_DEBUG_LEVEL=2` passed. Every log macro was
always compiled in; only the *runtime* check inside `TIREMO_Debug_Log()`
(real C, not preprocessor `#if`) was ever actually filtering anything.

**The fix**, in [`Sw/Common/Include/tiremo_debug.h`](./Sw/Common/Include/tiremo_debug.h):
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

**Verified, not assumed:** configured and built this tutorial's `Sw/`
twice, once at the default level and once with
`tdf.py build -DTIREMO_DEBUG_LEVEL=2`, and
`strings build/DebugUartLogging | grep "toggle @"` finds the INFO
log format strings twice in the default build and **zero** times at
level 2 — genuinely compiled out, not just runtime-suppressed. Binary
`.text` size drops from 12780 to 9436 bytes (~26%) between the two.

## Solution

[`Sw/`](./Sw/) has a real, buildable implementation:

```sh
cd Sw
tdf.py build
tdf.py flash
```

(See [01-Hello_Blink's Solution](../01-Hello_Blink/#solution) for what
`tdf.py` is and how to get it on `PATH` via `./install.sh` + `source
./export.sh`.)

Same shared vendor SDK as Tutorials 01/02 ([`Generation/`](../../Generation/)
at the repo root), same CMake build shape (see the same section for why
this repo moved off Make). This is the first tutorial in this repo whose
vendor source list needed real investigation rather than reuse —
bringing up a UART peripheral pulls in DMA and NMI vendor HAL modules
transitively (`hpl_dma.c`, `hpl_nmi.c`), discovered by compiling and
reading the linker's undefined-reference errors rather than guessing.

**Confirmed flashed and confirmed observed:** `tdf.py build` compiles
and links clean at both debug levels. `tdf.py flash` completed a full
program-and-verify cycle over SWD against a real, connected ABOV
A34G43A Starter Kit (`** Verified OK **`) — same as Tutorial 02.
UART output itself was unconfirmed for a while (no serial terminal
available), until `tdf.py monitor` (added in
[04-Button_Read_And_Debounce](../04-Button_Read_And_Debounce/#solution))
made that possible from this same environment. Genuinely observed on
this tutorial's flashed image: leveled `[INFO] LED1 toggle @ ... ms` /
`LED2 toggle @ ... ms` lines streaming continuously, matching the blink
pattern from Tutorial 02.

```sh
tdf.py monitor
```

resets the board over SWD and streams the log, color-coded by level.
See [04-Button_Read_And_Debounce's Solution](../04-Button_Read_And_Debounce/#solution)
for what `monitor` is and how it's built.

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
curriculum, the same way Tutorial 02 flagged `TogglePin()`'s
read-modify-write as worth remembering later.

### 4. The bug fix

Covered in full above — see **A Real Bug**. The short version: three
`#define`s added to [`Sw/Common/Include/tiremo_debug.h`](./Sw/Common/Include/tiremo_debug.h)
after the enum, nothing else in the file touched.

### 5. Build and flash

```sh
tdf.py build             # default level: INFO and above
tdf.py flash

# WARN and above only -- INFO lines compiled out. TIREMO_DEBUG_LEVEL is a
# CMake *cache* variable under the hood, set at configure time, not a
# one-shot build-time override like the old `make TIREMO_DEBUG_LEVEL=2`
# was -- it persists across rebuilds in this build/ directory until you
# pass -D again with a different value (or use a separate build
# directory per level). tdf.py forwards -D straight to cmake configure:
tdf.py build -DTIREMO_DEBUG_LEVEL=2
tdf.py flash
```

Same OS-detection, same bundled-OpenOCD flow as Tutorials 01/02, just
reached through `tdf.py` — see [01-Hello_Blink's Solution](../01-Hello_Blink/#solution)
for what it is. See **Solution** above for what's confirmed on real
hardware for this tutorial specifically.

## Done When

Boot log shows `[INFO]` lines for boot and every LED toggle over a
115200 8N1 serial connection, and reconfiguring with
`-DTIREMO_DEBUG_LEVEL=2` then rebuilding makes them disappear entirely —
verified via `strings`/binary size above, and via `tdf.py monitor`
directly on the serial output itself.

<p align="center"><img alt="" src="../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
