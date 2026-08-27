<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/Tiremo_Cloud/Design_Tutorials/01-Sensor_Telemetry_And_Shadow_Control/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Sensor Telemetry and Shadow Control</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Sensor Telemetry and Shadow Control

***Board: Tiremo Cortex (ESP32) · Section: Tiremo Cloud / Design Tutorials · Difficulty: Hard***

## Introduction

This is the first Tiremo Cloud **demo** that looks like a product: onboard
sensors are sampled and published as telemetry, while a cloud operator
opens and closes an output (LED / actuator) through classic Device Shadow
desired/delta — without rebooting.

It combines [Sensors & Local Storage](../../../Sensors_And_Storage/)
readings with [05 live shadow](../../Feature_Tutorials/05-Live_Shadow_Delta_Handling/)
and [06 telemetry](../../Feature_Tutorials/06-Publishing_Alarms_And_Telemetry/).

This tutorial is **not implemented yet**. Folders and this README are the
scaffold; firmware and a worked walkthrough come next.

## You Will Learn

- One ready loop: poll shadow delta, sample sensors, publish telemetry
- Shadow as on/off (and interval) only — samples stay on the telemetry topic
- Why a sensor JSON must not be stuffed into `state.reported` (document
  size and leftover delta)

## Prerequisites

- Feature tutorials [01](../../Feature_Tutorials/01-Device_Identity_From_MAC/)–
  [05](../../Feature_Tutorials/05-Live_Shadow_Delta_Handling/) (identity,
  provision, live shadow)
- [06-Publishing_Alarms_And_Telemetry](../../Feature_Tutorials/06-Publishing_Alarms_And_Telemetry/)
- SHT40 (and related) bring-up from [Sensors & Local Storage](../../../Sensors_And_Storage/)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Platform documentation

- Shadow open/close: [Device State Controller](https://docs.tiremo.ai/device-state-controller/introduction), [device integration](https://docs.tiremo.ai/device-state-controller/device-integration)
- Telemetry: [architecture & data flow](https://docs.tiremo.ai/introduction/phase-1), [low-rate telemetry](https://docs.tiremo.ai/use-cases/scenario-low-rate-telemetry)
- Identity and certs: [device provisioning](https://docs.tiremo.ai/device-provisioning/getting-started)

## Solution

No solution has been written for this tutorial yet.

## Step Outline

1. Provision / reconnect using feature tutorials 01–05
2. Apply live shadow so `led.on` (open/close) and `reporting.intervalS` take
   effect immediately
3. Sample the environmental sensors on that interval and publish telemetry
4. Confirm the Tiremo UI: shadow toggles the output; MQTT test client
   sees samples; delta still clears

## Done When

Flipping `desired.led.on` in the console turns the board output on and off
with no reset, and UART plus an MQTT subscription show periodic sensor
telemetry on a separate topic.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
