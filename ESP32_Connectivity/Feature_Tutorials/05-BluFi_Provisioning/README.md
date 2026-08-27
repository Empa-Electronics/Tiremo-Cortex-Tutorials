<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/ESP32_Connectivity/Feature_Tutorials/05-BluFi_Provisioning/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>BluFi BLE Provisioning</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# BluFi BLE Provisioning

***Board: Tiremo Cortex (ESP32) · Section: ESP32 Connectivity / Feature Tutorials · Difficulty: Hard***

## Introduction

Hardcoding an SSID does not scale past one bench unit. This tutorial has **no** `wifi_secrets.h`. A phone running EspBluFi pushes credentials over BLE; the MCU harvests `+SSID:` / `+PASSWORD:` / `WIFI GOT IP` and writes them to data flash (`BL01`). The next reset joins that saved AP with no advertising step.

The boot cascade (saved WiFi first, BluFi only as fallback, then NTP) is the [design tutorial](../../Design_Tutorials/01-Boot_Cascade_Saved_WiFi_Then_BluFi/). Do not add NTP here.

## You Will Learn

- `AT+BLUFINAME` then `AT+BLUFI=1`, and the async URCs `+BLUFICONN`, `+SSID:`, `+PASSWORD:`, `WIFI GOT IP`
- Why `TIREMO_ESP32_BlufiPoll()` must be the **only** UART2 consumer during the wait
- Why `IsConnected()` / `CWSTATE?` in that loop can swallow a credential URC
- Stopping BluFi (`AT+BLUFI=0`) and turning `CWAUTOCONN` back on after a successful provision

## Prerequisites

