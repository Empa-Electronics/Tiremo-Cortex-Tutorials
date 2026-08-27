<p class="sphinxhide" align="right"><a href="../../../../Sensors_And_Storage/Feature_Tutorials/03-LIS2DE12_Accelerometer/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>LIS2DE12 Accelerometer</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


> Bu sayfa [İngilizce orijinalinin](../../../../Sensors_And_Storage/Feature_Tutorials/03-LIS2DE12_Accelerometer/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

# LIS2DE12 Accelerometer

***Kart: Tiremo Cortex (ESP32) · Bölüm: Sensors & Local Storage / Feature Tutorials · Zorluk: Orta***

## Giriş

SHT40'ın aksine LIS2DE12'nin gerçek bir yapılandırma yüzeyi vardır — tam ölçek ve çıkış veri hızı sonraki düşme/şok/titreşim alarmları için önemlidir, bu yüzden sürücüyü ilk günden `Configure(scale_g, data_rate_hz)` alacak şekilde yazın.

## Neler Öğreneceksiniz

- Register haritası: `WHO_AM_I`, `CTRL_REG1` (ODR), `CTRL_REG4` (ölçek), `OUT_X/Y/Z_H`
- Neden `WriteRead()` (tekrarlanmış START) gerektiği
- Sol hizalı 8-bit örnekleri seçilen hassasiyetle mg'ye dönüştürmek

## Ön Koşullar

- [02-SHT40_Temperature_Humidity](../../../../Sensors_And_Storage/Feature_Tutorials/02-SHT40_Temperature_Humidity/README.md)
- `tdf.py monitor`

## Çözüm

**Bağlı kartta doğrulandı:** derleme temiz (`.text` 15708), flaş **Verified OK**, `WHO_AM_I=0x33`, düz dururken `Z=1036 mg`. Eksen eksen eğme henüz doğrulanmadı.

```text
[INFO] LIS2DE12: WHO_AM_I=0x33  FS=2g  ODR=100 Hz
[INFO] LIS2DE12: X=4 mg  Y=-28 mg  Z=1036 mg
```


<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
