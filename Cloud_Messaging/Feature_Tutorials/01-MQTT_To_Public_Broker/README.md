<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/Cloud_Messaging/Feature_Tutorials/01-MQTT_To_Public_Broker/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>MQTT to a Public Broker</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# MQTT to a Public Broker

***Board: Tiremo Cortex (ABOV A34G43A, Cortex-M4) Â· Section: Cloud Messaging (MQTT & TLS) / Feature Tutorials Â· Difficulty: Medium***

## Introduction

Before touching certificates, prove the ESP32's built-in MQTT client works at all: connect to a public test broker in the clear, publish a message, and subscribe to hear it echoed back. [Tutorial 03](../03-MQTT_mTLS_To_AWS_IoT/) then connects to Tiremo Cloud ([`iot.tiremo.ai`](https://iot.tiremo.ai)) with mutual TLS.

## You Will Learn

- `AT+MQTTUSERCFG` (scheme 1, no TLS) / `AT+MQTTCONN` / `AT+MQTTSUB` / `AT+MQTTPUB`
- Why ESP-AT's MQTT `scheme` starts at 1, not 0
- The difference between `MqttPublish` (string payload) and `MqttPublishRaw` (binary-safe)
- Why a client on a public broker needs its own client ID and topic

## Prerequisites

- [ESP32 Connectivity](../../../ESP32_Connectivity/) â€” WiFi join working
- A 2.4 GHz WiFi network with internet access, and this `Sw/`'s own `wifi_secrets.h` â€” see [Credentials](#credentials-do-not-commit-a-password)
- The setup scripts from [Getting Started](../../../Getting_Started/) already run, so `tdf` is available
- To watch the log: `tdf monitor` or any serial terminal (Tera Term, PuTTY, screen, minicom, â€¦) at 115200 8N1 on the board's USB serial port
- A Tiremo Cortex board (ABOV A34G43A MCU â€” see [Hardware Overview](../../../Hardware.md)) connected to your PC with a USB cable
- Optional: an MQTT client on your PC (for example [MQTTX](https://mqttx.app/)) to watch the board's messages arrive at the broker

## Understanding `scheme`: there is no `scheme 0`

`AT+MQTTUSERCFG`'s second parameter selects the transport. It looks like an enum that should start at 0, but in [Espressif's ESP-AT MQTT command reference](https://github.com/espressif/esp-at/blob/master/docs/en/AT_Command_Set/MQTT_AT_Commands.rst) it is **1-indexed**: `1` is plain TCP, `2`â€“`5` are TLS variants (`5` is mutual TLS, used in tutorial 03), and `6`â€“`10` are WebSocket variants. There is no `scheme 0`: `AT+MQTTUSERCFG=0,0,...` is rejected outright with a bare `\r\nERROR\r\n` (9 bytes) â€” the module refuses the command itself, before any network traffic. That is why the driver sends `1`.

## Understanding `MQTTPUBRAW`'s completion marker

`AT+MQTTPUBRAW` works in two steps: the module first accepts the command with `OK` and asks for the payload with a `>` prompt, and only after the raw bytes have been sent does it report the result as `+MQTTPUB:OK` or `+MQTTPUB:FAIL`. 

Captured on the board with `TIREMO_DEBUG_LEVEL=4`:

```text
[DEBUG] ESP32 TX (49 bytes):
{"data":"AT+MQTTPUBRAW=0,\"tiremo-cortex/mqtt-demo\",4,0,0\r\n"}
[DEBUG] ESP32 RX matched ">" after 9 bytes:
{"data":"\r\nOK\r\n\r\n>"}
[DEBUG] ESP32 RX matched "+MQTTPUB:OK" after 11 bytes:
{"data":"+MQTTPUB:OK"}
```

Matching a bare `"OK"` would happen to work, because `"+MQTTPUB:OK"` contains `"OK"` â€” but it would also return on the earlier `OK` and could never tell success from `+MQTTPUB:FAIL`. The driver therefore matches `+MQTTPUB:OK` explicitly.

## Understanding a shared public broker

`broker.emqx.io` is open to everyone on the internet, so two rules of MQTT matter here:

- **Client IDs must be unique.** When a second client connects with the same client ID, the broker drops the first one. If someone else runs this tutorial with the default `tiremo-cortex-01` at the same time, your board and theirs keep kicking each other off.
- **Topics are shared.** Everyone subscribed to `tiremo-cortex/mqtt-demo` sees every message published to it â€” yours, and anyone else's.

So before flashing, give `MQTT_CLIENT_ID` and `MQTT_TOPIC` in [`Sw/Src/prv_user_code.c`](./Sw/Src/prv_user_code.c) a unique suffix (your name, a random number, â€¦), for example `"tiremo-cortex-myname-4711"` and `"tiremo-cortex/myname-4711/demo"`. And never send anything private through a public test broker.

## Credentials (do not commit a password)

Same as [ESP32 Connectivity tutorial 02](../../../ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/#credentials-do-not-commit-a-password): this `Sw/` needs its own `wifi_secrets.h`. From the repo root:

```sh
cd Cloud_Messaging/Feature_Tutorials/01-MQTT_To_Public_Broker/Sw
cp wifi_secrets.h.example wifi_secrets.h
```

(In cmd.exe, use `copy` instead of `cp`.) Then fill in your network's SSID and password, or copy a `wifi_secrets.h` you already made for ESP32 Connectivity. The file is listed in [`Sw/.gitignore`](./Sw/.gitignore), so it never ends up in a commit. As in ESP32 Connectivity, the DEBUG-level TX dump shows `AT+CWJAP=<credentials hidden>` instead of the join command.

## Solution

From the repo root (after creating `wifi_secrets.h` as above):

```sh
cd Cloud_Messaging/Feature_Tutorials/01-MQTT_To_Public_Broker/Sw
tdf build
tdf flash
tdf monitor
```

(See [Getting_Started/01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution) for what `tdf` is. On macOS/Linux, `tdf.py` also works as an alias; on Windows, use `tdf`. Close any other terminal on the board's serial port before running `tdf monitor` â€” only one program can open it at a time.)

The ESP32 driver ([`tiremo_esp32.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c)) is ESP32 Connectivity tutorial 02's WiFi-join subset with MQTT functions added to the same file â€” one growing driver, not a second UART stack.

**Confirmed on real hardware:** `tdf build` compiles and links without errors on macOS and Windows 11 (`.text` about 17 KB; the exact size varies slightly with the toolchain version, and the linker prints the same expected, harmless `_close` / `_lseek` / `_read` / `_write is not implemented` warnings explained in [01-Hello_Blink's Solution](../../../Getting_Started/01-Hello_Blink/#solution)). `tdf flash` completed a full program-and-verify cycle over SWD on a real, connected Tiremo Cortex board (`** Verified OK **`). The log, with the network's name replaced:

```text
[INFO] MQTT_To_Public_Broker: boot
[INFO] ESP32: power cycling module...
[INFO] ESP32: power-on boot delay elapsed
[INFO] TIREMO_ESP32_Init: UART2 ready
[INFO] ESP32: command echo disabled (ATE0)
[INFO] ESP32: joining SSID "YourSSID"
[INFO] ESP32: WIFI GOT IP
[INFO] MQTT: client id "tiremo-cortex-01" configured (scheme 1, no TLS)
[INFO] MQTT: connecting to broker.emqx.io:1883
[INFO] MQTT: connected
[INFO] MQTT: subscribed to "tiremo-cortex/mqtt-demo"
[INFO] MQTT: publishing (string) "hello from Tiremo Cortex #0"
[INFO] MQTT: echo received on "tiremo-cortex/mqtt-demo"
[INFO] MQTT: echo payload: 27 bytes "hello from Tiremo Cortex #0"
[INFO] MQTT: publishing (raw) counter=1 as 4 bytes
[INFO] MQTT: echo received on "tiremo-cortex/mqtt-demo"
[INFO] MQTT: echo payload: 4 bytes "...."
```

This repeats every 5 seconds, alternating string (`MqttPublish`) and binary-safe (`MqttPublishRaw`) publishes. The raw payload is logged as `....` because four counter bytes aren't printable ASCII â€” expected, not an error.

### Watch the messages from your PC

The echo proves a round trip, but not that the messages really left the board and went through the internet. Subscribe to the same topic with a separate MQTT client on your PC â€” for example with the [MQTTX](https://mqttx.app/) command-line tool (use your own topic if you changed it):

```sh
mqttx sub -h broker.emqx.io -t tiremo-cortex/mqtt-demo
```

While the board runs, the board's string messages and its raw counters (as the bytes `00 00 00 0d`, `00 00 00 0f`, â€¦) arrive at your PC through `broker.emqx.io` every 5 seconds. This was checked on the test board with a separate subscriber on a PC.

## Walkthrough

### 1. MQTT identity â€” `TIREMO_ESP32_MqttUserConfig()`

```c
(void)snprintf(command, sizeof(command),
               "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",
               pClientId, pUsername, pPassword);
```

LinkID `0` (this driver only ever uses one MQTT link), scheme `1` (plain TCP â€” see above), empty username/password (this public broker needs none), no certificate (slots `0`, path `""` â€” those matter from tutorial 02 on).

### 2. Connect and subscribe

```c
TIREMO_ESP32_MqttConnect("broker.emqx.io", 1883, 10000U);
TIREMO_ESP32_MqttSubscribe("tiremo-cortex/mqtt-demo", 0U);
```

Both are ordinary `AT+MQTTCONN` / `AT+MQTTSUB`, matched on `"OK"`.

### 3. Two ways to publish

[`Sw/Src/prv_user_code.c`](./Sw/Src/prv_user_code.c) alternates between them on the same topic:

```c
TIREMO_ESP32_MqttPublish(topic, "hello from Tiremo Cortex #0", 0, false);
/* vs. */
uint8_t counter[4] = { 0, 0, 0, 1 };
TIREMO_ESP32_MqttPublishRaw(topic, counter, 4, 0, false);
```

`MqttPublish` sends the payload inside AT-command quotes â€” simple, but the payload can never contain `"`, `\r`, or `\n`. `MqttPublishRaw` sends it as raw, length-prefixed bytes instead: issue `AT+MQTTPUBRAW=...,<length>,...`, wait for the module's `>` prompt, then transmit exactly `<length>` raw bytes â€” safe for anything, including quotes in a JSON payload or a zero byte in a packed binary struct.

### 4. Receiving the echo â€” a length-prefixed parse

ESP-AT reports an inbound message as an unsolicited result code (URC):

```text
+MQTTSUBRECV:0,"tiremo-cortex/mqtt-demo",4,<4 raw bytes>
```

`TIREMO_ESP32_MqttWaitMessage()` in [`tiremo_esp32.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c) collects bytes into a capture buffer like `SendCommandCapture` does, but instead of matching a fixed substring it re-parses the buffer after every byte: find the `+MQTTSUBRECV:` marker, parse the quoted topic, parse the decimal length, then check whether the buffer already holds that many bytes *after* the length field. Only then is the message complete â€” a real length-prefixed parse, not a text heuristic, which is what makes it correct for the binary payload in step 3 too. The limit: the whole URC (header + payload) has to fit in the driver's 512-byte capture buffer.

## Done When

- `tdf monitor` shows `MQTT: connected`, then every 5 s a `publishing` line followed by `echo received` and the same payload (string and raw, alternating).
- A separate MQTT client on your PC, subscribed to the same topic on `broker.emqx.io`, receives the same messages.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials â€” Empa Electronics</sub></p>
