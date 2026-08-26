<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <h1>SysTick Timing</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# SysTick Timing

***Board: Tiremo Cortex (ESP32) · Section: Getting Started / Getting Started · Difficulty: Easy***

## Introduction

Every later tutorial — WiFi retries, MQTT polling, alarm debouncing — needs a free-running millisecond clock instead of a busy-wait. This tutorial brings up SysTick once so nothing downstream has to re-derive it.

## You Will Learn

- `TIREMO_SysTick_Init()` / `GetMs()` / `DelayMs()`
- Why elapsed-time math (`now - start < timeout`) is safer than nested delays

## Prerequisites

- [01-Hello_Blink](../01-Hello_Blink/)
- Toolchain and flashing set up per [Getting Started](../../Getting_Started/)

## Reference Implementation

This tutorial is one slice of the reference firmware kept in this repo.
Read these before or after attempting the steps below to check your
approach:

- [tiremo_systick.c](../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Common/Src/tiremo_systick.c)
- [tiremo_app.c — the for(;;) run loop shape](../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Tiremo_Core/Tiremo_App/Src/tiremo_app.c)

## Step Outline

1. Initialize SysTick at boot
2. Rewrite Tutorial 01's blink using `GetMs()` elapsed-time comparison instead of a busy delay loop
3. Add a second LED blinking at a different rate to prove both timers run off the same tick

## Done When

Two LEDs blink at independent rates from a single `TIREMO_APP_Run()`-style loop, no nested delays.

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
