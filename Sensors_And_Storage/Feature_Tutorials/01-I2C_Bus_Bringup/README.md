<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <h1>I2C Bus Bring-up</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# I2C Bus Bring-up

***Board: Tiremo Cortex (ESP32) · Section: Sensors & Local Storage / Feature Tutorials · Difficulty: Easy***

## Introduction

SHT40 and LIS2DE12 share I2C2. Before writing either sensor driver, get the bus itself working and provable — probing a known address is the fastest way to catch a wiring or pull-up problem early.

## You Will Learn

- The `Tiremo_Hal_I2C` wrapper: init, write, read, probe
- Reading a datasheet's 7-bit vs 8-bit address convention (see the LIS2DE12 SA0 note in board_config.h)

## Prerequisites

- [Getting_Started](../../../Getting_Started/) complete
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Reference Implementation

This tutorial is one slice of the reference firmware kept in this repo.
Read these before or after attempting the steps below to check your
approach:

- [tiremo_hal_i2c.c](../../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Tiremo_Hal/Src/tiremo_hal_i2c.c)
- [board_config.h — I2C bus + addresses](../../../referance_only/Tiremo-Secure-Iot-Sw/Tiremo_Lib/Board/board_config.h)

## Step Outline

1. Initialize I2C2 per `TIREMO_BOARD_SHT40_I2C_BUS`
2. Probe the SHT40 address (0x44) and the LIS2DE12 address (0x19) and log ACK/NACK for each
3. Confirm both ACK before moving on to individual sensor drivers

## Done When

Both sensor addresses ACK on I2C2 probe, logged over UART.

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
