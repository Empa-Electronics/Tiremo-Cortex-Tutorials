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
      <h1>AWS IoT Core'a MQTT mTLS</h1>
      <sub>Tiremo Cortex Tutorials — İlk yanıp sönmeden filo hazır bir cihaza, ESP32 + AWS IoT donanım yazılımı</sub>
    </td>
  </tr>
</table>


# AWS IoT Core'a MQTT mTLS

***Kart: Tiremo Cortex (ESP32) · Bölüm: Cloud Messaging (MQTT & TLS) / Feature Tutorials · Zorluk: Zor***

## Giriş

Kimlik bilgileri yazılabilir hale geldiğine (Tutorial 02) ve MQTT açık
metinde çalıştığına göre (Tutorial 01), ikisini birleştirin: gerçek bir
AWS IoT Thing için elle oluşturduğunuz bir sertifikayı modüle yükleyin
ve gerçek bir AWS IoT Core uç noktasına karşı gerçek bir karşılıklı-TLS
MQTT oturumu kurun. Bu eğitim başarılı bir `AT+MQTTCONN`'da duruyor —
henüz provizyon yok, henüz publish/subscribe yok (bunlar daha sonraki
`AWS_IoT_Core` müfredatında) — çünkü el sıkışmanın kendisini doğru
yapmak, bu bölümün tamamının gerçekten zor kısmı.

## Neler Öğreneceksiniz

- `MQTTUSERCFG` üzerinden TLS scheme 5 (sunucu sertifikasını doğrula, istemci sertifikası sun)
- SNI yönlendirmesi arkasında oturan özel domain uç noktaları için `AT+MQTTSNI` neden önemli
- Scheme 5'in `AT+MQTTCONN`'unun, modülün önce gerçek zamanı SNTP üzerinden senkronize etmesini neden gerektirdiği
- Bench testi için elle bir AWS IoT Thing + sertifika oluşturma, AWS IoT Core bunu otomatikleştirmeden önce
- Diğer eğitimlerin build'ini bozmadan gerçek bir cihaz özel anahtarını public bir git deposunun dışında tutmak

## Ön Koşullar

