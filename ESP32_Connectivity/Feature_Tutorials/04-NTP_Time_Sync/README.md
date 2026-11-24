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
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# NTP Time Sync

***Board: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Section: ESP32 Connectivity / Feature Tutorials · Difficulty: Medium***

## Introduction

[Sensors tutorial 07](../../../Sensors_And_Storage/Feature_Tutorials/07-Software_Wall_Clock/)'s software clock is RAM-only and starts unsynced. This tutorial is the missing input: after the tutorial 03 join path has an IPv4 address, enable SNTP on the ESP32, parse `CIPSNTPTIME` asctime into a Unix epoch, and call `TIREMO_Clock_SetEpoch()`. Every later MQTT timestamp depends on that, and the secure (TLS) MQTT connections in the cloud sections need a correct clock before they can connect at all — a certificate can't be checked without knowing today's date.

BluFi is tutorial 05. Do not add it here.

## You Will Learn

- `AT+CIPSNTPCFG` with **timezone 0** so asctime is UTC
- Why `CIPSNTPTIME?` matches `"OK"`, not `"+CIPSNTPTIME:"`
- Why `Thu Jan  1 00:00:08 1970` means "not synced yet", not a time in 1970
- Feeding the parsed epoch into the same `TIREMO_Clock` API as Sensors tutorial 07

## Prerequisites

- [03-Connect_Retry_State_Machine](../03-Connect_Retry_State_Machine/)
- [07-Software_Wall_Clock](../../../Sensors_And_Storage/Feature_Tutorials/07-Software_Wall_Clock/)
- A 2.4 GHz WiFi network with internet access (the module asks public NTP servers for the time), and this `Sw/`'s own `wifi_secrets.h` — see [Credentials](#credentials-do-not-commit-a-password)
- The setup scripts from [Getting Started](../../../Getting_Started/) already run, so `tdf` is available
- To watch the log: `tdf monitor` or any serial terminal (Tera Term, PuTTY, screen, minicom, …) at 115200 8N1 on the board's USB serial port
- A Tiremo Cortex board (ABOV A34G43A MCU — see [Hardware Overview](../../../Hardware.md)) connected to your PC with a USB cable

## Understanding CIPSNTPCFG timezone

```c
return TIREMO_ESP32_SendCommand(
    "AT+CIPSNTPCFG=1,0,\"pool.ntp.org\",\"time.google.com\"\r\n",
    "OK",
    TIREMO_ESP32_DEFAULT_CMD_TIMEOUT_MS);
```

The second argument is hours from UTC, not a Unix offset. **0** means the asctime from `CIPSNTPTIME?` is UTC, which is what Unix epoch is. `timezone=3` would print Turkey civil time; a naive parser would then `SetEpoch` three hours fast and every timestamp sent to the cloud would be wrong.

SNTP needs IPv4. This `Sw/` reuses tutorial 03's scan / retry / `WF01` persist — including the filtered scan, the `wifi_secrets.h` check, and the hidden `AT+CWJAP` in the DEBUG dump — then enables SNTP only after `WIFI GOT IP`.

## Understanding the asctime gotcha

The reply, as captured on the board with `TIREMO_DEBUG_LEVEL=4`, looks like:

```text
{"data":"+CIPSNTPTIME:Tue Nov 24 06:51:54 2026\r\nOK"}
```

The civil time sits **after** `+CIPSNTPTIME:`. Matching that marker returns the instant the colon arrives — one byte at a time — and the parser copies nothing. Same rule as `CWJAP?` in tutorial 02: match the terminating `"OK"`, then `strstr` the marker.

Until the module has an NTP sample, its clock simply counts up from Unix epoch 0. This was the first answer on the board, a second after SNTP was enabled:

```text
{"data":"+CIPSNTPTIME:Thu Jan  1 00:00:08 1970\r\nOK"}
```

That is a valid asctime and a valid epoch (8). Treating it as success would `SetEpoch(8)` and every log would look like 1970. `QuerySntpEpoch` rejects anything before 2020-01-01 (`1577836800`). `WaitSntpEpoch` polls once a second for up to 20 s; on the board the second poll already had the real time.

