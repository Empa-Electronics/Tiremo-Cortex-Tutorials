<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Connect/Retry State Machine</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Connect/Retry State Machine

***Board: Tiremo Cortex (ESP32) · Section: ESP32 Connectivity / Feature Tutorials · Difficulty: Medium***

## Introduction

Tutorial 02 joins once and hangs on failure. Real APs drop packets, DHCP stalls, and a saved network can be out of range. This tutorial wraps that join in **scan first**, **three attempts**, and **MCU flash for last-good credentials** — the host remembers what worked, not the ESP32's NVS.

NTP and BluFi stay later. Do not add them here.

## You Will Learn

- `AT+CWLAP` as a fail-fast "is this SSID even visible?" check
- Why `JoinAP` returns **got-IP / ERROR / timeout**, not a boolean
- When to **clear** last-good flash vs **retry without clearing**
- Reusing Sensors tutorial 06's flash layout with a **different magic** (`WF01`, not `TRM1`)

## Prerequisites

- [02-WiFi_Station_Join](../02-WiFi_Station_Join/)
- [06-Flash_Backed_Config](../../../Sensors_And_Storage/Feature_Tutorials/06-Flash_Backed_Config/) (same CRC/sector idea)
- `wifi_secrets.h` from tutorial 02 (copy the example into this `Sw/` too)
- `tdf.py` from [Getting Started](../../../Getting_Started/)

## Understanding scan-then-join

`AT+CWJAP` can sit for 20 s before it admits the AP is gone. `AT+CWLAP` prints every visible 2.4 GHz SSID in a few seconds. Matching the **quoted** name (`"YourSSID"`) in that dump means: do not even start `CWJAP` if the network is not on the air.

```c
(void)snprintf(expected, sizeof(expected), "\"%s\"", pSsid);

return TIREMO_ESP32_SendCommand("AT+CWLAP\r\n", expected, timeoutMs);
```

The RX ring and capture buffer grow to **512** bytes here. A `CWLAP` dump is much larger than `AT`/`OK`. If the list is huge, the SSID might fall off the end of the capture — that is a known limit of this tutorial's buffer, not of ESP-AT.

## Understanding ERROR vs timeout

Tutorial 02's `JoinAP` was a bool: saw `WIFI GOT IP` or not. A **wrong password** and a **slow DHCP** both look like "false" if you only wait for the success tag.

The transport already aborts a wait when it sees a framed `\r\nERROR\r\n` (unless that *is* the expected string). After that abort, `JoinAP` classifies:

| Outcome | Meaning | App does |
|---------|---------|----------|
| `TIREMO_ESP32_JOIN_GOT_IP` | `WIFI GOT IP` | save credentials, heartbeat |
| `TIREMO_ESP32_JOIN_ERROR` | module rejected the join (wrong password, invalid SSID format, …) | **clear** flash, idle |
| `TIREMO_ESP32_JOIN_TIMEOUT` | 20 s of silence / no GOT IP and no ERROR | retry (up to 3), **keep** flash until all three fail |

One timeout is a glitch. Three timeouts in a row is "this saved AP is not usable" — then clear. A framed ERROR is definite: retrying the same password will not help.

```c
if (TIREMO_ESP32_SendCommandCapture(command, "WIFI GOT IP", timeoutMs,
                                    response, sizeof(response)))
{
    return TIREMO_ESP32_JOIN_GOT_IP;
}

if (strstr(response, "\r\nERROR\r\n") != NULL)
{
    return TIREMO_ESP32_JOIN_ERROR;
}

return TIREMO_ESP32_JOIN_TIMEOUT;
```

## Understanding last-good SSID (`WF01`)

[`tiremo_persist.c`](./Sw/Common/Src/tiremo_persist.c) is the Sensors 06 pattern: DFMC sector 0, magic + version + CRC32 (poly `0xEDB88320`) over every byte *before* `crc32`. The magic is **`0x57463031` (`WF01`)**, not `TRM1` or `ENV1`, so a logger record and a WiFi record cannot be mistaken for each other.

