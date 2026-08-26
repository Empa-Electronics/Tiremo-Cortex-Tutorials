<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="220">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Flash-Backed Config</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Flash-Backed Config

***Board: Tiremo Cortex (ESP32) · Section: Sensors & Local Storage / Feature Tutorials · Difficulty: Medium***

## Introduction

WiFi credentials and sensor enable flags need to survive a power cycle. This tutorial builds the generic persistence mechanism — a magic-tagged, CRC32-checked record in data flash — that later tutorials (WiFi credentials, factory reset) both depend on.

## You Will Learn

- The `Tiremo_Hal_Flash` wrapper and why `CONFIG_HAL_FMC` must be enabled
- A magic-number + CRC32 record format for safe load/corrupt-detection

## Prerequisites

- [05-Microphone_RMS_To_dB](../05-Microphone_RMS_To_dB/)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Reference Implementation

This tutorial is one slice of the reference firmware kept in this repo.
Read these before or after attempting the steps below to check your
approach:

- [tiremo_hal_flash.c](../../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Tiremo_Hal/Src/tiremo_hal_flash.c)
- [tiremo_app_config.c](../../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Tiremo_Core/Tiremo_App/Src/tiremo_app_config.c)

## Step Outline

1. Define a small config struct (e.g. a counter and a string field)
2. Write it to a data-flash sector with a magic number and CRC32 trailer
3. On boot, load it back, validate the magic/CRC, and fall back to defaults on mismatch
4. Power-cycle the board and confirm the counter persisted

## Done When

A value written before a power cycle reads back correctly after it; a deliberately corrupted record falls back to defaults instead of garbage.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
