<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Device Identity from MAC</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Device Identity from MAC

***Board: Tiremo Cortex (ESP32) · Section: AWS IoT Core / Feature Tutorials · Difficulty: Medium***

## Introduction

Fleet provisioning and shadow topics both need a stable per-device identity. Rather than hardcoding one, derive it from hardware that's already unique: the ESP32's station MAC.

## You Will Learn

- `AT+CIPSTAMAC?` and formatting a MAC as a lowercase hex device ID
- Why identity should be read from hardware, not baked into firmware source

## Prerequisites

- [Cloud Messaging](../../../Cloud_Messaging/) — mTLS MQTT connection working
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Read the station MAC via `GetHwEui()`
2. Format it as a 12-hex-digit lowercase device ID with no separators
3. Log it on every boot and confirm it's stable across power cycles

## Done When

The same device ID is produced on every boot, and differs between two different boards.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
