<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%" height="16">
      <h1>Reliable Inbound Message Draining</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Reliable Inbound Message Draining

***Board: Tiremo Cortex (ESP32) · Section: Cloud Messaging (MQTT & TLS) / Feature Tutorials · Difficulty: Hard***

## Introduction

A real bug from the reference firmware's history: polling `AT+MQTTCONN?` on every loop tick flushes the RX ring and silently drops inbound messages arriving in that window. This tutorial reproduces the failure mode, then fixes it the way the reference firmware does.

## You Will Learn

- Why any AT command that flushes the RX ring is unsafe to run on a hot polling path
- Registering a message handler and draining exclusively through `MqttPoll()`

## Prerequisites

- [03-MQTT_mTLS_To_AWS_IoT](../03-MQTT_mTLS_To_AWS_IoT/)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Reproduce the bug: poll `MqttIsConnected()` every tick while another client publishes to a subscribed topic rapidly — observe dropped messages
2. Replace the hot-path probe with `MqttSetHandler()` + `MqttPoll(budgetMs)`
3. Repeat the rapid-publish test and confirm no messages are dropped

## Done When

Rapid-fire publishes from an external client are all received with the handler-driven loop, where they were previously dropped under the polling version.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%" height="16"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
