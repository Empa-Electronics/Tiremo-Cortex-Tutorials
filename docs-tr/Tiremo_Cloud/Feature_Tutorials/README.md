<p class="sphinxhide" align="right"><a href="../../../Tiremo_Cloud/Feature_Tutorials/README.md">English</a> | <a>Türkçe</a></p>
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

> Bu sayfa [İngilizce orijinalinin](../../../Tiremo_Cloud/Feature_Tutorials/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

Feature eğitimlerinin her biri tek bir kavramı ele alır: tek bir sürücü, tek bir AT komut dizisi, tek bir bulut temel işlemi. Sırayla ilerleyin; her eğitim ön koşulunu belirtir. İlgili platform sayfalarına [Tiremo Cloud](../) bölüm sayfasından ve [docs.tiremo.ai](https://docs.tiremo.ai) adresinden ulaşılır.

 <table style="width:100%">
 <tr>
 <td width="35%" align="center"><b>Eğitim</b>
 <td width="65%" align="center"><b>Açıklama</b>
 </tr>
 <tr>
 <td align="center"><a href="./01-Device_Identity_From_MAC/">Device Identity from MAC</a></td>
 <td>ESP32'nin station MAC'inden kalıcı bir deviceId / thingName türetmek.</td>
 </tr>
 <tr>
 <td align="center"><a href="./02-Minimal_JSON_Helpers/">Minimal JSON Helpers</a></td>
 <td>Tam bir ayrıştırıcı kütüphanesi olmadan shadow belgelerini işlemeye yetecek kadar JSON arama / yazma yardımcısı yazmak.</td>
 </tr>
 <tr>
 <td align="center"><a href="./03-Fleet_Provisioning_By_Claim/">Fleet Provisioning by Claim Certificate</a></td>
 <td>Filo provizyonunu uygulamak: CreateKeysAndCertificate → RegisterThing → verilen sertifikayı saklamak.</td>
 </tr>
 <tr>
 <td align="center"><a href="./04-Shadow_Get_Apply_Report/">Shadow Get + Apply + Report</a></td>
 <td>Açılışta shadow'u almak, desired durumu yerelde uygulamak ve Tiremo Cloud delta'yı temizlesin diye reported olarak geri bildirmek.</td>
 </tr>
 <tr>
 <td align="center"><a href="./05-Live_Shadow_Delta_Handling/">Live Shadow Delta Handling</a></td>
 <td>shadow/update/delta'ya abone olmak ve bir değişikliği yeniden başlatmadan, canlı uygulamak.</td>
 </tr>
 <tr>
 <td align="center"><a href="./06-Publishing_Alarms_And_Telemetry/">Publishing Alarms and Telemetry</a></td>
 <td>Sensör telemetrisini ve cihazda oluşan alarmları, shadow kontrol döngüsünden ayrı MQTT topic'lerinde yayınlamak.</td>
 </tr>
 <tr>
 <td align="center"><a href="./07-Tiremo_Jobs/">Tiremo Jobs</a></td>
 <td>MQTT üzerinden bir health-check işini kabul etmek (`notify` → `start-next` → `update SUCCEEDED`). OTA biçimli işler REJECTED olur.</td>
 </tr>
 <tr>
 <td align="center"><a href="./08-OTA_Firmware_Update/">OTA Firmware Update</a></td>
 <td>Bir Firmware-Update işini kabul etmek, yeni imajı ham bir TCP/SSL tüneli üzerinden indirmek, doğrulamak ve A34G43x'in A/B kod-flash bankalarında ona geçmek.</td>
 </tr>
 </table>

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Eğitimleri — Empa Electronics</sub></p>
