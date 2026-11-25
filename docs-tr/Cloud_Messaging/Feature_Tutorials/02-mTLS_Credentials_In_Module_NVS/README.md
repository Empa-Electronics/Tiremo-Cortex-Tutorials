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

***Kart: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Bölüm: Cloud Messaging (MQTT & TLS) / Feature Tutorials · Zorluk: Orta***

> Bu sayfa [İngilizce orijinalinin](../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Tiremo Cloud karşılıklı TLS (mTLS) ister: sunucu karta kim olduğunu kanıtlar, kart da sunucuya kim olduğunu kanıtlar. TLS kimlik bilgilerini MCU değil, ESP32 modülü tutar — kendi NVS'inde, `AT+SYSMFG` ile yazılmış olarak. Bu eğitim, [eğitim 03](../03-MQTT_mTLS_To_AWS_IoT/README.md) onu gerçek bir bağlantı için kullanmadan önce yalnızca bu saklama mekanizmasıyla ilgilidir.

## Neler Öğreneceksiniz

- Üç `TIREMO_ESP32_CredSlot_t` bölmesi (slot) ve neden MCU flash'ında değil modülde durdukları
- `AT+SYSMFG` ile yazma / okuma / silme — eğitim 01'deki `AT+MQTTPUBRAW` ile aynı yapıda, uzunluk önekli ikili bir protokol
- Sertifika ve anahtarları bir C dize sabitine elle yazmak yerine `Sw/certs/` altında gerçek `.pem` dosyalarında tutmak ve derleme sırasında yazılıma gömmek

## Ön Koşullar

- [01-MQTT_To_Public_Broker](../01-MQTT_To_Public_Broker/README.md) — bu eğitim onun taşıma katmanını, en küçük alt kümesine indirerek kullanır (burada WiFi ya da MQTT gerekmez)
- Test sertifikası üretmek için bilgisayarınızda `openssl` — bkz. [Test sertifikasını üretin](#test-sertifikasını-üretin)
- [Getting Started](../../../Getting_Started/) kurulum betikleri çalıştırılmış olmalı, böylece `tdf` kullanılabilir
- Log'u izlemek için: `tdf monitor` ya da kartın USB seri portunda 115200 8N1'de herhangi bir seri terminal (Tera Term, PuTTY, screen, minicom, …)
- USB kabloyla bilgisayarınıza bağlı bir Tiremo Cortex kartı (ABOV A34G43A MCU — bkz. [Donanıma Genel Bakış](../../../Hardware.md))

> **Uyarı — bu eğitim modülün MQTT kimlik bilgilerini siler.** Test, ESP32'nin üç MQTT sertifika bölmesine (`mqtt_ca`, `mqtt_cert`, `mqtt_key`) test verisi yazar ve ardından onları **siler**. Kartınızdaki modülde zaten gerçek bir cihaz sertifikası varsa (örneğin eğitim 03'ten ya da Tiremo Cloud kurulumundan), sonrasında o sertifika gider ve sertifika yeniden yazılana kadar kart buluta bağlanamaz.

## Üç bölmeyi anlamak (ve ESP-AT'nin kendi adlandırması)

[Espressif'in `AT+SYSMFG` dökümanı](https://github.com/espressif/esp-at/blob/master/docs/en/AT_Command_Examples/sysmfg_at_examples.rst), her istemci özelliği için **ayrı** bir ad alanı (namespace) kümesi ayırır — SSL istemcisi, MQTT istemcisi, HTTPS istemcisi ve WebSocket istemcisinin her birinin kendi kümesi vardır; hepsi aynı üç tür şeyi (sunucuyu doğrulayan bir CA, bir cihaz sertifikası, bir cihaz özel anahtarı) saklasa da. Burada önemli olan MQTT istemcisinin kümesidir, çünkü `AT+MQTTUSERCFG`'nin `cert_key_ID` / `CA_ID` parametreleri oradan okur:

| Bu sürücünün bölmesi | ESP-AT ad alanı | ESP-AT anahtarı | Amacı |
|---|---|---|---|
| `TIREMO_ESP32_CREDSLOT_ROOT_CA` | `mqtt_ca` | `mqtt_ca` | **İstemcinin** **sunucunun** sertifikasını doğrulamasını sağlar |
| `TIREMO_ESP32_CREDSLOT_CLIENT_CERT` | `mqtt_cert` | `mqtt_cert` | Bu cihazın kendi sertifikası |
| `TIREMO_ESP32_CREDSLOT_CLIENT_KEY` | `mqtt_key` | `mqtt_key` | Bu cihazın kendi özel anahtarı |

`mqtt_ca` üzerinde durmaya değer bir addır: "istemcinin CA'sı" gibi duyulur, ama görevi tam tersidir — *istemcinin* (bu kartın) *sunucunun* iddia ettiği kişi olup olmadığını kontrol etmek için kullandığı CA sertifikasıdır. TLS kılavuzları buna genellikle "Root CA" der. [`tiremo_esp32.h`](../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/Sw/Drivers/esp32/Include/tiremo_esp32.h), resmî dökümanda aradığınızda eşleşsin diye ESP-AT'nin gerçek ad alanı dizesini sürücüde tutar, ama *bölmeyi* yaptığı işe göre `ROOT_CA` olarak adlandırır.

## Neden `client_*` değil `mqtt_*` olması gerektiğini anlamak

Espressif'in dökümanı önce genel SSL istemcisi ad alanlarını anlatır: `.0`, `.1`, … anahtarlarıyla `client_ca` / `client_cert` / `client_key`. Bunlara yazmak gayet iyi çalışır — tuzak da budur. `AT+SYSMFG` genel bir anahtar-değer deposudur: *herhangi* bir ad alanına, uydurma bir ada bile yazar ve geri okur. Temiz bir yazma / geri okuma / silme döngüsü deponun çalıştığını kanıtlar; kimlik bilgilerinin TLS el sıkışmasının baktığı yerde olduğunu **kanıtlamaz**. ESP-AT'nin ad alanı tablosunda **MQTT istemcisi** kendi satırıdır: `AT+MQTTUSERCFG`'nin TLS'i yalnızca `mqtt_ca` / `mqtt_cert` / `mqtt_key`'den okur. `client_*` kümesi düz (MQTT olmayan) SSL bağlantılarına (`AT+CIPSTART`) aittir; MQTT oraya hiç bakmaz.

MQTT ad alanlarında `.0` da yoktur: genel SSL istemcisi, sertifikalar arasında dönüşüm yapan cihazlar için birden çok numaralı sertifika kümesini (`client_cert.0`, `client_cert.1`, …) destekler; MQTT istemcisinde ise her ad alanı için tam olarak bir anahtar vardır ve adı ad alanıyla aynıdır (`mqtt_cert.0` değil, `mqtt_cert`). Sürücünün `prvESP32CredSlotInfo()` fonksiyonu bunu doğrudan yansıtır — üç bölmenin hepsinde ad alanı ve anahtar aynı dizedir.

## `certs/` klasörünü anlamak (C dizesi değil)

Gerçek PEM içeriği, bir C dize sabitine elle yazması tam bir eziyet ve hataya açık olan türden bir şeydir — yanlış kaçışlanmış tek bir satır sonu ve sertifika sessizce ayrıştırılamaz hâle gelir. Bu yüzden buradaki üç kimlik bilgisi [`Sw/certs/`](Sw/certs/) içindeki gerçek dosyalardır:

- `root_ca.pem` commit'lidir: kendinden imzalı bir TEST kök CA'sı (`CN=... TEST Root CA - not for real use`).
- `client_cert.pem` ve `client_key.pem` gitignore'dadır: kendinden imzalı TEST çiftini kendiniz üretirsiniz (aşağıya bakın). Özel anahtar asla bir commit'e girmemelidir.

Güven hakkında hiçbir şey kanıtlamazlar — hiçbir sunucu onları kabul etmez. Bu eğitime yazılacak, geri okunacak ve karşılaştırılacak gerçek PEM baytları sağlamak için vardırlar. Bkz. [`certs/README.md`](Sw/certs/README.md).

[`cmake/embed_pem.cmake`](../../../../cmake/embed_pem.cmake) (repo kökünde, gerektiren her eğitim tarafından yeniden kullanılabilir), derleme sırasında bir `.pem` dosyasını üretilmiş bir `.c` kaynağına dönüştürür:

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

Sonuç, `const unsigned char g_rootCaPem[] = { 0x2d, 0x2d, ... };` ve ona eşlik eden bir `_len`'dir; `prv_user_code.c` bunlara düz bir `extern` ile başvurur. Dönüştürme yalnızca CMake'in kendi `file(READ ... HEX)` özelliğini kullanır — `xxd`, Perl ya da yalnızca Git Bash'te olan bir araç yoktur — böylece `tdf build`'in zaten gerektirdiklerinin ötesinde hiçbir şey olmadan Windows, macOS ve Linux'ta aynı şekilde çalışır. Eğitim 03 bu mekanizmayı yeniden kullanır: aynı üç `.pem` dosyasının *içeriğini* gerçek bir cihaz sertifikası, onun özel anahtarı ve sunucunun kök CA'sıyla değiştirin; derlemede hiçbir şey değişmez.

## SYSMFG protokolünü anlamak

Yazma (`operation=2`), eğitim 01'deki `AT+MQTTPUBRAW` ile aynı iki aşamalı yapıdadır — komutu bir bayt sayısıyla gönderin, yalın bir `>` istemini bekleyin, ardından tam olarak o kadar ham bayt iletin:

```c
(void)snprintf(command, sizeof(command), "AT+SYSMFG=2,\"%s\",\"%s\",8,%lu\r\n",
               pNamespace, pKey, (unsigned long)length);
TIREMO_ESP32_SendCommand(command, ">", timeout);
TIREMO_HAL_UART_Transmit(port, pData, length, true);
TIREMO_ESP32_SendCommand(NULL, "OK", timeout);
```

Tür `8`, ESP-AT'nin SYSMFG dökümanındaki tür tablosuna göre ikili veri anlamına gelir. Teknik olarak metin olan ama çok satırlı ve dize olarak yanlış işlenmesi kolay veriler için modül tarafında herhangi bir dize işlemesini önler.

Okuma (`operation=1`), değeri yanıtın içinde döndürür:

```text
+SYSMFG:"mqtt_cert","mqtt_cert",8,1262,<1262 raw bytes>
```

[`tiremo_esp32.c`](../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/Sw/Drivers/esp32/Src/tiremo_esp32.c) içindeki `TIREMO_ESP32_CredRead()`, bunu eğitim 01'deki `MqttWaitMessage()`'ın `+MQTTSUBRECV`'yi ayrıştırdığı şekilde ayrıştırır: zaten bildiği iki tırnaklı alanı (ad alanı, anahtar) atlar, tür rakamını atlar, ondalık uzunluğu ayrıştırır, sonra yanıt tamponunun ondan sonra o kadar bayt içerip içermediğine bakar. Değer ancak o zaman tamamdır — çok satırlı PEM değerinin tamamı için doğru olan, gerçek bir uzunluk önekli ayrıştırma. Silme (`operation=0`), veri aşaması olmadan `"OK"` ile eşleşen düz bir `AT+SYSMFG=0,"ns","key"` komutudur.

**Silinmiş** bir bölmeyi okumak bunun yerine çerçeveli bir `\r\nERROR\r\n` döndürür. `CredRead()` bunu normal, beklenen bir sonuç olarak ele alır (`ERROR` değil `WARN` olarak loglanır), çünkü bölmenin artık boş olduğunu kanıtlamak tam olarak testin son adımıdır.

## Çözüm

### Test sertifikasını üretin

Repo kök dizininden bu eğitimin `Sw/` klasörüne geçin ve TEST istemci çiftini üretin (tek satır):

```sh
cd Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/Sw
openssl req -x509 -newkey rsa:2048 -nodes -days 3650 -keyout certs/client_key.pem -out certs/client_cert.pem -subj "/CN=Tiremo Cortex Tutorial 02 TEST client - not for real use"
```

Windows'ta `openssl`, Git for Windows ile gelir: komutu Git Bash'te çalıştırın ve konuyu orada `"//CN=..."` olarak yazın (Git Bash aksi hâlde baştaki `/`'i bir Windows yoluna çevirir). macOS ve Linux'ta `openssl` genellikle zaten kuruludur. Bu adımı unutursanız derleme `Missing test client credentials` hatasıyla durur ve aynı komutu basar.

### Derleyin, yükleyin, izleyin

```sh
tdf build
tdf flash
tdf monitor
```

(`tdf`'nin ne olduğu için [Getting_Started/01-Hello_Blink'in Çözüm bölümüne](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) bakın. macOS/Linux'ta `tdf.py` takma ad olarak da çalışır; Windows'ta `tdf` kullanın. `tdf monitor`'ü çalıştırmadan önce kartın seri portundaki diğer terminalleri kapatın — portu aynı anda yalnızca bir program açabilir.) `wifi_secrets.h` gerekmez: `AT+SYSMFG` ağa hiç dokunmaz.

Sürücü ([`tiremo_esp32.c`](../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/Sw/Drivers/esp32/Src/tiremo_esp32.c)), [ESP32 Connectivity eğitim 01](../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/README.md)'in en küçük taşıma katmanıdır (`Init` / `SendCommand` / `IsPresent` / `DisableEcho`); üç kimlik bilgisi fonksiyonu onun üzerine eklenmiştir.

**Gerçek donanımda doğrulandı:** `tdf build`, macOS'ta ve Windows 11'de hatasız derlenip bağlanıyor (`.text` yaklaşık 21 KB; kesin boyut toolchain sürümüne göre biraz değişir, ve linker [01-Hello_Blink'in Çözüm bölümünde](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) açıklanan beklenen ve zararsız `_close` / `_lseek` / `_read` / `_write is not implemented` uyarılarını basar). `tdf flash`, gerçek, bağlı bir Tiremo Cortex kartına karşı SWD üzerinden tam bir programlama-ve-doğrulama döngüsünü tamamladı (`** Verified OK **`). Test açılışta bir kez çalışır:

```text
[INFO] mTLS_Credentials_In_Module_NVS: boot
[INFO] ESP32: power cycling module...
[INFO] ESP32: power-on boot delay elapsed
[INFO] TIREMO_ESP32_Init: UART2 ready
[INFO] ESP32: command echo disabled (ATE0)
[INFO] ROOT_CA: writing 1266 bytes...
[INFO] ROOT_CA: read-back matches byte-for-byte (1266 bytes)
[INFO] ROOT_CA: erased
[WARN] ESP32: SYSMFG read "mqtt_ca"/"mqtt_ca": ERROR (empty slot?)
[INFO] ROOT_CA: read after erase correctly failed (slot empty)
[INFO] CLIENT_CERT: writing 1262 bytes...
[INFO] CLIENT_CERT: read-back matches byte-for-byte (1262 bytes)
[INFO] CLIENT_CERT: erased
[WARN] ESP32: SYSMFG read "mqtt_cert"/"mqtt_cert": ERROR (empty slot?)
[INFO] CLIENT_CERT: read after erase correctly failed (slot empty)
[INFO] CLIENT_KEY: writing 1732 bytes...
[INFO] CLIENT_KEY: read-back matches byte-for-byte (1732 bytes)
[INFO] CLIENT_KEY: erased
[WARN] ESP32: SYSMFG read "mqtt_key"/"mqtt_key": ERROR (empty slot?)
[INFO] CLIENT_KEY: read after erase correctly failed (slot empty)
[INFO] mTLS_Credentials_In_Module_NVS: ALL THREE SLOTS PASSED
[INFO] mTLS_Credentials_In_Module_NVS: idle (test already ran once at boot)
```

Her bölme için: yazma, bayt bayt eşleşen bir geri okuma, silme ve silme sonrasında doğru şekilde başarısız olan bir okuma. `[WARN] ... ERROR (empty slot?)` satırları hata değildir — az önce silinmiş bir bölmeyi okumanın beklenen sonucudur ve her satırdaki ad alanı hangi bölmenin kontrol edildiğini gösterir. Testten sonra her 10 saniyede bir `idle` satırı tekrarlanır; testi yeniden çalıştırmak için RESET'e basın.

Bayt sayıları üç `.pem` dosyanızın boyutlarıdır; bu yüzden sizinkiler biraz farklı olacaktır: istemci çifti size aittir ve Windows'ta CRLF satır sonlarıyla yapılan bir checkout, `root_ca.pem`'i macOS ya da Linux'takinden birkaç bayt uzun yapar.

## Adım Adım İnceleme

### 1. Bilerek küçük tutulmuş bir taşıma katmanı

[`Sw/Src/main.c`](../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/Sw/Src/main.c) ve sürücünün `Init` / `SendCommand` / `IsPresent` / `DisableEcho` çekirdeği ESP32 Connectivity eğitim 01 ile aynıdır — WiFi bağlantısı yok, MQTT yok. `AT+SYSMFG` modüldeki yerel bir depodur; ağa ihtiyaç duymaz.

### 2. Kimlik bilgisi bölmesi API'si

[`Sw/Drivers/esp32/Include/tiremo_esp32.h`](../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/Sw/Drivers/esp32/Include/tiremo_esp32.h):

```c
bool TIREMO_ESP32_CredWrite(TIREMO_ESP32_CredSlot_t slot, const uint8_t *pData, uint32_t length);
bool TIREMO_ESP32_CredRead(TIREMO_ESP32_CredSlot_t slot, uint8_t *pOut, uint32_t outSize, uint32_t *pLengthOut);
bool TIREMO_ESP32_CredErase(TIREMO_ESP32_CredSlot_t slot);
```

Özel bir `prvESP32CredSlotInfo()` fonksiyonu her `TIREMO_ESP32_CredSlot_t`'yi kendi `{ad alanı, anahtar}` çiftine eşler — `ROOT_CA`'nın neden ESP-AT adı `mqtt_ca`'ya eşlendiği için yukarıdaki bölme tablosuna bakın.

### 3. Test — yaz, doğrula, sil, doğrula

[`Sw/Src/prv_user_code.c`](../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/Sw/Src/prv_user_code.c) içindeki `TestSlot()`, gömülü üç `.pem` dosyasının her biri için tam döngüyü çalıştırır; yalnızca çağrının başarılı olup olmadığına bakmak yerine geri okunan baytları `memcmp` ile karşılaştırır:

```c
TIREMO_ESP32_CredWrite(slot, pData, length);
TIREMO_ESP32_CredRead(slot, s_readBuf, sizeof(s_readBuf), &readLen);
/* readLen == length && memcmp(s_readBuf, pData, length) == 0 */
TIREMO_ESP32_CredErase(slot);
/* CredRead() now expected to return false */
```

## Tamamlanma Ölçütü

Log, üç bölmenin hepsi için `read-back matches byte-for-byte`, `erased` ve `read after erase correctly failed (slot empty)` gösteriyor ve `ALL THREE SLOTS PASSED` ile bitiyor.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
