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
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Software Wall Clock

***Board: Tiremo Cortex (ESP32) · Section: Sensors & Local Storage / Feature Tutorials · Difficulty: Medium***

## Introduction

The A34G43x has no hardware RTC. Wall-clock time is `epoch_at_sync + (SysTick_now - SysTick_at_sync) / 1000`. This tutorial plants a known epoch by hand; ESP32 Connectivity will later feed a real NTP sample into the same API.

## You Will Learn

- Why that offset is enough without a battery-backed RTC
- The tradeoff: RAM-only — every reboot starts unsynced until `SetEpoch()` runs again
- [`Sw/Common/Src/tiremo_clock.c`](./Sw/Common/Src/tiremo_clock.c) is copied from the reference firmware

## Prerequisites

- [06-Flash_Backed_Config](../06-Flash_Backed_Config/) conceptually; this `Sw/` is SysTick + UART only

## Understanding the offset

```c
void TIREMO_Clock_SetEpoch(uint32_t epochSeconds)
{
    s_epochAtSync = epochSeconds;
    s_msAtSync = TIREMO_SysTick_GetMs();
    s_synced = true;
}

*pEpochSeconds = s_epochAtSync + ((TIREMO_SysTick_GetMs() - s_msAtSync) / 1000U);
```

No NTP here. [`prv_user_code.c`](./Sw/Src/prv_user_code.c) calls `SetEpoch(1704067200)` (2024-01-01 00:00:00 UTC) once at boot.

## Solution

```sh
cd Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

**Verified:** `.text` 12344 bytes, flash **Verified OK**. Consecutive seconds:

```text
[INFO] clock: epoch=1704067201  elapsed=1 s
[INFO] clock: epoch=1704067202  elapsed=2 s
[INFO] clock: epoch=1704067203  elapsed=3 s
```

`elapsed` tracks real time one-for-one over the capture window. Multi-minute lockstep is the same math; not soaked for minutes this session.

## Done When

After one `SetEpoch()`, `GetEpoch()` advances with a real clock — **confirmed at 1 Hz** on hardware.


<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
