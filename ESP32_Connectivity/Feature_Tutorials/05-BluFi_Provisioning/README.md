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

***Board: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Section: ESP32 Connectivity / Feature Tutorials · Difficulty: Hard***

## Introduction

Hardcoding an SSID does not scale past one bench unit. This tutorial has **no** `wifi_secrets.h`. A phone running EspBluFi pushes credentials over BLE; the MCU harvests `+SSID:` / `+PASSWORD:` / `WIFI GOT IP` (when the module reports them) and writes them to data flash (`BL01`). The next reset joins that saved AP with no advertising step.

The boot cascade (saved WiFi first, BluFi only as fallback, then NTP) is the [design tutorial](../../Design_Tutorials/01-Boot_Cascade_Saved_WiFi_Then_BluFi/). Do not add NTP here.

## You Will Learn

- `AT+BLUFINAME` then `AT+BLUFI=1`, and the async URCs `+BLUFICONN`, `+SSID:`, `+PASSWORD:`, `WIFI GOT IP`
- Why `TIREMO_ESP32_BlufiPoll()` must be the **only** UART2 consumer during the wait
- Why `IsConnected()` / `CWSTATE?` in that loop can swallow a credential URC
- Stopping BluFi (`AT+BLUFI=0`) and turning `CWAUTOCONN` back on after a successful provision
- Rejoining a provisioned network when the module never told the MCU the password (`AT+CWJAP` with no parameters)

## Prerequisites

