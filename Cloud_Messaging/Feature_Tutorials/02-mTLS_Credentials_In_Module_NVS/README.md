<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="220">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>mTLS Credentials in Module NVS</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# mTLS Credentials in Module NVS

***Board: Tiremo Cortex (ESP32) · Section: Cloud Messaging (MQTT & TLS) / Feature Tutorials · Difficulty: Medium***

## Introduction

AWS IoT requires mutual TLS. The ESP32 module — not the MCU — holds the TLS credentials in its own NVS via `AT+SYSMFG`. This tutorial is purely about that storage mechanism, before Tutorial 03 uses it for a real connection.

## You Will Learn

- The four `TIREMO_ESP32_CredSlot_t` slots and why they live in the module, not MCU flash
- `AT+SYSMFG` write/read/erase semantics

## Prerequisites

- [01-MQTT_To_Public_Broker](../01-MQTT_To_Public_Broker/)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Reference Implementation

This tutorial is one slice of the reference firmware kept in this repo.
Read these before or after attempting the steps below to check your
approach:

- [tiremo_esp32.h — CredWrite/CredRead/CredErase](../../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Platforms/Platform_esp32/include/tiremo_esp32.h)

## Step Outline

1. Write a test PEM string into the ROOT_CA slot
2. Read it back and confirm it matches byte-for-byte
3. Erase it and confirm the read now fails

## Done When

Write/read/erase round-trips correctly for all three credential slots.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
