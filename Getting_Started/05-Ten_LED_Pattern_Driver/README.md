<p class="sphinxhide" align="right"><a>English</a> | <a href="../../docs-tr/Getting_Started/05-Ten_LED_Pattern_Driver/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../images/banner.png" width="100%">
      <h1>Ten-LED Pattern Driver</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Ten-LED Pattern Driver

***Board: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Section: Getting Started · Difficulty: Easy***

## Introduction

The production firmware selects LED patterns from shadow state (cloud-driven), but the pattern engine itself — solid/blink/pulse timing across all ten onboard LEDs — doesn't need the cloud to be built and tested. This tutorial builds it standalone: all ten LEDs cycle through the same three patterns automatically, staggered so each one is visibly independent, and a debounced USER button (Tutorial 04) advances the cycle manually.

## You Will Learn

- Driving all ten onboard LEDs from one module (`Drivers/led`), not just the two used since Tutorial 02
- Why the reference LED driver only has raw on/off/toggle, and why the solid/blink/pulse pattern engine has no reference implementation to copy — it's original code, layered on top
- A non-blocking, per-LED state machine with ten independent timers running off one shared `TIREMO_SysTick_GetMs()`
- Where new, generic firmware logic belongs: `Common/`, not application code — contrasted directly with Tutorial 04's debounce logic, which stayed in `prv_user_code.c` because it was specific to one caller
- Combining Tutorial 04's debounced button with this tutorial's pattern engine: a button press driving application state, not just producing a log line

## Prerequisites

