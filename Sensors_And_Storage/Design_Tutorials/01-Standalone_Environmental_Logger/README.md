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
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Standalone Environmental Logger

***Board: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Section: Sensors & Local Storage / Design Tutorials · Difficulty: Medium***

## Introduction

Before adding a single line of networking code, prove the full sensor stack works together: SHT40 + LIS2DE12 + battery + mic sampled on a schedule, with the last N readings persisted to flash and dumped over UART on boot.

## You Will Learn

- Composing the Feature Tutorial 02–06 drivers into one SysTick loop (no cloud)
- A flash ring of the last 8 samples (`ENV1` magic + CRC32), replayed oldest-first on boot
- Why persisted timestamps are `seq` + `uptimeMs`, not tutorial 07's RAM wall clock (that clock dies on reset)
- What changes in the I2C layer once two sensors share the bus in one loop

## Prerequisites

- All of [Sensors & Local Storage / Feature Tutorials](../../Feature_Tutorials/) (01–07)
- The setup scripts from [Getting Started](../../../Getting_Started/) already run, so `tdf` is available
- To watch the log: `tdf monitor` or any serial terminal (Tera Term, PuTTY, screen, minicom, …) at 115200 8N1 on the board's USB serial port, **at least 100 columns wide** (see the banner below)
- A Tiremo Cortex board (ABOV A34G43A MCU — see [Hardware Overview](../../../Hardware.md)) connected to your PC with a USB cable. The board's onboard CMSIS-DAP debug probe handles SWD flashing; no separate programmer is needed.

## The boot banner

Design Tutorials open with the Tiremo Cortex wordmark so a bare terminal identifies the board and the running application before any sensor output. Feature Tutorials stay quiet — the banner is what marks a tutorial as an integrated application rather than a single-peripheral exercise.

[`Sw/Common/Src/tiremo_banner.c`](./Sw/Common/Src/tiremo_banner.c) writes straight to the UART with `TIREMO_Debug_PutsLine`, not `TIREMO_LOG_INFO`, so no `[INFO]` prefix breaks up the artwork. It is suppressed when the runtime log level drops below INFO, so `tdf build -DTIREMO_DEBUG_LEVEL=2` gives you a silent boot.

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

Two constraints shaped the artwork. It is **100 columns wide**, so size your terminal to at least that or every row wraps and the wordmark falls apart. And the "blank" rows are a single space rather than truly empty, because `tdf monitor` skips empty lines and would otherwise collapse the gap between the two words.

`tdf monitor` opens the serial port first and then resets the board, so you see the banner and the whole boot, `history` replay included. With another terminal (Tera Term, PuTTY, …), connect first and then press RESET.

## Understanding the ring

[`Sw/Common/Src/tiremo_envlog.c`](./Sw/Common/Src/tiremo_envlog.c) owns data-flash **sector 0** — the same sector tutorial 06 used for `bootCount`, so running this logger replaces that record (and the other way round). Layout is magic `"ENV1"` + version + count + `nextSeq` + 8 `TIREMO_EnvSample_t` + CRC32 (same polynomial as 06). If the sector doesn't hold a valid `ENV1` record — for example the first time you run the logger — the history starts empty.

`seq` is monotonic across resets. `uptimeMs` is SysTick since *this* boot only. Tutorial 07's `TIREMO_Clock` is RAM-only, so it is not what gets stored.

`flags` bits: `0x01` SHT40, `0x02` accel, `0x04` battery. `0x07` means all three reads succeeded; a missing bit means that read failed and its fields are logged as `0`. Mic dB is best-effort from the DMA path (tutorial 05).

The main loop calls `TIREMO_MIC_Service()` every pass and takes a full sample every `SAMPLE_INTERVAL_MS` (3000 ms).

**Flash wear — don't leave it running for days.** Each append erases and rewrites the whole sector, so at one sample every 3 s the sector is erased about 1,200 times an hour (~28,800 a day). Data-flash sectors are only rated for a limited number of erase cycles (see the A34G43A datasheet), so a board left logging around the clock can wear the sector out within days. That's fine for a short tutorial run; a real product logs less often, buffers several samples in RAM before writing, or spreads writes over several sectors (wear levelling).

## Two sensors on one I2C bus

The SHT40 and LIS2DE12 share I2C2 and are read back-to-back every 3 s. That exposes three details in [`Sw/Tiremo_Hal/Src/tiremo_hal_i2c.c`](./Sw/Tiremo_Hal/Src/tiremo_hal_i2c.c) that a single-sensor demo can get away with, and this project's copy of the I2C HAL handles all three:

