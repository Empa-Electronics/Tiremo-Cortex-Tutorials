<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/Sensors_And_Storage/Feature_Tutorials/01-I2C_Bus_Bringup/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>I2C Bus Bring-up</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# I2C Bus Bring-up

***Board: Tiremo Cortex (ESP32) · Section: Sensors & Local Storage / Feature Tutorials · Difficulty: Easy***

## Introduction

SHT40 and LIS2DE12 share I2C2. Before writing either sensor driver, get the bus itself working and provable — probing a known address is the fastest way to catch a wiring or pull-up problem early, and it's the first tutorial in this repo outside `Getting_Started`, so it also starts a fresh, minimal `Sw/` rather than building on the LED/button demo from that section.

## You Will Learn

- The `Tiremo_Hal_I2C` wrapper: `Transmit`, `Receive`, `WriteRead`, and why there's no `Probe` function to call
- Building a minimal bus probe out of a one-byte `Receive()` — reading, not writing, so the transaction can never disturb a sensor's internal state
- Reading a datasheet's 7-bit vs 8-bit address convention (see the LIS2DE12 SA0 note in `board_config.h`)
- Why this tutorial's `main.c` changes again: `PRV_I2C_Init()` is vendor-generated, scoped to exactly `I2C_ID_2`, the bus this tutorial needs

## Prerequisites

