<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%" height="16">
      <h1>MQTT to a Public Broker</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# MQTT to a Public Broker

***Board: Tiremo Cortex (ESP32) · Section: Cloud Messaging (MQTT & TLS) / Feature Tutorials · Difficulty: Medium***

## Introduction

Before touching certificates, prove the ESP32's built-in MQTT client works at all — connect to a public test broker in the clear, publish a message, and subscribe to hear it echoed back.

## You Will Learn

- `AT+MQTTUSERCFG` (scheme 0, no TLS) / `AT+MQTTCONN` / `AT+MQTTPUB` / `AT+MQTTSUB`
- The difference between `MqttPublish` (string payload) and `MqttPublishRaw` (binary-safe)

## Prerequisites

- [ESP32 Connectivity](../../../ESP32_Connectivity/) — WiFi join working
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Configure MQTT identity with no TLS and connect to a public test broker
2. Subscribe to a topic and publish a message to that same topic
3. Confirm the published message arrives back via the subscription

## Done When

A message published by this board on a topic it also subscribes to is echoed back and logged.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%" height="16"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
