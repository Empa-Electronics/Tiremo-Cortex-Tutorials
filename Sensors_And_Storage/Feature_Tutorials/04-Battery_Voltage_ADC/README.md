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

***Board: Tiremo Cortex (ESP32) · Section: Sensors & Local Storage / Feature Tutorials · Difficulty: Medium***

## Introduction

There's no dedicated battery-sense pin — supply voltage is inferred by measuring the internal VCORE bandgap against AVDD. Good introduction to `Tiremo_Hal_ADC` before the microphone needs the same peripheral in burst mode.

## You Will Learn

- The `Tiremo_Hal_ADC` wrapper (`TIREMO_HAL_ADC_ReadChannel`)
- `AVDD_mV = (VCORE_REF_MV * 4096) / vcoreRaw` with `VCORE_REF_MV = 1000`

## Prerequisites

- [03-LIS2DE12_Accelerometer](../03-LIS2DE12_Accelerometer/) conceptually; this `Sw/` is a fresh project (ADC, no I2C)
- An ABOV A34G43A Starter Kit

## Understanding the bandgap trick

[`Sw/Drivers/battery/Src/tiremo_battery.c`](./Sw/Drivers/battery/Src/tiremo_battery.c) is copied from the reference driver. Channel 23 on ADC1 is the internal VCORE reference (~1.0 V). The 12-bit converter scales that against AVDD, so a larger raw code means a *smaller* computed AVDD.

`PRV_ADC_Init()` (vendor-generated) brings up ADC1 in single / software-trigger mode — that's the converter this tutorial uses. It also brings up ADC0 in burst/Timer1 mode for the next tutorial; this one never starts it.

## Solution

```sh
cd Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

**Verified:** `.text` 14436 bytes, flash **Verified OK**. Live log:

```text
[INFO] BAT: VCORE raw=2032  AVDD=2015 mV
```

Stable across repeats; `1000 * 4096 / 2032 = 2015`. A multimeter check against the same rail (the tutorial's full "Done When", ±50 mV) was **not** done this session — plug in a meter before trusting the absolute number. ~2.0 V is what this kit produced with the reference formula, not a simulated value.

## Walkthrough

1. [`board_config.h`](./Sw/Board/board_config.h) — `TIREMO_BOARD_BATTERY_ADC_ID` = ADC1, channel 23
2. [`main.c`](./Sw/Src/main.c) — `PRV_ADC_Init()`
3. [`prv_user_code.c`](./Sw/Src/prv_user_code.c) — 1 Hz `TIREMO_BAT_Read()`

## Done When

Computed AVDD within ~50 mV of a multimeter — **absolute mV not meter-checked**; raw/formula path confirmed live.


<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
