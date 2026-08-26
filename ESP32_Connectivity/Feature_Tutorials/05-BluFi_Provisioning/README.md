<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>BluFi BLE Provisioning</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# BluFi BLE Provisioning

***Board: Tiremo Cortex (ESP32) · Section: ESP32 Connectivity / Feature Tutorials · Difficulty: Hard***

## Introduction

Hardcoding an SSID doesn't scale past one bench unit. BluFi lets a phone app push credentials over BLE at first boot. This tutorial is also a lesson in careful RX-buffer ownership: polling the wrong AT command at the wrong moment can silently eat the credential URC.

## You Will Learn

- `AT+BLUFINAME` / `AT+BLUFI=1` and the asynchronous `+SSID:`/`+PASSWORD:`/`WIFI GOT IP` URCs
- Why `TIREMO_ESP32_BlufiPoll()` must be the *sole* consumer of the RX buffer during the wait — calling `IsConnected()` in the same loop can eat a credential URC before your code sees it

## Prerequisites

- [04-NTP_Time_Sync](../04-NTP_Time_Sync/)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Start BluFi advertising and wait for a phone app to submit credentials
2. Poll only `BlufiPoll()` during the wait loop; capture SSID/password as they arrive
3. Stop BluFi once connected, persist the harvested credentials, and re-enable auto-connect

## Done When

A phone BluFi app can provision the board with no hardcoded SSID anywhere in source, and the credentials persist across reboot.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
