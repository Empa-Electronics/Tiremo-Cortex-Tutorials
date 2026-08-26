<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="65%">
      <h1>Edge Alarms with Hysteresis</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Edge Alarms with Hysteresis

***Board: Tiremo Cortex (ESP32) · Section: Production Firmware / Feature Tutorials · Difficulty: Hard***

## Introduction

Alarms are edge-triggered, not level-triggered — a device must publish once on crossing a threshold and once on returning to normal, not repeatedly while sitting above the line. That state-tracking is the core of this tutorial.

## You Will Learn

- Edge-triggered alarm state per condition (a latch, not a repeated publish)
- The alarm payload shape (`alarmType`/`severity`/`code`/`exception`) and matching `*_NORMAL` clear events

## Prerequisites

- [01-Shadow_Driven_LED_State_Machine](../01-Shadow_Driven_LED_State_Machine/)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Pick one condition (e.g. temperature) and implement latch-on-cross, publish once, latch-off-on-return, publish once
2. Extend the same pattern to humidity, fall, shock, vibration, sound, and battery
3. Force each condition past its threshold on the bench and confirm exactly one alarm and one clear event per crossing

## Done When

Each of the seven alarm types fires exactly once per threshold crossing and clears exactly once per return to normal — no repeats while sitting on either side of the line.

<p align="center"><img alt="" src="../../../images/banner.png" width="65%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
