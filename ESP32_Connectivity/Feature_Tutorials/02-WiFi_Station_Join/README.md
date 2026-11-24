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

***Board: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Section: ESP32 Connectivity / Feature Tutorials · Difficulty: Medium***

## Introduction

The simplest possible WiFi connection: one hardcoded network, one join attempt, no retries. Tutorial 01 proved the UART can say `AT` and hear `OK`. This tutorial is the first *useful* AT sequence — and the first place substring-match can lie to you.

Retries, scan-first, and flash-backed last-good SSID are Tutorial 03. Do not add them here.

## You Will Learn

- `AT+CWMODE=1`, `AT+CWJAP`, `AT+CWSTATE?`
- Why `JoinAP` waits for `"WIFI GOT IP"` rather than `"OK"`
- Why `IsConnected` matches `"+CWSTATE:2"` rather than `"+CWSTATE:"`
- Why `GetConnectedSsid` matches `"OK"` rather than `"+CWJAP:"` — the quoted name arrives *after* that marker, one byte at a time
- How to read a failed join (`+CWJAP:<code>`)

## Prerequisites

- [01-AT_Command_Transport](../01-AT_Command_Transport/)
- A 2.4 GHz WiFi network whose name and password you know (the ESP32 only supports 2.4 GHz)
- The setup scripts from [Getting Started](../../../Getting_Started/) already run, so `tdf` is available
- To watch the log: `tdf monitor` or any serial terminal (Tera Term, PuTTY, screen, minicom, …) at 115200 8N1 on the board's USB serial port
- A Tiremo Cortex board (ABOV A34G43A MCU — see [Hardware Overview](../../../Hardware.md)) connected to your PC with a USB cable

## Understanding the three commands

ESP-AT treats WiFi as a small state machine you drive with text:

| Command | Meaning | This tutorial waits for |
|---------|---------|-------------------------|
| `AT+CWMODE=1` | station only, not AP or AP+STA | `"OK"` |
| `AT+CWJAP="ssid","pass"` | associate + DHCP | `"WIFI GOT IP"` |
| `AT+CWSTATE?` | current link state | `"+CWSTATE:2"` |

State 2 is "connected, has IPv4". 0 means no connection has been started, 1 is connected to the AP but no IPv4 address yet, 3 is connecting or reconnecting, 4 is disconnected. Confirming with `CWSTATE?` after `WIFI GOT IP` is not redundant: the URC can fire, then the module can drop the lease before your app has noticed.

`AT+CWAUTOCONN=0` plus `AT+CWQAP` run *before* join. The module keeps the last AP in its own NVS and will otherwise reconnect to that network on power-up, ignoring the SSID in `wifi_secrets.h`. Tutorial 03 is where "remember what worked" belongs — on the MCU, not as an accidental ESP-AT side effect.

## Understanding the substring gotcha

Tutorial 01's `SendCommand` returns as soon as `pExpected` appears **anywhere in the bytes received so far**. The ISR pushes **one byte at a time**. That combination is a foot-gun the moment the interesting data comes *after* a tag you might match on.

Three concrete cases in this file:

**1. `CWJAP` — match the event, not `"OK"`.**

