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

***Kart: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Bölüm: Cloud Messaging (MQTT & TLS) / Feature Tutorials · Zorluk: Orta***

> Bu sayfa [İngilizce orijinalinin](../../../../Cloud_Messaging/Feature_Tutorials/01-MQTT_To_Public_Broker/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Sertifikalara dokunmadan önce ESP32'nin yerleşik MQTT istemcisinin gerçekten çalıştığını kanıtlayın: herkese açık bir test broker'ına şifresiz bağlanın, bir mesaj yayınlayın ve aynı konuya abone olup mesajın geri geldiğini görün. [Eğitim 03](../03-MQTT_mTLS_To_AWS_IoT/README.md) ardından Tiremo Cloud'a ([`iot.tiremo.ai`](https://iot.tiremo.ai)) karşılıklı TLS (mTLS) ile bağlanır.

## Neler Öğreneceksiniz

- `AT+MQTTUSERCFG` (scheme 1, TLS yok) / `AT+MQTTCONN` / `AT+MQTTSUB` / `AT+MQTTPUB`
- ESP-AT'nin MQTT `scheme` değerinin neden 0'dan değil 1'den başladığı
- `MqttPublish` (metin yükü) ile `MqttPublishRaw` (ikili veriye güvenli) arasındaki fark
- Herkese açık bir broker'daki bir istemcinin neden kendine ait bir client ID'ye ve konuya ihtiyaç duyduğu

## Ön Koşullar

- [ESP32 Connectivity](../../../ESP32_Connectivity/) — WiFi bağlantısı çalışıyor
- İnternet erişimi olan 2,4 GHz bir WiFi ağı ve bu `Sw/` klasörünün kendi `wifi_secrets.h` dosyası — bkz. [Kimlik bilgileri](#kimlik-bilgileri-şifreyi-commit-etmeyin)
- [Getting Started](../../../Getting_Started/) kurulum betikleri çalıştırılmış olmalı, böylece `tdf` kullanılabilir
- Log'u izlemek için: `tdf monitor` ya da kartın USB seri portunda 115200 8N1'de herhangi bir seri terminal (Tera Term, PuTTY, screen, minicom, …)
- USB kabloyla bilgisayarınıza bağlı bir Tiremo Cortex kartı (ABOV A34G43A MCU — bkz. [Donanıma Genel Bakış](../../../Hardware.md))
- İsteğe bağlı: kartın mesajlarının broker'a ulaştığını izlemek için bilgisayarınızda bir MQTT istemcisi (örneğin [MQTTX](https://mqttx.app/))

## `scheme`'i anlamak: `scheme 0` yoktur

`AT+MQTTUSERCFG`'nin ikinci parametresi taşıma türünü seçer. 0'dan başlaması gereken bir sayı listesi gibi görünür, ama [Espressif'in ESP-AT MQTT komut referansında](https://github.com/espressif/esp-at/blob/master/docs/en/AT_Command_Set/MQTT_AT_Commands.rst) **1'den başlar**: `1` düz TCP, `2`–`5` TLS türleri (`5` karşılıklı TLS'tir, eğitim 03'te kullanılır), `6`–`10` WebSocket türleridir. `scheme 0` yoktur: `AT+MQTTUSERCFG=0,0,...` doğrudan yalın bir `\r\nERROR\r\n` (9 bayt) ile reddedilir — modül, daha herhangi bir ağ trafiği olmadan komutun kendisini reddeder. Sürücünün `1` göndermesinin nedeni budur.

## `MQTTPUBRAW`'ın tamamlanma işaretini anlamak

`AT+MQTTPUBRAW` iki adımda çalışır: modül komutu önce `OK` ile kabul eder ve yükü bir `>` istemiyle ister; ham baytlar gönderildikten sonra ancak sonucu `+MQTTPUB:OK` ya da `+MQTTPUB:FAIL` olarak bildirir. Kartta `TIREMO_DEBUG_LEVEL=4` ile yakalandı:

```text
[DEBUG] ESP32 TX (49 bytes):
{"data":"AT+MQTTPUBRAW=0,\"tiremo-cortex/mqtt-demo\",4,0,0\r\n"}
[DEBUG] ESP32 RX matched ">" after 9 bytes:
{"data":"\r\nOK\r\n\r\n>"}
[DEBUG] ESP32 RX matched "+MQTTPUB:OK" after 11 bytes:
{"data":"+MQTTPUB:OK"}
```

Yalın bir `"OK"` ile eşleşmek tesadüfen çalışırdı, çünkü `"+MQTTPUB:OK"` içinde `"OK"` geçer — ama daha önceki `OK`'de de dönerdi ve başarıyı `+MQTTPUB:FAIL`'den hiçbir zaman ayıramazdı. Bu yüzden sürücü açıkça `+MQTTPUB:OK` ile eşleşir.

## Paylaşılan bir public broker'ı anlamak

`broker.emqx.io` internetteki herkese açıktır; bu yüzden MQTT'nin iki kuralı burada önemlidir:

- **Client ID'ler benzersiz olmalıdır.** İkinci bir istemci aynı client ID ile bağlandığında broker ilkini düşürür. Başka biri bu eğitimi aynı anda varsayılan `tiremo-cortex-01` ile çalıştırırsa, sizin kartınız ve onunki birbirini sürekli atar.
- **Konular paylaşılır.** `tiremo-cortex/mqtt-demo` konusuna abone olan herkes, ona yayınlanan her mesajı görür — sizinkini de, başkalarınınkini de.

Bu yüzden yüklemeden önce [`Sw/Src/prv_user_code.c`](../../../../Cloud_Messaging/Feature_Tutorials/01-MQTT_To_Public_Broker/Sw/Src/prv_user_code.c) içindeki `MQTT_CLIENT_ID` ve `MQTT_TOPIC` değerlerine benzersiz bir ek verin (adınız, rastgele bir sayı, …), örneğin `"tiremo-cortex-myname-4711"` ve `"tiremo-cortex/myname-4711/demo"`. Ve herkese açık bir test broker'ı üzerinden asla özel bir şey göndermeyin.

## Kimlik bilgileri (şifreyi commit etmeyin)

[ESP32 Connectivity eğitim 02'deki](../../../ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/README.md#kimlik-bilgileri-şifreyi-commit-etmeyin) gibi: bu `Sw/` klasörünün kendi `wifi_secrets.h` dosyası gerekir. Repo kök dizininden:

```sh
cd Cloud_Messaging/Feature_Tutorials/01-MQTT_To_Public_Broker/Sw
cp wifi_secrets.h.example wifi_secrets.h
```

(cmd.exe'de `cp` yerine `copy` kullanın.) Ardından ağınızın SSID'sini ve şifresini girin ya da ESP32 Connectivity için hazırladığınız bir `wifi_secrets.h`'i kopyalayın. Dosya [`Sw/.gitignore`](../../../../Cloud_Messaging/Feature_Tutorials/01-MQTT_To_Public_Broker/Sw/.gitignore) içinde listelidir, yani hiçbir zaman bir commit'e girmez. ESP32 Connectivity'deki gibi, DEBUG seviyesindeki TX dökümü bağlanma komutunun yerine `AT+CWJAP=<credentials hidden>` gösterir.

## Çözüm

Repo kök dizininden (`wifi_secrets.h`'i yukarıdaki gibi oluşturduktan sonra):

```sh
cd Cloud_Messaging/Feature_Tutorials/01-MQTT_To_Public_Broker/Sw
tdf build
tdf flash
tdf monitor
```

(`tdf`'nin ne olduğu için [Getting_Started/01-Hello_Blink'in Çözüm bölümüne](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) bakın. macOS/Linux'ta `tdf.py` takma ad olarak da çalışır; Windows'ta `tdf` kullanın. `tdf monitor`'ü çalıştırmadan önce kartın seri portundaki diğer terminalleri kapatın — portu aynı anda yalnızca bir program açabilir.)

ESP32 sürücüsü ([`tiremo_esp32.c`](../../../../Cloud_Messaging/Feature_Tutorials/01-MQTT_To_Public_Broker/Sw/Drivers/esp32/Src/tiremo_esp32.c)), ESP32 Connectivity eğitim 02'nin WiFi bağlantı alt kümesidir; MQTT fonksiyonları aynı dosyaya eklenmiştir — ikinci bir UART yığını değil, büyüyen tek bir sürücü.

**Gerçek donanımda doğrulandı:** `tdf build`, macOS'ta ve Windows 11'de hatasız derlenip bağlanıyor (`.text` yaklaşık 17 KB; kesin boyut toolchain sürümüne göre biraz değişir, ve linker [01-Hello_Blink'in Çözüm bölümünde](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) açıklanan beklenen ve zararsız `_close` / `_lseek` / `_read` / `_write is not implemented` uyarılarını basar). `tdf flash`, gerçek, bağlı bir Tiremo Cortex kartına karşı SWD üzerinden tam bir programlama-ve-doğrulama döngüsünü tamamladı (`** Verified OK **`). Log (ağ adı değiştirildi):

```text
[INFO] MQTT_To_Public_Broker: boot
[INFO] ESP32: power cycling module...
[INFO] ESP32: power-on boot delay elapsed
[INFO] TIREMO_ESP32_Init: UART2 ready
[INFO] ESP32: command echo disabled (ATE0)
[INFO] ESP32: joining SSID "YourSSID"
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

Bu, her 5 saniyede bir metin (`MqttPublish`) ve ikili veriye güvenli (`MqttPublishRaw`) yayınlar arasında dönüşümlü olarak tekrarlanır. Ham yük `....` olarak loglanır, çünkü dört sayaç baytı yazdırılabilir ASCII değildir — bu beklenen bir durumdur, hata değil.

### Mesajları bilgisayarınızdan izleyin

Geri gelen mesaj bir gidiş-dönüşü kanıtlar, ama mesajların gerçekten karttan çıkıp internet üzerinden geçtiğini kanıtlamaz. Aynı konuya bilgisayarınızdaki ayrı bir MQTT istemcisiyle abone olun — örneğin [MQTTX](https://mqttx.app/) komut satırı aracıyla (konuyu değiştirdiyseniz kendi konunuzu kullanın):

```sh
mqttx sub -h broker.emqx.io -t tiremo-cortex/mqtt-demo
```

Kart çalışırken kartın metin mesajları ve ham sayaçları (`00 00 00 0d`, `00 00 00 0f`, … baytları olarak) her 5 saniyede bir `broker.emqx.io` üzerinden bilgisayarınıza ulaşır. Bu, test kartında bir bilgisayardaki ayrı bir aboneyle kontrol edildi.

## Adım Adım İnceleme

### 1. MQTT kimliği — `TIREMO_ESP32_MqttUserConfig()`

```c
(void)snprintf(command, sizeof(command),
               "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",
               pClientId, pUsername, pPassword);
```

LinkID `0` (bu sürücü yalnızca tek bir MQTT bağlantısı kullanır), scheme `1` (düz TCP — yukarıya bakın), boş kullanıcı adı/şifre (bu public broker hiçbirini istemez), sertifika yok (slotlar `0`, yol `""` — bunlar eğitim 02'den itibaren önem kazanır).

### 2. Bağlan ve abone ol

```c
TIREMO_ESP32_MqttConnect("broker.emqx.io", 1883, 10000U);
TIREMO_ESP32_MqttSubscribe("tiremo-cortex/mqtt-demo", 0U);
```

İkisi de `"OK"` ile eşleşen sıradan `AT+MQTTCONN` / `AT+MQTTSUB` komutlarıdır.

### 3. Yayınlamanın iki yolu

[`Sw/Src/prv_user_code.c`](../../../../Cloud_Messaging/Feature_Tutorials/01-MQTT_To_Public_Broker/Sw/Src/prv_user_code.c) aynı konuda ikisi arasında dönüşümlü çalışır:

```c
TIREMO_ESP32_MqttPublish(topic, "hello from Tiremo Cortex #0", 0, false);
/* vs. */
uint8_t counter[4] = { 0, 0, 0, 1 };
TIREMO_ESP32_MqttPublishRaw(topic, counter, 4, 0, false);
```

`MqttPublish` yükü AT komutunun tırnakları içinde gönderir — basittir, ama yük hiçbir zaman `"`, `\r` ya da `\n` içeremez. `MqttPublishRaw` ise yükü ham, uzunluğu önceden bildirilen baytlar olarak gönderir: `AT+MQTTPUBRAW=...,<uzunluk>,...` gönderilir, modülün `>` istemi beklenir, ardından tam olarak `<uzunluk>` kadar ham bayt iletilir — her şey için güvenlidir; bir JSON yükündeki tırnaklar ya da paketlenmiş bir ikili yapıdaki sıfır baytı dahil.

### 4. Geri gelen mesajı almak — uzunluk önekli ayrıştırma

ESP-AT gelen bir mesajı istenmeden gönderilen bir bildirim (URC) olarak bildirir:

```text
+MQTTSUBRECV:0,"tiremo-cortex/mqtt-demo",4,<4 raw bytes>
```

[`tiremo_esp32.c`](../../../../Cloud_Messaging/Feature_Tutorials/01-MQTT_To_Public_Broker/Sw/Drivers/esp32/Src/tiremo_esp32.c) içindeki `TIREMO_ESP32_MqttWaitMessage()`, baytları `SendCommandCapture` gibi bir yakalama tamponunda toplar, ama sabit bir alt dizeyle eşleşmek yerine her bayttan sonra tamponu yeniden ayrıştırır: `+MQTTSUBRECV:` işaretini bulur, tırnak içindeki konuyu ayrıştırır, ondalık uzunluğu ayrıştırır, sonra tamponun uzunluk alanından *sonra* o kadar bayt içerip içermediğine bakar. Mesaj ancak o zaman tamamdır — bir metin tahmini değil, gerçek bir uzunluk önekli ayrıştırma; 3. adımdaki ikili yük için de doğru çalışmasını sağlayan budur. Sınır: bildirimin tamamı (başlık + yük) sürücünün 512 baytlık yakalama tamponuna sığmalıdır.

## Tamamlanma Ölçütü

- `tdf monitor`, `MQTT: connected` satırını ve ardından her 5 saniyede bir `publishing` satırını, onu izleyen `echo received` satırını ve aynı yükü gösteriyor (metin ve ham, dönüşümlü).
- Bilgisayarınızda, `broker.emqx.io` üzerinde aynı konuya abone ayrı bir MQTT istemcisi aynı mesajları alıyor.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
