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
# macOS / Linux
./install.sh
source ./export.sh
```

```powershell
# Windows (PowerShell)
.\install.ps1
. .\export.ps1
```

```bat
:: Windows (cmd.exe)
install.bat
export.bat
```

`tdf.py` itself is plain Python and runs the same on all three — the
only per-OS pieces are these setup scripts (which check for the
toolchain and wire up `tdf_venv/` + `PATH`). No Git Bash or WSL needed
on Windows. `python tools\tdf.py build` works with no setup step at all.

Then, from any tutorial's `Sw/` directory:

```sh
tdf.py build             # configure + build
tdf.py flash              # build, then flash over SWD
tdf.py monitor            # reset the board, stream the UART log (color-coded)
tdf.py flash monitor      # chain: flash, then monitor
tdf.py build flash monitor
tdf.py clean              # remove build/
tdf.py clean-device       # mass-erase the board's chip flash (asks first)
tdf.py create-project <Name>   # scaffold a new, empty Sw/ project
tdf.py help                    # command overview
tdf.py help flash              # detailed help for one command
```

`source ./export.sh` (bash/zsh) and `. .\export.ps1` (PowerShell) also install tab completion for those command names.

See [01-Hello_Blink's Solution](Getting_Started/01-Hello_Blink/#solution)
for the full story — what each command actually does, why CMake instead
of a hand-written Makefile, and Windows-specific notes.

<p align="center"><img alt="" src="images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
