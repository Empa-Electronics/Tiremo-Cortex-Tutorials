<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../images/tiremo_sub_brand_1_mono_black.png" width="220">
      </picture>
      <br>
      <img alt="" src="../../images/banner.png" width="100%">
      <h1>Cloud Messaging (MQTT & TLS) Tutorials</h1>
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
 <td align="center"><a href="./01-MQTT_To_Public_Broker/">MQTT to a Public Broker</a></td>
 <td>Publish and subscribe against a public test broker, no TLS yet.</td>
 </tr>
 <tr>
 <td align="center"><a href="./02-mTLS_Credentials_In_Module_NVS/">mTLS Credentials in Module NVS</a></td>
 <td>Write Root CA / client cert / client key into the ESP32's SYSMFG slots.</td>
 </tr>
 <tr>
 <td align="center"><a href="./03-MQTT_mTLS_To_AWS_IoT/">MQTT mTLS to AWS IoT Core</a></td>
 <td>Connect to a real AWS IoT endpoint using a manually-issued device certificate — no fleet provisioning yet.</td>
 </tr>
 <tr>
 <td align="center"><a href="./04-Reliable_Inbound_Message_Draining/">Reliable Inbound Message Draining</a></td>
 <td>Switch from polling IsConnected() to a handler-driven MqttPoll() loop so inbound URCs stop getting dropped.</td>
 </tr>
 </table>

<p align="center"><img alt="" src="../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
