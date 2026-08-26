<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%" height="16">
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

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Implement `SetEpoch(epochSeconds)` capturing the current SysTick alongside it
2. Implement `GetEpoch()` computing current wall time from the offset
3. Manually call `SetEpoch()` with a known value and confirm `GetEpoch()` tracks real elapsed time

## Done When

After calling `SetEpoch()` once, `GetEpoch()` advances in lockstep with a real clock for several minutes with no additional sync.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%" height="16"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
