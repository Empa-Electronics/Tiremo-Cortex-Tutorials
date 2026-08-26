<p class="sphinxhide" align="right"><a href="../../../../ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>WiFi Station Join</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + AWS IoT üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# WiFi Station Join

***Kart: Tiremo Cortex (ESP32) · Bölüm: ESP32 Connectivity / Feature Tutorials · Zorluk: Orta***

> Bu sayfa [İngilizce orijinalinin](../../../../ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Mümkün olan en basit WiFi bağlantısı: bir sabit ağ, bir deneme, yeniden deneme yok. Eğitim 01 UART'ın `AT` deyip `OK` duyabildiğini kanıtladı. Bu eğitim ilk *işe yarar* AT dizisidir — ve alt dize eşleşmesinin yalan söyleyebileceği ilk yer.

Yeniden deneme, önce tarama ve flash'ta son iyi SSID Eğitim 03'tür. Buraya eklemeyin.

## Neler Öğreneceksiniz

- `AT+CWMODE=1`, `AT+CWJAP`, `AT+CWSTATE?`
- `JoinAP`'ın `"OK"` yerine `"WIFI GOT IP"` beklemesi
- `IsConnected`'ın `"+CWSTATE:"` yerine `"+CWSTATE:2"` eşlemesi
- `GetConnectedSsid`'in `"+CWJAP:"` yerine `"OK"` eşlemesi — tırnaklı ad o işaretten *sonra* gelir, üstelik bayt bayt

## Üç komutu anlamak

| Komut | Anlamı | Bu eğitim şunu bekler |
|-------|--------|------------------------|
| `AT+CWMODE=1` | yalnızca istasyon | `"OK"` |
| `AT+CWJAP="ssid","pass"` | ilişkilendir + DHCP | `"WIFI GOT IP"` |
| `AT+CWSTATE?` | anlık bağ durumu | `"+CWSTATE:2"` |

Durum 2 = bağlı, IPv4 var. 0 boşta, 1 bağlanıyor, 3 bağlı ama IP yok, 4 kopuk.

Join *öncesi* `AT+CWAUTOCONN=0` ve `AT+CWQAP`: modül son AP'yi kendi NVS'inde tutar ve aksi halde `wifi_secrets.h`'deki SSID'yi yok sayarak o ağa yeniden bağlanır.

## Alt dize tuzağını anlamak

Eğitim 01'in `SendCommand`'ı, `pExpected` şimdiye kadar gelen baytlarda **herhangi bir yerde** görünür görünmez döner. ISR **tek bayt** iter. İlginç veri, eşleyebileceğiniz bir etiketten *sonraysa* bu bir tuzaktır.

**1. `CWJAP` — `"OK"` eşlemeyin.** Komut DHCP bitmeden ACK verebilir. `JoinAP` `"WIFI GOT IP"` bekler:

```c
(void)snprintf(command, sizeof(command), "AT+CWJAP=\"%s\",\"%s\"\r\n",
               pSsid, pPassword);

return TIREMO_ESP32_SendCommand(command, "WIFI GOT IP", timeoutMs);
```

**2. `CWSTATE?` — `"+CWSTATE:"` eşlemeyin.** İki nokta, rakamdan bir bayt önce gelir. Prefiks 0/1/3/4 için de true döner. `IsConnected` rakamı da ister:

```c
bool TIREMO_ESP32_IsConnected(void)
{
    return TIREMO_ESP32_SendCommand("AT+CWSTATE?\r\n",
                                    "+CWSTATE:2",
                                    TIREMO_ESP32_STATUS_TIMEOUT_MS);
}
```

**3. `CWJAP?` — `"OK"` eşleyin, sonra ayrıştırın.** Tırnaklı SSID `+CWJAP:` *sonrasındadır*. İşarette dönmek `GetConnectedSsid`'i boş bırakır. Yanıtı bitiren `"OK"` tüm satırın tamponda olmasını bekler.

Kural: ihtiyaç duyduğunuz yük aynı satırda bir etiketten **sonraysa**, yanıtın **sonunu** (`"OK"`) eşleyin, sonra parse edin. Yük **etiketin kendisiyse** (`WIFI GOT IP`, `+CWSTATE:2`), onu eşleyin.

## Kimlik bilgileri (şifre commit etmeyin)

```sh
cd Sw
cp wifi_secrets.h.example wifi_secrets.h
```

`wifi_secrets.h` [`.gitignore`](../../../../ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/Sw/.gitignore) içindedir. Üretici yazılımı SSID'yi loglar, şifreyi asla.

## Çözüm

```sh
cd Sw
cp wifi_secrets.h.example wifi_secrets.h
tdf.py build
tdf.py flash
tdf.py monitor
```

**Doğrulandı:** `.text` 15200, flaş OK. Gitignore'daki `wifi_secrets.h` doldurulunca gerçek 2.4 GHz AP'de join doğrulandı (commit edilen örnek hâlâ `YourSSID` yer tutucusu).

Başarılı join:

```text
[INFO] WiFi_Station_Join: boot
[INFO] ESP32: station mode
[INFO] ESP32: joining SSID "YourSSID"
[INFO] ESP32: WIFI GOT IP
[INFO] ESP32: CWSTATE=2 (connected, IPv4)
[INFO] ESP32: CWJAP? SSID="YourSSID"
[INFO] ESP32: heartbeat still connected
```

## Adım adım

1. [`main.c`](../../../../ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/Sw/Src/main.c) — 01 ile aynı
2. [`wifi_secrets.h.example`](../../../../ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/Sw/wifi_secrets.h.example) — şifrenin tek yeri
3. [`tiremo_esp32.c`](../../../../ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/Sw/Drivers/esp32/Src/tiremo_esp32.c) — 01 taşımasına join API'leri
4. [`prv_user_code.c`](../../../../ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/Sw/Src/prv_user_code.c) — Init → ATE0 → NVS auto-join kapat → CWMODE → CWJAP → CWSTATE=2 → SSID yaz → heartbeat. Hata `ERROR` ile asılı kalır (reboot döngüsü yok)

## Bitti sayılır

Kart taze güç-açılışında gerçek bir AP'ye bağlanır, `CWSTATE?` durum 2 bildirir. `wifi_secrets.h`'i düzenleyin, flaşlayın, yukarıdaki logu doğrulayın.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
