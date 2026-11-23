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
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Microphone: RMS Capture to dB

***Board: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Section: Sensors & Local Storage / Feature Tutorials · Difficulty: Hard***

## Introduction

The hardest local-sensor tutorial: a Timer1-clocked ADC burst, DMA into a buffer, DC-offset removal, RMS, and RMS→dB — plus a small state machine that turns the level into `HIGH_VOICE` / `NORMAL` / `SILENCE` alerts. It's the first tutorial that talks to the vendor `hal_adc` / `hal_timer1` / `hal_pcu` drivers directly instead of `Tiremo_Hal`.

## You Will Learn

- Why burst-DMA capture is not in `Tiremo_Hal_ADC` (that wrapper is one polled conversion)
- Timer1 as the ADC trigger, HPL DMA arm/service, PA0 analog mux
- DC-offset removal + high-pass filter, RMS, and `20·log10(rms)` as integer dB (relative, not calibrated SPL)
- A hysteresis state machine for voice alerts
- `CONFIG_HAL_DMAC` is already `1` in this SDK's `abov_module_config.h`

## Prerequisites

- [04-Battery_Voltage_ADC](../04-Battery_Voltage_ADC/) — same ADC peripheral (single mode there, burst mode here)
- To watch the levels: `tdf monitor` (see [Getting_Started/04-Button_Read_And_Debounce's Solution](../../../Getting_Started/04-Button_Read_And_Debounce/#solution) for setup) or any serial terminal (Tera Term, PuTTY, screen, minicom, …) at 115200 8N1 on the board's USB serial port
- A Tiremo Cortex board (ABOV A34G43A MCU — see [Hardware Overview](../../../Hardware.md)) connected to your PC with a USB cable. The board's onboard CMSIS-DAP debug probe handles SWD flashing; no separate programmer is needed.

## The signal path

The **MP23ABS1** analog MEMS microphone and the **TS922** op-amp stage that amplifies it are both powered from the 2.7 V **VDDA** rail (the same rail [04-Battery_Voltage_ADC](../04-Battery_Voltage_ADC/) measures). The op-amp output (`MIC_IN2`) goes to **PA0 / AN0**, which the firmware reads on **ADC0, channel 0**.

## Understanding the bypass

[`Sw/Drivers/mic/`](./Sw/Drivers/mic/) is copied from the reference firmware (`tiremo_mic_bsp.c` + `tiremo_mic_app.c` + `tiremo_mic_adc_reg.h`). The BSP writes ADC/Timer1/DMA registers that `TIREMO_HAL_ADC_ReadChannel()` never exposes. `board_config.h` therefore uses vendor types (`ADC_ID_0`, `TIMER1_ID_0`, `PCU_ID_A`) — called out in that header.

`PRV_ADC_Init()` configures ADC0 burst + Timer1 trigger on channel 0; `PRV_Timer1_Init()` enables the ADC trigger output. The BSP sets Timer1 for **16 kHz** sampling, starts capture into a DMA buffer, and `TIREMO_MIC_Service()` polls for completed DMA blocks.

## Understanding the processing

Each 12-bit sample goes through these steps in [`tiremo_mic_app.c`](./Sw/Drivers/mic/Src/tiremo_mic_app.c):

1. **DC offset:** a fixed offset (`TIREMO_MIC_ADC_OFFSET`, `1705` counts) is subtracted.
2. **Volume and gain:** the signal is scaled by the input volume (default `80` of 100) and a fixed digital gain of `8`.
3. **High-pass filter:** a one-pole high-pass filter removes whatever DC is left, so small differences in the op-amp's bias point don't matter.
4. **RMS** over a **1024-sample** circular buffer (64 ms at 16 kHz), computed with an integer square root.
5. **Envelope:** the level follows rises quickly (attack ÷2) and falls slowly (release ÷16), so short peaks still register.

**dB** is `TIREMO_MIC_RmsToDb()`: `20·log10(rms) ≈ 6.0206 · log2(rms)`, in integers with linear interpolation between powers of two. It's a relative level, not calibrated SPL.

**Voice alerts** — `TIREMO_MIC_OnVoiceAlert()` is called only when the state changes:

| State | Enter when the level… | Leave when the level… |
|---|---|---|
| `HIGH_VOICE` | ≥ 1000 for 2 buffers in a row | < 700 for 8 buffers in a row (→ `SILENCE` if ≤ 180, else `NORMAL`) |
| `SILENCE` | ≤ 180 for 8 buffers in a row | > 450 (→ `NORMAL`), or ≥ 1000 for 2 buffers (→ `HIGH_VOICE`) |
| `NORMAL` | everything in between | — |

The gap between the enter and leave thresholds (hysteresis) keeps the alert from flickering when the level hovers near a threshold.

## Solution

From the repo root:

```sh
cd Sensors_And_Storage/Feature_Tutorials/05-Microphone_RMS_To_dB/Sw
tdf build
tdf flash
tdf monitor
```

(See [Getting_Started/01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution) for what `tdf` is. On macOS/Linux, `tdf.py` also works as an alias; on Windows, use `tdf`. Close any other terminal on the board's serial port before running `tdf monitor` — only one program can open it at a time.)

**Build notes:** besides the expected, harmless `_close` / `_lseek` / `_read` / `_write is not implemented` linker warnings explained in [01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution), this build also prints two `-Wenum-compare` warnings from the vendor ADC HAL header (`hal_adc_v_01_00_05.h`). They come from unmodified vendor code and are harmless.

**Confirmed on real hardware:** `tdf build` compiles and links on macOS and Windows 11 (`.text` about 18 KB; the exact size varies slightly with the toolchain version). `tdf flash` completed a full program-and-verify cycle over SWD on a real, connected Tiremo Cortex board (`** Verified OK **`). After the boot line and `MIC: capturing (talk near the mic vs silence)`, you get a `MIC: blocks=… rms=… dB=…` status line every second, plus a `MIC alert: …` line whenever the state changes. An excerpt from a real board — quiet, then a few words, then quiet again:

```text
[INFO] MIC: blocks=921850  rms=12  dB=21
[INFO] MIC: blocks=930398  rms=10  dB=19
[INFO] MIC: blocks=938991  rms=23  dB=26
[INFO] MIC alert: NORMAL  rms=806  dB=57
[INFO] MIC alert: HIGH_VOICE  rms=1282  dB=61
[INFO] MIC alert: NORMAL  rms=475  dB=53
[INFO] MIC alert: SILENCE  rms=133  dB=42
```

In a quiet room the level sits around rms 10–40 (about 20–35 dB); talking close to the mic pushes it to rms 1000–3500 (about 60–70 dB). `blocks` keeps climbing as long as DMA capture is running — if it stops, capture has stalled. The `rms` in an alert line is the level at the moment the state changed, so it sits near a threshold rather than at the peak. If the microphone can't be set up, the log shows `[ERROR] MIC: init failed` or `[ERROR] MIC: start failed` and the program stops there.

## Walkthrough

### 1. Board config — vendor IDs

[`Sw/Board/board_config.h`](./Sw/Board/board_config.h): `TIREMO_BOARD_MIC_ADC_ID` = `ADC_ID_0`, channel `0`, `TIREMO_BOARD_MIC_TIMER_ID` = `TIMER1_ID_0`, and the PA0 analog pin mux (`PCU_ID_A`, `PCU_PIN_ID_0`, `PCU_ALT_7`).

### 2. `main.c` — `PRV_ADC_Init()` + `PRV_Timer1_Init()`

[`Sw/Src/main.c`](./Sw/Src/main.c) calls `PRV_PORT_Init()`, `PRV_SCU_CLK_Init()`, `PRV_UART_Init()`, `PRV_ADC_Init()` (ADC0 burst + Timer1 trigger) and `PRV_Timer1_Init()` (ADC trigger output), then `PRV_USER_Code()`.

### 3. The loop

[`Sw/Src/prv_user_code.c`](./Sw/Src/prv_user_code.c) calls `TIREMO_MIC_Init()` and `TIREMO_MIC_Start()`, then loops calling `TIREMO_MIC_Service()` as often as possible and logs `blocks` / `rms` / `dB` once a second. It also overrides the weak `TIREMO_MIC_OnVoiceAlert()` callback to log each `HIGH_VOICE` / `NORMAL` / `SILENCE` change.

### 4. Build, flash, and watch

```sh
tdf build
tdf flash
tdf monitor
```

(Or chained in one command: `tdf build flash monitor`.) See **Solution** above for what the output should look like.

## Done When

`tdf monitor` shows `blocks` climbing every second, a low level (rms tens, ~20–35 dB) and a `SILENCE` alert when the room is quiet, and a `HIGH_VOICE` alert with rms above 1000 (~60 dB or more) when you talk close to the mic.


<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