- [04-NTP_Time_Sync](../04-NTP_Time_Sync/) conceptually (WiFi already works); this `Sw/` is join + BluFi, not SNTP
- EspBluFi on a phone: [Android](https://github.com/EspressifApp/EspBluFi-Android) / [iOS](https://apps.apple.com/app/espblufi/id1455860696)
- Phone Bluetooth on; Android also needs location enabled for BLE scan
- `tdf.py` from [Getting Started](../../../Getting_Started/)

## Understanding RX ownership

`SendCommand` **flushes** the UART2 ring before it transmits. `IsConnected()` is just `AT+CWSTATE?`. If you poll that while waiting for BluFi, the flush can throw away:

```text
+SSID:YourNetwork
+PASSWORD:secret
WIFI GOT IP
```

before `BlufiPoll` ever sees a line. Then you have IPv4 and no password to persist.

`BlufiPoll` only **pops** bytes, assembles lines, and updates a harvest struct. No TX. The wait loop is:

```c
for (;;)
{
    TIREMO_ESP32_BlufiPoll(&st);
    if (st.gotIp)
    {
        break;
    }
    TIREMO_SysTick_DelayMs(20U);
}
```

`AT+BLUFI=0` is itself a `SendCommand`. Copy `st` (SSID/password) **before** stop.

## Understanding the AT sequence

| Command | When | Wait for |
|---------|------|----------|
| `AT+CWMODE=1` | before BLE | `"OK"` |
| `AT+CWAUTOCONN=0` | before BLE | `"OK"` |
| `AT+BLUFINAME="Tiremo-Cortex"` | before `BLUFI=1` | `"OK"` |
| `AT+BLUFI=1` | start advertising | `"OK"` |
| *(no command)* | wait | URCs via `BlufiPoll` |
| `AT+CWAUTOCONN=1` | after `WIFI GOT IP` | `"OK"` |
| `AT+BLUFI=0` | after auto-connect is back on | `"OK"`, up to 5 s |

Name must be set **before** `AT+BLUFI=1` or the module advertises `BLUFI_DEVICE`. Bluetooth LE must not already be inited (`AT+BLEINIT=0`); a power-cycle from `TIREMO_ESP32_Init()` leaves it that way.

ESP-AT is *documented* to print `+SSID:` and `+PASSWORD:` when the app sends credentials, then `WIFI CONNECTED` / `WIFI GOT IP` when station DHCP succeeds. Not every build actually does: some emit only `+BLUFICONN`, `WIFI CONNECTED` and `WIFI GOT IP`. Any line the parser does not recognise is echoed as `ESP32: BluFi URC "..."` so you can see which build you have.

When `+PASSWORD:` never arrives the password is gone for good — `AT+CWJAP?` returns the SSID only. That is why `CWAUTOCONN=1` is sent **before** `AT+BLUFI=0` in the table above: the module's own NVS copy becomes the only way back onto that AP, so it must be armed before anything else can fail.

`AT+BLUFI=0` also answers `+BLUFIDISCONN` first and `OK` afterwards. With a one-second timeout the wait gives up early and the trailing `OK` arrives during the *next* command, satisfying its match — an `AT+CWJAP?` right after would return with no `+CWJAP:` line and lose the SSID too. The stop therefore waits 5 s, and `GetConnectedSsid()` retries when the marker is missing.

## Persist magic `BL01`

Same sector-0 CRC layout as tutorial 03, magic **`0x424C3031` (`BL01`)**, not `WF01`. A last-good join from tutorial 03/04 is **not** treated as a BluFi provision, so the first flash of this firmware always advertises. After EspBluFi succeeds, `BL01` is written; the next reset loads it and `CWJAP`s without advertising.

Flashing this image still **overwrites** DFMC sector 0 (TRM1 / ENV1 / WF01).

## Solution

[`Sw/`](./Sw/) CMake target `BluFiProvisioning`. No `wifi_secrets.h`.

```sh
cd Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

**Verified on the connected board:** `.text` 21156 bytes, flash **Verified OK**. `AT+BLUFI=1` accepted; UART:

```text
[INFO] BluFi_Provisioning: boot
[INFO] persist: no saved SSID (no wifi_secrets.h in this tutorial)
[INFO] ESP32: BluFi started, advertising as "Tiremo-Cortex"
[INFO] BluFi: open EspBluFi, connect to "Tiremo-Cortex", Configure
[INFO] BluFi: still advertising as "Tiremo-Cortex"
```

Phone Configure is the remaining human step. On a build that emits the credential URCs, expect:

```text
[INFO] ESP32: BluFi phone connected
[INFO] ESP32: BluFi +SSID:"YourSSID"
[INFO] ESP32: BluFi +PASSWORD: received (not logged)
[INFO] ESP32: BluFi WIFI GOT IP
[INFO] ESP32: BluFi stopped
[INFO] persist: saved SSID "YourSSID" (password not logged)
[INFO] ESP32: heartbeat still connected
```

On a build that does not, the SSID still lands in `BL01` and `CWAUTOCONN` carries the password:

```text
[INFO] ESP32: BluFi URC "WIFI CONNECTED"
[INFO] ESP32: BluFi WIFI GOT IP
[WARN] ESP32: GOT IP without +PASSWORD: URC (module NVS keeps the only copy)
[INFO] ESP32: SSID recovered from CWJAP? "YourSSID"
```

Second reset should log `persist: loaded SSID` and join without advertising. With an empty stored password that join relies on the module's NVS, which is exactly the `nvs` branch the [boot cascade design tutorial](../../Design_Tutorials/01-Boot_Cascade_Saved_WiFi_Then_BluFi/) formalises.

## Walkthrough

1. [`prv_user_code.c`](./Sw/Src/prv_user_code.c) — if `BL01` exists, scan/join like tutorial 03; else advertise forever until GOT IP.
2. [`tiremo_esp32.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c) — `BlufiStart` / `BlufiPoll` / `BlufiStop` / `EnableAutoConnect`.
3. [`tiremo_persist.h`](./Sw/Common/Include/tiremo_persist.h) — `BL01` record.

## Done When

EspBluFi can provision the board with no SSID in source, UART shows harvested `+SSID:` (password not logged) and `WIFI GOT IP`, and the next reboot joins from `BL01` with no BluFi wait. Advertising itself is confirmed on hardware; the phone step needs a human.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
