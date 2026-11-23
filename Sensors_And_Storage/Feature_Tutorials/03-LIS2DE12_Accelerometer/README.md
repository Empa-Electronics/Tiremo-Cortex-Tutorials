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

***Board: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Section: Sensors & Local Storage / Feature Tutorials · Difficulty: Medium***

## Introduction

Unlike SHT40, LIS2DE12 has real configuration surface — full-scale range and output data rate both matter for later fall/shock/vibration alarms, so build the driver to take those as parameters from day one. Tutorial 01 already proved the chip ACKs at `0x19`; this one talks the register map over `WriteRead()`.

## You Will Learn

- LIS2DE12 register map: `WHO_AM_I` (`0x0F` = `0x33`), `CTRL_REG1` (ODR), `CTRL_REG4` (scale), `STATUS_REG` (new-data flag), `OUT_X/Y/Z_H`
- Why this chip needs `TIREMO_HAL_I2C_WriteRead()` (repeated START), not SHT40's STOP-then-read
- Converting left-justified 8-bit samples to mg using the datasheet full-scale sensitivity (`15.6 / 31.2 / 62.5 / 187.5` mg/digit) with integer-only math

## Prerequisites

- [02-SHT40_Temperature_Humidity](../02-SHT40_Temperature_Humidity/) — same I2C HAL and `PRV_I2C_Init()`
- To watch the readings: `tdf monitor` (see [Getting_Started/04-Button_Read_And_Debounce's Solution](../../../Getting_Started/04-Button_Read_And_Debounce/#solution) for setup) or any serial terminal (Tera Term, PuTTY, screen, minicom, …) at 115200 8N1 on the board's USB serial port
- A Tiremo Cortex board (ABOV A34G43A MCU — see [Hardware Overview](../../../Hardware.md)) connected to your PC with a USB cable, with the LIS2DE12 on I2C2. The board's onboard CMSIS-DAP debug probe handles SWD flashing; no separate programmer is needed.

## Understanding WriteRead and the mg conversion

The LIS2DE12 treats the first byte of a transaction as a *register pointer*. A STOP between that write and the data read resets the pointer — so the HAL's `WriteRead()` issues a repeated START instead. Multi-byte reads also set bit 7 of the sub-address (`0x80`) to auto-increment through `OUT_X_H` … `OUT_Z_H`.

Writes on this ABOV peripheral use the same `WriteRead()` with a one-byte dummy read afterwards so the controller ends the frame cleanly — that's a quirk of this HAL, copied from the reference I2C adapter, not a datasheet rule.

8-bit samples are left-justified in a 16-bit word, so the raw 16-bit value is divided by 256 and multiplied by the datasheet sensitivity. To stay integer-only, the driver keeps the sensitivity in tenths of mg/digit and divides by `256 × 10 = 2560`:

| Full scale | Sensitivity (datasheet) | Stored as |
|---|---|---|
| ±2 g | 15.6 mg/digit | `156` |
| ±4 g | 31.2 mg/digit | `312` |
| ±8 g | 62.5 mg/digit | `625` |
| ±16 g | 187.5 mg/digit | `1875` |

```c
mg = (raw16 * sens_tenth_mg) / 2560;   /* ±2g -> 15.6 mg/digit */
```

Each `ReadAccel()` call waits for a fresh sample (`STATUS_REG` `ZYXDA` bit, 200 ms timeout) four times and returns the average, which smooths out the 8-bit quantization noise. At 100 Hz that blocks for roughly 40 ms per call — fine for this tutorial's 500 ms loop.

`Configure(scale_g, data_rate_hz)` programs `CTRL_REG4` FS bits and the `CTRL_REG1` ODR nibble + low-power XYZ enable (`0x0F`), then reads `CTRL_REG1` back to confirm the write. An ODR between supported rates is rounded up to the next one (1, 10, 25, 50, 100, 200, 400 Hz).

Production firmware wraps ST's PID library. This tutorial does not — the learning goal is those few registers.

## Solution

From the repo root:

```sh
cd Sensors_And_Storage/Feature_Tutorials/03-LIS2DE12_Accelerometer/Sw
tdf build
tdf flash
tdf monitor
```

(See [Getting_Started/01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution) for what `tdf` is. On macOS/Linux, `tdf.py` also works as an alias; on Windows, use `tdf`. Close any other terminal on the board's serial port before running `tdf monitor` — only one program can open it at a time.)

No new vendor sources beyond tutorials 01/02. New code is `Drivers/lis2de12`.

**Confirmed on real hardware:** `tdf build` compiles and links without errors on macOS and Windows 11 (`.text` about 15.5 KB; the exact size varies slightly with the toolchain version, and the linker prints the same expected, harmless `_close` / `_lseek` / `_read` / `_write is not implemented` warnings explained in [01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution)). `tdf flash` completed a full program-and-verify cycle over SWD on a real, connected Tiremo Cortex board (`** Verified OK **`). After the boot and `WHO_AM_I` lines, an `X=… mg  Y=… mg  Z=… mg` line follows every 500 ms. An excerpt from a real board, tilted onto one edge, then onto another, then laid back down:

```text
[INFO] LIS2DE12_Accelerometer: boot
[INFO] LIS2DE12: WHO_AM_I=0x33  FS=2g  ODR=100 Hz
...
[INFO] LIS2DE12: X=206 mg  Y=861 mg  Z=507 mg
...
[INFO] LIS2DE12: X=975 mg  Y=-222 mg  Z=-159 mg
...
[INFO] LIS2DE12: X=113 mg  Y=-175 mg  Z=963 mg
[INFO] LIS2DE12: X=117 mg  Y=-171 mg  Z=967 mg
```

With the board lying flat, gravity is on Z at about +1000 mg and X/Y stay small (a hundred mg or so is normal — the board is rarely perfectly level; in the last line above the total magnitude is √(117² + 171² + 967²) ≈ 989 mg, i.e. 1 g). Tilt it onto each edge in turn and the ~1000 mg moves to that axis (positive or negative depending on which way you tilt); halfway between two axes it splits across both. If the sensor doesn't answer at boot, the log shows `[ERROR] LIS2DE12: init failed (WHO_AM_I or Configure)` and the program stops there; a later failed read logs `[WARN] LIS2DE12: read failed`.

## Walkthrough

### 1. Board config — `0x19` on I2C2

[`Sw/Board/board_config.h`](./Sw/Board/board_config.h): `TIREMO_BOARD_LIS2DE12_I2C_BUS` / `_ADDRESS` only.

### 2. `main.c` — same `PRV_I2C_Init()` as 01/02

[`Sw/Src/main.c`](./Sw/Src/main.c) is unchanged from Tutorial 02: `PRV_PORT_Init()`, `PRV_SCU_CLK_Init()`, `PRV_UART_Init()`, `PRV_I2C_Init()` (I2C2, 100 kHz master), then `PRV_USER_Code()`. The LIS2DE12 shares I2C2 with the SHT40, so no new vendor bring-up call is needed.

### 3. Init, Configure, ReadAccel

[`Sw/Drivers/lis2de12/Src/tiremo_lis2de12.c`](./Sw/Drivers/lis2de12/Src/tiremo_lis2de12.c) reads `WHO_AM_I` (up to 10 tries, 10 ms apart) and expects `0x33`, then calls `Configure(2, 100)`. The loop in [`prv_user_code.c`](./Sw/Src/prv_user_code.c) logs the averaged mg values every 500 ms.

### 4. Build, flash, and watch

```sh
tdf build
tdf flash
tdf monitor
```

(Or chained in one command: `tdf build flash monitor`.) See **Solution** above for what the output should look like.

## Done When

`tdf monitor` shows `WHO_AM_I=0x33`, then about +1000 mg on Z with the board lying flat, and the ~1000 mg moves to X and to Y as you tilt the board onto each edge in turn.


<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
