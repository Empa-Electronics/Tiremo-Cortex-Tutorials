<p class="sphinxhide" align="right"><a href="../../../../Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>Tiremo Cloud'a MQTT mTLS</h1>
      <sub>Tiremo Cortex Tutorials — İlk yanıp sönmeden filo hazır bir cihaza, ESP32 + AWS IoT donanım yazılımı</sub>
    </td>
  </tr>
</table>


# Tiremo Cloud'a MQTT mTLS

***Kart: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Bölüm: Cloud Messaging (MQTT & TLS) / Feature Tutorials · Zorluk: Zor***

> Bu sayfa [İngilizce orijinalinin](../../../../Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Kimlik bilgileri yazılabilir hâldeyken ([eğitim 02](../02-mTLS_Credentials_In_Module_NVS/README.md)) ve MQTT şifresiz olarak çalışırken ([eğitim 01](../01-MQTT_To_Public_Broker/README.md)), ikisini birleştirin: modüle Tiremo UI'dan alınmış gerçek bir sertifika yükleyin ve Tiremo Cloud (`iot.tiremo.ai`) ile karşılıklı TLS'li bir MQTT oturumu açın. Bu eğitim başarılı bir `AT+MQTTCONN`'da durur — henüz yayınlama ya da abonelik yok (o, [Tiremo Cloud](../../../Tiremo_Cloud/) bölümünün konusudur) — çünkü zor olan kısım el sıkışmanın kendisini doğru yapmaktır.

## Neler Öğreneceksiniz

- `AT+MQTTUSERCFG` ile TLS scheme 5 (sunucunun sertifikasını doğrula, istemcinin sertifikasını sun)
- SNI yönlendirmesinin arkasında duran bir uç nokta için `AT+MQTTSNI`'nin neden önemli olduğu
- Scheme 5'in `AT+MQTTCONN`'unun neden modülün önce SNTP ile gerçek saati almasını gerektirdiği
- Tiremo Cloud filo kurulumu her cihaza kendi sertifikasını vermeden önce, tezgâhta test için Tiremo UI'dan indirilmiş tek bir sertifika kullanmak
- Gerçek bir cihaz özel anahtarını git deposunun dışında tutmak

## Ön Koşullar

- [02-mTLS_Credentials_In_Module_NVS](../02-mTLS_Credentials_In_Module_NVS/README.md) — bu eğitimin gerçek kimlik bilgilerini yüklediği kimlik bilgisi bölmesi sürücüsü
- Ürününüz için **Tiremo UI**'dan kimlik bilgileri: **Identifier** ve **Provisioning Certificates** (sertifika ve özel anahtar) — ürün sayfasında nerede olduklarını görmek için [Tiremo UI'dan kimlik bilgileri](../../../Tiremo_Cloud/Feature_Tutorials/03-Fleet_Provisioning_By_Claim/README.md#tiremo-uidan-kimlik-bilgileri) bölümüne, ayrıca aşağıdaki [`certs/` klasörü](#certs-klasörünü-anlamak) bölümüne bakın. Yanlarında görünen **Secret Key** bu eğitimde kullanılmaz; Tiremo Cloud bölümündeki filo kurulumu için gereklidir.
- İnternet erişimi olan 2,4 GHz bir WiFi ağı ve bu `Sw/` klasörünün kendi `wifi_secrets.h` dosyası — [Cloud Messaging eğitim 01'deki](../01-MQTT_To_Public_Broker/README.md#kimlik-bilgileri-şifreyi-commit-etmeyin) gibi
- [Getting Started](../../../Getting_Started/) kurulum betikleri çalıştırılmış olmalı, böylece `tdf` kullanılabilir
- Log'u izlemek için: `tdf monitor` ya da kartın USB seri portunda 115200 8N1'de herhangi bir seri terminal (Tera Term, PuTTY, screen, minicom, …)
- USB kabloyla bilgisayarınıza bağlı bir Tiremo Cortex kartı (ABOV A34G43A MCU — bkz. [Donanıma Genel Bakış](../../../Hardware.md))

## Platform dökümanı

- Uç nokta ve ilk mTLS oturumu: [first connection](https://docs.tiremo.ai/device-provisioning/third-phase)
- `iot.tiremo.ai`'de `AT+MQTTSNI`'nin neden gerekli olduğu: [TLS SNI Requirement](https://docs.tiremo.ai/device-provisioning/tls-sni)
- Claim sertifikaları ve filo kurulumu (Tiremo Cloud bölümü): [device provisioning](https://docs.tiremo.ai/device-provisioning/getting-started)

## Bağlanma sırasını anlamak

Scheme 5 ile `AT+MQTTUSERCFG`, MQTT kimliğini — client ID'yi — yapılandırır; `cert_key_ID` ve `CA_ID`'nin ikisi de `0`'dır; bu, modülün TLS motoruna [eğitim 02](../02-mTLS_Credentials_In_Module_NVS/README.md)'nin `TIREMO_ESP32_CredWrite()` fonksiyonunun `mqtt_cert` / `mqtt_key` / `mqtt_ca` bölmelerine en son yazdığı şeyi kullanmasını söyler. `AT+MQTTSNI` TLS SNI ana bilgisayar adını ayarlar. Paylaşılan, ana bilgisayar adına göre yönlendirilen bir TLS altyapısının arkasında duran bir uç nokta için gereklidir ([TLS SNI Requirement](https://docs.tiremo.ai/device-provisioning/tls-sni)): o olmadan TLS el sıkışması, SNI'sız varsayılanın yönlendirdiği yerle tamamlanabilir, ama bağlantı hedeflenen arka uca hiç ulaşmaz. Sıra `MQTTUSERCFG` → `MQTTSNI` → `MQTTCONN`'dur. (Keepalive ve son vasiyet için olan `AT+MQTTCONNCFG` burada gerekmez.)

```c
TIREMO_ESP32_MqttUserConfigMtls(MQTT_CLIENT_ID);   /* AT+MQTTUSERCFG=0,5,"<id>","","",0,0,"" */
TIREMO_ESP32_MqttSni(MQTT_ENDPOINT);               /* AT+MQTTSNI=0,"<host>"                 */
TIREMO_ESP32_MqttConnect(MQTT_ENDPOINT, 8883, ...);/* AT+MQTTCONN=0,"<host>",8883,0          */
```

## `certs/` klasörünü anlamak

Üç dosya, çok farklı üç hassasiyet düzeyi:

| Dosya | Commit'li mi? | Nedir |
|---|---|---|
| [`certs/root_ca.pem`](../../../../Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/Sw/certs/root_ca.pem) | Evet | **Amazon Root CA 1** — herkese açık, her Tiremo Cloud cihazı için aynı, `https://www.amazontrust.com/repository/AmazonRootCA1.pem` adresinden. Tiremo Cloud'un sunucu sertifikasını doğrular. |
| `certs/device_cert.pem.example` / `device_key.pem.example` | Evet | Biçimi gösteren yer tutucular. |
| `certs/device_cert.pem` | **Hayır** (gitignore) | Tiremo UI'daki ürün sayfanızın **Provisioning Certificates** bölümündeki sertifika. |
| `certs/device_key.pem` | **Hayır** (gitignore) | Onun özel anahtarı; aynı anda indirilir. Bir şifre gibi saklayın: **bu dosyaya sahip olan herkes ürününüzün kimlik bilgileriyle Tiremo Cloud'a bağlanabilir.** |

İndirme size ürününüzün tanımlayıcısını taşıyan iki dosya verir, örneğin `myproduct.certificate.pem` ve `myproduct.private.key`. Bunları `Sw/certs/` klasörüne koyun ve **yeniden adlandırın** — derleme yalnızca şu iki adı arar:

| İndirilen dosya | Yeni adı |
|---|---|
| `<identifier>.certificate.pem` | `device_cert.pem` |
| `<identifier>.private.key` | `device_key.pem` |

Dosyalardan biri eksikse derleme `Missing device credentials (certs/device_cert.pem, certs/device_key.pem)` hatasıyla durur ve aynı yeniden adlandırmayı gösterir. İki ad da gitignore'dadır; deponun `.gitignore` dosyası ayrıca bir `certs/` klasöründeki her `*.key` dosyasını da yok sayar; böylece yeniden adlandırılmamış bir `<identifier>.private.key` bile commit'e kaçmaz. Yine de commit atmadan önce `git status`'a bakın: bu iki dosya orada asla görünmemelidir. Bkz. [`certs/README.md`](Sw/certs/README.md).

`iot.tiremo.ai`'nin sunucu sertifikası `iot.tiremo.ai` → `Amazon RSA 2048 M04` → `Amazon Root CA 1` zinciriyle bağlanır; `root_ca.pem`'in Amazon Root CA 1 olmasının nedeni budur. Bir zinciri kendiniz `openssl s_client -connect iot.tiremo.ai:8883 -servername iot.tiremo.ai -showcerts` ile kontrol edebilirsiniz.

## Client ID'yi anlamak

[`Sw/Src/prv_user_code.c`](../../../../Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/Sw/Src/prv_user_code.c) içindeki `MQTT_CLIENT_ID`, `"YOUR_DEVICE_ID"` yer tutucusuyla gelir. Bunu Tiremo UI'daki ürün sayfanızda yazan **Identifier** ile değiştirin:

```c
#define MQTT_CLIENT_ID            "YOUR_DEVICE_ID"
```

Bağlantının kendisi sertifikayla doğrulanır: test kartında `AT+MQTTCONN`, yer tutucu yerinde dururken bile kabul edildi. Yine de her kullanıcının paylaşacağı bir yer tutucu yerine kendi Identifier'ınızı kullanın — aynı client ID ile bağlanan iki istemci birbirini düşürür (bkz. [eğitim 01](../01-MQTT_To_Public_Broker/README.md#paylaşılan-bir-public-brokerı-anlamak)). [Tiremo Cloud](../../../Tiremo_Cloud/) bölümünde ise filo kurulumuyla her cihaz kendi sertifikasını ve kimliğini alır.

## SNTP'yi anlamak: scheme 5 neden önce eşitlenmiş bir saat ister

Scheme 5 sunucunun sertifikasını doğrular; bu, geçerlilik tarihlerini kontrol etmek demektir — ve modülün açılışta gerçek zamanlı bir saati yoktur. Espressif'in `AT+MQTTUSERCFG` dökümanı bunu kolay gözden kaçan bir notta söyler:

> If `<scheme>` is configured to 3, 5, 8, or 10, in order to check the server certificate validity period, please make sure {IDF_TARGET_NAME} has obtained the current time before sending the `AT+MQTTCONN` command.

Bu yüzden yazılım, MQTT bağlanma sırasından önce SNTP'yi açar ve gerçek bir saat örneği bekler; bunun için [ESP32 Connectivity eğitim 04](../../../ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/README.md)'ün `EnableSntp()` / `WaitSntpEpoch()` fonksiyonlarını yeniden kullanır. Kartta `TIREMO_DEBUG_LEVEL=4` ile yakalandı: modül önce 1970'ten itibaren sayar, sonra gerçek saati bildirir:

```text
{"data":"+CIPSNTPTIME:Thu Jan  1 00:00:07 1970\r\nOK"}      <- not synced yet
{"data":"+CIPSNTPTIME:Tue Nov 24 09:43:28 2026\r\nOK"}      <- synced
```

`WaitSntpEpoch()`, bildirilen yıl bir alt sınırı (2020) geçene kadar sorar; böylece eşitlenmemiş sayacı asla gerçek bir saat sanmaz.

## Kimlik bilgilerinizi bağımsız olarak doğrulamak

Derleyip yüklemeden önce sertifikanızın, özel anahtarınızın ve kök CA'nın geçerli olduğunu ve birbirine ait olduğunu yazılım işin içinde olmadan kontrol edebilirsiniz — böylece kart daha sonra bağlanamazsa, kimlik bilgilerine mi yoksa modüle mi bakmanız gerektiğini zaten bilirsiniz. Bu eğitimin `Sw/` klasöründen:

```sh
openssl s_client -connect iot.tiremo.ai:8883 -servername iot.tiremo.ai -cert certs/device_cert.pem -key certs/device_key.pem -CAfile certs/root_ca.pem
```

Başarılı bir karşılıklı TLS el sıkışması `Verify return code: 0 (ok)` bildirir. Tam bir MQTT `CONNECT`'i (yalnızca TLS katmanını değil) kontrol etmek için aynı üç dosyayla [MQTTX](https://mqttx.app/) komut satırı aracını kullanın:

```sh
mqttx conn -h iot.tiremo.ai -p 8883 -l mqtts --cert certs/device_cert.pem --key certs/device_key.pem --ca certs/root_ca.pem -i YOUR_DEVICE_ID
```

## Çözüm

Repo kök dizininden, `wifi_secrets.h`'i ve iki `certs/` dosyasını yerleştirip `MQTT_CLIENT_ID`'yi ayarladıktan sonra:

```sh
cd Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/Sw
tdf build
tdf flash
tdf monitor
```

(`tdf`'nin ne olduğu için [Getting_Started/01-Hello_Blink'in Çözüm bölümüne](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) bakın. macOS/Linux'ta `tdf.py` takma ad olarak da çalışır; Windows'ta `tdf` kullanın. `tdf monitor`'ü çalıştırmadan önce kartın seri portundaki diğer terminalleri kapatın — portu aynı anda yalnızca bir program açabilir.)

Sürücü, [Cloud Messaging eğitim 01](../01-MQTT_To_Public_Broker/README.md)'in WiFi + MQTT sürücüsüdür; [eğitim 02](../02-mTLS_Credentials_In_Module_NVS/README.md)'nin kimlik bilgisi bölmesi fonksiyonları ve [ESP32 Connectivity eğitim 04](../../../ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/README.md)'ün SNTP fonksiyonları eklenmiştir; artı iki yeni fonksiyon: `TIREMO_ESP32_MqttUserConfigMtls()` (scheme 5) ve `TIREMO_ESP32_MqttSni()`.

**Gerçek donanımda doğrulandı:** `tdf build`, macOS'ta ve Windows 11'de hatasız derlenip bağlanıyor (`.text` yaklaşık 21,5 KB; kesin boyut toolchain sürümüne göre biraz değişir, ve linker [01-Hello_Blink'in Çözüm bölümünde](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) açıklanan beklenen ve zararsız `_close` / `_lseek` / `_read` / `_write is not implemented` uyarılarını basar). `tdf flash`, gerçek, bağlı bir Tiremo Cortex kartına karşı SWD üzerinden tam bir programlama-ve-doğrulama döngüsünü tamamladı (`** Verified OK **`) ve kart Tiremo UI'dan alınmış gerçek bir sertifikayla `iot.tiremo.ai` ile karşılıklı TLS'li bir oturum açtı. Log (ağ adı ve tanımlayıcı değiştirildi):

```text
[INFO] MQTT_mTLS_To_AWS_IoT: boot
[INFO] ESP32: power cycling module...
[INFO] ESP32: power-on boot delay elapsed
[INFO] TIREMO_ESP32_Init: UART2 ready
[INFO] ESP32: command echo disabled (ATE0)
[INFO] ROOT_CA: writing 1206 bytes...
[INFO] ROOT_CA: stored
[INFO] CLIENT_CERT: writing 1220 bytes...
[INFO] CLIENT_CERT: stored
[INFO] CLIENT_KEY: writing 1679 bytes...
[INFO] CLIENT_KEY: stored
[INFO] Credentials: all three slots loaded
[INFO] ESP32: joining SSID "YourSSID"
[INFO] ESP32: WIFI GOT IP
[INFO] ESP32: SNTP synced (epoch 1795512665)
[INFO] MQTT: client id "YOUR_DEVICE_ID" configured (scheme 5, mutual TLS)
[INFO] MQTT: SNI set to "iot.tiremo.ai"
[INFO] MQTT: connecting to iot.tiremo.ai:8883
[INFO] MQTT: connected (mutual TLS) -- MQTTUSERCFG/MQTTSNI/MQTTCONN all OK
[INFO] MQTT_mTLS_To_AWS_IoT: idle (connect already succeeded once at boot, no subscribe/publish attempted)
```

Sertifika boyutları kendi dosyalarınızınkidir. `connected` satırının arkasındaki ham AT izi (level 4), scheme `5` ve uç noktanın geri yansıtıldığı, modülün kendi onayıdır:

```text
{"data":"+MQTTCONNECTED:0,5,\"iot.tiremo.ai\",\"8883\",\"\",0\r\n\r\nOK"}
```

Level 4'te kimlik bilgisi yazmaları yalnızca komut başlıklarıyla görünür (`AT+SYSMFG=2,"mqtt_key","mqtt_key",8,1679`) — sertifika ve anahtar baytları modüle doğrudan gönderilir ve log'a hiç ulaşmaz — WiFi bağlantısı da `AT+CWJAP=<credentials hidden>` olarak görünür.

## Adım Adım İnceleme

### 1. Önce kimlik bilgilerini yükle, sonra WiFi'ye bağlan

[`Sw/Src/prv_user_code.c`](../../../../Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/Sw/Src/prv_user_code.c), üç bölmenin hepsini — kök CA, cihaz sertifikası, cihaz anahtarı; eğitim 02'deki aynı [`cmake/embed_pem.cmake`](../../../../cmake/embed_pem.cmake) mekanizmasıyla derleme sırasında `certs/*.pem`'den gömülür — WiFi'ye dokunmadan *önce* yazar, çünkü `AT+SYSMFG` yerel bir depodur ve ağ gerektirmez:

```c
LoadCred(TIREMO_ESP32_CREDSLOT_ROOT_CA, "ROOT_CA", g_rootCaPem, g_rootCaPem_len);
LoadCred(TIREMO_ESP32_CREDSLOT_CLIENT_CERT, "CLIENT_CERT", g_deviceCertPem, g_deviceCertPem_len);
LoadCred(TIREMO_ESP32_CREDSLOT_CLIENT_KEY, "CLIENT_KEY", g_deviceKeyPem, g_deviceKeyPem_len);
```

### 2. WiFi bağlantısı, ardından SNTP — herhangi bir MQTT komutundan önce

```c
TIREMO_ESP32_JoinAP(TIREMO_WIFI_SSID, TIREMO_WIFI_PASSWORD, JOIN_TIMEOUT_MS);
TIREMO_ESP32_EnableSntp();
TIREMO_ESP32_WaitSntpEpoch(&epoch, SNTP_TIMEOUT_MS);
```

Bu adımın neden var olduğu için yukarıdaki SNTP bölümüne bakın: atlarsanız `MQTTCONN` scheme 5 altında her seferinde başarısız olur ve genel bir `ERROR`'dan daha açık bir işaret vermez.

### 3. Bağlanma sırasının kendisi

```c
TIREMO_ESP32_MqttUserConfigMtls(MQTT_CLIENT_ID);
TIREMO_ESP32_MqttSni(MQTT_ENDPOINT);
TIREMO_ESP32_MqttConnect(MQTT_ENDPOINT, MQTT_PORT, MQTT_CONNECT_TIMEOUT_MS);
```

[`tiremo_esp32.c`](../../../../Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/Sw/Drivers/esp32/Src/tiremo_esp32.c) içindeki `TIREMO_ESP32_MqttUserConfigMtls()`, eğitim 01'in düz TCP'li `MqttUserConfig()` fonksiyonundan ayrıdır: scheme 5, boş kullanıcı adı ve şifre (kimlik doğrulama sertifikayladır), `cert_key_ID` / `CA_ID` ikisi de `0`. `TIREMO_ESP32_MqttSni()`, `AT+MQTTSNI=0,"<host>"` gönderir ve `"OK"` bekler.

## Tamamlanma Ölçütü

Log `Credentials: all three slots loaded`, `SNTP synced` ve `MQTT: connected (mutual TLS)` gösteriyor — kendi sertifikanızla Tiremo Cloud ile karşılıklı TLS'li bir oturum. Ve `git status` ne `device_cert.pem`'i ne de `device_key.pem`'i gösteriyor.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
