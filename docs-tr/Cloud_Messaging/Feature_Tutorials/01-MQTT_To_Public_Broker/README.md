<p class="sphinxhide" align="right"><a href="../../../../Cloud_Messaging/Feature_Tutorials/01-MQTT_To_Public_Broker/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>MQTT to a Public Broker</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# MQTT to a Public Broker

***Kart: Tiremo Cortex (ESP32) · Bölüm: Cloud Messaging (MQTT & TLS) / Feature Tutorials · Zorluk: Orta***

> Bu sayfa [İngilizce orijinalinin](../../../../Cloud_Messaging/Feature_Tutorials/01-MQTT_To_Public_Broker/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Sertifikalara dokunmadan önce, ESP32'nin yerleşik MQTT istemcisinin
gerçekten çalıştığını kanıtlayın — açık metinde bir genel test broker'ına
bağlanın, bir mesaj yayınlayın, ve o mesajın geri yankılandığını duymak
için abone olun. [`iot.tiremo.ai`](https://iot.tiremo.ai) karşılıklı TLS
eğitim 03'tür — bkz. [TLS SNI](https://docs.tiremo.ai/device-provisioning/tls-sni).

## Neler Öğreneceksiniz

- `AT+MQTTUSERCFG` (scheme 1, TLS yok) / `AT+MQTTCONN` / `AT+MQTTSUB` / `AT+MQTTPUB`
- `MqttPublish` (string payload) ile `MqttPublishRaw` (binary-safe) arasındaki fark
- Bunu inşa ederken bulunan ve düzeltilen gerçek bir hata: ESP-AT'nin MQTT `scheme` parametresi 0-indeksli değil, 1-indeksli

## Ön Koşullar

- [ESP32 Connectivity](../../../../ESP32_Connectivity/README.md) — WiFi bağlantısı çalışıyor
- Bu `Sw/`'de `wifi_secrets.h` (gitignored), ESP32 Connectivity ile aynı desen
- Toolchain ve flaşlama [Getting Started](../../../../Getting_Started/README.md)'a göre kurulu
- Yayınlama/yankı döngüsünü izlemek için `tdf.py monitor` (kurulum için [Getting_Started/04-Button_Read_And_Debounce'un Çözüm bölümüne](../../../../Getting_Started/04-Button_Read_And_Debounce/README.md#solution) bakın)

## Gerçek Bir Hata: `scheme 0` Diye Bir Şey Yok

Bu eğitimin taslak anahatı (hiçbir şey inşa edilmeden önce yazılmıştı)
"scheme 0, TLS yok" diyordu — makul bir tahmin, çünkü bu kod tabanının
başka her yerinde 0-indeksli enum'lar norm. İlk gerçek derleme bu
tahminin yanlış olduğunu hemen kanıtladı:

```text
[DEBUG] ESP32 TX: {"data":"AT+MQTTUSERCFG=0,0,\"tiremo-cortex-01\",\"\",\"\",0,0,\"\"\r\n"}
[WARN] ESP32 RX: "OK" rejected with ERROR after 9 bytes
{"data":"\r\nERROR\r\n"}
```

Dokuz bayt tam olarak `\r\nERROR\r\n` — modül komutu doğrudan reddetti,
bir bağlantı hatası değil. Ham AT trafiğini görmek için
`-D TIREMO_DEBUG_LEVEL=4` ile yeniden derlemek (yukarıda) gönderilen
tam baytların sözdizimsel olarak doğru biçimlendiğini doğruladı, bu da
sorunu `scheme` değerinin kendisine daralttı.
[Espressif'in kendi ESP-AT MQTT komut referansını](https://github.com/espressif/esp-at/blob/master/docs/en/AT_Command_Set/MQTT_AT_Commands.rst)
kontrol etmek bunu çözdü: `scheme` **1-indeksli** — `1` düz TCP, `2`–`5`
TLS varyantları (`5` karşılıklı TLS, Tutorial 03'ün taslağının bu
değer konusunda haklı olduğunu doğruluyor), `6`–`10` WebSocket
varyantları. `scheme 0` diye bir şey yok.
[`Sw/Drivers/esp32/Src/tiremo_esp32.c`](../../../../Cloud_Messaging/Feature_Tutorials/01-MQTT_To_Public_Broker/Sw/Drivers/esp32/Src/tiremo_esp32.c)'nin
`TIREMO_ESP32_MqttUserConfig()`'inde tek bir rakamı düzeltmek hatanın
tamamıydı.

Aynı debug izi ikinci, daha küçük bir uyuşmazlığı da ortaya çıkardı:
`AT+MQTTPUBRAW`'ın gerçek tamamlama işareti `+MQTTPUB:OK` /
`+MQTTPUB:FAIL`, düz bir `OK` değil (aşağıdaki izdeki o ilk `OK`,
modül ham baytları `>` ile istemeden önce, sadece komutun kabul
edildiğini gösteriyor):

```text
[DEBUG] ESP32 RX matched ">" after 9 bytes: {"data":"\r\nOK\r\n\r\n>"}
[DEBUG] ESP32 RX matched "+MQTTPUB:OK" after 11 bytes: {"data":"+MQTTPUB:OK"}
```

Düz bir `"OK"` eşleşmesi, teknik olarak burada tesadüfen yine de
çalışırdı — `"+MQTTPUB:OK"`, `"OK"`'yı bir alt dize olarak içeriyor.
Gerçek işareti eşleştirmek ise başarıyı `+MQTTPUB:FAIL`'den gerçekten
ayırt eden şey, bu yüzden sürücü bu tesadüfe güvenmek yerine ona
açıkça eşleşiyor.

## Çözüm

[`Sw/`](../../../../Cloud_Messaging/Feature_Tutorials/01-MQTT_To_Public_Broker/Sw/) gerçek, derlenebilir bir uygulamaya sahip:

```sh
cd Sw
cp wifi_secrets.h.example wifi_secrets.h   # once; then edit
tdf.py build
tdf.py flash
tdf.py monitor
```

ESP32 Connectivity ile aynı CMake şekli; `GEN_DIR` dört seviye yukarıda
[`Generation/`](../../../../Generation/)'a ulaşıyor. ESP32 sürücüsü
([`tiremo_esp32.c`](../../../../Cloud_Messaging/Feature_Tutorials/01-MQTT_To_Public_Broker/Sw/Drivers/esp32/Src/tiremo_esp32.c))
ESP32 Connectivity Tutorial 02'nin WiFi-join alt kümesi (tam BluFi
yığını değil — bu eğitim ona ihtiyaç duymuyor), aynı dosyaya eklenen
MQTT fonksiyonlarıyla birlikte, o bölümün "bir büyüyen sürücü, ikinci
bir UART yığını değil" desenini sürdürerek. Referans üretici
yazılımının hiçbir yerinde MQTT/TLS kodu yok — bu sürücünün MQTT
yarısı orijinal, gerçek ESP-AT komut setine karşı inşa edildi ve
hatası ayıklandı.

**Bağlı kartta gerçekten, gerçek hayatta doğrulandı — ve gerçek bir
genel broker üzerinden bağımsız olarak da:** `tdf.py build` temiz
şekilde derlenip bağlanıyor (`.text` 17268 bayt). `tdf.py flash`, SWD
üzerinden tam bir programlama-ve-doğrulama döngüsünü tamamladı
(`** Verified OK **`). `tdf.py monitor` ardından, doğrudan gerçek
donanımdan şunu gösterdi:

```text
[INFO] ESP32: WIFI GOT IP
[INFO] MQTT: client id "tiremo-cortex-01" configured (scheme 1, no TLS)
[INFO] MQTT: connecting to broker.emqx.io:1883
[INFO] MQTT: connected
[INFO] MQTT: subscribed to "tiremo-cortex/mqtt-demo"
[INFO] MQTT: publishing (string) "hello from Tiremo Cortex #0"
[INFO] MQTT: echo received on "tiremo-cortex/mqtt-demo"
[INFO] MQTT: echo payload: 27 bytes "hello from Tiremo Cortex #0"
[INFO] MQTT: publishing (raw) counter=1 as 4 bytes
[INFO] MQTT: echo received on "tiremo-cortex/mqtt-demo"
[INFO] MQTT: echo payload: 4 bytes "...."
```

her 5 saniyede bir tekrarlanıyor, string (`MqttPublish`) ve
binary-safe (`MqttPublishRaw`, dört ham sayaç baytı yazdırılabilir
ASCII olmadığı için `....` olarak loglanıyor — beklenen, hata değil)
yayınları arasında geçiş yapıyor.

Bu tek başına bir gidiş-dönüş kanıtlıyor, ama mesajın gerçekten bu
makineden çıktığını kanıtlamıyor. Bu yüzden bu ayrıca bağımsız olarak
da kontrol edildi: bir `mqttx` CLI abonesi
(`mqttx sub -h broker.emqx.io -t tiremo-cortex/mqtt-demo`), bu üretici
yazılımıyla ya da bu depoyla hiçbir bağlantısı olmayan tamamen ayrı bir
MQTT istemcisi, aynı anda çalıştırıldı ve gerçekten aynı mesajları aldı
— `hello from Tiremo Cortex #6`, `#8`, ve ham `00 00 00 07` /
`00 00 00 09` sayaç baytları — bunun gerçekten açık internet üzerinden
`broker.emqx.io`'dan geçtiğini, yerel bir loopback olmadığını
doğruluyor.

## Adım Adım

### 1. MQTT kimliği — `TIREMO_ESP32_MqttUserConfig()`

```c
(void)snprintf(command, sizeof(command),
               "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",
               pClientId, pUsername, pPassword);
```

LinkID `0` (bu sürücü yalnızca tek bir MQTT bağlantısı kullanır),
scheme `1` (düz TCP — yukarıdaki **Gerçek Bir Hata**'ya bakın), boş
kullanıcı adı/parola (bu genel broker hiçbirini gerektirmiyor), sertifika/CA
yok (slot `0`, yol `""` — bunlar Tutorial 02'den itibaren önem
kazanıyor).

### 2. Bağlan ve abone ol

```c
TIREMO_ESP32_MqttConnect("broker.emqx.io", 1883, 10000U);
TIREMO_ESP32_MqttSubscribe("tiremo-cortex/mqtt-demo", 0U);
```

İkisi de sıradan `AT+MQTTCONN` / `AT+MQTTSUB`, `"OK"` üzerinde
eşleşiyor — burada yeni bir yanıt-ayrıştırma numarası gerekmiyor.

### 3. Yayınlamanın iki yolu

[`Sw/Src/prv_user_code.c`](../../../../Cloud_Messaging/Feature_Tutorials/01-MQTT_To_Public_Broker/Sw/Src/prv_user_code.c),
aynı konu üzerinde ikisi arasında geçiş yapar:

```c
TIREMO_ESP32_MqttPublish(topic, "hello from Tiremo Cortex #0", 0, false);
/* vs. */
uint8_t counter[4] = { 0, 0, 0, 1 };
TIREMO_ESP32_MqttPublishRaw(topic, counter, 4, 0, false);
```

`MqttPublish`, payload'ı AT-komut tırnakları içinde gönderir — basit,
ama payload'ın asla `"`, `\r`, ya da `\n` içeremeyeceği anlamına gelir.
`MqttPublishRaw` bunun yerine onu ham, uzunluk-önekli baytlar olarak
gönderir: `AT+MQTTPUBRAW=...,<uzunluk>,...`'ı yayınla, modülün `>`
istemini bekle, sonra tam olarak `<uzunluk>` ham bayt aktar — bu
müfredatta ilerde tırnaklı bir JSON payload'ının ya da paketlenmiş bir
binary struct'ın içerebileceği gömülü sıfır bayt dahil, her şey için
güvenli.

### 4. Yankıyı almak — gerçek bir uzunluk-önekli ayrıştırma

ESP-AT, gelen bir mesajı istenmeyen bir URC olarak bildirir:

```text
+MQTTSUBRECV:0,"tiremo-cortex/mqtt-demo",4,<4 ham bayt>
```

[`tiremo_esp32.c`](../../../../Cloud_Messaging/Feature_Tutorials/01-MQTT_To_Public_Broker/Sw/Drivers/esp32/Src/tiremo_esp32.c)'deki
`TIREMO_ESP32_MqttWaitMessage()`, `SendCommandCapture`'ın yaptığı gibi
baytları bir yakalama tamponuna biriktirir, ama sabit bir alt diziyi
eşleştirmek yerine, her bayttan sonra tamponu yeniden ayrıştırır:
`+MQTTSUBRECV:` işaretini bul, tırnaklı konuyu ayrıştır, ondalık
uzunluk alanını ayrıştır, sonra tamponun uzunluk alanının *ötesinde*
o kadar baytı zaten tutup tutmadığını kontrol et. Mesaj ancak o zaman
tamamdır — bir metin sezgiseli değil, gerçek bir uzunluk-önekli
ayrıştırma, ki bu onu 3. adımdaki binary payload için de doğru kılan
şey. Gerçek tek sınır: tüm URC'nin (başlık + payload) bu sürücünün
512 baytlık yakalama tamponuna sığması gerekiyor, ESP32
Connectivity'de `AT+CWLAP`'ın tarama dökümünün zaten sahip olduğu aynı
kısıtlama.

### 5. Derle, flaşla, izle

```sh
tdf.py build
tdf.py flash
tdf.py monitor
```

Bu eğitim için gerçek donanımda tam olarak neyin doğrulandığı, bağımsız
`mqttx` çapraz kontrolü dahil, için yukarıdaki **Çözüm** bölümüne
bakın.

## Ne Zaman Tamamlanmış Sayılır

Bu kart tarafından, aynı zamanda abone olduğu bir konuda yayınlanan bir
mesaj geri yankılanır ve loglanır — hem bu kartın kendi logu hem de
aynı genel broker'da bağımsız bir `mqttx` abonesi tarafından gerçekten
doğrulandı; bkz. **Çözüm**.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