- [02-mTLS_Credentials_In_Module_NVS](../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/) — bu eğitimin gerçek kimlik bilgilerini üzerinden yüklediği credential-slot sürücüsü
- Bir AWS IoT Core uç noktası ve elle oluşturulmuş bir Thing sertifikası — tam olarak neyi nereden alacağınız için aşağıdaki **`certs/` Klasörünü Anlamak (Gerçek ve Gitignore'lu)**'a bakın
- Bu `Sw/` içinde `wifi_secrets.h` (gitignore'lu), diğer tüm Cloud Messaging / ESP32 Connectivity eğitimleriyle aynı desen
- [Getting Started](../../../../Getting_Started/)'a göre kurulmuş araç zinciri ve flashleme

## Bağlantı Sırasını Anlamak

`AT+MQTTUSERCFG` (scheme 5), MQTT kimliğini yapılandırır — istemci ID'si
ve `cert_key_ID`/`CA_ID`'nin ikisi de `0`, modülün TLS motoruna
[Tutorial 02](../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/)'nin
`TIREMO_ESP32_CredWrite()`'ının en son `mqtt_cert`/`mqtt_key`/`mqtt_ca`
slotlarına ne yazdıysa onu kullanmasını söyler. `AT+MQTTSNI`, TLS SNI
hostname'ini ayarlar — paylaşılan TLS-sonlandıran altyapının arkasında
oturabilecek özel bir domain uç noktası için gerekli; onsuz, TLS el
sıkışması SNI'siz varsayılanın yönlendirdiği her neyse ona karşı
tamamlanabilirken, bağlantı asıl hedeflenen backend'e hiç ulaşmaz. Sıra
önemli: `MQTTUSERCFG` → `MQTTSNI` → `MQTTCONN`, arada `AT+MQTTCONNCFG`
olmadan (o komut keepalive/LWT için, kimlik/TLS ile alakasız, ve burada
çağrılmasına gerek yok).

```c
TIREMO_ESP32_MqttUserConfigMtls(MQTT_CLIENT_ID);   /* AT+MQTTUSERCFG=0,5,"<id>","","",0,0,"" */
TIREMO_ESP32_MqttSni(MQTT_ENDPOINT);               /* AT+MQTTSNI=0,"<host>"                 */
TIREMO_ESP32_MqttConnect(MQTT_ENDPOINT, 8883, ...);/* AT+MQTTCONN=0,"<host>",8883,0          */
```

## `certs/` Klasörünü Anlamak (Gerçek ve Gitignore'lu)

Üç dosya, üç çok farklı hassasiyet seviyesi:

| Dosya | Commit'lendi mi? | Ne olduğu |
|---|---|---|
| [`certs/root_ca.pem`](../../../../Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/Sw/certs/root_ca.pem) | Evet | **Amazon Root CA 1** — gerçekten public, her AWS hesabı için aynı, doğrudan `https://www.amazontrust.com/repository/AmazonRootCA1.pem`'den alındı. AWS IoT'nin sunucu sertifikasını doğrular. |
| `certs/device_cert.pem.example` / `device_key.pem.example` | Evet | Formatı gösteren yer tutucular. |
| `certs/device_cert.pem` | **Hayır** (gitignore'lu) | Sizin AWS IoT Thing'inizin kendi sertifikası, onu oluşturduğunuzda AWS konsolundan indirilir. |
| `certs/device_key.pem` | **Hayır** (gitignore'lu) | Sizin AWS IoT Thing'inizin kendi özel anahtarı, aynı anda indirilir — AWS bunu yalnızca bir kez gösterir. Bu dosyaya sahip olan herkes AWS hesabınıza bu cihaz olarak bağlanabilir. |

Bu, `wifi_secrets.h`'ın bu depoda her yerde zaten nasıl ele alındığını
yansıtıyor: gitignore'lu gerçek bir dosya, commit'lenmiş bir `.example`
şablonu. İki `.example` dosyasını aynı klasörde `device_cert.pem` ve
`device_key.pem` olarak kopyalayın ve build'den önce gerçek Thing'inizin
sertifikasını ve anahtarını doldurun — bkz.
[`certs/README.md`](../../../../Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/Sw/certs/README.md).

Root CA konusuna gelince: bu özel domain'in sunucu sertifikası
`iot.tiremo.ai` → `Amazon RSA 2048 M04` → `Amazon Root CA 1` zincirini
izliyor, bunu varsaymadan önce doğrudan `openssl s_client -showcerts`
ile kontrol ettim — AWS IoT'nin standart uç noktalarının kullandığı
aynı root, sadece özel bir domain olduğu için ayrı bir tane değil. Bu
her özel domain için mutlaka doğru olmayabilir (bir domain sahibi
ACM'yi tamamen farklı bir sertifikayla yapılandırabilir); varsaymak
yerine kendinizinkini aynı şekilde kontrol edin.

## SNTP'yi Anlamak: Scheme 5 Neden Önce Senkronize Bir Saate İhtiyaç Duyar

Scheme 5, sunucunun sertifikasını doğrular, bu da geçerlilik
tarihlerini kontrol etmek anlamına gelir — ve modülün önyüklemede
RTC'si yok. Gerçek bir saat olmadan, ESP-AT'nin TLS motoru "bu
sertifika şu anda geçerli mi" diye değerlendiremez, bu yüzden
`AT+MQTTCONN`, modülün önce SNTP üzerinden geçerli zamanı almış
olmasını gerektirir. Espressif'in kendi `AT+MQTTUSERCFG`
dokümantasyonu bunu doğrudan belirtiyor, bir parametre açıklaması
değil bir not olarak — ilk okumada gözden kaçması kolay:

> `<scheme>` 3, 5, 8 veya 10 olarak yapılandırılmışsa, sunucu
> sertifikasının geçerlilik süresini kontrol etmek için, lütfen
> `AT+MQTTCONN` komutunu göndermeden önce {IDF_TARGET_NAME}'in geçerli
> zamanı almış olduğundan emin olun.

Bu eğitimin sürücüsü, MQTT bağlantı sırasını çalıştırmadan önce SNTP'yi
etkinleştirir ve gerçek bir zaman örneği bekler, ikinci bir SNTP
ayrıştırıcı yazmak yerine
[ESP32 Connectivity Tutorial 04](../../../../ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/)'ün
zaten doğrulanmış `EnableSntp()`/`WaitSntpEpoch()` fonksiyonlarını
yeniden kullanır. Gerçek donanımda, modül SNTP gerçekten tamamlanana
kadar Unix epoch'unu (1 Oca 1970) raporlar, sonra gerçek geçerli
zamanı:

```text
[DEBUG] ESP32 TX (17 bytes): {"data":"AT+CIPSNTPTIME?\r\n"}
[DEBUG] ESP32 RX matched "OK" after 41 bytes:
{"data":"+CIPSNTPTIME:Thu Jan  1 00:00:04 1970\r\nOK"}      <- henüz senkronize değil
[DEBUG] ESP32 TX (17 bytes): {"data":"AT+CIPSNTPTIME?\r\n"}
[DEBUG] ESP32 RX matched "OK" after 41 bytes:
{"data":"+CIPSNTPTIME:Wed Aug 26 21:12:17 2026\r\nOK"}       <- senkronize
[INFO] ESP32: SNTP synced (epoch 1787778737)
```

`WaitSntpEpoch()`, raporlanan yıl bir makul alt sınırı (2020) geçene
kadar tam olarak bu şekilde yokluyor, yani senkronize olmayan
varsayılanı asla gerçek zamanla karıştırmıyor.

## Kimlik Bilgilerinizi Bağımsız Olarak Doğrulama

Build ve flash yapmadan önce, sertifikanızın, özel anahtarınızın ve
root CA'nızın geçerli ve doğru eşleştirilmiş olduğunu donanım yazılımı
devrede *olmadan* doğrulamaya değer — bu şekilde, daha sonra bir şey
bağlanmazsa, kimlik bilgilerine mi yoksa modüle mi bakmanız gerektiğini
zaten bilirsiniz.

```sh
openssl s_client -connect <sizin-uç-noktanız>:8883 -servername <sizin-uç-noktanız> \
    -cert certs/device_cert.pem -key certs/device_key.pem -CAfile certs/root_ca.pem
```

Başarılı bir karşılıklı-TLS el sıkışması `Verify return code: 0 (ok)`
raporlar. Ayrıca [`mqttx`](https://mqttx.app/) CLI ile tam bir MQTT
`CONNECT`'i de (yalnızca ham TLS katmanını değil) bağımsız olarak
doğrulayabilirsiniz:

```sh
mqttx sub -h <sizin-uç-noktanız> -p 8883 -l mqtts -t 'some/topic' \
    --cert certs/device_cert.pem --key certs/device_key.pem --ca certs/root_ca.pem \
    -i <sizin-istemci-id'niz>
```

Buradaki bir `✔ Connected`, AWS IoT'nin bu sertifikayı ve istemci
ID'sini gerçek bir oturum için kabul ettiğini doğrular — sonraki
subscribe, bağlantının kendisiyle alakasız policy nedenleriyle
reddedilse bile.

## Çözüm

[`Sw/`](../../../../Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/Sw/) gerçek, derlenebilir bir uygulamaya sahip:

```sh
cd Sw
cp wifi_secrets.h.example wifi_secrets.h                # bir kez; sonra düzenleyin
cp certs/device_cert.pem.example certs/device_cert.pem  # bir kez; sonra gerçek sertifikanızı yapıştırın
cp certs/device_key.pem.example certs/device_key.pem    # bir kez; sonra gerçek anahtarınızı yapıştırın
# ayrıca Src/prv_user_code.c içindeki MQTT_ENDPOINT / MQTT_CLIENT_ID'yi kendinizinkine düzenleyin
tdf.py build
tdf.py flash
tdf.py monitor
```

Temel:
[Cloud Messaging Tutorial 01](../../../../Cloud_Messaging/Feature_Tutorials/01-MQTT_To_Public_Broker/)'in
WiFi+MQTT sürücüsü (mTLS'e özgü parçalar dışında ihtiyaç duyulan her
şeyi zaten taşıyor), üzerine
[Tutorial 02](../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/)'nin
credential-slot fonksiyonları ve
[ESP32 Connectivity Tutorial 04](../../../../ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/)'ün
SNTP fonksiyonları katlanmış, artı bu eğitimin eklediği iki yeni
fonksiyon: `TIREMO_ESP32_MqttUserConfigMtls()` (scheme 5) ve
`TIREMO_ESP32_MqttSni()`.

Bu build **subscribe veya publish yapmıyor** — bilinçli olarak yalnızca
mTLS el sıkışmasının kendisini kanıtlıyor, çünkü bu bölümün tamamının
gerçekten zor kısmı bu. Gerçek bir AWS konusuna karşı pub/sub daha
sonraki `AWS_IoT_Core` müfredatında; bu eğitimin kendi orijinal "test
konusuna publish et" adımı bu turda bilinçli olarak denenmedi.

**Gerçek bir AWS IoT Core uç noktasına karşı, gerçekten doğrulandı:**
`tdf.py build` temiz şekilde derlenip bağlanıyor (`.text` 21648 bayt).
`tdf.py flash`, SWD üzerinden tam bir programlama-ve-doğrulama
döngüsünü tamamladı (`** Verified OK **`). `tdf.py monitor` ardından,
doğrudan gerçek donanımdan şunu gösterdi:

```text
[INFO] Credentials: all three slots loaded
[INFO] ESP32: joining SSID "..."
[INFO] ESP32: WIFI GOT IP
[INFO] ESP32: SNTP synced (epoch 1787779064)
[INFO] MQTT: client id "..." configured (scheme 5, mutual TLS)
[INFO] MQTT: SNI set to "iot.tiremo.ai"
[INFO] MQTT: connecting to iot.tiremo.ai:8883
[INFO] MQTT: connected (mutual TLS) -- MQTTUSERCFG/MQTTSNI/MQTTCONN all OK
```

O son satırın arkasındaki ham AT izi, sadece bu sürücünün çıplak bir
`OK`'ı yorumlaması değil, modülün kendi doğrulaması:

```text
{"data":"+MQTTCONNECTED:0,5,\"iot.tiremo.ai\",\"8883\",\"\",0\r\n\r\nOK"}
```

Scheme `5` ve gerçek uç nokta geri yansıtılmış `+MQTTCONNECTED` —
yerel bir loopback veya şanslı bir `OK` eşleşmesi değil, AWS IoT
Core'a karşı gerçek bir karşılıklı-TLS oturumu.

## Adım Adım

### 1. Kimlik bilgilerini yükle, sonra WiFi'a katıl

[`Sw/Src/prv_user_code.c`](../../../../Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/Sw/Src/prv_user_code.c),
Tutorial 02'nin tanıttığı aynı
[`cmake/embed_pem.cmake`](../../../../cmake/embed_pem.cmake)
mekanizmasıyla build zamanında `certs/*.pem`'den gömülen üç slotun
hepsini — root CA, cihaz sertifikası, cihaz anahtarı — WiFi'a hiç
dokunmadan *önce* yazıyor, çünkü `AT+SYSMFG` saf yerel NVS depolaması
ve bir ağa ihtiyaç duymuyor:

```c
LoadCred(TIREMO_ESP32_CREDSLOT_ROOT_CA, "ROOT_CA", g_rootCaPem, g_rootCaPem_len);
LoadCred(TIREMO_ESP32_CREDSLOT_CLIENT_CERT, "CLIENT_CERT", g_deviceCertPem, g_deviceCertPem_len);
LoadCred(TIREMO_ESP32_CREDSLOT_CLIENT_KEY, "CLIENT_KEY", g_deviceKeyPem, g_deviceKeyPem_len);
```

### 2. WiFi'a katıl, sonra SNTP — herhangi bir MQTT komutundan önce

```c
TIREMO_ESP32_JoinAP(TIREMO_WIFI_SSID, TIREMO_WIFI_PASSWORD, JOIN_TIMEOUT_MS);
TIREMO_ESP32_EnableSntp();
TIREMO_ESP32_WaitSntpEpoch(&epoch, SNTP_TIMEOUT_MS);
```

Bu adımın neden var olduğu için yukarıdaki **SNTP'yi Anlamak**'a bakın
— atlayın ve `MQTTCONN`, scheme 5 altında her seferinde, genel bir
`ERROR`'dan daha net bir sinyal olmadan başarısız olur.

### 3. Bağlantı sırasının kendisi

```c
TIREMO_ESP32_MqttUserConfigMtls(MQTT_CLIENT_ID);
TIREMO_ESP32_MqttSni(MQTT_ENDPOINT);
TIREMO_ESP32_MqttConnect(MQTT_ENDPOINT, MQTT_PORT, MQTT_CONNECT_TIMEOUT_MS);
```

[`tiremo_esp32.c`](../../../../Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/Sw/Drivers/esp32/Src/tiremo_esp32.c)
içindeki `TIREMO_ESP32_MqttUserConfigMtls()`, Tutorial 01'in düz-TCP
`MqttUserConfig()`'inden yeni, ayrı bir fonksiyon — scheme 5, boş
kullanıcı adı/parola (kimlik doğrulama kimlik bilgileriyle değil
sertifikayla), `cert_key_ID`/`CA_ID` ikisi de `0`. `TIREMO_ESP32_MqttSni()`,
`"OK"`'a gönderilen doğrudan bir `AT+MQTTSNI=0,"<host>"` — bunun
ötesinde bir yanıt ayrıştırmasına gerek yok.

### 4. Build, flash ve izle

```sh
tdf.py build
tdf.py flash
tdf.py monitor
```

Gerçek donanımda şu anda ne doğrulandığı için yukarıdaki **Çözüm**'e
bakın.

## Tamamlandığında

Modülün kendi `+MQTTCONNECTED` yanıtıyla doğrulanan, gerçek bir AWS
IoT Core uç noktasına karşı başarılı bir `AT+MQTTCONN` karşılıklı-TLS
oturumu — yukarıda gerçekten doğrulandı. Bu eğitimin orijinal "test
konusuna publish et, AWS konsolunda alımı doğrula" adımı bu turda
bilinçli olarak denenmedi (bkz. **Çözüm**); bu, müfredat gerçek AWS IoT
Core pub/sub'ına ulaştığında ele alınacak.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
