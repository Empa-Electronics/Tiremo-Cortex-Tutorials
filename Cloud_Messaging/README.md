<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../images/banner.png" width="100%">
      <h1>Cloud Messaging (MQTT & TLS)</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Cloud Messaging (MQTT & TLS)

Get MQTT working end to end: a public broker first with no TLS, then mutual-TLS credentials staged in the ESP32's own NVS, then a real Tiremo Cloud session, then the handler-driven polling pattern production firmware relies on.

The broker is [`iot.tiremo.ai`](https://iot.tiremo.ai). TLS SNI and the first mutual-TLS connection are documented in [TLS SNI](https://docs.tiremo.ai/device-provisioning/tls-sni) and [first connection](https://docs.tiremo.ai/device-provisioning/third-phase). Fleet provisioning comes later in [Tiremo Cloud](../Tiremo_Cloud/) — see [device provisioning](https://docs.tiremo.ai/device-provisioning/getting-started).

- The [**Feature Tutorials**](./Feature_Tutorials/) cover one concept each — a single driver, a single AT command sequence, a single cloud primitive.

<p align="center"><img alt="" src="../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
