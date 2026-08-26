<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="images/tiremo_sub_brand_1_mono_black.png" width="220">
      </picture>
      <br>
      <img alt="" src="images/banner.png" width="100%">
      <h1>Frequently Asked Questions</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Frequently Asked Questions

- [How are the tutorials organized?](#how-are-the-tutorials-organized)
- [What's the difference between a Feature Tutorial and a Design Tutorial?](#whats-the-difference-between-a-feature-tutorial-and-a-design-tutorial)
- [Where do I check my work?](#where-do-i-check-my-work)
- [How do I contribute a tutorial?](#how-do-i-contribute-a-tutorial)

## How are the tutorials organized?

Tutorials are organized into topics (folders), each covering one layer of
the firmware stack, roughly in build order:

- [Getting Started](./Getting_Started): toolchain, first blink, UART logging, button, LED patterns — no networking.
- [Sensors & Local Storage](./Sensors_And_Storage): SHT40, LIS2DE12, battery, microphone, flash config, software clock.
- [ESP32 Connectivity](./ESP32_Connectivity): AT transport, WiFi join, retries, NTP, BluFi provisioning.
- [Cloud Messaging](./Cloud_Messaging): MQTT, mTLS credentials, AWS IoT Core connection, reliable polling.
- [AWS IoT Core](./AWS_IoT_Core): device identity, fleet provisioning, JSON helpers, device shadow.
- [Production Firmware](./Production_Firmware): shadow-driven LEDs, alarms, telemetry, jobs, factory reset, link recovery, and the full capstone build.
- [Developer Contributed](./Developer_Contributed): anything community-submitted that doesn't fit the numbered curriculum.

## What's the difference between a Feature Tutorial and a Design Tutorial?

A Feature Tutorial teaches one concept in isolation — one driver, one AT
command sequence, one cloud primitive. A Design Tutorial combines several
already-completed Feature Tutorials into one working subsystem. Design
Tutorials always list their Feature Tutorial prerequisites.

## Where do I check my work?

Every tutorial's README has a **Reference Implementation** section pointing
into [`referance_only/Tiremo-Secure-Iot-Sw/`](referance_only/Tiremo-Secure-Iot-Sw/) — the reference firmware this curriculum
is built from. If your implementation diverges, that's where to compare.

## How do I contribute a tutorial?

Open a PR adding a numbered folder under the relevant topic (or under
[Developer_Contributed](./Developer_Contributed) if it doesn't fit the
numbered curriculum), with a `README.md` following the same structure as
the existing tutorials: Introduction, You Will Learn, Prerequisites,
Reference Implementation, Step Outline, Done When.

<p align="center"><img alt="" src="images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
