<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../images/banner.png" width="100%" height="16">
      <h1>Production Firmware Tutorials</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Feature Tutorials

Feature tutorials cover one concept each — a single driver, a single AT command sequence, a single cloud primitive. Work through them in order; each one names its prerequisite.

 <table style="width:100%">
 <tr>
 <td width="35%" align="center"><b>Tutorial</b>
 <td width="65%" align="center"><b>Description</b>
 </tr>
 <tr>
 <td align="center"><a href="./01-Shadow_Driven_LED_State_Machine/">Shadow-Driven LED State Machine</a></td>
 <td>Auto-select normal/alert/low_battery/connecting/off LED modes from live device state.</td>
 </tr>
 <tr>
 <td align="center"><a href="./02-Edge_Alarms_With_Hysteresis/">Edge Alarms with Hysteresis</a></td>
 <td>Publish threshold-crossing alarm and clear events for temperature, humidity, fall, shock, vibration, sound, and battery.</td>
 </tr>
 <tr>
 <td align="center"><a href="./03-Batched_Telemetry_Publisher/">Batched Telemetry Publisher</a></td>
 <td>Publish averaged sensor batches on a shadow-configurable interval, with retries and adaptive polling.</td>
 </tr>
 <tr>
 <td align="center"><a href="./04-AWS_IoT_Jobs_Health_Check/">AWS IoT Jobs — Health Check</a></td>
 <td>Handle jobs/notify → start-next → UpdateJobExecution SUCCEEDED with statusDetails.</td>
 </tr>
 <tr>
 <td align="center"><a href="./05-Reject_Unsupported_Jobs/">Reject Unsupported Jobs (OTA Stub)</a></td>
 <td>Detect an OTA-shaped job and REJECTED it with a reason — fail closed instead of hanging.</td>
 </tr>
 <tr>
 <td align="center"><a href="./06-Factory_Reset/">Factory Reset</a></td>
 <td>10 s USER-button hold wipes ESP32 SYSMFG credentials and MCU config flash, then reboots.</td>
 </tr>
 <tr>
 <td align="center"><a href="./07-Link_Recovery_And_Backoff/">Link Recovery & Backoff</a></td>
 <td>Detect MQTT session loss, reconnect with backoff, and reboot after repeated failures.</td>
 </tr>
 </table>

<p align="center"><img alt="" src="../../images/banner.png" width="100%" height="16"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
