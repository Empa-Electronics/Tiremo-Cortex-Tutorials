<p class="sphinxhide" align="right"><a>English</a> | <a href="docs-tr/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="images/tiremo_sub_brand_1_mono_black.png" width="220">
      </picture>
      <br>
      <img alt="" src="images/banner.png" width="100%">
      <h1>Tiremo Cortex — In-Depth Tutorials</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


## About This Board

This repository is a step-by-step build-up of firmware for the **Tiremo
Cortex**, an ESP32-based IoT sensor board. WiFi (station join + BluFi BLE
provisioning) is the only radio, and AWS IoT Core is reached over MQTT with
mutual TLS through the ESP32's own AT command set.

Every tutorial here is a smaller, standalone slice of a real, working
firmware kept in this repo for reference (see
[Reference Implementation](#reference-implementation) below) — so at any
point you can check your work against a proven implementation of the same
feature.

## Where to Start

New to this codebase? Start with [Getting Started](./Getting_Started/README.md) —
blink an LED, bring up UART logging, read the button, drive the ten-LED bank.
No WiFi or cloud account needed for that section.

From there, tutorials are organized by topic under [Tutorials](#tutorials),
roughly in the order a real bring-up would tackle them: local sensors, then
ESP32 WiFi, then cloud messaging, then AWS IoT Core, then the production
behaviors (shadow-driven LEDs, alarms, telemetry, jobs, factory reset) that
turn a working prototype into a fleet-ready device.

Each topic splits into two kinds of tutorial:

- **Feature Tutorials** cover one concept each — a single driver, a single AT
  command sequence, a single cloud primitive.
- **Design Tutorials** combine several feature tutorials into one working
  subsystem, closer to what actually ships.

## How to Get Help

- Check a tutorial's **Reference Implementation** links first — the answer
  is usually a working example away in `referance_only/Tiremo-Secure-Iot-Sw/`.
- For questions or issues about a tutorial, open an
  [Issue](https://github.com/Empa-Electronics/Tiremo-Cortex-Tutorials/issues).

## How to Download the Repository

```sh
git clone https://github.com/Empa-Electronics/Tiremo-Cortex-Tutorials.git
```

## Release Notes

[Change Log](./CHANGELOG.md)

## Tutorials

<table border="0" width="100%">
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
    <tbody>
        <tr>
            <td colspan="2" height="24" align="center">
                <strong><a href="./Cloud_Messaging/">Cloud Messaging (MQTT & TLS)</a></strong>
            </td>
        </tr>
        <tr>
            <td colspan="2" height="48">
                Get MQTT working end to end: a public broker first with no TLS, then mutual-TLS credentials staged in the ESP32's own NVS, then a real AWS IoT Core session, then the handler-driven polling pattern production firmware relies on.
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
            <td><a href="./Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/">MQTT mTLS to AWS IoT Core</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="./Cloud_Messaging/Feature_Tutorials/04-Reliable_Inbound_Message_Draining/">Reliable Inbound Message Draining</a></td>
            <td></td>
        </tr>
    </tbody>
</table>

<table border="0" width="100%">
    <tbody>
        <tr>
            <td colspan="2" height="24" align="center">
                <strong><a href="./AWS_IoT_Core/">AWS IoT Core</a></strong>
            </td>
        </tr>
        <tr>
            <td colspan="2" height="48">
                Device identity, fleet provisioning by claim certificate, the minimal JSON helpers this firmware relies on instead of a full parser library, and the device shadow control loop — get, apply, report, and live delta.
            </td>
        </tr>
        <tr>
            <td height="24"><strong>Feature Tutorials</strong></td>
            <td height="24"><strong>Design Tutorials</strong></td>
        </tr>
        <tr>
            <td><a href="./AWS_IoT_Core/Feature_Tutorials/01-Device_Identity_From_MAC/">Device Identity from MAC</a></td>
            <td><a href="./AWS_IoT_Core/Design_Tutorials/01-Provision_And_Sync_A_Real_Thing/">Provision and Sync a Real Thing</a></td>
        </tr>
        <tr>
            <td><a href="./AWS_IoT_Core/Feature_Tutorials/02-Fleet_Provisioning_By_Claim/">Fleet Provisioning by Claim Certificate</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="./AWS_IoT_Core/Feature_Tutorials/03-Minimal_JSON_Helpers/">Minimal JSON Helpers</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="./AWS_IoT_Core/Feature_Tutorials/04-Shadow_Get_Apply_Report/">Shadow Get + Apply + Report</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="./AWS_IoT_Core/Feature_Tutorials/05-Live_Shadow_Delta_Handling/">Live Shadow Delta Handling</a></td>
            <td></td>
        </tr>
    </tbody>
</table>

<table border="0" width="100%">
    <tbody>
        <tr>
            <td colspan="2" height="24" align="center">
                <strong><a href="./Production_Firmware/">Production Firmware</a></strong>
            </td>
        </tr>
        <tr>
            <td colspan="2" height="48">
                The behaviors that turn a working prototype into a fleet-ready device: shadow-driven LEDs, edge alarms with hysteresis, batched telemetry, AWS IoT Jobs, factory reset, and link recovery — culminating in the full capstone build.
            </td>
        </tr>
        <tr>
            <td height="24"><strong>Feature Tutorials</strong></td>
            <td height="24"><strong>Design Tutorials</strong></td>
        </tr>
        <tr>
            <td><a href="./Production_Firmware/Feature_Tutorials/01-Shadow_Driven_LED_State_Machine/">Shadow-Driven LED State Machine</a></td>
            <td><a href="./Production_Firmware/Design_Tutorials/01-Full_Event_Loop_Integration/">Full Event Loop Integration (Capstone)</a></td>
        </tr>
        <tr>
            <td><a href="./Production_Firmware/Feature_Tutorials/02-Edge_Alarms_With_Hysteresis/">Edge Alarms with Hysteresis</a></td>
            <td><a href="./Production_Firmware/Design_Tutorials/02-Add_A_New_Sensor_End_To_End/">Add a New Sensor End-to-End</a></td>
        </tr>
        <tr>
            <td><a href="./Production_Firmware/Feature_Tutorials/03-Batched_Telemetry_Publisher/">Batched Telemetry Publisher</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="./Production_Firmware/Feature_Tutorials/04-AWS_IoT_Jobs_Health_Check/">AWS IoT Jobs — Health Check</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="./Production_Firmware/Feature_Tutorials/05-Reject_Unsupported_Jobs/">Reject Unsupported Jobs (OTA Stub)</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="./Production_Firmware/Feature_Tutorials/06-Factory_Reset/">Factory Reset</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="./Production_Firmware/Feature_Tutorials/07-Link_Recovery_And_Backoff/">Link Recovery & Backoff</a></td>
            <td></td>
        </tr>
    </tbody>
</table>

<table border="0" width="100%">
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


## Reference Implementation

[`referance_only/Tiremo-Secure-Iot-Sw/`](referance_only/Tiremo-Secure-Iot-Sw/) is the reference firmware
this curriculum is built from — same sensors, same AWS IoT
shadow/jobs/provisioning design. Its [`README.md`](referance_only/Tiremo-Secure-Iot-Sw/README.md) and
[`SHADOW_README.md`](referance_only/Tiremo-Secure-Iot-Sw/SHADOW_README.md) are worth reading in full
before AWS IoT Core — nearly every field name and topic used there
matches this curriculum exactly.

<p align="center"><img alt="" src="images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
