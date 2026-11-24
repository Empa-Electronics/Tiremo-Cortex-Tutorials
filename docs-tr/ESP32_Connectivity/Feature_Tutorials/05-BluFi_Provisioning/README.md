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

***Kart: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Bölüm: ESP32 Connectivity / Feature Tutorials · Zorluk: Zor***

> Bu sayfa [İngilizce orijinalinin](../../../../ESP32_Connectivity/Feature_Tutorials/05-BluFi_Provisioning/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

SSID'yi koda gömmek, tezgâhtaki tek bir kartın ötesine ölçeklenmez. Bu eğitimde `wifi_secrets.h` **yoktur**. EspBluFi çalıştıran bir telefon kimlik bilgilerini BLE üzerinden gönderir; MCU `+SSID:` / `+PASSWORD:` / `WIFI GOT IP` satırlarını (modül bunları bildiriyorsa) toplar ve veri flash'ına (`BL01`) yazar. Bir sonraki reset, yayın adımı olmadan o kayıtlı AP'ye bağlanır.

Açılış sırası (önce kayıtlı WiFi, BluFi yalnızca yedek olarak, sonra NTP) [tasarım eğitiminin](../../Design_Tutorials/01-Boot_Cascade_Saved_WiFi_Then_BluFi/README.md) konusudur. NTP'yi buraya eklemeyin.

## Neler Öğreneceksiniz

- Önce `AT+BLUFINAME`, sonra `AT+BLUFI=1`; ve eşzamansız bildirimler (URC) `+BLUFICONN`, `+SSID:`, `+PASSWORD:`, `WIFI GOT IP`
- Bekleme sırasında `TIREMO_ESP32_BlufiPoll()`'un neden UART2'nin **tek** okuyucusu olması gerektiği
- O döngüde `IsConnected()` / `CWSTATE?` çağırmanın neden bir kimlik bilgisi bildirimini yutabileceği
- Başarılı bir kurulumdan sonra BluFi'yi durdurmak (`AT+BLUFI=0`) ve `CWAUTOCONN`'u yeniden açmak
- Modül şifreyi MCU'ya hiç bildirmediğinde kurulmuş ağa yeniden bağlanmak (parametresiz `AT+CWJAP`)

## Ön Koşullar

- Kavramsal olarak [04-NTP_Time_Sync](../04-NTP_Time_Sync/README.md) (WiFi zaten çalışıyor); bu `Sw/` bağlanma + BluFi'dir, SNTP değil
- Telefonda EspBluFi: [Android](https://github.com/EspressifApp/EspBluFi-Android) / [iOS](https://apps.apple.com/app/espblufi/id1455860696)
- Telefonda Bluetooth açık; Android'de BLE taraması için konumun da açık olması gerekir
- Adını ve şifresini bildiğiniz 2,4 GHz bir WiFi ağı — bunları bir dosyaya değil, telefon uygulamasına yazarsınız
- [Getting Started](../../../Getting_Started/) kurulum betikleri çalıştırılmış olmalı, böylece `tdf` kullanılabilir
- Log'u izlemek için: `tdf monitor` ya da kartın USB seri portunda 115200 8N1'de herhangi bir seri terminal (Tera Term, PuTTY, screen, minicom, …)
- USB kabloyla bilgisayarınıza bağlı bir Tiremo Cortex kartı (ABOV A34G43A MCU — bkz. [Donanıma Genel Bakış](../../../Hardware.md))

## RX sahipliğini anlamak

`SendCommand` göndermeden önce UART2 halkasını **temizler**. `IsConnected()` yalnızca `AT+CWSTATE?`'tir. BluFi'yi beklerken onu sorgularsanız, bu temizleme şunları:

```text
+SSID:YourNetwork
+PASSWORD:secret
WIFI GOT IP
```

`BlufiPoll` daha tek bir satır görmeden çöpe atabilir. O zaman bir IPv4 adresiniz olur ama kaydedecek şifreniz olmaz.

`BlufiPoll` yalnızca baytları **çeker**, satırları birleştirir ve bir toplama yapısını günceller. Hiç TX yapmaz. Bekleme döngüsü:

```c
for (;;)
{
    TIREMO_ESP32_BlufiPoll(&st);
    if (st.gotIp)
    {
        break;
    }
    TIREMO_SysTick_DelayMs(20U);
}
```

`AT+BLUFI=0`'ın kendisi de bir `SendCommand`'dır. `st`'yi (SSID/şifre) durdurmadan **önce** kopyalayın.

## AT dizisini anlamak

| Komut | Ne zaman | Beklenen |
|-------|----------|----------|
| `AT+CWMODE=1` | BLE'den önce | `"OK"` |
| `AT+CWAUTOCONN=0` | BLE'den önce | `"OK"` |
| `AT+BLUFINAME="Tiremo-Cortex"` | `BLUFI=1`'den önce | `"OK"` |
| `AT+BLUFI=1` | yayını başlat | `"OK"` |
| *(komut yok)* | bekle | `BlufiPoll` ile bildirimler |
| `AT+CWAUTOCONN=1` | `WIFI GOT IP`'den sonra | `"OK"` |
| `AT+BLUFI=0` | otomatik bağlanma yeniden açıldıktan sonra | `"OK"`, en fazla 5 sn |

Ad, `AT+BLUFI=1`'den **önce** ayarlanmalıdır; yoksa modül `BLUFI_DEVICE` adıyla yayın yapar. Bluetooth LE önceden başlatılmış olmamalıdır (`AT+BLEINIT=0`); `TIREMO_ESP32_Init()`'teki güç döngüsü onu bu durumda bırakır.

ESP-AT'nin, uygulama kimlik bilgilerini gönderdiğinde `+SSID:` ve `+PASSWORD:`, istasyon DHCP'si başarılı olunca da `WIFI CONNECTED` / `WIFI GOT IP` bastığı *belgelenmiştir*. Her sürüm bunu gerçekten yapmaz: bazıları yalnızca `+BLUFICONN`, `WIFI CONNECTED` ve `WIFI GOT IP` basar. Bu eğitimin test edildiği Tiremo Cortex kartındaki ESP32 modülü de bunlardan biridir. Ayrıştırıcının tanımadığı her satır `ESP32: BluFi URC "..."` olarak yansıtılır; böylece hangi sürüme sahip olduğunuzu görebilirsiniz.

`+PASSWORD:` hiç gelmezse MCU şifreyi hiçbir zaman öğrenmez — `AT+CWJAP?` yalnızca SSID'yi döndürür ve `BL01` kaydı boş bir şifreyle kaydedilir. Modülün kendisi ise şifreyi kendi NVS'inde hâlâ tutar. Yukarıdaki tabloda `CWAUTOCONN=1`'in `AT+BLUFI=0`'dan **önce** gönderilmesinin nedeni budur: o AP'ye geri dönmenin tek yolu modülün NVS'indeki kopya olur, bu yüzden başka bir şey başarısız olmadan önce devreye alınmalıdır.

Bir sonraki açılışta, şifresi boş olan kayıtlı bir SSID'ye `AT+CWJAP="<ssid>",""` ile **bağlanılmamalıdır**: modül bunu "bu ağ, şifresiz" olarak anlar ve `ERROR` ile reddeder — yazılım da `BL01`'i silip her açılışta telefonu yeniden beklerdi. Bunun yerine `JoinSaved()` parametresiz bir `AT+CWJAP` gönderir; bu, modüle kendi NVS'inde saklı AP'ye bağlanmasını söyler:

```c
if (pPassword[0] == '\0')
{
    join = TIREMO_ESP32_JoinStoredAP(JOIN_TIMEOUT_MS);   /* "AT+CWJAP\r\n" */
}
else
{
    join = TIREMO_ESP32_JoinAP(pSsid, pPassword, JOIN_TIMEOUT_MS);
}
```

`AT+BLUFI=0` ayrıca önce `+BLUFIDISCONN`, ardından `OK` yanıtı verir. Bir saniyelik zaman aşımıyla bekleme erken biter ve sondaki `OK` *bir sonraki* komut sırasında gelip onun eşleşmesini sağlar — hemen ardından gelen bir `AT+CWJAP?` `+CWJAP:` satırı olmadan döner ve SSID de kaybolur. Bu yüzden durdurma 5 saniye bekler ve `GetConnectedSsid()` işaret eksikse yeniden dener.

## Kalıcılık magic'i `BL01`

Eğitim 03'teki aynı sektör-0 CRC düzeni, magic **`0x424C3031` (`BL01`)**, `WF01` değil. Eğitim 03/04'ten kalan son çalışan bağlantı bir BluFi kurulumu olarak **sayılmaz**; bu yüzden bu yazılımın ilk yüklenişinde kart her zaman yayın yapar. EspBluFi başarılı olunca `BL01` yazılır; bir sonraki reset onu yükler ve yayın yapmadan bağlanır.

Bu imajı yüklemek de DFMC sektör 0'ın (TRM1 / ENV1 / WF01) **üzerine yazar**.

## Çözüm

Oluşturulacak bir `wifi_secrets.h` yoktur. Repo kök dizininden:

```sh
cd ESP32_Connectivity/Feature_Tutorials/05-BluFi_Provisioning/Sw
tdf build
tdf flash
tdf monitor
```

(`tdf`'nin ne olduğu için [Getting_Started/01-Hello_Blink'in Çözüm bölümüne](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) bakın. macOS/Linux'ta `tdf.py` takma ad olarak da çalışır; Windows'ta `tdf` kullanın. `tdf monitor`'ü çalıştırmadan önce kartın seri portundaki diğer terminalleri kapatın — portu aynı anda yalnızca bir program açabilir.)

**Gerçek donanımda doğrulandı:** `tdf build`, macOS'ta ve Windows 11'de hatasız derlenip bağlanıyor (`.text` yaklaşık 21 KB; kesin boyut toolchain sürümüne göre biraz değişir, ve linker [01-Hello_Blink'in Çözüm bölümünde](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) açıklanan beklenen ve zararsız `_close` / `_lseek` / `_read` / `_write is not implemented` uyarılarını basar). `tdf flash`, gerçek, bağlı bir Tiremo Cortex kartına karşı SWD üzerinden tam bir programlama-ve-doğrulama döngüsünü tamamladı (`** Verified OK **`); kart EspBluFi ile kuruldu ve bir resetten sonra kendiliğinden yeniden bağlandı.

### 1. Telefondan kurulum

Henüz `BL01` kaydı yokken açılış `persist: no saved SSID (no wifi_secrets.h in this tutorial)` loglar ve yayına başlar; her 5 saniyede bir hatırlatma satırı gelir:

```text
[INFO] ESP32: BluFi started, advertising as "Tiremo-Cortex"
[INFO] BluFi: open EspBluFi, connect to "Tiremo-Cortex", Configure
[INFO] BluFi: still advertising as "Tiremo-Cortex"
```

Telefonda: EspBluFi'yi açın, cihaz listesinde **Tiremo-Cortex**'i bulup bağlanın, **Configure**'a dokunun, **Station** modunu seçin, ağınızın SSID'sini ve şifresini girip gönderin. Bu, ESP-AT sürümü `+SSID:` / `+PASSWORD:` bildirmeyen test kartının logudur (ağ adı değiştirildi):

```text
[INFO] ESP32: BluFi phone connected
[INFO] ESP32: BluFi URC "WIFI CONNECTED"
[INFO] ESP32: BluFi WIFI GOT IP
[WARN] ESP32: GOT IP without +SSID: URC
[WARN] ESP32: GOT IP without +PASSWORD: URC (module NVS keeps the only copy)
[INFO] ESP32: BluFi stopped
[INFO] ESP32: SSID recovered from CWJAP? "YourSSID"
[INFO] persist: saved SSID "YourSSID" (password not logged)
[INFO] ESP32: heartbeat still connected
```

Böyle bir sürümde iki `[WARN]` satırı beklenen durumdur — SSID `AT+CWJAP?` ile geri alınır, şifre modülde kalır. Kimlik bilgilerini bildiren bir sürümde bunların yerine `ESP32: BluFi +SSID:"YourSSID"` ve `ESP32: BluFi +PASSWORD: received (not logged)` görürsünüz ve şifre de `BL01`'e kaydedilir.

Telefon **Tiremo-Cortex**'i listelemiyorsa: Bluetooth'un (Android'de konumun da) açık olduğunu, kartın logunda `still advertising` göründüğünü ve uygulamanın ayarlarında cihaz listesinin ada göre filtrelenmediğini kontrol edin.

### 2. Reset: kart telefonsuz yeniden bağlanır

RESET'e basın (ya da `tdf monitor`'ü yeniden çalıştırın). Kart `BL01`'i yükler, kendiliğinden bağlanır ve hiç yayına başlamaz:

```text
[INFO] BluFi_Provisioning: boot
[INFO] ESP32: power cycling module...
[INFO] ESP32: power-on boot delay elapsed
[INFO] TIREMO_ESP32_Init: UART2 ready
[INFO] ESP32: command echo disabled (ATE0)
[INFO] persist: loaded SSID "YourSSID"
[INFO] ESP32: scanning for saved SSID "YourSSID"
[INFO] ESP32: join attempt 1/3
[INFO] ESP32: no saved password, joining the AP stored in the module
[INFO] ESP32: WIFI GOT IP (saved credentials)
[INFO] ESP32: heartbeat still connected
```

`no saved password` satırı yalnızca `+PASSWORD:` bildirmeyen bir sürümde görünür; kayıtlı bir şifre varsa kart bunun yerine `AT+CWJAP="<ssid>","<şifre>"` ile bağlanır. [Açılış sırası tasarım eğitimi](../../Design_Tutorials/01-Boot_Cascade_Saved_WiFi_Then_BluFi/README.md), bu "önce kayıtlı ağ, yedek olarak BluFi" sırasını eksiksiz bir açılış akışına dönüştürür.

## Adım Adım İnceleme

1. [`prv_user_code.c`](../../../../ESP32_Connectivity/Feature_Tutorials/05-BluFi_Provisioning/Sw/Src/prv_user_code.c) — `BL01` varsa eğitim 03'teki gibi tara/bağlan (şifre kaydedilmediyse modülün sakladığı AP ile); yoksa GOT IP'ye kadar yayın yap.
2. [`tiremo_esp32.c`](../../../../ESP32_Connectivity/Feature_Tutorials/05-BluFi_Provisioning/Sw/Drivers/esp32/Src/tiremo_esp32.c) — `BlufiStart` / `BlufiPoll` / `BlufiStop` / `EnableAutoConnect` / `JoinStoredAP`, artı eğitim 03'ün filtreli taraması ve DEBUG seviyesindeki TX dökümünde gizlenen `AT+CWJAP`.
3. [`tiremo_persist.h`](../../../../ESP32_Connectivity/Feature_Tutorials/05-BluFi_Provisioning/Sw/Common/Include/tiremo_persist.h) — `BL01` kaydı. Eğitim 03'ün `WF01`'i gibi, alınan bir şifreyi MCU'nun veri flash'ında düz metin olarak tutar.

## Tamamlanma Ölçütü

- EspBluFi, kaynak kodun hiçbir yerinde SSID olmadan kartı kuruyor ve log `BluFi WIFI GOT IP` ile `persist: saved SSID` gösteriyor.
- Bir resetten sonra log `persist: loaded SSID` ve `WIFI GOT IP (saved credentials)` gösteriyor — `BluFi started` satırı olmadan ve telefonda hiçbir şey yapmadan.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
