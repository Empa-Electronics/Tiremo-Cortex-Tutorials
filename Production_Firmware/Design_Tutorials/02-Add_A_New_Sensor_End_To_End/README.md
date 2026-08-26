<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="220">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Add a New Sensor End-to-End</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Add a New Sensor End-to-End

***Board: Tiremo Cortex (ESP32) · Section: Production Firmware / Design Tutorials · Difficulty: Hard***

## Introduction

A stretch tutorial for extending this board beyond its stock sensor set. It walks the same five layers every existing sensor crosses — Hal, Driver, shadow schema, alarm, telemetry — as a repeatable template, demonstrating the reference firmware's 'swappable backend' design principle in practice.

## You Will Learn

- Where a new sensor plugs into each layer: Hal (if a new peripheral type is needed), Driver, shadow desired/reported schema, alarm thresholds, telemetry payload
- Keeping cloud-facing schema additions backward compatible with the existing shadow document

## Prerequisites

- [01-Full_Event_Loop_Integration](../01-Full_Event_Loop_Integration/)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Reference Implementation

This tutorial is one slice of the reference firmware kept in this repo.
Read these before or after attempting the steps below to check your
approach:

- [README.md — Design principles: swappable backends](../../../referance_only/Tiremo-Secure-Iot-Sw/README.md)
- [Any existing driver as a template, e.g. tiremo_lis2de12.c](../../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Drivers/lis2de12/Src/tiremo_lis2de12.c)

## Step Outline

1. Pick a sensor not already in the kit and write its driver following the Sensors_And_Storage pattern
2. Add its shadow desired/reported block following SHADOW_README.md's field conventions
3. Wire it into alarm servicing and batched telemetry alongside the existing sensors
4. Confirm the new sensor's shadow block round-trips (desired → apply → reported) exactly like the built-in ones

## Done When

The new sensor has full parity with built-in sensors: shadow-configurable, alarms with hysteresis, included in batched telemetry — with no changes required to any other sensor's code.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
