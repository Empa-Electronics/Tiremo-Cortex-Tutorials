<p class="sphinxhide" align="right"><a href="../../../../Intelligence/Design_Tutorials/01-Accelerometer_Telemetry/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>Accelerometer Telemetry</h1>
      <sub>Tiremo Cortex Eğitimleri: ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>

> Bu sayfa [İngilizce orijinalinin](../../../../Intelligence/Design_Tutorials/01-Accelerometer_Telemetry/README.md)
> Türkçe çevirisidir. `Sw/` yalnızca İngilizce eğitim klasöründedir.

# Accelerometer Telemetry

***Kart: Tiremo Cortex (ESP32) · Bölüm: Intelligence / Design Tutorials · Zorluk: Orta***

## Bu ikili ne yapar

Özellik eğitimi 05 ile aynı Tiremo Cloud oturumundan sonra (WiFi, SNTP, filoya kayıt, shadow get/apply/report) üretici yazılımı LIS2DE12'yi her **125 ms** örnekler (yazılım 8 Hz, yonga ODR 10 Hz, ±2 g) ve örnek başına bir JSON yayınlar:

```json
{"acc_x":12,"acc_y":-3,"acc_z":1004}
```

Konu: `pub/<product_id>/<device_id>/telemetry` (QoS 0). Değerler milli-g. Data Explorer CSV sütunları `data.acc_x` / `data.acc_y` / `data.acc_z` olur.

Tam yürüyüş bu üretici yazılımından sonra gelir. Pencereli modeli tüketen HMR vakası: [03-HMR_Edge_Deployment](../03-HMR_Edge_Deployment/).

## Derleme ve yükleme

```text
cd Intelligence/Design_Tutorials/01-Accelerometer_Telemetry/Sw
cp wifi_secrets.h.example wifi_secrets.h
cp aws_iot_config.h.example aws_iot_config.h
cp certs/claim_cert.pem.example certs/claim_cert.pem
cp certs/claim_key.pem.example certs/claim_key.pem
```

SSID, ürün kimliği/sır ve gerçek claim sertifika/anahtarını doldurun. `Sw/` içinden:

```text
tdf.py build
tdf.py flash
tdf.py monitor
```

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Eğitimleri, Empa Electronics</sub></p>