They still **share sector 0**. Flashing this tutorial **overwrites** whatever Sensors 06 / the environmental logger stored there. That is expected: one sector, one owner per firmware image.

Boot order for the SSID:

1. Valid `WF01` record with a non-empty SSID → use that (and its saved password).
2. Else → `wifi_secrets.h`.

Success after join → `Persist_Save`. Scan miss on a **flash** SSID, framed ERROR, or three timeouts → `Persist_Clear`. A scan miss on **secrets** does not erase flash (there was nothing to erase).

`AT+CWAUTOCONN=0` and `AT+CWQAP` still run first, same as tutorial 02: the **MCU** owns "remember what worked", not the module NVS.

## Credentials (do not commit a password)

```sh
cd Sw
cp wifi_secrets.h.example wifi_secrets.h
```

`wifi_secrets.h` is in [`Sw/.gitignore`](./Sw/.gitignore). Logs print the SSID, never the password.

## Solution

[`Sw/`](./Sw/) has a real, buildable implementation. CMake target `ConnectRetryStateMachine`. `GEN_DIR` is four levels up, same as 01/02. Flash HAL + `hal_fmc.c` join the link for persist.

```sh
cd Sw
cp wifi_secrets.h.example wifi_secrets.h   # once; then edit
tdf.py build
tdf.py flash
tdf.py monitor
```

**Verified on the connected board:** `.text` 19440 bytes, flash **Verified OK**. After join, UART showed `ESP32: heartbeat still connected` (CMSIS-DAP serial still drops across SWD reset; `tdf.py monitor` can miss scan/join, same race as 01/02).

First boot (empty `WF01`, secrets filled):

```text
[INFO] Connect_Retry_State_Machine: boot
[INFO] persist: no saved SSID, using wifi_secrets.h "YourSSID"
[INFO] ESP32: power cycling module...
[INFO] ESP32: scanning for "YourSSID"
[INFO] ESP32: SSID visible
[INFO] ESP32: join attempt 1/3
[INFO] ESP32: WIFI GOT IP
[INFO] ESP32: CWSTATE=2
[INFO] persist: saved SSID "YourSSID" (password not logged)
[INFO] ESP32: heartbeat still connected
```

Second boot should load flash instead of secrets:

```text
[INFO] persist: loaded SSID "YourSSID"
```

Out of range (rename the AP, or use a SSID that is not on the air):

```text
[INFO] ESP32: scanning for "YourSSID"
[WARN] ESP32: SSID not visible, skip join (fail fast)
[INFO] persist: cleared last-good SSID    # only if that SSID came from flash
[INFO] ESP32: idle, not connected
```

## Walkthrough

1. [`main.c`](./Sw/Src/main.c) — unchanged from 01: `PRV_UART_Init()` clocks UART0 + UART2.
2. [`board_config.h`](./Sw/Board/board_config.h) — UART2 / PA12 only.
3. [`wifi_secrets.h.example`](./Sw/wifi_secrets.h.example) — fallback credentials.
4. [`tiremo_hal_flash`](./Sw/Tiremo_Hal/Include/tiremo_hal_flash.h) + [`tiremo_persist`](./Sw/Common/Include/tiremo_persist.h) — sector 0, `WF01`.
5. [`tiremo_esp32.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c) — 02 APIs plus `ScanForSsid`, `JoinStatus` on `JoinAP`, 512-byte ring.
6. [`prv_user_code.c`](./Sw/Src/prv_user_code.c) — load persist or secrets → Init → `ATE0` → drop NVS auto-join → scan → join 1..3 → save or clear → heartbeat or idle. Transport/init failures still hang with `ERROR`.

## Done When

An SSID that is not in `CWLAP` never starts `CWJAP`. An in-range AP connects within three attempts and the next reset logs `persist: loaded SSID`. A framed `ERROR` idles without burning the remaining retries.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
