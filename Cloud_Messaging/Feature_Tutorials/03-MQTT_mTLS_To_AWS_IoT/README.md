<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>MQTT mTLS to AWS IoT Core</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# MQTT mTLS to AWS IoT Core

***Board: Tiremo Cortex (ESP32) · Section: Cloud Messaging (MQTT & TLS) / Feature Tutorials · Difficulty: Hard***

## Introduction

With credentials writable (Tutorial 02) and MQTT working in the clear
(Tutorial 01), combine them: load a certificate you created by hand for
a real AWS IoT Thing onto the module, and get a genuine mutual-TLS MQTT
session up against a real AWS IoT Core endpoint. This tutorial stops at
a successful `AT+MQTTCONN` — no provisioning, no publish/subscribe yet
(that's later `AWS_IoT_Core` curriculum) — because getting the handshake
itself right is the genuinely hard part of this whole section.

## You Will Learn

- TLS scheme 5 (verify server cert, present client cert) via `MQTTUSERCFG`
- Why `AT+MQTTSNI` matters for a custom domain endpoint sitting behind SNI routing
- Why scheme 5's `AT+MQTTCONN` requires the module to have synced real time over SNTP first
- Manually creating one AWS IoT Thing + certificate for bench testing, before AWS IoT Core automates it
- Keeping a real device private key out of a public git repo without breaking the build for other tutorials

## Prerequisites

- [02-mTLS_Credentials_In_Module_NVS](../02-mTLS_Credentials_In_Module_NVS/) — the credential-slot driver this tutorial loads real credentials through
- An AWS IoT Core endpoint and one manually-created Thing certificate — see **Understanding the `certs/` Folder** below for exactly what to fetch and where it goes
- `wifi_secrets.h` in this `Sw/` (gitignored), same pattern as every other Cloud Messaging / ESP32 Connectivity tutorial
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)

## Understanding the Connect Sequence

