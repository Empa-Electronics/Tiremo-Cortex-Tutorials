<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="220">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Full Event Loop Integration (Capstone)</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Full Event Loop Integration (Capstone)

***Board: Tiremo Cortex (ESP32) · Section: Production Firmware / Design Tutorials · Difficulty: Hard***

## Introduction

This is the capstone: every driver, connectivity path, and cloud behavior built across the previous six sections, assembled into the same two-function shape the production firmware uses — one-time boot setup, then a single event-loop tick doing MQTT poll → delta → jobs → alarms → telemetry → LEDs.

## You Will Learn

- Structuring a whole firmware as `ProbeAndInitialize()` once + `Run()` forever, rather than one giant `main()`
- The ordering within one event-loop tick, and why each step is bounded so the loop stays responsive

## Prerequisites

- Every tutorial in this repository
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Boot: hardware init → load config → sensor bring-up → connection setup (WiFi/BluFi → cloud)
2. Run loop: service factory-reset hold, then one bounded event-loop tick, then link recovery
3. Event-loop tick: MQTT poll → shadow delta → jobs → alarms → telemetry → LEDs, in that order
4. Leave the device running unattended for an extended soak and confirm no drift, leak, or wedge

## Done When

A single firmware image boots from factory-fresh, provisions, and runs the full sensor/alarm/telemetry/jobs/LED loop indefinitely, matching the reference firmware's behavior.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
