<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="65%">
      <h1>Boot Cascade: Saved WiFi → BluFi</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Boot Cascade: Saved WiFi → BluFi

***Board: Tiremo Cortex (ESP32) · Section: ESP32 Connectivity / Design Tutorials · Difficulty: Hard***

## Introduction

This ties Tutorials 01–05 together into the board's real boot-time connection flow: try saved WiFi (SSID+password, or the ESP32's own NVS auto-rejoin), and if that's not possible, fall back to BluFi provisioning.

## You Will Learn

- Structuring a cascade as a chain of `prvTryX()` functions that each return a plain bool
- Deciding *which* WiFi path to attempt based on what's actually saved (SSID+password vs. SSID-only vs. nothing)

## Prerequisites

- All of ESP32 Connectivity / Feature Tutorials (01–05)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. If SSID+password are saved, scan-then-join (Tutorial 03)
2. Else if only an SSID is saved, try the ESP32's own NVS auto-rejoin
3. Else, force BluFi provisioning (Tutorial 05) and harvest whatever it learns
4. On any successful path, sync NTP (Tutorial 04) before returning

## Done When

From a factory-fresh board, BluFi provisioning succeeds and is remembered; on the next boot, the same board reconnects via saved WiFi with no BluFi step.

<p align="center"><img alt="" src="../../../images/banner.png" width="65%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
