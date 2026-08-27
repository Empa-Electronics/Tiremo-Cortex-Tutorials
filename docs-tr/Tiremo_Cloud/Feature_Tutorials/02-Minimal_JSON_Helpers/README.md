<p class="sphinxhide" align="right"><a href="../../../../AWS_IoT_Core/Feature_Tutorials/02-Minimal_JSON_Helpers/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>Minimal JSON Helpers</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + AWS IoT üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Minimal JSON Helpers

***Kart: Tiremo Cortex (ESP32) · Bölüm: AWS IoT Core / Feature Tutorials · Zorluk: Orta***

> Bu sayfa [İngilizce orijinalinin](../../../../AWS_IoT_Core/Feature_Tutorials/02-Minimal_JSON_Helpers/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Shadow belgeleri, filo kurulumu yanıtları ve Jobs yükleri JSON'dur. Genel amaçlı bir ayrıştırıcı (cJSON, jsmn) işe yarar ve bu MCU firmware'inin ihtiyaç duymadığı yığın, özyineleme ve bağımlılık da getirir. Bu eğitim, sonraki AWS IoT Core eğitimlerinin gerçekten çağırdığı dört find yardımcısını ve yazıcıyı kurar — bilinen, sınırlı belge şekillerine göre kapsamlanır.

Bu `Sw/` ESP32 veya AWS ile **konuşmaz**. Testler flash'taki sabit bir örnek shadow üzerinde çalışır ve UART0'da geçti/kaldı basar.

## Neler Öğreneceksiniz

- Şemayı zaten biliyorsanız mevcut derinlik taramasının yeterli olduğu
- Anahtarların yalnızca elinizdeki nesnenin *kardeşleri* olarak eşleşmesi (`reporting` içindeki `enabled` kökte görünmez)
- `"note"` içindeki iki nokta veya süslü parantezin yapı sanılmaması için string atlama
- İç içe `state.reported` basabilen ve aynı Find* yardımcılarıyla yeniden ayrıştırılabilen bir `JsonWriter`

## Önkoşullar

- Kavramsal olarak [01-Device_Identity_From_MAC](../../../../AWS_IoT_Core/Feature_Tutorials/01-Device_Identity_From_MAC/); bu `Sw/` yalnızca [Getting Started](../../../../Getting_Started/) debug UART'ını ister
- Toolchain ve flaş Getting Started'a göre

Filo kurulumu [03](../../../../AWS_IoT_Core/Feature_Tutorials/03-Fleet_Provisioning_By_Claim/)'tür. `CreateKeysAndCertificate` yanıtları da JSON olduğu için bundan sonra gelir.

## Bu-derinlik taramasını anlamak

`TIREMO_Json_FindObject(doc, len, "state", ...)` torunlara değil `doc`'un üyelerine bakar. İç içe alanlar dilimi geri besleyerek ulaşılır. Kök belgede `FindBool("enabled")` kelime daha derinde geçse bile **başarısız olur** — donanımda doğrulandı (`root has no sibling key enabled`).

## Çözüm

```sh
cd Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

**Doğrulandı:** `.text` 16156, flaş OK. UART:

```text
[INFO] Minimal_JSON_Helpers: boot
[INFO] json: FindBool reporting.enabled == true
[INFO] json: FindUint reporting.intervalS == 60
[INFO] json: FindString note (colon inside string)
[INFO] json: root has no sibling key enabled
[INFO] json: wrote {"state":{"reported":{"reporting":{"enabled":true,"intervalS":60},"led":{"on":false},"note":"colon:not-a-key"}}}
[INFO] json: echo enabled == true
[INFO] json: round-trip ok
```

## Bitti sayılır

Dört Find* iç içe alanları çıkarır, eksik kardeş anahtar fail olur, yazıcı aynı yardımcıların kabul ettiği JSON üretir. Donanımda yukarıdaki gibi doğrulandı.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
