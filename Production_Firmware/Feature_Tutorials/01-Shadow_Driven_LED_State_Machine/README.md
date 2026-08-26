<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Shadow-Driven LED State Machine</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Shadow-Driven LED State Machine

***Board: Tiremo Cortex (ESP32) · Section: Production Firmware / Feature Tutorials · Difficulty: Hard***

## Introduction

Getting_Started Tutorial 05 built the pattern engine. This tutorial adds the selection logic on top: which mode is active is a function of MQTT connection state, alarms, and battery level — not something set directly.

## You Will Learn

- Priority-ordered mode selection (alert beats low_battery beats connecting beats normal)
- Reporting `current_mode` back to the shadow so the cloud sees live device state

## Prerequisites

- [AWS IoT Core](../../../AWS_IoT_Core/) complete
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Implement mode selection: off (if disabled) > alert > low_battery > connecting > normal
2. Drive the Getting_Started pattern engine from the selected mode
3. Publish `current_mode` in shadow reported and confirm it updates live in the AWS console as state changes

## Done When

Disconnecting WiFi flips the LED to `connecting` within one poll cycle, and the shadow's reported `current_mode` matches what's physically lit.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
