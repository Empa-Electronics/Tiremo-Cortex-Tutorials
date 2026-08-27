<p class="sphinxhide" align="right"><a href="../../../../Sensors_And_Storage/Feature_Tutorials/05-Microphone_RMS_To_dB/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>Microphone: RMS Capture to dB</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


> Bu sayfa [İngilizce orijinalinin](../../../../Sensors_And_Storage/Feature_Tutorials/05-Microphone_RMS_To_dB/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

# Microphone: RMS Capture to dB

***Kart: Tiremo Cortex (ESP32) · Bölüm: Sensors & Local Storage / Feature Tutorials · Zorluk: Zor***

## Giriş

Timer1 tetikli ADC burst, DMA, DC ofset, RMS→dB. `Tiremo_Hal` yerine satıcı `hal_adc` / `hal_timer1` / `hal_pcu`.

## Çözüm

Sürücü referans üretici yazılımından kopyalandı. **Doğrulandı:** `.text` 18316, flaş OK, sessiz odada `rms≈18 dB≈24`, DMA `blocks` artıyor. Mikrofona konuşma bu oturumda doğrulanmadı.

```text
[INFO] MIC: blocks=18078  rms=18  dB=24
```


<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
