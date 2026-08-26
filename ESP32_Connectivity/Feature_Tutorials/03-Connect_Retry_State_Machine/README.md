<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Connect/Retry State Machine</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Connect/Retry State Machine

***Board: Tiremo Cortex (ESP32) · Section: ESP32 Connectivity / Feature Tutorials · Difficulty: Medium***

## Introduction

Real WiFi joins fail sometimes — a saved AP might be out of range, or briefly overloaded. This wraps Tutorial 02's bare join in a scan-first, retry-with-backoff loop, and persists whichever SSID actually worked using the Sensors & Local Storage flash-config tutorial.

## You Will Learn

- Scan-for-SSID before attempting a join, to fail fast on an out-of-range AP
- A bounded retry loop with a fixed delay between attempts
- Clearing saved credentials on a definite join failure vs. keeping them on a timeout

## Prerequisites

- [02-WiFi_Station_Join](../02-WiFi_Station_Join/), and [Flash_Backed_Config](../../../Sensors_And_Storage/Feature_Tutorials/06-Flash_Backed_Config/)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Scan for the saved SSID before joining; skip straight to failure if it's not visible
2. Attempt join with up to 3 retries and a fixed delay between them
3. On success, persist the SSID; on repeated failure, clear saved credentials

## Done When

An out-of-range SSID fails fast (no join attempt); an in-range SSID with a transient failure still connects within 3 retries.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
