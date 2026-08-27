<p class="sphinxhide" align="right"><a href="../../../../ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>NTP Time Sync</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# NTP Time Sync

***Kart: Tiremo Cortex (ESP32) · Bölüm: ESP32 Connectivity / Feature Tutorials · Zorluk: Orta***

> Bu sayfa [İngilizce orijinalinin](../../../../ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Eğitim 07'nin yazılım saati yalnızca RAM'dedir ve eşlenmemiş başlar. Bu eğitim eksik girdidir: eğitim 03 join yolu IPv4 aldıktan sonra ESP32'de SNTP açılır, `CIPSNTPTIME` asctime Unix epoch'a çevrilir, `TIREMO_Clock_SetEpoch()` çağrılır. Sonraki her MQTT zaman damgası buna bağlıdır. Tiremo Cloud scheme-5 MQTT, `AT+MQTTCONN` öncesi gerçek bir saat de ister — bkz. [TLS SNI](https://docs.tiremo.ai/device-provisioning/tls-sni) ve [ilk bağlantı](https://docs.tiremo.ai/device-provisioning/third-phase).

BluFi eğitim 05'tir. Buraya eklemeyin.

## Neler Öğreneceksiniz

- `AT+CIPSNTPCFG` **timezone 0** ile (asctime UTC)
- `CIPSNTPTIME?`'nin `"+CIPSNTPTIME:"` değil `"OK"` eşlemesi
- `Thu Jan  1 00:00:00 1970`'in "henüz senkron yok" olduğu
- Ayrıştırılan epoch'u Sensors eğitim 07 ile aynı `TIREMO_Clock` API'sine vermek

## Önkoşullar

- [03-Connect_Retry_State_Machine](../../../../ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/)
- [07-Software_Wall_Clock](../../../../Sensors_And_Storage/Feature_Tutorials/07-Software_Wall_Clock/)
- Bu `Sw/` altında `wifi_secrets.h` (gitignore)
- [Getting Started](../../../../Getting_Started/) içindeki `tdf.py`

## CIPSNTPCFG zaman dilimi

İkinci argüman UTC'den **saat**tir. **0** = asctime UTC = Unix epoch. `timezone=3` Türkiye sivil saatini basar; naif parse `SetEpoch`'u üç saat ileri alır.

SNTP IPv4 ister. Bu `Sw/` eğitim 03 tarama / yeniden deneme / `WF01` yolunu kullanır, `WIFI GOT IP` sonrası SNTP açar.

## asctime tuzağı

```text
+CIPSNTPTIME:Wed Aug 26 18:36:51 2026
OK
```

Sivil saat `+CIPSNTPTIME:` **sonrasındadır**. İşarette dönmek (bayt bayt RX) boş parse bırakır. Eğitim 02 `CWJAP?` kuralı: bitiş `"OK"` eşle, sonra marker'ı `strstr` et.

NTP örneği gelene kadar ESP-AT Unix epoch 0 basar. Bunu başarı saymak `SetEpoch(0)` yapar. `QuerySntpEpoch` 2020-01-01 (`1577836800`) öncesini reddeder. `WaitSntpEpoch` 20 s boyunca saniyede bir yoklar.

Gün alanı boşluk dolguludur (`Jan  1`). Ayrıştırıcı `sscanf` kullanmaz (newlib-nano POSIX I/O çeker).

```c
TIREMO_Clock_SetEpoch(epoch);
```

Sonrasında `GetEpoch()` hâlâ `epoch_at_sync + elapsed_ms / 1000` — eğitim 07 matematiği, gerçek NTP örneğiyle.

## Kimlik bilgileri

```sh
cd Sw
cp wifi_secrets.h.example wifi_secrets.h
```

## Çözüm

```sh
cd Sw
cp wifi_secrets.h.example wifi_secrets.h
tdf.py build
tdf.py flash
tdf.py monitor
```

**Doğrulandı:** `.text` 20956, flaş OK. Join sonrası `GetEpoch()` duvar saatiyle bire bir ilerledi:

```text
[INFO] ESP32: WIFI GOT IP
[INFO] ESP32: SNTP enabled (timezone 0 = UTC)
[INFO] clock: epoch=1787769411  elapsed=9 s
[INFO] clock: epoch=1787769412  elapsed=10 s
```

`1787769411` = **2026-08-26 18:36:51 UTC**. `elapsed` ve `epoch` 30+ s birlikte ilerledi.

## Adım adım

1. Eğitim 03 join yolu
2. [`tiremo_clock.c`](../../../../ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/Sw/Common/Src/tiremo_clock.c) — Sensors 07 kopyası
3. [`tiremo_esp32.c`](../../../../ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/Sw/Drivers/esp32/Src/tiremo_esp32.c) — SNTP + asctime
4. [`prv_user_code.c`](../../../../ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/Sw/Src/prv_user_code.c) — GOT IP → SNTP → `SetEpoch` → 1 Hz log

## Bitti sayılır

Senkron hemen ardından `GetEpoch()` gerçek UTC ile birkaç saniye içinde örtüşür, sonra saniyede bir artar — **donanımda doğrulandı**.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
