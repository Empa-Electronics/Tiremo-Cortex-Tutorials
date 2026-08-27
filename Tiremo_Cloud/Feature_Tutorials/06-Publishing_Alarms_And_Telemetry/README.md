<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/Tiremo_Cloud/Feature_Tutorials/06-Publishing_Alarms_And_Telemetry/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Publishing Alarms and Telemetry</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Publishing Alarms and Telemetry

***Board: Tiremo Cortex (ESP32) · Section: Tiremo Cloud / Feature Tutorials · Difficulty: Hard***

## Introduction

Shadow is the control plane. Telemetry and alarms are the data plane: the
device publishes sensor samples and edge-raised events on MQTT topics Tiremo Cloud
can route, store, or alert on, without stuffing that stream into the
shadow document.

This tutorial is **not implemented yet**. Folders and this README are the
scaffold; firmware and a worked walkthrough come next.

## You Will Learn

- A small, stable topic layout (`telemetry`, `alarms`) under the Thing name
- Why periodic telemetry is QoS 0/1 JSON, not a shadow `reported` rewrite
- An alarm as a distinct publish (threshold crossed) rather than a shadow
  field that storms `update/delta`

## Prerequisites

- [05-Live_Shadow_Delta_Handling](../05-Live_Shadow_Delta_Handling/) — MQTT
  session, Thing name, standing subscriptions
- [02-Minimal_JSON_Helpers](../02-Minimal_JSON_Helpers/) — bounded JSON write
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Platform documentation

Telemetry is the data plane, not the shadow control loop. See
[architecture & data flow](https://docs.tiremo.ai/introduction/phase-1)
and the [low-rate telemetry](https://docs.tiremo.ai/use-cases/scenario-low-rate-telemetry)
use case. Shadow remains
[Device State Controller](https://docs.tiremo.ai/device-state-controller/introduction).

## Solution

No solution has been written for this tutorial yet.

## Step Outline

1. After the tutorial 05 ready loop, publish a compact telemetry JSON on a
   timer (`reporting.intervalS` from the last applied shadow)
2. Raise one alarm topic when a local threshold is crossed; clear it when
   the value returns (hysteresis)
3. Keep shadow traffic limited to desired/reported control fields

## Done When

UART shows periodic telemetry publishes and a single alarm raise/clear pair
that a cloud rule can subscribe to, with no leftover shadow delta storm.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