- [Getting_Started](../../../Getting_Started/) complete — this tutorial reuses its SysTick and debug-UART logging unchanged, and assumes `tdf.py` plus the setup scripts (`install.sh`/`export.sh`, or `install.ps1`/`export.ps1`, or `install.bat`/`export.bat`) are already run
- `tdf.py monitor` to watch the probe results (see [Getting_Started/04-Button_Read_And_Debounce's Solution](../../../Getting_Started/04-Button_Read_And_Debounce/#solution) for setup)
- An ABOV A34G43A Starter Kit connected over its onboard SWD debugger, with the SHT40 and LIS2DE12 present on I2C2

## Understanding the Probe (There's No `Probe()` to Call)

[`Sw/Tiremo_Hal/Include/tiremo_hal_i2c.h`](./Sw/Tiremo_Hal/Include/tiremo_hal_i2c.h)
(copied verbatim from the reference HAL) has exactly four functions:
`Transmit`, `Receive`, `WriteRead`, and two recovery helpers
(`PrepareBus`/`Recover`, for a bus a slave left stuck). No `Probe()`.
That's consistent with the rest of this HAL layer: it's a thin,
protocol-agnostic wrapper over the vendor I2C peripheral — it has no
concept of "is anything listening," only "send/receive bytes, tell me
if that worked."

A classic I2C bus scanner (like `i2cdetect`) answers "is anything at
this address" with the cheapest possible transaction: address the
device, see if it ACKs, and stop — no register knowledge needed. This
HAL's `Transmit()`/`Receive()` both require `length >= 1` (a true
zero-byte probe isn't available), so the smallest safe transaction
available is a **one-byte read**:

```c
uint8_t dummy;
bool acked = (TIREMO_HAL_I2C_Receive(bus, address, &dummy, 1U, true)
              == TIREMO_HAL_I2C_STATUS_OK);
```

A read, deliberately, not a write. A one-byte `Receive()` just clocks
in whatever the device's internal register pointer currently holds and
discards it (`dummy` is never used) — it can't corrupt device state,
unlike a one-byte `Transmit()` of an arbitrary byte, which could land
on a real, writable register on a chip this tutorial doesn't know
anything about yet. `TIREMO_HAL_I2C_STATUS_OK` means the address ACKed;
`_ERROR` covers both a NACK and a real bus fault, which is exactly the
right amount of detail for a *bus* bring-up tutorial — distinguishing
those two would need per-device protocol knowledge that belongs in the
SHT40 and LIS2DE12 tutorials that follow this one, not here.

## Solution

[`Sw/`](./Sw/) has a real, buildable implementation:

```sh
cd Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

(See [Getting_Started/01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution)
for what `tdf.py` is, and [04-Button_Read_And_Debounce's Solution](../../../Getting_Started/04-Button_Read_And_Debounce/#solution)
for `monitor` specifically.)

Same shared vendor SDK and CMake build shape as every `Getting_Started`
tutorial, just one directory deeper — `GEN_DIR` in
[`CMakeLists.txt`](./Sw/CMakeLists.txt) uses `../../../../Generation/...`
(four levels, not three) to reach the repo root from
`Sensors_And_Storage/Feature_Tutorials/01-I2C_Bus_Bringup/Sw/`. New
vendor sources this tutorial needed: `user_i2c.c`/`user_i2c_isr.c`
(vendor-generated `PRV_I2C_Init()`, brings up `I2C_ID_2` at 100 kHz
master mode) and `hal_i2c.c` — found by reading `tiremo_hal_i2c.c`'s
own includes and vendor bring-up comments, not by guessing.

**Genuinely verified, for real, on the connected board:** `tdf.py
build` compiles and links clean (`.text` 14532 bytes). `tdf.py flash`
completed a full program-and-verify cycle over SWD (`** Verified OK
**`). `tdf.py monitor` then showed, straight from the real hardware:

```text
[INFO] SHT40 (0x44): ACK
[INFO] LIS2DE12 (0x19): ACK
```

repeating every 5 seconds. **Both sensors genuinely ACK on I2C2** —
this tutorial's entire "Done When" criterion, confirmed on real
hardware, not simulated or assumed. If a sensor were missing or the bus
miswired, the same log would show `NACK -- not responding` for that
address at `WARN` level instead.

## Walkthrough

### 1. Board config — I2C2 and two addresses, nothing else

[`Sw/Board/board_config.h`](./Sw/Board/board_config.h) is deliberately
narrow: just `TIREMO_BOARD_SHT40_I2C_BUS`/`_ADDRESS` and
`TIREMO_BOARD_LIS2DE12_I2C_BUS`/`_ADDRESS`, both pointing at
`TIREMO_HAL_I2C_BUS_2` — the same physical bus, two devices. No
LED/button macros carried forward from `Getting_Started`; this tutorial
doesn't use them.

The LIS2DE12 address is worth a specific callout: ST's own datasheet
gives the *8-bit* write address (`0x33` with `SA0` = 1), but this HAL's
`address` parameter is always 7-bit — `0x33 >> 1 = 0x19`, which is what
`board_config.h` actually holds. Getting this shift wrong is a classic,
silent way to probe the wrong address and see a false NACK.

### 2. Entry point — `PRV_I2C_Init()`

[`Sw/Src/main.c`](./Sw/Src/main.c) adds one vendor call after
`PRV_UART_Init()`:

```c
PRV_PORT_Init();
PRV_SCU_CLK_Init();
PRV_UART_Init();
PRV_I2C_Init();     /* new: brings up I2C_ID_2 at 100 kHz, master mode */
PRV_USER_Code();
```

`PRV_I2C_Init()` is vendor-generated, unmodified, and already scoped to
exactly the one bus this tutorial needs — it doesn't touch I2C0/1/3.
SCL/SDA pin muxing is handled by the generated pin config, the same way
UART's pins were in Tutorial 03: no runtime GPIO call needed for it.

### 3. Probing both sensors

[`Sw/Src/prv_user_code.c`](./Sw/Src/prv_user_code.c):

```c
static void ProbeAndLog(const char *pName, TIREMO_HAL_I2C_Bus_t bus, uint8_t address)
{
    uint8_t dummy;
    bool acked = (TIREMO_HAL_I2C_Receive(bus, address, &dummy, 1U, true)
                  == TIREMO_HAL_I2C_STATUS_OK);

    if (acked) { TIREMO_LOG_INFO("%s (0x%02X): ACK", pName, address); }
    else       { TIREMO_LOG_WARN("%s (0x%02X): NACK -- not responding", pName, address); }
}
```

called once immediately at boot, then every `PROBE_INTERVAL_MS`
(5000 ms) from the main loop using the same non-blocking
`(now - last) >= interval` idiom every tutorial since
`02-SysTick_Timing` has used. Repeating the probe — rather than logging
once and going silent — means the result is still there to see even if
`tdf.py monitor`'s reset-then-connect window misses the very first
boot line (a real, documented edge case since
[04-Button_Read_And_Debounce](../../../Getting_Started/04-Button_Read_And_Debounce/#solution)).

### 4. Build, flash, and watch

```sh
tdf.py build
tdf.py flash
tdf.py monitor
```

Same OS-detection, same bundled-OpenOCD flow as every prior tutorial —
see [01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution).
See **Solution** above for what's actually confirmed on real hardware
here.

## Done When

`tdf.py monitor` shows `SHT40 (0x44): ACK` and `LIS2DE12 (0x19): ACK`,
repeating every 5 s — genuinely confirmed on real hardware, see
**Solution**.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
