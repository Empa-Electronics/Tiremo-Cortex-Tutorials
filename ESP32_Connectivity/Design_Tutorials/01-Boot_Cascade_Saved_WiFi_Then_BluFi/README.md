<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/ESP32_Connectivity/Design_Tutorials/01-Boot_Cascade_Saved_WiFi_Then_BluFi/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Boot Cascade: Saved WiFi → BluFi</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Boot Cascade: Saved WiFi → BluFi

***Board: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Section: ESP32 Connectivity / Design Tutorials · Difficulty: Hard***

## Introduction

Feature tutorials 01–05 each proved one piece. This is the board's real boot flow: try MCU-saved credentials, else an SSID-only NVS rejoin, else BluFi, and on any success sync NTP. No `wifi_secrets.h`.

## You Will Learn

- A cascade of `prvTryX()` functions that each return a plain `bool`
- Branching on **SSID+password** vs **SSID only** vs **nothing** in `CS01` flash
- Why BluFi stays the last resort, and why NTP runs only after IPv4

## Prerequisites

- All of [ESP32 Connectivity / Feature Tutorials](../../Feature_Tutorials/) (01–05)
- EspBluFi on a phone (same as [tutorial 05](../../Feature_Tutorials/05-BluFi_Provisioning/)) for the factory-fresh path, and a 2.4 GHz WiFi network with internet access (for NTP)
- The setup scripts from [Getting Started](../../../Getting_Started/) already run, so `tdf` is available
- To watch the log: `tdf monitor` or any serial terminal (Tera Term, PuTTY, screen, minicom, …) at 115200 8N1 on the board's USB serial port, **at least 100 columns wide** (for the banner)
- A Tiremo Cortex board (ABOV A34G43A MCU — see [Hardware Overview](../../../Hardware.md)) connected to your PC with a USB cable

## The boot banner

