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

***Kart: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Bölüm: ESP32 Connectivity / Feature Tutorials · Zorluk: Orta***

> Bu sayfa [İngilizce orijinalinin](../../../../ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

[Sensors eğitimi 07](../../../Sensors_And_Storage/Feature_Tutorials/07-Software_Wall_Clock/README.md)'nin yazılım saati yalnızca RAM'dedir ve senkronize olmadan başlar. Bu eğitim eksik girdiyi sağlar: eğitim 03'ün bağlanma yolu bir IPv4 adresi aldıktan sonra ESP32'de SNTP'yi açar, `CIPSNTPTIME` asctime metnini bir Unix epoch'una çevirir ve `TIREMO_Clock_SetEpoch()`'u çağırır. Sonraki her MQTT zaman damgası buna bağlıdır; ayrıca bulut bölümlerindeki güvenli (TLS) MQTT bağlantıları, bağlanabilmek için önce doğru bir saate ihtiyaç duyar — bugünün tarihi bilinmeden bir sertifika doğrulanamaz.

BluFi eğitim 05'tir. Onu buraya eklemeyin.

## Neler Öğreneceksiniz

- asctime UTC olsun diye `AT+CIPSNTPCFG`'yi **saat dilimi 0** ile kullanmak
- `CIPSNTPTIME?`'in neden `"+CIPSNTPTIME:"` ile değil `"OK"` ile eşleştiği
- `Thu Jan  1 00:00:08 1970`'in neden 1970'te bir zaman değil, "henüz senkronize olmadı" anlamına geldiği
- Ayrıştırılan epoch'u Sensors eğitimi 07'deki aynı `TIREMO_Clock` API'sine vermek

## Ön Koşullar

- [03-Connect_Retry_State_Machine](../03-Connect_Retry_State_Machine/README.md)
- [07-Software_Wall_Clock](../../../Sensors_And_Storage/Feature_Tutorials/07-Software_Wall_Clock/README.md)
- İnternet erişimi olan 2,4 GHz bir WiFi ağı (modül saati herkese açık NTP sunucularından sorar) ve bu `Sw/` klasörünün kendi `wifi_secrets.h` dosyası — bkz. [Kimlik bilgileri](#kimlik-bilgileri-şifreyi-commit-etmeyin)
- [Getting Started](../../../Getting_Started/) kurulum betikleri çalıştırılmış olmalı, böylece `tdf` kullanılabilir
- Log'u izlemek için: `tdf monitor` ya da kartın USB seri portunda 115200 8N1'de herhangi bir seri terminal (Tera Term, PuTTY, screen, minicom, …)
- USB kabloyla bilgisayarınıza bağlı bir Tiremo Cortex kartı (ABOV A34G43A MCU — bkz. [Donanıma Genel Bakış](../../../Hardware.md))

## CIPSNTPCFG saat dilimini anlamak

```c
return TIREMO_ESP32_SendCommand(
    "AT+CIPSNTPCFG=1,0,\"pool.ntp.org\",\"time.google.com\"\r\n",
    "OK",
    TIREMO_ESP32_DEFAULT_CMD_TIMEOUT_MS);
```

İkinci argüman bir Unix kayması değil, UTC'ye göre saat farkıdır. **0**, `CIPSNTPTIME?`'in verdiği asctime'ın UTC olduğu anlamına gelir; Unix epoch da zaten UTC'dir. `timezone=3` Türkiye saatini basardı; o zaman saf bir ayrıştırıcı `SetEpoch`'u üç saat ileri ayarlar ve buluta gönderilen her zaman damgası yanlış olurdu.

SNTP IPv4 gerektirir. Bu `Sw/`, eğitim 03'ün tarama / yeniden deneme / `WF01` kalıcılığını yeniden kullanır — filtreli tarama, `wifi_secrets.h` kontrolü ve DEBUG dökümünde gizlenen `AT+CWJAP` dahil — ve SNTP'yi ancak `WIFI GOT IP`'den sonra açar.

## asctime tuzağını anlamak

Kartta `TIREMO_DEBUG_LEVEL=4` ile yakalanan yanıt şöyledir:

```text
{"data":"+CIPSNTPTIME:Tue Nov 24 06:51:54 2026\r\nOK"}
```

Tarih ve saat `+CIPSNTPTIME:`'dan **sonra** gelir. O işaretle eşleşmek, iki nokta geldiği anda döner — baytlar teker teker gelir — ve ayrıştırıcı hiçbir şey kopyalayamaz. Eğitim 02'deki `CWJAP?` ile aynı kural: sondaki `"OK"` ile eşleşin, sonra işareti `strstr` ile bulun.

Modül bir NTP örneği alana kadar saati Unix epoch 0'dan saymaya başlar. SNTP açıldıktan bir saniye sonra kartın verdiği ilk yanıt buydu:

```text
{"data":"+CIPSNTPTIME:Thu Jan  1 00:00:08 1970\r\nOK"}
```

Bu geçerli bir asctime ve geçerli bir epoch'tur (8). Başarı sayılsaydı `SetEpoch(8)` yapılır ve her log 1970'teymiş gibi görünürdü. `QuerySntpEpoch`, 2020-01-01'den (`1577836800`) önceki her şeyi reddeder. `WaitSntpEpoch` saniyede bir, en fazla 20 saniye sorar; kartta ikinci sorgu gerçek saati almıştı bile.

Gün alanı boşlukla doldurulur (`Jan  1` ve `Nov 24`). Ayrıştırıcı boşlukları atlar; `sscanf` kullanmaz (newlib-nano `sscanf`, bu yazılımın uygulamadığı POSIX G/Ç fonksiyonlarını çeker).

```c
if (!TIREMO_ESP32_SendCommandCapture("AT+CIPSNTPTIME?\r\n", "OK",
                                     TIREMO_ESP32_STATUS_TIMEOUT_MS,
                                     response, sizeof(response)))
{
    return false;
}

return prvParseSntpResponse(response, pEpoch);
```

Ardından:

```c
TIREMO_Clock_SetEpoch(epoch);
```

Bundan sonra `GetEpoch()` yine `epoch_at_sync + elapsed_ms / 1000`'dir — eğitim 07'nin hesabı, yalnızca `1704067200` yerine gerçek bir NTP örneğiyle.

## Kimlik bilgileri (şifreyi commit etmeyin)

[Eğitim 03'teki](../03-Connect_Retry_State_Machine/README.md#kimlik-bilgileri-şifreyi-commit-etmeyin) gibi: bu `Sw/` klasörünün kendi `wifi_secrets.h` dosyası gerekir. Repo kök dizininden:

```sh
cd ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/Sw
cp wifi_secrets.h.example wifi_secrets.h
```

(cmd.exe'de `cp` yerine `copy` kullanın.) Ardından ağınızın SSID'sini ve şifresini girin ya da eğitim 02 veya 03 için hazırladığınız `wifi_secrets.h`'i kopyalayın. Dosya [`Sw/.gitignore`](../../../../ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/Sw/.gitignore) içinde listelidir, yani hiçbir zaman bir commit'e girmez.

## Çözüm

Repo kök dizininden (`wifi_secrets.h`'i yukarıdaki gibi oluşturduktan sonra):

```sh
cd ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/Sw
tdf build
tdf flash
tdf monitor
```

(`tdf`'nin ne olduğu için [Getting_Started/01-Hello_Blink'in Çözüm bölümüne](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) bakın. macOS/Linux'ta `tdf.py` takma ad olarak da çalışır; Windows'ta `tdf` kullanın. `tdf monitor`'ü çalıştırmadan önce kartın seri portundaki diğer terminalleri kapatın — portu aynı anda yalnızca bir program açabilir.)

**Gerçek donanımda doğrulandı:** `tdf build`, macOS'ta ve Windows 11'de hatasız derlenip bağlanıyor (`.text` yaklaşık 21 KB; kesin boyut toolchain sürümüne göre biraz değişir, ve linker [01-Hello_Blink'in Çözüm bölümünde](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) açıklanan beklenen ve zararsız `_close` / `_lseek` / `_read` / `_write is not implemented` uyarılarını basar). `tdf flash`, gerçek, bağlı bir Tiremo Cortex kartına karşı SWD üzerinden tam bir programlama-ve-doğrulama döngüsünü tamamladı (`** Verified OK **`). Bir resetten sonraki log (ağ adı değiştirildi):

```text
[INFO] NTP_Time_Sync: boot
[INFO] persist: loaded SSID "YourSSID"
[INFO] ESP32: power cycling module...
[INFO] ESP32: power-on boot delay elapsed
[INFO] TIREMO_ESP32_Init: UART2 ready
[INFO] ESP32: command echo disabled (ATE0)
[INFO] ESP32: scanning for "YourSSID"
[INFO] ESP32: SSID visible
[INFO] ESP32: join attempt 1/3
[INFO] ESP32: WIFI GOT IP
[INFO] ESP32: CWSTATE=2
[INFO] persist: saved SSID "YourSSID" (password not logged)
[INFO] ESP32: SNTP enabled (timezone 0 = UTC)
[INFO] clock: SetEpoch 1795503159 (from CIPSNTPTIME asctime)
[INFO] clock: epoch=1795503160  elapsed=1 s
[INFO] clock: epoch=1795503161  elapsed=2 s
[INFO] clock: epoch=1795503162  elapsed=3 s
```

`1795503159`, **2026-11-24 06:52:39 UTC**'dir ve bilgisayarın saatiyle bir saniye içinde örtüştü. Bundan sonra `epoch` ve `elapsed` birlikte, saniyede bir artar. Sizin sayılarınız bugünün tarihi olacaktır — herhangi bir [epoch dönüştürücü](https://www.epochconverter.com/) onları okunabilir bir saate çevirir. `clock:` satırları resetten yaklaşık 15 saniye sonra başlar: modülün açılması için 3,5 saniye, ardından tarama, bağlanma ve ilk NTP yanıtı için birkaçar saniye.

## Adım Adım İnceleme

1. [`main.c`](../../../../ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/Sw/Src/main.c) — UART0 + UART2, değişmedi.
2. Eğitim 03'ün bağlanma yolu — tarama, 3× `CWJAP`, `WF01` kalıcılığı.
3. [`tiremo_clock.c`](../../../../ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/Sw/Common/Src/tiremo_clock.c) — Sensors 07'den kopyalandı; epoch artık sabit bir değer yerine NTP'den gelir.
4. [`tiremo_esp32.c`](../../../../ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/Sw/Drivers/esp32/Src/tiremo_esp32.c) — `EnableSntp`, `QuerySntpEpoch`, `WaitSntpEpoch`, asctime → UTC epoch.
5. [`prv_user_code.c`](../../../../ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/Sw/Src/prv_user_code.c) — GOT IP'den sonra: `CIPSNTPCFG` → SNTP'yi bekle → `SetEpoch` → saniyede bir `GetEpoch` logu. Bir SNTP hatası yeniden başlatma yerine bir `[ERROR]` satırıyla durur.

## Tamamlanma Ölçütü

`SetEpoch` değeri, güncel UTC saatiyle (bir epoch dönüştürücüyle kontrol edin) birkaç saniye içinde örtüşüyor ve sonrasında `epoch` ile `elapsed` her saniye birer artıyor.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
