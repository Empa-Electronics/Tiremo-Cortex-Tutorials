<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="220">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Provision and Sync a Real Thing</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Provision and Sync a Real Thing

***Board: Tiremo Cortex (ESP32) · Section: AWS IoT Core / Design Tutorials · Difficulty: Hard***

## Introduction

This ties AWS IoT Core's feature tutorials into one coherent boot sequence, end to end: a factory-fresh board provisions itself, connects with its own issued identity, fetches its shadow, applies desired state, and reports back — all against a real AWS IoT account, not a bench certificate.

## You Will Learn

- Sequencing provisioning before the shadow exchange, and reusing the same MQTT session for both
- Structuring this as one `EstablishCloud()`-style function other tutorials can call

## Prerequisites

- All of AWS IoT Core / Feature Tutorials (01–05)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Reference Implementation

This tutorial is one slice of the reference firmware kept in this repo.
Read these before or after attempting the steps below to check your
approach:

- [tiremo_app_connectivity.c — prvEstablishCloud/prvCloudFirstCheckup](../../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Tiremo_Core/Tiremo_App/Src/tiremo_app_connectivity.c)
- [tiremo_iot_checkup.c](../../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Tiremo_Core/Tiremo_IoT/Src/tiremo_iot_checkup.c)

## Step Outline

1. On first boot, run Tutorial 02's provisioning flow if no cert is stored yet
2. Reconnect using the issued (or previously stored) certificate
3. Run Tutorial 04/05's get + standing delta subscription
4. Power-cycle and confirm the second boot skips provisioning and goes straight to shadow sync

## Done When

First boot provisions and syncs from a clean AWS account; every subsequent boot skips provisioning and syncs directly, using the same code path Tutorial 05 verified live.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
