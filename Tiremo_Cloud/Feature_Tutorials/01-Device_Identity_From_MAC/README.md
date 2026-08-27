<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/AWS_IoT_Core/Feature_Tutorials/01-Device_Identity_From_MAC/README.md">Türkçe</a></p>
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
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Device Identity from MAC

***Board: Tiremo Cortex (ESP32) · Section: AWS IoT Core / Feature Tutorials · Difficulty: Medium***

## Introduction

Fleet provisioning and shadow topics both need a stable per-device name. Hardcoding `"tiremo-cortex-01"` works for one bench unit and fails the moment a second board shares the firmware. The ESP32 already has a unique 48-bit station MAC; this tutorial reads it with `AT+CIPSTAMAC?` and turns it into a 12-digit lowercase hex `deviceId` that later tutorials can use as a Thing name and MQTT client ID.

This `Sw/` does **not** join WiFi or talk to AWS. Identity is a hardware property, not a cloud session.

## You Will Learn

- `AT+CIPSTAMAC?` and why the wait must match `"OK"`, not `"+CIPSTAMAC:"`
- Formatting six MAC bytes as a 12-hex-digit lowercase ID with no separators
- Why identity is read from the module, not baked into firmware source

## Prerequisites

- [ESP32 Connectivity / 01-AT_Command_Transport](../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/) — this `Sw/` is that transport plus `GetHwEui()`
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)
- `tdf.py monitor` to watch the ID (see [Getting_Started/04-Button_Read_And_Debounce's Solution](../../../Getting_Started/04-Button_Read_And_Debounce/#solution) for setup)

Cloud Messaging is the previous *section* in the curriculum. You do not need an MQTT broker for this tutorial.

## Understanding station MAC vs a serial number you invent

The module has two MACs. **Station** (`AT+CIPSTAMAC?`) is the address it uses as a WiFi client. SoftAP (`AT+CIPAPMAC?`) is typically the station address plus one. Fleet identity should follow the station MAC: that is the address an AP already sees, and it does not change when you later enable BluFi or SoftAP.

A typical reply:

```text
+CIPSTAMAC:"30:ed:a0:80:63:24"
OK
```

`TIREMO_ESP32_GetHwEui()` waits for the terminating `"OK"` and then parses the quoted, colon-separated hex. Matching `"+CIPSTAMAC:"` would return as soon as those eleven characters arrived — one byte at a time — and leave the quoted address still in flight. The same trap is documented for `AT+CWJAP?` in the WiFi tutorials.

The six bytes then become a canonical ID:

| Form | Example | Who uses it |
|------|---------|-------------|
| Colon MAC (log only) | `30:ed:a0:80:63:24` | Humans |
| `deviceId` | `30eda0806324` | Thing name / MQTT client ID later |

Lowercase, no `0x`, no colons. AWS IoT Thing names are case-sensitive; picking one canonical spelling here means fleet-provisioning templates and shadow topics never disagree.

`TIREMO_Identity_FromEui()` lives in [`tiremo_identity.c`](./Sw/Common/Src/tiremo_identity.c) so tutorial 02 can reuse the same string without copying a formatter. The driver only ever returns raw bytes.

## Solution

```sh
cd Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

**Verified on the connected board:** `.text` 15060 bytes, flash **Verified OK**. UART:

```text
[INFO] Device_Identity_From_MAC: boot
[INFO] ESP32: power cycling module...
[INFO] ESP32: power-on boot delay elapsed
[INFO] TIREMO_ESP32_Init: UART2 ready
[INFO] ESP32: command echo disabled (ATE0)
[INFO] identity: station MAC 30:ed:a0:80:63:24
[INFO] identity: deviceId 30eda0806324
[INFO] identity: still 30eda0806324
```

The heartbeat re-queries `CIPSTAMAC?` every 5 s and confirms the same 12 hex digits. Uniqueness across two physical boards is a human check: flash the same image on a second Cortex and the `deviceId` line must differ.

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

The same `deviceId` is produced on every boot of this board, and a second board (when you have one) prints a different 12-hex string. Same-boot stability is **confirmed** on hardware (`30eda0806324` held across heartbeat re-reads). Two-board uniqueness needs a second unit.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
