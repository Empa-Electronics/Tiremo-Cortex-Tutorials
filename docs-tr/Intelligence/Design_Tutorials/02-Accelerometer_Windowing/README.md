<p class="sphinxhide" align="right"><a href="../../../../Intelligence/Design_Tutorials/02-Accelerometer_Windowing/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>Accelerometer Windowing</h1>
      <sub>Tiremo Cortex Eğitimleri: ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>

> Bu sayfa [İngilizce orijinalinin](../../../../Intelligence/Design_Tutorials/02-Accelerometer_Windowing/README.md)
> Türkçe çevirisidir. `Sw/` yalnızca İngilizce eğitim klasöründedir.

# Accelerometer Windowing

***Kart: Tiremo Cortex (ESP32) · Bölüm: Intelligence / Design Tutorials · Zorluk: Orta***

## Bu ikili ne yapar

Ağ yok. Tasarım eğitimi önyükleme afişinden sonra LIS2DE12 ±2 g / 10 Hz yapılandırılır, her **125 ms** örneklenir ve **8 örnek × 3 eksen** (`float[24]`, milli-g, xyz serpiştirilmiş) yuvarlanan pencere doldurulur. Pencere dolunca 24 değer UART'tan dökülür ve sonraki pencere başlar. [03](../03-HMR_Edge_Deployment/) Edge modelinin beklediği düzen; bu ikili `tiremo_ai_run` çağırmaz.

## Derleme ve yükleme

`Sw/` içinden (WiFi sırrı veya claim PEM yok):

```text
cd Intelligence/Design_Tutorials/02-Accelerometer_Windowing/Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Eğitimleri, Empa Electronics</sub></p>
