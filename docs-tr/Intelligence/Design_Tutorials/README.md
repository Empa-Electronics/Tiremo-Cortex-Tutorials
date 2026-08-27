<p class="sphinxhide" align="right"><a href="../../../Intelligence/Design_Tutorials/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Intelligence Eğitimleri</h1>
      <sub>Tiremo Cortex Eğitimleri: ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>

> Bu sayfa [İngilizce orijinalinin](../../../Intelligence/Design_Tutorials/README.md)
> Türkçe çevirisidir. `Datasets/` ve `Tiremo_AI/` yalnızca İngilizce
> eğitim klasöründedir.

# Design Tutorials

Design tutorials, Feature Tutorials'taki Intelligence nesnelerini, gerçekte gönderilene daha yakın, çalışan alt sistemlerde birleştirir. Sırayla yükleyin: canlı `acc_x`/`acc_y`/`acc_z` telemetrisi, yerel 8 örnekli pencere, ardından aynı pencerede HMR Edge paketi.

 <table style="width:100%">
 <tr>
 <td width="35%" align="center"><b>Eğitim</b>
 <td width="65%" align="center"><b>Açıklama</b>
 </tr>
 <tr>
 <td align="center"><a href="./01-Accelerometer_Telemetry/">Accelerometer Telemetry</a></td>
 <td>LIS2DE12 yazılım 8 Hz; örnek başına bir MQTT JSON <code>{"acc_x","acc_y","acc_z"}</code>, konu <code>…/telemetry</code>.</td>
 </tr>
 <tr>
 <td align="center"><a href="./02-Accelerometer_Windowing/">Accelerometer Windowing</a></td>
 <td>Aynı sensör ve tempo; yuvarlanan 8×3 milli-g tampon UART'a dökülür. Bulut yok, <code>Tiremo_AI</code> yok.</td>
 </tr>
 <tr>
 <td align="center"><a href="./03-HMR_Edge_Deployment/">HMR Gesture Classifier on the Edge</a></td>
 <td>3 eksenli HMR yakalamasını 128 Hz'den 8 Hz ve 4 Hz'e götürün, Intelligence'da beş sınıflı bir sınıflandırıcı eğitin ve üretilen <code>Tiremo_AI/</code> paketini Cortex'te çalıştırın (<code>Sw/</code> çıkarımı MQTT ile yayınlar).</td>
 </tr>
 </table>

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Eğitimleri, Empa Electronics</sub></p>
