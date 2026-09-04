<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../images/banner.png" width="100%">
      <h1>Tiremo Cloud</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Tiremo Cloud

The Tiremo Cortex is built to join **Tiremo Cloud** at
[`iot.tiremo.ai`](https://iot.tiremo.ai). Operators use the **Tiremo UI**
to inspect Things, edit shadow desired state, and queue jobs. Claim
certificates, product identifier, and product secret come from the UI —
see [Getting credentials from the Tiremo UI](./Feature_Tutorials/03-Fleet_Provisioning_By_Claim/#getting-credentials-from-the-tiremo-ui)
(figures in [`figures/`](./figures/)).

Platform documentation (not this firmware repo) is
[docs.tiremo.ai](https://docs.tiremo.ai):

| Cortex tutorial | Platform docs |
|---|---|
| [03 Fleet provisioning](./Feature_Tutorials/03-Fleet_Provisioning_By_Claim/) | [Device provisioning](https://docs.tiremo.ai/device-provisioning/getting-started), [Phase 1](https://docs.tiremo.ai/device-provisioning/first-phase) (bootstrap cert, product identifier, secret), [Phase 2](https://docs.tiremo.ai/device-provisioning/second-phase), [TLS SNI](https://docs.tiremo.ai/device-provisioning/tls-sni), [first connection](https://docs.tiremo.ai/device-provisioning/third-phase) |
| [04](./Feature_Tutorials/04-Shadow_Get_Apply_Report/) / [05](./Feature_Tutorials/05-Live_Shadow_Delta_Handling/) shadow | [Device State Controller](https://docs.tiremo.ai/device-state-controller/introduction), [firmware integration](https://docs.tiremo.ai/device-state-controller/device-integration) |
| [06 Telemetry and alarms](./Feature_Tutorials/06-Publishing_Alarms_And_Telemetry/) | [Phase 3 publish topics](https://docs.tiremo.ai/device-provisioning/third-phase), [architecture & data flow](https://docs.tiremo.ai/introduction/phase-1), [low-rate telemetry](https://docs.tiremo.ai/use-cases/scenario-low-rate-telemetry) |
| [07 Tiremo Jobs](./Feature_Tutorials/07-Tiremo_Jobs/) | [Jobs introduction](https://docs.tiremo.ai/developer-guide/jobs/jobs-introduction), [job templates](https://docs.tiremo.ai/developer-guide/jobs/jobs-phase-1-definition) |
| [08 OTA Firmware Update](./Feature_Tutorials/08-OTA_Firmware_Update/) | [Jobs introduction](https://docs.tiremo.ai/developer-guide/jobs/jobs-introduction), [job templates](https://docs.tiremo.ai/developer-guide/jobs/jobs-phase-1-definition) |

Also: [Developer Guide](https://docs.tiremo.ai/developer-guide), [PoC guide](https://docs.tiremo.ai/poc-guide), [platform FAQ](https://docs.tiremo.ai/faq), [product dictionary](https://docs.tiremo.ai/product-dictionary/).

This section covers device identity, the minimal JSON helpers this
firmware relies on instead of a full parser library, fleet provisioning
by claim certificate, the device shadow control loop, telemetry and
alarms, Tiremo Jobs, and OTA firmware updates.

- The [**Feature Tutorials**](./Feature_Tutorials/) cover one concept each — a single driver, a single AT command sequence, a single cloud primitive.

- The [**Design Tutorials**](./Design_Tutorials/) combine several feature tutorials into one working subsystem.

<p align="center"><img alt="" src="../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
