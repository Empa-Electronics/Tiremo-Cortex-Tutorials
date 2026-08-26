<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%" height="16">
      <h1>Standalone Environmental Logger</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Standalone Environmental Logger

***Board: Tiremo Cortex (ESP32) · Section: Sensors & Local Storage / Design Tutorials · Difficulty: Medium***

## Introduction

Before adding a single line of networking code, prove the full sensor stack works together: SHT40 + LIS2DE12 + battery + mic sampled on a schedule, with the last N readings persisted to flash and dumped over UART on boot.

## You Will Learn

- Composing multiple drivers into one sample loop without a cloud dependency
- Batch-storing readings to flash and replaying them on boot

## Prerequisites

- All of Sensors & Local Storage / Feature Tutorials (01–07)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Sample all four sensors on a fixed interval using the shared SysTick loop
2. Store the last N samples as a ring buffer in flash
3. On boot, dump stored history over UART before resuming live sampling

## Done When

Power-cycling mid-run does not lose the last several samples; UART shows them replayed on the next boot.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%" height="16"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
