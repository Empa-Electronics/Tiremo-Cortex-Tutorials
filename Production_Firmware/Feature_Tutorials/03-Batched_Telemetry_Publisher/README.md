<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="220">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Batched Telemetry Publisher</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Batched Telemetry Publisher

***Board: Tiremo Cortex (ESP32) · Section: Production Firmware / Feature Tutorials · Difficulty: Hard***

## Introduction

Telemetry isn't a raw per-sample publish — it's an average over `batch_size` samples, published on `report_interval_ms`, with publish retries and an event-loop poll budget that adapts so a fast report interval doesn't starve MQTT polling.

## You Will Learn

- Averaging N samples into one publish instead of publishing every reading
- Retry-with-limit on a failed publish (`retry_interval_ms` / `max_retries`)
- Why the MQTT poll budget must shrink as `report_interval_ms` shrinks

## Prerequisites

- [02-Edge_Alarms_With_Hysteresis](../02-Edge_Alarms_With_Hysteresis/)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Accumulate `batch_size` samples per enabled sensor, then publish the average
2. Add publish retry with `retry_interval_ms` backoff up to `max_retries`
3. Set `report_interval_ms` to something aggressive (e.g. 200 ms) via shadow and confirm the loop still services MQTT without falling behind

## Done When

At the default interval, telemetry publishes are correctly averaged batches; dropping `report_interval_ms` to 200 ms via shadow does not stall MQTT polling or alarm servicing.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
