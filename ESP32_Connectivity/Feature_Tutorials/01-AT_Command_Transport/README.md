<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%" height="16">
      <h1>AT Command Transport</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# AT Command Transport

***Board: Tiremo Cortex (ESP32) · Section: ESP32 Connectivity / Feature Tutorials · Difficulty: Medium***

## Introduction

Every ESP32 feature from here on rides on this transport: power sequencing, a UART RX ring buffer, and an expect/timeout command primitive. Get this rock solid before building WiFi or MQTT on top of it.

## You Will Learn

- The module power-cycle sequence (off → boot delay → AT probe)
- An expect-substring-with-timeout command primitive over an interrupt-driven RX ring buffer
- Why echo (`ATE0`) must be disabled before parsing later responses

## Prerequisites

- [Getting_Started](../../../Getting_Started/) complete
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Power-cycle the ESP32 supply pin, holding off then on per `TIREMO_BOARD_ESP32_PWR_OFF_MS`/`PWR_BOOT_MS`
2. Send `AT` and wait for `OK` within a timeout
3. Send `ATE0` to disable echo and confirm subsequent responses are echo-free

## Done When

`AT` reliably returns `OK` after every power cycle, and echoed command text no longer appears in captured responses.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%" height="16"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
