<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>NTP Time Sync</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# NTP Time Sync

***Board: Tiremo Cortex (ESP32) · Section: ESP32 Connectivity / Feature Tutorials · Difficulty: Medium***

## Introduction

Tutorial 07's software clock is RAM-only and starts unsynced. This tutorial is the missing input: after the tutorial 03 join path has an IPv4 address, enable SNTP on the ESP32, parse `CIPSNTPTIME` asctime into a Unix epoch, and call `TIREMO_Clock_SetEpoch()`. Every later MQTT timestamp depends on that.

BluFi stays tutorial 05. Do not add it here.

## You Will Learn

- `AT+CIPSNTPCFG` with **timezone 0** so asctime is UTC
- Why `CIPSNTPTIME?` matches `"OK"`, not `"+CIPSNTPTIME:"`
- Why `Thu Jan  1 00:00:00 1970` is "not synced yet", not midnight 1970
- Feeding the parsed epoch into the same `TIREMO_Clock` API as Sensors tutorial 07

## Prerequisites

- [03-Connect_Retry_State_Machine](../03-Connect_Retry_State_Machine/)
- [07-Software_Wall_Clock](../../../Sensors_And_Storage/Feature_Tutorials/07-Software_Wall_Clock/)
- `wifi_secrets.h` in this `Sw/` (gitignored), same as 02/03
- `tdf.py` from [Getting Started](../../../Getting_Started/)

## Understanding CIPSNTPCFG timezone

```c
return TIREMO_ESP32_SendCommand(
    "AT+CIPSNTPCFG=1,0,\"pool.ntp.org\",\"time.google.com\"\r\n",
    "OK",
    TIREMO_ESP32_DEFAULT_CMD_TIMEOUT_MS);
```

The second argument is hours from UTC, not a Unix offset. **0** means the asctime from `CIPSNTPTIME?` is UTC, which is what Unix epoch is. `timezone=3` would print Turkey civil time; a naive parser would then `SetEpoch` three hours fast and every AWS timestamp would be wrong.

SNTP needs IPv4. This `Sw/` reuses tutorial 03's scan / retry / `WF01` persist, then enables SNTP only after `WIFI GOT IP`.

## Understanding the asctime gotcha

The reply looks like:

```text
+CIPSNTPTIME:Wed Aug 26 18:36:51 2026
OK
```

The civil time sits **after** `+CIPSNTPTIME:`. Matching that marker returns the instant the colon arrives — one byte at a time — and the parser copies nothing. Same rule as `CWJAP?` in tutorial 02: match the terminating `"OK"`, then `strstr` the marker.

Until the module has an NTP sample, ESP-AT prints Unix epoch 0:

```text
+CIPSNTPTIME:Thu Jan  1 00:00:00 1970
```

That is a valid asctime and a valid epoch. Treating it as success would `SetEpoch(0)` and every log would look like 1970. `QuerySntpEpoch` rejects anything before 2020-01-01 (`1577836800`). `WaitSntpEpoch` polls once a second for 20 s.

The day field is space-padded (`Jan  1` vs `Aug 26`). The parser skips whitespace; it does not use `sscanf` (newlib-nano `sscanf` pulls POSIX I/O stubs this firmware does not implement).

```c
if (!TIREMO_ESP32_SendCommandCapture("AT+CIPSNTPTIME?\r\n", "OK",
                                     TIREMO_ESP32_STATUS_TIMEOUT_MS,
                                     response, sizeof(response)))
{
    return false;
}

return prvParseSntpResponse(response, pEpoch);
```

Then:

```c
TIREMO_Clock_SetEpoch(epoch);
```

After that, `GetEpoch()` is still `epoch_at_sync + elapsed_ms / 1000` — tutorial 07's math, with a real NTP sample instead of `1704067200`.

## Credentials (do not commit a password)

```sh
cd Sw
cp wifi_secrets.h.example wifi_secrets.h
```

`wifi_secrets.h` is in [`Sw/.gitignore`](./Sw/.gitignore).

## Solution

[`Sw/`](./Sw/) CMake target `NTPTimeSync`. Same `GEN_DIR` as 01–03, plus `tiremo_clock.c` from tutorial 07.

```sh
cd Sw
cp wifi_secrets.h.example wifi_secrets.h   # once; then edit
tdf.py build
tdf.py flash
tdf.py monitor
```

**Verified on the connected board:** `.text` 20956 bytes, flash **Verified OK**. After join, `GetEpoch()` tracked wall clock one-for-one:

```text
[INFO] ESP32: WIFI GOT IP
[INFO] ESP32: CWSTATE=2
[INFO] ESP32: SNTP enabled (timezone 0 = UTC)
[INFO] clock: SetEpoch 1787769402 (from CIPSNTPTIME asctime)
[INFO] clock: epoch=1787769411  elapsed=9 s
[INFO] clock: epoch=1787769412  elapsed=10 s
[INFO] clock: epoch=1787769413  elapsed=11 s
```

`1787769411` is **2026-08-26 18:36:51 UTC**. `elapsed` and `epoch` advanced together for 30+ seconds. `tdf.py monitor` can still miss the SNTP lines (reset-then-open).

## Walkthrough

1. [`main.c`](./Sw/Src/main.c) — UART0 + UART2, unchanged.
2. Tutorial 03 join path — scan, 3× `CWJAP`, `WF01` persist.
3. [`tiremo_clock.c`](./Sw/Common/Src/tiremo_clock.c) — copied from Sensors 07; no planted epoch.
4. [`tiremo_esp32.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c) — `EnableSntp`, `QuerySntpEpoch`, `WaitSntpEpoch`, asctime → UTC epoch.
5. [`prv_user_code.c`](./Sw/Src/prv_user_code.c) — after GOT IP: `CIPSNTPCFG` → wait SNTP → `SetEpoch` → 1 Hz `GetEpoch` log. SNTP failure hangs with `ERROR` (no reboot loop).

## Done When

Right after sync, `GetEpoch()` matches real UTC within a few seconds, and then advances one second per second — **confirmed on hardware**.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
