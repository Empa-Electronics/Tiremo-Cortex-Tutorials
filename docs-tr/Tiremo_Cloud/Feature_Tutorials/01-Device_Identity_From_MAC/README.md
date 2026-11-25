<p class="sphinxhide" align="right"><a href="../../../../Tiremo_Cloud/Feature_Tutorials/01-Device_Identity_From_MAC/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>Device Identity from MAC</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Device Identity from MAC

***Kart: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Bölüm: Tiremo Cloud / Feature Tutorials · Zorluk: Orta***

> Bu sayfa [İngilizce orijinalinin](../../../../Tiremo_Cloud/Feature_Tutorials/01-Device_Identity_From_MAC/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Fleet provisioning ve shadow topic'lerinin ikisi de cihaz başına kararlı bir ad ister. `"tiremo-cortex-01"` gibi koda gömülü bir ad tek bir test kartında işe yarar, ama aynı üretici yazılımı ikinci bir karta yüklendiği anda bozulur. ESP32'nin zaten benzersiz, 48 bitlik bir station MAC adresi var; bu eğitim onu `AT+CIPSTAMAC?` ile okur ve sonraki eğitimlerin Thing adı ve MQTT client ID olarak kullanabileceği 12 haneli, küçük harfli hex bir `deviceId`'ye çevirir.

Bu `Sw/` WiFi'ye **bağlanmaz** ve Tiremo Cloud ile konuşmaz. Kimlik bir donanım özelliğidir, bir bulut oturumu değil.

## Neler Öğreneceksiniz

- `AT+CIPSTAMAC?` ve beklemenin neden `"+CIPSTAMAC:"` değil `"OK"` ile eşleşmesi gerektiği
- Altı MAC baytını ayırıcısız, 12 hex haneli, küçük harfli bir kimliğe biçimlemek
- Kimliğin neden üretici yazılımı kaynağına gömülmek yerine modülden okunduğu

## Ön Koşullar

- [ESP32 Connectivity / 01-AT_Command_Transport](../../../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/README.md) — bu `Sw/`, o taşıma katmanı artı `GetHwEui()`'dir
- [Getting Started](../../../Getting_Started/) bölümündeki kurulum betikleri çalıştırılmış olmalı, böylece `tdf` kullanılabilir
- Logu izlemek için: `tdf monitor` veya herhangi bir seri terminal (Tera Term, PuTTY, screen, minicom, …), kartın USB seri portunda 115200 8N1
- Bilgisayarınıza USB kablosuyla bağlı bir Tiremo Cortex kartı (ABOV A34G43A MCU — bkz. [Donanım Genel Bakışı](../../../Hardware.md))

Müfredatta bir önceki *bölüm* Cloud Messaging'dir. Bu eğitim için MQTT broker gerekmez.

## Platform dökümanı

Bu eğitimin ürettiği ad, sonraki eğitimlerin
[fleet provisioning](https://docs.tiremo.ai/device-provisioning/getting-started)
sırasında `deviceIdentifier` olarak ve shadow topic'lerinde Thing adı olarak
gönderdiği addır ([Device State Controller](https://docs.tiremo.ai/device-state-controller/introduction)).

## Station MAC ile uydurulmuş bir seri numarası arasındaki farkı anlamak

Modülün iki MAC adresi vardır. **Station** (`AT+CIPSTAMAC?`), WiFi istemcisi olarak kullandığı adrestir. SoftAP (`AT+CIPAPMAC?`) genellikle station adresinin bir fazlasıdır. Filo kimliği station MAC'i esas almalıdır: bu, bir AP'nin zaten gördüğü adrestir ve sonradan BluFi veya SoftAP'yi açtığınızda değişmez.

Tipik bir yanıt (örnek adres):

```text
+CIPSTAMAC:"aa:bb:cc:11:22:33"
OK
```

`TIREMO_ESP32_GetHwEui()` sondaki `"OK"`'i bekler, ardından tırnak içindeki, iki nokta üst üsteyle ayrılmış hex'i ayrıştırır. `"+CIPSTAMAC:"` ile eşleşmek, bu on bir karakter gelir gelmez — bayt bayt — geri döner ve tırnaklı adres hâlâ yoldayken kalır. Aynı tuzak `AT+CWJAP?` için [ESP32 Connectivity / 02-WiFi_Station_Join](../../../ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/README.md) eğitiminde anlatılır.

Altı bayt ardından kanonik bir kimliğe dönüşür:

| Biçim | Örnek | Kim kullanır |
|------|---------|-------------|
| İki noktalı MAC (yalnızca log) | `aa:bb:cc:11:22:33` | İnsanlar |
| `deviceId` | `aabbcc112233` | Sonraki eğitimlerde Thing adı / MQTT client ID |

Küçük harf, `0x` yok, iki nokta yok. Thing adları büyük/küçük harfe duyarlıdır; burada tek bir kanonik yazım seçmek, fleet provisioning şablonlarının ve shadow topic'lerinin hiçbir zaman çelişmemesini sağlar.

`TIREMO_Identity_FromEui()`, [`tiremo_identity.c`](../../../../Tiremo_Cloud/Feature_Tutorials/01-Device_Identity_From_MAC/Sw/Common/Src/tiremo_identity.c) içindedir; böylece bu ada ihtiyaç duyan bulut eğitimleri — [03](../03-Fleet_Provisioning_By_Claim/README.md) ve sonrası — kendi biçimleyicilerini yazmadan aynı dizeyi kullanır. Sürücü yalnızca ham baytları döndürür.

## Çözüm

Depo kök dizininden:

```sh
cd Tiremo_Cloud/Feature_Tutorials/01-Device_Identity_From_MAC/Sw
tdf build
tdf flash
tdf monitor
```

(`tdf`'nin ne olduğu için [Getting_Started/01-Hello_Blink'in Çözüm bölümüne](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) bakın. macOS/Linux'ta `tdf.py` de takma ad olarak çalışır; Windows'ta `tdf` kullanın. `tdf monitor` çalıştırmadan önce kartın seri portundaki diğer terminalleri kapatın — port aynı anda yalnızca bir program tarafından açılabilir.)

**Gerçek donanımda doğrulandı:** `tdf build` macOS ve Windows 11'de hatasız derler ve bağlar (`.text` yaklaşık 14.9 KB; tam boyut araç zinciri sürümüne göre biraz değişir ve bağlayıcı, [01-Hello_Blink'in Çözüm bölümünde](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) açıklanan aynı beklenen, zararsız `_close` / `_lseek` / `_read` / `_write is not implemented` uyarılarını yazdırır). `tdf flash`, bağlı gerçek bir Tiremo Cortex kartında SWD üzerinden tam bir programlama ve doğrulama döngüsünü tamamladı (`** Verified OK **`) ve UART logu şunu gösterdi (kartın MAC adresi aşağıda değiştirilmiştir):

```text
[INFO] Device_Identity_From_MAC: boot
[INFO] ESP32: power cycling module...
[INFO] ESP32: power-on boot delay elapsed
[INFO] TIREMO_ESP32_Init: UART2 ready
[INFO] ESP32: command echo disabled (ATE0)
[INFO] identity: station MAC aa:bb:cc:dd:ee:ff
[INFO] identity: deviceId aabbccddeeff
[INFO] identity: still aabbccddeeff
[INFO] identity: still aabbccddeeff
[INFO] identity: still aabbccddeeff
```

Heartbeat her 5 sn'de `CIPSTAMAC?` sorgusunu tekrarlar ve aynı 12 hex haneyi doğrular. Bir okuma farklı çıkarsa log bir hata olarak `identity: MAC changed <eski> -> <yeni>` gösterir. İkinci bir Tiremo Cortex, aynı `GetHwEui()` / `FromEui()` koduyla okunduğunda farklı bir `deviceId` verdi.

## Adım Adım İnceleme

1. [`tiremo_esp32.c`](../../../../Tiremo_Cloud/Feature_Tutorials/01-Device_Identity_From_MAC/Sw/Drivers/esp32/Src/tiremo_esp32.c) — `TIREMO_ESP32_GetHwEui()` (`AT+CIPSTAMAC?`, `"OK"` ile eşleş, `"aa:bb:cc:dd:ee:ff"`'yi ayrıştır).
2. [`tiremo_identity.c`](../../../../Tiremo_Cloud/Feature_Tutorials/01-Device_Identity_From_MAC/Sw/Common/Src/tiremo_identity.c) — `FromEui` (12 hex) ve `FormatMac` (iki noktalı, yalnızca log için).
3. [`prv_user_code.c`](../../../../Tiremo_Cloud/Feature_Tutorials/01-Device_Identity_From_MAC/Sw/Src/prv_user_code.c) — açılış logu ve kimliğin değişmediğini 5 sn'de bir yeniden okuyarak doğrulama.

`GetHwEui()` içinden:

```c
if (!TIREMO_ESP32_SendCommandCapture("AT+CIPSTAMAC?\r\n", "OK",
                                     TIREMO_ESP32_DEFAULT_CMD_TIMEOUT_MS,
                                     response, sizeof(response)))
{
    return false;
}

return prvParseMacQuoted(response, pEui);
```

## Tamamlanma Ölçütü

- Bu kartın her açılışında aynı 12 hex haneli `deviceId` yazdırılır ve her `identity: still` satırı onu tekrarlar.
- Aynı imajı çalıştıran ikinci bir kart farklı bir `deviceId` yazdırır.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Eğitimleri — Empa Electronics</sub></p>
