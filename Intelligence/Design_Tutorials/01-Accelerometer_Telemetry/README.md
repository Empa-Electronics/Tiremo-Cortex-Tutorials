<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/Intelligence/Design_Tutorials/01-Accelerometer_Telemetry/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Accelerometer Telemetry</h1>
      <sub>Tiremo Cortex Tutorials: ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>

# Accelerometer Telemetry

***Board: Tiremo Cortex (ESP32) · Section: Intelligence / Design Tutorials · Difficulty: Medium***

## What this binary does

After the same Tiremo Cloud session as feature tutorial 05 (WiFi, SNTP, fleet provision, shadow get/apply/report), the firmware samples the LIS2DE12 every **125 ms** (8 Hz software pace, chip ODR 10 Hz, ±2 g) and publishes one JSON object per sample:

```json
{"acc_x":12,"acc_y":-3,"acc_z":1004}
```

Topic: `pub/<product_id>/<device_id>/telemetry` (QoS 0). Values are milli-g. Data Explorer CSV columns become `data.acc_x` / `data.acc_y` / `data.acc_z`.

Full walkthrough (labels, Data Explorer, screenshots) comes after this firmware exists. For the HMR case that consumes a later windowed model, see [03-HMR_Edge_Deployment](../03-HMR_Edge_Deployment/).

## Build and flash

Copy secrets and claim PEMs (same files as [Tiremo Cloud 05](../../../Tiremo_Cloud/Feature_Tutorials/05-Live_Shadow_Delta_Handling/)):

```text
cd Intelligence/Design_Tutorials/01-Accelerometer_Telemetry/Sw
cp wifi_secrets.h.example wifi_secrets.h
cp aws_iot_config.h.example aws_iot_config.h
cp certs/claim_cert.pem.example certs/claim_cert.pem
cp certs/claim_key.pem.example certs/claim_key.pem
```

Fill in SSID, product identifier/secret, and the real claim certificate/key. Then from `Sw/`:

```text
tdf.py build
tdf.py flash
tdf.py monitor
```

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials, Empa Electronics</sub></p>
