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
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Boot Cascade: Saved WiFi → BluFi

***Kart: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Bölüm: ESP32 Connectivity / Design Tutorials · Zorluk: Zor***

> Bu sayfa [İngilizce orijinalinin](../../../../ESP32_Connectivity/Design_Tutorials/01-Boot_Cascade_Saved_WiFi_Then_BluFi/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Feature eğitimleri 01–05 her biri tek bir parçayı kanıtladı. Bu, kartın gerçek açılış akışıdır: MCU'da kayıtlı kimlik bilgilerini dene, olmazsa yalnızca SSID ile NVS üzerinden yeniden bağlan, o da olmazsa BluFi; herhangi biri başarılı olunca NTP ile saati eşitle. `wifi_secrets.h` yoktur.

## Neler Öğreneceksiniz

- Her biri düz bir `bool` döndüren `prvTryX()` fonksiyonlarından oluşan bir kademe (cascade)
- `CS01` flash kaydında **SSID+şifre**, **yalnızca SSID** ve **hiçbir şey** durumlarına göre dallanmak
- BluFi'nin neden son çare olarak kaldığı ve NTP'nin neden yalnızca IPv4'ten sonra çalıştığı

## Ön Koşullar

- [ESP32 Connectivity / Feature Tutorials](../../Feature_Tutorials/) eğitimlerinin tamamı (01–05)
- Fabrikadan çıkmış gibi (kayıtsız) açılış yolu için telefonda EspBluFi ([eğitim 05](../../Feature_Tutorials/05-BluFi_Provisioning/README.md)'teki gibi) ve internet erişimi olan 2,4 GHz bir WiFi ağı (NTP için)
- [Getting Started](../../../Getting_Started/) kurulum betikleri çalıştırılmış olmalı, böylece `tdf` kullanılabilir
- Log'u izlemek için: `tdf monitor` ya da kartın USB seri portunda 115200 8N1'de herhangi bir seri terminal (Tera Term, PuTTY, screen, minicom, …), **en az 100 sütun genişliğinde** (banner için)
- USB kabloyla bilgisayarınıza bağlı bir Tiremo Cortex kartı (ABOV A34G43A MCU — bkz. [Donanıma Genel Bakış](../../../Hardware.md))

## Açılış banner'ı

Design Tutorials, Tiremo Cortex logosunu basar; böylece boş bir terminal bile uygulamayı tanıtır. Feature Tutorials sessiz kalır. [`tiremo_banner.c`](../../../../ESP32_Connectivity/Design_Tutorials/01-Boot_Cascade_Saved_WiFi_Then_BluFi/Sw/Common/Src/tiremo_banner.c), [çevresel logger'daki](../../../Sensors_And_Storage/Design_Tutorials/01-Standalone_Environmental_Logger/README.md#açılış-bannerı) çizimin aynısıdır (100 sütun genişliğinde; "boş" satırlar tek boşluk içerir, böylece `tdf monitor` aradaki boşluğu yok etmez). Buradaki başlık satırları:

```text
  Boot Cascade: Saved WiFi then BluFi
  Design Tutorial - Empa Electronics
```

`tdf monitor` önce seri portu açar, sonra kartı resetler; bu yüzden ilk gördüğünüz şey banner'dır. Başka bir terminal kullanıyorsanız önce bağlanın, sonra RESET'e basın.

## Kademeyi anlamak

[`prv_user_code.c`](../../../../ESP32_Connectivity/Design_Tutorials/01-Boot_Cascade_Saved_WiFi_Then_BluFi/Sw/Src/prv_user_code.c) devasa iç içe bir `if` kullanmaz. Her yol tek bir fonksiyondur:

```c
if (TIREMO_Persist_HasSsid(&saved) && TIREMO_Persist_HasPassword(&saved))
{
    online = prvTryScanJoin(saved.ssid, saved.password);   /* tutorial 03 */
}
else if (TIREMO_Persist_HasSsid(&saved))
{
    online = prvTryNvsRejoin(saved.ssid);                  /* NVS password */
}

if (!online)
{
    online = prvTryBlufi(&harvest);                        /* tutorial 05 */
}

prvSyncNtpThenHeartbeat(pPath);                            /* tutorial 04 */
```

| Kayıt | Yol | Ne yapar |
|-------|-----|----------|
| SSID + şifre | `saved` | `CWLAP="ssid"` (eğitim 03'ün filtreli taraması), ardından en fazla 3 kez `CWJAP="ssid","pass"` |
| SSID, boş şifre | `nvs` | `CWAUTOCONN=1`, ardından modül kendiliğinden yeniden bağlanırken 15 sn boyunca `CWSTATE`'i izler |
| `CS01` yok / bozuk | `blufi` | `Tiremo-Cortex` olarak yayın yapar, bildirimleri toplar, `CS01`'i kaydeder |

Başarısız bir `saved` ya da `nvs` denemesi BluFi'ye **düşer**. Ürün davranışı budur: yanlış bir şifre yüzünden yeniden başlatma döngüsüne girmeyin.

Magic **`CS01`**'dir (`0x43533031`), `WF01` / `BL01` değil. Eğitim 03'ün son çalışan kaydı sektör 0'da dursa bile (o kayıt yanlış magic'e sahiptir) bu imajı yüklemek kademe için fabrikadan çıkmış gibi bir başlangıçtır. Sektörün üzerine yine de ilk başarılı BluFi kaydında yazılır.

`BlufiPoll` bekleme sırasında UART2'nin tek okuyucusu olarak kalır (eğitim 05). NTP, `CIPSNTPTIME?`'de yine `"OK"` ile eşleşir (eğitim 04). Eğitim 02–05'teki gibi, DEBUG seviyesindeki TX dökümü kayıtlı bir şifre yerine `AT+CWJAP=<credentials hidden>` gösterir.

## Hiç alamadığınız şifreyi anlamak

Her ESP-AT sürümü `+PASSWORD:` basmaz. Bu eğitimin test edildiği Tiremo Cortex kartında kurulum `+BLUFICONN`, ardından `WIFI CONNECTED`, ardından `WIFI GOT IP` basar — hiçbir kimlik bilgisi bildirimi olmadan. Şifre bu durumda geri alınamaz: `AT+CWJAP?` tasarım gereği yalnızca SSID ile yanıt verir.

Kademe bunu bir hata değil, normal bir sonuç olarak ele alır:

1. `CWAUTOCONN=1`, `AT+BLUFI=0`'dan **önce** gönderilir; böylece durdurma komutu başarısız olsa bile modülün kendi NVS'indeki kimlik bilgileri korunur.
2. `+SSID:` hiç gelmediyse SSID `AT+CWJAP?`'ten alınır.
3. `CS01` boş bir şifreyle yazılır; bu da bir sonraki açılışı, şifreyi modülün kendisinin sağladığı `nvs` yoluna yönlendirir.

Bu yolda iki zamanlama tuzağı vardır. `AT+BLUFI=0` önce `+BLUFIDISCONN`, ancak ondan sonra `OK` yanıtı verir; bu yüzden bir saniyelik zaman aşımı erken dolar — ve geç gelen `OK` *bir sonraki* komutun eşleşmesini sağlar. `AT+CWJAP?`'in `+CWJAP:` satırı olmadan dönüp SSID'yi kaybetmesinin nedeni budur. Bu yüzden `BlufiStop()` 5 saniye bekler ve `GetConnectedSsid()` işaret eksikse yeniden dener:

```c
pMarker = strstr(response, "+CWJAP:");
if (pMarker == NULL)
{
    TIREMO_LOG_WARN("ESP32: CWJAP? matched a stale OK, retrying");
    TIREMO_SysTick_DelayMs(TIREMO_ESP32_SETTLE_MS);
    continue;
}
```

Ayrıştırıcının tanımadığı her BluFi satırı `ESP32: BluFi URC "..."` olarak yansıtılır; böylece bu bildirimleri farklı yazan bir yazılım, kaybolmak yerine logda görünür.

## Çözüm

Oluşturulacak bir `wifi_secrets.h` yoktur. Repo kök dizininden:

```sh
cd ESP32_Connectivity/Design_Tutorials/01-Boot_Cascade_Saved_WiFi_Then_BluFi/Sw
tdf build
tdf flash
tdf monitor
```

(`tdf`'nin ne olduğu için [Getting_Started/01-Hello_Blink'in Çözüm bölümüne](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) bakın. macOS/Linux'ta `tdf.py` takma ad olarak da çalışır; Windows'ta `tdf` kullanın. `tdf monitor`'ü çalıştırmadan önce kartın seri portundaki diğer terminalleri kapatın — portu aynı anda yalnızca bir program açabilir.)

**Gerçek donanımda doğrulandı:** `tdf build`, macOS'ta ve Windows 11'de hatasız derlenip bağlanıyor (`.text` yaklaşık 25 KB; kesin boyut toolchain sürümüne göre biraz değişir, ve linker [01-Hello_Blink'in Çözüm bölümünde](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) açıklanan beklenen ve zararsız `_close` / `_lseek` / `_read` / `_write is not implemented` uyarılarını basar). `tdf flash`, gerçek, bağlı bir Tiremo Cortex kartına karşı SWD üzerinden tam bir programlama-ve-doğrulama döngüsünü tamamladı (`** Verified OK **`) ve aşağıdaki iki aşama da bu kartta çalıştı. Ağ adları `YourSSID` ile değiştirildi.

### 1. Kayıtsız açılış: BluFi

`CS01` kaydı yokken açılış `persist: no CS01 record (factory-fresh for this app)` ve `cascade: BluFi fallback` loglar, `Tiremo-Cortex` olarak yayına başlar ve her 5 saniyede bir `BluFi: still advertising as "Tiremo-Cortex"` satırını tekrarlar. Kartı EspBluFi ile tam olarak [eğitim 05'teki](../../Feature_Tutorials/05-BluFi_Provisioning/README.md#1-telefondan-kurulum) gibi kurun (Configure → Station → SSID ve şifre). Test kartında:

```text
[INFO] ESP32: BluFi phone connected
[INFO] ESP32: BluFi URC "WIFI CONNECTED"
[INFO] ESP32: BluFi WIFI GOT IP
[INFO] ESP32: BluFi stopped
[INFO] cascade: SSID from CWJAP? "YourSSID"
[INFO] cascade: no +PASSWORD: URC, saving SSID only (next boot uses NVS rejoin)
[INFO] persist: saved SSID "YourSSID" (password not logged)
[INFO] ESP32: SNTP enabled (timezone 0 = UTC)
[INFO] clock: SetEpoch 1795506247  path=blufi
[INFO] cascade: online path=blufi epoch=1795506252
[INFO] cascade: online path=blufi epoch=1795506257
```

### 2. Reset: `nvs` yolu

Telefona dokunmadan RESET'e basın (ya da `tdf monitor`'ü yeniden çalıştırın). Kart BluFi'yi tamamen atlar:

```text
  Boot Cascade: Saved WiFi then BluFi
  Design Tutorial - Empa Electronics

[INFO] Boot_Cascade: boot
[INFO] ESP32: power cycling module...
[INFO] ESP32: power-on boot delay elapsed
[INFO] TIREMO_ESP32_Init: UART2 ready
[INFO] ESP32: command echo disabled (ATE0)
[INFO] persist: loaded SSID "YourSSID" password=no
[INFO] cascade: try NVS auto-rejoin (SSID only, no MCU password)
[WARN] ESP32 RX: timeout after 2000 ms, 59 bytes without "+CWSTATE:2"
[WARN] ESP32 RX: timeout after 2000 ms, 54 bytes without "+CWSTATE:2"
[INFO] cascade: NVS rejoin got IP
[INFO] ESP32: SNTP enabled (timezone 0 = UTC)
[INFO] clock: SetEpoch 1795506430  path=nvs
[INFO] cascade: online path=nvs epoch=1795506435
```

İki `[WARN]` satırı beklenen durumdur: modül kendiliğinden yeniden bağlanırken (durum 0–3, henüz 2 değil) çalışan `CWSTATE?` sorgularıdır. Ardından `cascade: online` her 5 saniyede bir tekrarlanır.

`+PASSWORD:` bildiren bir AT sürümü ise şifreyi `CS01`'e kaydeder ve bunun yerine `saved` yolunu kullanır; MCU'nun kendi kopyasıyla tarar ve bağlanır.

## Adım Adım İnceleme

1. [`tiremo_banner.c`](../../../../ESP32_Connectivity/Design_Tutorials/01-Boot_Cascade_Saved_WiFi_Then_BluFi/Sw/Common/Src/tiremo_banner.c) — Design Tutorial işareti
2. [`prv_user_code.c`](../../../../ESP32_Connectivity/Design_Tutorials/01-Boot_Cascade_Saved_WiFi_Then_BluFi/Sw/Src/prv_user_code.c) — `prvTryScanJoin` / `prvTryNvsRejoin` / `prvTryBlufi` / NTP kalp atışı
3. [`tiremo_esp32.c`](../../../../ESP32_Connectivity/Design_Tutorials/01-Boot_Cascade_Saved_WiFi_Then_BluFi/Sw/Drivers/esp32/Src/tiremo_esp32.c) — eğitim 01–05'in birleşimi (filtreli tarama, bağlanma, SNTP, BluFi, `JoinStoredSsid`, DEBUG dökümünde gizlenen `AT+CWJAP`)
4. [`tiremo_persist.h`](../../../../ESP32_Connectivity/Design_Tutorials/01-Boot_Cascade_Saved_WiFi_Then_BluFi/Sw/Common/Include/tiremo_persist.h) — `CS01` + `HasPassword`. Alınan bir şifre, eğitim 03 ve 05'teki gibi MCU'nun veri flash'ında düz metin olarak tutulur.
5. [`tiremo_clock.c`](../../../../ESP32_Connectivity/Design_Tutorials/01-Boot_Cascade_Saved_WiFi_Then_BluFi/Sw/Common/Src/tiremo_clock.c) — Sensors eğitimi 07'nin API'si, NTP'den ayarlanır

## Tamamlanma Ölçütü

- Kayıtsız bir açılış BluFi yayını yapıyor; EspBluFi'den sonra log `persist: saved SSID` ve `clock: SetEpoch ... path=blufi` gösteriyor.
- Bir resetten sonra, telefonda hiçbir şey yapmadan, log `persist: loaded SSID`, ardından `clock: SetEpoch ... path=nvs` (`+PASSWORD:` bildiren bir AT sürümünde `path=saved`) ve her 5 saniyede bir `cascade: online` gösteriyor — `BluFi started` satırı olmadan.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
