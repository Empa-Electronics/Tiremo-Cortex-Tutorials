<p class="sphinxhide" align="right"><a href="../../Cloud_Messaging/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../images/banner.png" width="100%">
      <h1>Cloud Messaging (MQTT & TLS)</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Cloud Messaging (MQTT & TLS)

> Bu sayfa [İngilizce orijinalinin](../../Cloud_Messaging/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

MQTT'yi uçtan uca çalıştırın: önce TLS'siz herkese açık bir broker, sonra ESP32'nin kendi NVS'ine yerleştirilen karşılıklı TLS (mTLS) kimlik bilgileri, ardından gerçek bir Tiremo Cloud oturumu.

Broker [`iot.tiremo.ai`](https://iot.tiremo.ai)'dir. TLS SNI ve ilk mTLS bağlantısı [TLS SNI](https://docs.tiremo.ai/device-provisioning/tls-sni) ve [ilk bağlantı](https://docs.tiremo.ai/device-provisioning/third-phase) sayfalarında anlatılır. Filo provizyonu daha sonra [Tiremo Cloud](../Tiremo_Cloud/) bölümünde gelir; bkz. [cihaz provizyonu](https://docs.tiremo.ai/device-provisioning/getting-started).

- [**Feature Tutorials**](./Feature_Tutorials/) her biri tek bir kavramı ele alır: tek bir sürücü, tek bir AT komut dizisi, tek bir bulut temel işlemi.

<p align="center"><img alt="" src="../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Eğitimleri — Empa Electronics</sub></p>
