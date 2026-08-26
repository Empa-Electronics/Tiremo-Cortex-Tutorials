<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="65%">
      <h1>LIS2DE12 Accelerometer</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# LIS2DE12 Accelerometer

***Board: Tiremo Cortex (ESP32) · Section: Sensors & Local Storage / Feature Tutorials · Difficulty: Medium***

## Introduction

Unlike SHT40, LIS2DE12 has real configuration surface — full-scale range and output data rate both matter for later fall/shock/vibration alarms, so build the driver to take those as parameters from day one.

## You Will Learn

- LIS2DE12 register map: CTRL_REG1 (ODR), CTRL_REG4 (scale), OUT_X/Y/Z_H
- Converting raw counts to mg using the selected full-scale sensitivity

## Prerequisites

- [02-SHT40_Temperature_Humidity](../02-SHT40_Temperature_Humidity/)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Configure ODR and full-scale via a `Configure(scale_g, data_rate_hz)` call
2. Read and convert X/Y/Z to signed mg
3. Verify by tilting the board and watching gravity move between axes

## Done When

Tilting the board on each axis in turn shows ~1000 mg on that axis and near-zero on the others.

<p align="center"><img alt="" src="../../../images/banner.png" width="65%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
