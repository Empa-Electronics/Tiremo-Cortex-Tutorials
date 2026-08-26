<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%" height="16">
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

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Implement `FindObject`/`FindBool`/`FindUint`/`FindString` against a fixed sample shadow document
2. Implement a `JsonWriter` that can build a small nested object
3. Round-trip: find a field, change it, write it back, and re-parse to confirm

## Done When

All four Find* helpers correctly extract nested fields from a realistic shadow document, and the writer produces valid JSON a real parser accepts.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%" height="16"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
