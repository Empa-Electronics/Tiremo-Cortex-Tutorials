<p class="sphinxhide" align="right"><a href="../../../../Tiremo_Cloud/Feature_Tutorials/07-Tiremo_Jobs/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>Tiremo Jobs</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Tiremo Jobs

***Kart: Tiremo Cortex (ESP32) · Bölüm: Tiremo Cloud / Feature Tutorials · Zorluk: Zor***

> Bu sayfa [İngilizce orijinalinin](../../../../Tiremo_Cloud/Feature_Tutorials/07-Tiremo_Jobs/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Tiremo Jobs, bir Thing üzerine iş kuyruğa almanın yoludur: JSON bir
`$aws/things/<thingName>/jobs/...` konusunda gelir, cihaz `IN_PROGRESS` /
`SUCCEEDED` / `FAILED` bildirir, sıradaki iş sunulur. Shadow'dan farklı
olarak iş, duran bir desired/reported döngüsü değil, durumu olan tek
seferlik bir yürütmedir.

Bu eğitim **henüz uygulanmadı**. Klasörler ve bu README iskelettir;
üretici yazılımı ve adım adım anlatım sonra gelecek.

## Neler Öğreneceksiniz

- Jobs MQTT konu ailesi (`notify-next`, `start-next`, `update`)
- Bir işin başka bir shadow delta gibi ele alınmaması
- Tiremo UInun bitmiş bir iş göstermesi için yürütme durumunun raporlanması

## Önkoşullar

- [05-Live_Shadow_Delta_Handling](../../../../Tiremo_Cloud/Feature_Tutorials/05-Live_Shadow_Delta_Handling/)
- [06-Publishing_Alarms_And_Telemetry](../06-Publishing_Alarms_And_Telemetry/) önerilir
- [Getting Started](../../../Getting_Started/) ile araç zinciri

## Platform belgelendirmesi

- [Tiremo Jobs'a giriş](https://docs.tiremo.ai/developer-guide/jobs/jobs-introduction)
- [İş şablonları](https://docs.tiremo.ai/developer-guide/jobs/jobs-phase-1-definition)
- [İş oluşturma](https://docs.tiremo.ai/developer-guide/jobs/jobs-phase-2-creation)
- [Yürütme izleme](https://docs.tiremo.ai/developer-guide/jobs/jobs-phase-3-monitoring)

Jobs MQTT ile klasik shadow konularını karıştırmayın —
[cihaz entegrasyonu](https://docs.tiremo.ai/device-state-controller/device-integration).

## Çözüm

Bu eğitim için henüz bir çözüm yazılmadı.

## Adım Taslağı

1. `jobs/notify-next` abone ol (açılışta kuyruk doluysa start-next)
2. Bir iş belgesini kabul et, işlemi günlükle, sınırlı yerel eylemi çalıştır
3. `jobs/<jobId>/update` ile `SUCCEEDED` veya `FAILED` yayınla

## Bitti Sayılınca

Tiremo UI'da oluşturulan bir iş kartta bir kez kabul edilir, yürütülür
ve konsolda başarılı işaretlenir; UART eşleşir.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Eğitimleri — Empa Electronics</sub></p>
