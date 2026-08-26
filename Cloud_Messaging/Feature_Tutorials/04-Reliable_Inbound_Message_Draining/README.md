<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
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

## Reference Implementation

This tutorial is one slice of the reference firmware kept in this repo.
Read these before or after attempting the steps below to check your
approach:

- [tiremo_esp32.h — MqttSetHandler/MqttPoll](../../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Platforms/Platform_esp32/include/tiremo_esp32.h)
- [CHANGELOG.md — Fixed: hot-path MqttIsConnected() dropping URCs](../../../referance_only/Tiremo-Secure-Iot-Sw/CHANGELOG.md)

## Step Outline

1. Reproduce the bug: poll `MqttIsConnected()` every tick while another client publishes to a subscribed topic rapidly — observe dropped messages
2. Replace the hot-path probe with `MqttSetHandler()` + `MqttPoll(budgetMs)`
3. Repeat the rapid-publish test and confirm no messages are dropped

## Done When

Rapid-fire publishes from an external client are all received with the handler-driven loop, where they were previously dropped under the polling version.

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
