<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/Sensors_And_Storage/Feature_Tutorials/05-Microphone_RMS_To_dB/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Microphone: RMS Capture to dB</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Microphone: RMS Capture to dB

***Board: Tiremo Cortex (ESP32) · Section: Sensors & Local Storage / Feature Tutorials · Difficulty: Hard***

## Introduction

Hardest local-sensor tutorial: Timer1-clocked ADC burst, DMA into a buffer, DC-offset removal, RMS→dB. First tutorial that talks to vendor `hal_adc` / `hal_timer1` / `hal_pcu` directly instead of `Tiremo_Hal`.

## You Will Learn

- Why burst-DMA capture is not in `Tiremo_Hal_ADC` (that wrapper is one polled conversion)
- Timer1 as the ADC trigger, HPL DMA arm/service, PA0 analog mux
- DC-offset + RMS and `20·log10(rms)` as integer dB (relative, not calibrated SPL)
- `CONFIG_HAL_DMAC` is already `1` in this SDK's `abov_module_config.h`

## Prerequisites

- [04-Battery_Voltage_ADC](../04-Battery_Voltage_ADC/)
- Kit with the MP23ABS1 on PA0 / ADC0

## Understanding the bypass

[`Sw/Drivers/mic/`](./Sw/Drivers/mic/) is copied from the reference firmware (`tiremo_mic_bsp.c` + `tiremo_mic_app.c` + `tiremo_mic_adc_reg.h`). The BSP writes ADC/Timer1/DMA registers that `TIREMO_HAL_ADC_ReadChannel()` never exposes. `board_config.h` therefore uses vendor types (`ADC_ID_0`, `TIMER1_ID_0`, `PCU_ID_A`) — called out in that header.

`PRV_ADC_Init()` configures ADC0 burst + Timer1 trigger on channel 0; `PRV_Timer1_Init()` enables the ADC trigger output. The BSP then starts capture and `Service()` polls DMA-complete.

dB is `TIREMO_MIC_RmsToDb()`: `20*log10(rms) ≈ 6.0206 * log2(rms)`, integer. Not SPL.

## Solution

```sh
cd Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

**Verified:** `.text` 18316 bytes, flash **Verified OK**. Quiet room:

```text
[INFO] MIC: blocks=18078  rms=18  dB=24
[INFO] MIC: blocks=27122  rms=20  dB=25
```

`blocks` climbing proves DMA is actually completing. Talking near the mic (dB should rise) was **not** confirmed this session — needs a human voice.

## Walkthrough

1. [`main.c`](./Sw/Src/main.c) — `PRV_ADC_Init()` + `PRV_Timer1_Init()`
2. [`prv_user_code.c`](./Sw/Src/prv_user_code.c) — `Init` / `Start` / poll `Service()`, log RMS/dB once a second; `TIREMO_MIC_OnVoiceAlert()` logs HIGH_VOICE / SILENCE

## Done When

dB rises when you talk, settles when silent — **quiet baseline confirmed**; talk-vs-silence not confirmed.


<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
