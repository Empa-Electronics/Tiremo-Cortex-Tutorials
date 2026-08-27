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

***Board: Tiremo Cortex (ESP32) · Section: ESP32 Connectivity / Design Tutorials · Difficulty: Hard***

## Introduction

Feature tutorials 01–05 each proved one piece. This is the board's real boot flow: try MCU-saved credentials, else an SSID-only NVS rejoin, else BluFi, and on any success sync NTP. No `wifi_secrets.h`.

## You Will Learn

- A cascade of `prvTryX()` functions that each return a plain `bool`
- Branching on **SSID+password** vs **SSID only** vs **nothing** in `CS01` flash
- Why BluFi stays the last resort, and why NTP runs only after IPv4

## Prerequisites

- All of [ESP32 Connectivity / Feature Tutorials](../../Feature_Tutorials/) (01–05)
- `tdf.py` from [Getting Started](../../../Getting_Started/)
- EspBluFi on a phone (same as tutorial 05) for the factory-fresh path

## The boot banner

Design Tutorials print the Tiremo Cortex wordmark so a bare terminal identifies the application. Feature Tutorials stay quiet. [`tiremo_banner.c`](./Sw/Common/Src/tiremo_banner.c) is the same artwork as the environmental logger (100 columns; single-space “blank” rows so `tdf.py monitor` does not collapse the gap). Title line:

```text
  Boot Cascade: Saved WiFi then BluFi
  Design Tutorial - Empa Electronics
```

`tdf.py monitor` resets then opens the port, so it usually misses the banner.

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
| SSID + password | `saved` | `CWLAP` then `CWJAP="ssid","pass"` up to 3 times |
| SSID, empty password | `nvs` | `CWAUTOCONN=1`, then watch `CWSTATE` for 15 s while the module rejoins by itself |
| Missing / bad `CS01` | `blufi` | Advertise `Tiremo-Cortex`, harvest URCs, save `CS01` |

A failed `saved` or `nvs` attempt **falls through** to BluFi. That is the product behaviour: do not reboot-loop a bad password.

Magic is **`CS01`** (`0x43533031`), not `WF01` / `BL01`. Flashing this image is factory-fresh for the cascade even if tutorial 03 still has a last-good join in sector 0 (that record is the wrong magic). The sector is still overwritten on the first successful BluFi save.

`BlufiPoll` remains the only UART2 reader during the wait (tutorial 05). NTP still matches `"OK"` on `CIPSNTPTIME?` (tutorial 04).

## Understanding the password you never get

Not every ESP-AT build prints `+PASSWORD:`. On the board this tutorial was written against, provisioning emits `+BLUFICONN`, then `WIFI CONNECTED`, then `WIFI GOT IP` — and no credential URCs at all. The password is then unrecoverable: `AT+CWJAP?` answers with the SSID only, by design.

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

```sh
cd Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

**Verified on the connected board:** `.text` 25196 bytes, flash **Verified OK**. Factory-fresh boot advertised and was provisioned from EspBluFi:

```text
[INFO] persist: no CS01 record (factory-fresh for this app)
[INFO] cascade: BluFi fallback
[INFO] ESP32: BluFi started, advertising as "Tiremo-Cortex"
[INFO] ESP32: BluFi phone connected
[INFO] ESP32: BluFi URC "WIFI CONNECTED"
[INFO] ESP32: BluFi WIFI GOT IP
[INFO] ESP32: BluFi stopped
[INFO] cascade: SSID from CWJAP? "Zyxel_4AA1"
[INFO] cascade: no +PASSWORD: URC, saving SSID only (next boot uses NVS rejoin)
[INFO] persist: saved SSID "Zyxel_4AA1" (password not logged)
[INFO] clock: SetEpoch 1787773450  path=blufi
```

The next reset skipped BluFi entirely and came up on the `nvs` path. The two `CWSTATE` warnings are the poll loop watching the module finish its own rejoin:

```text
[INFO] persist: loaded SSID "Zyxel_4AA1" password=no
[INFO] cascade: try NVS auto-rejoin (SSID only, no MCU password)
[WARN] ESP32 RX: timeout after 2000 ms, 33 bytes without "+CWSTATE:2"
[INFO] cascade: NVS rejoin got IP
[INFO] clock: SetEpoch 1787773514  path=nvs
```

An AT build that *does* emit `+PASSWORD:` stores it in `CS01` and takes the `saved` path instead, scanning and joining with the MCU's own copy.

## Walkthrough

1. [`tiremo_banner.c`](./Sw/Common/Src/tiremo_banner.c) — Design Tutorial mark
2. [`prv_user_code.c`](./Sw/Src/prv_user_code.c) — `prvTryScanJoin` / `prvTryNvsRejoin` / `prvTryBlufi` / NTP heartbeat
3. [`tiremo_esp32.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c) — union of tutorials 01–05 (join, SNTP, BluFi, `JoinStoredSsid`)
4. [`tiremo_persist.h`](./Sw/Common/Include/tiremo_persist.h) — `CS01` + `HasPassword`
5. [`tiremo_clock.c`](./Sw/Common/Src/tiremo_clock.c) — tutorial 07 API, set from NTP

## Done When

Factory-fresh boot advertises BluFi; after EspBluFi, `CS01` is stored; the following reset joins WiFi and syncs NTP with no BluFi step. All three stages are **confirmed** on hardware, ending at `cascade: online path=nvs` because this AT build never sent `+PASSWORD:`.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
