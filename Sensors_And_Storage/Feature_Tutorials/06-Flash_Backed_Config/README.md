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
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Flash-Backed Config

***Board: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Section: Sensors & Local Storage / Feature Tutorials · Difficulty: Medium***

## Introduction

WiFi credentials and sensor flags need to survive a power cycle. This tutorial builds the generic persistence mechanism — magic + CRC32 in on-chip data flash — that later WiFi / factory-reset tutorials depend on.

## You Will Learn

- `Tiremo_Hal_Flash`: erase / write / read one 2 KB sector of the on-chip data flash (DFMC). `CONFIG_HAL_FMC` is already `1` in this SDK
- A `"TRM1"` magic + version + CRC32 (poly `0xEDB88320`) record, same idea as the production `TIREMO_APP` config, stripped to `bootCount` + a short `label`
- Why the record survives a reflash, and why a firmware should only write it when something changed

## Prerequisites

- [05-Microphone_RMS_To_dB](../05-Microphone_RMS_To_dB/) conceptually; this `Sw/` is flash-only (no sensors)
- To watch the log: `tdf monitor` (see [Getting_Started/04-Button_Read_And_Debounce's Solution](../../../Getting_Started/04-Button_Read_And_Debounce/#solution) for setup) or any serial terminal (Tera Term, PuTTY, screen, minicom, …) at 115200 8N1 on the board's USB serial port
- A Tiremo Cortex board (ABOV A34G43A MCU — see [Hardware Overview](../../../Hardware.md)) connected to your PC with a USB cable. The data flash is on-chip; no extra hardware is needed. The board's onboard CMSIS-DAP debug probe handles SWD flashing.

## Where the data lives

The A34G43A has two separate flash areas:

| Area | Address | Used for |
|---|---|---|
| Code flash | `0x00000000` | The firmware image that `tdf flash` programs |
| Data flash (DFMC) | `0x0E000000` | This tutorial's record, at offset `0` of the first 2 KB sector |

Because they're separate, `tdf flash` replaces the firmware without touching the record, and the SWD reset that `tdf monitor` performs doesn't touch it either. `tdf clean-device` mass-erases only the **code** flash range (`0x0`–`0x200000`), which doesn't include the data flash at `0x0E000000` — so the record survives that too. Confirmed on a real board: after `tdf clean-device` the code flash read back as all `0xFF` while the record at `0x0E000000` was still intact, and after `tdf flash` the firmware loaded it and carried on counting. To clear the record, erase its sector from firmware (`TIREMO_HAL_FLASH_EraseSector(0)`).

## Understanding the record

[`Sw/Common/Src/tiremo_persist.c`](./Sw/Common/Src/tiremo_persist.c) stores one 32-byte `TIREMO_PersistRecord_t`:

| Field | Size | Value |
|---|---|---|
| `magic` | 4 bytes | `0x54524D31` (`"TRM1"`) |
| `version` | 2 bytes | `1` |
| `reserved` | 2 bytes | `0` |
| `bootCount` | 4 bytes | incremented on every boot |
| `label` | 16 bytes | `"tutorial"` (NUL-terminated) |
| `crc32` | 4 bytes | CRC32 over every byte **before** this field |

`Load()` reads the record and checks magic, version, then the CRC. Blank flash, a wrong magic or version, or a bad CRC all count as "no valid record", and the caller applies defaults. `Save()` erases the sector, then writes the record with a fresh CRC.

## What the program does

[`Sw/Src/prv_user_code.c`](./Sw/Src/prv_user_code.c) runs these steps **once, at boot**, then idles:

1. Load the record — or fall back to defaults if there's no valid one.
2. Increment `bootCount`, set `label` to `"tutorial"`, and save.
3. Copy the record in RAM, flip one bit of its `crc32`, and check that `IsValid()` rejects it — a "corrupted record" test that doesn't touch the real sector.

So the record is rewritten once per reset, not continuously. Flash sectors only survive a limited number of erase/write cycles, so a real product should only call `Save()` when a value actually changed — not on every boot as this demo does.

## Solution

From the repo root:

```sh
cd Sensors_And_Storage/Feature_Tutorials/06-Flash_Backed_Config/Sw
tdf build
tdf flash
tdf monitor
```

(See [Getting_Started/01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution) for what `tdf` is. On macOS/Linux, `tdf.py` also works as an alias; on Windows, use `tdf`. Close any other terminal on the board's serial port before running `tdf monitor` — only one program can open it at a time.)

**Confirmed on real hardware:** `tdf build` compiles and links without errors on macOS and Windows 11 (`.text` about 16 KB; the exact size varies slightly with the toolchain version, and the linker prints the same expected, harmless `_close` / `_lseek` / `_read` / `_write is not implemented` warnings explained in [01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution)). `tdf flash` completed a full program-and-verify cycle over SWD on a real, connected Tiremo Cortex board (`** Verified OK **`). The log after a reset:

```text
[INFO] Flash_Backed_Config: boot
[INFO] persist: loaded bootCount=1 label=tutorial
[INFO] persist: saved bootCount=2 (survives reset)
[INFO] persist: corrupt CRC rejected (expected)
```

Press RESET (or run `tdf monitor`, which resets the board) and `bootCount` goes up by one each time — that's the persistence proof. On the very first boot with an empty data flash the second line reads `persist: no valid record, using defaults` and the count starts at `1`. If the flash driver can't be set up, the log shows `[ERROR] persist: flash init failed` and the program stops there; a failed write logs `[WARN] persist: save failed`.

## Walkthrough

### 1. `Tiremo_Hal_Flash`

[`Sw/Tiremo_Hal/Src/tiremo_hal_flash.c`](./Sw/Tiremo_Hal/Src/tiremo_hal_flash.c), copied from the reference HAL, wraps the vendor `HAL_FMC_*` calls on `FMC_ID_DFMC`: erase one 2 KB-aligned sector, write, read. Offsets are relative to the start of the data flash.

### 2. `tiremo_persist.c`

[`Sw/Common/Src/tiremo_persist.c`](./Sw/Common/Src/tiremo_persist.c) is the tutorial-sized record logic: `Init`, `Defaults`, `Load`, `Save`, `IsValid`, and the bit-by-bit CRC32 (poly `0xEDB88320`, init/final XOR `0xFFFFFFFF`).

### 3. `prv_user_code.c`

[`Sw/Src/prv_user_code.c`](./Sw/Src/prv_user_code.c) — load / increment / save / corrupt-demo, once at boot (see **What the program does** above).

### 4. Build, flash, and watch

```sh
tdf build
tdf flash
tdf monitor
```

(Or chained in one command: `tdf build flash monitor`.) See **Solution** above for what the output should look like.

## Done When

After each reset the log shows `persist: loaded bootCount=N` followed by `saved bootCount=N+1`, `bootCount` keeps counting up across resets, `tdf flash` and `tdf clean-device`, and every boot ends with `persist: corrupt CRC rejected (expected)`.


<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
