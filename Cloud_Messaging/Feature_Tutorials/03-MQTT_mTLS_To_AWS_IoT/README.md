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
      <h1>MQTT mTLS to Tiremo Cloud</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# MQTT mTLS to Tiremo Cloud

***Board: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Section: Cloud Messaging (MQTT & TLS) / Feature Tutorials · Difficulty: Hard***

## Introduction

With credentials writable ([tutorial 02](../02-mTLS_Credentials_In_Module_NVS/)) and MQTT working in the clear ([tutorial 01](../01-MQTT_To_Public_Broker/)), combine them: load a real certificate from the Tiremo UI onto the module and open a mutual-TLS MQTT session with Tiremo Cloud (`iot.tiremo.ai`). This tutorial stops at a successful `AT+MQTTCONN` — no publish or subscribe yet (that is the [Tiremo Cloud](../../../Tiremo_Cloud/) section) — because getting the handshake itself right is the hard part.

## You Will Learn

- TLS scheme 5 (verify the server's certificate, present the client's) via `AT+MQTTUSERCFG`
- Why `AT+MQTTSNI` matters for an endpoint that sits behind SNI routing
- Why scheme 5's `AT+MQTTCONN` needs the module to have synced the real time over SNTP first
- Using one certificate downloaded from the Tiremo UI for bench testing, before Tiremo Cloud fleet provisioning gives every device its own
- Keeping a real device private key out of a git repository

## Prerequisites

