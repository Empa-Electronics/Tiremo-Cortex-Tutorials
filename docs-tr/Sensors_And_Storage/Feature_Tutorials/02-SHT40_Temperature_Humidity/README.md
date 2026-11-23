<p class="sphinxhide" align="right"><a href="../../../../Sensors_And_Storage/Feature_Tutorials/02-SHT40_Temperature_Humidity/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>SHT40 Temperature / Humidity</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# SHT40 Temperature / Humidity

***Kart: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Bölüm: Sensors & Local Storage / Feature Tutorials · Zorluk: Kolay***

> Bu sayfa [İngilizce orijinalinin](../../../../Sensors_And_Storage/Feature_Tutorials/02-SHT40_Temperature_Humidity/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

SHT40, yığındaki ilk gerçek sensör sürücüsüdür ve en basit olanıdır —
tek bir I2C işlem dizisi hem sıcaklığı hem nemi döndürür. Daha sonra
gölge şemasının (Tiremo Cloud) `unit` ve `offset` ile ayarlayacağı
sensör de budur. Eğitim 01, çipin I2C2 üzerinde `0x44`'te ACK ettiğini
zaten kanıtladı; bu eğitim o adresteki asıl SHT4x protokolünü öğretir.

## Neler Öğreneceksiniz

- Sensirion SHT4x ölçüm komut/yanıt protokolü: `0xFD` komutu, bekleme, CRC'li altı bayt
- Bu çipin neden `WriteRead()` değil, STOP-sonra-okuma kullandığı (o LIS2DE12 eğitimi)
- Ham tick'leri milli-°C / milli-%RH'ye kayan nokta ve 64-bit çarpma olmadan, tamsayı matematiğiyle dönüştürmek
- Sürücünün neden `Drivers/sht40`'ta yaşadığı ve 1 Hz döngünün neden `prv_user_code.c`'de kaldığı

## Ön Koşullar

- [01-I2C_Bus_Bringup](../../../../Sensors_And_Storage/Feature_Tutorials/01-I2C_Bus_Bringup/README.md) — bu eğitim o `Sw/`'nin SysTick'ini, debug UART'ını, I2C HAL'ini ve `PRV_I2C_Init()`'ini değişmeden yeniden kullanır
- Okumaları izlemek için: `tdf monitor` (kurulum için [Getting_Started/04-Button_Read_And_Debounce'un Çözüm bölümüne](../../../Getting_Started/04-Button_Read_And_Debounce/README.md#çözüm) bakın) ya da kartın USB seri portunda 115200 8N1'de herhangi bir seri terminal (Tera Term, PuTTY, screen, minicom, …)
- Bilgisayarınıza USB kablosuyla bağlı, SHT40'ın I2C2 üzerinde bulunduğu bir Tiremo Cortex kartı (ABOV A34G43A MCU — bkz. [Donanım Genel Bakışı](../../../Hardware.md)). Kartın üzerindeki CMSIS-DAP hata ayıklayıcısı SWD üzerinden flaşlamayı yapar; ayrı bir programlayıcı gerekmez.

## SHT4x Protokolünü Anlamak

Üretim üretici yazılımı Sensirion'un kendi C kütüphanesini
(`sht4x_measure_high_precision()`) sarmalar. Bu eğitim **yapmaz** —
öğrenme hedefi komut/yanıtın kendisidir, bu yüzden
[`Sw/Drivers/sht40/Src/tiremo_sht40.c`](../../../../Sensors_And_Storage/Feature_Tutorials/02-SHT40_Temperature_Humidity/Sw/Drivers/sht40/Src/tiremo_sht40.c)
onu doğrudan `Tiremo_Hal_I2C` üzerinde uygular. Datasheet'ten üç gerçek,
tüm sürücüdür:

1. **Komut `0xFD`** — "T ve RH ölç, yüksek hassasiyet." Bir bayt,
   `TIREMO_HAL_I2C_Transmit()` ile yazılır, sonra bir STOP. Dönüşüm,
   bus boştayken gerçekleşir.
2. **Bekle, sonra altı bayt oku.** Tipik dönüşüm 6.9 ms, en fazla
   8.3 ms; bu sürücü 10 ms bekler (Sensirion'un kendi sürücüsünün
   kullandığı aynı marj). Çok erken verilen bir okuma NACK eder. Yanıt:
   `[T_MSB, T_LSB, T_CRC, RH_MSB, RH_LSB, RH_CRC]`.
3. **CRC-8, polinom `0x31`, init `0xFF`** — sıcaklık baytlarının
   üzerinde bir CRC, nem baytlarının üzerinde bir CRC. Uyuşmazlık,
   okumanın "yeterince yakın" değil, çöp olduğu anlamına gelir.

O STOP-sonra-bekle-sonra-oku, `TIREMO_HAL_I2C_WriteRead()` değildir.
`WriteRead()`, yazma ile okuma arasına tekrarlanmış bir START koyar;
LIS2DE12 bunu ister çünkü yazmayı bir register işaretçisi olarak
ele alır. SHT4x yazmayı "dönüşümü başlat" olarak ele alır; tekrarlanmış
bir START onu kesintiye uğratır. İki çip, aynı bus, iki işlem şekli —
HAL'in her iki API'ye sahip olmasının nedeni budur.

Dönüşüm, datasheet'ten:

```text
T  (°C)  = -45 + 175 * ticks / 65535
RH (%RH) =  -6 + 125 * ticks / 65535
```

`175000 * 65535` 32 bite sığmaz ve bu üretici yazılımın gerçekten
derlendiği bayraklarda donanım FPU yoktur (`nofp` newlib nano).
Sensirion'un tamsayı formu, bu sürücünün kullandığıdır:

```c
temp_mC  = ((21875 * (int32_t)ticks) >> 13) - 45000;  /* milli-degC */
rh_mRh   = ((15625 * (int32_t)ticks) >> 13) -  6000;  /* milli-%RH  */
```

(`175000/65535 ≈ 21875/2^13`, nem için de aynı.) Bağıl nem ardından
`[0, 100000]` milli-%RH'ye sıkıştırılır — formül kenarlarda 0–100'ün
hafif dışına düşebilir, datasheet'in kendisi de bunu uyarır.

Milli birimler, kayan nokta değil, sonraki eğitimlerin JSON'a ve cihaz
gölgesine koyacağı şeydir. Loglama, uygulama kodunda `milli / 1000` ve
`milli % 1000` olarak bölerek `33.282 C` basar — sürücü hiçbir zaman
bir string biçimlemez.

`TIREMO_SHT40_Read()` o ~10 ms için bloklar. 1 Hz'de bu zararsızdır;
sıkı bir iç döngü "ölçümü başlat"ı "sonucu topla"dan ayırırdı. Bu
eğitim, protokolün baştan sona okuyabileceğiniz tek bir fonksiyon
olarak kalması için tek atımlık API'yi tutar.

## Çözüm

[`Sw/`](../../../../Sensors_And_Storage/Feature_Tutorials/02-SHT40_Temperature_Humidity/Sw/) gerçek, derlenebilir bir uygulamaya sahip:

```sh
cd Sensors_And_Storage/Feature_Tutorials/02-SHT40_Temperature_Humidity/Sw
tdf build
tdf flash
tdf monitor
```

(Depo kökünden çalıştırın. `tdf`'nin ne olduğu için [Getting_Started/01-Hello_Blink'in Çözüm bölümüne](../../../Getting_Started/01-Hello_Blink/README.md#çözüm),
özellikle `monitor` için [04-Button_Read_And_Debounce'un Çözüm bölümüne](../../../Getting_Started/04-Button_Read_And_Debounce/README.md#çözüm)
bakın. macOS/Linux'ta `tdf.py` takma ad olarak da çalışır; Windows'ta
`tdf` kullanın. `tdf monitor`'ü çalıştırmadan önce kartın seri portundaki
diğer terminalleri kapatın — portu aynı anda yalnızca bir program
açabilir.)

[01-I2C_Bus_Bringup](../../../../Sensors_And_Storage/Feature_Tutorials/01-I2C_Bus_Bringup/README.md)
ile aynı paylaşılan satıcı SDK'sı ve aynı CMake şekli — `GEN_DIR` hâlâ
`../../../../Generation/...` kullanır. Yeni satıcı kaynağı yok: I2C2
zaten ayakta. Yeni olan `Drivers/sht40`, bu deponun ilk I2C cihaz
sürücüsü (`Getting_Started` 04 ve 05'teki buton ve LED sürücüleri GPIO idi).

**Gerçek donanımda doğrulandı:** `tdf build`, macOS'ta ve Windows 11'de
hatasız derlenip bağlanıyor (`.text` yaklaşık 15,5 KB; kesin boyut
toolchain sürümüne göre biraz değişir, ve linker
[01-Hello_Blink'in Çözüm bölümünde](../../../Getting_Started/01-Hello_Blink/README.md#çözüm)
açıklanan beklenen ve zararsız `_close` / `_lseek` / `_read` / `_write is
not implemented` uyarılarını basar). `tdf flash`, gerçek, bağlı bir
Tiremo Cortex kartına karşı SWD üzerinden tam bir programlama-ve-doğrulama
döngüsünü tamamladı (`** Verified OK **`) ve log şöyle görünür:

```text
[INFO] SHT40_Temperature_Humidity: boot
[INFO] SHT40 T: 27.410 C
[INFO] SHT40 RH: 34.559 %RH
[INFO] SHT40 T: 27.375 C
[INFO] SHT40 RH: 34.393 %RH
[INFO] SHT40 T: 27.367 C
[INFO] SHT40 RH: 34.281 %RH
```

her saniye yeni bir T/RH çifti gelir. Ardışık örnekler arasında değerler
yalnızca son bir-iki basamakta oynar — bu normal sensör gürültüsüdür.
Kesin değerler odaya ve kartın kendi ısınmasına bağlıdır. Sensörün
tepki verdiğini görmek için üzerine hafifçe nefes verin: sıcaklık ve nem
birkaç okuma içinde yükselir, sonra geri oturur. Bir okuma başarısız
olursa (NACK ya da CRC uyuşmazlığı), log bunun yerine sarı bir
`[WARN] SHT40: read failed (NACK or CRC)` satırı gösterir.

## Adım Adım

### 1. Kart yapılandırması — I2C2 ve bir adres

[`Sw/Board/board_config.h`](../../../../Sensors_And_Storage/Feature_Tutorials/02-SHT40_Temperature_Humidity/Sw/Board/board_config.h)
yalnızca `TIREMO_BOARD_SHT40_I2C_BUS` / `_ADDRESS` tutar
(`TIREMO_HAL_I2C_BUS_2`, `0x44`). LIS2DE12 aynı kablolarda ama kapsam
dışında; bir sonraki eğitimde geri gelir.

### 2. Giriş noktası — Eğitim 01 ile aynı `PRV_I2C_Init()`

[`Sw/Src/main.c`](../../../../Sensors_And_Storage/Feature_Tutorials/02-SHT40_Temperature_Humidity/Sw/Src/main.c)
yeni bir satıcı çağrısı eklemez. SHT40 hâlâ `I2C_ID_2`'de yaşar:

```c
PRV_PORT_Init();
PRV_SCU_CLK_Init();
PRV_UART_Init();
PRV_I2C_Init();     /* zaten I2C_ID_2'ye kapsamlandırılmış, 100 kHz master */
PRV_USER_Code();
```

### 3. Sürücü — komut, bekle, CRC, dönüştür

[`Sw/Drivers/sht40/Src/tiremo_sht40.c`](../../../../Sensors_And_Storage/Feature_Tutorials/02-SHT40_Temperature_Humidity/Sw/Drivers/sht40/Src/tiremo_sht40.c):

```c
const uint8_t command = 0xFDU;

TIREMO_HAL_I2C_Transmit(bus, address, &command, 1U, true);
TIREMO_SysTick_DelayMs(10U);
TIREMO_HAL_I2C_Receive(bus, address, response, 6U, true);
/* her 2-baytlık sözcüğü CRC'le; sonra milli-birim dönüşümü */
```

`TIREMO_SHT40_Init()` yalnızca `TIREMO_HAL_I2C_PrepareBus()` çağırır —
HAL'in zaten sahip olduğu sıkışık-bus kurtarması, yeni bir protokol değil.

### 4. Döngü — 1 Hz, milli-birimleri ondalık olarak logla

[`Sw/Src/prv_user_code.c`](../../../../Sensors_And_Storage/Feature_Tutorials/02-SHT40_Temperature_Humidity/Sw/Src/prv_user_code.c)
açılışta bir kez örnekler, sonra her `READ_INTERVAL_MS` (1000 ms)
`02-SysTick_Timing`'den beri her eğitimin kullandığı aynı
`(now - last) >= interval` deyimiyle. Sürekli okumak — bir kez loglayıp
susmak yerine — seri terminali herhangi bir anda açıp güncel değerleri
görebilmeniz, ve sensörün değişikliklere (nefes, sıcak bir parmak)
tepkisini canlı izleyebilmeniz anlamına gelir.

### 5. Derle, flaşla ve izle

```sh
tdf build
tdf flash
tdf monitor
```

(Ya da tek komutta zincirleyerek: `tdf build flash monitor`.) Çıktının
nasıl görünmesi gerektiği için yukarıdaki **Çözüm** bölümüne bakın.

## Bittiğinde

`tdf monitor` saniyede bir sıcaklık ve nem çifti gösterir ve hiç
`read failed` satırı yoktur; ardışık değerler sensör gürültüsü içinde
kararlıdır ve sensöre nefes verdiğinizde ikisi de yükselir.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
