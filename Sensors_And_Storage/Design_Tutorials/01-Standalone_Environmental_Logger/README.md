<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/Sensors_And_Storage/Design_Tutorials/01-Standalone_Environmental_Logger/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Standalone Environmental Logger</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Standalone Environmental Logger

***Board: Tiremo Cortex (ESP32) · Section: Sensors & Local Storage / Design Tutorials · Difficulty: Medium***

## Introduction

Before adding a single line of networking code, prove the full sensor stack works together: SHT40 + LIS2DE12 + battery + mic sampled on a schedule, with the last N readings persisted to flash and dumped over UART on boot.

## You Will Learn

- Composing the Feature Tutorial 02–06 drivers into one SysTick loop (no cloud)
- A flash ring of the last 8 samples (`ENV1` magic + CRC32), replayed oldest-first on boot
- Why persisted timestamps are `seq` + `uptimeMs`, not tutorial 07's RAM wall clock (that clock dies on reset)

## Prerequisites

- All of [Sensors & Local Storage / Feature Tutorials](../../Feature_Tutorials/) (01–07)
- `tdf.py` set up per [Getting Started](../../../Getting_Started/)
- An ABOV A34G43A Starter Kit

## The boot banner

Design Tutorials open with the Tiremo Cortex wordmark so a bare terminal identifies the board and the running application before any sensor output. Feature Tutorials stay quiet — the banner is what marks a tutorial as an integrated application rather than a single-peripheral exercise.

[`Sw/Common/Src/tiremo_banner.c`](./Sw/Common/Src/tiremo_banner.c) writes straight to the UART with `TIREMO_Debug_PutsLine`, not `TIREMO_LOG_INFO`, so no `[INFO]` prefix breaks up the artwork. It is suppressed when the runtime log level drops below INFO, so `tdf.py build -D TIREMO_DEBUG_LEVEL=2` gives you a silent boot.

```text
              ####
   #####      ####
   #####
   #####
############ #####   ###########   ###########     ############### ###########       ############
############ #####   ########### ###############   ############################    ###############
   #####     #####   ######     #####       #####  ######     #######     #####   ######      #####
   #####     #####   #####      #################  #####       #####       ##### ######       ######
   #####     #####   #####      #################  #####       #####       ##### #####         #####
   #####     #####   #####      #####              #####       #####       #####  #####       ######
   ######### #####   #####      ######    ######   #####       #####       #####  #######   #######
    ######## #####   #####        #############    #####       #####       #####    #############
      ###### #####   #####          #########      #####       #####       #####      #########


                                                                    ###
                                   ######       ######    ###  ## ########    #####    ####    #####
                                 ##########  ###########  ####### ######## ###########  ####  ####
                                ####         ###     #### ####      ###   ####     ####   #######
                                ###         ####      ### ####      ###   #############    ####
                                ####        ####      ### ####      ###   ####            #######
                                 ##########  ###########  ####      #####  ###########  ####  ####
                                   #######     #######    ####       ####    #######   ####    #####

  Standalone Environmental Logger
  Design Tutorial - Empa Electronics
```

Two constraints shaped the artwork. It is **100 columns wide**, so size your terminal to at least that or every row wraps and the wordmark falls apart. And the "blank" rows are a single space rather than truly empty, because `tdf.py monitor` skips empty lines (`tools/tdf.py:214`) and would otherwise collapse the gap between the two words.

Note that `tdf.py monitor` resets the board *then* opens the serial port, so it usually misses the banner — the same race that swallows the first `hist` lines. To watch a boot from the beginning, open the port first and reset separately from another terminal with `tdf.py reset-device`.

## Understanding the ring

[`Sw/Common/Src/tiremo_envlog.c`](./Sw/Common/Src/tiremo_envlog.c) owns data-flash **sector 0** — the same sector tutorial 06 used for `bootCount`. Flashing this logger replaces that record. Layout is magic `"ENV1"` + version + count + `nextSeq` + 8 `TIREMO_EnvSample_t` + CRC32 (same polynomial as 06). Each append erase+writes the whole sector (fine for a tutorial; not a wear-levelled filesystem).

`seq` is monotonic across resets. `uptimeMs` is SysTick since *this* boot only. Tutorial 07's `TIREMO_Clock` is RAM-only, so it is not what gets stored.

`flags` bits: `0x01` SHT40, `0x02` accel, `0x04` battery. `0x07` means all three reads succeeded. Mic dB is best-effort from the DMA path (tutorial 05).

`flags=0x01` with `acc=0/0/0` and `AVDD=0 mV` means LIS2DE12 and battery never passed `Init()` — SHT40 `Read()` still works without init, so temperature/humidity look healthy while the other two stay at zero. Call `TIREMO_SHT40_Init`, `TIREMO_LIS2DE12_Init`, and `TIREMO_BAT_Init` before the sample loop.

The main loop calls `TIREMO_MIC_Service()` every pass and takes a full sample every `SAMPLE_INTERVAL_MS` (3000 ms).

## Solution

```sh
cd Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

Let it log a few `live` lines, Ctrl+C, then `tdf.py monitor` again (SWD reset, data flash intact).

**Verified on the connected board:** `.text` 29752 bytes, flash **Verified OK**. Live sample:

```text
[INFO] live seq=155 up=4245 ms  T=32.913 C  RH=54.287 %RH  acc=128/8/1012 mg  AVDD=2015 mV  mic=37 dB  flags=0x07
```

After reset, history replayed then sampling resumed (later `seq` continues):

```text
[INFO] hist seq=2 up=10168 ms  T=32.598 C  RH=54.869 %RH  acc=4/-32/1056 mg  AVDD=2015 mV  mic=31 dB  flags=0x07
[INFO] live seq=21 up=3087 ms  T=32.398 C  RH=54.775 %RH  ...
```

(`tdf.py monitor` can miss the earliest `hist` lines in the reset-then-open window; the `hist` line above is from a real replay. `seq` climbing across that reset is the persistence proof.)

## Walkthrough

1. [`main.c`](./Sw/Src/main.c) — `PRV_I2C_Init`, `PRV_ADC_Init`, `PRV_Timer1_Init`
2. [`board_config.h`](./Sw/Board/board_config.h) — union of tutorials 02–05
3. [`prv_user_code.c`](./Sw/Src/prv_user_code.c) — banner, init sensors, dump ring, sample + append
4. [`tiremo_banner.c`](./Sw/Common/Src/tiremo_banner.c) — copy this into any new Design Tutorial's `Common/`
5. Drivers copied from the feature tutorials, not rewritten

## Done When

Reset mid-run does not lose the last several samples; UART replays them as `hist` then continues `live` — **confirmed** (`seq` survived reset; `flags=0x07` on live samples).

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
