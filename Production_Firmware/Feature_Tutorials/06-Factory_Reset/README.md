<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <h1>Factory Reset</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Factory Reset

***Board: Tiremo Cortex (ESP32) · Section: Production Firmware / Feature Tutorials · Difficulty: Medium***

## Introduction

Combines Getting_Started's button-hold detection with Sensors_And_Storage's flash config and Cloud_Messaging's credential slots into one destructive-but-necessary operation: return the device to a provisionable state.

## You Will Learn

- Detecting a sustained hold (not just a press) as the trigger, reusing Tutorial 04 from Getting_Started
- Sequencing a multi-target wipe (module credentials, then MCU flash) before reboot so a failure partway doesn't leave an inconsistent state

## Prerequisites

- [05-Reject_Unsupported_Jobs](../05-Reject_Unsupported_Jobs/)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Reference Implementation

This tutorial is one slice of the reference firmware kept in this repo.
Read these before or after attempting the steps below to check your
approach:

- [tiremo_app_factory.c](../../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Tiremo_Core/Tiremo_App/Src/tiremo_app_factory.c)
- [SHADOW_README.md — Factory reset (hardware)](../../../referance_only/Tiremo-Secure-Iot-Sw/SHADOW_README.md)

## Step Outline

1. Detect a 10 s USER-button hold during both the BluFi wait and the main run loop
2. On trigger: erase ESP32 SYSMFG credential slots, leave the AP, disable auto-connect, `AT+RESTORE`
3. Erase the MCU config-flash record from Sensors_And_Storage Tutorial 06
4. Reboot via `NVIC_SystemReset()` and confirm the next boot re-provisions from scratch

## Done When

Holding USER for 10 s wipes all stored WiFi and cloud identity; the next boot behaves exactly like a factory-fresh unit.

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