- [02-mTLS_Credentials_In_Module_NVS](../02-mTLS_Credentials_In_Module_NVS/) — the credential-slot driver this tutorial loads the real credentials through
- Credentials from the **Tiremo UI** for your product: the **Identifier**, and the **Provisioning Certificates** (certificate and private key) — see [Getting credentials from the Tiremo UI](../../../Tiremo_Cloud/Feature_Tutorials/03-Fleet_Provisioning_By_Claim/#getting-credentials-from-the-tiremo-ui) for where they are on the product page, and [The `certs/` folder](#understanding-the-certs-folder) below. The **Secret Key** shown next to them is not used in this tutorial; it is needed for fleet provisioning in the Tiremo Cloud section.
- A 2.4 GHz WiFi network with internet access, and this `Sw/`'s own `wifi_secrets.h` — same as [Cloud Messaging tutorial 01](../01-MQTT_To_Public_Broker/#credentials-do-not-commit-a-password)
- The setup scripts from [Getting Started](../../../Getting_Started/) already run, so `tdf` is available
- To watch the log: `tdf monitor` or any serial terminal (Tera Term, PuTTY, screen, minicom, …) at 115200 8N1 on the board's USB serial port
- A Tiremo Cortex board (ABOV A34G43A MCU — see [Hardware Overview](../../../Hardware.md)) connected to your PC with a USB cable

## Platform documentation

- Endpoint and first mTLS session: [first connection](https://docs.tiremo.ai/device-provisioning/third-phase)
- Why `AT+MQTTSNI` is required on `iot.tiremo.ai`: [TLS SNI Requirement](https://docs.tiremo.ai/device-provisioning/tls-sni)
- Claim certificates and fleet provisioning (the Tiremo Cloud section): [device provisioning](https://docs.tiremo.ai/device-provisioning/getting-started)

## Understanding the connect sequence

`AT+MQTTUSERCFG` with scheme 5 configures the MQTT identity — the client ID — with `cert_key_ID` and `CA_ID` both `0`, which tells the module's TLS engine to use whatever [tutorial 02](../02-mTLS_Credentials_In_Module_NVS/)'s `TIREMO_ESP32_CredWrite()` last stored in the `mqtt_cert` / `mqtt_key` / `mqtt_ca` slots. `AT+MQTTSNI` sets the TLS SNI hostname. It is required for an endpoint that sits behind shared, hostname-routed TLS infrastructure ([TLS SNI Requirement](https://docs.tiremo.ai/device-provisioning/tls-sni)): without it, the TLS handshake can complete against whatever the SNI-less default routes to, while the connection never reaches the intended backend. The order is `MQTTUSERCFG` → `MQTTSNI` → `MQTTCONN`. (`AT+MQTTCONNCFG`, for keepalive and last will, is not needed here.)

```c
TIREMO_ESP32_MqttUserConfigMtls(MQTT_CLIENT_ID);   /* AT+MQTTUSERCFG=0,5,"<id>","","",0,0,"" */
TIREMO_ESP32_MqttSni(MQTT_ENDPOINT);               /* AT+MQTTSNI=0,"<host>"                 */
TIREMO_ESP32_MqttConnect(MQTT_ENDPOINT, 8883, ...);/* AT+MQTTCONN=0,"<host>",8883,0          */
```

## Understanding the `certs/` folder

Three files, three very different sensitivity levels:

| File | Committed? | What it is |
|---|---|---|
| [`certs/root_ca.pem`](./Sw/certs/root_ca.pem) | Yes | **Amazon Root CA 1** — public, the same for every Tiremo Cloud device, from `https://www.amazontrust.com/repository/AmazonRootCA1.pem`. Verifies Tiremo Cloud's server certificate. |
| `certs/device_cert.pem.example` / `device_key.pem.example` | Yes | Placeholders showing the format. |
| `certs/device_cert.pem` | **No** (gitignored) | The certificate from **Provisioning Certificates** on your product page in the Tiremo UI. |
| `certs/device_key.pem` | **No** (gitignored) | Its private key, downloaded at the same time. Keep it like a password: **whoever has this file can connect to Tiremo Cloud with your product's credentials.** |

The download gives you two files named after your product's identifier, for example `myproduct.certificate.pem` and `myproduct.private.key`. Put them into `Sw/certs/` and **rename** them — the build only looks for these two names:

| Downloaded file | Rename to |
|---|---|
| `<identifier>.certificate.pem` | `device_cert.pem` |
| `<identifier>.private.key` | `device_key.pem` |

If either file is missing, the build stops with `Missing device credentials (certs/device_cert.pem, certs/device_key.pem)` and shows the same renaming. Both names are gitignored, and the repository's `.gitignore` also ignores any `*.key` file in a `certs/` folder, so even an un-renamed `<identifier>.private.key` doesn't slip into a commit. Still, check `git status` before you commit: neither file may ever appear in it. See [`certs/README.md`](./Sw/certs/README.md).

The server certificate of `iot.tiremo.ai` chains `iot.tiremo.ai` → `Amazon RSA 2048 M04` → `Amazon Root CA 1`, which is why `root_ca.pem` is Amazon Root CA 1. You can check a chain yourself with `openssl s_client -connect iot.tiremo.ai:8883 -servername iot.tiremo.ai -showcerts`.

## Understanding the client ID

`MQTT_CLIENT_ID` in [`Sw/Src/prv_user_code.c`](./Sw/Src/prv_user_code.c) ships as the placeholder `"YOUR_DEVICE_ID"`. Replace it with the **Identifier** from your product page in the Tiremo UI:

```c
#define MQTT_CLIENT_ID            "YOUR_DEVICE_ID"
```

The connection itself is authenticated by the certificate: on the test board, `AT+MQTTCONN` was accepted even with the placeholder still in place. Use your Identifier anyway, rather than a placeholder that every customer would share — two clients connecting with the same client ID drop each other (see [tutorial 01](../01-MQTT_To_Public_Broker/#understanding-a-shared-public-broker)). In the [Tiremo Cloud](../../../Tiremo_Cloud/) section, each device then gets its own certificate and identity through fleet provisioning.

## Understanding SNTP: why scheme 5 needs a synced clock first

Scheme 5 verifies the server's certificate, which means checking its validity dates — and the module has no real-time clock at boot. Espressif's `AT+MQTTUSERCFG` documentation says so in a note that is easy to miss:

> If `<scheme>` is configured to 3, 5, 8, or 10, in order to check the server certificate validity period, please make sure {IDF_TARGET_NAME} has obtained the current time before sending the `AT+MQTTCONN` command.

So the firmware enables SNTP and waits for a real time sample before the MQTT connect sequence, reusing [ESP32 Connectivity tutorial 04](../../../ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/)'s `EnableSntp()` / `WaitSntpEpoch()`. Captured on the board with `TIREMO_DEBUG_LEVEL=4`: the module first counts up from 1970, then reports the real time:

```text
{"data":"+CIPSNTPTIME:Thu Jan  1 00:00:07 1970\r\nOK"}      <- not synced yet
{"data":"+CIPSNTPTIME:Tue Nov 24 09:43:28 2026\r\nOK"}      <- synced
```

`WaitSntpEpoch()` polls until the reported year is past a sanity floor (2020), so it never mistakes the unsynced counter for a real time.

## Verifying your credentials independently

Before building and flashing, you can check that your certificate, private key, and root CA are valid and belong together *without* the firmware in the loop — so if the board doesn't connect later, you already know whether to look at the credentials or at the module. From this tutorial's `Sw/` folder:

```sh
openssl s_client -connect iot.tiremo.ai:8883 -servername iot.tiremo.ai -cert certs/device_cert.pem -key certs/device_key.pem -CAfile certs/root_ca.pem
```

A successful mutual-TLS handshake reports `Verify return code: 0 (ok)`. To check a full MQTT `CONNECT` (not just the TLS layer), use the [MQTTX](https://mqttx.app/) command-line tool with the same three files:

```sh
mqttx conn -h iot.tiremo.ai -p 8883 -l mqtts --cert certs/device_cert.pem --key certs/device_key.pem --ca certs/root_ca.pem -i YOUR_DEVICE_ID
```

## Solution

From the repo root, after putting `wifi_secrets.h` and the two `certs/` files in place and setting `MQTT_CLIENT_ID`:

```sh
cd Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/Sw
tdf build
tdf flash
tdf monitor
```

(See [Getting_Started/01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution) for what `tdf` is. On macOS/Linux, `tdf.py` also works as an alias; on Windows, use `tdf`. Close any other terminal on the board's serial port before running `tdf monitor` — only one program can open it at a time.)

The driver is [Cloud Messaging tutorial 01](../01-MQTT_To_Public_Broker/)'s WiFi + MQTT driver, with [tutorial 02](../02-mTLS_Credentials_In_Module_NVS/)'s credential-slot functions and [ESP32 Connectivity tutorial 04](../../../ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/)'s SNTP functions added, plus two new functions: `TIREMO_ESP32_MqttUserConfigMtls()` (scheme 5) and `TIREMO_ESP32_MqttSni()`.

**Confirmed on real hardware:** `tdf build` compiles and links without errors on macOS and Windows 11 (`.text` about 21.5 KB; the exact size varies slightly with the toolchain version, and the linker prints the same expected, harmless `_close` / `_lseek` / `_read` / `_write is not implemented` warnings explained in [01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution)). `tdf flash` completed a full program-and-verify cycle over SWD on a real, connected Tiremo Cortex board (`** Verified OK **`), and the board opened a mutual-TLS session with `iot.tiremo.ai` using a real certificate from the Tiremo UI. The log (network name and identifier replaced):

```text
[INFO] MQTT_mTLS_To_AWS_IoT: boot
[INFO] ESP32: power cycling module...
[INFO] ESP32: power-on boot delay elapsed
[INFO] TIREMO_ESP32_Init: UART2 ready
[INFO] ESP32: command echo disabled (ATE0)
[INFO] ROOT_CA: writing 1206 bytes...
[INFO] ROOT_CA: stored
[INFO] CLIENT_CERT: writing 1220 bytes...
[INFO] CLIENT_CERT: stored
[INFO] CLIENT_KEY: writing 1679 bytes...
[INFO] CLIENT_KEY: stored
[INFO] Credentials: all three slots loaded
[INFO] ESP32: joining SSID "YourSSID"
[INFO] ESP32: WIFI GOT IP
[INFO] ESP32: SNTP synced (epoch 1795512665)
[INFO] MQTT: client id "YOUR_DEVICE_ID" configured (scheme 5, mutual TLS)
[INFO] MQTT: SNI set to "iot.tiremo.ai"
[INFO] MQTT: connecting to iot.tiremo.ai:8883
[INFO] MQTT: connected (mutual TLS) -- MQTTUSERCFG/MQTTSNI/MQTTCONN all OK
[INFO] MQTT_mTLS_To_AWS_IoT: idle (connect already succeeded once at boot, no subscribe/publish attempted)
```

The certificate sizes are those of your own files. The raw AT trace behind the `connected` line (level 4) is the module's own confirmation, with scheme `5` and the endpoint echoed back:

```text
{"data":"+MQTTCONNECTED:0,5,\"iot.tiremo.ai\",\"8883\",\"\",0\r\n\r\nOK"}
```

At level 4 the credential writes appear only as their command headers (`AT+SYSMFG=2,"mqtt_key","mqtt_key",8,1679`) — the certificate and key bytes are sent to the module directly and never reach the log — and the WiFi join shows as `AT+CWJAP=<credentials hidden>`.

## Walkthrough

### 1. Load credentials, then join WiFi

[`Sw/Src/prv_user_code.c`](./Sw/Src/prv_user_code.c) writes all three slots — root CA, device certificate, device key, embedded from `certs/*.pem` at build time by the same [`cmake/embed_pem.cmake`](../../../cmake/embed_pem.cmake) mechanism as tutorial 02 — *before* touching WiFi, since `AT+SYSMFG` is local storage and needs no network:

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

See the SNTP section above for why this step exists: skip it and `MQTTCONN` fails every time under scheme 5, with nothing clearer than a generic `ERROR`.

### 3. The connect sequence itself

```c
TIREMO_ESP32_MqttUserConfigMtls(MQTT_CLIENT_ID);
TIREMO_ESP32_MqttSni(MQTT_ENDPOINT);
TIREMO_ESP32_MqttConnect(MQTT_ENDPOINT, MQTT_PORT, MQTT_CONNECT_TIMEOUT_MS);
```

`TIREMO_ESP32_MqttUserConfigMtls()` in [`tiremo_esp32.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c) is separate from tutorial 01's plain-TCP `MqttUserConfig()`: scheme 5, empty username and password (authentication is by certificate), `cert_key_ID` / `CA_ID` both `0`. `TIREMO_ESP32_MqttSni()` sends `AT+MQTTSNI=0,"<host>"` and waits for `"OK"`.

## Done When

The log shows `Credentials: all three slots loaded`, `SNTP synced`, and `MQTT: connected (mutual TLS)` — a mutual-TLS session with Tiremo Cloud using your own certificate. And `git status` shows neither `device_cert.pem` nor `device_key.pem`.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
