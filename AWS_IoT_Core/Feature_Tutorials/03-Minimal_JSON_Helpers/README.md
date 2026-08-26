<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <h1>Minimal JSON Helpers</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Minimal JSON Helpers

***Board: Tiremo Cortex (ESP32) · Section: AWS IoT Core / Feature Tutorials · Difficulty: Medium***

## Introduction

Shadow and jobs documents are JSON, but pulling in a general-purpose parser is overkill on this MCU. This tutorial builds the minimal `FindObject`/`FindBool`/`FindUint`/`Writer` surface the rest of AWS IoT Core needs — a good exercise in scoping a library to exactly what's used.

## You Will Learn

- Why a hand-rolled scan-and-extract approach is enough for known, bounded document shapes
- The tradeoffs versus a real JSON library (cJSON, jsmn) on a memory-constrained MCU

## Prerequisites

- [02-Fleet_Provisioning_By_Claim](../02-Fleet_Provisioning_By_Claim/)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Reference Implementation

This tutorial is one slice of the reference firmware kept in this repo.
Read these before or after attempting the steps below to check your
approach:

- [tiremo_iot_json.c](../../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Tiremo_Core/Tiremo_IoT/Src/tiremo_iot_json.c)
- [referance_json.json — referenced but gitignored; build your own sample from SHADOW_README.md](../../../referance_only/Tiremo-Secure-Iot-Sw/SHADOW_README.md)

## Step Outline

1. Implement `FindObject`/`FindBool`/`FindUint`/`FindString` against a fixed sample shadow document
2. Implement a `JsonWriter` that can build a small nested object
3. Round-trip: find a field, change it, write it back, and re-parse to confirm

## Done When

All four Find* helpers correctly extract nested fields from a realistic shadow document, and the writer produces valid JSON a real parser accepts.

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