- [04-NTP_Time_Sync](../04-NTP_Time_Sync/) conceptually (WiFi already works); this `Sw/` is join + BluFi, not SNTP
- EspBluFi on a phone: [Android](https://github.com/EspressifApp/EspBluFi-Android) / [iOS](https://apps.apple.com/app/espblufi/id1455860696)
- Phone Bluetooth on; Android also needs location enabled for BLE scan
- A 2.4 GHz WiFi network whose name and password you know — you type them into the phone app, not into a file
- The setup scripts from [Getting Started](../../../Getting_Started/) already run, so `tdf` is available
- To watch the log: `tdf monitor` or any serial terminal (Tera Term, PuTTY, screen, minicom, …) at 115200 8N1 on the board's USB serial port
- A Tiremo Cortex board (ABOV A34G43A MCU — see [Hardware Overview](../../../Hardware.md)) connected to your PC with a USB cable

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

ESP-AT is *documented* to print `+SSID:` and `+PASSWORD:` when the app sends credentials, then `WIFI CONNECTED` / `WIFI GOT IP` when station DHCP succeeds. Not every build actually does: some emit only `+BLUFICONN`, `WIFI CONNECTED` and `WIFI GOT IP`. The ESP32 module on the Tiremo Cortex board used to test this tutorial is one of those. Any line the parser does not recognise is echoed as `ESP32: BluFi URC "..."` so you can see which build you have.

When `+PASSWORD:` never arrives, the MCU never learns the password — `AT+CWJAP?` returns the SSID only, and the `BL01` record is saved with an empty password. The module itself still has the password in its own NVS. That is why `CWAUTOCONN=1` is sent **before** `AT+BLUFI=0` in the table above: the module's NVS copy becomes the only way back onto that AP, so it must be armed before anything else can fail.

On the next boot, a saved SSID with an empty password must **not** be joined with `AT+CWJAP="<ssid>",""`: the module takes that as "this network, no password" and rejects it with `ERROR` — the firmware would then clear `BL01` and ask the phone again on every boot. Instead, `JoinSaved()` sends a plain `AT+CWJAP` with no parameters, which tells the module to join the AP stored in its own NVS:

```c
if (pPassword[0] == '\0')
{
    join = TIREMO_ESP32_JoinStoredAP(JOIN_TIMEOUT_MS);   /* "AT+CWJAP\r\n" */
}
else
{
    join = TIREMO_ESP32_JoinAP(pSsid, pPassword, JOIN_TIMEOUT_MS);
}
```

`AT+BLUFI=0` also answers `+BLUFIDISCONN` first and `OK` afterwards. With a one-second timeout the wait gives up early and the trailing `OK` arrives during the *next* command, satisfying its match — an `AT+CWJAP?` right after would return with no `+CWJAP:` line and lose the SSID too. The stop therefore waits 5 s, and `GetConnectedSsid()` retries when the marker is missing.

## Persist magic `BL01`

Same sector-0 CRC layout as tutorial 03, magic **`0x424C3031` (`BL01`)**, not `WF01`. A last-good join from tutorial 03/04 is **not** treated as a BluFi provision, so the first flash of this firmware always advertises. After EspBluFi succeeds, `BL01` is written; the next reset loads it and `CWJAP`s without advertising.

Flashing this image still **overwrites** DFMC sector 0 (TRM1 / ENV1 / WF01).

## Solution

There is no `wifi_secrets.h` to create. From the repo root:

```sh
cd ESP32_Connectivity/Feature_Tutorials/05-BluFi_Provisioning/Sw
tdf build
tdf flash
tdf monitor
```

(See [Getting_Started/01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution) for what `tdf` is. On macOS/Linux, `tdf.py` also works as an alias; on Windows, use `tdf`. Close any other terminal on the board's serial port before running `tdf monitor` — only one program can open it at a time.)

**Confirmed on real hardware:** `tdf build` compiles and links without errors on macOS and Windows 11 (`.text` about 21 KB; the exact size varies slightly with the toolchain version, and the linker prints the same expected, harmless `_close` / `_lseek` / `_read` / `_write is not implemented` warnings explained in [01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution)). `tdf flash` completed a full program-and-verify cycle over SWD on a real, connected Tiremo Cortex board (`** Verified OK **`), the board was provisioned from EspBluFi, and it rejoined on its own after a reset.

### 1. Provision from the phone

With no `BL01` record yet, the boot logs `persist: no saved SSID (no wifi_secrets.h in this tutorial)` and starts advertising, with a reminder line every 5 s:

```text
[INFO] ESP32: BluFi started, advertising as "Tiremo-Cortex"
[INFO] BluFi: open EspBluFi, connect to "Tiremo-Cortex", Configure
[INFO] BluFi: still advertising as "Tiremo-Cortex"
```

On the phone: open EspBluFi, find **Tiremo-Cortex** in the device list and connect, tap **Configure**, choose **Station** mode, enter your network's SSID and password, and send. This is the log from the test board, whose ESP-AT build does not report `+SSID:` / `+PASSWORD:` (network name replaced):

```text
[INFO] ESP32: BluFi phone connected
[INFO] ESP32: BluFi URC "WIFI CONNECTED"
[INFO] ESP32: BluFi WIFI GOT IP
[WARN] ESP32: GOT IP without +SSID: URC
[WARN] ESP32: GOT IP without +PASSWORD: URC (module NVS keeps the only copy)
[INFO] ESP32: BluFi stopped
[INFO] ESP32: SSID recovered from CWJAP? "YourSSID"
[INFO] persist: saved SSID "YourSSID" (password not logged)
[INFO] ESP32: heartbeat still connected
```

The two `[WARN]` lines are expected on such a build — the SSID is recovered with `AT+CWJAP?`, and the password stays in the module. On a build that does report the credentials, you see `ESP32: BluFi +SSID:"YourSSID"` and `ESP32: BluFi +PASSWORD: received (not logged)` instead, and the password is saved in `BL01` too.

If the phone doesn't list **Tiremo-Cortex**: check that Bluetooth (and on Android, location) is on, that the board's log shows `still advertising`, and that the app's settings don't filter the device list by name.

### 2. Reset: the board rejoins without the phone

Press RESET (or run `tdf monitor` again). The board loads `BL01`, joins on its own, and never starts advertising:

```text
[INFO] BluFi_Provisioning: boot
[INFO] ESP32: power cycling module...
[INFO] ESP32: power-on boot delay elapsed
[INFO] TIREMO_ESP32_Init: UART2 ready
[INFO] ESP32: command echo disabled (ATE0)
[INFO] persist: loaded SSID "YourSSID"
[INFO] ESP32: scanning for saved SSID "YourSSID"
[INFO] ESP32: join attempt 1/3
[INFO] ESP32: no saved password, joining the AP stored in the module
[INFO] ESP32: WIFI GOT IP (saved credentials)
[INFO] ESP32: heartbeat still connected
```

The `no saved password` line appears only on a build without `+PASSWORD:`; with a saved password the board joins with `AT+CWJAP="<ssid>","<password>"` instead. The [boot cascade design tutorial](../../Design_Tutorials/01-Boot_Cascade_Saved_WiFi_Then_BluFi/) builds this saved-network-first, BluFi-as-fallback order into a complete boot sequence.

## Walkthrough

1. [`prv_user_code.c`](./Sw/Src/prv_user_code.c) — if `BL01` exists, scan/join like tutorial 03 (with the module's stored AP when no password was saved); else advertise until GOT IP.
2. [`tiremo_esp32.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c) — `BlufiStart` / `BlufiPoll` / `BlufiStop` / `EnableAutoConnect` / `JoinStoredAP`, plus tutorial 03's filtered scan and the hidden `AT+CWJAP` in the DEBUG-level TX dump.
3. [`tiremo_persist.h`](./Sw/Common/Include/tiremo_persist.h) — `BL01` record. Like tutorial 03's `WF01`, it keeps a received password in plain text in the MCU's data flash.

## Done When

- EspBluFi provisions the board with no SSID anywhere in the source, and the log shows `BluFi WIFI GOT IP` and `persist: saved SSID`.
- After a reset the log shows `persist: loaded SSID` and `WIFI GOT IP (saved credentials)` — with no `BluFi started` line and nothing to do on the phone.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
