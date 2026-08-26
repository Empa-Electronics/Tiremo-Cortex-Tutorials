<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
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

## Reference Implementation

This tutorial is one slice of the reference firmware kept in this repo.
Read these before or after attempting the steps below to check your
approach:

- [tiremo_led.c](../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Drivers/led/Src/tiremo_led.c)
- [board_config.h — LED_COUNT / active-low](../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Board/board_config.h)

## Step Outline

1. Implement `solid`, `blink`, and `pulse` pattern functions taking an LED index and interval
2. Cycle all ten LEDs through the three patterns on a timer
3. Wire the USER button to cycle patterns manually, combining Tutorials 04 and 05

## Done When

All ten LEDs run independently addressable solid/blink/pulse patterns, switchable via button press.

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
