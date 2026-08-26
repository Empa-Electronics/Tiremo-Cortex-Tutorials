<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="65%">
      <h1>Fleet Provisioning by Claim Certificate</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Fleet Provisioning by Claim Certificate

***Board: Tiremo Cortex (ESP32) · Section: AWS IoT Core / Feature Tutorials · Difficulty: Hard***

## Introduction

This replaces Cloud Messaging Tutorial 03's manually-created certificate with the real fleet-provisioning flow: connect with a shared claim certificate, request a unique device certificate over MQTT, register the Thing, then switch to the issued credentials.

## You Will Learn

- AWS IoT's provisioning-by-claim MQTT request/response topics
- Storing the issued certificate into SYSMFG and writing a provisioning marker so it only runs once

## Prerequisites

- [01-Device_Identity_From_MAC](../01-Device_Identity_From_MAC/); an AWS IoT provisioning template and claim certificate
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Connect with the shared claim certificate
2. Publish `CreateKeysAndCertificate`, capture the issued cert/key and ownership token
3. Publish `RegisterThing` with the device's identity as provisioning parameters
4. Store the issued cert into SYSMFG, write the provisioning marker, and reconnect using it

## Done When

A factory-fresh board provisions itself into a uniquely-named AWS IoT Thing with no manual console step, and skips provisioning on the next boot.

<p align="center"><img alt="" src="../../../images/banner.png" width="65%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
