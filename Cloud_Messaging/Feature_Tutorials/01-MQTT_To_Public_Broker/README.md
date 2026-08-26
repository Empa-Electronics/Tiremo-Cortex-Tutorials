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
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# MQTT to a Public Broker

***Board: Tiremo Cortex (ESP32) · Section: Cloud Messaging (MQTT & TLS) / Feature Tutorials · Difficulty: Medium***

## Introduction

Before touching certificates, prove the ESP32's built-in MQTT client
works at all — connect to a public test broker in the clear, publish a
message, and subscribe to hear it echoed back.

## You Will Learn

- `AT+MQTTUSERCFG` (scheme 1, no TLS) / `AT+MQTTCONN` / `AT+MQTTSUB` / `AT+MQTTPUB`
- The difference between `MqttPublish` (string payload) and `MqttPublishRaw` (binary-safe)
- A real bug found and fixed while building this: ESP-AT's MQTT `scheme` parameter is 1-indexed, not 0-indexed

## Prerequisites

- [ESP32 Connectivity](../../../ESP32_Connectivity/) — WiFi join working
- `wifi_secrets.h` in this `Sw/` (gitignored), same pattern as ESP32 Connectivity
- Toolchain and flashing set up per [Getting Started](../../../Getting_Started/)
- `tdf.py monitor` to watch the publish/echo cycle (see [Getting_Started/04-Button_Read_And_Debounce's Solution](../../../Getting_Started/04-Button_Read_And_Debounce/#solution) for setup)

## A Real Bug: `scheme 0` Doesn't Exist

The stub outline for this tutorial (written before any of this was
built) said "scheme 0, no TLS" — a reasonable guess, since 0-indexed
enums are the norm everywhere else in this codebase. The first real
build proved that guess wrong immediately:

```text
[DEBUG] ESP32 TX: {"data":"AT+MQTTUSERCFG=0,0,\"tiremo-cortex-01\",\"\",\"\",0,0,\"\"\r\n"}
[WARN] ESP32 RX: "OK" rejected with ERROR after 9 bytes
{"data":"\r\nERROR\r\n"}
```

Nine bytes is exactly `\r\nERROR\r\n` — the module rejected the command
outright, not a connection failure. Rebuilding with
`-D TIREMO_DEBUG_LEVEL=4` to see the raw AT traffic (above) confirmed
the exact bytes sent were syntactically well-formed, which narrowed it
to the `scheme` value itself. Checking
[Espressif's own ESP-AT MQTT command reference](https://github.com/espressif/esp-at/blob/master/docs/en/AT_Command_Set/MQTT_AT_Commands.rst)
settled it: `scheme` is **1-indexed** — `1` is plain TCP, `2`–`5` are
TLS variants (`5` is mutual TLS, confirming Tutorial 03's stub was
right about that value), `6`–`10` are WebSocket variants. There is no
`scheme 0`. Fixing the one digit in
[`Sw/Drivers/esp32/Src/tiremo_esp32.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c)'s
`TIREMO_ESP32_MqttUserConfig()` was the whole fix.

The same debug trace surfaced a second, smaller mismatch:
`AT+MQTTPUBRAW`'s actual completion marker is `+MQTTPUB:OK` /
`+MQTTPUB:FAIL`, not a bare `OK` (that first `OK` in the trace below is
just the command being accepted, before the module asks for the raw
bytes with `>`):

```text
[DEBUG] ESP32 RX matched ">" after 9 bytes: {"data":"\r\nOK\r\n\r\n>"}
[DEBUG] ESP32 RX matched "+MQTTPUB:OK" after 11 bytes: {"data":"+MQTTPUB:OK"}
```

A bare `"OK"` match would have technically still worked here, by
accident — `"+MQTTPUB:OK"` contains `"OK"` as a substring. Matching the
real marker instead is what actually distinguishes success from
`+MQTTPUB:FAIL`, so the driver matches on it explicitly rather than
relying on that coincidence.

## Solution

[`Sw/`](./Sw/) has a real, buildable implementation:

```sh
cd Sw
cp wifi_secrets.h.example wifi_secrets.h   # once; then edit
tdf.py build
tdf.py flash
tdf.py monitor
```

Same CMake shape as ESP32 Connectivity; `GEN_DIR` is four levels up to
[`Generation/`](../../../Generation/). The ESP32 driver
([`tiremo_esp32.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c)) is
ESP32 Connectivity Tutorial 02's WiFi-join subset (not the full BluFi
stack — this tutorial doesn't need it) with MQTT functions added to the
same file, continuing that section's "one growing driver, not a second
UART stack" pattern. No reference MQTT/TLS code exists anywhere in the
private reference firmware — this driver's MQTT half is original,
built and debugged against the real ESP-AT command set.

**Genuinely verified, for real, on the connected board — and
independently, through a real public broker:** `tdf.py build` compiles
and links clean (`.text` 17268 bytes). `tdf.py flash` completed a full
program-and-verify cycle over SWD (`** Verified OK **`). `tdf.py
monitor` then showed, straight from the real hardware:

```text
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

repeating every 5 seconds, alternating string (`MqttPublish`) and
binary-safe (`MqttPublishRaw`, logged as `....` because four raw
counter bytes aren't printable ASCII — expected, not an error) publishes.

That alone proves a round trip, but not that the message actually left
this machine. So this was also checked independently: an `mqttx` CLI
subscriber (`mqttx sub -h broker.emqx.io -t tiremo-cortex/mqtt-demo`),
a completely separate MQTT client with no connection to this firmware
or this repo, was run at the same time and genuinely received the same
messages — `hello from Tiremo Cortex #6`, `#8`, and the raw
`00 00 00 07` / `00 00 00 09` counter bytes — confirming this really
went through `broker.emqx.io` over the open internet, not a local
loopback.

## Walkthrough

### 1. MQTT identity — `TIREMO_ESP32_MqttUserConfig()`

```c
(void)snprintf(command, sizeof(command),
               "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",
               pClientId, pUsername, pPassword);
```

LinkID `0` (this driver only ever uses one MQTT link), scheme `1`
(plain TCP — see **A Real Bug** above), empty username/password (this
public broker needs none), no cert/CA (slots `0`, path `""` — those
matter starting Tutorial 02).

### 2. Connect and subscribe

```c
TIREMO_ESP32_MqttConnect("broker.emqx.io", 1883, 10000U);
TIREMO_ESP32_MqttSubscribe("tiremo-cortex/mqtt-demo", 0U);
```

Both are ordinary `AT+MQTTCONN` / `AT+MQTTSUB`, matched on `"OK"` —
no new response-parsing tricks needed here.

### 3. Two ways to publish

[`Sw/Src/prv_user_code.c`](./Sw/Src/prv_user_code.c) alternates between
them on the same topic:

```c
TIREMO_ESP32_MqttPublish(topic, "hello from Tiremo Cortex #0", 0, false);
/* vs. */
uint8_t counter[4] = { 0, 0, 0, 1 };
TIREMO_ESP32_MqttPublishRaw(topic, counter, 4, 0, false);
```

`MqttPublish` sends the payload inside AT-command quotes — simple, but
it means the payload can never contain `"`, `\r`, or `\n`.
`MqttPublishRaw` sends it as raw, length-prefixed bytes instead: issue
`AT+MQTTPUBRAW=...,<length>,...`, wait for the module's `>` prompt,
then transmit exactly `<length>` raw bytes — safe for anything,
including the embedded zero byte a JSON-with-quotes payload or a
packed binary struct might contain later in this curriculum.

### 4. Receiving the echo — a real length-prefixed parse

ESP-AT reports an inbound message as an unsolicited URC:

```text
+MQTTSUBRECV:0,"tiremo-cortex/mqtt-demo",4,<4 raw bytes>
```

`TIREMO_ESP32_MqttWaitMessage()` in
[`tiremo_esp32.c`](./Sw/Drivers/esp32/Src/tiremo_esp32.c) accumulates
bytes into a capture buffer exactly like `SendCommandCapture` does, but
instead of matching a fixed substring, it re-parses the buffer after
every byte: find the `+MQTTSUBRECV:` marker, parse the quoted topic,
parse the decimal length field, then check whether the buffer already
holds that many bytes *past* the length field. Only then is the
message complete — a genuine length-prefixed parse, not a text
heuristic, which is what makes it correct for the binary payload in
step 3 too. The one real limit: the whole URC (header + payload) has to
fit inside this driver's 512-byte capture buffer, same constraint
`AT+CWLAP`'s scan dump already had in ESP32 Connectivity.

### 5. Build, flash, and watch

```sh
tdf.py build
tdf.py flash
tdf.py monitor
```

See **Solution** above for what's actually confirmed on real hardware
here, including the independent `mqttx` cross-check.

## Done When

A message published by this board on a topic it also subscribes to is
echoed back and logged — genuinely confirmed, both by this board's own
log and by an independent `mqttx` subscriber on the same public
broker; see **Solution**.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