`AT+MQTTUSERCFG` (scheme 5) configures the MQTT identity — client ID,
and `cert_key_ID`/`CA_ID` both `0`, telling the module's TLS engine to
use whatever [Tutorial 02](../02-mTLS_Credentials_In_Module_NVS/)'s
`TIREMO_ESP32_CredWrite()` last stored in the `mqtt_cert`/`mqtt_key`/
`mqtt_ca` slots. `AT+MQTTSNI` sets the TLS SNI hostname — required for
a custom domain endpoint that may sit behind shared TLS-terminating
infrastructure; without it, the TLS handshake can complete against
whatever the SNI-less default routes to, while the connection never
actually reaches the intended backend. Order matters: `MQTTUSERCFG` →
`MQTTSNI` → `MQTTCONN`, with no `AT+MQTTCONNCFG` in between (that
command is for keepalive/LWT, unrelated to identity/TLS, and calling it
here isn't needed).

```c
TIREMO_ESP32_MqttUserConfigMtls(MQTT_CLIENT_ID);   /* AT+MQTTUSERCFG=0,5,"<id>","","",0,0,"" */
TIREMO_ESP32_MqttSni(MQTT_ENDPOINT);               /* AT+MQTTSNI=0,"<host>"                 */
TIREMO_ESP32_MqttConnect(MQTT_ENDPOINT, 8883, ...);/* AT+MQTTCONN=0,"<host>",8883,0          */
```

## Understanding the `certs/` Folder (Real vs. Gitignored)

Three files, three very different sensitivity levels:

| File | Committed? | What it is |
|---|---|---|
| [`certs/root_ca.pem`](./Sw/certs/root_ca.pem) | Yes | **Amazon Root CA 1** — genuinely public, identical for every AWS account, fetched straight from `https://www.amazontrust.com/repository/AmazonRootCA1.pem`. Verifies AWS IoT's server certificate. |
| `certs/device_cert.pem.example` / `device_key.pem.example` | Yes | Placeholders showing the format. |
| `certs/device_cert.pem` | **No** (gitignored) | Your AWS IoT Thing's own certificate, downloaded from the AWS console when you created it. |
| `certs/device_key.pem` | **No** (gitignored) | Your AWS IoT Thing's own private key, downloaded at the same time — AWS only shows it once. Whoever has this file can connect to your AWS account as this device. |

This mirrors how `wifi_secrets.h` is already handled everywhere else in
this repo: a gitignored real file, a committed `.example` template.
Copy the two `.example` files to `device_cert.pem` and `device_key.pem`
in this same folder and fill in your real Thing's certificate and key
before building — see [`certs/README.md`](./Sw/certs/README.md).

Root CA aside: this custom domain's server certificate happens to chain
`iot.tiremo.ai` → `Amazon RSA 2048 M04` → `Amazon Root CA 1`, checked
directly with `openssl s_client -showcerts` before assuming it — the
same root AWS IoT's standard endpoints use, not a separate one just
because it's a custom domain. That won't necessarily be true for every
custom domain (a domain owner can configure ACM with a different
certificate entirely); check yours the same way rather than assuming.

## Understanding SNTP: Why Scheme 5 Needs a Synced Clock First

Scheme 5 verifies the server's certificate, which means checking its
validity dates — and the module has no RTC at boot. Without a real
clock, ESP-AT's TLS engine can't evaluate "is this certificate
currently valid," so `AT+MQTTCONN` requires the module to have
obtained the current time via SNTP first. Espressif's own
`AT+MQTTUSERCFG` documentation states this directly, as a note rather
than a parameter description — easy to miss on a first read:

> If `<scheme>` is configured to 3, 5, 8, or 10, in order to check the
> server certificate validity period, please make sure {IDF_TARGET_NAME}
> has obtained the current time before sending the `AT+MQTTCONN`
> command.

This tutorial's driver enables SNTP and waits for a real time sample
before running the MQTT connect sequence, reusing
[ESP32 Connectivity Tutorial 04](../../../ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/)'s
already-verified `EnableSntp()`/`WaitSntpEpoch()` functions instead of
writing a second SNTP parser. On real hardware, the module reports the
Unix epoch (1 Jan 1970) until SNTP actually completes, then the real
current time once it has:

```text
[DEBUG] ESP32 TX (17 bytes): {"data":"AT+CIPSNTPTIME?\r\n"}
[DEBUG] ESP32 RX matched "OK" after 41 bytes:
{"data":"+CIPSNTPTIME:Thu Jan  1 00:00:04 1970\r\nOK"}      <- not yet synced
[DEBUG] ESP32 TX (17 bytes): {"data":"AT+CIPSNTPTIME?\r\n"}
[DEBUG] ESP32 RX matched "OK" after 41 bytes:
{"data":"+CIPSNTPTIME:Wed Aug 26 21:12:17 2026\r\nOK"}       <- synced
[INFO] ESP32: SNTP synced (epoch 1787778737)
```

`WaitSntpEpoch()` polls exactly this way until the reported year is
past a sanity floor (2020), so it never mistakes the unsynced default
for a real time.

## Verifying Your Credentials Independently

Before building and flashing, it's worth confirming your certificate,
private key, and root CA are valid and correctly paired *without* the
firmware in the loop — that way, if something doesn't connect later,
you already know whether to look at the credentials or the module.

```sh
openssl s_client -connect <your-endpoint>:8883 -servername <your-endpoint> \
    -cert certs/device_cert.pem -key certs/device_key.pem -CAfile certs/root_ca.pem
```

A successful mutual-TLS handshake reports `Verify return code: 0 (ok)`.
You can also confirm a full MQTT `CONNECT` (not just the raw TLS layer)
independently with the [`mqttx`](https://mqttx.app/) CLI:

```sh
mqttx sub -h <your-endpoint> -p 8883 -l mqtts -t 'some/topic' \
    --cert certs/device_cert.pem --key certs/device_key.pem --ca certs/root_ca.pem \
    -i <your-client-id>
```

A `✔ Connected` here confirms AWS IoT accepts this certificate and
client ID for a real session — even if the subsequent subscribe is
rejected for policy reasons unrelated to the connection itself.

## Solution

[`Sw/`](./Sw/) has a real, buildable implementation:

```sh
cd Sw
cp wifi_secrets.h.example wifi_secrets.h                # once; then edit
cp certs/device_cert.pem.example certs/device_cert.pem  # once; then paste your real cert
cp certs/device_key.pem.example certs/device_key.pem    # once; then paste your real key
# also edit MQTT_ENDPOINT / MQTT_CLIENT_ID in Src/prv_user_code.c to your own
tdf.py build
tdf.py flash
tdf.py monitor
```

Base: [Cloud Messaging Tutorial 01](../01-MQTT_To_Public_Broker/)'s
WiFi+MQTT driver (it already carries everything needed except
mTLS-specific pieces), with [Tutorial 02](../02-mTLS_Credentials_In_Module_NVS/)'s
credential-slot functions and [ESP32 Connectivity Tutorial 04](../../../ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/)'s
SNTP functions folded in on top, plus two new functions this tutorial
adds: `TIREMO_ESP32_MqttUserConfigMtls()` (scheme 5) and
`TIREMO_ESP32_MqttSni()`.

This build does **not** subscribe or publish — it proves the mTLS
handshake itself, deliberately, since that's the genuinely hard part of
this whole section. Pub/sub against a real AWS topic is later
`AWS_IoT_Core` curriculum; this tutorial's own original "publish to a
test topic" step was intentionally not attempted this round.

**Genuinely verified, for real, against a real AWS IoT Core endpoint:**
`tdf.py build` compiles and links clean (`.text` 21648 bytes). `tdf.py
flash` completed a full program-and-verify cycle over SWD (`**
Verified OK **`). `tdf.py monitor` then showed, straight from the real
hardware:

```text
[INFO] Credentials: all three slots loaded
[INFO] ESP32: joining SSID "..."
[INFO] ESP32: WIFI GOT IP
[INFO] ESP32: SNTP synced (epoch 1787779064)
[INFO] MQTT: client id "..." configured (scheme 5, mutual TLS)
[INFO] MQTT: SNI set to "iot.tiremo.ai"
[INFO] MQTT: connecting to iot.tiremo.ai:8883
[INFO] MQTT: connected (mutual TLS) -- MQTTUSERCFG/MQTTSNI/MQTTCONN all OK
```

The raw AT trace behind that last line is the module's own confirmation,
not just this driver's interpretation of a bare `OK`:

```text
{"data":"+MQTTCONNECTED:0,5,\"iot.tiremo.ai\",\"8883\",\"\",0\r\n\r\nOK"}
```

`+MQTTCONNECTED` with scheme `5` and the real endpoint echoed back —
a genuine mutual-TLS session against AWS IoT Core, not a local loopback
or a lucky `OK` match.

## Walkthrough

### 1. Load credentials, then join WiFi

[`Sw/Src/prv_user_code.c`](./Sw/Src/prv_user_code.c) writes all three
slots — root CA, device cert, device key, embedded from `certs/*.pem`
at build time by the same [`cmake/embed_pem.cmake`](../../../cmake/embed_pem.cmake)
mechanism Tutorial 02 introduced — *before* touching WiFi at all, since
`AT+SYSMFG` is pure local NVS storage and doesn't need a network:

```c
LoadCred(TIREMO_ESP32_CREDSLOT_ROOT_CA, "ROOT_CA", g_rootCaPem, g_rootCaPem_len);
LoadCred(TIREMO_ESP32_CREDSLOT_CLIENT_CERT, "CLIENT_CERT", g_deviceCertPem, g_deviceCertPem_len);
LoadCred(TIREMO_ESP32_CREDSLOT_CLIENT_KEY, "CLIENT_KEY", g_deviceKeyPem, g_deviceKeyPem_len);
```

### 2. WiFi join, then SNTP — before any MQTT command

```c
TIREMO_ESP32_JoinAP(TIREMO_WIFI_SSID, TIREMO_WIFI_PASSWORD, JOIN_TIMEOUT_MS);
TIREMO_ESP32_EnableSntp();
TIREMO_ESP32_WaitSntpEpoch(&epoch, SNTP_TIMEOUT_MS);
```

See **Understanding SNTP** above for why this step exists at all — skip
it and `MQTTCONN` fails every time under scheme 5, with no clearer
signal than a generic `ERROR`.

### 3. The connect sequence itself

```c
TIREMO_ESP32_MqttUserConfigMtls(MQTT_CLIENT_ID);
TIREMO_ESP32_MqttSni(MQTT_ENDPOINT);
TIREMO_ESP32_MqttConnect(MQTT_ENDPOINT, MQTT_PORT, MQTT_CONNECT_TIMEOUT_MS);
```

`TIREMO_ESP32_MqttUserConfigMtls()` in
[`tiremo_esp32.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c) is a new,
separate function from Tutorial 01's plain-TCP `MqttUserConfig()` —
scheme 5, empty username/password (auth is by certificate, not
credentials), `cert_key_ID`/`CA_ID` both `0`. `TIREMO_ESP32_MqttSni()`
is a direct `AT+MQTTSNI=0,"<host>"` sent to `"OK"`, no response
parsing needed beyond that.

### 4. Build, flash, and watch

```sh
tdf.py build
tdf.py flash
tdf.py monitor
```

See **Solution** above for what's actually confirmed on real hardware.

## Done When

A successful `AT+MQTTCONN` mutual-TLS session against a real AWS IoT
Core endpoint, confirmed by the module's own `+MQTTCONNECTED` response
— genuinely verified above. This tutorial's original "publish to a
test topic, confirm receipt in the AWS console" step was deliberately
not attempted this round (see **Solution**); that's covered once this
curriculum reaches real pub/sub against AWS IoT Core.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
