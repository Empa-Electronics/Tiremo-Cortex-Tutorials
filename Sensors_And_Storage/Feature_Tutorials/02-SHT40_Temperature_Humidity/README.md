<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/Sensors_And_Storage/Feature_Tutorials/02-SHT40_Temperature_Humidity/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>SHT40 Temperature / Humidity</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# SHT40 Temperature / Humidity

***Board: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Section: Sensors & Local Storage / Feature Tutorials · Difficulty: Easy***

## Introduction

SHT40 is the first real sensor driver in the stack, and the simplest — a single I2C transaction sequence returns both temperature and humidity. It's also the sensor the shadow schema (Tiremo Cloud) will later tune via `unit` and `offset`. Tutorial 01 already proved the chip ACKs at `0x44` on I2C2; this one teaches the actual SHT4x protocol on that address.

## You Will Learn

- The Sensirion SHT4x measurement command/response protocol: command `0xFD`, wait, six bytes with CRC
- Why this chip uses a STOP-then-read, not `WriteRead()` (that's the LIS2DE12 tutorial)
- Converting raw ticks to milli-°C / milli-%RH with integer math — no floating point, no 64-bit multiply
- Why the driver lives in `Drivers/sht40` and the 1 Hz loop stays in `prv_user_code.c`

## Prerequisites

- [01-I2C_Bus_Bringup](../01-I2C_Bus_Bringup/) — this tutorial reuses that `Sw/`'s SysTick, debug UART, I2C HAL, and `PRV_I2C_Init()` unchanged
- To watch the readings: `tdf monitor` (see [Getting_Started/04-Button_Read_And_Debounce's Solution](../../../Getting_Started/04-Button_Read_And_Debounce/#solution) for setup) or any serial terminal (Tera Term, PuTTY, screen, minicom, …) at 115200 8N1 on the board's USB serial port
- A Tiremo Cortex board (ABOV A34G43A MCU — see [Hardware Overview](../../../Hardware.md)) connected to your PC with a USB cable, with the SHT40 present on I2C2. The board's onboard CMSIS-DAP debug probe handles SWD flashing; no separate programmer is needed.

## Understanding the SHT4x Protocol

The production firmware wraps Sensirion's own C library
(`sht4x_measure_high_precision()`). This tutorial does **not** — the
learning goal is the command/response itself, so
[`Sw/Drivers/sht40/Src/tiremo_sht40.c`](./Sw/Drivers/sht40/Src/tiremo_sht40.c)
implements it directly on `Tiremo_Hal_I2C`. Three facts from the
datasheet are the whole driver:

1. **Command `0xFD`** — "measure T & RH, high precision." One byte,
   written with `TIREMO_HAL_I2C_Transmit()`, then a STOP. Conversion
   happens while the bus is idle.
2. **Wait, then read six bytes.** Typical conversion is 6.9 ms, max
   8.3 ms; this driver waits 10 ms (same margin Sensirion's own driver
   uses). A read issued too early NACKs. The response is
   `[T_MSB, T_LSB, T_CRC, RH_MSB, RH_LSB, RH_CRC]`.
3. **CRC-8, polynomial `0x31`, init `0xFF`** — one CRC over the two
   temperature bytes, one over the two humidity bytes. A mismatch means
   the reading is garbage, not "close enough."

That STOP-then-wait-then-read is not `TIREMO_HAL_I2C_WriteRead()`.
`WriteRead()` issues a repeated START between the write and the read,
which the LIS2DE12 needs because it treats the write as a register
pointer. The SHT4x treats the write as "start a conversion"; a
repeated START would interrupt it. Two chips, same bus, two
transaction shapes — that's why the HAL has both APIs.

Conversion, from the datasheet:

```text
T  (°C)  = -45 + 175 * ticks / 65535
RH (%RH) =  -6 + 125 * ticks / 65535
```

`175000 * 65535` does not fit in 32 bits, and this firmware has no
hardware FPU in the flags it actually compiles with (`nofp` newlib
nano). Sensirion's integer form is the one this driver uses:

```c
temp_mC  = ((21875 * (int32_t)ticks) >> 13) - 45000;  /* milli-degC */
rh_mRh   = ((15625 * (int32_t)ticks) >> 13) -  6000;  /* milli-%RH  */
```

(`175000/65535 ≈ 21875/2^13`, same for humidity.) Relative humidity is
then clamped to `[0, 100000]` milli-%RH — the formula can land slightly
outside 0–100 near the edges, which the datasheet itself warns about.

Milli-units, not floats, are what later tutorials will put in JSON and
the device shadow. Logging prints them as `33.282 C` by splitting
`milli / 1000` and `milli % 1000` in application code — the driver
never formats a string.

`TIREMO_SHT40_Read()` is blocking for those ~10 ms. At 1 Hz that is
harmless; a tight inner loop would split "start measure" from "collect
result" instead. This tutorial keeps the one-shot API so the protocol
stays one function you can read top to bottom.

## Solution

[`Sw/`](./Sw/) has a real, buildable implementation:

```sh
cd Sensors_And_Storage/Feature_Tutorials/02-SHT40_Temperature_Humidity/Sw
tdf build
tdf flash
tdf monitor
```

(Run from the repo root. See [Getting_Started/01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution)
for what `tdf` is, and [04-Button_Read_And_Debounce's Solution](../../../Getting_Started/04-Button_Read_And_Debounce/#solution)
for `monitor` specifically. On macOS/Linux, `tdf.py` also works as an
alias; on Windows, use `tdf`. Close any other terminal on the board's
serial port before running `tdf monitor` — only one program can open
it at a time.)

Same shared vendor SDK and CMake shape as
[01-I2C_Bus_Bringup](../01-I2C_Bus_Bringup/) — `GEN_DIR` still uses
`../../../../Generation/...`. No new vendor sources: I2C2 is already
up. What's new is `Drivers/sht40`, this repo's first I2C device driver
(the button and LED drivers in `Getting_Started` 04 and 05 were GPIO).

**Confirmed on real hardware:** `tdf build` compiles and links without
errors on macOS and Windows 11 (`.text` about 15.5 KB; the exact size
varies slightly with the toolchain version, and the linker prints the
same expected, harmless `_close` / `_lseek` / `_read` / `_write is not
implemented` warnings explained in [01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution)).
`tdf flash` completed a full program-and-verify cycle over SWD on a
real, connected Tiremo Cortex board (`** Verified OK **`), and the log
looks like this:

```text
[INFO] SHT40_Temperature_Humidity: boot
[INFO] SHT40 T: 27.410 C
[INFO] SHT40 RH: 34.559 %RH
[INFO] SHT40 T: 27.375 C
[INFO] SHT40 RH: 34.393 %RH
[INFO] SHT40 T: 27.367 C
[INFO] SHT40 RH: 34.281 %RH
```

with a new T/RH pair every second. Between successive samples the
values only wander in the last digit or two — that's normal sensor
noise. Your exact values depend on the room and on the board's own
warm-up. To see the sensor react, breathe gently on it: both
temperature and humidity rise within a few readings and then settle
back. If a read fails (NACK or a CRC mismatch), the log shows a yellow
`[WARN] SHT40: read failed (NACK or CRC)` line instead.

## Walkthrough

### 1. Board config — I2C2 and one address

[`Sw/Board/board_config.h`](./Sw/Board/board_config.h) keeps only
`TIREMO_BOARD_SHT40_I2C_BUS` / `_ADDRESS` (`TIREMO_HAL_I2C_BUS_2`,
`0x44`). LIS2DE12 is on the same wires but is out of scope; it comes
back in the next tutorial.

### 2. Entry point — same `PRV_I2C_Init()` as Tutorial 01

[`Sw/Src/main.c`](./Sw/Src/main.c) does not grow a new vendor call.
The SHT40 still lives on `I2C_ID_2`:

```c
PRV_PORT_Init();
PRV_SCU_CLK_Init();
PRV_UART_Init();
PRV_I2C_Init();     /* already scoped to I2C_ID_2, 100 kHz master */
PRV_USER_Code();
```

### 3. The driver — command, wait, CRC, convert

[`Sw/Drivers/sht40/Src/tiremo_sht40.c`](./Sw/Drivers/sht40/Src/tiremo_sht40.c):

```c
const uint8_t command = 0xFDU;

TIREMO_HAL_I2C_Transmit(bus, address, &command, 1U, true);
TIREMO_SysTick_DelayMs(10U);
TIREMO_HAL_I2C_Receive(bus, address, response, 6U, true);
/* CRC each 2-byte word; then milli-unit conversion */
```

`TIREMO_SHT40_Init()` only calls `TIREMO_HAL_I2C_PrepareBus()` — same
stuck-bus recovery the HAL already had, not a new protocol.

### 4. The loop — 1 Hz, log milli-units as decimal

[`Sw/Src/prv_user_code.c`](./Sw/Src/prv_user_code.c) samples once at
boot, then every `READ_INTERVAL_MS` (1000 ms) with the same
`(now - last) >= interval` idiom as every tutorial since
`02-SysTick_Timing`. Reading continuously — rather than logging once
and going silent — means you can open a serial terminal at any time and
see current values, and you can watch the sensor respond to changes
(breath, a warm finger) live.

### 5. Build, flash, and watch

```sh
tdf build
tdf flash
tdf monitor
```

(Or chained in one command: `tdf build flash monitor`.) See
**Solution** above for what the output should look like.

## Done When

`tdf monitor` shows a temperature and humidity pair once per second,
with no `read failed` lines; successive values are stable within sensor
noise, and both rise when you breathe on the sensor.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
