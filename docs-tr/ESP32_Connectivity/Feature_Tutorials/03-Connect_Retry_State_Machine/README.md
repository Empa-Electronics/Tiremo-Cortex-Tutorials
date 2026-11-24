<p class="sphinxhide" align="right"><a href="../../../../ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>Connect/Retry State Machine</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Connect/Retry State Machine

***Kart: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Bölüm: ESP32 Connectivity / Feature Tutorials · Zorluk: Orta***

> Bu sayfa [İngilizce orijinalinin](../../../../ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Eğitim 02 bir kez bağlanmayı dener ve başarısız olursa durur. Gerçek AP'ler paket kaybeder, DHCP gecikir, kayıtlı bir ağ kapsama dışında olabilir. Bu eğitim o bağlanmayı **önce tarama**, **üç deneme** ve **MCU flash'ında son çalışan kimlik bilgileri** ile sarar — neyin çalıştığını ESP32'nin NVS'i değil, MCU hatırlar.

NTP ve BluFi sonraki eğitimlerde gelir. Onları buraya eklemeyin.

## Neler Öğreneceksiniz

- `AT+CWLAP="<ssid>"` ile "bu SSID yayında mı?" hızlı kontrolü — ve filtresiz bir taramanın neden bir sonraki komutu bozduğu
- `JoinAP`'in neden boolean değil, **IP alındı / ERROR / zaman aşımı** döndürdüğü
- Son çalışan kaydı flash'tan ne zaman **silmeli**, ne zaman **silmeden yeniden denemeli**
- Sensors eğitimi 06'nın flash düzenini **farklı bir magic** ile yeniden kullanmak (`TRM1` değil, `WF01`)

## Ön Koşullar

- [02-WiFi_Station_Join](../02-WiFi_Station_Join/README.md)
- [06-Flash_Backed_Config](../../../Sensors_And_Storage/Feature_Tutorials/06-Flash_Backed_Config/README.md) (aynı CRC / sektör fikri)
- Eğitim 02'deki gibi adını ve şifresini bildiğiniz 2,4 GHz bir WiFi ağı (bu `Sw/` klasörünün kendi `wifi_secrets.h` dosyası gerekir — bkz. [Kimlik bilgileri](#kimlik-bilgileri-şifreyi-commit-etmeyin))
- [Getting Started](../../../Getting_Started/) kurulum betikleri çalıştırılmış olmalı, böylece `tdf` kullanılabilir
- Log'u izlemek için: `tdf monitor` ya da kartın USB seri portunda 115200 8N1'de herhangi bir seri terminal (Tera Term, PuTTY, screen, minicom, …)
- USB kabloyla bilgisayarınıza bağlı bir Tiremo Cortex kartı (ABOV A34G43A MCU — bkz. [Donanıma Genel Bakış](../../../Hardware.md))

## Önce tara, sonra bağlan

`AT+CWJAP`, AP ortadan kalktığını kabul etmeden önce 20 saniye bekleyebilir. Bir tarama ise birkaç saniyede yanıt verir; bu yüzden önce tarayın ve ağ yayında değilse `CWJAP`'ı hiç başlatmayın.

`AT+CWLAP="YourSSID"` yalnızca o ada sahip erişim noktalarını listeler — her AP için bir satır; birden fazla AP'si olan bir ağ (mesh ya da ofis ağı) birkaç kez görünür — ve `OK` ile biter. `ScanForSsid` bu `OK`'yi bekler, ardından yanıtta **tırnak içindeki** adı arar:

```c
(void)snprintf(command, sizeof(command), "AT+CWLAP=\"%s\"\r\n", pSsid);
(void)snprintf(expected, sizeof(expected), "\"%s\"", pSsid);

response[0] = '\0';
if (!TIREMO_ESP32_SendCommandCapture(command, "\r\nOK", timeoutMs,
                                     response, sizeof(response)))
{
    return false;
}

return (strstr(response, expected) != NULL);
```

Bu adda yayında hiçbir ağ yoksa yanıt yalnızca `OK`'dir ve fonksiyon `false` döner.

Neden düz bir `AT+CWLAP` gönderip ilk `"YourSSID"`'de durmuyoruz? Düz komut yakındaki **bütün** ağları listeler; bir ofiste bu kolayca bir düzine satırdır. Bizim adımız görünür görünmez dönmek, listenin geri kalanını hâlâ gelirken bırakır — ve o satırlar bir sonraki komutun, yani bağlanmanın, yanıt tamponuna düşer. Kalabalık bir ofiste gerçek bir kartta bu, her açılışta tam 20 saniyelik bir bağlanma zaman aşımına mal oldu (`511 bytes without "WIFI GOT IP"`: tampon artık tarama satırlarıyla doluydu). Daha kötüsü, listenin ilk 511 baytından sonra çıkan bir ad hiç görülmez ve kayıtlı bir ağ "kapsama dışı" sayılıp silinirdi. Tek bir SSID istemek ve `OK`'yi beklemek yanıtı kısa tutar ve arkada gelen veri bırakmaz.

RX halkası ve yakalama tamponu bu eğitimde **512** bayta çıkar. Filtrelenmiş bir `CWLAP` yanıtı AP başına yaklaşık 75 bayttır; yani aynı ada sahip altı AP'ye yer vardır.

## ERROR ile zaman aşımı farkını anlamak

Eğitim 02'nin `JoinAP`'i bir bool'du: `WIFI GOT IP` görüldü ya da görülmedi. Yalnızca başarı etiketini beklerseniz **yanlış şifre** de **yavaş DHCP** de aynı "false" gibi görünür.

Taşıma katmanı, çerçeveli bir `\r\nERROR\r\n` gördüğünde beklemeyi zaten keser (beklenen dize o değilse). Bu kesmeden sonra `JoinAP` sonucu sınıflandırır:

| Sonuç | Anlamı | Uygulamanın yaptığı |
|-------|--------|---------------------|
| `TIREMO_ESP32_JOIN_GOT_IP` | `WIFI GOT IP` | kimlik bilgilerini kaydet, kalp atışı |
| `TIREMO_ESP32_JOIN_ERROR` | modül bağlanmayı reddetti (yanlış şifre, geçersiz SSID biçimi, …) | flash'ı **sil**, boşta bekle |
| `TIREMO_ESP32_JOIN_TIMEOUT` | 20 sn sessizlik / ne GOT IP ne ERROR | yeniden dene (en fazla 3), üçü de başarısız olana kadar flash'ı **koru** |

Bir zaman aşımı anlık bir aksaklıktır. Art arda üç zaman aşımı ise "bu kayıtlı AP kullanılamıyor" demektir — o zaman silin. Çerçeveli bir ERROR kesindir: aynı şifreyle yeniden denemek işe yaramaz.

```c
if (TIREMO_ESP32_SendCommandCapture(command, "WIFI GOT IP", timeoutMs,
                                    response, sizeof(response)))
{
    return TIREMO_ESP32_JOIN_GOT_IP;
}

if (strstr(response, "\r\nERROR\r\n") != NULL)
{
    return TIREMO_ESP32_JOIN_ERROR;
}

return TIREMO_ESP32_JOIN_TIMEOUT;
```

## Son çalışan SSID'yi anlamak (`WF01`)

[`tiremo_persist.c`](../../../../ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/Sw/Common/Src/tiremo_persist.c), Sensors 06'daki desenin aynısıdır: DFMC sektör 0, magic + sürüm + `crc32`'den *önceki* her bayt üzerinde CRC32 (polinom `0xEDB88320`). Magic **`0x57463031` (`WF01`)**'dir, `TRM1` ya da `ENV1` değil; böylece bir logger kaydı ile bir WiFi kaydı birbirine karıştırılamaz.

Yine de **sektör 0'ı paylaşırlar**. Bu eğitimi yüklemek, Sensors 06'nın ya da çevresel logger'ın orada sakladığı her şeyin **üzerine yazar**. Bu beklenen bir durumdur: bir sektör, her yazılım imajında tek bir sahip.

SSID için açılış sırası:

1. Geçerli, SSID'si boş olmayan bir `WF01` kaydı var **ve** `wifi_secrets.h` hâlâ aynı SSID ve şifreyi içeriyor → kayıtlı olanı kullan (`persist: loaded SSID`).
2. Geçerli bir kayıt var ama `wifi_secrets.h` artık farklı bilgiler içeriyor → dosyayı son kayıttan sonra düzenlemişsiniz demektir, `wifi_secrets.h`'i kullan (`persist: wifi_secrets.h changed since the last save, using it`). Bu kontrol olmasaydı dosyayı düzenleyip yeniden yüklemenin hiçbir etkisi olmazdı: eski kayıtlı ağ kazanmaya devam ederdi ve `tdf erase-device` bu kaydın bulunduğu veri flash'ını silmez.
3. Geçerli kayıt yok → `wifi_secrets.h` (`persist: no saved SSID, using wifi_secrets.h`).

Bağlantı başarılı → `Persist_Save`. **Kayıtlı** bir SSID taramada görünmezse, çerçeveli ERROR'da ya da üç zaman aşımında → `Persist_Clear`. `wifi_secrets.h`'ten gelen bir SSID'nin taramada görünmemesi flash'ı silmez.

Bu eğitimde kayıt yalnızca `wifi_secrets.h`'ten gelebilir; bu yüzden 1. durum yalnızca kaydet/yükle döngüsünün çalıştığını doğrular. Desen asıl işe [05-BluFi_Provisioning](../05-BluFi_Provisioning/README.md)'de yarar; orada kimlik bilgileri bir telefondan da gelebilir.

`AT+CWAUTOCONN=0` ve `AT+CWQAP` eğitim 02'deki gibi yine önce çalışır: "çalışanı hatırla" işi modülün NVS'inde değil, **MCU**'dadır.

Kayıt, şifreyi MCU'nun veri flash'ında **düz metin** olarak tutar. Bir eğitim için sorun değildir, ama SWD probu olan herkes onu geri okuyabilir — bir ürün onu şifreler ya da korumalı bir alanda saklar.

## Kimlik bilgileri (şifreyi commit etmeyin)

Bu `Sw/` klasörünün kendi `wifi_secrets.h` dosyası gerekir; [eğitim 02'deki](../02-WiFi_Station_Join/README.md#kimlik-bilgileri-şifreyi-commit-etmeyin) gibi örnekten oluşturulur. Repo kök dizininden:

```sh
cd ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/Sw
cp wifi_secrets.h.example wifi_secrets.h
```

(cmd.exe'de `cp` yerine `copy` kullanın.) Ardından ağınızın SSID'sini ve şifresini girin. Eğitim 02'nin `Sw/` klasöründe hazırladığınız `wifi_secrets.h`'i de kopyalayabilirsiniz. Dosya [`Sw/.gitignore`](../../../../ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/Sw/.gitignore) içinde listelidir, yani hiçbir zaman bir commit'e girmez.

Yazılım, eğitim 02'deki gibi SSID'yi loglar ama şifreyi loglamaz: `TIREMO_DEBUG_LEVEL=4`'te sürücünün TX dökümü bağlanma komutunun yerine `AT+CWJAP=<credentials hidden>` gösterir.

## Çözüm

Repo kök dizininden (`wifi_secrets.h`'i yukarıdaki gibi oluşturduktan sonra):

```sh
cd ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/Sw
tdf build
tdf flash
tdf monitor
```

(`tdf`'nin ne olduğu için [Getting_Started/01-Hello_Blink'in Çözüm bölümüne](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) bakın. macOS/Linux'ta `tdf.py` takma ad olarak da çalışır; Windows'ta `tdf` kullanın. `tdf monitor`'ü çalıştırmadan önce kartın seri portundaki diğer terminalleri kapatın — portu aynı anda yalnızca bir program açabilir.)

**Gerçek donanımda doğrulandı:** `tdf build`, macOS'ta ve Windows 11'de hatasız derlenip bağlanıyor (`.text` yaklaşık 19,5 KB; kesin boyut toolchain sürümüne göre biraz değişir, ve linker [01-Hello_Blink'in Çözüm bölümünde](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) açıklanan beklenen ve zararsız `_close` / `_lseek` / `_read` / `_write is not implemented` uyarılarını basar). `tdf flash`, gerçek, bağlı bir Tiremo Cortex kartına karşı SWD üzerinden tam bir programlama-ve-doğrulama döngüsünü tamamladı (`** Verified OK **`) ve kart gerçek bir 2,4 GHz ağa ilk denemede bağlandı.

İlk açılış (henüz `WF01` kaydı yok, `wifi_secrets.h` doldurulmuş):

```text
[INFO] Connect_Retry_State_Machine: boot
[INFO] persist: no saved SSID, using wifi_secrets.h "YourSSID"
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
[INFO] ESP32: heartbeat still connected
```

Bir resetten sonra ilk `persist:` satırı değişir — kimlik bilgileri artık flash'tan gelir:

```text
[INFO] persist: loaded SSID "YourSSID"
```

Şimdi `wifi_secrets.h` içindeki `TIREMO_WIFI_SSID`'yi yayında olmayan bir ağın adıyla değiştirin, sonra yeniden derleyip yükleyin. Yeni dosya kayıtlı kayda göre önceliklidir, tarama hızla başarısız olur ve kart 3 × 20 saniyeyi bağlanma denemelerine harcamak yerine boşta bekler:

```text
[INFO] Connect_Retry_State_Machine: boot
[INFO] persist: wifi_secrets.h changed since the last save, using it: "SomeOtherSSID"
[INFO] ESP32: power cycling module...
[INFO] ESP32: power-on boot delay elapsed
[INFO] TIREMO_ESP32_Init: UART2 ready
[INFO] ESP32: command echo disabled (ATE0)
[INFO] ESP32: scanning for "SomeOtherSSID"
[WARN] ESP32: SSID not visible, skip join (fail fast)
[INFO] ESP32: idle, not connected
```

Bu SSID flash'tan değil `wifi_secrets.h`'ten geldiği için kayıtlı kayda dokunulmaz: eski SSID'yi geri yazıp tekrar yüklerseniz açılış `persist: loaded SSID` loglar ve bağlanır. Bunun yerine **kayıtlı** bir SSID kapsama dışına çıkarsa, aynı `SSID not visible` satırını `persist: cleared last-good SSID` izler.

## Adım Adım İnceleme

1. [`main.c`](../../../../ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/Sw/Src/main.c) — 01'den değişmedi: `PRV_UART_Init()` UART0 + UART2'nin saatini açar.
2. [`board_config.h`](../../../../ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/Sw/Board/board_config.h) — yalnızca UART2 / PA12.
3. [`wifi_secrets.h.example`](../../../../ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/Sw/wifi_secrets.h.example) — kayıt yokken (ya da daha eski bir kayıt varken) kullanılan kimlik bilgileri.
4. [`tiremo_hal_flash`](../../../../ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/Sw/Tiremo_Hal/Include/tiremo_hal_flash.h) + [`tiremo_persist`](../../../../ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/Sw/Common/Include/tiremo_persist.h) — sektör 0, `WF01`.
5. [`tiremo_esp32.c`](../../../../ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/Sw/Drivers/esp32/Src/tiremo_esp32.c) — 02'nin API'leri artı `ScanForSsid` (filtreli `CWLAP`, `OK`'yi bekler), `JoinAP`'te `JoinStatus`, 512 baytlık halka ve DEBUG seviyesindeki TX dökümünde gizlenen `AT+CWJAP` komutu.
6. [`prv_user_code.c`](../../../../ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/Sw/Src/prv_user_code.c) — kayıtlı kaydı ya da `wifi_secrets.h`'i seç → Init → `ATE0` → NVS'teki otomatik bağlanmayı kapat → tara → 1..3 kez bağlan → kaydet ya da sil → kalp atışı ya da boşta bekle. Taşıma katmanı / başlatma hataları yine bir `[ERROR]` satırıyla durur.

## Tamamlanma Ölçütü

- Kapsamadaki bir ağa bağlanılıyor (normalde ilk denemede) ve bir sonraki reset `persist: loaded SSID` logluyor.
- `wifi_secrets.h`'i değiştirip yeniden yükledikten sonra açılış `persist: wifi_secrets.h changed since the last save, using it` logluyor.
- Yayında olmayan bir SSID `SSID not visible, skip join (fail fast)` logluyor ve `CWJAP` hiç başlamıyor.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