- [04-Button_Read_And_Debounce](../04-Button_Read_And_Debounce/) — this tutorial reuses its debounce state machine and its `Drivers/button` verbatim
- `arm-none-eabi-gcc`, `cmake` (3.20+), and `ninja` on `PATH` (see [01-Hello_Blink's Prerequisites](../01-Hello_Blink/#prerequisites))
- To watch the scene changes in the log: `tdf monitor` (see [04-Button_Read_And_Debounce's Solution](../04-Button_Read_And_Debounce/#solution) for setup — needs `pyserial`, via the `tdf_venv/` that `./install.sh` / `install.ps1` / `install.bat` creates), or any serial terminal (Tera Term, PuTTY, screen, minicom, …) at 115200 8N1 on the board's USB serial port. The LEDs themselves work without any terminal.
- A Tiremo Cortex board (ABOV A34G43A MCU — see [Hardware Overview](../../Hardware.md)) connected to your PC with a USB cable, with all ten LEDs and the USER button accessible. The board's onboard CMSIS-DAP debug probe handles SWD flashing; no separate programmer is needed.

## Understanding the Pattern Engine (There's No Reference to Copy From)

Every prior tutorial's new module started from a real file in the reference firmware, trimmed or copied verbatim. This one's different. [`Sw/Drivers/led/Src/tiremo_led.c`](./Sw/Drivers/led/Src/tiremo_led.c) — copied verbatim, same as every other `Drivers/` module so far — only has `TIREMO_LED_On/Off/Toggle/SetState/GetState/AllOn/AllOff`. Raw, immediate, one-shot calls. There is no `tiremo_led_pattern.c` anywhere in the reference firmware; the production system's LED patterns are driven by higher-level shadow state (cloud-side, out of scope for this tutorial and not covered here), so the timing layer that turns "blink at 2 Hz" into a running timer is built from scratch in this tutorial.

So [`Sw/Common/Src/tiremo_led_pattern.c`](./Sw/Common/Src/tiremo_led_pattern.c) is original: a per-LED state machine — pattern type, interval, last-change timestamp, current on/off phase — serviced non-blockingly every loop tick, the same `(now - last) >= interval` idiom every tutorial since 02-SysTick_Timing has used, just ten independent copies of it running at once instead of one or two.

**Where it lives, and why that's a real decision, not an arbitrary one:** Tutorial 04's button-debounce state machine stayed inside `prv_user_code.c` — application code — because it was specific to what that one tutorial's loop needed. This pattern engine goes in `Common/` instead, next to `tiremo_debug.c` and `tiremo_systick.c`, because it's generic: any future caller that wants an LED blinking at some rate can call `TIREMO_LEDPattern_Set()` without knowing or caring what's currently running on that LED, the same way any caller can reach for `TIREMO_LOG_INFO()` without knowing anything about the UART underneath it. `Common/` is the deciding factor here — reusable service layer over a hardware driver, versus one-off logic for a single caller.

**No PWM, so "pulse" isn't a fade.** `TIREMO_HAL_GPIO` is digital-only — there's no dimming primitive to build a real fade in/out on top of. `PULSE` here means a short bright burst (20% of the period) followed by a long rest, distinct from `BLINK`'s even 50/50 split — a duty-cycle heartbeat, not a brightness ramp. Worth knowing before assuming "pulse" means what it would on hardware with PWM-capable LED channels.

## Solution

[`Sw/`](./Sw/) has a real, buildable implementation. From the repo root:

```sh
cd Getting_Started/05-Ten_LED_Pattern_Driver/Sw
tdf build
tdf flash
tdf monitor
```

(See [01-Hello_Blink's Solution](../01-Hello_Blink/#solution) for what
`tdf` is, and [04-Button_Read_And_Debounce's Solution](../04-Button_Read_And_Debounce/#solution)
for `monitor` specifically. On macOS/Linux, `tdf.py` also works as an
alias; on Windows, use `tdf`. Close any other terminal on the board's
serial port before running `tdf monitor` — only one program can open
it at a time.)

Same shared vendor SDK and CMake build shape as every prior tutorial. No
new vendor sources needed — all ten LEDs are pure GPIO, already covered
by `hal_pcu.c` since Tutorial 01 — so the new build inputs are this
tutorial's own `Drivers/led/Src/tiremo_led.c` (copied) and
`Common/Src/tiremo_led_pattern.c` (original), added to `CMakeLists.txt`
alongside Tutorial 04's button sources.

**Confirmed on real hardware:** `tdf build` compiles and links without
errors on macOS and Windows 11 (`.text` about 13.5 KB; the exact size
varies slightly with the toolchain version, and the linker prints the
same expected, harmless `_close` / `_lseek` / `_read` / `_write is not
implemented` warnings explained in [01-Hello_Blink's Solution](../01-Hello_Blink/#solution)).
`tdf flash` completed a full program-and-verify cycle over SWD on a
real, connected Tiremo Cortex board (`** Verified OK **`), and the log
from a fresh boot looks like this:

```text
[INFO] Ten_LED_Pattern_Driver: boot
[INFO] scene -> SOLID @ 3 ms
[INFO] scene -> BLINK (auto) @ 8003 ms
[INFO] scene -> PULSE (auto) @ 16003 ms
[INFO] scene -> SOLID (auto) @ 24003 ms
```

(After PULSE the cycle wraps back to SOLID and keeps repeating every 8 s.)

The timestamps show exactly the designed 8000 ms `SCENE_INTERVAL_MS`
between each auto-advance. **What you should see on the board:** all
ten LEDs lit during SOLID; during BLINK and PULSE they cascade at ten
different rates (LED_1 fastest with a 300 ms period, LED_10 slowest at
1650 ms). Press the USER button at any point and the board jumps to the
next scene immediately: you get a yellow `[WARN] USER button: pressed`
line followed by a yellow `[WARN] scene -> ... (button) @ ... ms` line,
and the 8-second auto-advance timer restarts from that moment.

## Walkthrough

### 1. Board config — the rest of the LED bank

[`Sw/Board/board_config.h`](./Sw/Board/board_config.h) keeps Tutorial
04's LED1/LED2/button macros and adds `TIREMO_BOARD_LED3_PORT`/`PIN`
through `LED10`, plus `TIREMO_BOARD_LED_COUNT` (10), copied from the
reference `board_config.h`. `Drivers/led/Src/tiremo_led.c` indexes
straight off this count, so getting it right is what makes
`TIREMO_LED_Init()` bring up all ten pins instead of silently only two.

### 2. The pattern engine

Covered in full above — see **Understanding the Pattern Engine**. The
short version, its public API:

```c
void TIREMO_LEDPattern_Init(void);
void TIREMO_LEDPattern_Set(TIREMO_LED_Id_t ledId, TIREMO_LEDPattern_Type_t pattern, uint32_t intervalMs, uint32_t now);
void TIREMO_LEDPattern_Service(uint32_t now);
```

`Set()` takes effect immediately (OFF/SOLID write the pin right away;
BLINK/PULSE start their first phase timed from `now`, avoiding an
initial-tick sentinel hack). `Service()` walks all ten LEDs every loop
iteration; OFF/SOLID cost nothing there, only BLINK/PULSE do per-tick
timing work.

### 3. The demo: staggered scenes, auto and manual advance

[`Sw/Src/prv_user_code.c`](./Sw/Src/prv_user_code.c) retires Tutorial
02/03/04's dedicated LED1/LED2 loop — they're just `TIREMO_LED_1`/`_2`
among all ten now. `ApplyScene()` sets every LED to the same pattern
type but a different interval:

```c
for (index = 0U; index < TIREMO_BOARD_LED_COUNT; index++)
{
    intervalMs = PATTERN_BASE_INTERVAL_MS + (index * PATTERN_STEP_MS); /* 300, 450, ..., 1650 */
    TIREMO_LEDPattern_Set((TIREMO_LED_Id_t)index, pattern, intervalMs, now);
}
```

so BLINK/PULSE scenes visibly cascade across the board instead of all
ten LEDs changing in perfect unison — the point being demonstrated is
that they're *independently addressable*, and a synchronized blink
wouldn't look any different from one LED wired ten times over. The main
loop auto-advances the scene every `SCENE_INTERVAL_MS` (8000 ms), and
separately, `PollButton()` — Tutorial 04's exact debounce state
machine, now returning `true` on the tick it detects a fresh debounced
press — advances it immediately on a real button press, resetting the
auto-advance timer either way:

```c
if ((now - sceneLastMs) >= SCENE_INTERVAL_MS)         { /* advance, log "(auto)" */ }
if (PollButton(now))                                    { /* advance, log "(button)" */ }
```

Unlike Tutorials 02–04, individual LED toggles aren't logged here — with
ten independently-timed LEDs running at once, per-toggle logging would
flood the UART and defeat the point of watching scene changes. Only
scene transitions are logged, each with its trigger (`auto` or
`button`) and a millisecond timestamp.

### 4. Build, flash, and watch

```sh
tdf build
tdf flash
tdf monitor
```

(Or chained in one command: `tdf build flash monitor`.) Same
OS-detection, same bundled-OpenOCD flow as every prior tutorial — see
[01-Hello_Blink's Solution](../01-Hello_Blink/#solution). See
**Solution** above for what the output should look like.

## Done When

`tdf monitor` shows `scene -> SOLID`, then `scene -> BLINK (auto)`,
then `scene -> PULSE (auto)` at 8-second intervals, then wraps back to
`SOLID (auto)`. All ten LEDs are visibly on (SOLID), then cascading at
ten different blink rates (BLINK), then cascading as short heartbeat
pulses (PULSE), and a USER button press at any point jumps to the next
scene immediately, logged as `(button)` instead of `(auto)`.

<p align="center"><img alt="" src="../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
