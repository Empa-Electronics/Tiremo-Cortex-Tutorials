<p class="sphinxhide" align="right"><a href="../../../Cloud_Messaging/Feature_Tutorials/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Feature Tutorials</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Feature Tutorials

> Bu sayfa [İngilizce orijinalinin](../../../Cloud_Messaging/Feature_Tutorials/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

Feature eğitimlerinin her biri tek bir kavramı ele alır: tek bir sürücü, tek bir AT komut dizisi, tek bir bulut temel işlemi. Sırayla ilerleyin; her eğitim ön koşulunu belirtir. Eğitim 03 [`iot.tiremo.ai`](https://iot.tiremo.ai) ile konuşur; bkz. [TLS SNI](https://docs.tiremo.ai/device-provisioning/tls-sni) ve [ilk bağlantı](https://docs.tiremo.ai/device-provisioning/third-phase).

 <table style="width:100%">
 <tr>
 <td width="35%" align="center"><b>Eğitim</b>
 <td width="65%" align="center"><b>Açıklama</b>
 </tr>
 <tr>
 <td align="center"><a href="./01-MQTT_To_Public_Broker/">MQTT to a Public Broker</a></td>
 <td>Herkese açık bir test broker'ına yayın yapmak ve abone olmak; henüz TLS yok.</td>
 </tr>
 <tr>
 <td align="center"><a href="./02-mTLS_Credentials_In_Module_NVS/">mTLS Credentials in Module NVS</a></td>
 <td>Root CA / istemci sertifikası / istemci anahtarını ESP32'nin SYSMFG yuvalarına yazmak.</td>
 </tr>
 <tr>
 <td align="center"><a href="./03-MQTT_mTLS_To_AWS_IoT/">MQTT mTLS to Tiremo Cloud</a></td>
 <td>Elle verilmiş bir cihaz sertifikasıyla Tiremo Cloud IoT uç noktasına bağlanmak; henüz filo provizyonu yok.</td>
 </tr>
 </table>

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Eğitimleri — Empa Electronics</sub></p>