On a successful join the module first reports two status lines, and only then ends the command with `OK`. This is the reply as captured on the board with `TIREMO_DEBUG_LEVEL=4` (see [Solution](#solution)); the capture stops at `WIFI GOT IP` because that is what `JoinAP` waits for:

```text
{"data":"WIFI CONNECTED\r\nWIFI GOT IP"}
```

`OK` only says "this command has finished" — it is the same marker every command ends with. What you actually need is an IPv4 address, and `WIFI GOT IP` is the line that says so. `JoinAP` therefore waits for exactly that event:

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

`AT+CWJAP?` looks like this (captured on the board; SSID and BSSID replaced):

```text
+CWJAP:"YourSSID","aa:bb:cc:dd:ee:ff",6,-54,0,1,3,0,1

OK
```

After the SSID and the AP's MAC address (BSSID) come the channel (`6`) and the signal strength in dBm (`-54`).

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

The SSID and password live in `Sw/wifi_secrets.h`, which you create from the example. That file is listed in [`Sw/.gitignore`](./Sw/.gitignore), so your password never ends up in a commit. From the repo root:

```sh
cd ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/Sw
cp wifi_secrets.h.example wifi_secrets.h
```

(`cp` works in PowerShell, bash, and zsh; in cmd.exe, use `copy wifi_secrets.h.example wifi_secrets.h`.) Then open `wifi_secrets.h` and fill in your network:

```c
#define TIREMO_WIFI_SSID     "YourSSID"
#define TIREMO_WIFI_PASSWORD "YourPassword"
```

Rebuild after editing. If you forget this step, the build stops with `Copy Sw/wifi_secrets.h.example to Sw/wifi_secrets.h and set TIREMO_WIFI_SSID / TIREMO_WIFI_PASSWORD`.

The firmware logs the SSID but not the password: at the default log level it never prints the `AT+CWJAP` command, and with `TIREMO_DEBUG_LEVEL=4` the driver replaces that command in the TX dump with `AT+CWJAP=<credentials hidden>`. Still, keep the board's log in mind before you share it — it does contain your network's name.

## Solution

From the repo root (after creating `wifi_secrets.h` as above):

```sh
cd ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/Sw
tdf build
tdf flash
tdf monitor
```

(See [Getting_Started/01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution) for what `tdf` is. On macOS/Linux, `tdf.py` also works as an alias; on Windows, use `tdf`. Close any other terminal on the board's serial port before running `tdf monitor` — only one program can open it at a time.)

The new functions live in the same `tiremo_esp32.c` as tutorial 01, not in a second UART stack.

**Confirmed on real hardware:** `tdf build` compiles and links without errors on macOS and Windows 11 (`.text` about 15 KB; the exact size varies slightly with the toolchain version, and the linker prints the same expected, harmless `_close` / `_lseek` / `_read` / `_write is not implemented` warnings explained in [01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution)). `tdf flash` completed a full program-and-verify cycle over SWD on a real, connected Tiremo Cortex board (`** Verified OK **`), and the board joined a real 2.4 GHz network. A successful join looks like this:

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

The join usually takes a few seconds; the firmware gives up after 20 s. After that, a `heartbeat still connected` line follows every 5 s.

### When the join fails

If the join fails, ESP-AT answers `+CWJAP:<code>` followed by `ERROR`, and the wait ends right away instead of running out the 20 s. At the default log level you see:

```text
[INFO] ESP32: joining SSID "YourSSID"
[WARN] ESP32 RX: "WIFI GOT IP" rejected with ERROR after 19 bytes
[ERROR] ESP32: CWJAP did not report WIFI GOT IP
```

To see the code itself, rebuild with `tdf build -DTIREMO_DEBUG_LEVEL=4` and flash again; the reply is then dumped under the `[WARN]` line. This is the board's reply for a network name that doesn't exist:

```text
{"data":"+CWJAP:3\r\n\r\nERROR\r\n"}
```

| Code | Meaning | What to check |
|------|---------|---------------|
| `1` | connection timeout | the network is too far away or busy; move closer |
| `2` | wrong password | `TIREMO_WIFI_PASSWORD` in `wifi_secrets.h` |
| `3` | network not found | the SSID spelling (it is case-sensitive), and that the network is 2.4 GHz |
| `4` | connection failed | the router's settings (for example a MAC filter) |

Run `tdf build -DTIREMO_DEBUG_LEVEL=` (empty) to go back to the default level afterwards.

## Walkthrough

1. [`main.c`](./Sw/Src/main.c) — unchanged from 01: `PRV_UART_Init()` clocks UART0 + UART2.
2. [`board_config.h`](./Sw/Board/board_config.h) — still UART2 / PA12 only. No WiFi knowledge belongs in the pin map.
3. [`wifi_secrets.h`](./Sw/wifi_secrets.h.example) — the only place the password lives. Application code, not the driver.
4. [`tiremo_esp32.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c) — adds `SetStationMode`, `DisableAutoConnect`, `JoinAP`, `Disconnect`, `IsConnected`, `GetConnectedSsid`. The transport from 01 is unchanged, except that its DEBUG-level TX dump hides the `AT+CWJAP` command so the password never reaches the log.
5. [`prv_user_code.c`](./Sw/Src/prv_user_code.c) — Init → present → `ATE0` → drop NVS auto-join → `CWMODE=1` → `CWJAP` → `CWSTATE=2` → print SSID → 5 s heartbeat. Failures stop with an `[ERROR]` line instead of rebooting, so a bad password stays visible on the screen.

## Done When

After every reset, the board joins your network on its own — `WIFI GOT IP`, `CWSTATE=2 (connected, IPv4)`, and `CWJAP? SSID=` with your network's name — and `heartbeat still connected` keeps arriving every 5 s.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
