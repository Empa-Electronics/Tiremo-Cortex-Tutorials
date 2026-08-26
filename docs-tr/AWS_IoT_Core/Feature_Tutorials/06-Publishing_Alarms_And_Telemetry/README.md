<p class="sphinxhide" align="right"><a href="../../../../AWS_IoT_Core/Feature_Tutorials/06-Publishing_Alarms_And_Telemetry/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>Publishing Alarms and Telemetry</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + AWS IoT üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Publishing Alarms and Telemetry

***Kart: Tiremo Cortex (ESP32) · Bölüm: AWS IoT Core / Feature Tutorials · Zorluk: Zor***

> Bu sayfa [İngilizce orijinalinin](../../../../AWS_IoT_Core/Feature_Tutorials/06-Publishing_Alarms_And_Telemetry/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Shadow kontrol düzlemidir. Telemetri ve alarmlar veri düzlemidir: cihaz
örnekleri ve kenarda yükselen olayları AWS'nin yönlendirebileceği MQTT
konularında yayınlar; bu akış shadow belgesine doldurulmaz.

Bu eğitim **henüz uygulanmadı**. Klasörler ve bu README iskelettir;
üretici yazılımı ve adım adım anlatım sonra gelecek.

## Neler Öğreneceksiniz

- Thing adı altında küçük, kararlı bir konu düzeni (`telemetry`, `alarms`)
- Periyodik telemetrinin shadow `reported` yeniden yazımı değil, QoS 0/1 JSON olduğu
- Alarmın `update/delta` fırtınası çıkaran bir shadow alanı değil, ayrı bir yayın olduğu

## Önkoşullar

- [05-Live_Shadow_Delta_Handling](../../../../AWS_IoT_Core/Feature_Tutorials/05-Live_Shadow_Delta_Handling/)
- [02-Minimal_JSON_Helpers](../../../../AWS_IoT_Core/Feature_Tutorials/02-Minimal_JSON_Helpers/)
- [Getting Started](../../../Getting_Started/) ile araç zinciri

## Çözüm

Bu eğitim için henüz bir çözüm yazılmadı.

## Adım Taslağı

1. Eğitim 05 hazır döngüsünden sonra zamanlayıcıyla sıkışık bir telemetri JSON'u yayınla
2. Yerel eşik aşılınca bir alarm konusu yükselt; değer dönünce temizle (histerezis)
3. Shadow trafiğini desired/reported kontrol alanlarıyla sınırla

## Bitti Sayılınca

UART periyodik telemetri yayınlarını ve tek bir alarm yükselt/temizle çiftini
gösterir; artakalan shadow delta fırtınası yoktur.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Eğitimleri — Empa Electronics</sub></p>
