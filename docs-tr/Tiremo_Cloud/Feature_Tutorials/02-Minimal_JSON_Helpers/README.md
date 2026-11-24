<p class="sphinxhide" align="right"><a href="../../../../Tiremo_Cloud/Feature_Tutorials/02-Minimal_JSON_Helpers/README.md">English</a> | <a>Türkçe</a></p>
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
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Minimal JSON Helpers

***Kart: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Bölüm: Tiremo Cloud / Feature Tutorials · Zorluk: Orta***

> Bu sayfa [İngilizce orijinalinin](../../../../Tiremo_Cloud/Feature_Tutorials/02-Minimal_JSON_Helpers/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Shadow belgeleri, fleet provisioning yanıtları ve Jobs içerikleri JSON'dur. Genel amaçlı bir ayrıştırıcı (cJSON, jsmn) işe yarardı, ama bu MCU üretici yazılımının geri kalanının ihtiyaç duymadığı heap kullanımını, özyinelemeyi ve bir bağımlılığı da getirirdi. Bu eğitim, sonraki Tiremo Cloud eğitimlerinin gerçekten çağırdığı dört bulma (find) yardımcısını ve yazıcıyı kurar — yapısı bilinen, boyutu sınırlı belgeler için.

Bu `Sw/` ESP32 ile veya Tiremo Cloud ile **konuşmaz**. Testler flash'taki sabit bir örnek shadow üzerinde çalışır ve UART0'a geçti/kaldı yazar.

## Neler Öğreneceksiniz

- Şemayı zaten biliyorsanız neden yalnızca o anki derinliği taramanın yeterli olduğu
- Anahtarları yalnızca size verilen nesnenin *kardeşleri* olarak eşleştirmek (böylece `reporting` içindeki `enabled` kökte görünmez)
- String'leri atlamak, böylece `"note"` içindeki bir iki nokta veya süslü parantezin yapı gibi görünmemesi
- İç içe bir `state.reported` nesnesi üretebilen ve sonra aynı Find* yardımcılarıyla sorunsuz ayrıştırılabilen bir `JsonWriter`

## Ön Koşullar

- Kavramsal olarak [01-Device_Identity_From_MAC](../01-Device_Identity_From_MAC/README.md) (Tiremo Cloud bölümü başladı); bu `Sw/` yalnızca [Getting Started](../../../Getting_Started/) bölümündeki debug UART'ını kullanır
- [Getting Started](../../../Getting_Started/) bölümündeki kurulum betikleri çalıştırılmış olmalı, böylece `tdf` kullanılabilir
- Logu izlemek için: `tdf monitor` veya herhangi bir seri terminal (Tera Term, PuTTY, screen, minicom, …), kartın USB seri portunda 115200 8N1
- Bilgisayarınıza USB kablosuyla bağlı bir Tiremo Cortex kartı (ABOV A34G43A MCU — bkz. [Donanım Genel Bakışı](../../../Hardware.md))

Fleet provisioning [03](../03-Fleet_Provisioning_By_Claim/README.md)'tür. `CreateKeysAndCertificate` yanıtları da JSON olduğu için bu eğitimden sonra gelir.

## Platform dökümanı

Bu yardımcıların ayrıştırdığı belgeler platform tarafında tanımlanır:
[fleet provisioning](https://docs.tiremo.ai/device-provisioning/second-phase),
[Device State Controller](https://docs.tiremo.ai/device-state-controller/introduction)
(klasik Device Shadow) ve
[Tiremo Jobs](https://docs.tiremo.ai/developer-guide/jobs/jobs-introduction).

## Bu derinlikte taramayı anlamak

`TIREMO_Json_FindObject(doc, len, "state", ...)` torunlara değil, `doc`'un üyelerine bakar. İç içe alanlara, dönen dilimi tekrar vererek ulaşılır:

```c
(void)TIREMO_Json_FindObject(doc, len, "state", &pState, &stateLen);
(void)TIREMO_Json_FindObject(pState, stateLen, "desired", &pDesired, &desiredLen);
(void)TIREMO_Json_FindObject(pDesired, desiredLen, "reporting", &pRep, &repLen);
(void)TIREMO_Json_FindBool(pRep, repLen, "enabled", &enabled);
```

API'nin tamamı bu. JSON Pointer yok, bellek ayırma yok, DOM yok. Kök belgede `FindBool("enabled")`, kelime daha derinde geçse bile **başarısız olur**; test bunu `root has no sibling key enabled` olarak loglar.

String'ler tek parça değer olarak atlanır. Örnekte `"note":"colon:not-a-key"` vardır; `:` için `strstr` kullanan saf bir yaklaşım burada çoktan yanılmış olurdu.

Yazıcı, bir raporun diğer yarısıdır: yalnızca değişen anahtarı değil, uygulanan desired nesnesinin *tamamını* (iç içe `reporting` ve `led` dahil) geri yansıtın. Eğitim 04, bir shadow delta'sını temizlemek için buna ihtiyaç duyacak.

## Çözüm

Depo kök dizininden:

```sh
cd Tiremo_Cloud/Feature_Tutorials/02-Minimal_JSON_Helpers/Sw
tdf build
tdf flash
tdf monitor
```

(`tdf`'nin ne olduğu için [Getting_Started/01-Hello_Blink'in Çözüm bölümüne](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) bakın. macOS/Linux'ta `tdf.py` de takma ad olarak çalışır; Windows'ta `tdf` kullanın. `tdf monitor` çalıştırmadan önce kartın seri portundaki diğer terminalleri kapatın — port aynı anda yalnızca bir program tarafından açılabilir.)

**Gerçek donanımda doğrulandı:** `tdf build` macOS ve Windows 11'de hatasız derler ve bağlar (`.text` yaklaşık 16.3 KB; tam boyut araç zinciri sürümüne göre biraz değişir ve bağlayıcı, [01-Hello_Blink'in Çözüm bölümünde](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) açıklanan aynı beklenen, zararsız `_close` / `_lseek` / `_read` / `_write is not implemented` uyarılarını yazdırır). `tdf flash`, bağlı gerçek bir Tiremo Cortex kartında SWD üzerinden tam bir programlama ve doğrulama döngüsünü tamamladı (`** Verified OK **`) ve UART logu şunu gösterdi:

```text
[INFO] Minimal_JSON_Helpers: boot
[INFO] json: FindObject state
[INFO] json: FindObject desired
[INFO] json: FindObject reporting
[INFO] json: FindBool reporting.enabled == true
[INFO] json: FindUint reporting.intervalS == 60
[INFO] json: FindObject led
[INFO] json: FindBool led.on == false
[INFO] json: FindString note (colon inside string)
[INFO] json: FindUint version == 7
[INFO] json: root has no sibling key enabled
[INFO] json: writer finish
[INFO] json: wrote {"state":{"reported":{"reporting":{"enabled":true,"intervalS":60},"led":{"on":false},"note":"colon:not-a-key"}}}
[INFO] json: re-parse state
[INFO] json: re-parse reported
[INFO] json: re-parse reporting
[INFO] json: echo enabled == true
[INFO] json: echo intervalS == 60
[INFO] json: round-trip ok
[INFO] json: still ok
[INFO] json: still ok
```

Her kontrol bir satır loglar. Başarısız bir kontrol bunun yerine `[ERROR] json: FAIL <kontrol>` loglar ve program `round-trip ok` satırına ulaşmadan `json: one or more checks failed` ile durur. Ardından `json: still ok` her 5 sn'de bir tekrarlanır.

## Adım Adım İnceleme

1. [`tiremo_json.c`](../../../../Tiremo_Cloud/Feature_Tutorials/02-Minimal_JSON_Helpers/Sw/Common/Src/tiremo_json.c) — `FindObject` / `FindBool` / `FindUint` / `FindString` ve `TIREMO_JsonWriter_*`
2. [`prv_user_code.c`](../../../../Tiremo_Cloud/Feature_Tutorials/02-Minimal_JSON_Helpers/Sw/Src/prv_user_code.c) — örnek shadow, kardeş olmayan anahtar için negatif test, reported yazma, yeniden ayrıştırma

## Tamamlanma Ölçütü

- Dört Find* yardımcısı, örnek shadow belgesindeki iç içe alanları çıkarır.
- Kök belgede `enabled` aranması başarısız olur, çünkü orada kardeş anahtar değildir.
- Yazıcının ürettiği `state.reported` belgesi aynı yardımcılarla yeniden ayrıştırılır ve log `json: round-trip ok` ile biter.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Eğitimleri — Empa Electronics</sub></p>
