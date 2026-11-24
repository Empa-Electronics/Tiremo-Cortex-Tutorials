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
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# WiFi Station Join

***Kart: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Bölüm: ESP32 Connectivity / Feature Tutorials · Zorluk: Orta***

> Bu sayfa [İngilizce orijinalinin](../../../../ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Mümkün olan en basit WiFi bağlantısı: tek bir sabit ağ, tek bir bağlanma denemesi, yeniden deneme yok. Eğitim 01, UART'ın `AT` gönderip `OK` alabildiğini kanıtladı. Bu eğitim ilk *işe yarar* AT dizisidir — ve alt dize eşleşmesinin sizi yanıltabileceği ilk yerdir.

Yeniden denemeler, önce tarama ve flash'ta saklanan son çalışan SSID Eğitim 03'ün konusudur. Onları buraya eklemeyin.

## Neler Öğreneceksiniz

- `AT+CWMODE=1`, `AT+CWJAP`, `AT+CWSTATE?`
- `JoinAP`'in neden `"OK"` yerine `"WIFI GOT IP"` beklediği
- `IsConnected`'ın neden `"+CWSTATE:"` yerine `"+CWSTATE:2"` ile eşleştiği
- `GetConnectedSsid`'in neden `"+CWJAP:"` yerine `"OK"` ile eşleştiği — tırnak içindeki ad o işaretten *sonra*, bayt bayt gelir
- Başarısız bir bağlantının nasıl okunacağı (`+CWJAP:<kod>`)

## Ön Koşullar

- [01-AT_Command_Transport](../01-AT_Command_Transport/README.md)
- Adını ve şifresini bildiğiniz 2,4 GHz bir WiFi ağı (ESP32 yalnızca 2,4 GHz destekler)
- [Getting Started](../../../Getting_Started/) kurulum betikleri çalıştırılmış olmalı, böylece `tdf` kullanılabilir
- Log'u izlemek için: `tdf monitor` ya da kartın USB seri portunda 115200 8N1'de herhangi bir seri terminal (Tera Term, PuTTY, screen, minicom, …)
- USB kabloyla bilgisayarınıza bağlı bir Tiremo Cortex kartı (ABOV A34G43A MCU — bkz. [Donanıma Genel Bakış](../../../Hardware.md))

## Üç komutu anlamak

ESP-AT, WiFi'yi metinle yönettiğiniz küçük bir durum makinesi olarak ele alır:

| Komut | Anlamı | Bu eğitimin beklediği |
|-------|--------|------------------------|
| `AT+CWMODE=1` | yalnızca istasyon (station) modu, AP veya AP+STA değil | `"OK"` |
| `AT+CWJAP="ssid","pass"` | ağa bağlan + DHCP | `"WIFI GOT IP"` |
| `AT+CWSTATE?` | mevcut bağlantı durumu | `"+CWSTATE:2"` |

Durum 2, "bağlı, IPv4 adresi var" demektir. 0 henüz bağlantı başlatılmadığı, 1 AP'ye bağlı ama henüz IPv4 adresi olmadığı, 3 bağlanıyor ya da yeniden bağlanıyor olduğu, 4 bağlantının koptuğu anlamına gelir. `WIFI GOT IP`'den sonra `CWSTATE?` ile doğrulamak gereksiz değildir: bu bildirim (URC) gelebilir, ardından modül uygulamanız fark etmeden adresi kaybedebilir.

`AT+CWAUTOCONN=0` ve `AT+CWQAP` bağlanmadan *önce* çalışır. Modül son bağlandığı AP'yi kendi NVS'inde tutar ve aksi hâlde açılışta `wifi_secrets.h`'teki SSID'yi yok sayarak o ağa yeniden bağlanır. "Çalışanı hatırla" işi Eğitim 03'e aittir — ESP-AT'nin istenmeyen bir yan etkisi olarak değil, MCU'da.

## Alt dize tuzağını anlamak

Eğitim 01'deki `SendCommand`, `pExpected` **o ana kadar alınan baytların herhangi bir yerinde** görünür görünmez döner. ISR ise baytları **teker teker** iter. Bu ikisi birleşince, ilgilendiğiniz veri eşleşebileceğiniz bir etiketten *sonra* geldiği anda tuzağa düşersiniz.

Bu dosyadan üç somut örnek:

**1. `CWJAP` — `"OK"` ile değil, olayla eşleşin.**

Başarılı bir bağlantıda modül önce iki durum satırı bildirir, komutu ancak ondan sonra `OK` ile bitirir. Bu, kartta `TIREMO_DEBUG_LEVEL=4` ile yakalanan yanıttır ([Çözüm](#çözüm) bölümüne bakın); yakalama `WIFI GOT IP`'de durur çünkü `JoinAP`'in beklediği budur:

```text
{"data":"WIFI CONNECTED\r\nWIFI GOT IP"}
```

`OK` yalnızca "bu komut bitti" der — her komutun sonunda gelen aynı işarettir. Sizin asıl ihtiyacınız bir IPv4 adresidir ve bunu söyleyen satır `WIFI GOT IP`'dir. Bu yüzden `JoinAP` tam olarak bu olayı bekler:

```c
(void)snprintf(command, sizeof(command), "AT+CWJAP=\"%s\",\"%s\"\r\n",
               pSsid, pPassword);

return TIREMO_ESP32_SendCommand(command, "WIFI GOT IP", timeoutMs);
```

**2. `CWSTATE?` — `"+CWSTATE:"` ile eşleşmeyin.**

Yanıt `+`, `C`, `W`, … `:` ve en son rakam şeklinde bayt bayt gelir. Öneki eşlerseniz durum 0, 1, 3 ya da 4'te de 2'deki kadar rahat dönersiniz. `IsConnected` bu yüzden rakamı da eşler:

```c
bool TIREMO_ESP32_IsConnected(void)
{
    return TIREMO_ESP32_SendCommand("AT+CWSTATE?\r\n",
                                    "+CWSTATE:2",
                                    TIREMO_ESP32_STATUS_TIMEOUT_MS);
}
```

**3. `CWJAP?` — bu kez `"OK"` ile eşleşin, sonra ayrıştırın.**

`AT+CWJAP?` yanıtı şöyledir (kartta yakalandı; SSID ve BSSID değiştirildi):

```text
+CWJAP:"YourSSID","aa:bb:cc:dd:ee:ff",6,-54,0,1,3,0,1

OK
```

SSID ve AP'nin MAC adresinden (BSSID) sonra kanal (`6`) ve dBm cinsinden sinyal gücü (`-54`) gelir.

Tırnak içindeki SSID `+CWJAP:`'tan *sonra* durur. O işaretle eşleşmek, iki nokta geldiği anda döner — `GetConnectedSsid` hiçbir şey kopyalayamaz. Sondaki `"OK"` ile eşleşmek ise satırın tamamı yakalama tamponuna gelene kadar bekler; ardından `strstr` / `strchr` tırnakları ayıklar:

```c
if (!TIREMO_ESP32_SendCommandCapture("AT+CWJAP?\r\n", "OK",
                                     TIREMO_ESP32_STATUS_TIMEOUT_MS,
                                     response, sizeof(response)))
{
    return false;
}

pMarker = strstr(response, "+CWJAP:");
pStart = strchr(pMarker, '"');
pStart++;
pEnd = strchr(pStart, '"');
```

Pratik kural: ihtiyacınız olan veri aynı satırda bir etiketin **arkasındaysa**, yanıtın **sonuyla** (`"OK"`) eşleşin, sonra ayrıştırın. Veri etiketin **kendisiyse** (`WIFI GOT IP`, `+CWSTATE:2`), onunla eşleşin.

## Kimlik bilgileri (şifreyi commit etmeyin)

SSID ve şifre, örnekten oluşturacağınız `Sw/wifi_secrets.h` dosyasında durur. Bu dosya [`Sw/.gitignore`](../../../../ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/Sw/.gitignore) içinde listelidir, böylece şifreniz hiçbir zaman bir commit'e girmez. Repo kök dizininden:

```sh
cd ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/Sw
cp wifi_secrets.h.example wifi_secrets.h
```

(`cp` PowerShell, bash ve zsh'te çalışır; cmd.exe'de `copy wifi_secrets.h.example wifi_secrets.h` kullanın.) Ardından `wifi_secrets.h`'i açıp ağınızın bilgilerini girin:

```c
#define TIREMO_WIFI_SSID     "YourSSID"
#define TIREMO_WIFI_PASSWORD "YourPassword"
```

Düzenledikten sonra yeniden derleyin. Bu adımı unutursanız derleme `Copy Sw/wifi_secrets.h.example to Sw/wifi_secrets.h and set TIREMO_WIFI_SSID / TIREMO_WIFI_PASSWORD` hatasıyla durur.

Yazılım SSID'yi loglar, şifreyi loglamaz: varsayılan log seviyesinde `AT+CWJAP` komutunu hiç basmaz; `TIREMO_DEBUG_LEVEL=4`'te ise sürücü TX dökümünde bu komutun yerine `AT+CWJAP=<credentials hidden>` yazar. Yine de log'u paylaşmadan önce bunu aklınızda tutun — ağınızın adını içerir.

## Çözüm

Repo kök dizininden (`wifi_secrets.h`'i yukarıdaki gibi oluşturduktan sonra):

```sh
cd ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/Sw
tdf build
tdf flash
tdf monitor
```

(`tdf`'nin ne olduğu için [Getting_Started/01-Hello_Blink'in Çözüm bölümüne](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) bakın. macOS/Linux'ta `tdf.py` takma ad olarak da çalışır; Windows'ta `tdf` kullanın. `tdf monitor`'ü çalıştırmadan önce kartın seri portundaki diğer terminalleri kapatın — portu aynı anda yalnızca bir program açabilir.)

Yeni fonksiyonlar, ikinci bir UART yığınında değil, eğitim 01'deki aynı `tiremo_esp32.c` dosyasındadır.

**Gerçek donanımda doğrulandı:** `tdf build`, macOS'ta ve Windows 11'de hatasız derlenip bağlanıyor (`.text` yaklaşık 15 KB; kesin boyut toolchain sürümüne göre biraz değişir, ve linker [01-Hello_Blink'in Çözüm bölümünde](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) açıklanan beklenen ve zararsız `_close` / `_lseek` / `_read` / `_write is not implemented` uyarılarını basar). `tdf flash`, gerçek, bağlı bir Tiremo Cortex kartına karşı SWD üzerinden tam bir programlama-ve-doğrulama döngüsünü tamamladı (`** Verified OK **`) ve kart gerçek bir 2,4 GHz ağa bağlandı. Başarılı bir bağlantı şöyle görünür:

```text
[INFO] WiFi_Station_Join: boot
[INFO] ESP32: power cycling module...
[INFO] ESP32: power-on boot delay elapsed
[INFO] TIREMO_ESP32_Init: UART2 ready
[INFO] ESP32: command echo disabled (ATE0)
[INFO] ESP32: station mode
[INFO] ESP32: joining SSID "YourSSID"
[INFO] ESP32: WIFI GOT IP
[INFO] ESP32: CWSTATE=2 (connected, IPv4)
[INFO] ESP32: CWJAP? SSID="YourSSID"
[INFO] ESP32: heartbeat still connected
```

Bağlanma genellikle birkaç saniye sürer; yazılım 20 saniyeden sonra vazgeçer. Ardından her 5 saniyede bir `heartbeat still connected` satırı gelir.

### Bağlantı başarısız olursa

Bağlantı başarısız olursa ESP-AT `+CWJAP:<kod>` ve ardından `ERROR` yanıtı verir; bekleme 20 saniyeyi doldurmadan hemen biter. Varsayılan log seviyesinde şunu görürsünüz:

```text
[INFO] ESP32: joining SSID "YourSSID"
[WARN] ESP32 RX: "WIFI GOT IP" rejected with ERROR after 19 bytes
[ERROR] ESP32: CWJAP did not report WIFI GOT IP
```

Kodun kendisini görmek için `tdf build -DTIREMO_DEBUG_LEVEL=4` ile yeniden derleyip tekrar yükleyin; yanıt bu durumda `[WARN]` satırının altına dökülür. Var olmayan bir ağ adı için kartın verdiği yanıt:

```text
{"data":"+CWJAP:3\r\n\r\nERROR\r\n"}
```

| Kod | Anlamı | Neye bakmalı |
|-----|--------|--------------|
| `1` | bağlantı zaman aşımı | ağ çok uzakta ya da yoğun; yaklaşın |
| `2` | yanlış şifre | `wifi_secrets.h` içindeki `TIREMO_WIFI_PASSWORD` |
| `3` | ağ bulunamadı | SSID'nin yazımı (büyük/küçük harfe duyarlıdır) ve ağın 2,4 GHz olduğu |
| `4` | bağlantı başarısız | modemin ayarları (örneğin MAC filtresi) |

İşiniz bitince varsayılan seviyeye dönmek için `tdf build -DTIREMO_DEBUG_LEVEL=` (boş) çalıştırın.

## Adım Adım İnceleme

1. [`main.c`](../../../../ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/Sw/Src/main.c) — 01'den değişmedi: `PRV_UART_Init()` UART0 + UART2'nin saatini açar.
2. [`board_config.h`](../../../../ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/Sw/Board/board_config.h) — hâlâ yalnızca UART2 / PA12. Pin haritasına WiFi bilgisi girmez.
3. [`wifi_secrets.h`](../../../../ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/Sw/wifi_secrets.h.example) — şifrenin bulunduğu tek yer. Sürücünün değil, uygulamanın parçası.
4. [`tiremo_esp32.c`](../../../../ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/Sw/Drivers/esp32/Src/tiremo_esp32.c) — `SetStationMode`, `DisableAutoConnect`, `JoinAP`, `Disconnect`, `IsConnected`, `GetConnectedSsid` ekler. 01'deki taşıma katmanı değişmedi; tek fark, DEBUG seviyesindeki TX dökümünün `AT+CWJAP` komutunu gizlemesidir, böylece şifre log'a hiç ulaşmaz.
5. [`prv_user_code.c`](../../../../ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/Sw/Src/prv_user_code.c) — Init → modül var mı → `ATE0` → NVS'teki otomatik bağlanmayı kapat → `CWMODE=1` → `CWJAP` → `CWSTATE=2` → SSID'yi yazdır → 5 sn'lik kalp atışı. Hatalar yeniden başlatma yerine bir `[ERROR]` satırıyla durur; böylece yanlış bir şifre ekranda görünür kalır.

## Tamamlanma Ölçütü

Her resetten sonra kart ağınıza kendiliğinden bağlanıyor — `WIFI GOT IP`, `CWSTATE=2 (connected, IPv4)` ve ağınızın adıyla `CWJAP? SSID=` — ve `heartbeat still connected` her 5 saniyede bir gelmeye devam ediyor.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
