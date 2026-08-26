<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
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

## Reference Implementation

This tutorial is one slice of the reference firmware kept in this repo.
Read these before or after attempting the steps below to check your
approach:

- [board_config.h — LED pin map](../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Board/board_config.h)
- [tiremo_hal_gpio.c](../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Tiremo_Hal/Src/tiremo_hal_gpio.c)

## Step Outline

1. Wire up `Tiremo_Hal_GPIO_Init()` for LED1's port/pin as an active-low output
2. Busy-wait toggle it on a fixed interval
3. `./build.sh` then `./flash.sh` and confirm the LED blinks on real hardware

## Done When

LED1 blinks at 1 Hz after a fresh flash, with no debugger attached.

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
