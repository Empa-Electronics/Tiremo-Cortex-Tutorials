<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/Sensors_And_Storage/Feature_Tutorials/07-Software_Wall_Clock/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Software Wall Clock</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Software Wall Clock

***Board: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Section: Sensors & Local Storage / Feature Tutorials · Difficulty: Medium***

## Introduction

The A34G43A on this board has no battery-backed hardware RTC, so wall-clock time is computed in software: `epoch_at_sync + (SysTick_now - SysTick_at_sync) / 1000`. This tutorial plants a known epoch by hand; the [NTP Time Sync](../../../ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/) tutorial later feeds a real NTP time into the same API.

## You Will Learn

- How a sync point plus the millisecond SysTick counter gives you a running wall clock
- The tradeoffs of a software clock: it's RAM-only (every reboot starts unsynced), it drifts with the CPU clock, and the 32-bit millisecond counter wraps — so it needs to be re-synced regularly
- [`Sw/Common/Src/tiremo_clock.c`](./Sw/Common/Src/tiremo_clock.c) is copied from the reference firmware

## Prerequisites

- [06-Flash_Backed_Config](../06-Flash_Backed_Config/) conceptually; this `Sw/` is SysTick + UART only
- To watch the log: `tdf monitor` (see [Getting_Started/04-Button_Read_And_Debounce's Solution](../../../Getting_Started/04-Button_Read_And_Debounce/#solution) for setup) or any serial terminal (Tera Term, PuTTY, screen, minicom, …) at 115200 8N1 on the board's USB serial port
- A Tiremo Cortex board (ABOV A34G43A MCU — see [Hardware Overview](../../../Hardware.md)) connected to your PC with a USB cable. The board's onboard CMSIS-DAP debug probe handles SWD flashing; no separate programmer is needed.

## Understanding the offset

[`tiremo_clock.c`](./Sw/Common/Src/tiremo_clock.c) stores one sync point and adds the time elapsed since then:

```c
void TIREMO_Clock_SetEpoch(uint32_t epochSeconds)
{
    s_epochAtSync = epochSeconds;
    s_msAtSync = TIREMO_SysTick_GetMs();
    s_synced = true;
}

/* inside TIREMO_Clock_GetEpoch(): */
elapsedMs = TIREMO_SysTick_GetMs() - s_msAtSync;
*pEpochSeconds = s_epochAtSync + (elapsedMs / 1000U);
```

`GetEpoch()` returns `false` until `SetEpoch()` has been called once. No NTP here: [`prv_user_code.c`](./Sw/Src/prv_user_code.c) calls `SetEpoch(1704067200)` (2024-01-01 00:00:00 UTC) once at boot, then logs `GetEpoch()` every second.

## Limits of a software clock

This approach is simple and good enough for timestamps, as long as you keep two limits in mind:

- **Drift.** The CPU clock (and therefore SysTick) comes from the PLL fed by the chip's **internal RC oscillator (HSI)** — this board has no crystal on XIN/XOUT. An RC oscillator is much less accurate than a crystal and changes with temperature, so the clock slowly drifts away from real time. For scale: an error of 0.5 % is about 7 minutes per day. Re-sync from NTP periodically instead of relying on one sync.
- **Counter wrap.** `TIREMO_SysTick_GetMs()` is a 32-bit millisecond counter that wraps after 2³² ms ≈ **49.7 days**. The unsigned subtraction handles a single wrap correctly, but if more than 49.7 days pass since the last `SetEpoch()`, the elapsed time wraps back to zero and the clock jumps back. Re-syncing well inside that window (e.g. daily) avoids both problems.

## Solution

From the repo root:

```sh
cd Sensors_And_Storage/Feature_Tutorials/07-Software_Wall_Clock/Sw
tdf build
tdf flash
tdf monitor
```

(See [Getting_Started/01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution) for what `tdf` is. On macOS/Linux, `tdf.py` also works as an alias; on Windows, use `tdf`. Close any other terminal on the board's serial port before running `tdf monitor` — only one program can open it at a time.)

**Confirmed on real hardware:** `tdf build` compiles and links without errors on macOS and Windows 11 (`.text` about 12 KB; the exact size varies slightly with the toolchain version, and the linker prints the same expected, harmless `_close` / `_lseek` / `_read` / `_write is not implemented` warnings explained in [01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution)). `tdf flash` completed a full program-and-verify cycle over SWD on a real, connected Tiremo Cortex board (`** Verified OK **`). The log:

```text
[INFO] Software_Wall_Clock: boot
[INFO] clock: SetEpoch(1704067200)
[INFO] clock: epoch=1704067201  elapsed=1 s
[INFO] clock: epoch=1704067202  elapsed=2 s
[INFO] clock: epoch=1704067203  elapsed=3 s
```

with one new line per second; `elapsed` is the number of seconds since `SetEpoch()`. If the clock were read before any sync, the log would show `[WARN] clock: not synced` instead.

## Walkthrough

### 1. `tiremo_clock.c`

[`Sw/Common/Src/tiremo_clock.c`](./Sw/Common/Src/tiremo_clock.c): `TIREMO_Clock_SetEpoch()`, `TIREMO_Clock_GetEpoch()` and `TIREMO_Clock_IsSynced()` — three functions and three static variables, built on `TIREMO_SysTick_GetMs()` from [Getting_Started/02-SysTick_Timing](../../../Getting_Started/02-SysTick_Timing/).

### 2. `prv_user_code.c`

[`Sw/Src/prv_user_code.c`](./Sw/Src/prv_user_code.c): `SetEpoch(1704067200)` once at boot, then a 1 s non-blocking loop that logs `epoch` and `elapsed`.

### 3. Build, flash, and watch

```sh
tdf build
tdf flash
tdf monitor
```

(Or chained in one command: `tdf build flash monitor`.) See **Solution** above for what the output should look like.

## Done When

After the `SetEpoch(1704067200)` line, `tdf monitor` shows `epoch` going up by one every second and `elapsed` counting 1, 2, 3, … — and you can explain why a real product must re-sync this clock periodically (drift and the 49.7-day wrap).


<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
