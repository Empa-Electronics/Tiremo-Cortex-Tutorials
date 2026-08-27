<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../images/banner.png" width="100%">
      <h1>Tiremo Cloud Tutorials</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Feature Tutorials

Feature tutorials cover one concept each — a single driver, a single AT command sequence, a single cloud primitive. Work through them in order; each one names its prerequisite. Matching platform pages are linked from the [Tiremo Cloud](../) section README and from [docs.tiremo.ai](https://docs.tiremo.ai).

 <table style="width:100%">
 <tr>
 <td width="35%" align="center"><b>Tutorial</b>
 <td width="65%" align="center"><b>Description</b>
 </tr>
 <tr>
 <td align="center"><a href="./01-Device_Identity_From_MAC/">Device Identity from MAC</a></td>
 <td>Derive a stable deviceId/thingName from the ESP32's station MAC.</td>
 </tr>
 <tr>
 <td align="center"><a href="./02-Minimal_JSON_Helpers/">Minimal JSON Helpers</a></td>
 <td>Build just enough JSON find/write helpers to parse shadow documents without a full parser library.</td>
 </tr>
 <tr>
 <td align="center"><a href="./03-Fleet_Provisioning_By_Claim/">Fleet Provisioning by Claim Certificate</a></td>
 <td>Implement fleet provisioning: CreateKeysAndCertificate → RegisterThing → store the issued cert.</td>
 </tr>
 <tr>
 <td align="center"><a href="./04-Shadow_Get_Apply_Report/">Shadow Get + Apply + Report</a></td>
 <td>On boot, fetch the shadow, apply desired state locally, and echo reported so Tiremo Cloud clears the delta.</td>
 </tr>
 <tr>
 <td align="center"><a href="./05-Live_Shadow_Delta_Handling/">Live Shadow Delta Handling</a></td>
 <td>Subscribe to shadow/update/delta and apply a change live, with no reboot.</td>
 </tr>
 <tr>
 <td align="center"><a href="./06-Publishing_Alarms_And_Telemetry/">Publishing Alarms and Telemetry</a></td>
 <td>Publish sensor telemetry and edge alarms on MQTT topics, separate from the shadow control loop. (Not implemented yet.)</td>
 </tr>
 <tr>
 <td align="center"><a href="./07-Tiremo_Jobs/">Tiremo Jobs</a></td>
 <td>Accept and complete a Tiremo Job over MQTT, with IN_PROGRESS / SUCCEEDED status. (Not implemented yet.)</td>
 </tr>
 </table>

<p align="center"><img alt="" src="../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