The day field is space-padded (`Jan  1` vs `Nov 24`). The parser skips whitespace; it does not use `sscanf` (newlib-nano `sscanf` pulls POSIX I/O stubs this firmware does not implement).

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

Same as [tutorial 03](../03-Connect_Retry_State_Machine/#credentials-do-not-commit-a-password): this `Sw/` needs its own `wifi_secrets.h`. From the repo root:

```sh
cd ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/Sw
cp wifi_secrets.h.example wifi_secrets.h
```

(In cmd.exe, use `copy` instead of `cp`.) Then fill in your network's SSID and password, or copy the `wifi_secrets.h` you made for tutorial 02 or 03. The file is listed in [`Sw/.gitignore`](./Sw/.gitignore), so it never ends up in a commit.

## Solution

From the repo root (after creating `wifi_secrets.h` as above):

```sh
cd ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/Sw
tdf build
tdf flash
tdf monitor
```

(See [Getting_Started/01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution) for what `tdf` is. On macOS/Linux, `tdf.py` also works as an alias; on Windows, use `tdf`. Close any other terminal on the board's serial port before running `tdf monitor` — only one program can open it at a time.)

**Confirmed on real hardware:** `tdf build` compiles and links without errors on macOS and Windows 11 (`.text` about 21 KB; the exact size varies slightly with the toolchain version, and the linker prints the same expected, harmless `_close` / `_lseek` / `_read` / `_write is not implemented` warnings explained in [01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution)). `tdf flash` completed a full program-and-verify cycle over SWD on a real, connected Tiremo Cortex board (`** Verified OK **`). The log after a reset, with the network's name replaced:

```text
[INFO] NTP_Time_Sync: boot
[INFO] persist: loaded SSID "YourSSID"
[INFO] ESP32: power cycling module...
[INFO] ESP32: power-on boot delay elapsed
[INFO] TIREMO_ESP32_Init: UART2 ready
[INFO] ESP32: command echo disabled (ATE0)
[INFO] ESP32: scanning for "YourSSID"
[INFO] ESP32: SSID visible
[INFO] ESP32: join attempt 1/3
[INFO] ESP32: WIFI GOT IP
[INFO] ESP32: CWSTATE=2
[INFO] persist: saved SSID "YourSSID" (password not logged)
[INFO] ESP32: SNTP enabled (timezone 0 = UTC)
[INFO] clock: SetEpoch 1795503159 (from CIPSNTPTIME asctime)
[INFO] clock: epoch=1795503160  elapsed=1 s
[INFO] clock: epoch=1795503161  elapsed=2 s
[INFO] clock: epoch=1795503162  elapsed=3 s
```

`1795503159` is **2026-11-24 06:52:39 UTC**, and it matched the PC's clock to within a second. From then on `epoch` and `elapsed` go up together, one per second. Your numbers will be today's date — any [epoch converter](https://www.epochconverter.com/) turns them back into a readable time. The `clock:` lines start about 15 s after the reset: 3.5 s for the module to power up, then a few seconds each for the scan, the join, and the first NTP answer.

## Walkthrough

1. [`main.c`](./Sw/Src/main.c) — UART0 + UART2, unchanged.
2. Tutorial 03 join path — scan, 3× `CWJAP`, `WF01` persist.
3. [`tiremo_clock.c`](./Sw/Common/Src/tiremo_clock.c) — copied from Sensors 07; the epoch now comes from NTP instead of a hardcoded value.
4. [`tiremo_esp32.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c) — `EnableSntp`, `QuerySntpEpoch`, `WaitSntpEpoch`, asctime → UTC epoch.
5. [`prv_user_code.c`](./Sw/Src/prv_user_code.c) — after GOT IP: `CIPSNTPCFG` → wait SNTP → `SetEpoch` → 1 Hz `GetEpoch` log. An SNTP failure stops with an `[ERROR]` line instead of rebooting.

## Done When

The `SetEpoch` value matches the current UTC time (check it with an epoch converter) to within a few seconds, and after that `epoch` and `elapsed` both go up by one every second.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
