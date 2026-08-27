<p class="sphinxhide" align="right"><a href="../../../../Tiremo_Cloud/Feature_Tutorials/01-Device_Identity_From_MAC/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>Device Identity from MAC</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Device Identity from MAC

***Kart: Tiremo Cortex (ESP32) · Bölüm: Tiremo Cloud / Feature Tutorials · Zorluk: Orta***

> Bu sayfa [İngilizce orijinalinin](../../../../Tiremo_Cloud/Feature_Tutorials/01-Device_Identity_From_MAC/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Filo kurulumu ve shadow konuları kararlı, cihaz başına bir isim ister. `"tiremo-cortex-01"` bir tezgah cihazında işe yarar; aynı firmware ikinci karta yazıldığı anda çöker. ESP32'nin zaten benzersiz 48 bitlik station MAC'i var; bu eğitim onu `AT+CIPSTAMAC?` ile okur ve sonraki eğitimlerin Thing adı / MQTT istemci kimliği olarak kullanacağı 12 haneli küçük harf hex `deviceId`'ye çevirir.

Bu `Sw/` **WiFi'ye katılmaz ve Tiremo Cloud ile konuşmaz.** Kimlik bir donanım özelliğidir, bir bulut oturumu değil.

## Neler Öğreneceksiniz

- `AT+CIPSTAMAC?` ve beklemenin `"+CIPSTAMAC:"` değil `"OK"` eşleştirmesi gerektiği
- Altı MAC baytını ayırıcısız 12 hex haneli küçük harf kimliğe biçimlemek
- Kimliğin kaynak koda gömülmek yerine modülden okunması

## Önkoşullar

- [ESP32 Connectivity / 01-AT_Command_Transport](../../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/) — bu `Sw/` o taşıma katmanı artı `GetHwEui()`
- [Getting Started](../../../../Getting_Started/) ile toolchain ve flaş
- Kimliği izlemek için `tdf.py monitor`

Cloud Messaging müfredatta önceki *bölümdür*. Bu eğitim için MQTT broker gerekmez.

## Platform belgelendirmesi

Bu eğitimin ürettiği ad, sonraki eğitimlerde
[filo kurulumu](https://docs.tiremo.ai/device-provisioning/getting-started)
sırasında `deviceIdentifier` ve shadow konularında Thing adı olarak gider
([Device State Controller](https://docs.tiremo.ai/device-state-controller/introduction)).

## Station MAC ile uydurma seri numarası

Modülün iki MAC'i vardır. **Station** (`AT+CIPSTAMAC?`) WiFi istemcisi olarak kullandığı adrestir. SoftAP (`AT+CIPAPMAC?`) genelde station artı birdir. Filo kimliği station MAC'i izlemelidir: AP'nin zaten gördüğü adrestir ve sonra BluFi / SoftAP açsanız da değişmez.

Tipik yanıt:

```text
+CIPSTAMAC:"30:ed:a0:80:63:24"
OK
```

`TIREMO_ESP32_GetHwEui()` sonlandırıcı `"OK"`'i bekler, sonra tırnaklı, iki nokta üst üste ayrılmış hex'i ayrıştırır. `"+CIPSTAMAC:"` eşleştirmek on bir karakter gelir gelmez — bayt bayt — döner ve tırnaklı adres hâlâ yoldayken boş bırakır. WiFi eğitimlerindeki `AT+CWJAP?` tuzağı ile aynıdır.

Altı bayt kanonik kimliğe dönüşür:

| Biçim | Örnek | Kim kullanır |
|-------|--------|--------------|
| İki noktalı MAC (yalnız log) | `30:ed:a0:80:63:24` | İnsanlar |
| `deviceId` | `30eda0806324` | Sonra Thing adı / MQTT client ID |

Küçük harf, `0x` yok, iki nokta yok. Thing adları büyük/küçük harfe duyarlıdır; burada tek bir yazım seçmek, sonraki şablonların ve shadow konularının anlaşmazlığa düşmemesini sağlar.

`TIREMO_Identity_FromEui()` [`tiremo_identity.c`](../../../../Tiremo_Cloud/Feature_Tutorials/01-Device_Identity_From_MAC/Sw/Common/Src/tiremo_identity.c) içindedir; eğitim 02 aynı dizeyi kopyalamadan kullanır. Sürücü yalnızca ham bayt döner.

## Çözüm

```sh
cd Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

**Doğrulandı:** `.text` 15060, flaş OK. UART:

```text
[INFO] Device_Identity_From_MAC: boot
[INFO] ESP32: power cycling module...
[INFO] ESP32: power-on boot delay elapsed
[INFO] TIREMO_ESP32_Init: UART2 ready
[INFO] ESP32: command echo disabled (ATE0)
[INFO] identity: station MAC 30:ed:a0:80:63:24
[INFO] identity: deviceId 30eda0806324
[INFO] identity: still 30eda0806324
```

Heartbeat her 5 s `CIPSTAMAC?` tekrar sorgular ve aynı 12 hex haneyi doğrular. İki fiziksel kart arasında benzersizlik insan kontrolüdür: aynı imajı ikinci bir Cortex'e yazın, `deviceId` satırı farklı olmalıdır.

## Adım adım

1. [`tiremo_esp32.c`](../../../../Tiremo_Cloud/Feature_Tutorials/01-Device_Identity_From_MAC/Sw/Drivers/esp32/Src/tiremo_esp32.c) — `GetHwEui()`
2. [`tiremo_identity.c`](../../../../Tiremo_Cloud/Feature_Tutorials/01-Device_Identity_From_MAC/Sw/Common/Src/tiremo_identity.c) — `FromEui` / `FormatMac`
3. [`prv_user_code.c`](../../../../Tiremo_Cloud/Feature_Tutorials/01-Device_Identity_From_MAC/Sw/Src/prv_user_code.c) — boot log + 5 s yeniden okuma

## Bitti sayılır

Bu kartın her açılışında aynı `deviceId` basılır; ikinci bir kart (olduğunda) farklı 12 hex basar. Aynı-boot kararlılığı donanımda **doğrulandı** (`30eda0806324` heartbeat yeniden okumalarında kaldı). İki kartlık benzersizlik ikinci bir ünite ister.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
