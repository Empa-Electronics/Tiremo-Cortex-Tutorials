<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Reject Unsupported Jobs (OTA Stub)</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Reject Unsupported Jobs (OTA Stub)

***Board: Tiremo Cortex (ESP32) · Section: Production Firmware / Feature Tutorials · Difficulty: Medium***

## Introduction

This board doesn't implement OTA yet, but a job document that looks like one must still be handled cleanly — rejected with a clear reason, not left stuck IN_PROGRESS forever. This is the reference firmware's 'fail closed' design principle in miniature.

## You Will Learn

- Detecting an OTA-shaped job by ID prefix and by document content (`files`, `streamId`, `firmwareUpdate`, ...)
- Completing a job as `REJECTED` with a `reason` in `statusDetails`, rather than ignoring it

## Prerequisites

- [04-AWS_IoT_Jobs_Health_Check](../04-AWS_IoT_Jobs_Health_Check/)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Write a detector that flags a job document as OTA-shaped from its ID or content
2. On a detected OTA job, publish `REJECTED` with `reason: "ota_not_implemented"`
3. Create both a health-check job and an OTA-shaped job in the console and confirm each gets the correct outcome

## Done When

A health-check job reaches SUCCEEDED; an OTA-shaped job reaches REJECTED with a reason — neither is left stuck IN_PROGRESS.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
