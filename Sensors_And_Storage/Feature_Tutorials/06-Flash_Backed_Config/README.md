<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/Sensors_And_Storage/Feature_Tutorials/06-Flash_Backed_Config/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Flash-Backed Config</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Flash-Backed Config

***Board: Tiremo Cortex (ESP32) · Section: Sensors & Local Storage / Feature Tutorials · Difficulty: Medium***

## Introduction

WiFi credentials and sensor flags need to survive a power cycle. This tutorial builds the generic persistence mechanism — magic + CRC32 in data flash — that later WiFi / factory-reset tutorials depend on.

## You Will Learn

- `Tiremo_Hal_Flash` (erase/write/read one 2 KB DFMC sector). `CONFIG_HAL_FMC` is already `1` in this SDK
- A `"TRM1"` magic + version + CRC32 (poly `0xEDB88320`) record, same idea as production `TIREMO_APP` config, stripped to `bootCount` + a short `label`

## Prerequisites

- [05-Microphone_RMS_To_dB](../05-Microphone_RMS_To_dB/) conceptually; this `Sw/` is flash-only
- Kit (data flash is on-chip)

## Understanding the record

[`Sw/Common/Src/tiremo_persist.c`](./Sw/Common/Src/tiremo_persist.c) stores one `TIREMO_PersistRecord_t` at data-flash offset 0. Load checks magic, version, then CRC over every byte *before* the `crc32` field. Bad flash (blank, wrong magic, bad CRC) → caller applies defaults. Save is erase-sector then write.

The loop also takes a RAM copy, flips one CRC bit, and asserts `IsValid()` fails — the "deliberately corrupted record" check without destroying the sector.

`tdf.py flash` programs **code** flash; the DFMC record survives it. `tdf.py monitor`'s SWD reset does too. A full chip erase (`tdf.py clean-device`) does not.

## Solution

```sh
cd Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

**Verified:** `.text` 16120 bytes, flash **Verified OK**. After a previous run had already stored a record:

```text
[INFO] persist: loaded bootCount=4 label=tutorial
[INFO] persist: saved bootCount=5 (survives reset)
[INFO] persist: corrupt CRC rejected (expected)
```

`bootCount` climbing across resets/reflashes is the persistence proof. First-ever boot on a blank DFMC logs `no valid record, using defaults` then saves `1`.

## Walkthrough

1. [`tiremo_hal_flash.c`](./Sw/Tiremo_Hal/Src/tiremo_hal_flash.c) — copied from reference
2. [`tiremo_persist.c`](./Sw/Common/Src/tiremo_persist.c) — tutorial-sized blob
3. [`prv_user_code.c`](./Sw/Src/prv_user_code.c) — load / increment / save / corrupt-demo

## Done When

A value written before reset reads back after; corrupt CRC falls back — **both confirmed** (`bootCount` 4→5, corrupt demo rejected).


<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
