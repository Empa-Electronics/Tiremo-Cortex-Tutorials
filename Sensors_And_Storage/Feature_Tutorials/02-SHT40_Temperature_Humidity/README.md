<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="220">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>SHT40 Temperature / Humidity</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# SHT40 Temperature / Humidity

***Board: Tiremo Cortex (ESP32) · Section: Sensors & Local Storage / Feature Tutorials · Difficulty: Easy***

## Introduction

SHT40 is the first real sensor driver in the stack, and the simplest — a single I2C transaction returns both temperature and humidity. It's also the sensor the shadow schema (AWS IoT Core) will later tune via `unit` and `offset`.

## You Will Learn

- The Sensirion SHT4x measurement command/response protocol
- Converting raw ticks to milli-°C / milli-%RH

## Prerequisites

- [01-I2C_Bus_Bringup](../01-I2C_Bus_Bringup/)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Send the SHT40 measurement command and read back 6 bytes with CRC
2. Convert to milli-°C / milli-%RH per the Sensirion formula
3. Log a reading once per second

## Done When

UART prints plausible room temperature/humidity, stable within sensor noise across repeated reads.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
