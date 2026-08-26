<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="65%">
      <h1>Microphone: RMS Capture to dB</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Microphone: RMS Capture to dB

***Board: Tiremo Cortex (ESP32) · Section: Sensors & Local Storage / Feature Tutorials · Difficulty: Hard***

## Introduction

This is the hardest local-sensor tutorial: burst-mode ADC clocked by Timer1, captured via DMA, then DC-offset removal and an RMS→dB conversion. It's also the first tutorial that touches vendor HAL types directly instead of `Tiremo_Hal`.

## You Will Learn

- Why this driver bypasses `Tiremo_Hal` and talks to vendor `hal_adc`/`hal_timer1`/`hal_pcu` directly
- Timer-triggered ADC burst capture into a DMA buffer
- DC-offset removal and RMS→dB (`20·log10(rms)`, relative not calibrated SPL)

## Prerequisites

- [04-Battery_Voltage_ADC](../04-Battery_Voltage_ADC/); `CONFIG_HAL_DMAC` must be enabled in the vendor module config
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Enable `CONFIG_HAL_DMAC` and wire Timer1 to trigger ADC0 bursts on PA0
2. Capture a burst window into a DMA buffer and compute a DC-offset-corrected RMS
3. Convert RMS to an approximate dB value and log it while talking near the mic vs. silent

## Done When

dB reading rises measurably when you talk near the mic and settles to a quiet baseline in silence.

<p align="center"><img alt="" src="../../../images/banner.png" width="65%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
