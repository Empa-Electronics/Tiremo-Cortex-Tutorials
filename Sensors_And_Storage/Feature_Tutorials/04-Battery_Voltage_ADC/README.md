<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="220">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Battery Voltage via ADC</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Battery Voltage via ADC

***Board: Tiremo Cortex (ESP32) · Section: Sensors & Local Storage / Feature Tutorials · Difficulty: Medium***

## Introduction

There's no dedicated battery-sense pin on this board — supply voltage is inferred by measuring the internal bandgap reference against AVDD. It's a good introduction to the `Tiremo_Hal` ADC wrapper before the microphone tutorial needs the same peripheral in burst mode.

## You Will Learn

- The `Tiremo_Hal_ADC` wrapper
- Deriving AVDD from a known bandgap reference voltage and a raw ADC code

## Prerequisites

- [03-LIS2DE12_Accelerometer](../03-LIS2DE12_Accelerometer/)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Reference Implementation

This tutorial is one slice of the reference firmware kept in this repo.
Read these before or after attempting the steps below to check your
approach:

- [tiremo_battery.c](../../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Drivers/battery/Src/tiremo_battery.c)
- [tiremo_hal_adc.c](../../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Tiremo_Hal/Src/tiremo_hal_adc.c)

## Step Outline

1. Sample the VCORE bandgap channel (`TIREMO_BOARD_BATTERY_VCORE_CHANNEL`)
2. Derive AVDD in mV from the raw code
3. Log both raw code and computed mV, and sanity-check against a multimeter

## Done When

Computed AVDD is within ~50 mV of a multimeter reading on the same supply rail.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
