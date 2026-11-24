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

***Board: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Section: ESP32 Connectivity / Feature Tutorials · Difficulty: Medium***

## Introduction

Tutorial 02 joins once and stops on failure. Real APs drop packets, DHCP stalls, and a saved network can be out of range. This tutorial wraps that join in **scan first**, **three attempts**, and **MCU flash for last-good credentials** — the MCU remembers what worked, not the ESP32's NVS.

NTP and BluFi come in later tutorials. Do not add them here.

## You Will Learn

- `AT+CWLAP="<ssid>"` as a fail-fast "is this SSID even visible?" check — and why an unfiltered scan breaks the next command
- Why `JoinAP` returns **got-IP / ERROR / timeout**, not a boolean
- When to **clear** last-good flash vs **retry without clearing**
- Reusing Sensors tutorial 06's flash layout with a **different magic** (`WF01`, not `TRM1`)

## Prerequisites

- [02-WiFi_Station_Join](../02-WiFi_Station_Join/)
- [06-Flash_Backed_Config](../../../Sensors_And_Storage/Feature_Tutorials/06-Flash_Backed_Config/) (same CRC/sector idea)
- A 2.4 GHz WiFi network whose name and password you know, as in tutorial 02 (this `Sw/` needs its own `wifi_secrets.h` — see [Credentials](#credentials-do-not-commit-a-password))
- The setup scripts from [Getting Started](../../../Getting_Started/) already run, so `tdf` is available
- To watch the log: `tdf monitor` or any serial terminal (Tera Term, PuTTY, screen, minicom, …) at 115200 8N1 on the board's USB serial port
- A Tiremo Cortex board (ABOV A34G43A MCU — see [Hardware Overview](../../../Hardware.md)) connected to your PC with a USB cable

## Understanding scan-then-join

`AT+CWJAP` can sit for 20 s before it admits the AP is gone. A scan answers in a few seconds, so scan first and do not even start `CWJAP` if the network is not on the air.

`AT+CWLAP="YourSSID"` lists only the access points with that name — one line per AP, so a network with several APs (a mesh or an office) shows up several times — and then ends with `OK`. `ScanForSsid` waits for that `OK` and then looks for the **quoted** name in the reply:

```c
(void)snprintf(command, sizeof(command), "AT+CWLAP=\"%s\"\r\n", pSsid);
(void)snprintf(expected, sizeof(expected), "\"%s\"", pSsid);

response[0] = '\0';
if (!TIREMO_ESP32_SendCommandCapture(command, "\r\nOK", timeoutMs,
                                     response, sizeof(response)))
{
    return false;
}

return (strstr(response, expected) != NULL);
```

When nothing with that name is on the air, the reply is just `OK`, and the function returns `false`.

Why not a plain `AT+CWLAP` and stop at the first `"YourSSID"`? That lists **every** nearby network, which in an office is easily a dozen lines. Returning as soon as our name appears leaves the rest of the list still streaming in — and it lands in the response buffer of the next command, the join. On a real board in a busy office this cost a full 20 s join timeout on every boot (`511 bytes without "WIFI GOT IP"`: the buffer was full of leftover scan lines). Worse, a name that only appears after the first 511 bytes of the list would never be seen at all, and a saved network would be cleared as "out of range". Asking for one SSID and waiting for `OK` keeps the reply short and nothing in flight.

The RX ring and capture buffer grow to **512** bytes in this tutorial. A filtered `CWLAP` reply is about 75 bytes per AP, so that is room for six APs with the same name.

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

1. A valid `WF01` record with a non-empty SSID, **and** `wifi_secrets.h` still holds the same SSID and password → use the saved record (`persist: loaded SSID`).
2. A valid record, but `wifi_secrets.h` now holds different credentials → you edited the file since the last save, so use `wifi_secrets.h` (`persist: wifi_secrets.h changed since the last save, using it`). Without this check, editing the file and reflashing would have no effect: the old saved network would keep winning, and `tdf erase-device` does not erase the data flash this record lives in.
3. No valid record → `wifi_secrets.h` (`persist: no saved SSID, using wifi_secrets.h`).

Success after join → `Persist_Save`. Scan miss on a **saved** SSID, framed ERROR, or three timeouts → `Persist_Clear`. A scan miss on a `wifi_secrets.h` SSID does not erase flash.

In this tutorial the saved record can only ever come from `wifi_secrets.h`, so case 1 simply confirms that the save/load round trip works. The pattern pays off in [05-BluFi_Provisioning](../05-BluFi_Provisioning/), where credentials can also arrive from a phone.

`AT+CWAUTOCONN=0` and `AT+CWQAP` still run first, same as tutorial 02: the **MCU** owns "remember what worked", not the module NVS.

The record holds the password **in plain text** in the MCU's data flash. That is fine for a tutorial, but anyone with an SWD probe can read it back — a product would encrypt it or keep it in protected storage.

## Credentials (do not commit a password)

This `Sw/` needs its own `wifi_secrets.h`, made from the example the same way as in [tutorial 02](../02-WiFi_Station_Join/#credentials-do-not-commit-a-password). From the repo root:

```sh
cd ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/Sw
cp wifi_secrets.h.example wifi_secrets.h
```

(In cmd.exe, use `copy` instead of `cp`.) Then fill in your network's SSID and password. You can also copy the `wifi_secrets.h` you already made in tutorial 02's `Sw/`. The file is listed in [`Sw/.gitignore`](./Sw/.gitignore), so it never ends up in a commit.

The firmware logs the SSID but not the password, as in tutorial 02: with `TIREMO_DEBUG_LEVEL=4` the driver's TX dump shows `AT+CWJAP=<credentials hidden>` instead of the join command.

## Solution

From the repo root (after creating `wifi_secrets.h` as above):

```sh
cd ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/Sw
tdf build
tdf flash
tdf monitor
```

(See [Getting_Started/01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution) for what `tdf` is. On macOS/Linux, `tdf.py` also works as an alias; on Windows, use `tdf`. Close any other terminal on the board's serial port before running `tdf monitor` — only one program can open it at a time.)

**Confirmed on real hardware:** `tdf build` compiles and links without errors on macOS and Windows 11 (`.text` about 19.5 KB; the exact size varies slightly with the toolchain version, and the linker prints the same expected, harmless `_close` / `_lseek` / `_read` / `_write is not implemented` warnings explained in [01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution)). `tdf flash` completed a full program-and-verify cycle over SWD on a real, connected Tiremo Cortex board (`** Verified OK **`), and the board joined a real 2.4 GHz network on the first attempt.

First boot (no `WF01` record yet, `wifi_secrets.h` filled in):

```text
[INFO] Connect_Retry_State_Machine: boot
[INFO] persist: no saved SSID, using wifi_secrets.h "YourSSID"
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
[INFO] ESP32: heartbeat still connected
```

After a reset, the first `persist:` line changes — the credentials now come from flash:

```text
[INFO] persist: loaded SSID "YourSSID"
```

Now change `TIREMO_WIFI_SSID` in `wifi_secrets.h` to a network that is not on the air, then build and flash again. The new file wins over the saved record, the scan fails fast, and the board idles instead of spending 3 × 20 s on joins:

```text
[INFO] Connect_Retry_State_Machine: boot
[INFO] persist: wifi_secrets.h changed since the last save, using it: "SomeOtherSSID"
[INFO] ESP32: power cycling module...
[INFO] ESP32: power-on boot delay elapsed
[INFO] TIREMO_ESP32_Init: UART2 ready
[INFO] ESP32: command echo disabled (ATE0)
[INFO] ESP32: scanning for "SomeOtherSSID"
[WARN] ESP32: SSID not visible, skip join (fail fast)
[INFO] ESP32: idle, not connected
```

That SSID came from `wifi_secrets.h`, not from flash, so the saved record is left alone: put the old SSID back, flash again, and the boot logs `persist: loaded SSID` and connects. If a **saved** SSID drops out of range instead, the same `SSID not visible` line is followed by `persist: cleared last-good SSID`.

## Walkthrough

1. [`main.c`](./Sw/Src/main.c) — unchanged from 01: `PRV_UART_Init()` clocks UART0 + UART2.
2. [`board_config.h`](./Sw/Board/board_config.h) — UART2 / PA12 only.
3. [`wifi_secrets.h.example`](./Sw/wifi_secrets.h.example) — the credentials used when nothing (or something older) is saved.
4. [`tiremo_hal_flash`](./Sw/Tiremo_Hal/Include/tiremo_hal_flash.h) + [`tiremo_persist`](./Sw/Common/Include/tiremo_persist.h) — sector 0, `WF01`.
5. [`tiremo_esp32.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c) — 02 APIs plus `ScanForSsid` (filtered `CWLAP`, waits for `OK`), `JoinStatus` on `JoinAP`, 512-byte ring, and the `AT+CWJAP` command hidden in the DEBUG-level TX dump.
6. [`prv_user_code.c`](./Sw/Src/prv_user_code.c) — pick saved record or `wifi_secrets.h` → Init → `ATE0` → drop NVS auto-join → scan → join 1..3 → save or clear → heartbeat or idle. Transport/init failures still stop with an `[ERROR]` line.

## Done When

- An in-range network connects (normally on the first attempt), and the next reset logs `persist: loaded SSID`.
- After you change `wifi_secrets.h` and flash again, the boot logs `persist: wifi_secrets.h changed since the last save, using it`.
- An SSID that is not on the air logs `SSID not visible, skip join (fail fast)` and never starts `CWJAP`.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
