<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%" height="16">
      <h1>Link Recovery & Backoff</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Link Recovery & Backoff

***Board: Tiremo Cortex (ESP32) · Section: Production Firmware / Feature Tutorials · Difficulty: Hard***

## Introduction

Networks drop. This tutorial builds the resilience layer that notices an MQTT session died, retries reconnection with a backoff delay, and — as a last resort — reboots the MCU after enough consecutive failures rather than staying wedged forever.

## You Will Learn

- Detecting session loss without relying on a hot-path connection probe (reuse Cloud_Messaging Tutorial 04's lesson)
- Backoff-then-retry instead of tight-loop reconnect attempts
- Choosing a sane consecutive-failure ceiling before triggering `NVIC_SystemReset()`

## Prerequisites

- [06-Factory_Reset](../06-Factory_Reset/)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Detect MQTT session loss and mark connectivity as down in the shadow's reported connectivity block
2. On a fixed backoff interval, attempt reconnect: network, then MQTT, then re-arm standing subscriptions
3. After N consecutive failed attempts, reboot; confirm the device comes back up and resumes normal operation

## Done When

Killing the AP mid-session triggers a `connecting` LED mode and shadow update, followed by a successful reconnect once the AP returns — or a clean reboot if it doesn't return within N attempts.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%" height="16"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
