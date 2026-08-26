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
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Ten-LED Pattern Driver

***Board: Tiremo Cortex (ESP32) · Section: Getting Started / Getting Started · Difficulty: Easy***

## Introduction

The production firmware selects LED patterns from shadow state (cloud-driven), but the pattern engine itself — solid/blink/pulse timing across all ten onboard LEDs — doesn't need the cloud to be built and tested. This tutorial builds it standalone: all ten LEDs cycle through the same three patterns automatically, staggered so each one is visibly independent, and a debounced USER button (Tutorial 04) advances the cycle manually.

## You Will Learn

- Driving all ten onboard LEDs from one module (`Drivers/led`), not just the two used since Tutorial 02
- Why the reference LED driver only has raw on/off/toggle, and why the solid/blink/pulse pattern engine has no reference implementation to copy — it's original code, layered on top
- A non-blocking, per-LED state machine with ten independent timers running off one shared `TIREMO_SysTick_GetMs()`
- Where new, generic firmware logic belongs: `Common/`, not application code — contrasted directly with Tutorial 04's debounce logic, which stayed in `prv_user_code.c` because it was specific to one caller
- Combining Tutorials 04 and 05: a debounced button press driving application state, not just producing a log line

## Prerequisites

- [04-Button_Read_And_Debounce](../04-Button_Read_And_Debounce/) — this tutorial reuses its debounce state machine and its `Drivers/button` verbatim
- `arm-none-eabi-gcc`, `cmake` (3.20+), and `ninja` on `PATH` (see [01-Hello_Blink's Prerequisites](../01-Hello_Blink/#prerequisites))
- `tdf.py monitor` to watch the scene changes (see [04-Button_Read_And_Debounce's Solution](../04-Button_Read_And_Debounce/#solution) for setup — needs `pyserial`, via `./install.sh`'s `tdf_venv/`)
- An ABOV A34G43A Starter Kit connected over its onboard SWD debugger, with all ten LEDs and the USER button accessible

## Understanding the Pattern Engine (There's No Reference to Copy From)

Every prior tutorial's new module started from a real file in the reference firmware, trimmed or copied verbatim. This one's different. [`Sw/Drivers/led/Src/tiremo_led.c`](./Sw/Drivers/led/Src/tiremo_led.c) — copied verbatim, same as every other `Drivers/` module so far — only has `TIREMO_LED_On/Off/Toggle/SetState/GetState/AllOn/AllOff`. Raw, immediate, one-shot calls. There is no `tiremo_led_pattern.c` anywhere in the reference firmware; the production system's LED patterns are driven by higher-level shadow state (cloud-side, out of scope for this tutorial and not covered here), and whatever translates "blink at 2 Hz" into a running timer isn't in the code this repo has access to.

So [`Sw/Common/Src/tiremo_led_pattern.c`](./Sw/Common/Src/tiremo_led_pattern.c) is original: a per-LED state machine — pattern type, interval, last-change timestamp, current on/off phase — serviced non-blockingly every loop tick, the same `(now - last) >= interval` idiom every tutorial since 02-SysTick_Timing has used, just ten independent copies of it running at once instead of one or two.

**Where it lives, and why that's a real decision, not an arbitrary one:** Tutorial 04's button-debounce state machine stayed inside `prv_user_code.c` — application code — because it was specific to what that one tutorial's loop needed. This pattern engine goes in `Common/` instead, next to `tiremo_debug.c` and `tiremo_systick.c`, because it's generic: any future caller that wants an LED blinking at some rate can call `TIREMO_LEDPattern_Set()` without knowing or caring what's currently running on that LED, the same way any caller can reach for `TIREMO_LOG_INFO()` without knowing anything about the UART underneath it. `Common/` is the deciding factor here — reusable service layer over a hardware driver, versus one-off logic for a single caller.

**No PWM, so "pulse" isn't a fade.** `TIREMO_HAL_GPIO` is digital-only — there's no dimming primitive to build a real fade in/out on top of. `PULSE` here means a short bright burst (20% of the period) followed by a long rest, distinct from `BLINK`'s even 50/50 split — a duty-cycle heartbeat, not a brightness ramp. Worth knowing before assuming "pulse" means what it would on hardware with PWM-capable LED channels.

## Solution

[`Sw/`](./Sw/) has a real, buildable implementation:

```sh
cd Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

(See [01-Hello_Blink's Solution](../01-Hello_Blink/#solution) for what
`tdf.py` is, and [04-Button_Read_And_Debounce's Solution](../04-Button_Read_And_Debounce/#solution)
for `monitor` specifically.)

Same shared vendor SDK and CMake build shape as every prior tutorial. No
new vendor sources needed — all ten LEDs are pure GPIO, already covered
by `hal_pcu.c` since Tutorial 01 — so the new build inputs are this
tutorial's own `Drivers/led/Src/tiremo_led.c` (copied) and
`Common/Src/tiremo_led_pattern.c` (original), added to `CMakeLists.txt`
alongside Tutorial 04's button sources.

**Genuinely verified, for real, on the connected board:** `tdf.py build`
compiles and links clean (`.text` 13796 bytes). `tdf.py flash` completed
a full program-and-verify cycle over SWD (`** Verified OK **`). Then
`tdf.py monitor`, reset first, captured a clean boot-to-live log:

```text
[INFO] scene -> SOLID @ 3 ms
[INFO] scene -> BLINK (auto) @ 8003 ms
[INFO] scene -> PULSE (auto) @ 16003 ms
```

Exactly the designed 8000 ms `SCENE_INTERVAL_MS` between each auto-advance,
confirmed from real hardware timestamps, not assumed. **What's genuinely
unconfirmed:** whether all ten LEDs are actually visibly staggered and
correctly wired (I can't see the physical board), and whether a real
USER button press advances the scene early and logs `(button)` instead
of `(auto)` — that needs a human finger, same limitation as Tutorial 04.
If you're checking this yourself: `tdf.py monitor`, watch all ten LEDs
cascade during the BLINK and PULSE scenes (LED_1 fastest at 300 ms,
LED_10 slowest at 1650 ms), and press the USER button to confirm it
jumps to the next scene immediately, logged as `scene -> ... (button)`.

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
tdf.py build
tdf.py flash
tdf.py monitor
```

Same OS-detection, same bundled-OpenOCD flow as every prior tutorial —
see [01-Hello_Blink's Solution](../01-Hello_Blink/#solution). See
**Solution** above for what's actually confirmed on real hardware here.

## Done When

`tdf.py monitor` shows `scene -> SOLID`, then `scene -> BLINK (auto)`,
then `scene -> PULSE (auto)` at 8-second intervals — genuinely
confirmed, see **Solution**. All ten LEDs should be visibly on
(SOLID), then cascading at ten different blink rates (BLINK), then
cascading as short heartbeat pulses (PULSE), and a USER button press at
any point should jump to the next scene immediately, logged as
`(button)` instead of `(auto)`. The visual/physical parts are not yet
confirmed on a real board by me — see the honesty note in **Solution**.

<p align="center"><img alt="" src="../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
