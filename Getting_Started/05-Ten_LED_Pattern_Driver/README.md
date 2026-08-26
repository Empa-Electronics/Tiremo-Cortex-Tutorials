<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../images/banner.png" width="65%">
      <h1>Ten-LED Pattern Driver</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Ten-LED Pattern Driver

***Board: Tiremo Cortex (ESP32) · Section: Getting Started / Getting Started · Difficulty: Easy***

## Introduction

The production firmware selects LED patterns from shadow state (Production Firmware), but the pattern engine itself — solid/blink/pulse timing — doesn't need the cloud to be built and tested. Build it standalone now.

## You Will Learn

- Driving multiple active-low LEDs from one module
- A tiny state machine for solid / blink / pulse patterns driven off SysTick

## Prerequisites

- [04-Button_Read_And_Debounce](../04-Button_Read_And_Debounce/)
- Toolchain and flashing set up per [Getting Started](../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Implement `solid`, `blink`, and `pulse` pattern functions taking an LED index and interval
2. Cycle all ten LEDs through the three patterns on a timer
3. Wire the USER button to cycle patterns manually, combining Tutorials 04 and 05

## Done When

All ten LEDs run independently addressable solid/blink/pulse patterns, switchable via button press.

<p align="center"><img alt="" src="../../images/banner.png" width="65%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
