<p class="sphinxhide" align="right"><a href="../../../../Sensors_And_Storage/Feature_Tutorials/04-Battery_Voltage_ADC/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>Battery Voltage via ADC</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + AWS IoT üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


> Bu sayfa [İngilizce orijinalinin](../../../../Sensors_And_Storage/Feature_Tutorials/04-Battery_Voltage_ADC/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

# Battery Voltage via ADC

***Kart: Tiremo Cortex (ESP32) · Bölüm: Sensors & Local Storage / Feature Tutorials · Zorluk: Orta***

## Giriş

Ayrılmış bir batarya-sense pini yok — besleme gerilimi dahili VCORE bant aralığının AVDD'ye karşı ölçülmesiyle çıkarılır.

## Çözüm

**Doğrulandı:** `.text` 14436, flaş OK, canlı `VCORE raw=2032  AVDD=2015 mV`. Multimetre ile ±50 mV karşılaştırması bu oturumda yapılmadı.

```text
[INFO] BAT: VCORE raw=2032  AVDD=2015 mV
```


<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
