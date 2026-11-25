<p class="sphinxhide" align="right"><a href="../../../../Sensors_And_Storage/Feature_Tutorials/06-Flash_Backed_Config/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>Flash-Backed Config</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Flash-Backed Config

***Kart: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Bölüm: Sensors & Local Storage / Feature Tutorials · Zorluk: Orta***

> Bu sayfa [İngilizce orijinalinin](../../../../Sensors_And_Storage/Feature_Tutorials/06-Flash_Backed_Config/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

WiFi kimlik bilgileri ve sensör bayraklarının bir güç döngüsünden sağ çıkması gerekir. Bu eğitim, sonraki WiFi / fabrika-sıfırlama eğitimlerinin dayandığı genel kalıcılık mekanizmasını — çip üzerindeki data flash'ta magic + CRC32 — inşa eder.

## Neler Öğreneceksiniz

- `Tiremo_Hal_Flash`: çip üzerindeki data flash'ın (DFMC) 2 KB'lık bir sektörünü silmek / yazmak / okumak. Bu SDK'da `CONFIG_HAL_FMC` zaten `1`
- Üretimdeki `TIREMO_APP` yapılandırmasıyla aynı fikirde, `bootCount` + kısa bir `label`'a indirgenmiş bir `"TRM1"` magic + sürüm + CRC32 (polinom `0xEDB88320`) kaydı
- Kaydın neden yeniden flaşlamadan sağ çıktığı ve bir üretici yazılımının onu neden yalnızca bir şey değiştiğinde yazması gerektiği

## Ön Koşullar

- Kavramsal olarak [05-Microphone_RMS_To_dB](../05-Microphone_RMS_To_dB/README.md); bu `Sw/` yalnızca flash kullanır (sensör yok)
- Log'u izlemek için: `tdf monitor` (kurulum için [Getting_Started/04-Button_Read_And_Debounce'un Çözüm bölümüne](../../../Getting_Started/04-Button_Read_And_Debounce/README.md#çözüm) bakın) ya da kartın USB seri portunda 115200 8N1'de herhangi bir seri terminal (Tera Term, PuTTY, screen, minicom, …)
- Bilgisayarınıza USB kablosuyla bağlı bir Tiremo Cortex kartı (ABOV A34G43A MCU — bkz. [Donanım Genel Bakışı](../../../Hardware.md)). Data flash çip üzerindedir; ek donanım gerekmez. Kartın üzerindeki CMSIS-DAP hata ayıklayıcısı SWD üzerinden flaşlamayı yapar.

## Veri nerede duruyor

A34G43A'nın iki ayrı flash alanı vardır:

| Alan | Adres | Kullanım |
|---|---|---|
| Code flash | `0x00000000` | `tdf flash`'ın programladığı üretici yazılımı imajı |
| Data flash (DFMC) | `0x0E000000` | Bu eğitimin kaydı, ilk 2 KB'lık sektörün `0` ofsetinde |

Ayrı oldukları için `tdf flash` kayda dokunmadan üretici yazılımını değiştirir, `tdf monitor`'ün yaptığı SWD reseti de ona dokunmaz. `tdf clean-device` yalnızca **code** flash aralığını (`0x0`–`0x200000`) toptan siler; bu aralık `0x0E000000`'daki data flash'ı içermez — bu yüzden kayıt bundan da sağ çıkar. Gerçek bir kartta doğrulandı: `tdf clean-device`'tan sonra code flash tamamen `0xFF` okundu, `0x0E000000`'daki kayıt ise bozulmadan duruyordu; `tdf flash`'tan sonra üretici yazılımı kaydı yükleyip saymaya devam etti. Kaydı silmek için sektörünü üretici yazılımından silin (`TIREMO_HAL_FLASH_EraseSector(0)`).

## Kaydı anlamak

[`Sw/Common/Src/tiremo_persist.c`](../../../../Sensors_And_Storage/Feature_Tutorials/06-Flash_Backed_Config/Sw/Common/Src/tiremo_persist.c), 32 baytlık tek bir `TIREMO_PersistRecord_t` saklar:

| Alan | Boyut | Değer |
|---|---|---|
| `magic` | 4 bayt | `0x54524D31` (`"TRM1"`) |
| `version` | 2 bayt | `1` |
| `reserved` | 2 bayt | `0` |
| `bootCount` | 4 bayt | her açılışta bir artırılır |
| `label` | 16 bayt | `"tutorial"` (NUL ile biten) |
| `crc32` | 4 bayt | bu alandan **önceki** her bayt üzerinden CRC32 |

`Load()` kaydı okur ve sırasıyla magic'i, sürümü, sonra CRC'yi kontrol eder. Boş flash, yanlış magic ya da sürüm veya hatalı CRC "geçerli kayıt yok" sayılır ve çağıran taraf varsayılanları uygular. `Save()` sektörü siler, ardından kaydı yeni bir CRC ile yazar.

## Program ne yapıyor

[`Sw/Src/prv_user_code.c`](../../../../Sensors_And_Storage/Feature_Tutorials/06-Flash_Backed_Config/Sw/Src/prv_user_code.c) bu adımları **açılışta bir kez** çalıştırır, sonra boşta bekler:

1. Kaydı yükle — geçerli bir kayıt yoksa varsayılanlara dön.
2. `bootCount`'u artır, `label`'ı `"tutorial"` yap ve kaydet.
3. Kaydı RAM'de kopyala, `crc32`'sinin bir bitini çevir ve `IsValid()`'in onu reddettiğini kontrol et — gerçek sektöre dokunmayan bir "bozuk kayıt" testi.

Yani kayıt sürekli değil, her resette bir kez yeniden yazılır. Flash sektörleri yalnızca sınırlı sayıda silme/yazma döngüsüne dayanır; bu yüzden gerçek bir ürün `Save()`'i bu demodaki gibi her açılışta değil, yalnızca bir değer gerçekten değiştiğinde çağırmalıdır.

## Çözüm

Depo kökünden:

```sh
cd Sensors_And_Storage/Feature_Tutorials/06-Flash_Backed_Config/Sw
tdf build
tdf flash
tdf monitor
```

(`tdf`'nin ne olduğu için [Getting_Started/01-Hello_Blink'in Çözüm bölümüne](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) bakın. macOS/Linux'ta `tdf.py` takma ad olarak da çalışır; Windows'ta `tdf` kullanın. `tdf monitor`'ü çalıştırmadan önce kartın seri portundaki diğer terminalleri kapatın — portu aynı anda yalnızca bir program açabilir.)

**Gerçek donanımda doğrulandı:** `tdf build`, macOS'ta ve Windows 11'de hatasız derlenip bağlanıyor (`.text` yaklaşık 16 KB; kesin boyut toolchain sürümüne göre biraz değişir, ve linker [01-Hello_Blink'in Çözüm bölümünde](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) açıklanan beklenen ve zararsız `_close` / `_lseek` / `_read` / `_write is not implemented` uyarılarını basar). `tdf flash`, gerçek, bağlı bir Tiremo Cortex kartına karşı SWD üzerinden tam bir programlama-ve-doğrulama döngüsünü tamamladı (`** Verified OK **`). Bir resetten sonraki log:

```text
[INFO] Flash_Backed_Config: boot
[INFO] persist: loaded bootCount=1 label=tutorial
[INFO] persist: saved bootCount=2 (survives reset)
[INFO] persist: corrupt CRC rejected (expected)
```

RESET'e basın (ya da kartı resetleyen `tdf monitor`'ü çalıştırın); `bootCount` her seferinde bir artar — kalıcılığın kanıtı budur. Data flash boşken yapılan ilk açılışta ikinci satır `persist: no valid record, using defaults` olur ve sayaç `1`'den başlar. Flash sürücüsü kurulamazsa log `[ERROR] persist: flash init failed` gösterir ve program orada durur; başarısız bir yazma `[WARN] persist: save failed` loglar.

## Adım Adım İnceleme

### 1. `Tiremo_Hal_Flash`

Referans HAL'den kopyalanan [`Sw/Tiremo_Hal/Src/tiremo_hal_flash.c`](../../../../Sensors_And_Storage/Feature_Tutorials/06-Flash_Backed_Config/Sw/Tiremo_Hal/Src/tiremo_hal_flash.c), `FMC_ID_DFMC` üzerindeki satıcı `HAL_FMC_*` çağrılarını sarmalar: 2 KB hizalı bir sektörü silme, yazma, okuma. Ofsetler data flash'ın başlangıcına göredir.

### 2. `tiremo_persist.c`

[`Sw/Common/Src/tiremo_persist.c`](../../../../Sensors_And_Storage/Feature_Tutorials/06-Flash_Backed_Config/Sw/Common/Src/tiremo_persist.c), eğitim boyutunda kayıt mantığıdır: `Init`, `Defaults`, `Load`, `Save`, `IsValid` ve bit bit CRC32 (polinom `0xEDB88320`, başlangıç/son XOR `0xFFFFFFFF`).

### 3. `prv_user_code.c`

[`Sw/Src/prv_user_code.c`](../../../../Sensors_And_Storage/Feature_Tutorials/06-Flash_Backed_Config/Sw/Src/prv_user_code.c) — açılışta bir kez yükle / artır / kaydet / bozuk-kayıt demosu (bkz. yukarıdaki **Program ne yapıyor**).

### 4. Derle, flaşla ve izle

```sh
tdf build
tdf flash
tdf monitor
```

(Ya da tek komutta zincirleyerek: `tdf build flash monitor`.) Çıktının nasıl görünmesi gerektiği için yukarıdaki **Çözüm** bölümüne bakın.

## Tamamlanma Ölçütü

Her resetten sonra log `persist: loaded bootCount=N` ve ardından `saved bootCount=N+1` gösterir, `bootCount` resetler, `tdf flash` ve `tdf clean-device` boyunca artmaya devam eder, ve her açılış `persist: corrupt CRC rejected (expected)` ile biter.


<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
