<p class="sphinxhide" align="right"><a href="../../../../ESP32_Connectivity/Feature_Tutorials/05-BluFi_Provisioning/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>BluFi BLE Provisioning</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# BluFi BLE Provisioning

***Kart: Tiremo Cortex (ESP32) · Bölüm: ESP32 Connectivity / Feature Tutorials · Zorluk: Zor***

> Bu sayfa [İngilizce orijinalinin](../../../../ESP32_Connectivity/Feature_Tutorials/05-BluFi_Provisioning/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Sabit SSID bir tezgah cihazından öteye ölçeklenmez. Bu eğitimde **`wifi_secrets.h` yok**. EspBluFi kimlik bilgisini BLE üzerinden gönderir; MCU `+SSID:` / `+PASSWORD:` / `WIFI GOT IP` toplar ve flash'a yazar (`BL01`). Sonraki reset reklam vermeden o AP'ye katılır.

Kayıtlı WiFi → BluFi → NTP zinciri [tasarım eğitimidir](../../../../ESP32_Connectivity/Design_Tutorials/01-Boot_Cascade_Saved_WiFi_Then_BluFi/). NTP'yi buraya eklemeyin.

## Neler Öğreneceksiniz

- `AT+BLUFINAME` sonra `AT+BLUFI=1`; `+BLUFICONN`, `+SSID:`, `+PASSWORD:`, `WIFI GOT IP`
- Beklerken `BlufiPoll()`'un UART2'nin **tek** tüketicisi olması
- Aynı döngüde `IsConnected()`'ın kimlik URC'sini yutabilmesi
- Başarıdan sonra `AT+BLUFI=0` ve `CWAUTOCONN=1`

## RX sahipliği

`SendCommand` iletmeden önce halkayı **siler**. `IsConnected()` = `AT+CWSTATE?`. BluFi beklerken bunu yoklamak `+SSID:` / `+PASSWORD:` / `WIFI GOT IP` satırlarını atabilir.

`BlufiPoll` yalnızca bayt **pop** eder, satır kurar, hasat yapısını günceller. TX yok. `AT+BLUFI=0` de bir `SendCommand`'dır — SSID/şifreyi **önce** kopyalayın.

## AT dizisi

Ad `AT+BLUFI=1` **öncesi** set edilmeli, yoksa `BLUFI_DEVICE` yayınlanır. BLE zaten init olmamalı (`AT+BLEINIT=0`); güç döngüsü bunu sağlar.

Şifre flash'a yazılır, loglanmaz.

## Gelmeyen kimlik URC'leri

ESP-AT dokümanı `+SSID:` ve `+PASSWORD:` basıldığını söyler, ama her sürüm basmaz: bazıları yalnızca `+BLUFICONN`, `WIFI CONNECTED` ve `WIFI GOT IP` üretir. Tanınmayan her satır `ESP32: BluFi URC "..."` olarak loglanır; hangi sürümde olduğunuz böyle görülür.

`+PASSWORD:` gelmezse şifre kalıcı olarak kaybolur — `AT+CWJAP?` sadece SSID döner. Bu yüzden `CWAUTOCONN=1`, `AT+BLUFI=0`'dan **önce** gönderilir: modülün NVS kopyası o AP'ye dönmenin tek yolu olduğundan, başka bir şey başarısız olmadan önce açılmalıdır.

`AT+BLUFI=0` ayrıca önce `+BLUFIDISCONN`, sonra `OK` döner. Bir saniyelik timeout erken vazgeçer ve geciken `OK` **sonraki** komut sırasında gelip onun eşleşmesini karşılar; hemen ardından gelen `AT+CWJAP?` `+CWJAP:` satırı olmadan döner ve SSID de kaybolur. Bu nedenle durdurma 5 s bekler ve `GetConnectedSsid()` işaretçi yoksa tekrar dener.

## Persist `BL01`

Eğitim 03 ile aynı sektör 0 / CRC, magic **`BL01`**, `WF01` değil. 03/04'ten kalan join kaydı BluFi kurulumu sayılmaz; ilk flaş her zaman reklam verir. EspBluFi başarısından sonra `BL01` yazılır; sonraki reset reklam olmadan `CWJAP` eder.

## Çözüm

```sh
cd Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

**Doğrulandı:** `.text` 21156, flaş OK. `AT+BLUFI=1` kabul edildi:

```text
[INFO] BluFi_Provisioning: boot
[INFO] persist: no saved SSID (no wifi_secrets.h in this tutorial)
[INFO] ESP32: BluFi started, advertising as "Tiremo-Cortex"
[INFO] BluFi: still advertising as "Tiremo-Cortex"
```

Telefon ile Configure insan adımıdır. Kimlik URC'lerini basan bir sürümde `+SSID:` (şifre loglanmaz), `WIFI GOT IP`, `persist: saved SSID` beklenir. Basmayan bir sürümde SSID `AT+CWJAP?` ile kurtarılır ve şifreyi `CWAUTOCONN` taşır:

```text
[INFO] ESP32: BluFi URC "WIFI CONNECTED"
[INFO] ESP32: BluFi WIFI GOT IP
[WARN] ESP32: GOT IP without +PASSWORD: URC (module NVS keeps the only copy)
[INFO] ESP32: SSID recovered from CWJAP? "YourSSID"
```

İkinci reset `persist: loaded SSID` ile join etmeli.

## Bitti sayılır

EspBluFi kaynakta SSID olmadan kartı kurar; sonraki reboot `BL01`'den bağlanır. Reklam donanımda doğrulandı; telefon adımı insan ister.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
