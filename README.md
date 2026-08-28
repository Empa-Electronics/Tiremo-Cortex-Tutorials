<p class="sphinxhide" align="right"><a>English</a> | <a href="docs-tr/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="images/banner.png" width="100%">
      <h1>Tiremo Cortex — In-Depth Tutorials</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


## About This Board

This repository is a step-by-step build-up of firmware for the **Tiremo
Cortex**, an ESP32-based IoT sensor board. WiFi (station join + BluFi BLE
provisioning) is the only radio. The board is built to connect to
**Tiremo Cloud** at [`iot.tiremo.ai`](https://iot.tiremo.ai) over MQTT with mutual TLS through the
ESP32's own AT command set. Operators use the **Tiremo UI**. Platform
concepts (provisioning, Device Shadow, jobs, telemetry) are in the
[Tiremo documentation](https://docs.tiremo.ai).

Every tutorial here is a smaller, standalone slice of a complete firmware
build — see [Solutions](#solutions) below for where things currently stand.

## Where to Start

New to this codebase? Start with [Getting Started](./Getting_Started/README.md) —
blink an LED, bring up UART logging, read the button, drive the ten-LED bank.
No WiFi or cloud account needed for that section.

From there, tutorials are organized by topic under [Tutorials](#tutorials),
roughly in the order a real bring-up would tackle them: local sensors, then
ESP32 WiFi, then cloud messaging, then Tiremo Cloud, then Intelligence
(collect telemetry, train in the Tiremo UI, deploy a Solution).

Each topic splits into two kinds of tutorial:

- **Feature Tutorials** cover one concept each — a single driver, a single AT
  command sequence, a single cloud primitive.
- **Design Tutorials** combine several feature tutorials into one working
  subsystem, closer to what actually ships.

## How to Get Help

- Check a tutorial's **Solution** section first — it says whether a worked
  solution exists yet for that tutorial.
- For the cloud platform (MQTT, fleet provisioning, Device Shadow, jobs),
  start at the [Tiremo documentation](https://docs.tiremo.ai) — each
  [Tiremo Cloud](./Tiremo_Cloud/) tutorial links the matching pages.
- For questions or issues about a tutorial, open an
  [Issue](https://github.com/Empa-Electronics/Tiremo-Cortex-Tutorials/issues).

## How to Download the Repository

```sh
git clone https://github.com/Empa-Electronics/Tiremo-Cortex-Tutorials.git
```

## Hardware

[Hardware Overview](./Hardware.md) — board block diagram, host connection diagram, and pinout reference. No firmware, just the board itself.

## Release Notes

[Change Log](./CHANGELOG.md)

## Tutorials

<table border="0" width="100%">
    <colgroup><col width="50%"><col width="50%"></colgroup>
    <tbody>
        <tr>
            <td colspan="2" height="24" align="center">
                <strong><a href="./Getting_Started">Getting Started</a></strong>
            </td>
        </tr>
        <tr>
            <td colspan="2" height="48">
                Start here! Blink, log, read a button, and drive all ten LEDs — no WiFi or cloud required yet.
            </td>
        </tr>
        <tr>
            <td><a href="./Getting_Started/01-Hello_Blink/">Hello Blink</a></td>
            <td><a href="./Getting_Started/02-SysTick_Timing/">SysTick Timing</a></td>
        </tr>
        <tr>
            <td><a href="./Getting_Started/03-Debug_UART_Logging/">Debug UART Logging</a></td>
            <td><a href="./Getting_Started/04-Button_Read_And_Debounce/">Button Read & Debounce</a></td>
        </tr>
    </tbody>
</table>

<table border="0" width="100%">
    <colgroup><col width="50%"><col width="50%"></colgroup>
    <tbody>
        <tr>
            <td colspan="2" height="24" align="center">
                <strong><a href="./Sensors_And_Storage/">Sensors & Local Storage</a></strong>
            </td>
        </tr>
        <tr>
            <td colspan="2" height="48">
                Bring up every onboard sensor and driver — SHT40, LIS2DE12, battery, microphone — plus flash-backed config persistence and the software wall clock, with no networking involved yet.
            </td>
        </tr>
        <tr>
            <td height="24"><strong>Feature Tutorials</strong></td>
            <td height="24"><strong>Design Tutorials</strong></td>
        </tr>
        <tr>
            <td><a href="./Sensors_And_Storage/Feature_Tutorials/01-I2C_Bus_Bringup/">I2C Bus Bring-up</a></td>
            <td><a href="./Sensors_And_Storage/Design_Tutorials/01-Standalone_Environmental_Logger/">Standalone Environmental Logger</a></td>
        </tr>
        <tr>
            <td><a href="./Sensors_And_Storage/Feature_Tutorials/02-SHT40_Temperature_Humidity/">SHT40 Temperature / Humidity</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="./Sensors_And_Storage/Feature_Tutorials/03-LIS2DE12_Accelerometer/">LIS2DE12 Accelerometer</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="./Sensors_And_Storage/Feature_Tutorials/04-Battery_Voltage_ADC/">Battery Voltage via ADC</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="./Sensors_And_Storage/Feature_Tutorials/05-Microphone_RMS_To_dB/">Microphone: RMS Capture to dB</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="./Sensors_And_Storage/Feature_Tutorials/06-Flash_Backed_Config/">Flash-Backed Config</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="./Sensors_And_Storage/Feature_Tutorials/07-Software_Wall_Clock/">Software Wall Clock</a></td>
            <td></td>
        </tr>
    </tbody>
</table>

<table border="0" width="100%">
    <colgroup><col width="50%"><col width="50%"></colgroup>
    <tbody>
        <tr>
            <td colspan="2" height="24" align="center">
                <strong><a href="./ESP32_Connectivity/">ESP32 Connectivity</a></strong>
            </td>
        </tr>
        <tr>
            <td colspan="2" height="48">
                Bring up the ESP32 AT-command transport, join WiFi with retries, sync time over NTP, and provision credentials over BluFi BLE — the full connection cascade this board uses to get online.
            </td>
        </tr>
        <tr>
            <td height="24"><strong>Feature Tutorials</strong></td>
            <td height="24"><strong>Design Tutorials</strong></td>
        </tr>
        <tr>
            <td><a href="./ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/">AT Command Transport</a></td>
            <td><a href="./ESP32_Connectivity/Design_Tutorials/01-Boot_Cascade_Saved_WiFi_Then_BluFi/">Boot Cascade: Saved WiFi → BluFi</a></td>
        </tr>
        <tr>
            <td><a href="./ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/">WiFi Station Join</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="./ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/">Connect/Retry State Machine</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="./ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/">NTP Time Sync</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="./ESP32_Connectivity/Feature_Tutorials/05-BluFi_Provisioning/">BluFi BLE Provisioning</a></td>
            <td></td>
        </tr>
    </tbody>
</table>

<table border="0" width="100%">
    <colgroup><col width="50%"><col width="50%"></colgroup>
    <tbody>
        <tr>
            <td colspan="2" height="24" align="center">
                <strong><a href="./Cloud_Messaging/">Cloud Messaging (MQTT & TLS)</a></strong>
            </td>
        </tr>
        <tr>
            <td colspan="2" height="48">
                Get MQTT working end to end: a public broker first with no TLS, then mutual-TLS credentials staged in the ESP32's own NVS, then a real Tiremo Cloud session.
            </td>
        </tr>
        <tr>
            <td height="24"><strong>Feature Tutorials</strong></td>
            <td height="24"><strong>Design Tutorials</strong></td>
        </tr>
        <tr>
            <td><a href="./Cloud_Messaging/Feature_Tutorials/01-MQTT_To_Public_Broker/">MQTT to a Public Broker</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="./Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/">mTLS Credentials in Module NVS</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="./Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/">MQTT mTLS to Tiremo Cloud</a></td>
            <td></td>
        </tr>
    </tbody>
</table>

<table border="0" width="100%">
    <colgroup><col width="50%"><col width="50%"></colgroup>
    <tbody>
        <tr>
            <td colspan="2" height="24" align="center">
                <strong><a href="./Tiremo_Cloud/">Tiremo Cloud</a></strong>
            </td>
        </tr>
        <tr>
            <td colspan="2" height="48">
                Device identity, the minimal JSON helpers this firmware relies on instead of a full parser library, fleet provisioning by claim certificate, the device shadow control loop, telemetry and alarms, and Tiremo Jobs.
            </td>
        </tr>
        <tr>
            <td height="24"><strong>Feature Tutorials</strong></td>
            <td height="24"><strong>Design Tutorials</strong></td>
        </tr>
        <tr>
            <td><a href="./Tiremo_Cloud/Feature_Tutorials/01-Device_Identity_From_MAC/">Device Identity from MAC</a></td>
            <td><a href="./Tiremo_Cloud/Design_Tutorials/01-Sensor_Telemetry_And_Shadow_Control/">Sensor Telemetry and Shadow Control</a></td>
        </tr>
        <tr>
            <td><a href="./Tiremo_Cloud/Feature_Tutorials/02-Minimal_JSON_Helpers/">Minimal JSON Helpers</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="./Tiremo_Cloud/Feature_Tutorials/03-Fleet_Provisioning_By_Claim/">Fleet Provisioning by Claim Certificate</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="./Tiremo_Cloud/Feature_Tutorials/04-Shadow_Get_Apply_Report/">Shadow Get + Apply + Report</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="./Tiremo_Cloud/Feature_Tutorials/05-Live_Shadow_Delta_Handling/">Live Shadow Delta Handling</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="./Tiremo_Cloud/Feature_Tutorials/06-Publishing_Alarms_And_Telemetry/">Publishing Alarms and Telemetry</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="./Tiremo_Cloud/Feature_Tutorials/07-Tiremo_Jobs/">Tiremo Jobs</a></td>
            <td></td>
        </tr>
    </tbody>
</table>

<table border="0" width="100%">
    <colgroup><col width="50%"><col width="50%"></colgroup>
    <tbody>
        <tr>
            <td colspan="2" height="24" align="center">
                <strong><a href="./Intelligence/">Intelligence</a></strong>
            </td>
        </tr>
        <tr>
            <td colspan="2" height="48">
                Collect training data (via Data Explorer or any path you prefer), turn it into a Data Source, train an Experiment, and deploy a Solution, including an on-device Edge package, from the Tiremo UI. Feature tutorials stay UI-only. Three design tutorials flash LIS2DE12 firmware: per-sample telemetry, a local 8-sample window, then HMR Edge inference.
            </td>
        </tr>
        <tr>
            <td height="24"><strong>Feature Tutorials</strong></td>
            <td height="24"><strong>Design Tutorials</strong></td>
        </tr>
        <tr>
            <td><a href="./Intelligence/Feature_Tutorials/01-Data_Collection/">Data Collection</a></td>
            <td><a href="./Intelligence/Design_Tutorials/01-Accelerometer_Telemetry/">Accelerometer Telemetry</a></td>
        </tr>
        <tr>
            <td><a href="./Intelligence/Feature_Tutorials/02-Data_Sources/">Data Sources</a></td>
            <td><a href="./Intelligence/Design_Tutorials/02-Accelerometer_Windowing/">Accelerometer Windowing</a></td>
        </tr>
        <tr>
            <td><a href="./Intelligence/Feature_Tutorials/03-Experiments/">Experiments</a></td>
            <td><a href="./Intelligence/Design_Tutorials/03-Edge_Deployment_HMR/">HMR Gesture Classifier on the Edge</a></td>
        </tr>
        <tr>
            <td><a href="./Intelligence/Feature_Tutorials/04-Solutions/">Solutions</a></td>
            <td></td>
        </tr>
    </tbody>
</table>

<table border="0" width="100%">
    <colgroup><col width="50%"><col width="50%"></colgroup>
    <tbody>
        <tr>
            <td colspan="2" height="24" align="center">
                <strong><a href="./Developer_Contributed">Developer Contributed</a></strong>
            </td>
        </tr>
        <tr>
            <td colspan="2" height="48">
                Community and internal tutorials that don't fit the numbered curriculum — bring-up notes,
                alternate backends, bench tooling. Contributions welcome.
            </td>
        </tr>
    </tbody>
</table>


## Solutions

Most tutorials don't have a written solution yet — this repository is
still mostly a curriculum scaffold (structure, learning goals, step
outlines), not a finished course. Each tutorial ships its own worked
solution, when one exists, in a local `Sw/` folder next to its
`README.md`; check that tutorial's own **Solution** section for current
status.

Buildable tutorials share one copy of the ABOV vendor SDK, checked into
this repo once at [`Generation/`](Generation/), and one build/flash CLI:
[`tools/tdf.py`](tools/tdf.py) ("Tiremo Development Framework" — the
same role `idf.py` plays for ESP-IDF). First time only:

```sh
./install.sh
source ./export.sh
```

Then, from any tutorial's `Sw/` directory:

```sh
tdf.py build             # configure + build
tdf.py flash              # build, then flash over SWD
tdf.py monitor            # reset the board, stream the UART log (color-coded)
tdf.py clean              # remove build/
tdf.py clean-device       # mass-erase the board's chip flash (asks first)
tdf.py create-project <Name>   # scaffold a new, empty Sw/ project
```

See [01-Hello_Blink's Solution](Getting_Started/01-Hello_Blink/#solution)
for the full story — what each command actually does, why CMake instead
of a hand-written Makefile, and Windows-specific notes.

- [x] [01-Hello_Blink](Getting_Started/01-Hello_Blink/) — built, flashed, verified on real hardware
- [x] [02-SysTick_Timing](Getting_Started/02-SysTick_Timing/) — built, flashed, verified
- [x] [03-Debug_UART_Logging](Getting_Started/03-Debug_UART_Logging/) — built, flashed, verified; UART output confirmed via `tdf.py monitor`
- [x] [04-Button_Read_And_Debounce](Getting_Started/04-Button_Read_And_Debounce/) — built, flashed, verified; button-press behavior itself not yet confirmed (needs a human finger — see the tutorial's own Solution)
- [x] [05-Ten_LED_Pattern_Driver](Getting_Started/05-Ten_LED_Pattern_Driver/) — built, flashed, verified; auto-cycle timing confirmed; LED staggering and button-triggered advance not yet confirmed (see the tutorial's own Solution)
- [x] [Sensors_And_Storage/01-I2C_Bus_Bringup](Sensors_And_Storage/Feature_Tutorials/01-I2C_Bus_Bringup/) — built, flashed, verified; both SHT40 and LIS2DE12 genuinely ACK on I2C2, confirmed live over `tdf.py monitor`
- [x] [Sensors_And_Storage/02-SHT40_Temperature_Humidity](Sensors_And_Storage/Feature_Tutorials/02-SHT40_Temperature_Humidity/) — built, flashed, verified; live T/RH once per second over `tdf.py monitor` (CRC-valid, stable within sensor noise)
- [x] [Sensors_And_Storage/03-LIS2DE12_Accelerometer](Sensors_And_Storage/Feature_Tutorials/03-LIS2DE12_Accelerometer/) — built, flashed, verified; WHO_AM_I=0x33, Z≈1036 mg at rest; axis-by-axis tilt not confirmed
- [x] [Sensors_And_Storage/04-Battery_Voltage_ADC](Sensors_And_Storage/Feature_Tutorials/04-Battery_Voltage_ADC/) — built, flashed, verified; live VCORE raw + AVDD mV; not compared to a multimeter
- [x] [Sensors_And_Storage/05-Microphone_RMS_To_dB](Sensors_And_Storage/Feature_Tutorials/05-Microphone_RMS_To_dB/) — built, flashed, verified; DMA blocks climbing, quiet ~24 dB; talk-vs-silence not confirmed
- [x] [Sensors_And_Storage/06-Flash_Backed_Config](Sensors_And_Storage/Feature_Tutorials/06-Flash_Backed_Config/) — built, flashed, verified; bootCount persisted across reset, corrupt CRC rejected
- [x] [Sensors_And_Storage/07-Software_Wall_Clock](Sensors_And_Storage/Feature_Tutorials/07-Software_Wall_Clock/) — built, flashed, verified; GetEpoch advances 1 s per second after a manual SetEpoch
- [x] [Sensors_And_Storage/Design/01-Standalone_Environmental_Logger](Sensors_And_Storage/Design_Tutorials/01-Standalone_Environmental_Logger/) — built, flashed, verified; live SHT40+accel+battery+mic sample (`flags=0x07`); `seq` history survived SWD reset
- [x] [ESP32_Connectivity/01-AT_Command_Transport](ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/) — built, flashed, verified; `AT`→`OK`, echo-on then `ATE0` echo-free, 5 s heartbeat
- [x] [ESP32_Connectivity/02-WiFi_Station_Join](ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/) — built (`.text` 15200); `CWMODE` / `CWJAP` / `CWSTATE:2` + substring-gotcha; join confirmed on a real 2.4 GHz AP
- [x] [ESP32_Connectivity/03-Connect_Retry_State_Machine](ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/) — built (`.text` 19440), flashed; `CWLAP` scan-first, 3 retries, `WF01` last-good SSID; heartbeat after join confirmed
- [x] [ESP32_Connectivity/04-NTP_Time_Sync](ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/) — built (`.text` 20956), flashed; SNTP → `SetEpoch`; `GetEpoch()` matched UTC and advanced 1 s/s
- [x] [ESP32_Connectivity/05-BluFi_Provisioning](ESP32_Connectivity/Feature_Tutorials/05-BluFi_Provisioning/) — built (`.text` 21156), flashed; `AT+BLUFI=1` advertising `Tiremo-Cortex` confirmed; EspBluFi Configure is a human step
- [x] [Cloud_Messaging/01-MQTT_To_Public_Broker](Cloud_Messaging/Feature_Tutorials/01-MQTT_To_Public_Broker/) — built (`.text` 17268), flashed, verified over SWD; full publish/subscribe round trip (string + binary-safe) confirmed live, and independently cross-checked with a separate `mqttx` CLI subscriber on the same public broker
- [x] [Cloud_Messaging/02-mTLS_Credentials_In_Module_NVS](Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/) — built (`.text` 21140), flashed, verified over SWD; write/read-back/erase round-trips byte-for-byte confirmed live for all three real (self-signed test) PEM credential slots against the correct `mqtt_*` SYSMFG namespace (see the tutorial's own "A Real Bug" note)
- [x] [Cloud_Messaging/03-MQTT_mTLS_To_AWS_IoT](Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/) — built (`.text` 21648), flashed, verified over SWD; genuine mutual-TLS `+MQTTCONNECTED` against a real Tiremo Cloud endpoint, with SNTP time sync ahead of the connect sequence; connect only, no publish/subscribe yet
- [x] [ESP32_Connectivity/Design/01-Boot_Cascade_Saved_WiFi_Then_BluFi](ESP32_Connectivity/Design_Tutorials/01-Boot_Cascade_Saved_WiFi_Then_BluFi/) — built (`.text` 25196), flashed; EspBluFi provisioned → `CS01` saved → next reset came up on the `nvs` path with NTP, no BluFi step
- [x] [Tiremo_Cloud/01-Device_Identity_From_MAC](Tiremo_Cloud/Feature_Tutorials/01-Device_Identity_From_MAC/) — built (`.text` 15060), flashed; `CIPSTAMAC?` → `deviceId 30eda0806324`, heartbeat re-read unchanged
- [x] [Tiremo_Cloud/02-Minimal_JSON_Helpers](Tiremo_Cloud/Feature_Tutorials/02-Minimal_JSON_Helpers/) — built (`.text` 16156), flashed; Find* + writer round-trip on a sample shadow (`reporting.enabled`, colon-in-string, sibling-key miss)
- [x] [Tiremo_Cloud/04-Shadow_Get_Apply_Report](Tiremo_Cloud/Feature_Tutorials/04-Shadow_Get_Apply_Report/) — built (`.text` 33764), flashed; boot GET → apply `reporting.enabled` / `led.on` → full desired echo as `reported` → second GET `no leftover delta`
- [x] [Tiremo_Cloud/05-Live_Shadow_Delta_Handling](Tiremo_Cloud/Feature_Tutorials/05-Live_Shadow_Delta_Handling/) — built (`.text` 34928), flashed; boot GET/report then `subscribed update/delta`; flip `desired` in the console to apply live (no reboot)
- [ ] [Tiremo_Cloud/06-Publishing_Alarms_And_Telemetry](Tiremo_Cloud/Feature_Tutorials/06-Publishing_Alarms_And_Telemetry/) — built (`.text` 37528); 05 ready loop + `pub/{product}/{device}/telemetry` / `alarm`; not yet flashed/verified on hardware
- [ ] [Tiremo_Cloud/07-Tiremo_Jobs](Tiremo_Cloud/Feature_Tutorials/07-Tiremo_Jobs/) — built; 06 ready loop + jobs notify/start-next/health SUCCEEDED; not yet flashed/verified on hardware
- [ ] [Tiremo_Cloud/Design/01-Sensor_Telemetry_And_Shadow_Control](Tiremo_Cloud/Design_Tutorials/01-Sensor_Telemetry_And_Shadow_Control/) — README scaffold only
- [x] [Intelligence/01-Data_Collection](Intelligence/Feature_Tutorials/01-Data_Collection/) — UI walkthrough written
- [x] [Intelligence/02-Data_Sources](Intelligence/Feature_Tutorials/02-Data_Sources/) — UI walkthrough written
- [x] [Intelligence/03-Experiments](Intelligence/Feature_Tutorials/03-Experiments/) — UI walkthrough written
- [x] [Intelligence/04-Solutions](Intelligence/Feature_Tutorials/04-Solutions/) — UI walkthrough written
- [x] [Intelligence/Design/01-Accelerometer_Telemetry](Intelligence/Design_Tutorials/01-Accelerometer_Telemetry/) — `Sw/` publishes per-sample `acc_x`/`acc_y`/`acc_z` telemetry
- [x] [Intelligence/Design/02-Accelerometer_Windowing](Intelligence/Design_Tutorials/02-Accelerometer_Windowing/) — `Sw/` UART tumbling 8×3 window
- [x] [Intelligence/Design/03-Edge_Deployment_HMR](Intelligence/Design_Tutorials/03-Edge_Deployment_HMR/) — end-to-end HMR case; `Datasets/` and `Sw/` (with `Sw/Tiremo_AI/`) inference firmware

<p align="center"><img alt="" src="images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
