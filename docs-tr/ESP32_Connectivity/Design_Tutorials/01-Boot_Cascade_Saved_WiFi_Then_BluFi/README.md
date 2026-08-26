<p class="sphinxhide" align="right"><a href="../../../../ESP32_Connectivity/Design_Tutorials/01-Boot_Cascade_Saved_WiFi_Then_BluFi/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>Boot Cascade: Saved WiFi → BluFi</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + AWS IoT üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Boot Cascade: Saved WiFi → BluFi

***Kart: Tiremo Cortex (ESP32) · Bölüm: ESP32 Connectivity / Design Tutorials · Zorluk: Zor***

> Bu sayfa [İngilizce orijinalinin](../../../../ESP32_Connectivity/Design_Tutorials/01-Boot_Cascade_Saved_WiFi_Then_BluFi/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Özellik eğitimleri 01–05 her biri bir parçayı kanıtladı. Bu, kartın gerçek açılış akışıdır: MCU'da kayıtlı kimlik, yoksa yalnızca SSID ile NVS yeniden katılım, yoksa BluFi; her başarıda NTP. `wifi_secrets.h` yok.

## Neler Öğreneceksiniz

- Her biri `bool` dönen `prvTryX()` zinciri
- `CS01` flash'ta **SSID+şifre** / **yalnız SSID** / **hiçbiri**
- BluFi'nin son çare olması; NTP'nin yalnızca IPv4 sonrası çalışması

## Açılış banner'ı

Design Tutorials Tiremo Cortex wordmark'ını basar. Feature Tutorials sessizdir. Çizim çevre kaydedici ile aynıdır (100 sütun).

## Kaskadı anlamak

| Persist | Yol | Ne yapar |
|---------|-----|----------|
| SSID + şifre | `saved` | `CWLAP` sonra 3× `CWJAP` |
| SSID, boş şifre | `nvs` | `CWAUTOCONN=1`, sonra modül kendi bağlanırken 15 s `CWSTATE` izlenir |
| `CS01` yok | `blufi` | `Tiremo-Cortex` reklam, URC hasadı, `CS01` kayıt |

`saved` veya `nvs` fail olursa **BluFi'ye düşer**. Magic **`CS01`**, `WF01`/`BL01` değil.

## Hiç gelmeyen şifre

Her ESP-AT sürümü `+PASSWORD:` basmaz. Bu tutorial'ın yazıldığı kartta kurulum yalnızca `+BLUFICONN`, `WIFI CONNECTED` ve `WIFI GOT IP` üretir; kimlik bilgisi URC'si yoktur. Şifre artık geri alınamaz: `AT+CWJAP?` tasarım gereği sadece SSID döner.

Kaskad bunu hata değil, normal bir sonuç kabul eder:

1. `CWAUTOCONN=1` `AT+BLUFI=0`'dan **önce** gönderilir; böylece durdurma komutu başarısız olsa bile modülün NVS'indeki kopya korunur.
2. `+SSID:` gelmediyse SSID `AT+CWJAP?` ile alınır.
3. `CS01` boş şifreyle yazılır; sonraki açılış `nvs` yoluna gider ve şifreyi modül kendi verir.

Bu yolda iki zamanlama tuzağı var. `AT+BLUFI=0` önce `+BLUFIDISCONN`, ancak sonra `OK` döner; bir saniyelik timeout erken dolar ve geciken `OK` **bir sonraki** komutun eşleşmesini karşılar. `AT+CWJAP?` sorgusunun `+CWJAP:` satırı olmadan dönmesinin ve SSID'nin kaybolmasının sebebi budur. Bu yüzden `BlufiStop()` 5 s bekler ve `GetConnectedSsid()` işaretçi yoksa tekrar dener:

```c
pMarker = strstr(response, "+CWJAP:");
if (pMarker == NULL)
{
    TIREMO_LOG_WARN("ESP32: CWJAP? matched a stale OK, retrying");
    TIREMO_SysTick_DelayMs(TIREMO_ESP32_SETTLE_MS);
    continue;
}
```

Ayrıştırıcının tanımadığı her BluFi satırı `ESP32: BluFi URC "..."` olarak basılır; farklı yazan bir firmware sessizce kaybolmak yerine logda görünür.

## Çözüm

```sh
cd Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

**Doğrulandı:** `.text` 25196, flaş OK. Fabrika taze açılış reklam verdi ve EspBluFi ile kuruldu:

```text
[INFO] persist: no CS01 record (factory-fresh for this app)
[INFO] cascade: BluFi fallback
[INFO] ESP32: BluFi phone connected
[INFO] ESP32: BluFi URC "WIFI CONNECTED"
[INFO] ESP32: BluFi WIFI GOT IP
[INFO] cascade: SSID from CWJAP? "Zyxel_4AA1"
[INFO] cascade: no +PASSWORD: URC, saving SSID only (next boot uses NVS rejoin)
[INFO] persist: saved SSID "Zyxel_4AA1" (password not logged)
[INFO] clock: SetEpoch 1787773450  path=blufi
```

Sonraki reset BluFi'yi tamamen atladı ve `nvs` yolundan açıldı. `CWSTATE` uyarıları, modül kendi yeniden bağlanmasını bitirirken dönen poll döngüsüdür:

```text
[INFO] persist: loaded SSID "Zyxel_4AA1" password=no
[INFO] cascade: try NVS auto-rejoin (SSID only, no MCU password)
[WARN] ESP32 RX: timeout after 2000 ms, 33 bytes without "+CWSTATE:2"
[INFO] cascade: NVS rejoin got IP
[INFO] clock: SetEpoch 1787773514  path=nvs
```

`+PASSWORD:` basan bir AT sürümünde şifre `CS01`'e yazılır ve sonraki açılış `saved` yolunu kullanır.

## Bitti sayılır

Fabrika taze açılış BluFi reklamı verir; kurulumdan sonra sonraki reset WiFi + NTP. Üç aşama da donanımda **doğrulandı**; bu AT sürümü `+PASSWORD:` göndermediği için son durak `cascade: online path=nvs`.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
