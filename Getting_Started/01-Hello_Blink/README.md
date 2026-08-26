<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../images/banner.png" width="65%">
      <h1>Hello Blink</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Hello Blink

***Board: Tiremo Cortex (ESP32) · Section: Getting Started / Getting Started · Difficulty: Easy***

## Introduction

Before anything talks to WiFi or AWS, you need a build that flashes and a board that visibly does something. This tutorial blinks LED1 to prove the toolchain, OpenOCD wiring, and board bring-up all work.

## You Will Learn

- Active-low LED wiring and the `TIREMO_BOARD_LED*` pin macros
- The `Tiremo_Hal` GPIO wrapper (`tiremo_hal_gpio.c`)
- `build.sh` / `flash.sh` / OpenOCD round-trip

## Prerequisites

- None — first tutorial in this section
- Toolchain and flashing set up per [Getting Started](../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Wire up `Tiremo_Hal_GPIO_Init()` for LED1's port/pin as an active-low output
2. Busy-wait toggle it on a fixed interval
3. `./build.sh` then `./flash.sh` and confirm the LED blinks on real hardware

## Done When

LED1 blinks at 1 Hz after a fresh flash, with no debugger attached.

<p align="center"><img alt="" src="../../images/banner.png" width="65%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
