<p class="sphinxhide" align="right"><a href="../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>mTLS Credentials in Module NVS</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# mTLS Credentials in Module NVS

***Kart: Tiremo Cortex (ESP32) · Bölüm: Cloud Messaging (MQTT & TLS) / Feature Tutorials · Zorluk: Orta***

> Bu sayfa [İngilizce orijinalinin](../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

AWS IoT karşılıklı TLS gerektirir. ESP32 modülü — MCU değil — TLS
kimlik bilgilerini kendi NVS'sinde `AT+SYSMFG` üzerinden tutar. Bu
eğitim, Tutorial 03 onu gerçek bir bağlantı için kullanmadan önce,
tamamen bu depolama mekanizması hakkındadır.

## Neler Öğreneceksiniz

- Üç `TIREMO_ESP32_CredSlot_t` slotu ve neden MCU flash'ında değil modülde yaşadıkları
- `AT+SYSMFG` yazma/okuma/silme semantiği — `AT+MQTTPUBRAW` ile aynı şekilde gerçek bir uzunluk-önekli binary protokol
- Sertifika/anahtar materyalini `Sw/certs/` altında gerçek `.pem` dosyalarında tutmak, derleme zamanında üretici yazılımına gömülmüş — bir C string literal'e elle yazılmamış

## Ön Koşullar

- [01-MQTT_To_Public_Broker](../01-MQTT_To_Public_Broker/) — bu eğitim onun taşıma katmanını, minimal alt kümeye kırpılmış olarak yeniden kullanır (burada WiFi/MQTT gerekmiyor)
- Toolchain ve flaşlama [Getting Started](../../../../Getting_Started/README.md)'a göre kurulu
- Yazma/okuma/silme döngüsünü izlemek için `tdf.py monitor` (kurulum için [Getting_Started/04-Button_Read_And_Debounce'un Çözüm bölümüne](../../../../Getting_Started/04-Button_Read_And_Debounce/README.md#solution) bakın)
- Test sertifikalarını kendiniz yeniden oluşturmak isterseniz kendi makinenizde `openssl` — [`Sw/certs/`](../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/Sw/certs/)'dekiler zaten commit edilmiş ve flaşlamaya hazır

## Üç Slotu Anlamak (ve ESP-AT'nin Kendi Adlandırması)

[Espressif'in kendi `AT+SYSMFG` dokümantasyonu](https://github.com/espressif/esp-at/blob/master/docs/en/AT_Command_Examples/sysmfg_at_examples.rst),
her istemci özelliği için **ayrı** bir namespace seti ayırır — SSL
istemci, MQTT istemci, HTTPS istemci ve WebSocket istemci, hepsi aynı
üç şeyi (sunucuyu doğrulayan bir CA, bir cihaz sertifikası, bir cihaz
özel anahtarı) saklasa da, her birinin kendi seti var. Burada gerçekten
önemli olan set, MQTT istemcininki — çünkü `AT+MQTTUSERCFG`'nin
`cert_key_ID`/`CA_ID` parametrelerinin okuduğu yer orası:

| Bu sürücünün slotu | ESP-AT namespace | ESP-AT anahtarı | Amaç |
|---|---|---|---|
| `TIREMO_ESP32_CREDSLOT_ROOT_CA` | `mqtt_ca` | `mqtt_ca` | **İstemcinin** **sunucunun** sertifikasını doğrulamasını sağlar |
| `TIREMO_ESP32_CREDSLOT_CLIENT_CERT` | `mqtt_cert` | `mqtt_cert` | Bu cihazın kendi sertifikası |
| `TIREMO_ESP32_CREDSLOT_CLIENT_KEY` | `mqtt_key` | `mqtt_key` | Bu cihazın kendi özel anahtarı |

`mqtt_ca`, üzerinde durulmaya değer gerçek bir ESP-AT adı: "istemcinin
CA'sı" gibi geliyor, ama gerçek işi tam tersi — bu, *istemcinin* (bu
kart) *sunucunun* (AWS IoT) iddia ettiği kişi olduğunu kontrol etmek
için kullandığı CA sertifikası. AWS IoT eğitimleri genellikle aynı şeye
"Root CA" der. [`tiremo_esp32.h`](../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/Sw/Drivers/esp32/Include/tiremo_esp32.h),
ESP-AT'nin gerçek namespace dizesini sürücüde tutar (böylece resmi
dokümanları ararken eşleşir), ama *slot*'u gerçekte yaptığı şey için
`ROOT_CA` olarak adlandırır.

Eğitimin kendi orijinal adım anahatı bir yerde "dört slot," başka bir
yerde "üç" diyordu — bunların hiçbiri inşa edilmeden önceki bir
tutarsızlık. Gerçek komut referansını kontrol etmek bunu çözdü: üç.
Dördüncü bir namespace yok.

## Gerçek Bir Hata: Generic `client_*` Namespace, MQTT'nin Okuduğu Yer Değil

Bu eğitimin ilk sürümü `client_ca`/`client_cert`/`client_key`
namespace'lerini (`.0` anahtarlarla) kullanıyordu — Espressif'in
dokümanlarının genel "SSL istemci" başlığı altında ilk anlattığı set.
Yazma, geri okuma ve silme, gerçek donanımda, ilk seferde sorunsuz
geçti.

Bu geçen sonuç yanıltıcıydı. `AT+SYSMFG` genel amaçlı bir key-value
depolama — uydurma bir isim dahil, *herhangi* bir namespace adına
seve seve yazar ve ondan geri okur. Round-trip baytların doğru
çıkması, `AT+MQTTUSERCFG`'nin TLS motorunun gerçekten o namespace'e
baktığı anlamına gelmez. Bakmıyordu da: Espressif'in kendi
namespace/anahtar eşleme tablosu, **MQTT İstemci**'yi **SSL
İstemci**'den ayrı bir satır olarak listeliyor, kendi ayrılmış
setiyle — `mqtt_ca`/`mqtt_cert`/`mqtt_key`, `.0` eki yok, tek bir set.
Generic `client_*` seti, düz (MQTT olmayan) bir `AT+CIPSTART` SSL
bağlantısının okuduğu yer; MQTT oraya hiç bakmıyor.

Bu, yalnızca Tutorial 03 için araştırma yaparken, gerçek bir AWS IoT
bağlantısı kurmadan önce bu sürücüyü gerçek namespace tablosuna karşı
tekrar kontrol ederken ortaya çıktı — burada bir şeyin başarısız
olmasından değil. Temiz bir yazma / geri okuma / silme döngüsü,
*depolamanın* çalıştığının kanıtıdır; *kimlik bilgilerinin TLS
handshake'inin gerçekten baktığı yere bağlandığının* kanıtı değildir.
Bunlar farklı iddialar, ve Tutorial 03 için önemli olan yalnızca
ikincisi. Sürücü, header'daki doküman yorumu ve yukarıdaki tablo,
hepsi `mqtt_ca`/`mqtt_cert`/`mqtt_key`'e düzeltildi — düzeltmeden
sonra gerçek donanımda yeniden doğrulandı (aşağıdaki **Çözüm**'e
bakın).

## Eksik `.0`'ı Anlamak

Generic SSL istemci namespace'i, birden fazla indeksli sertifika
setini destekler — `client_cert.0`, `client_cert.1`, vb. — birkaç
sertifika arasında rotasyon yapan dağıtımlar için. **MQTT** istemci
namespace'i desteklemez: Espressif'in kendi dokümanları her namespace
için yalnızca bir anahtar listeler, ve o anahtarın adı, hiç indeks
olmadan, namespace'in adıyla aynıdır (`mqtt_cert`, `mqtt_cert.0`
değil). Bu sürücünün `prvESP32CredSlotInfo()`'su bunu doğrudan
yansıtır — namespace ve anahtar, üç slot için de aynı dizedir.

## `certs/` Klasörünü Anlamak (Bir C String Değil)

Gerçek PEM içeriği, bir C string literal'e elle yazmak için tam olarak
sefil ve hataya açık türden bir şey — bir yanlış escape edilmiş yeni
satır ve sertifika sessizce ayrıştırılmayı durdurur. Bu yüzden buradaki
üç kimlik bilgisi gerçek dosyalar:
[`Sw/certs/root_ca.pem`](../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/Sw/certs/root_ca.pem),
[`client_cert.pem`](../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/Sw/certs/client_cert.pem),
ve [`client_key.pem`](../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/Sw/certs/client_key.pem)
— gerçek, yerel olarak üretilmiş, kendinden imzalı **test**
sertifikaları (`openssl req -x509 -newkey rsa:2048 ...`), kendi konu
satırlarında (`CN=... TEST ... - not for real use`) böyle olarak açıkça
etiketlenmiş. Güven konusunda hiçbir şey kanıtlamazlar; yalnızca bu
eğitime yazacak, geri okuyacak ve karşılaştıracak gerçek PEM baytları
vermek için varlar.

[`cmake/embed_pem.cmake`](../../../../cmake/embed_pem.cmake) (depo
kökü — gelecekteki herhangi bir eğitim bu ihtiyaç duyarsa yeniden
kullanılabilir), bir `.pem` dosyasını derleme zamanında üretilen bir
`.c` kaynağına dönüştürür:

```cmake
function(embed_pem name symbol)
    add_custom_command(
        OUTPUT ${GENERATED_DIR}/${name}.c
        COMMAND ${CMAKE_COMMAND} -DINPUT=${CERTS_DIR}/${name}.pem
                -DOUTPUT=${GENERATED_DIR}/${name}.c -DSYMBOL=${symbol}
                -P ${EMBED_PEM_SCRIPT}
        DEPENDS ${CERTS_DIR}/${name}.pem ${EMBED_PEM_SCRIPT}
    )
endfunction()
```

`const unsigned char g_rootCaPem[] = { 0x2d, 0x2d, ... };` artı eşleşen
bir `_len` üretiyor, `prv_user_code.c` bunu düz bir `extern` ile
referanslıyor — XXD yok, Perl yok, yalnızca Git-Bash'te çalışan bir
araç yok, sadece hex-to-array dönüşümünü yapan CMake'in kendi
`file(READ ... HEX)`'i, bu yüzden bu, `tdf.py build`'in zaten
gerektirdiğinin ötesinde hiçbir şey olmadan Windows/macOS/Linux'ta
aynı şekilde çalışır. Tutorial 03 bu tam mekanizmayı yeniden kullanır:
bu aynı üç `.pem` dosyasının *içeriğini* gerçek bir AWS IoT cihaz
sertifikası, root CA, ve özel anahtarla değiştirin, ve derleme betiği
hakkında hiçbir şey değişmez.

## SYSMFG Protokolünü Anlamak

Yazma (`operation=2`), Tutorial 01'deki `AT+MQTTPUBRAW` ile aynı
iki aşamalı şekil — komutu bir bayt sayısıyla yayınla, düz bir `>`
istemini bekle, sonra tam olarak o kadar ham bayt aktar:

```c
(void)snprintf(command, sizeof(command), "AT+SYSMFG=2,\"%s\",\"%s\",8,%lu\r\n",
               pNamespace, pKey, (unsigned long)length);
TIREMO_ESP32_SendCommand(command, ">", timeout);
TIREMO_HAL_UART_Transmit(port, pData, length, true);
TIREMO_ESP32_SendCommand(NULL, "OK", timeout);
```

(Tip `8` = binary — ESP-AT'nin kendi SYSMFG dokümanlarındaki tip
tablosuyla eşleşiyor, ve teknik olarak metin ama satır kırıklı ve
tek bir string gibi yanlış ele alınması kolay veri için modül
tarafında herhangi bir string işlemeden kaçınıyor.)

Okuma (`operation=1`), değeri yanıtın içinde satır içi yanıtlıyor:

```text
+SYSMFG:"mqtt_cert","mqtt_cert",8,1164,<1164 ham bayt>
```

[`tiremo_esp32.c`](../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/Sw/Drivers/esp32/Src/tiremo_esp32.c)'deki
`TIREMO_ESP32_CredRead()`, bunu Tutorial 01'in `MqttWaitMessage()`'ının
`+MQTTSUBRECV`'i ayrıştırdığı aynı şekilde ayrıştırıyor: zaten bildiği
iki tırnaklı alanı atla (namespace, anahtar), tip rakamını atla, ondalık
uzunluğu ayrıştır, sonra yanıt tamponunun ötesinde o kadar baytı zaten
tutup tutmadığını kontrol et. Değer ancak o zaman tamamdır — bir metin
sezgiseli değil, tam çok satırlı PEM değeri için doğru olan gerçek bir
uzunluk-önekli ayrıştırma. Silme (`operation=0`), veri fazı olmayan,
`"OK"` üzerinde eşleşen düz bir `AT+SYSMFG=0,"ns","key"`.

**Silinmiş** bir slotu okumak bunun yerine framed bir `\r\nERROR\r\n`
ile yanıtlıyor — `CredRead()` bunu normal, beklenen bir sonuç olarak
ele alıyor (`ERROR` değil `WARN` seviyesinde loglanıyor), çünkü "slotun
şimdi boş olduğunu kanıtla," bu eğitimin kendi adım anahatının Adım
3'ünün tam olarak istediği şey.

## Çözüm

[`Sw/`](../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/Sw/) gerçek, derlenebilir bir uygulamaya sahip:

```sh
cd Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

Tutorial 01 ile aynı CMake şekli, bu eğitimin ihtiyaç duymadığı
WiFi/MQTT ağırlığı olmadan — sürücü
([`tiremo_esp32.c`](../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/Sw/Drivers/esp32/Src/tiremo_esp32.c))
[ESP32 Connectivity Tutorial 01](../../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/README.md)'in
minimal taşımasına (yalnızca `Init`/`SendCommand`/`IsPresent`/`DisableEcho`)
dayanıyor, üzerine üç kimlik bilgisi fonksiyonu eklenmiş. Hiç
`wifi_secrets.h` gerekmiyor — `AT+SYSMFG` asla ağa dokunmuyor.

**Bağlı kartta, gerçekten, gerçek hayatta doğrulandı — yukarıdaki
namespace düzeltmesinden sonra:** `tdf.py build` temiz şekilde
derlenip bağlanıyor (`.text` 21140 bayt). `tdf.py flash`, SWD üzerinden
tam bir programlama-ve-doğrulama döngüsünü tamamladı (`** Verified OK
**`). `tdf.py monitor` ardından, doğrudan gerçek donanımdan, düzeltilmiş
`mqtt_*` namespace'lerine karşı temiz bir geçiş gösterdi:

```text
[INFO] ROOT_CA: writing 1245 bytes...
[INFO] ROOT_CA: read-back matches byte-for-byte (1245 bytes)
[INFO] ROOT_CA: erased
[WARN] ESP32: SYSMFG read "mqtt_ca"/"mqtt_ca": ERROR (empty slot?)
[INFO] ROOT_CA: read after erase correctly failed (slot empty)
[INFO] CLIENT_CERT: writing 1241 bytes...
[INFO] CLIENT_CERT: read-back matches byte-for-byte (1241 bytes)
[INFO] CLIENT_CERT: erased
[WARN] ESP32: SYSMFG read "mqtt_cert"/"mqtt_cert": ERROR (empty slot?)
[INFO] CLIENT_CERT: read after erase correctly failed (slot empty)
[INFO] CLIENT_KEY: writing 1704 bytes...
[INFO] CLIENT_KEY: read-back matches byte-for-byte (1704 bytes)
[INFO] CLIENT_KEY: erased
[WARN] ESP32: SYSMFG read "mqtt_key"/"mqtt_key": ERROR (empty slot?)
[INFO] CLIENT_KEY: read after erase correctly failed (slot empty)
[INFO] mTLS_Credentials_In_Module_NVS: ALL THREE SLOTS PASSED
```

Üç slotun hepsi: yazma, bayt-bayt okuma-geri eşleşmesi (1245 / 1241 /
1704 bayt — üç test PEM dosyasının gerçek boyutları, yuvarlak/uydurma
sayılar değil), silme, ve doğru şekilde başarısız olan bir silme-sonrası
okuma. `[WARN] ... ERROR (empty slot?)` satırları hata değil — sürücünün
az önce silinmiş bir slotu okumanın tam beklenen sonucunu loglaması,
şimdi üçü için de görünür (her satırdaki namespace dizeleri, bunun
orijinal `client_*` değil `mqtt_*`'ye karşı çalıştığının gerçek kanıtı).

## Adım Adım

### 1. Bilinçli olarak minimal bir taşıma

[`Sw/Src/main.c`](../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/Sw/Src/main.c)
ve sürücünün `Init`/`SendCommand`/`IsPresent`/`DisableEcho` çekirdeği,
ESP32 Connectivity Tutorial 01'den değişmedi — WiFi bağlantısı yok,
MQTT yok. `AT+SYSMFG`, modül üzerinde saf yerel NVS depolaması; bir ağa
ihtiyaç duymuyor, bu yüzden bu eğitim sadece orada kullanılmadan
dursun diye bir tane inşa etmiyor.

### 2. Kimlik bilgisi slot API'si

[`Sw/Drivers/esp32/Include/tiremo_esp32.h`](../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/Sw/Drivers/esp32/Include/tiremo_esp32.h):

```c
bool TIREMO_ESP32_CredWrite(TIREMO_ESP32_CredSlot_t slot, const uint8_t *pData, uint32_t length);
bool TIREMO_ESP32_CredRead(TIREMO_ESP32_CredSlot_t slot, uint8_t *pOut, uint32_t outSize, uint32_t *pLengthOut);
bool TIREMO_ESP32_CredErase(TIREMO_ESP32_CredSlot_t slot);
```

Özel bir `prvESP32CredSlotInfo()`, her `TIREMO_ESP32_CredSlot_t`'i
kendi `{namespace, key}` çiftine eşler — `ROOT_CA`'nın neden ESP-AT
adı `mqtt_ca`'ya eşlendiği için yukarıdaki **Üç Slotu Anlamak**'a
bakın.

### 3. Test — yaz, doğrula, sil, doğrula

[`Sw/Src/prv_user_code.c`](../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/Sw/Src/prv_user_code.c)'nin
`TestSlot()`'u, eğitimin kendi adım anahatındaki tam döngüyü üç gömülü
`.pem` dosyasının her birine karşı çalıştırır, yalnızca çağrının
başarılı olup olmadığını kontrol etmek yerine geri okunan baytları
orijinaliyle `memcmp` ile karşılaştırarak:

```c
TIREMO_ESP32_CredWrite(slot, pData, length);
TIREMO_ESP32_CredRead(slot, s_readBuf, sizeof(s_readBuf), &readLen);
/* readLen == length && memcmp(s_readBuf, pData, length) == 0 */
TIREMO_ESP32_CredErase(slot);
/* CredRead() artık false döndürmesi bekleniyor */
```

### 4. Derle, flaşla, izle

```sh
tdf.py build
tdf.py flash
tdf.py monitor
```

Bu eğitim için gerçek donanımda tam olarak neyin doğrulandığı için
yukarıdaki **Çözüm** bölümüne bakın.

## Ne Zaman Tamamlanmış Sayılır

Yazma/okuma/silme, üç kimlik bilgisi slotunun hepsi için doğru şekilde
gidiş-dönüş yapar — gerçek donanımda gerçekten doğrulandı, bkz.
**Çözüm**.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
