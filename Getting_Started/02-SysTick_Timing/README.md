<p class="sphinxhide" align="right"><a>English</a> | <a href="../../docs-tr/Getting_Started/02-SysTick_Timing/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../images/banner.png" width="100%">
      <h1>SysTick Timing</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# SysTick Timing

***Board: Tiremo Cortex (ESP32) · Section: Getting Started / Getting Started · Difficulty: Easy***

## Introduction

Every later tutorial — WiFi retries, MQTT polling, alarm debouncing — needs a free-running millisecond clock instead of a busy-wait. This tutorial brings up SysTick once so nothing downstream has to re-derive it, and proves it with something Tutorial 01's busy-wait loop structurally cannot do: run two LEDs at two different, accurate rates from one loop.

## You Will Learn

- `TIREMO_SysTick_Init()` / `TIREMO_SysTick_GetMs()` / `TIREMO_SysTick_DelayMs()`
- Why elapsed-time math (`now - last >= interval`) is safer than nested delays, and how it survives the millisecond counter wrapping around
- The same `Tiremo_Hal` (vendor-facing) / `Common` (application-facing) split Tutorial 01 introduced for GPIO, applied to timing

## Prerequisites

- [01-Hello_Blink](../01-Hello_Blink/) — this tutorial reuses its GPIO HAL and board_config.h pattern unchanged
- `arm-none-eabi-gcc`, `cmake` (3.20+), and `ninja` on `PATH` (see [01-Hello_Blink's Prerequisites](../01-Hello_Blink/#prerequisites) — identical here, including why Windows needs no Git Bash/WSL with this build system)
- An ABOV A34G43A Starter Kit connected over its onboard SWD debugger

## Understanding Non-Blocking Timing

Tutorial 01's `prvDelay()` was a blocking busy-wait: the CPU sits in an
empty loop doing nothing else until the count finishes. That's fine for
one LED. It structurally cannot drive a second LED at a different rate,
service a button, or poll anything else — the CPU is stuck counting.
Every tutorial from here on needs to do more than one thing per loop
iteration, so this is the point where that has to change.

**The hardware side** ([`Tiremo_Hal/Src/tiremo_hal_systick.c`](./Sw/Tiremo_Hal/Src/tiremo_hal_systick.c))
configures the Cortex-M4 core's own SysTick peripheral — not an ABOV
vendor timer, but a standard piece of every Cortex-M core — to interrupt
once a millisecond:

```c
reloadTicks = SystemCoreClock / 1000UL;   /* 1000 Hz = 1 ms period */
SysTick_Config(reloadTicks);              /* CMSIS core call, not vendor HAL */
```

`SystemCoreClock` is the same global Tutorial 01's `PRV_SCU_CLK_Init()`
already sets as a side effect of configuring the PLL — no new vendor
source files are needed for this tutorial at all, just this one small
CMSIS-based file plus the millisecond-counter logic on top of it.

**The application side** ([`Common/Src/tiremo_systick.c`](./Sw/Common/Src/tiremo_systick.c))
is what every later tutorial actually calls. It registers a callback with
the hardware layer above, and on every 1 ms interrupt increments a
free-running counter:

```c
static volatile uint32_t s_msTick = 0U;

static void prvSysTickHandleTick(void)
{
    s_msTick++;
}

uint32_t TIREMO_SysTick_GetMs(void)
{
    return s_msTick;
}
```

That header also declares `TIREMO_SysTick_DelayMs()` — a *blocking* delay
built the same way as `prvDelay()`, just accurate — and a weak
`TIREMO_SysTick_OnTick1ms()` hook for future per-millisecond application
work. Neither is used in this tutorial: the whole point here is the
non-blocking pattern below, not a more accurate version of blocking.

**The pattern every non-blocking loop in this repo follows** is one
comparison per tracked thing, each with its own "last fired" timestamp:

```c
now = TIREMO_SysTick_GetMs();
if ((now - lastMs) >= interval)
{
    /* do the thing */
    lastMs = now;
}
```

`now - lastMs` is unsigned subtraction, which is the detail that makes
this safe forever, not just until the counter wraps: at `UINT32_MAX + 1`
the counter rolls over to `0`, but unsigned wraparound arithmetic makes
`now - lastMs` compute the correct elapsed time regardless — no special
case needed. This is the exact idiom the full reference firmware uses
throughout its connection-retry and telemetry-interval logic.

## Solution

[`Sw/`](./Sw/) has a real, buildable implementation:

```sh
cd Sw
tdf.py build
tdf.py flash
```

(First time in a new shell, `source ../../../export.sh` from the repo
root puts `tdf.py` on `PATH` — see [01-Hello_Blink's Solution](../01-Hello_Blink/#solution)
for what `tdf.py` actually is, and why this repo uses it instead of raw
`cmake`/`make` invocations.)

Same shared vendor SDK as Tutorial 01 ([`Generation/`](../../Generation/)
at the repo root), same CMake shape (this tutorial, along with 01 and
03, moved from a hand-written Makefile to CMake — see the same section
for why) — this time linking a single ~5.5 KB image with the SysTick
layer added on top of Tutorial 01's GPIO HAL.

**Confirmed on real hardware.** `tdf.py build` compiles and links
clean, `tdf.py flash` completed a full
program-and-verify cycle over SWD against a real, connected ABOV
A34G43A Starter Kit, and — checked visually on the board — both LEDs
blink at clearly different rates. `LED1_INTERVAL_MS` /
`LED2_INTERVAL_MS` in `Sw/Src/prv_user_code.c` ended up tuned to `1000`
/ `5000` (2 s and 10 s full cycles) rather than this write-up's original
guess of `500` / `300` — a much bigger gap made the two rates
unmistakable at a glance, which a subtler difference didn't.

## Walkthrough

### 1. Board config — add a second LED

[`Sw/Board/board_config.h`](./Sw/Board/board_config.h) adds LED2 alongside
Tutorial 01's LED1, same pattern, same file:

```c
#define TIREMO_BOARD_LED1_PORT  TIREMO_HAL_GPIO_PORT_B
#define TIREMO_BOARD_LED1_PIN   TIREMO_HAL_GPIO_PIN_4
#define TIREMO_BOARD_LED2_PORT  TIREMO_HAL_GPIO_PORT_B
#define TIREMO_BOARD_LED2_PIN   TIREMO_HAL_GPIO_PIN_9
```

### 2. Entry point — unchanged from Tutorial 01

[`Sw/Src/main.c`](./Sw/Src/main.c) is byte-for-byte the same shape as
Tutorial 01's: `PRV_PORT_Init()`, `PRV_SCU_CLK_Init()`, `PRV_USER_Code()`.
SysTick bring-up happens inside `PRV_USER_Code()` instead of here — it's
this tutorial's new application behavior, not a change to board-level
boot sequencing, so `main.c` has no reason to change yet. (It will, once
a later tutorial genuinely needs to change what boots before the
application does — Tutorial 03 adds UART init here, for instance.)

### 3. Two independent timers, one loop

[`Sw/Src/prv_user_code.c`](./Sw/Src/prv_user_code.c) is the whole program:

```c
#define LED1_INTERVAL_MS (1000U)  /* -> 2 s full blink cycle */
#define LED2_INTERVAL_MS (5000U)  /* -> 10 s full cycle -- deliberately much slower than LED1's */

void PRV_USER_Code(void)
{
    uint32_t led1LastMs, led2LastMs, now;

    TIREMO_SysTick_Init();
    /* ... set both LEDs as outputs, both start off ... */

    led1LastMs = TIREMO_SysTick_GetMs();
    led2LastMs = led1LastMs;

    for (;;)
    {
        now = TIREMO_SysTick_GetMs();

        if ((now - led1LastMs) >= LED1_INTERVAL_MS)
        {
            TIREMO_HAL_GPIO_TogglePin(TIREMO_BOARD_LED1_PORT, TIREMO_BOARD_LED1_PIN);
            led1LastMs = now;
        }
        if ((now - led2LastMs) >= LED2_INTERVAL_MS)
        {
            TIREMO_HAL_GPIO_TogglePin(TIREMO_BOARD_LED2_PORT, TIREMO_BOARD_LED2_PIN);
            led2LastMs = now;
        }
    }
}
```

Neither `if` ever blocks, so the loop is free to keep spinning through
both checks thousands of times a second — this is the shape every
tutorial from here on builds on: one `for (;;)`, several independent
timed things inside it, nothing waiting on anything else.

### 4. Build — no new vendor sources this time

[`Sw/CMakeLists.txt`](./Sw/CMakeLists.txt) is Tutorial 01's `CMakeLists.txt`
plus two lines added to `add_executable()` —
`Tiremo_Hal/Src/tiremo_hal_systick.c` and `Common/Src/tiremo_systick.c`
— and a `Common/Include` added to `target_include_directories()`. The
vendor file list doesn't change at all: SysTick is a core Cortex-M4
peripheral reached through CMSIS, not an ABOV HAL module, so no vendor
`.c` files needed adding. Worth contrasting with Tutorial 01's build
write-up, where getting the vendor source list right (and working around
a real Make bug) was most of the difficulty — here it was the easy part.

### 5. Flash

```sh
tdf.py build
tdf.py flash
```

Same OS-detection, same bundled-OpenOCD flow as Tutorial 01, just
reached through `tdf.py` instead of typing the full `cmake --build
build --target flash` — see the **Solution** section above for
confirmation this actually reached real hardware.

## Done When

Two LEDs blink at independent, visibly different rates from a single non-blocking loop, with no debugger attached. **Confirmed** — see Solution above.

<p align="center"><img alt="" src="../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
