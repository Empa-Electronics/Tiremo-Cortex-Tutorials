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
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# mTLS Credentials in Module NVS

***Board: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Section: Cloud Messaging (MQTT & TLS) / Feature Tutorials · Difficulty: Medium***

## Introduction

Tiremo Cloud requires mutual TLS: the server proves who it is to the board, and the board proves who it is to the server. The ESP32 module — not the MCU — holds the TLS credentials, in its own NVS, written with `AT+SYSMFG`. This tutorial is purely about that storage mechanism, before [tutorial 03](../03-MQTT_mTLS_To_AWS_IoT/) uses it for a real connection.

## You Will Learn

- The three `TIREMO_ESP32_CredSlot_t` slots, and why they live in the module, not in MCU flash
- `AT+SYSMFG` write / read / erase — a length-prefixed binary protocol, the same shape as `AT+MQTTPUBRAW` in tutorial 01
- Keeping certificate and key material in real `.pem` files under `Sw/certs/`, embedded into the firmware at build time — not hand-typed into a C string literal

## Prerequisites

- [01-MQTT_To_Public_Broker](../01-MQTT_To_Public_Broker/) — this tutorial reuses its transport layer, trimmed back to the minimal subset (no WiFi or MQTT needed here)
- `openssl` on your PC to generate a test certificate — see [Generate the test certificate](#generate-the-test-certificate)
- The setup scripts from [Getting Started](../../../Getting_Started/) already run, so `tdf` is available
- To watch the log: `tdf monitor` or any serial terminal (Tera Term, PuTTY, screen, minicom, …) at 115200 8N1 on the board's USB serial port
- A Tiremo Cortex board (ABOV A34G43A MCU — see [Hardware Overview](../../../Hardware.md)) connected to your PC with a USB cable

> **Warning — this tutorial erases the module's MQTT credentials.** The test writes test data into the ESP32's three MQTT certificate slots (`mqtt_ca`, `mqtt_cert`, `mqtt_key`) and then **erases** them. If the module on your board already holds a real device certificate (for example from tutorial 03 or from Tiremo Cloud provisioning), it is gone afterwards, and the board can't connect to the cloud until the certificate is written again.

## Understanding the three slots (and ESP-AT's own naming)

[Espressif's `AT+SYSMFG` documentation](https://github.com/espressif/esp-at/blob/master/docs/en/AT_Command_Examples/sysmfg_at_examples.rst) reserves a **separate** set of namespaces per client feature — SSL client, MQTT client, HTTPS client, and WebSocket client each get their own, even though they all store the same three kinds of thing (a CA that verifies the server, a device certificate, a device private key). The set that matters here is the MQTT client's, because that is what `AT+MQTTUSERCFG`'s `cert_key_ID` / `CA_ID` parameters read from:

| This driver's slot | ESP-AT namespace | ESP-AT key | Purpose |
|---|---|---|---|
| `TIREMO_ESP32_CREDSLOT_ROOT_CA` | `mqtt_ca` | `mqtt_ca` | Lets the **client** verify the **server's** certificate |
| `TIREMO_ESP32_CREDSLOT_CLIENT_CERT` | `mqtt_cert` | `mqtt_cert` | This device's own certificate |
| `TIREMO_ESP32_CREDSLOT_CLIENT_KEY` | `mqtt_key` | `mqtt_key` | This device's own private key |

`mqtt_ca` is a name worth pausing on: it sounds like "the client's CA", but its job is the opposite — it is the CA certificate the *client* (this board) uses to check that the *server* is who it claims to be. TLS guides usually call this the "Root CA". [`tiremo_esp32.h`](./Sw/Drivers/esp32/Include/tiremo_esp32.h) keeps ESP-AT's real namespace string in the driver (so it matches the official docs when you look it up), but names the *slot* `ROOT_CA` for what it actually does.

## Understanding why it must be `mqtt_*`, not `client_*`

Espressif's docs describe the generic SSL client namespaces first: `client_ca` / `client_cert` / `client_key`, with `.0`, `.1`, … keys. Writing to those works perfectly well — and that is the trap. `AT+SYSMFG` is generic key-value storage: it will write to and read back from *any* namespace name, even a made-up one. A clean write / read-back / erase cycle proves the storage works; it does **not** prove the credentials are where the TLS handshake looks. In ESP-AT's namespace table, **MQTT client** is its own row: `AT+MQTTUSERCFG`'s TLS reads only `mqtt_ca` / `mqtt_cert` / `mqtt_key`. The `client_*` set belongs to plain (non-MQTT) SSL connections (`AT+CIPSTART`), and MQTT never looks there.

The MQTT namespaces also have no `.0`: the generic SSL client supports several indexed certificate sets (`client_cert.0`, `client_cert.1`, …) for devices that rotate between certificates, but the MQTT client has exactly one key per namespace, named the same as the namespace (`mqtt_cert`, not `mqtt_cert.0`). The driver's `prvESP32CredSlotInfo()` reflects that directly — namespace and key are the same string for all three slots.

## Understanding the `certs/` folder (not a C string)

Real PEM content is exactly the kind of thing that's miserable and error-prone to hand-type into a C string literal — one wrong escaped newline and the certificate silently stops parsing. So the three credentials here are real files in [`Sw/certs/`](./Sw/certs/):

- `root_ca.pem` is committed: a self-signed TEST root CA (`CN=... TEST Root CA - not for real use`).
- `client_cert.pem` and `client_key.pem` are gitignored: you generate a self-signed TEST pair yourself (see below). The private key must never end up in a commit.

They prove nothing about trust — no server accepts them. They exist to give this tutorial real PEM bytes to write, read back, and compare. See [`certs/README.md`](./Sw/certs/README.md).

[`cmake/embed_pem.cmake`](../../../cmake/embed_pem.cmake) (at the repo root, reusable by any tutorial that needs it) converts a `.pem` file into a generated `.c` source at build time:

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

The result is `const unsigned char g_rootCaPem[] = { 0x2d, 0x2d, ... };` plus a matching `_len`, which `prv_user_code.c` references with a plain `extern`. The conversion uses only CMake's own `file(READ ... HEX)` — no `xxd`, Perl, or Git-Bash-only tool — so it works the same on Windows, macOS, and Linux with nothing beyond what `tdf build` already needs. Tutorial 03 reuses this mechanism: swap the *content* of the same three `.pem` files for a real device certificate, its private key, and the server's root CA, and nothing about the build changes.

## Understanding the SYSMFG protocol

Write (`operation=2`) has the same two-stage shape as `AT+MQTTPUBRAW` in tutorial 01 — send the command with a byte count, wait for a bare `>` prompt, then transmit exactly that many raw bytes:

```c
(void)snprintf(command, sizeof(command), "AT+SYSMFG=2,\"%s\",\"%s\",8,%lu\r\n",
               pNamespace, pKey, (unsigned long)length);
TIREMO_ESP32_SendCommand(command, ">", timeout);
TIREMO_HAL_UART_Transmit(port, pData, length, true);
TIREMO_ESP32_SendCommand(NULL, "OK", timeout);
```

Type `8` means binary, per the type table in ESP-AT's SYSMFG docs. It avoids any string processing on the module's side for data that is technically text, but multi-line and easy to mishandle as a string.

Read (`operation=1`) answers with the value inline in the response:

```text
+SYSMFG:"mqtt_cert","mqtt_cert",8,1262,<1262 raw bytes>
```

`TIREMO_ESP32_CredRead()` in [`tiremo_esp32.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c) parses this the same way tutorial 01's `MqttWaitMessage()` parses `+MQTTSUBRECV`: skip the two quoted fields it already knows (namespace, key), skip the type digit, parse the decimal length, then check whether the response buffer already holds that many bytes after it. Only then is the value complete — a real length-prefixed parse, correct for the whole multi-line PEM value. Erase (`operation=0`) is a plain `AT+SYSMFG=0,"ns","key"` matched on `"OK"`, with no data phase.

Reading an **erased** slot answers with a framed `\r\nERROR\r\n` instead. `CredRead()` treats that as a normal, expected outcome (logged at `WARN`, not `ERROR`), because proving the slot is now empty is exactly the last step of the test.

## Solution

### Generate the test certificate

From the repo root, go to this tutorial's `Sw/` folder and generate the TEST client pair (one line):

```sh
cd Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/Sw
openssl req -x509 -newkey rsa:2048 -nodes -days 3650 -keyout certs/client_key.pem -out certs/client_cert.pem -subj "/CN=Tiremo Cortex Tutorial 02 TEST client - not for real use"
```

On Windows, `openssl` comes with Git for Windows: run the command in Git Bash, and write the subject as `"//CN=..."` there (Git Bash otherwise turns the leading `/` into a Windows path). On macOS and Linux, `openssl` is usually already installed. If you forget this step, the build stops with `Missing test client credentials` and prints the same command.

### Build, flash, watch

```sh
tdf build
tdf flash
tdf monitor
```

(See [Getting_Started/01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution) for what `tdf` is. On macOS/Linux, `tdf.py` also works as an alias; on Windows, use `tdf`. Close any other terminal on the board's serial port before running `tdf monitor` — only one program can open it at a time.) No `wifi_secrets.h` is needed: `AT+SYSMFG` never touches the network.

The driver ([`tiremo_esp32.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c)) is [ESP32 Connectivity tutorial 01](../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/)'s minimal transport (`Init` / `SendCommand` / `IsPresent` / `DisableEcho`) with the three credential functions added on top.

**Confirmed on real hardware:** `tdf build` compiles and links without errors on macOS and Windows 11 (`.text` about 21 KB; the exact size varies slightly with the toolchain version, and the linker prints the same expected, harmless `_close` / `_lseek` / `_read` / `_write is not implemented` warnings explained in [01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution)). `tdf flash` completed a full program-and-verify cycle over SWD on a real, connected Tiremo Cortex board (`** Verified OK **`). The test runs once at boot:

```text
[INFO] mTLS_Credentials_In_Module_NVS: boot
[INFO] ESP32: power cycling module...
[INFO] ESP32: power-on boot delay elapsed
[INFO] TIREMO_ESP32_Init: UART2 ready
[INFO] ESP32: command echo disabled (ATE0)
[INFO] ROOT_CA: writing 1266 bytes...
[INFO] ROOT_CA: read-back matches byte-for-byte (1266 bytes)
[INFO] ROOT_CA: erased
[WARN] ESP32: SYSMFG read "mqtt_ca"/"mqtt_ca": ERROR (empty slot?)
[INFO] ROOT_CA: read after erase correctly failed (slot empty)
[INFO] CLIENT_CERT: writing 1262 bytes...
[INFO] CLIENT_CERT: read-back matches byte-for-byte (1262 bytes)
[INFO] CLIENT_CERT: erased
[WARN] ESP32: SYSMFG read "mqtt_cert"/"mqtt_cert": ERROR (empty slot?)
[INFO] CLIENT_CERT: read after erase correctly failed (slot empty)
[INFO] CLIENT_KEY: writing 1732 bytes...
[INFO] CLIENT_KEY: read-back matches byte-for-byte (1732 bytes)
[INFO] CLIENT_KEY: erased
[WARN] ESP32: SYSMFG read "mqtt_key"/"mqtt_key": ERROR (empty slot?)
[INFO] CLIENT_KEY: read after erase correctly failed (slot empty)
[INFO] mTLS_Credentials_In_Module_NVS: ALL THREE SLOTS PASSED
[INFO] mTLS_Credentials_In_Module_NVS: idle (test already ran once at boot)
```

For each slot: write, a byte-for-byte read-back match, erase, and a read after erase that correctly fails. The `[WARN] ... ERROR (empty slot?)` lines are not failures — they are the expected result of reading a slot that was just erased, and the namespace in each line shows which slot was checked. After the test, an `idle` line repeats every 10 s; press RESET to run it again.

The byte counts are the sizes of your three `.pem` files, so yours will differ slightly: the client pair is your own, and on Windows a checkout with CRLF line endings makes `root_ca.pem` a few bytes longer than on macOS or Linux.

## Walkthrough

### 1. A minimal transport, on purpose

[`Sw/Src/main.c`](./Sw/Src/main.c) and the driver's `Init` / `SendCommand` / `IsPresent` / `DisableEcho` core are the same as ESP32 Connectivity tutorial 01 — no WiFi join, no MQTT. `AT+SYSMFG` is local storage on the module; it doesn't need a network.

### 2. The credential slot API

[`Sw/Drivers/esp32/Include/tiremo_esp32.h`](./Sw/Drivers/esp32/Include/tiremo_esp32.h):

```c
bool TIREMO_ESP32_CredWrite(TIREMO_ESP32_CredSlot_t slot, const uint8_t *pData, uint32_t length);
bool TIREMO_ESP32_CredRead(TIREMO_ESP32_CredSlot_t slot, uint8_t *pOut, uint32_t outSize, uint32_t *pLengthOut);
bool TIREMO_ESP32_CredErase(TIREMO_ESP32_CredSlot_t slot);
```

A private `prvESP32CredSlotInfo()` maps each `TIREMO_ESP32_CredSlot_t` to its `{namespace, key}` pair — see the slot table above for why `ROOT_CA` maps to the ESP-AT name `mqtt_ca`.

### 3. The test — write, verify, erase, verify

`TestSlot()` in [`Sw/Src/prv_user_code.c`](./Sw/Src/prv_user_code.c) runs the full cycle for each of the three embedded `.pem` files, comparing the read-back bytes with `memcmp` instead of only checking that the call succeeded:

```c
TIREMO_ESP32_CredWrite(slot, pData, length);
TIREMO_ESP32_CredRead(slot, s_readBuf, sizeof(s_readBuf), &readLen);
/* readLen == length && memcmp(s_readBuf, pData, length) == 0 */
TIREMO_ESP32_CredErase(slot);
/* CredRead() now expected to return false */
```

## Done When

The log shows, for all three slots, `read-back matches byte-for-byte`, `erased`, and `read after erase correctly failed (slot empty)`, and ends with `ALL THREE SLOTS PASSED`.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
