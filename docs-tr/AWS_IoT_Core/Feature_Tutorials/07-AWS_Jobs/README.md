<p class="sphinxhide" align="right"><a href="../../../../AWS_IoT_Core/Feature_Tutorials/07-AWS_Jobs/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>AWS Jobs</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + AWS IoT üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# AWS Jobs

***Kart: Tiremo Cortex (ESP32) · Bölüm: AWS IoT Core / Feature Tutorials · Zorluk: Zor***

> Bu sayfa [İngilizce orijinalinin](../../../../AWS_IoT_Core/Feature_Tutorials/07-AWS_Jobs/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

AWS IoT Jobs, bir Thing üzerine iş kuyruğa almanın yoludur: JSON bir
`$aws/things/<thingName>/jobs/...` konusunda gelir, cihaz `IN_PROGRESS` /
`SUCCEEDED` / `FAILED` bildirir, sıradaki iş sunulur. Shadow'dan farklı
olarak iş, duran bir desired/reported döngüsü değil, durumu olan tek
seferlik bir yürütmedir.

Bu eğitim **henüz uygulanmadı**. Klasörler ve bu README iskelettir;
üretici yazılımı ve adım adım anlatım sonra gelecek.

## Neler Öğreneceksiniz

- Jobs MQTT konu ailesi (`notify-next`, `start-next`, `update`)
- Bir işin başka bir shadow delta gibi ele alınmaması
- AWS konsolunun bitmiş bir iş göstermesi için yürütme durumunun raporlanması

## Önkoşullar

- [05-Live_Shadow_Delta_Handling](../../../../AWS_IoT_Core/Feature_Tutorials/05-Live_Shadow_Delta_Handling/)
- [06-Publishing_Alarms_And_Telemetry](../06-Publishing_Alarms_And_Telemetry/) önerilir
- [Getting Started](../../../Getting_Started/) ile araç zinciri

## Çözüm

Bu eğitim için henüz bir çözüm yazılmadı.

## Adım Taslağı

1. `jobs/notify-next` abone ol (açılışta kuyruk doluysa start-next)
2. Bir iş belgesini kabul et, işlemi günlükle, sınırlı yerel eylemi çalıştır
3. `jobs/<jobId>/update` ile `SUCCEEDED` veya `FAILED` yayınla

## Bitti Sayılınca

AWS konsolunda oluşturulan bir iş kartta bir kez kabul edilir, yürütülür
ve konsolda başarılı işaretlenir; UART eşleşir.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Eğitimleri — Empa Electronics</sub></p>
