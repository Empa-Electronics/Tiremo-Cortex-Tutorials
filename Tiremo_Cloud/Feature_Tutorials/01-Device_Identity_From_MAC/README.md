<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/Tiremo_Cloud/Feature_Tutorials/01-Device_Identity_From_MAC/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Device Identity from MAC</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Device Identity from MAC

***Board: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Section: Tiremo Cloud / Feature Tutorials · Difficulty: Medium***

## Introduction

Fleet provisioning and shadow topics both need a stable per-device name. Hardcoding `"tiremo-cortex-01"` works for one bench unit and fails the moment a second board shares the firmware. The ESP32 already has a unique 48-bit station MAC; this tutorial reads it with `AT+CIPSTAMAC?` and turns it into a 12-digit lowercase hex `deviceId` that later tutorials can use as a Thing name and MQTT client ID.

This `Sw/` does **not** join WiFi or talk to Tiremo Cloud. Identity is a hardware property, not a cloud session.

## You Will Learn

- `AT+CIPSTAMAC?` and why the wait must match `"OK"`, not `"+CIPSTAMAC:"`
- Formatting six MAC bytes as a 12-hex-digit lowercase ID with no separators
- Why identity is read from the module, not baked into firmware source

## Prerequisites

- [ESP32 Connectivity / 01-AT_Command_Transport](../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/) — this `Sw/` is that transport plus `GetHwEui()`
- The setup scripts from [Getting Started](../../../Getting_Started/) already run, so `tdf` is available
- To watch the log: `tdf monitor` or any serial terminal (Tera Term, PuTTY, screen, minicom, …) at 115200 8N1 on the board's USB serial port
- A Tiremo Cortex board (ABOV A34G43A MCU — see [Hardware Overview](../../../Hardware.md)) connected to your PC with a USB cable

Cloud Messaging is the previous *section* in the curriculum. You do not need an MQTT broker for this tutorial.

## Platform documentation

The name this tutorial produces is what later tutorials send as
`deviceIdentifier` during [fleet provisioning](https://docs.tiremo.ai/device-provisioning/getting-started)
and as the Thing name on shadow topics
([Device State Controller](https://docs.tiremo.ai/device-state-controller/introduction)).

## Understanding station MAC vs a serial number you invent

The module has two MACs. **Station** (`AT+CIPSTAMAC?`) is the address it uses as a WiFi client. SoftAP (`AT+CIPAPMAC?`) is typically the station address plus one. Fleet identity should follow the station MAC: that is the address an AP already sees, and it does not change when you later enable BluFi or SoftAP.

A typical reply (example address):

```text
+CIPSTAMAC:"aa:bb:cc:11:22:33"
OK
```

`TIREMO_ESP32_GetHwEui()` waits for the terminating `"OK"` and then parses the quoted, colon-separated hex. Matching `"+CIPSTAMAC:"` would return as soon as those eleven characters arrived — one byte at a time — and leave the quoted address still in flight. The same trap is documented for `AT+CWJAP?` in [ESP32 Connectivity / 02-WiFi_Station_Join](../../../ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/).

The six bytes then become a canonical ID:

| Form | Example | Who uses it |
|------|---------|-------------|
| Colon MAC (log only) | `aa:bb:cc:11:22:33` | Humans |
| `deviceId` | `aabbcc112233` | Thing name / MQTT client ID later |

Lowercase, no `0x`, no colons. Thing names are case-sensitive; picking one canonical spelling here means fleet-provisioning templates and shadow topics never disagree.

`TIREMO_Identity_FromEui()` lives in [`tiremo_identity.c`](./Sw/Common/Src/tiremo_identity.c) so the cloud tutorials that need the name — [03](../03-Fleet_Provisioning_By_Claim/) onward — reuse the same string without writing their own formatter. The driver only ever returns raw bytes.

## Solution

From the repo root:

```sh
cd Tiremo_Cloud/Feature_Tutorials/01-Device_Identity_From_MAC/Sw
tdf build
tdf flash
tdf monitor
```

(See [Getting_Started/01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution) for what `tdf` is. On macOS/Linux, `tdf.py` also works as an alias; on Windows, use `tdf`. Close any other terminal on the board's serial port before running `tdf monitor` — only one program can open it at a time.)

**Confirmed on real hardware:** `tdf build` compiles and links without errors on macOS and Windows 11 (`.text` about 14.9 KB; the exact size varies slightly with the toolchain version, and the linker prints the same expected, harmless `_close` / `_lseek` / `_read` / `_write is not implemented` warnings explained in [01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution)). `tdf flash` completed a full program-and-verify cycle over SWD on a real, connected Tiremo Cortex board (`** Verified OK **`), and the UART log showed (the board's MAC is replaced below):

```text
[INFO] Device_Identity_From_MAC: boot
[INFO] ESP32: power cycling module...
[INFO] ESP32: power-on boot delay elapsed
[INFO] TIREMO_ESP32_Init: UART2 ready
[INFO] ESP32: command echo disabled (ATE0)
[INFO] identity: station MAC aa:bb:cc:dd:ee:ff
[INFO] identity: deviceId aabbccddeeff
[INFO] identity: still aabbccddeeff
[INFO] identity: still aabbccddeeff
[INFO] identity: still aabbccddeeff
```

The heartbeat re-queries `CIPSTAMAC?` every 5 s and confirms the same 12 hex digits. If a re-read ever differs, the log shows `identity: MAC changed <old> -> <new>` as an error. A second Tiremo Cortex, read with the same `GetHwEui()` / `FromEui()` code, gave a different `deviceId`.

## Walkthrough

1. [`tiremo_esp32.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c) — `TIREMO_ESP32_GetHwEui()` (`AT+CIPSTAMAC?`, match `"OK"`, parse `"aa:bb:cc:dd:ee:ff"`).
2. [`tiremo_identity.c`](./Sw/Common/Src/tiremo_identity.c) — `FromEui` (12 hex) and `FormatMac` (colons, logs only).
3. [`prv_user_code.c`](./Sw/Src/prv_user_code.c) — boot log + 5 s re-read that the ID did not change.

From `GetHwEui()`:

```c
if (!TIREMO_ESP32_SendCommandCapture("AT+CIPSTAMAC?\r\n", "OK",
                                     TIREMO_ESP32_DEFAULT_CMD_TIMEOUT_MS,
                                     response, sizeof(response)))
{
    return false;
}

return prvParseMacQuoted(response, pEui);
```

## Done When

- The same 12-hex `deviceId` is printed on every boot of this board, and every `identity: still` line repeats it.
- A second board running the same image prints a different `deviceId`.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
