<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="220">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>WiFi Station Join</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# WiFi Station Join

***Board: Tiremo Cortex (ESP32) · Section: ESP32 Connectivity / Feature Tutorials · Difficulty: Medium***

## Introduction

The simplest possible WiFi connection: a hardcoded network, one join attempt, no retries yet. This isolates the AT command sequence from the retry/persistence logic that Tutorial 03 adds on top.

## You Will Learn

- `AT+CWMODE=1`, `AT+CWJAP`, `AT+CWSTATE?`
- The CHANGELOG's real gotcha: matching a response on `"OK"` rather than a substring that appears before the payload you need

## Prerequisites

- [01-AT_Command_Transport](../01-AT_Command_Transport/)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Reference Implementation

This tutorial is one slice of the reference firmware kept in this repo.
Read these before or after attempting the steps below to check your
approach:

- [tiremo_esp32.c — JoinAP/IsConnected](../../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Platforms/Platform_esp32/src/tiremo_esp32.c)
- [CHANGELOG.md — Fixed: CWJAP/CIPSNTPTIME substring bug](../../../referance_only/Tiremo-Secure-Iot-Sw/CHANGELOG.md)

## Step Outline

1. Set station mode with `CWMODE=1`
2. Join a hardcoded SSID/password and wait for `WIFI GOT IP`
3. Poll `CWSTATE?` and confirm state 2 (connected with IPv4)

## Done When

Board joins a real AP after a fresh power-up with no manual intervention, confirmed by `CWSTATE?`.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
