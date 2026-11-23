<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/Sensors_And_Storage/Feature_Tutorials/04-Battery_Voltage_ADC/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Battery Voltage via ADC</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Battery Voltage via ADC

***Board: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Section: Sensors & Local Storage / Feature Tutorials · Difficulty: Medium***

## Introduction

There's no dedicated battery-sense pin on this board. Instead, the firmware infers the MCU's **analog supply voltage (AVDD, the VDDA rail)** by measuring the chip's internal VCORE reference against it. This is the supply-voltage monitor the rest of the firmware reports as "battery" — so it's worth being precise about what it really measures (see below). It's also a good introduction to `Tiremo_Hal_ADC` before the microphone tutorial uses the same peripheral in burst mode.

## You Will Learn

- The `Tiremo_Hal_ADC` wrapper (`TIREMO_HAL_ADC_ReadChannel`)
- How an internal reference channel lets you compute the ADC's own supply: `AVDD_mV = (VCORE_REF_MV * 4096) / vcoreRaw` with `VCORE_REF_MV = 1000`
- What that number does — and doesn't — tell you about the battery

## Prerequisites

- [03-LIS2DE12_Accelerometer](../03-LIS2DE12_Accelerometer/) conceptually; this `Sw/` is a fresh project (ADC, no I2C)
- To watch the readings: `tdf monitor` (see [Getting_Started/04-Button_Read_And_Debounce's Solution](../../../Getting_Started/04-Button_Read_And_Debounce/#solution) for setup) or any serial terminal (Tera Term, PuTTY, screen, minicom, …) at 115200 8N1 on the board's USB serial port
- A Tiremo Cortex board (ABOV A34G43A MCU — see [Hardware Overview](../../../Hardware.md)) connected to your PC with a USB cable. The board's onboard CMSIS-DAP debug probe handles SWD flashing; no separate programmer is needed.
- Optional: a multimeter, to check the result against the real VDDA rail

## Understanding the reference trick

[`Sw/Drivers/battery/Src/tiremo_battery.c`](./Sw/Drivers/battery/Src/tiremo_battery.c) is copied from the reference driver. Channel 23 on ADC1 is internal — it has no external pin — and connects to the chip's VCORE reference (~1.0 V). The 12-bit converter measures every input as a fraction of its own supply, AVDD, so:

```text
vcoreRaw = VCORE / AVDD * 4096   ->   AVDD = VCORE * 4096 / vcoreRaw
```

A larger raw code therefore means a *smaller* AVDD. With `VCORE_REF_MV = 1000`, a raw value of `1520` gives `1000 * 4096 / 1520 = 2694 mV`.

**What it measures on this board.** AVDD (MCU pin 11) is fed from the **VDDA** rail, which an **LDK130M-R** adjustable LDO generates at **2.7 V** for the analog microphone (`R1 = 40K + 7.5K`, `R2 = 20K`, `VO = 0.8 V × (1 + R1/R2) = 2.70 V`). That LDO is itself fed from the 3.3 V `VDD` rail (USB 5 V → LD1117S33). So on a USB-powered board, this tutorial reads a stable ~2.7 V: it tells you the analog supply is healthy, not how full a battery is. A battery would only show up here once it sagged far enough that the regulators could no longer hold their outputs.

`TIREMO_BAT_Init()` busy-waits (500 000 loops) before the first read so the internal reference can settle. `PRV_ADC_Init()` (vendor-generated) brings up ADC1 in single / software-trigger mode — the converter this tutorial uses. It also brings up ADC0 in burst/Timer1 mode for the next tutorial; this one never starts it.

## Solution

From the repo root:

```sh
cd Sensors_And_Storage/Feature_Tutorials/04-Battery_Voltage_ADC/Sw
tdf build
tdf flash
tdf monitor
```

(See [Getting_Started/01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution) for what `tdf` is. On macOS/Linux, `tdf.py` also works as an alias; on Windows, use `tdf`. Close any other terminal on the board's serial port before running `tdf monitor` — only one program can open it at a time.)

**Build notes:** besides the expected, harmless `_close` / `_lseek` / `_read` / `_write is not implemented` linker warnings explained in [01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution), this build also prints two `-Wenum-compare` warnings from the vendor ADC HAL header (`hal_adc_v_01_00_05.h`). They come from unmodified vendor code and are harmless.

**Confirmed on real hardware:** `tdf build` compiles and links on macOS and Windows 11 (`.text` about 14 KB; the exact size varies slightly with the toolchain version). `tdf flash` completed a full program-and-verify cycle over SWD on a real, connected Tiremo Cortex board (`** Verified OK **`). The log:

```text
[INFO] Battery_Voltage_ADC: boot
[INFO] BAT: VCORE raw=1520  AVDD=2694 mV
[INFO] BAT: VCORE raw=1520  AVDD=2694 mV
[INFO] BAT: VCORE raw=1520  AVDD=2694 mV
```

with a new line every second. A multimeter on the same board's VDDA rail (C16, or MCU pin 11) read **2.69 V** — the firmware result matches the real rail.

## Walkthrough

### 1. Board config — ADC1, channel 23

[`Sw/Board/board_config.h`](./Sw/Board/board_config.h): `TIREMO_BOARD_BATTERY_ADC_ID` = `TIREMO_HAL_ADC_ID_1`, `TIREMO_BOARD_BATTERY_VCORE_CHANNEL` = `23`. No GPIO is involved — the channel is internal.

### 2. `main.c` — `PRV_ADC_Init()`

[`Sw/Src/main.c`](./Sw/Src/main.c) calls `PRV_PORT_Init()`, `PRV_SCU_CLK_Init()`, `PRV_UART_Init()`, then `PRV_ADC_Init()` (vendor-generated ADC bring-up) before `PRV_USER_Code()`. There's no I2C in this project.

### 3. The loop — 1 Hz `TIREMO_BAT_Read()`

[`Sw/Src/prv_user_code.c`](./Sw/Src/prv_user_code.c) calls `TIREMO_BAT_Init()` once (logging `[ERROR] BAT: init failed` and stopping if it fails), then `TIREMO_BAT_Read()` at boot and every 1000 ms, logging both the raw code and the computed AVDD. A failed read logs `[WARN] BAT: read failed`.

### 4. Build, flash, and watch

```sh
tdf build
tdf flash
tdf monitor
```

(Or chained in one command: `tdf build flash monitor`.) See **Solution** above for what the output should look like.

## Done When

`tdf monitor` shows `BAT: VCORE raw=…  AVDD=… mV` once per second with AVDD around 2.7 V, and — if you have a multimeter — the value is within ~50 mV of the VDDA rail measured at C16 or MCU pin 11.


<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
