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

***Kart: Tiremo Cortex (ESP32) · Bölüm: ESP32 Connectivity / Feature Tutorials · Zorluk: Orta***

> Bu sayfa [İngilizce orijinalinin](../../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Bundan sonraki her ESP32 özelliği bu taşıma katmanına biner: güç sıralaması, UART RX halka tamponu, ve beklenti/zaman aşımı komut ilkelı. WiFi veya MQTT eklemeden önce bunu sağlamlaştırın. Bu eğitim üretim sürücüsünün tamamını kopyalamaz — yalnızca `Init` / `SendCommand` / `IsPresent` / `DisableEcho`. Join, NTP, BluFi ve MQTT aynı dosyaya sonraki eğitimlerde eklenir.

## Neler Öğreneceksiniz

- Modül güç döngüsü (kapalı → boot gecikmesi → AT probe)
- Kesme tabanlı RX halkası üzerinde alt dize + zaman aşımı komut ilkelı
- Sonraki yanıtları ayrıştırmadan önce yankının (`ATE0`) kapatılması gerektiği
- Debug log ve AT komutlarının iki ayrı UART olduğu (UART0 / UART2)

## İki UART'ı anlamak

MCU *sizinle* ve *radyoyla* iki farklı seri porttan konuşur. Karıştırmak ilk hatanın en yaygınıdır.

| Port | Pinler | Kim konuşur | Ne görürsünüz |
|------|--------|-------------|----------------|
| **UART0** | USB debug konsolu | `TIREMO_LOG_INFO` / `tdf.py monitor` | `[INFO] AT_Command_Transport: boot` |
| **UART2** | PA8 RX, PA9 TX | Modüldeki ESP-AT | `AT\r\n` sonra `OK` |

[`PRV_UART_Init()`](../../../../Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/user_uart.c) **ikisini de** (ve burada kullanılmayan UART1'i) saatler. Sonra iki Tiremo init IRQ'yu alır:

- `TIREMO_Debug_Init()` **UART0**'ı debug halkasıyla yeniden kurar (Getting Started 03).
- `TIREMO_ESP32_Init()` **UART2**'yi bu eğitimin AT halkasıyla yeniden kurar.

`tdf.py monitor`'a `AT` yazarsanız MCU loguna konuşursunuz, ESP32'ye değil. `TIREMO_Debug_Puts`'u UART2'ye yönlendirirseniz AT ayrıştırıcısını bozarsınız.

## Güç döngüsünü anlamak

ESP32, MCU açık diye açık değildir. PA12 bir **besleme kapısıdır**, active-high: `LOW` = modül kapalı, `HIGH` = açık. Önceki üretici yazılımından kalmış bir WiFi oturumu (veya yarı boot olmuş AT yığını) `AT`'ye çöp cevap verir. Temiz çözüm her zaman: kapat, bekle, aç, boot için bekle, *sonra* probe et.

Gecikmeler [`board_config.h`](../../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/Sw/Board/board_config.h) içindedir: **500 ms kapalı**, **3000 ms boot**. Bu 3.5 saniye yüzünden `tdf.py monitor` (önce reset, *sonra* portu aç) ilk satırları sık kaçırır. Üretici yazılımı çalışmıştır; terminal henüz dinlemiyordur.

`TIREMO_ESP32_Init()` sıradan önce PA12'yi push-pull çıkış yapar, UART2 IRQ'sunu kaydeder ve 1 baytlık receive kurar. `prvESP32ArmRx()` çalışmadan gelen AT baytlarının gidecek yeri yoktur.

## RX halkasını anlamak

ESP-AT yanıtları bir bayt patlamasıdır (`\r\nOK\r\n`, sonra tüm bir tarama listesi). ISR bunları ayrıştıramaz — diğer kesmeleri çok uzun tutardı. Bu yüzden ISR yalnızca **saklar**, ana döngü **ayrıştırır**.

1. `prvESP32ArmRx()` UART HAL'den `s_rxByte`'a **bir** bayt ister, blocking değil.
2. Bayt gelince `prvESP32UARTCallback` `RX_DONE` görür, 256 baytlık (2'nin kuvveti) halkaya iter, sonraki baytı kurar.
3. Hat hatası veya TX bittiğinde RX meşgulse yeniden kurar; bir glitch boruyu durdurmaz.

Halka 256 bayttır çünkü `AT` / `ATE0` yanıtları küçüktür. Eğitim 03'teki `CWLAP` taraması ve sonraki MQTT sertifika patlamaları daha büyük halka ister — o değişiklik oraya aittir, buraya değil.

## Beklenti / zaman aşımını anlamak

[`TIREMO_ESP32_SendCommand()`](../../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/Sw/Drivers/esp32/Include/tiremo_esp32.h) bu eğitimin tüm AT API'sidir. Üç argüman:

- `pCommand` — **olduğu gibi** gönderilecek baytlar. `"\r\n"` siz eklemelisiniz; ESP-AT çıplak `AT`'yi komut saymaz.
- `pExpected` — tam satır değil **alt dize**. `"OK"` hem `\r\nOK\r\n` hem gürültülü `WIFI GOT IP\r\nOK\r\n` ile eşleşir.
- `timeoutMs` — alt dize hiç gelmezse vazgeç.

Öğrencilerin takıldığı üç nokta:

1. **TX öncesi flush.** Boot sohbeti (`ready`, `WIFI DISCONNECT`) halkada kalıp sonraki komutun cevabı gibi görünmemeli.
2. **Satır değil alt dize eşleşmesi.** `OK` için rahat, aynı zamanda tuzak: yük hâlâ basılırken `"OK"` bekliyorsanız erken dönersiniz. Eğitim 02'deki `CWSTATE` tuzağı tam olarak budur. Bu eğitimde `"OK"` tüm yüktür, bu yüzden güvenlidir.
3. **Çerçeveli `ERROR` erken biter.** Modül `\r\nERROR\r\n` dediyse, gelmeyecek bir `OK` için `timeoutMs`'in geri kalanını beklemek boşa zamandır.

`SendCommandCapture` aynı döngüdür, artı birikmiş baytların kopyası — uygulama *bakabilsin* diye. `IsPresent()` yalnızca `SendCommand("AT\r\n", "OK", 1000)`.

Burada bir saniyeye kadar blocking sorun değil: henüz servis edilecek MQTT yok.

## Neden önce `ATE0`

ESP-AT, yankıyı kapatana kadar her komutu **yansıtır**. İlk `AT\r\n` kabaca şöyle döner:

```text
AT\r\n
\r\n
OK\r\n
```

`ATE0` sonrası aynı probe yalnızca:

```text
\r\n
OK\r\n
```

Yankı açık kalırsa `"OK"`, `"WIFI GOT IP"` veya `+SSID:` arayan sonraki ayrıştırıcılar, az önce gönderdiğiniz komutu da aynı tamponda görür. `prv_user_code.c` içindeki `ResponseContainsEcho()` baştaki CR/LF'i atlar ve yükün hâlâ `AT` ile başlayıp başlamadığına bakar — bu eğitimin kanıtı budur, üretim API'si değil.

`IsPresent()` sonrası `TIREMO_ESP32_DisableEcho()` bir kez çağrılır; modülü fabrika sıfırlamadıkça bir daha değil.

## Çözüm

```sh
cd Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

**Doğrulandı:** `.text` 14624, flaş OK.

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

`tdf.py monitor` önce resetleyip sonra portu açtığı için 3 s'lik boot gecikmesinin başını kaçırabilir.

## Adım adım

1. [`main.c`](../../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/Sw/Src/main.c) — Debug UART Logging ile aynı şekil: port, saatler, **`PRV_UART_Init()`**, sonra `PRV_USER_Code()`. `PRV_UART_Init()` olmadan UART2 saatlenmez.
2. [`board_config.h`](../../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/Sw/Board/board_config.h) — sürücünün bildiği tek kart bilgisi: UART2, PA12, polarite, 500 / 3000 ms.
3. [`tiremo_esp32.c`](../../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/Sw/Drivers/esp32/Src/tiremo_esp32.c) — taşıma katmanı. Üretim sürücüsünün **alt kümesi**.
4. [`prv_user_code.c`](../../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/Sw/Src/prv_user_code.c) — ders: Init → `IsPresent` → yankılı `AT` → `ATE0` → yankısız `AT` → 5 s heartbeat. Init/probe başarısızlığındaki `for (;;) { }` kasıtlıdır: sessiz reboot döngüsü bir kablolama hatasını gizler.

## Bitti sayılır

Her güç döngüsünden sonra `AT` güvenilir biçimde `OK` döner ve yakalanan yanıtlarda yankılanmış komut metni kalmaz — **doğrulandı**.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
