<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%" height="16">
      <h1>Live Shadow Delta Handling</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Live Shadow Delta Handling

***Board: Tiremo Cortex (ESP32) · Section: AWS IoT Core / Feature Tutorials · Difficulty: Hard***

## Introduction

Tutorial 04 only applies desired state once, at boot. Real control-plane behavior means reacting to a delta the moment AWS publishes one — this tutorial adds the standing subscription and live-apply path.

## You Will Learn

- The standing `shadow/update/delta` subscription, distinct from the one-shot `shadow/get` exchange
- Reusing Tutorial 04's apply/report logic for both the boot-time get and the live delta

## Prerequisites

- [04-Shadow_Get_Apply_Report](../04-Shadow_Get_Apply_Report/)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Subscribe to `shadow/update/delta` once at boot, alongside the get/accepted flow
2. On an inbound delta, apply it and publish reported using the same code path as Tutorial 04
3. From the AWS console, change `desired` twice in a row while the board runs and confirm both take effect with no reboot

## Done When

Toggling a field in the AWS console while the board is running changes device behavior within one MQTT poll cycle, with no reboot required.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%" height="16"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
