<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/Sensors_And_Storage/Feature_Tutorials/03-LIS2DE12_Accelerometer/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>LIS2DE12 Accelerometer</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# LIS2DE12 Accelerometer

***Board: Tiremo Cortex (ESP32) · Section: Sensors & Local Storage / Feature Tutorials · Difficulty: Medium***

## Introduction

Unlike SHT40, LIS2DE12 has real configuration surface — full-scale range and output data rate both matter for later fall/shock/vibration alarms, so build the driver to take those as parameters from day one. Tutorial 01 already proved the chip ACKs at `0x19`; this one talks the register map over `WriteRead()`.

## You Will Learn

- LIS2DE12 register map: `WHO_AM_I` (`0x0F` = `0x33`), `CTRL_REG1` (ODR), `CTRL_REG4` (scale), `OUT_X/Y/Z_H`
- Why this chip needs `TIREMO_HAL_I2C_WriteRead()` (repeated START), not SHT40's STOP-then-read
- Converting left-justified 8-bit samples to mg using the selected full-scale sensitivity (`16/32/64/192` mg/digit)

## Prerequisites

- [02-SHT40_Temperature_Humidity](../02-SHT40_Temperature_Humidity/) — same I2C HAL and `PRV_I2C_Init()`
- `tdf.py monitor` (see [04-Button_Read_And_Debounce's Solution](../../../Getting_Started/04-Button_Read_And_Debounce/#solution))
- An ABOV A34G43A Starter Kit with the LIS2DE12 on I2C2

## Understanding WriteRead and the mg conversion

The LIS2DE12 treats the first byte of a transaction as a *register pointer*. A STOP between that write and the data read resets the pointer — so the HAL's `WriteRead()` issues a repeated START instead. Multi-byte reads also set bit 7 of the sub-address (`0x80`) to auto-increment through `OUT_X_H` … `OUT_Z_H`.

Writes on this ABOV peripheral use the same `WriteRead()` with a one-byte dummy read afterwards so the controller ends the frame cleanly — that's a quirk of this HAL, copied from the reference I2C adapter, not a datasheet rule.

8-bit samples are left-justified in a 16-bit word. The reference conversion (and this driver) is:

```c
mg = (raw16 * sens_mg) / 256;   /* ±2g -> 16 mg/digit */
```

`Configure(scale_g, data_rate_hz)` programs `CTRL_REG4` FS bits and `CTRL_REG1` ODR nibble + low-power XYZ enable (`0x0F`), then read-backs `CTRL_REG1`.

Production firmware wraps ST's PID library. This tutorial does not — the learning goal is those three registers.

## Solution

```sh
cd Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

No new vendor sources beyond tutorial 01/02. New code is `Drivers/lis2de12`.

**Genuinely verified on the connected board:** `tdf.py build` links clean (`.text` 15708 bytes). `tdf.py flash` **Verified OK**. `tdf.py monitor` showed:

```text
[INFO] LIS2DE12: WHO_AM_I=0x33  FS=2g  ODR=100 Hz
[INFO] LIS2DE12: X=4 mg  Y=-28 mg  Z=1036 mg
[INFO] LIS2DE12: X=0 mg  Y=-20 mg  Z=1036 mg
```

Board sitting flat: gravity is on Z at ~1000 mg, X/Y near zero. Tilting each axis in turn (the full "Done When") still needs hands on the board — not confirmed this session.

## Walkthrough

### 1. Board config — `0x19` on I2C2

[`Sw/Board/board_config.h`](./Sw/Board/board_config.h): `TIREMO_BOARD_LIS2DE12_I2C_BUS` / `_ADDRESS` only.

### 2. `main.c` — same `PRV_I2C_Init()` as 01/02

### 3. Init, Configure, ReadAccel

[`Sw/Drivers/lis2de12/Src/tiremo_lis2de12.c`](./Sw/Drivers/lis2de12/Src/tiremo_lis2de12.c) probes `WHO_AM_I`, then `Configure(2, 100)`. The loop in [`prv_user_code.c`](./Sw/Src/prv_user_code.c) logs mg every 500 ms.

## Done When

Tilting the board on each axis in turn shows ~1000 mg on that axis — **Z-at-rest ~1036 mg confirmed**; X/Y tilt not confirmed (needs hands on the board).


<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
