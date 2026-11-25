<p class="sphinxhide" align="right"><a href="../../Tiremo_Cloud/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../images/banner.png" width="100%">
      <h1>Tiremo Cloud</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Tiremo Cloud

> Bu sayfa [İngilizce orijinalinin](../../Tiremo_Cloud/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

Tiremo Cortex, [`iot.tiremo.ai`](https://iot.tiremo.ai) adresindeki **Tiremo
Cloud**'a katılmak için tasarlanmıştır. Operatörler Thing'leri incelemek,
shadow desired durumunu düzenlemek ve iş kuyruğa almak için **Tiremo UI**'ı
kullanır. Claim sertifikaları, ürün tanımlayıcısı ve ürün gizli anahtarı
UI'dan gelir; bkz. [Tiremo UI'dan kimlik bilgileri](./Feature_Tutorials/03-Fleet_Provisioning_By_Claim/#tiremo-uidan-kimlik-bilgileri)
(ekran görüntüleri [`figures/`](./figures/) içinde).

Platform dökümanı (bu firmware deposu değil)
[docs.tiremo.ai](https://docs.tiremo.ai) adresindedir:

| Cortex eğitimi | Platform dökümanı |
|---|---|
| [03 Filo provizyonu](./Feature_Tutorials/03-Fleet_Provisioning_By_Claim/) | [Cihaz provizyonu](https://docs.tiremo.ai/device-provisioning/getting-started), [Phase 1](https://docs.tiremo.ai/device-provisioning/first-phase) (bootstrap sertifikası, ürün tanımlayıcısı, gizli anahtar), [Phase 2](https://docs.tiremo.ai/device-provisioning/second-phase), [TLS SNI](https://docs.tiremo.ai/device-provisioning/tls-sni), [ilk bağlantı](https://docs.tiremo.ai/device-provisioning/third-phase) |
| [04](./Feature_Tutorials/04-Shadow_Get_Apply_Report/) / [05](./Feature_Tutorials/05-Live_Shadow_Delta_Handling/) shadow | [Device State Controller](https://docs.tiremo.ai/device-state-controller/introduction), [firmware entegrasyonu](https://docs.tiremo.ai/device-state-controller/device-integration) |
| [06 Telemetri ve alarmlar](./Feature_Tutorials/06-Publishing_Alarms_And_Telemetry/) | [Phase 3 yayın topic'leri](https://docs.tiremo.ai/device-provisioning/third-phase), [mimari ve veri akışı](https://docs.tiremo.ai/introduction/phase-1), [düşük hızlı telemetri](https://docs.tiremo.ai/use-cases/scenario-low-rate-telemetry) |
| [07 Tiremo Jobs](./Feature_Tutorials/07-Tiremo_Jobs/) | [Jobs'a giriş](https://docs.tiremo.ai/developer-guide/jobs/jobs-introduction), [iş şablonları](https://docs.tiremo.ai/developer-guide/jobs/jobs-phase-1-definition) |
| [08 OTA Firmware Update](./Feature_Tutorials/08-OTA_Firmware_Update/) | [Jobs'a giriş](https://docs.tiremo.ai/developer-guide/jobs/jobs-introduction), [iş şablonları](https://docs.tiremo.ai/developer-guide/jobs/jobs-phase-1-definition) |

Ayrıca: [Developer Guide](https://docs.tiremo.ai/developer-guide), [PoC rehberi](https://docs.tiremo.ai/poc-guide), [platform SSS](https://docs.tiremo.ai/faq), [ürün sözlüğü](https://docs.tiremo.ai/product-dictionary/).

Bu bölüm cihaz kimliğini, bu firmware'in tam bir ayrıştırıcı kütüphanesi
yerine kullandığı küçük JSON yardımcılarını, claim sertifikasıyla filo
provizyonunu, cihaz shadow kontrol döngüsünü, telemetri ve alarmları,
Tiremo Jobs'u ve OTA firmware güncellemelerini kapsar.

- [**Feature Tutorials**](./Feature_Tutorials/) her biri tek bir kavramı ele alır: tek bir sürücü, tek bir AT komut dizisi, tek bir bulut temel işlemi.

- [**Design Tutorials**](./Design_Tutorials/) birkaç feature eğitimini çalışan tek bir alt sisteme birleştirir.

<p align="center"><img alt="" src="../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Eğitimleri — Empa Electronics</sub></p>
