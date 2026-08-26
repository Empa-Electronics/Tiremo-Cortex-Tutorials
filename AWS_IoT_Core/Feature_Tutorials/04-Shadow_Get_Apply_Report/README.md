<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="65%">
      <h1>Shadow Get + Apply + Report</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Shadow Get + Apply + Report

***Board: Tiremo Cortex (ESP32) · Section: AWS IoT Core / Feature Tutorials · Difficulty: Hard***

## Introduction

This is the device shadow control loop in its simplest form: get, apply one field, and report back the full applied state — including nested objects — so AWS doesn't keep showing a permanent delta.

## You Will Learn

- `shadow/get` / `shadow/get/accepted` request-response over MQTT
- Why reported must echo the *full* applied desired object, not just the changed keys — a real bug fixed in the reference firmware

## Prerequisites

- [03-Minimal_JSON_Helpers](../03-Minimal_JSON_Helpers/)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Subscribe to `shadow/get/accepted`, publish an empty `shadow/get` request
2. Parse `state.desired`, apply one field locally (e.g. `reporting.enabled`)
3. Publish `state.reported` echoing the full applied desired object, nested fields included
4. Confirm in the AWS console that the shadow delta clears after report

## Done When

Setting `desired.reporting.enabled` in the AWS console results in a `reported` echo that clears the delta, with no leftover delta on a re-fetch.

<p align="center"><img alt="" src="../../../images/banner.png" width="65%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
