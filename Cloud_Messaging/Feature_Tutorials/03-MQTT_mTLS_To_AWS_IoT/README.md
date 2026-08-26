<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="220">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>MQTT mTLS to AWS IoT Core</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# MQTT mTLS to AWS IoT Core

***Board: Tiremo Cortex (ESP32) · Section: Cloud Messaging (MQTT & TLS) / Feature Tutorials · Difficulty: Hard***

## Introduction

With credentials writable (Tutorial 02) and MQTT working in the clear (Tutorial 01), combine them: load a certificate you created by hand in the AWS console onto a single Thing, and get a real mTLS MQTT session up.

## You Will Learn

- TLS scheme 5 (verify server cert, present client cert) via `MQTTUSERCFG`
- Why `AT+MQTTSNI` matters for a custom domain endpoint sitting behind SNI routing
- Manually creating one AWS IoT Thing + certificate for bench testing, before AWS IoT Core automates it

## Prerequisites

- [02-mTLS_Credentials_In_Module_NVS](../02-mTLS_Credentials_In_Module_NVS/); an AWS IoT Core endpoint and one manually-created Thing certificate
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Reference Implementation

This tutorial is one slice of the reference firmware kept in this repo.
Read these before or after attempting the steps below to check your
approach:

- [tiremo_iot_client.c](../../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Tiremo_Core/Tiremo_IoT/Src/tiremo_iot_client.c)
- [tiremo_esp32.h — MqttUserConfig/MqttSni/MqttConnect](../../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Platforms/Platform_esp32/include/tiremo_esp32.h)

## Step Outline

1. Load Root CA / cert / key into SYSMFG slots via Tutorial 02's driver
2. Configure MQTT identity for scheme 5 and set the SNI hostname
3. Connect to your AWS IoT endpoint and publish to a test topic; confirm receipt in the AWS IoT MQTT test client

## Done When

A publish from the board appears in the AWS IoT console's MQTT test client on the expected topic.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
