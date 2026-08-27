<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>WiFi Station Join</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# WiFi Station Join

***Board: Tiremo Cortex (ESP32) · Section: ESP32 Connectivity / Feature Tutorials · Difficulty: Medium***

## Introduction

The simplest possible WiFi connection: one hardcoded network, one join attempt, no retries. Tutorial 01 proved the UART can say `AT` and hear `OK`. This tutorial is the first *useful* AT sequence — and the first place substring-match can lie to you.

Retries, scan-first, and flash-backed last-good SSID are Tutorial 03. Do not add them here.

## You Will Learn

- `AT+CWMODE=1`, `AT+CWJAP`, `AT+CWSTATE?`
- Why `JoinAP` waits for `"WIFI GOT IP"` rather than `"OK"`
- Why `IsConnected` matches `"+CWSTATE:2"` rather than `"+CWSTATE:"`
- Why `GetConnectedSsid` matches `"OK"` rather than `"+CWJAP:"` — the quoted name arrives *after* that marker, one byte at a time

## Prerequisites

- [01-AT_Command_Transport](../01-AT_Command_Transport/)
- A 2.4 GHz access point you control (ESP-AT station mode is 2.4 GHz)
- `tdf.py` from [Getting Started](../../../Getting_Started/)

## Understanding the three commands

ESP-AT treats WiFi as a small state machine you drive with text:

| Command | Meaning | This tutorial waits for |
|---------|---------|-------------------------|
| `AT+CWMODE=1` | station only, not AP or AP+STA | `"OK"` |
| `AT+CWJAP="ssid","pass"` | associate + DHCP | `"WIFI GOT IP"` |
| `AT+CWSTATE?` | current link state | `"+CWSTATE:2"` |

State 2 is "connected, has IPv4". 0 is idle, 1 is connecting, 3 is connected but no IP yet, 4 is disconnected. Confirming with `CWSTATE?` after `WIFI GOT IP` is not redundant: the URC can fire, then the module can drop the lease before your app has noticed.

`AT+CWAUTOCONN=0` plus `AT+CWQAP` run *before* join. The module keeps the last AP in its own NVS and will otherwise reconnect to that network on power-up, ignoring the SSID in `wifi_secrets.h`. Tutorial 03 is where "remember what worked" belongs — on the MCU, not as an accidental ESP-AT side effect.

## Understanding the substring gotcha

Tutorial 01's `SendCommand` returns as soon as `pExpected` appears **anywhere in the bytes received so far**. The ISR pushes **one byte at a time**. That combination is a foot-gun the moment the interesting data comes *after* a tag you might match on.

Three concrete cases in this file:

**1. `CWJAP` — do not match `"OK"`.**

The module can ACK the command while DHCP is still running:

```text
OK
WIFI CONNECTED
WIFI GOT IP
```

Matching `"OK"` returns before you have an address. `JoinAP` therefore waits for `"WIFI GOT IP"`:

```c
(void)snprintf(command, sizeof(command), "AT+CWJAP=\"%s\",\"%s\"\r\n",
               pSsid, pPassword);

return TIREMO_ESP32_SendCommand(command, "WIFI GOT IP", timeoutMs);
```

**2. `CWSTATE?` — do not match `"+CWSTATE:"`.**

The reply is typed out as `+`, `C`, `W`, … `:`, then the digit. Matching the prefix returns on state 0, 1, 3, or 4 just as happily as 2. `IsConnected` matches the digit too:

```c
bool TIREMO_ESP32_IsConnected(void)
{
    return TIREMO_ESP32_SendCommand("AT+CWSTATE?\r\n",
                                    "+CWSTATE:2",
                                    TIREMO_ESP32_STATUS_TIMEOUT_MS);
}
```

**3. `CWJAP?` — *do* match `"OK"`, then parse.**

`AT+CWJAP?` looks like:

```text
+CWJAP:"YourSSID","aa:bb:cc:dd:ee:ff",1,-40,0,0,0,0,0
OK
```

