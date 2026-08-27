<p class="sphinxhide" align="right"><a href="../../../../Tiremo_Cloud/Design_Tutorials/01-Sensor_Telemetry_And_Shadow_Control/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>Sensor Telemetry and Shadow Control</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Sensor Telemetry and Shadow Control

***Kart: Tiremo Cortex (ESP32) · Bölüm: Tiremo Cloud / Design Tutorials · Zorluk: Zor***

> Bu sayfa [İngilizce orijinalinin](../../../../Tiremo_Cloud/Design_Tutorials/01-Sensor_Telemetry_And_Shadow_Control/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Bu, ürüne benzeyen ilk Tiremo Cloud **demosu**: kart üzerindeki sensörler
örneklenip telemetri olarak yayınlanır; bulut operatörü klasik Device
Shadow desired/delta ile bir çıkışı (LED / eyleyici) açıp kapatır —
sıfırlama yoktur.

[Sensors & Local Storage](../../../../Sensors_And_Storage/) okumalarını
[05 canlı shadow](../../../../Tiremo_Cloud/Feature_Tutorials/05-Live_Shadow_Delta_Handling/)
ve [06 telemetri](../../Feature_Tutorials/06-Publishing_Alarms_And_Telemetry/)
ile birleştirir.

Bu eğitim **henüz uygulanmadı**. Klasörler ve bu README iskelettir;
üretici yazılımı ve adım adım anlatım sonra gelecek.

## Neler Öğreneceksiniz

- Tek hazır döngü: shadow delta yokla, sensör örnekle, telemetri yayınla
- Shadow yalnızca aç/kapa (ve aralık) — örnekler telemetri konusundadır
- Sensör JSON'unun `state.reported`'a doldurulmaması (belge boyutu ve artakalan delta)

## Önkoşullar

- Özellik eğitimleri [01](../../../../Tiremo_Cloud/Feature_Tutorials/01-Device_Identity_From_MAC/)–
  [05](../../../../Tiremo_Cloud/Feature_Tutorials/05-Live_Shadow_Delta_Handling/)
  (kimlik, kurulum, canlı shadow)
- [06-Publishing_Alarms_And_Telemetry](../../Feature_Tutorials/06-Publishing_Alarms_And_Telemetry/)
- [Sensors & Local Storage](../../../../Sensors_And_Storage/)
- [Getting Started](../../../Getting_Started/) ile araç zinciri

## Platform belgelendirmesi

- Shadow açık/kapalı: [Device State Controller](https://docs.tiremo.ai/device-state-controller/introduction), [cihaz entegrasyonu](https://docs.tiremo.ai/device-state-controller/device-integration)
- Telemetri: [mimari ve veri akışı](https://docs.tiremo.ai/introduction/phase-1), [düşük hızlı telemetri](https://docs.tiremo.ai/use-cases/scenario-low-rate-telemetry)
- Kimlik ve sertifikalar: [cihaz kurulumu](https://docs.tiremo.ai/device-provisioning/getting-started)

## Çözüm

Bu eğitim için henüz bir çözüm yazılmadı.

## Adım Taslağı

1. Özellik eğitimleri 01–05 ile kurulum / yeniden bağlan
2. Canlı shadow ile `led.on` (aç/kapa) ve `reporting.intervalS` hemen uygulansın
3. O aralıkta çevresel sensörleri örnekle ve telemetri yayınla
4. Tiremo UI: shadow çıkışı değiştirir; MQTT örnekleri görür; delta temizlenir

## Bitti Sayılınca

Konsolda `desired.led.on` çevirmek kart çıkışını sıfırlamadan açıp kapatır;
UART ve bir MQTT aboneliği ayrı konuda periyodik sensör telemetrisi gösterir.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Eğitimleri — Empa Electronics</sub></p>
