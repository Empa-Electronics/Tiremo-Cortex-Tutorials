<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>AWS IoT Jobs — Health Check</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# AWS IoT Jobs — Health Check

***Board: Tiremo Cortex (ESP32) · Section: Production Firmware / Feature Tutorials · Difficulty: Hard***

## Introduction

AWS IoT Jobs let the cloud ask a fleet of devices to run a named operation and report back. This tutorial implements the simplest kind — a health check — end to end: notification, claiming, and completion.

## You Will Learn

- The `jobs/notify` → `jobs/start-next` → `jobs/<id>/update` sequence
- Handling both a full execution document and a bare summary notify that requires a follow-up `start-next`

## Prerequisites

- [03-Batched_Telemetry_Publisher](../03-Batched_Telemetry_Publisher/)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Subscribe to `jobs/notify` and `jobs/notify-next`
2. On notification, call `start-next`, then `UpdateJobExecution` with status `SUCCEEDED` and `statusDetails` (firmware version, connectivity, uptime)
3. Create a health-check job execution in the AWS console and confirm it reaches `SUCCEEDED` with the expected detail fields

## Done When

A job created in the AWS console reaches `SUCCEEDED` with `firmware_version`, `connectivity`, `mqtt`, and `uptime_s` populated in `statusDetails`.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