- **NACK the last byte.** When reading, the ACK/NACK for a byte is decided when the master starts receiving it, so ACK is turned off *before* the last byte is clocked in. If the last byte is ACKed, the sensor keeps driving the next byte onto SDA while the master tries to send STOP, and the bus is left stuck.
- **Actually drive STOP.** After requesting STOP, the pending status flag is cleared so the peripheral releases SCL and the STOP condition reaches the bus.
- **Read-only transfers.** `TIREMO_HAL_I2C_WriteRead()` accepts `txLength = 0` and addresses the device for reading directly. The SHT40 driver uses `WriteRead()` for both its command write and its 6-byte read, so every transfer goes through the HAL's timeout instead of the vendor's blocking `HAL_I2C_Transmit` / `HAL_I2C_Receive`.

Without these, the symptoms are SHT40 reads failing every other sample (`flags=0x04` / `0x05` with `T=0.000`) and the LIS2DE12 never initialising (`acc=0/0/0`).

## Solution

From the repo root:

```sh
cd Sensors_And_Storage/Design_Tutorials/01-Standalone_Environmental_Logger/Sw
tdf build
tdf flash
tdf monitor
```

(See [Getting_Started/01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution) for what `tdf` is. On macOS/Linux, `tdf.py` also works as an alias; on Windows, use `tdf`. Close any other terminal on the board's serial port before running `tdf monitor` — only one program can open it at a time.)

**Build notes:** besides the expected, harmless `_close` / `_lseek` / `_read` / `_write is not implemented` linker warnings explained in [01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution), this build also prints `-Wenum-compare` warnings from the vendor ADC HAL header (`hal_adc_v_01_00_05.h`). They come from unmodified vendor code and are harmless.

**Confirmed on real hardware:** `tdf build` compiles and links without errors on macOS and Windows 11 (`.text` about 28.5 KB; the exact size varies slightly with the toolchain version). `tdf flash` completed a full program-and-verify cycle over SWD on a real, connected Tiremo Cortex board (`** Verified OK **`). After the banner, a first boot on an empty data flash looks like this:

```text
[INFO] Standalone_Environmental_Logger: boot
[INFO] history: (empty)
[INFO] live seq=0 up=4326 ms  T=25.476 C  RH=49.711 %RH  acc=-128/-226/924 mg  AVDD=2647 mV  mic=21 dB  flags=0x07
[INFO] live seq=1 up=7326 ms  T=25.530 C  RH=49.677 %RH  acc=-124/-234/932 mg  AVDD=2649 mV  mic=33 dB  flags=0x07
[INFO] live seq=2 up=10326 ms  T=25.543 C  RH=49.566 %RH  acc=-132/-234/936 mg  AVDD=2647 mV  mic=31 dB  flags=0x07
```

Every line has `flags=0x07`; `AVDD` sits around 2.65–2.7 V (the VDDA rail — see [04-Battery_Voltage_ADC](../../Feature_Tutorials/04-Battery_Voltage_ADC/)); the accelerometer magnitude is about 1 g. Now press RESET (or run `tdf monitor` again). The boot prints `history: 8 sample(s), oldest first`, replays the last eight samples tagged `hist` with their original `seq` and `up` values, and then new `live` samples continue from the next `seq` — that's the persistence proof.

## Walkthrough

1. [`main.c`](./Sw/Src/main.c) — `PRV_I2C_Init`, `PRV_ADC_Init`, `PRV_Timer1_Init` on top of the usual port / clock / UART init
2. [`board_config.h`](./Sw/Board/board_config.h) — union of tutorials 02–05
3. [`prv_user_code.c`](./Sw/Src/prv_user_code.c) — banner, init the flash log and all sensors (a failing sensor logs a `[WARN]` and the logger carries on without it), replay the ring, then sample + append every 3 s
4. [`tiremo_banner.c`](./Sw/Common/Src/tiremo_banner.c) — copy this into any new Design Tutorial's `Common/`
5. Drivers copied from the feature tutorials, plus the I2C HAL details described in **Two sensors on one I2C bus** above

## Done When

Every `live` line shows `flags=0x07`, and after a reset the boot replays the last samples as `hist` lines and then continues `live` with the next `seq` — no samples from before the reset are lost.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