Design Tutorials print the Tiremo Cortex wordmark so a bare terminal identifies the application. Feature Tutorials stay quiet. [`tiremo_banner.c`](./Sw/Common/Src/tiremo_banner.c) is the same artwork as the [environmental logger](../../../Sensors_And_Storage/Design_Tutorials/01-Standalone_Environmental_Logger/#the-boot-banner) (100 columns wide; single-space "blank" rows so `tdf monitor` does not collapse the gap). Its title lines here:

```text
  Boot Cascade: Saved WiFi then BluFi
  Design Tutorial - Empa Electronics
```

`tdf monitor` opens the serial port first and then resets the board, so the banner is the first thing you see. With another terminal, connect first and then press RESET.

## Understanding the cascade

[`prv_user_code.c`](./Sw/Src/prv_user_code.c) does not nest a giant `if`. Each path is one function:

```c
if (TIREMO_Persist_HasSsid(&saved) && TIREMO_Persist_HasPassword(&saved))
{
    online = prvTryScanJoin(saved.ssid, saved.password);   /* tutorial 03 */
}
else if (TIREMO_Persist_HasSsid(&saved))
{
    online = prvTryNvsRejoin(saved.ssid);                  /* NVS password */
}

if (!online)
{
    online = prvTryBlufi(&harvest);                        /* tutorial 05 */
}

prvSyncNtpThenHeartbeat(pPath);                            /* tutorial 04 */
```

| Persist | Path | What it does |
|---------|------|----------------|
| SSID + password | `saved` | `CWLAP="ssid"` (tutorial 03's filtered scan) then `CWJAP="ssid","pass"` up to 3 times |
| SSID, empty password | `nvs` | `CWAUTOCONN=1`, then watch `CWSTATE` for 15 s while the module rejoins by itself |
| Missing / bad `CS01` | `blufi` | Advertise `Tiremo-Cortex`, harvest URCs, save `CS01` |

A failed `saved` or `nvs` attempt **falls through** to BluFi. That is the product behaviour: do not reboot-loop a bad password.

Magic is **`CS01`** (`0x43533031`), not `WF01` / `BL01`. Flashing this image is factory-fresh for the cascade even if tutorial 03 still has a last-good join in sector 0 (that record is the wrong magic). The sector is still overwritten on the first successful BluFi save.

`BlufiPoll` remains the only UART2 reader during the wait (tutorial 05). NTP still matches `"OK"` on `CIPSNTPTIME?` (tutorial 04). As in tutorials 02–05, the DEBUG-level TX dump shows `AT+CWJAP=<credentials hidden>` instead of a saved password.

## Understanding the password you never get

Not every ESP-AT build prints `+PASSWORD:`. On the Tiremo Cortex board used to test this tutorial, provisioning emits `+BLUFICONN`, then `WIFI CONNECTED`, then `WIFI GOT IP` — and no credential URCs at all. The password is then unrecoverable: `AT+CWJAP?` answers with the SSID only, by design.

The cascade treats that as a normal outcome instead of an error:

1. `CWAUTOCONN=1` is sent **before** `AT+BLUFI=0`, so the module's own NVS copy of the credentials survives even if the stop command fails.
2. The SSID comes from `AT+CWJAP?` when `+SSID:` never arrived.
3. `CS01` is written with an empty password, which routes the next boot to the `nvs` path, where the module supplies the password itself.

Two timing traps sit on this path. `AT+BLUFI=0` answers `+BLUFIDISCONN` first and `OK` only afterwards, so a one-second timeout expires early — and the late `OK` then satisfies the *next* command's match. That is what makes `AT+CWJAP?` return without a `+CWJAP:` line and lose the SSID. `BlufiStop()` therefore waits 5 s, and `GetConnectedSsid()` retries when the marker is missing:

```c
pMarker = strstr(response, "+CWJAP:");
if (pMarker == NULL)
{
    TIREMO_LOG_WARN("ESP32: CWJAP? matched a stale OK, retrying");
    TIREMO_SysTick_DelayMs(TIREMO_ESP32_SETTLE_MS);
    continue;
}
```

Any BluFi line the parser does not recognise is echoed as `ESP32: BluFi URC "..."`, so a firmware that spells these URCs differently shows up in the log instead of disappearing.

## Solution

There is no `wifi_secrets.h` to create. From the repo root:

```sh
cd ESP32_Connectivity/Design_Tutorials/01-Boot_Cascade_Saved_WiFi_Then_BluFi/Sw
tdf build
tdf flash
tdf monitor
```

(See [Getting_Started/01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution) for what `tdf` is. On macOS/Linux, `tdf.py` also works as an alias; on Windows, use `tdf`. Close any other terminal on the board's serial port before running `tdf monitor` — only one program can open it at a time.)

**Confirmed on real hardware:** `tdf build` compiles and links without errors on macOS and Windows 11 (`.text` about 25 KB; the exact size varies slightly with the toolchain version, and the linker prints the same expected, harmless `_close` / `_lseek` / `_read` / `_write is not implemented` warnings explained in [01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution)). `tdf flash` completed a full program-and-verify cycle over SWD on a real, connected Tiremo Cortex board (`** Verified OK **`), and both stages below ran on it. Network names are replaced with `YourSSID`.

### 1. Factory-fresh boot: BluFi

With no `CS01` record, the boot logs `persist: no CS01 record (factory-fresh for this app)` and `cascade: BluFi fallback`, starts advertising as `Tiremo-Cortex`, and repeats `BluFi: still advertising as "Tiremo-Cortex"` every 5 s. Provision it from EspBluFi exactly as in [tutorial 05](../../Feature_Tutorials/05-BluFi_Provisioning/#1-provision-from-the-phone) (Configure → Station → SSID and password). On the test board:

```text
[INFO] ESP32: BluFi phone connected
[INFO] ESP32: BluFi URC "WIFI CONNECTED"
[INFO] ESP32: BluFi WIFI GOT IP
[INFO] ESP32: BluFi stopped
[INFO] cascade: SSID from CWJAP? "YourSSID"
[INFO] cascade: no +PASSWORD: URC, saving SSID only (next boot uses NVS rejoin)
[INFO] persist: saved SSID "YourSSID" (password not logged)
[INFO] ESP32: SNTP enabled (timezone 0 = UTC)
[INFO] clock: SetEpoch 1795506247  path=blufi
[INFO] cascade: online path=blufi epoch=1795506252
[INFO] cascade: online path=blufi epoch=1795506257
```

### 2. Reset: the `nvs` path

Press RESET (or run `tdf monitor` again) without touching the phone. The board skips BluFi entirely:

```text
  Boot Cascade: Saved WiFi then BluFi
  Design Tutorial - Empa Electronics

[INFO] Boot_Cascade: boot
[INFO] ESP32: power cycling module...
[INFO] ESP32: power-on boot delay elapsed
[INFO] TIREMO_ESP32_Init: UART2 ready
[INFO] ESP32: command echo disabled (ATE0)
[INFO] persist: loaded SSID "YourSSID" password=no
[INFO] cascade: try NVS auto-rejoin (SSID only, no MCU password)
[WARN] ESP32 RX: timeout after 2000 ms, 59 bytes without "+CWSTATE:2"
[WARN] ESP32 RX: timeout after 2000 ms, 54 bytes without "+CWSTATE:2"
[INFO] cascade: NVS rejoin got IP
[INFO] ESP32: SNTP enabled (timezone 0 = UTC)
[INFO] clock: SetEpoch 1795506430  path=nvs
[INFO] cascade: online path=nvs epoch=1795506435
```

The two `[WARN]` lines are expected: they are the `CWSTATE?` polls that ran while the module was still rejoining by itself (state 0–3, not 2 yet). `cascade: online` then repeats every 5 s.

An AT build that *does* emit `+PASSWORD:` stores it in `CS01` and takes the `saved` path instead, scanning and joining with the MCU's own copy.

## Walkthrough

1. [`tiremo_banner.c`](./Sw/Common/Src/tiremo_banner.c) — Design Tutorial mark
2. [`prv_user_code.c`](./Sw/Src/prv_user_code.c) — `prvTryScanJoin` / `prvTryNvsRejoin` / `prvTryBlufi` / NTP heartbeat
3. [`tiremo_esp32.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c) — union of tutorials 01–05 (filtered scan, join, SNTP, BluFi, `JoinStoredSsid`, hidden `AT+CWJAP` in the DEBUG dump)
4. [`tiremo_persist.h`](./Sw/Common/Include/tiremo_persist.h) — `CS01` + `HasPassword`. A received password is kept in plain text in the MCU's data flash, as in tutorials 03 and 05.
5. [`tiremo_clock.c`](./Sw/Common/Src/tiremo_clock.c) — Sensors tutorial 07's API, set from NTP

## Done When

- A factory-fresh boot advertises BluFi; after EspBluFi, the log shows `persist: saved SSID` and `clock: SetEpoch ... path=blufi`.
- After a reset, with nothing done on the phone, the log shows `persist: loaded SSID`, then `clock: SetEpoch ... path=nvs` (or `path=saved` on an AT build that reports `+PASSWORD:`) and `cascade: online` every 5 s — with no `BluFi started` line.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
