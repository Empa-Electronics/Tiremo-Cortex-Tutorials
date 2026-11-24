<p class="sphinxhide" align="right"><a href="../../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>AT Command Transport</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# AT Command Transport

***Kart: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Bölüm: ESP32 Connectivity / Feature Tutorials · Zorluk: Orta***

> Bu sayfa [İngilizce orijinalinin](../../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Bundan sonraki her ESP32 özelliği bu taşıma katmanının üzerinde çalışır: güç sıralaması, bir UART RX halka tamponu ve beklenen-yanıt/zaman aşımı komut temel işlemi. WiFi ya da MQTT'yi üzerine kurmadan önce bunu sağlamlaştırın. Bu eğitimdeki sürücü bilerek küçük tutulmuştur — yalnızca Init / SendCommand / IsPresent / DisableEcho. Join, NTP, BluFi ve MQTT, sonraki eğitimlerde aynı dosyaya eklenen fonksiyonlardır.

## Neler Öğreneceksiniz

- Modülün güç döngüsü sırası (kapat → boot gecikmesi → AT yoklaması)
- Kesme tabanlı bir RX halka tamponu üzerinde "alt dizeyi zaman aşımına kadar bekle" komut temel işlemi
- Sonraki yanıtları ayrıştırmadan önce yankının (`ATE0`) neden kapatılması gerektiği
- Debug log'unun ve AT komutlarının iki ayrı UART'tan geçtiği (UART0 ve UART2)

## Ön Koşullar

- [Getting_Started](../../../Getting_Started/) tamamlanmış olmalı — SysTick, debug UART ve `tdf` bilindiği varsayılır
- Log'u izlemek için: `tdf monitor` ya da kartın USB seri portunda 115200 8N1'de herhangi bir seri terminal (Tera Term, PuTTY, screen, minicom, …)
- USB kabloyla bilgisayarınıza bağlı bir Tiremo Cortex kartı (ABOV A34G43A MCU — bkz. [Donanıma Genel Bakış](../../../Hardware.md)). ESP32 modülü kartın üzerindedir; UART2'ye (PA8/PA9) bağlıdır ve beslemesi PA12 ile açılıp kapatılır. Kart üzerindeki CMSIS-DAP hata ayıklama probu SWD ile programlamayı yapar; ayrı bir programlayıcı gerekmez.

## İki UART'ı anlamak

MCU *sizinle* ve *radyoyla* iki farklı seri porttan konuşur. Bunları karıştırmak en yaygın ilk hatadır.

| Port | Pinler | Kim konuşur | Ne görürsünüz |
|------|--------|-------------|----------------|
| **UART0** | USB debug konsolu | `TIREMO_LOG_INFO` / `tdf monitor` | `[INFO] AT_Command_Transport: boot` |
| **UART2** | PA8 RX, PA9 TX | Modüldeki ESP-AT yazılımı | `AT\r\n`, ardından `OK` |

[`PRV_UART_Init()`](../../../../Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/user_uart.c) **ikisinin de** (ve burada kullanılmayan UART1'in) saatini açar. Ardından iki Tiremo init fonksiyonu IRQ'ları devralır:

- `TIREMO_Debug_Init()` **UART0**'ı debug halkasıyla yeniden kurar (Getting Started 03).
- `TIREMO_ESP32_Init()` **UART2**'yi bu eğitimin AT halkasıyla yeniden kurar.

Bilgisayarınız yalnızca UART0'ı görür: `tdf monitor` yalnızca MCU'nun logladıklarını gösterir; bir seri terminalde USB portuna `AT` yazsanız bile bu, ESP32'ye değil MCU'nun debug UART'ına gider. Modülle yalnızca yazılım UART2 üzerinden konuşur. UART2'ye `TIREMO_Debug_Puts` ile yazarsanız AT ayrıştırıcısını bozarsınız. Bu bölümün geri kalanında bu iki hattı ayrı tutun.

## Güç döngüsünü anlamak

ESP32, MCU'ya güç geldi diye açılmaz. PA12 active-high bir **besleme kapısıdır**: `LOW` = modül kapalı, `HIGH` = modül açık. Önceki yazılımdan kalmış bir WiFi oturumu (ya da yarı boot olmuş bir AT yığını) `AT`'ye anlamsız bir yanıt verir. Temiz çözüm her zaman aynıdır: kapat, bekle, aç, boot'u bekle, *sonra* yokla.

[`tiremo_esp32.c`](../../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/Sw/Drivers/esp32/Src/tiremo_esp32.c) dosyasından:

```c
static void prvESP32PowerCycle(void)
{
    TIREMO_LOG_INFO("ESP32: power cycling module...");

    (void)TIREMO_HAL_GPIO_WritePin(TIREMO_ESP32_PWR_PORT,
                                   TIREMO_ESP32_PWR_PIN,
                                   TIREMO_ESP32_PWR_OFF);
    TIREMO_SysTick_DelayMs(TIREMO_ESP32_PWR_OFF_MS);

    (void)TIREMO_HAL_GPIO_WritePin(TIREMO_ESP32_PWR_PORT,
                                   TIREMO_ESP32_PWR_PIN,
                                   TIREMO_ESP32_PWR_ON);
    TIREMO_SysTick_DelayMs(TIREMO_ESP32_PWR_BOOT_MS);

    TIREMO_LOG_INFO("ESP32: power-on boot delay elapsed");
}
```

Gecikmeler [`board_config.h`](../../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/Sw/Board/board_config.h) içindedir: **500 ms kapalı**, **3000 ms boot**. Bu yüzden `ESP32: power cycling module...` ile `ESP32: power-on boot delay elapsed` arasında yaklaşık 3,5 saniyelik bir duraklama bekleyin — bu, modülün boot etmesidir, takılma değil.

`TIREMO_ESP32_Init()` bu sıradan önce PA12'yi push-pull çıkış yapar; sonrasında UART2 IRQ'sunu kaydeder ve 1 baytlık bir alım başlatır. `prvESP32ArmRx()` çalışana kadar gelen AT baytlarının gidecek yeri yoktur.

## RX halkasını anlamak

ESP-AT yanıtları bir bayt patlaması olarak gelir (`\r\nOK\r\n`, ileride bütün bir tarama listesi). ISR bunları ayrıştıramaz — diğer kesmeleri çok uzun süre bekletirdi. Bu yüzden ISR yalnızca **saklar**, ana döngü **ayrıştırır**.

1. `prvESP32ArmRx()`, UART HAL'den `s_rxByte` içine **bir** bayt ister (bloklamadan).
2. O bayt gelince `prvESP32UARTCallback` `RX_DONE` olayını görür, baytı 256 baytlık (2'nin kuvveti) halkaya iter ve bir sonraki baytı ister.
3. Bir hat hatası ya da RX meşgulken gelen TX-bitti olayı da alımı yeniden başlatır; böylece bir parazit hattı durdurmaz.

```c
static void prvESP32UARTCallback(uint32_t event, void *pContext)
{
    (void)pContext;

    if (!s_initialized)
    {
        return;
    }

    if ((event & TIREMO_HAL_UART_EVENT_RX_DONE) != 0U)
    {
        prvESP32RxRingPush(s_rxByte);
        prvESP32ArmRx();
    }

    if ((event & TIREMO_HAL_UART_EVENT_LINE_ERROR) != 0U)
    {
        prvESP32ArmRx();
    }

    if (((event & TIREMO_HAL_UART_EVENT_TX_DONE) != 0U) && s_rxArmPending)
    {
        prvESP32ArmRx();
    }
}
```

Halka 256 bayttır çünkü `AT` / `ATE0` yanıtları çok kısadır. Eğitim 03'teki `CWLAP` taraması ve sonraki MQTT sertifika patlamaları daha büyük bir halka ister — o değişiklik oraya aittir, buraya değil.

## Beklenen yanıt / zaman aşımını anlamak

[`TIREMO_ESP32_SendCommand()`](../../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/Sw/Drivers/esp32/Include/tiremo_esp32.h) bu eğitimin AT API'sinin tamamıdır. Şunları verirsiniz:

- `pCommand` — **olduğu gibi** gönderilecek baytlar. `"\r\n"`'yi siz eklemelisiniz; ESP-AT çıplak bir `AT`'yi komut olarak kabul etmez.
- `pExpected` — tam bir satır değil, bir **alt dize**. `"OK"` hem `\r\nOK\r\n` ile hem de gürültülü bir `WIFI GOT IP\r\nOK\r\n` ile eşleşir.
- `timeoutMs` — alt dize hiç gelmezse vazgeçme süresi.

Uygulama tek bir bloklayan fonksiyondur; loglama ve yanıt kopyalama çıkarılmış hâliyle çekirdeği:

```c
if (pCommand != NULL)
{
    prvESP32RxRingFlush();
    /* ... TIREMO_HAL_UART_Transmit(pCommand) on UART2 ... */
}

response[0] = '\0';
startMs = TIREMO_SysTick_GetMs();

while ((!matched) && (!errored) &&
       ((TIREMO_SysTick_GetMs() - startMs) < timeoutMs))
{
    while ((!matched) && (!errored) && prvESP32RxRingPop(&byte))
    {
        if (length < (sizeof(response) - 1U))
        {
            response[length++] = (char)byte;
            response[length] = '\0';
        }

        if (strstr(response, pExpected) != NULL)
        {
            matched = true;
        }
        else if ((strcmp(pExpected, "\r\nERROR\r\n") != 0) &&
                 (strstr(response, "\r\nERROR\r\n") != NULL))
        {
            errored = true;
        }
    }
}
```

Öğrencilerin takıldığı üç ayrıntı:

1. **Göndermeden önce temizle.** Boot sırasında gelen artıklar (`ready`, `WIFI DISCONNECT`) halkada kalıp bir sonraki komutun yanıtı gibi görünmemelidir.
2. **Satır değil, alt dize eşleşmesi.** Bu `OK` için kullanışlıdır ama aynı zamanda bir tuzaktır: modül hâlâ bir veri basarken o verinin içinde tesadüfen bu iki harf geçiyorsa ve siz `"OK"` bekliyorsanız, erken dönersiniz. Eğitim 02'deki `CWSTATE` tuzağı tam olarak budur. Bu eğitimde `"OK"` yanıtın tamamı olduğu için güvenlidir.
3. **Çerçeveli `ERROR` beklemeyi keser.** Modül `\r\nERROR\r\n` yanıtı verirse, hiç gelmeyecek bir `OK` için `timeoutMs`'in geri kalanını beklemek boşa zamandır. `\r\n` çerçevesi, sonradan gelecek bir sertifika verisinin içindeki `ERROR` kelimesiyle eşleşmeyi önler.

`SendCommandCapture` aynı döngüdür, artı uygulamanın biriken baytlara *bakabilmesi* için bir kopya. `IsPresent()` ise yalnızca `SendCommand("AT\r\n", "OK", 1000)`'dir.

Burada bir saniyeye kadar bloklamak sorun değildir: henüz servis edilecek bir MQTT yok. Gerçek bir olay döngüsü de bu aynı temel işlemi kullanır, yalnızca durum sorgularında daha kısa zaman aşımlarıyla.

## Neden önce `ATE0`

ESP-AT, yankıyı kapatana kadar her komutu geri **yansıtır**. Bu yüzden ilk `AT\r\n` kabaca şöyle döner:

```text
AT\r\n
\r\n
OK\r\n
```

`ATE0`'dan sonra aynı yoklama yalnızca şudur:

```text
\r\n
OK\r\n
```

Yankıyı açık bırakırsanız, ileride `"OK"`, `"WIFI GOT IP"` ya da `+SSID:` arayan ayrıştırıcılar az önce gönderdiğiniz komutu da aynı tamponda görür. `prv_user_code.c` içindeki `ResponseContainsEcho()` baştaki CR/LF karakterlerini atlar ve yanıtın hâlâ `AT` ile başlayıp başlamadığına bakar — bu, bu eğitimin göstermek için var olduğu kanıttır, bir üretim API'si değil.

`TIREMO_ESP32_DisableEcho()`'yu `IsPresent()`'ten sonra bir kez çağırın; modülü fabrika ayarlarına döndürmedikçe bir daha çağırmayın.

## Çözüm

Repo kök dizininden:

```sh
cd ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/Sw
tdf build
tdf flash
tdf monitor
```

(`tdf`'nin ne olduğu için [Getting_Started/01-Hello_Blink'in Çözüm bölümüne](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) bakın. macOS/Linux'ta `tdf.py` takma ad olarak da çalışır; Windows'ta `tdf` kullanın. `tdf monitor`'ü çalıştırmadan önce kartın seri portundaki diğer terminalleri kapatın — portu aynı anda yalnızca bir program açabilir.)

**Gerçek donanımda doğrulandı:** `tdf build`, macOS'ta ve Windows 11'de hatasız derlenip bağlanıyor (`.text` yaklaşık 14,5 KB; kesin boyut toolchain sürümüne göre biraz değişir, ve linker [01-Hello_Blink'in Çözüm bölümünde](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) açıklanan beklenen ve zararsız `_close` / `_lseek` / `_read` / `_write is not implemented` uyarılarını basar). `tdf flash`, gerçek, bağlı bir Tiremo Cortex kartına karşı SWD üzerinden tam bir programlama-ve-doğrulama döngüsünü tamamladı (`** Verified OK **`). Açılış log'u:

```text
[INFO] AT_Command_Transport: boot
[INFO] ESP32: power cycling module...
[INFO] ESP32: power-on boot delay elapsed
[INFO] TIREMO_ESP32_Init: UART2 ready
[INFO] ESP32: AT -> OK (module present)
[INFO] ESP32: AT (echo still on) capture starts with echo=yes
[INFO] ESP32: command echo disabled (ATE0)
[INFO] ESP32: AT after ATE0 is echo-free (OK only)
[INFO] ESP32: heartbeat AT -> OK
```

Her 5 saniyede bir `heartbeat AT -> OK` satırı gelir.

Hattaki ham baytları görmek için `tdf build -DTIREMO_DEBUG_LEVEL=4` ile yeniden derleyip tekrar yükleyin. Bu durumda her komut ve yanıtı JSON-kaçışlı dizeler olarak dökülür. İkinci `AT` (yankı hâlâ açık), ardından `ATE0` ve ondan sonraki `AT`:

```text
[DEBUG] ESP32 TX (4 bytes):
{"data":"AT\r\n"}
[DEBUG] ESP32 RX matched "OK" after 8 bytes:
{"data":"AT\r\n\r\nOK"}
[INFO] ESP32: AT (echo still on) capture starts with echo=yes
[DEBUG] ESP32 TX (6 bytes):
{"data":"ATE0\r\n"}
[DEBUG] ESP32 RX matched "OK" after 10 bytes:
{"data":"ATE0\r\n\r\nOK"}
[INFO] ESP32: command echo disabled (ATE0)
[DEBUG] ESP32 TX (4 bytes):
{"data":"AT\r\n"}
[DEBUG] ESP32 RX matched "OK" after 4 bytes:
{"data":"\r\nOK"}
[INFO] ESP32: AT after ATE0 is echo-free (OK only)
```

İlk yanıtın başındaki yankılanmış `AT\r\n`, `ATE0`'ın kaldırdığı şeyin ta kendisidir. (Döküm `OK`'de biter çünkü okuma, beklenen alt dize görülür görülmez döner.) Varsayılan seviyeye dönmek için `tdf build -DTIREMO_DEBUG_LEVEL=` (boş) çalıştırın.

## Adım Adım İnceleme

Dosyaları bu sırayla okuyun — her biri tek bir fikir ekler.

1. [`main.c`](../../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/Sw/Src/main.c) — Debug UART Logging ile aynı yapı: port başlatma, saatler, **`PRV_UART_Init()`**, ardından `PRV_USER_Code()`. `PRV_UART_Init()` olmadan UART2'nin saati açılmaz ve `TIREMO_ESP32_Init()` IRQ kaydında başarısız olur.

2. [`board_config.h`](../../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/Sw/Board/board_config.h) — sürücünün bilmesine izin verilen tek kart bilgisi: UART2, PA12, açık/kapalı polaritesi, 500 ms / 3000 ms. İleride bir kartta besleme pini değişirse bu dosya değişir, `tiremo_esp32.c` değişmez.

3. [`tiremo_esp32.h`](../../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/Sw/Drivers/esp32/Include/tiremo_esp32.h) / [`.c`](../../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/Sw/Drivers/esp32/Src/tiremo_esp32.c) — taşıma katmanı, en azda tutulmuş hâliyle: Init, SendCommand, IsPresent, DisableEcho. WiFi bağlantısı, NTP, BluFi ve MQTT, sonraki eğitimlerde bu aynı dosyaya eklenen fonksiyonlardır; ikinci bir UART yığını değil.

4. [`prv_user_code.c`](../../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/Sw/Src/prv_user_code.c) — sürücü değil, dersin kendisi:
   - SysTick + debug UART (log'ların bir saati ve bir konsolu olsun diye)
   - `TIREMO_ESP32_Init()` (güç döngüsü + halka)
   - `IsPresent()` — başarısız olursa dur. `OK` olmadan yapılacak işe yarar bir şey yok.
   - `ATE0`'dan **önce** ikinci bir `AT` yakala ve `echo=yes` logla
   - `DisableEcho()`
   - `AT`'yi tekrar yakala ve `echo-free` logla
   - Sonsuza dek: her 5 saniyede bir kalp atışı olarak `IsPresent()` — modül UART2'de yanıt vermeyi keserse `OK` yerine `[WARN] ESP32: heartbeat AT failed` görürsünüz

Init/yoklama başarısızlığındaki `for (;;) { }` bilinçlidir: sessiz bir yeniden başlatma döngüsü bir kablolama hatasını gizlerdi. Donanımı (ya da PA12 polaritesini) düzeltin, sonra resetleyin.

## Tamamlanma Ölçütü

Her resetten sonra `tdf monitor`'de `ESP32: AT -> OK (module present)` görünüyor, `ATE0`'dan önceki yakalama `echo=yes`, sonrakisi `echo-free (OK only)` bildiriyor ve `heartbeat AT -> OK` her 5 saniyede bir gelmeye devam ediyor.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
