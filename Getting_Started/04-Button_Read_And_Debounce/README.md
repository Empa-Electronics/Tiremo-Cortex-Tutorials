<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../images/banner.png" width="100%" height="16">
      <h1>Button Read & Debounce</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Button Read & Debounce

***Board: Tiremo Cortex (ESP32) · Section: Getting Started / Getting Started · Difficulty: Easy***

## Introduction

The USER button is only used for one thing in the production firmware — a 10-second factory-reset hold — but that means you need clean, debounced press/hold detection before you can build that feature later.

## You Will Learn

- Active-low button read via `Tiremo_Hal_GPIO`
- Simple hold-duration timing built on Tutorial 02's SysTick

## Prerequisites

- [03-Debug_UART_Logging](../03-Debug_UART_Logging/)
- Toolchain and flashing set up per [Getting Started](../../Getting_Started/)

## Solution

No solution has been written for this tutorial yet — this repo is still a curriculum scaffold, not a finished course.

## Step Outline

1. Poll the USER button GPIO each loop tick
2. Log press and release events with the held duration in ms
3. Confirm a full 10 s hold is detected and logged distinctly from a short press

## Done When

UART log distinguishes a short press from a 10 s+ hold with correct millisecond timing.

<p align="center"><img alt="" src="../../images/banner.png" width="100%" height="16"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