The quoted SSID sits *after* `+CWJAP:`. Matching that marker returns the instant the colon arrives — `GetConnectedSsid` would copy nothing. Matching the terminating `"OK"` waits until the whole line is in the capture buffer, then `strstr` / `strchr` pull out the quotes:

```c
if (!TIREMO_ESP32_SendCommandCapture("AT+CWJAP?\r\n", "OK",
                                     TIREMO_ESP32_STATUS_TIMEOUT_MS,
                                     response, sizeof(response)))
{
    return false;
}

pMarker = strstr(response, "+CWJAP:");
pStart = strchr(pMarker, '"');
pStart++;
pEnd = strchr(pStart, '"');
```

Rule of thumb: if the payload you need is **after** a tag on the same line, match the **end** of the response (`"OK"`), then parse. If the payload **is** the tag (`WIFI GOT IP`, `+CWSTATE:2`), match that.

## Credentials (do not commit a password)

Copy the example and edit it. `wifi_secrets.h` is in [`Sw/.gitignore`](./Sw/.gitignore).

```sh
cd Sw
cp wifi_secrets.h.example wifi_secrets.h
```

```c
#define TIREMO_WIFI_SSID     "YourSSID"
#define TIREMO_WIFI_PASSWORD "YourPassword"
```

Rebuild after editing. The firmware logs the SSID, never the password.

## Solution

[`Sw/`](./Sw/) has a real, buildable implementation:

```sh
cd Sw
cp wifi_secrets.h.example wifi_secrets.h   # once; then edit
tdf.py build
tdf.py flash
tdf.py monitor
```

Same CMake shape as tutorial 01 (`GEN_DIR` four levels up). New APIs live in the same `tiremo_esp32.c`, not a second UART stack.

**Verified on the connected board:** `.text` 15200 bytes, flash **Verified OK**. Join confirmed on a real 2.4 GHz AP after filling gitignored `wifi_secrets.h` (the committed example is still the placeholder `YourSSID`).

A successful join looks like:

```text
[INFO] WiFi_Station_Join: boot
[INFO] ESP32: power cycling module...
[INFO] ESP32: power-on boot delay elapsed
[INFO] TIREMO_ESP32_Init: UART2 ready
[INFO] ESP32: command echo disabled (ATE0)
[INFO] ESP32: station mode
[INFO] ESP32: joining SSID "YourSSID"
[INFO] ESP32: WIFI GOT IP
[INFO] ESP32: CWSTATE=2 (connected, IPv4)
[INFO] ESP32: CWJAP? SSID="YourSSID"
[INFO] ESP32: heartbeat still connected
```

`tdf.py monitor` still resets then opens the port, so it can miss the power-cycle lines (same race as tutorial 01). Join itself takes up to 20 s.

## Walkthrough

1. [`main.c`](./Sw/Src/main.c) — unchanged from 01: `PRV_UART_Init()` clocks UART0 + UART2.
2. [`board_config.h`](./Sw/Board/board_config.h) — still UART2 / PA12 only. No WiFi knowledge belongs in the pin map.
3. [`wifi_secrets.h`](./Sw/wifi_secrets.h.example) — the only place the password lives. Application code, not the driver.
4. [`tiremo_esp32.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c) — adds `SetStationMode`, `DisableAutoConnect`, `JoinAP`, `Disconnect`, `IsConnected`, `GetConnectedSsid`. Transport from 01 is untouched.
5. [`prv_user_code.c`](./Sw/Src/prv_user_code.c) — Init → present → `ATE0` → drop NVS auto-join → `CWMODE=1` → `CWJAP` → `CWSTATE=2` → print SSID → 5 s heartbeat. Failures hang with an `ERROR` so a bad password is visible instead of a reboot loop.

## Done When

The board joins a real AP after a fresh power-up with no typing at the monitor, and `CWSTATE?` reports state 2. Edit `wifi_secrets.h`, flash, and confirm the log above.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
