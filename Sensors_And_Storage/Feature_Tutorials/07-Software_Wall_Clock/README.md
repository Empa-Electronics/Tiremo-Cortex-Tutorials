<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <h1>Software Wall Clock</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Software Wall Clock

***Board: Tiremo Cortex (ESP32) · Section: Sensors & Local Storage / Feature Tutorials · Difficulty: Medium***

## Introduction

The A34G43x has no hardware RTC, so wall-clock time is reconstructed from an NTP-provided epoch plus the free-running SysTick counter. This tutorial builds that pattern standalone; ESP32 Connectivity will feed it a real NTP sync.

## You Will Learn

- Why `epoch_at_sync + (SysTick_now - SysTick_at_sync)` is enough without a battery-backed RTC
- The tradeoff: this clock resets to zero on every reboot until re-synced

## Prerequisites

- [06-Flash_Backed_Config](../06-Flash_Backed_Config/)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Reference Implementation

This tutorial is one slice of the reference firmware kept in this repo.
Read these before or after attempting the steps below to check your
approach:

- [tiremo_clock.c](../../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Common/Src/tiremo_clock.c)

## Step Outline

1. Implement `SetEpoch(epochSeconds)` capturing the current SysTick alongside it
2. Implement `GetEpoch()` computing current wall time from the offset
3. Manually call `SetEpoch()` with a known value and confirm `GetEpoch()` tracks real elapsed time

## Done When

After calling `SetEpoch()` once, `GetEpoch()` advances in lockstep with a real clock for several minutes with no additional sync.

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
