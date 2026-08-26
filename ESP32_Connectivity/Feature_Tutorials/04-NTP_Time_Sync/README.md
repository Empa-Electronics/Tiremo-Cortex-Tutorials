<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="220">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>NTP Time Sync</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# NTP Time Sync

***Board: Tiremo Cortex (ESP32) · Section: ESP32 Connectivity / Feature Tutorials · Difficulty: Medium***

## Introduction

Now that the board can join WiFi, get real wall-clock time and plug it into the software clock built in Sensors & Local Storage — every timestamped MQTT message downstream depends on this being correct.

## You Will Learn

- `AT+CIPSNTPCFG` / `AT+CIPSNTPTIME?` and parsing the asctime response
- Feeding a parsed epoch into `TIREMO_Clock_SetEpoch()`

## Prerequisites

- [03-Connect_Retry_State_Machine](../03-Connect_Retry_State_Machine/), and [Software_Wall_Clock](../../../Sensors_And_Storage/Feature_Tutorials/07-Software_Wall_Clock/)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Reference Implementation

This tutorial is one slice of the reference firmware kept in this repo.
Read these before or after attempting the steps below to check your
approach:

- [tiremo_esp32.c — SyncNtp](../../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Platforms/Platform_esp32/src/tiremo_esp32.c)
- [tiremo_clock.c](../../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Common/Src/tiremo_clock.c)

## Step Outline

1. Enable SNTP and poll `CIPSNTPTIME?` until a valid time is returned or a timeout elapses
2. Parse the asctime string into an epoch value
3. Call `TIREMO_Clock_SetEpoch()` and confirm `GetEpoch()` matches real time

## Done When

`GetEpoch()` matches wall-clock time within a few seconds right after sync, and keeps tracking correctly afterward.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
