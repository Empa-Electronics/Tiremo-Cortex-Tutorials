<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/Tiremo_Cloud/Feature_Tutorials/07-Tiremo_Jobs/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Tiremo Jobs</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Tiremo Jobs

***Board: Tiremo Cortex (ESP32) · Section: Tiremo Cloud / Feature Tutorials · Difficulty: Hard***

## Introduction

Tiremo Jobs is how a fleet queues work onto a Thing: a JSON document
arrives on `$aws/things/<thingName>/jobs/...`, the device reports
`IN_PROGRESS` / `SUCCEEDED` / `FAILED`, and the next job is offered.
Unlike shadow, a job is a one-shot execution with a status, not a
standing desired/reported loop.

This tutorial is **not implemented yet**. Folders and this README are the
scaffold; firmware and a worked walkthrough come next.

## You Will Learn

- The Jobs MQTT topic family (`notify-next`, `start-next`, `update`)
- Why a job must not be treated as another shadow delta
- Reporting execution status so the Tiremo UI shows a finished job

## Prerequisites

- [05-Live_Shadow_Delta_Handling](../05-Live_Shadow_Delta_Handling/) — MQTT
  session and Thing name
- [06-Publishing_Alarms_And_Telemetry](../06-Publishing_Alarms_And_Telemetry/)
  recommended so the ready loop already publishes without blocking
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Platform documentation

- [Introduction to Tiremo Jobs](https://docs.tiremo.ai/developer-guide/jobs/jobs-introduction)
- [Job templates and target selection](https://docs.tiremo.ai/developer-guide/jobs/jobs-phase-1-definition)
- [Job creation](https://docs.tiremo.ai/developer-guide/jobs/jobs-phase-2-creation)
- [Execution monitoring](https://docs.tiremo.ai/developer-guide/jobs/jobs-phase-3-monitoring)

Do not mix Jobs MQTT with classic shadow topics — see
[device firmware integration](https://docs.tiremo.ai/device-state-controller/device-integration).

## Solution

No solution has been written for this tutorial yet.

## Step Outline

1. Subscribe to `jobs/notify-next` (and start-next if the queue is already
   non-empty at boot)
2. Accept one job document, log the operation, run a bounded local action
3. Publish `jobs/<jobId>/update` with `SUCCEEDED` or `FAILED`

## Done When

A job created in the Tiremo UI is accepted on the board, executed once,
and marked succeeded in the Tiremo UI with matching UART.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
