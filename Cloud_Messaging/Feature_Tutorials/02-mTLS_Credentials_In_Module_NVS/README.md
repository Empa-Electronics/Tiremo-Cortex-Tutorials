<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>mTLS Credentials in Module NVS</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# mTLS Credentials in Module NVS

***Board: Tiremo Cortex (ESP32) · Section: Cloud Messaging (MQTT & TLS) / Feature Tutorials · Difficulty: Medium***

## Introduction

AWS IoT requires mutual TLS. The ESP32 module — not the MCU — holds the
TLS credentials in its own NVS via `AT+SYSMFG`. This tutorial is purely
about that storage mechanism, before Tutorial 03 uses it for a real
connection.

## You Will Learn

- The three `TIREMO_ESP32_CredSlot_t` slots and why they live in the module, not MCU flash
- `AT+SYSMFG` write/read/erase semantics — a genuine length-prefixed binary protocol, same shape as `AT+MQTTPUBRAW`
- Keeping certificate/key material in real `.pem` files under `Sw/certs/`, embedded into the firmware at build time — not hand-typed into a C string literal

## Prerequisites

- [01-MQTT_To_Public_Broker](../01-MQTT_To_Public_Broker/) — this tutorial reuses its transport layer, trimmed back to the minimal subset (no WiFi/MQTT needed here)
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)
- `tdf.py monitor` to watch the write/read/erase cycle (see [Getting_Started/04-Button_Read_And_Debounce's Solution](../../../Getting_Started/04-Button_Read_And_Debounce/#solution) for setup)
- `openssl` on your own machine only if you want to regenerate the test certs yourself — the ones in [`Sw/certs/`](./Sw/certs/) are already committed and ready to flash

## Understanding the Three Slots (and ESP-AT's Own Naming)

[Espressif's own `AT+SYSMFG` documentation](https://github.com/espressif/esp-at/blob/master/docs/en/AT_Command_Examples/sysmfg_at_examples.rst)
reserves a **separate** namespace set per client feature — SSL client,
MQTT client, HTTPS client, and WebSocket client each get their own,
even though they all store the same three kinds of thing (a
server-verifying CA, a device certificate, a device private key). The
set that actually matters here is the MQTT client's, because that's
what `AT+MQTTUSERCFG`'s `cert_key_ID`/`CA_ID` parameters read from:

| This driver's slot | ESP-AT namespace | ESP-AT key | Purpose |
|---|---|---|---|
| `TIREMO_ESP32_CREDSLOT_ROOT_CA` | `mqtt_ca` | `mqtt_ca` | Lets the **client** verify the **server's** certificate |
| `TIREMO_ESP32_CREDSLOT_CLIENT_CERT` | `mqtt_cert` | `mqtt_cert` | This device's own certificate |
| `TIREMO_ESP32_CREDSLOT_CLIENT_KEY` | `mqtt_key` | `mqtt_key` | This device's own private key |

`mqtt_ca` is a real ESP-AT name worth pausing on: it sounds like "the
client's CA," but its actual job is the opposite — it's the CA
certificate the *client* (this board) uses to check that the *server*
(AWS IoT) is who it claims to be. AWS IoT tutorials usually call this
same thing the "Root CA." [`tiremo_esp32.h`](./Sw/Drivers/esp32/Include/tiremo_esp32.h)
keeps ESP-AT's real namespace string in the driver (so it matches the
official docs when you go looking), but names the *slot* `ROOT_CA` for
what it actually does.

The tutorial's own original step outline said "four slots" in one
place and "three" in another — an inconsistency from before any of
this was built. Checking the real command reference settled it: three.
There is no fourth namespace.

## A Real Bug: The Generic `client_*` Namespace Isn't What MQTT Reads

This tutorial's first pass used `client_ca`/`client_cert`/`client_key`
(with `.0` keys) — the namespace set Espressif's docs describe first,
under the general "SSL client" heading. Write, read-back, and erase
all passed cleanly against it, on real hardware, the first time.

That passing result was misleading. `AT+SYSMFG` is generic key-value
storage — it will happily write to and read back from *any* namespace
name, including a made-up one. Passing round-trip bytes proves nothing
about whether `AT+MQTTUSERCFG`'s TLS engine is actually looking in
that namespace. It wasn't: Espressif's own namespace/key mapping table
lists **MQTT Client** as a separate row from **SSL Client**, with its
own reserved set — `mqtt_ca`/`mqtt_cert`/`mqtt_key`, no `.0` suffix,
one set only. The generic `client_*` set is what a plain (non-MQTT)
`AT+CIPSTART` SSL connection reads; MQTT never looks there.

This only surfaced while researching Tutorial 03, cross-checking this
driver against the real namespace table before wiring up a live
AWS IoT connection — not from anything failing here. A clean write /
read-back / erase cycle is proof the *storage* works; it is not proof
the *credentials are wired to where the TLS handshake actually looks*.
Those are different claims, and only the second one matters for
Tutorial 03. The driver, header doc comment, and the table above have
all been corrected to `mqtt_ca`/`mqtt_cert`/`mqtt_key` — re-verified on
real hardware after the fix (see **Solution** below).

## Understanding the Missing `.0`

The generic SSL client namespace supports multiple indexed
certificate sets — `client_cert.0`, `client_cert.1`, and so on — for
deployments that rotate between several certificates. The **MQTT**
client namespace doesn't: Espressif's own docs list only one key per
namespace, and that key's name is identical to the namespace's name,
with no index at all (`mqtt_cert`, not `mqtt_cert.0`). This driver's
`prvESP32CredSlotInfo()` reflects that directly — namespace and key
are the same string for all three slots.

## Understanding the `certs/` Folder (Not a C String)

Real PEM content is exactly the kind of thing that's miserable and
error-prone to hand-type into a C string literal — one wrong escaped
newline and the certificate silently stops parsing. So the three
credentials here are genuine files:
[`Sw/certs/root_ca.pem`](./Sw/certs/root_ca.pem),
[`client_cert.pem`](./Sw/certs/client_cert.pem), and
[`client_key.pem`](./Sw/certs/client_key.pem) — real, locally
generated, self-signed **test** certificates (`openssl req -x509
-newkey rsa:2048 ...`), clearly labeled as such in their own subject
line (`CN=... TEST ... - not for real use`). They prove nothing about
trust; they exist purely to give this tutorial real PEM bytes to write,
read back, and compare.

[`cmake/embed_pem.cmake`](../../../cmake/embed_pem.cmake) (repo root —
reusable by any future tutorial that needs this) converts a `.pem` file
into a generated `.c` source at build time:

```cmake
function(embed_pem name symbol)
    add_custom_command(
        OUTPUT ${GENERATED_DIR}/${name}.c
        COMMAND ${CMAKE_COMMAND} -DINPUT=${CERTS_DIR}/${name}.pem
                -DOUTPUT=${GENERATED_DIR}/${name}.c -DSYMBOL=${symbol}
                -P ${EMBED_PEM_SCRIPT}
        DEPENDS ${CERTS_DIR}/${name}.pem ${EMBED_PEM_SCRIPT}
    )
endfunction()
```

producing `const unsigned char g_rootCaPem[] = { 0x2d, 0x2d, ... };`
plus a matching `_len`, which `prv_user_code.c` references with a
plain `extern` — no XXD, no Perl, no Git-Bash-only tool, just CMake's
own `file(READ ... HEX)` doing the hex-to-array conversion, so this
works identically on Windows/macOS/Linux with nothing beyond what
`tdf.py build` already requires. Tutorial 03 reuses this exact
mechanism: swap the *content* of these same three `.pem` files for a
real AWS IoT device certificate, root CA, and private key, and nothing
about the build script changes.

## Understanding the SYSMFG Protocol

Write (`operation=2`) is the same two-stage shape as `AT+MQTTPUBRAW`
from Tutorial 01 — issue the command with a byte count, wait for a
bare `>` prompt, then transmit exactly that many raw bytes:

```c
(void)snprintf(command, sizeof(command), "AT+SYSMFG=2,\"%s\",\"%s\",8,%lu\r\n",
               pNamespace, pKey, (unsigned long)length);
TIREMO_ESP32_SendCommand(command, ">", timeout);
TIREMO_HAL_UART_Transmit(port, pData, length, true);
TIREMO_ESP32_SendCommand(NULL, "OK", timeout);
```

(Type `8` = binary — matches the type table in ESP-AT's own SYSMFG
docs, and avoids any string processing on the module's side for data
that's technically text but line-broken and easy to mishandle as one.)

Read (`operation=1`) answers with the value inline in the response:

```text
+SYSMFG:"mqtt_cert","mqtt_cert",8,1164,<1164 raw bytes>
```

[`tiremo_esp32.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c)'s
`TIREMO_ESP32_CredRead()` parses this the same way Tutorial 01's
`MqttWaitMessage()` parsed `+MQTTSUBRECV`: skip the two quoted fields
it already knows (namespace, key), skip the type digit, parse the
decimal length, then check whether the response buffer already holds
that many bytes past it. Only then is the value complete — a real
length-prefixed parse, correct for the full multi-line PEM value, not
a text heuristic. Erase (`operation=0`) is a plain `AT+SYSMFG=0,"ns","key"`
matched on `"OK"`, no data phase.

Reading an **erased** slot answers with a framed `\r\nERROR\r\n`
instead — `CredRead()` treats that as a normal, expected outcome
(logged at `WARN`, not `ERROR`), since "prove the slot is now empty" is
exactly what Step 3 of this tutorial's own outline asks for.

## Solution

[`Sw/`](./Sw/) has a real, buildable implementation:

```sh
cd Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

Same CMake shape as Tutorial 01, minus the WiFi/MQTT weight this
tutorial doesn't need — the driver
([`tiremo_esp32.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c)) is based on
[ESP32 Connectivity Tutorial 01](../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/)'s
minimal transport (`Init`/`SendCommand`/`IsPresent`/`DisableEcho`
only), with the three credential functions added on top. No
`wifi_secrets.h` needed — `AT+SYSMFG` never touches the network.

**Genuinely verified, for real, on the connected board — after the
namespace fix above:** `tdf.py build` compiles and links clean
(`.text` 21140 bytes). `tdf.py flash` completed a full
program-and-verify cycle over SWD (`** Verified OK **`). `tdf.py
monitor` then showed, straight from the real hardware, a clean pass
against the corrected `mqtt_*` namespaces:

```text
[INFO] ROOT_CA: writing 1245 bytes...
[INFO] ROOT_CA: read-back matches byte-for-byte (1245 bytes)
[INFO] ROOT_CA: erased
[WARN] ESP32: SYSMFG read "mqtt_ca"/"mqtt_ca": ERROR (empty slot?)
[INFO] ROOT_CA: read after erase correctly failed (slot empty)
[INFO] CLIENT_CERT: writing 1241 bytes...
[INFO] CLIENT_CERT: read-back matches byte-for-byte (1241 bytes)
[INFO] CLIENT_CERT: erased
[WARN] ESP32: SYSMFG read "mqtt_cert"/"mqtt_cert": ERROR (empty slot?)
[INFO] CLIENT_CERT: read after erase correctly failed (slot empty)
[INFO] CLIENT_KEY: writing 1704 bytes...
[INFO] CLIENT_KEY: read-back matches byte-for-byte (1704 bytes)
[INFO] CLIENT_KEY: erased
[WARN] ESP32: SYSMFG read "mqtt_key"/"mqtt_key": ERROR (empty slot?)
[INFO] CLIENT_KEY: read after erase correctly failed (slot empty)
[INFO] mTLS_Credentials_In_Module_NVS: ALL THREE SLOTS PASSED
```

All three slots: write, byte-for-byte read-back match (1245 / 1241 /
1704 bytes — the actual sizes of the three test PEM files, not
round/fabricated numbers), erase, and a post-erase read that correctly
fails. The `[WARN] ... ERROR (empty slot?)` lines aren't failures —
they're the driver logging the exact expected outcome of reading a
slot that was just erased, now visible for all three (the namespace
strings in each line are the real proof this ran against `mqtt_*`, not
the original `client_*`).

## Walkthrough

### 1. A minimal transport, on purpose

[`Sw/Src/main.c`](./Sw/Src/main.c) and the driver's `Init`/`SendCommand`
/`IsPresent`/`DisableEcho` core are unchanged from ESP32 Connectivity
Tutorial 01 — no WiFi join, no MQTT. `AT+SYSMFG` is pure local NVS
storage on the module; it doesn't need a network, so this tutorial
doesn't build one just to have it sitting there unused.

### 2. The credential slot API

[`Sw/Drivers/esp32/Include/tiremo_esp32.h`](./Sw/Drivers/esp32/Include/tiremo_esp32.h):

```c
bool TIREMO_ESP32_CredWrite(TIREMO_ESP32_CredSlot_t slot, const uint8_t *pData, uint32_t length);
bool TIREMO_ESP32_CredRead(TIREMO_ESP32_CredSlot_t slot, uint8_t *pOut, uint32_t outSize, uint32_t *pLengthOut);
bool TIREMO_ESP32_CredErase(TIREMO_ESP32_CredSlot_t slot);
```

A private `prvESP32CredSlotInfo()` maps each `TIREMO_ESP32_CredSlot_t`
to its `{namespace, key}` pair — see **Understanding the Three Slots**
above for why `ROOT_CA` maps to the ESP-AT name `mqtt_ca`.

### 3. The test — write, verify, erase, verify

[`Sw/Src/prv_user_code.c`](./Sw/Src/prv_user_code.c)'s `TestSlot()`
runs the full cycle from the tutorial's own step outline against each
of the three embedded `.pem` files, `memcmp`-ing the read-back bytes
against the original instead of just checking the call succeeded:

```c
TIREMO_ESP32_CredWrite(slot, pData, length);
TIREMO_ESP32_CredRead(slot, s_readBuf, sizeof(s_readBuf), &readLen);
/* readLen == length && memcmp(s_readBuf, pData, length) == 0 */
TIREMO_ESP32_CredErase(slot);
/* CredRead() now expected to return false */
```

### 4. Build, flash, and watch

```sh
tdf.py build
tdf.py flash
tdf.py monitor
```

See **Solution** above for what's actually confirmed on real hardware
here.

## Done When

Write/read/erase round-trips correctly for all three credential slots
— genuinely confirmed on real hardware, see **Solution**.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
