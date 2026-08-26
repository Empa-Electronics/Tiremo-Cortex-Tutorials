<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../images/tiremo_sub_brand_1_mono_black.png" width="220">
      </picture>
      <br>
      <img alt="" src="../../images/banner.png" width="100%">
      <h1>Debug UART Logging</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Debug UART Logging

***Board: Tiremo Cortex (ESP32) · Section: Getting Started / Getting Started · Difficulty: Easy***

## Introduction

Every tutorial from here on will be debugged over a UART log, not a debugger breakpoint — that's how the reference firmware itself was built. Get `TIREMO_LOG_*` working now.

## You Will Learn

- `TIREMO_LOG_ERROR/WARN/INFO/DEBUG` and the `TIREMO_DEBUG_LEVEL` compile-time ceiling
- Overriding verbosity with `make TIREMO_DEBUG_LEVEL=N`
- Why runtime `TIREMO_Debug_SetLevel()` can only go quieter, never louder

## Prerequisites

- [02-SysTick_Timing](../02-SysTick_Timing/)
- Toolchain and flashing set up per [Getting Started](../../Getting_Started/)

## Reference Implementation

This tutorial is one slice of the reference firmware kept in this repo.
Read these before or after attempting the steps below to check your
approach:

- [tiremo_debug.h](../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Common/Include/tiremo_debug.h)
- [tiremo_debug.c](../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Common/Src/tiremo_debug.c)
- [README.md — Debug verbosity](../../referance_only/Tiremo-Secure-Iot-Sw/README.md)

## Step Outline

1. Bring up the debug UART at boot
2. Replace ad-hoc printouts in Tutorial 02 with `TIREMO_LOG_INFO`
3. Rebuild at `TIREMO_DEBUG_LEVEL=2` and `=4` and confirm verbosity actually changes

## Done When

Boot log shows leveled `[INFO]` lines, and rebuilding at a lower debug level quiets them without touching source.

<p align="center"><img alt="" src="../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
