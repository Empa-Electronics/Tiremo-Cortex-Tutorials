<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
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

## Reference Implementation

This tutorial is one slice of the reference firmware kept in this repo.
Read these before or after attempting the steps below to check your
approach:

- [tiremo_iot_checkup.c](../../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Tiremo_Core/Tiremo_IoT/Src/tiremo_iot_checkup.c)
- [tiremo_iot_runtime.c — Shadow_ApplyDocument/Shadow_PublishReported](../../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Tiremo_Core/Tiremo_IoT/Src/tiremo_iot_runtime.c)
- [SHADOW_README.md](../../../referance_only/Tiremo-Secure-Iot-Sw/SHADOW_README.md)

## Step Outline

1. Subscribe to `shadow/get/accepted`, publish an empty `shadow/get` request
2. Parse `state.desired`, apply one field locally (e.g. `reporting.enabled`)
3. Publish `state.reported` echoing the full applied desired object, nested fields included
4. Confirm in the AWS console that the shadow delta clears after report

## Done When

Setting `desired.reporting.enabled` in the AWS console results in a `reported` echo that clears the delta, with no leftover delta on a re-fetch.

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
