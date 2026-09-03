<p class="sphinxhide" align="right"><a href="../../../../Sensors_And_Storage/Feature_Tutorials/01-I2C_Bus_Bringup/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>I2C Bus Bring-up</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# I2C Bus Bring-up

***Kart: Tiremo Cortex (ESP32) · Bölüm: Sensors & Local Storage / Feature Tutorials · Zorluk: Kolay***

> Bu sayfa [İngilizce orijinalinin](../../../../Sensors_And_Storage/Feature_Tutorials/01-I2C_Bus_Bringup/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

SHT40 ve LIS2DE12, I2C2'yi paylaşır. Her iki sensör sürücüsünü yazmadan
önce, bus'ın kendisini çalışır ve kanıtlanabilir hale getirin — bilinen
bir adresi probelamak, bir kablolama ya da pull-up sorununu erkenden
yakalamanın en hızlı yolu. Bu, `Getting_Started` dışındaki bu depodaki
ilk eğitim, bu yüzden de o bölümün LED/buton demosu üzerine inşa etmek
yerine taze, minimal bir `Sw/` ile başlıyor.

## Neler Öğreneceksiniz

- `Tiremo_Hal_I2C` sarmalayıcısı: `Transmit`, `Receive`, `WriteRead`, ve neden çağrılacak bir `Probe` fonksiyonu olmadığı
- Bir baytlık `Receive()`'dan minimal bir bus probu inşa etmek — yazmak değil okumak, böylece işlem hiçbir zaman bir sensörün dahili durumunu bozamaz
- Bir datasheet'in 7-bit'e karşı 8-bit adres kuralını okumak (`board_config.h`'deki LIS2DE12 SA0 notuna bakın)
- Bu eğitimin `main.c`'inin neden yine değiştiği: `PRV_I2C_Init()` satıcı tarafından üretilmiştir, tam olarak `I2C_ID_2`'ye kapsamlandırılmıştır — bu eğitimin ihtiyaç duyduğu bus

## Ön Koşullar

- [Getting_Started](../../../../Getting_Started/README.md) tamamlanmış — bu eğitim onun SysTick'ini ve debug-UART loglamasını değişmeden yeniden kullanır, ve `tdf.py` ile kurulum betiklerinin (`install.sh`/`export.sh`, ya da `install.ps1`/`export.ps1`, ya da `install.bat`/`export.bat`) zaten çalıştırılmış olduğunu varsayar
- Probe sonuçlarını izlemek için `tdf.py monitor` (kurulum için [Getting_Started/04-Button_Read_And_Debounce'un Çözüm bölümüne](../../../../Getting_Started/04-Button_Read_And_Debounce/README.md#solution) bakın)
- SHT40 ve LIS2DE12'nin I2C2 üzerinde bulunduğu, SWD üzerinden bağlı bir ABOV A34G43A Starter Kit

## Probe'u Anlamak (Çağrılacak Bir `Probe()` Yok)

[`Sw/Tiremo_Hal/Include/tiremo_hal_i2c.h`](../../../../Sensors_And_Storage/Feature_Tutorials/01-I2C_Bus_Bringup/Sw/Tiremo_Hal/Include/tiremo_hal_i2c.h)
(referans HAL'den birebir kopyalandı) tam olarak dört fonksiyona sahip:
`Transmit`, `Receive`, `WriteRead`, ve iki kurtarma yardımcısı
(`PrepareBus`/`Recover`, bir slave'in sıkışık bıraktığı bir bus için).
`Probe()` yok. Bu, HAL katmanının geri kalanıyla tutarlı: satıcı I2C
çevre biriminin üzerinde ince, protokolden bağımsız bir sarmalayıcı —
"bir şey dinliyor mu" kavramı yok, yalnızca "bayt gönder/al, çalışıp
çalışmadığını söyle" var.

Klasik bir I2C bus tarayıcısı (`i2cdetect` gibi), "bu adreste bir şey
var mı" sorusunu mümkün olan en ucuz işlemle yanıtlar: cihazı adresle,
ACK edip etmediğine bak, ve dur — hiçbir register bilgisi gerekmez. Bu
HAL'in `Transmit()`/`Receive()`'i ikisi de `length >= 1` gerektiriyor
(gerçek bir sıfır-bayt probu mevcut değil), bu yüzden mevcut en küçük
güvenli işlem **bir baytlık bir okuma**:

```c
uint8_t dummy;
bool acked = (TIREMO_HAL_I2C_Receive(bus, address, &dummy, 1U, true)
              == TIREMO_HAL_I2C_STATUS_OK);
```

Bilinçli olarak bir okuma, bir yazma değil. Bir baytlık bir `Receive()`
yalnızca cihazın dahili register işaretçisinin şu anda tuttuğu şeyi
içeri çeker ve atar (`dummy` hiç kullanılmaz) — cihaz durumunu
bozamaz, bu eğitimin henüz hiçbir şey bilmediği bir çipte gerçek,
yazılabilir bir register'a inebilecek rastgele bir baytın bir baytlık
`Transmit()`'inin aksine. `TIREMO_HAL_I2C_STATUS_OK`, adresin ACK
ettiği anlamına gelir; `_ERROR` hem bir NACK'i hem de gerçek bir bus
hatasını kapsar, ki bu tam olarak bir *bus* bring-up eğitimi için doğru
detay seviyesi — bu ikisini ayırt etmek, burada değil bunu takip eden
SHT40 ve LIS2DE12 eğitimlerine ait cihaz-başına protokol bilgisi
gerektirirdi.

## Çözüm

[`Sw/`](../../../../Sensors_And_Storage/Feature_Tutorials/01-I2C_Bus_Bringup/Sw/) gerçek, derlenebilir bir uygulamaya sahip:

```sh
cd Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

(`tdf.py`'nin ne olduğu için [Getting_Started/01-Hello_Blink'in Çözüm bölümüne](../../../../Getting_Started/01-Hello_Blink/README.md#solution),
özellikle `monitor` için [04-Button_Read_And_Debounce'un Çözüm bölümüne](../../../../Getting_Started/04-Button_Read_And_Debounce/README.md#solution)
bakın.)

Her `Getting_Started` eğitimiyle aynı paylaşılan satıcı SDK'sı ve aynı
CMake derleme şekli, sadece bir dizin daha derin —
[`CMakeLists.txt`](../../../../Sensors_And_Storage/Feature_Tutorials/01-I2C_Bus_Bringup/Sw/CMakeLists.txt)'deki
`GEN_DIR`, `Sensors_And_Storage/Feature_Tutorials/01-I2C_Bus_Bringup/Sw/`'den
depo köküne ulaşmak için `../../../../Generation/...` kullanıyor (üç değil,
dört seviye). Bu eğitimin ihtiyaç duyduğu yeni satıcı kaynakları:
`user_i2c.c`/`user_i2c_isr.c` (satıcı tarafından üretilen
`PRV_I2C_Init()`, `I2C_ID_2`'yi 100 kHz master modda devreye alır) ve
`hal_i2c.c` — tahmin ederek değil, `tiremo_hal_i2c.c`'nin kendi
include'larını ve satıcı devreye alma yorumlarını okuyarak bulundu.

**Bağlı kartta, gerçekten, gerçek hayatta doğrulandı:** `tdf.py build`
temiz şekilde derlenip bağlanıyor (`.text` 14532 bayt). `tdf.py flash`,
SWD üzerinden tam bir programlama-ve-doğrulama döngüsünü tamamladı
(`** Verified OK **`). Ardından `tdf.py monitor`, doğrudan gerçek
donanımdan şunu gösterdi:

```text
[INFO] SHT40 (0x44): ACK
[INFO] LIS2DE12 (0x19): ACK
```

her 5 saniyede bir tekrarlanıyor. **Her iki sensör de I2C2'de gerçekten
ACK ediyor** — bu eğitimin tüm "Ne Zaman Tamamlanmış Sayılır" ölçütü,
gerçek donanımda doğrulandı, simüle edilmedi ya da varsayılmadı. Bir
sensör eksik olsaydı ya da bus yanlış kablolanmış olsaydı, aynı log o
adres için `WARN` seviyesinde `NACK -- not responding` gösterirdi.

## Adım Adım

### 1. Kart yapılandırması — I2C2 ve iki adres, başka hiçbir şey yok

[`Sw/Board/board_config.h`](../../../../Sensors_And_Storage/Feature_Tutorials/01-I2C_Bus_Bringup/Sw/Board/board_config.h)
bilinçli olarak dar: yalnızca `TIREMO_BOARD_SHT40_I2C_BUS`/`_ADDRESS` ve
`TIREMO_BOARD_LIS2DE12_I2C_BUS`/`_ADDRESS`, ikisi de
`TIREMO_HAL_I2C_BUS_2`'yi işaret ediyor — aynı fiziksel bus, iki cihaz.
`Getting_Started`'den taşınan LED/buton makrosu yok; bu eğitim onları
kullanmıyor.

LIS2DE12 adresi özellikle belirtilmeye değer: ST'nin kendi datasheet'i
*8-bit* yazma adresini verir (`SA0` = 1 ile `0x33`), ama bu HAL'in
`address` parametresi her zaman 7-bit — `0x33 >> 1 = 0x19`, ki
`board_config.h`'nin gerçekte tuttuğu şey bu. Bu kaydırmayı yanlış
yapmak, yanlış adresi probelamak ve sahte bir NACK görmek için klasik,
sessiz bir yol.

### 2. Giriş noktası — `PRV_I2C_Init()`

[`Sw/Src/main.c`](../../../../Sensors_And_Storage/Feature_Tutorials/01-I2C_Bus_Bringup/Sw/Src/main.c),
`PRV_UART_Init()`'den sonra bir satıcı çağrısı ekler:

```c
PRV_PORT_Init();
PRV_SCU_CLK_Init();
PRV_UART_Init();
PRV_I2C_Init();     /* yeni: I2C_ID_2'yi 100 kHz'de, master modda devreye alır */
PRV_USER_Code();
```

`PRV_I2C_Init()` satıcı tarafından üretilmiştir, değiştirilmemiştir, ve
zaten bu eğitimin ihtiyaç duyduğu tek bus'a kapsamlandırılmıştır —
I2C0/1/3'e dokunmaz. SCL/SDA pin muxlama, üretilen pin yapılandırması
tarafından ele alınır, Tutorial 03'te UART'ın pinlerinin olduğu gibi:
onun için hiç çalışma zamanı GPIO çağrısı gerekmez.

### 3. Her iki sensörü probelamak

[`Sw/Src/prv_user_code.c`](../../../../Sensors_And_Storage/Feature_Tutorials/01-I2C_Bus_Bringup/Sw/Src/prv_user_code.c):

```c
static void ProbeAndLog(const char *pName, TIREMO_HAL_I2C_Bus_t bus, uint8_t address)
{
    uint8_t dummy;
    bool acked = (TIREMO_HAL_I2C_Receive(bus, address, &dummy, 1U, true)
                  == TIREMO_HAL_I2C_STATUS_OK);

    if (acked) { TIREMO_LOG_INFO("%s (0x%02X): ACK", pName, address); }
    else       { TIREMO_LOG_WARN("%s (0x%02X): NACK -- not responding", pName, address); }
}
```

önyüklemede hemen bir kez, ardından ana döngüden `02-SysTick_Timing`'den
beri her eğitimin kullandığı aynı bloklamayan `(now - last) >= interval`
deyimini kullanarak her `PROBE_INTERVAL_MS`'de (5000 ms) çağrılır.
Probe'u — bir kez loglayıp susmak yerine — tekrarlamak, `tdf.py
monitor`'ın reset-sonra-bağlan penceresi ilk önyükleme satırını kaçırsa
bile sonucun hâlâ görülebilir olması anlamına gelir (gerçek, belgelenmiş
bir uç durum,
[04-Button_Read_And_Debounce](../../../../Getting_Started/04-Button_Read_And_Debounce/README.md#solution)'den
beri).

### 4. Derle, flaşla, izle

```sh
tdf.py build
tdf.py flash
tdf.py monitor
```

Önceki her eğitimle aynı işletim sistemi tespiti, aynı paketlenmiş
OpenOCD akışı — bkz. [01-Hello_Blink'in Çözüm bölümü](../../../../Getting_Started/01-Hello_Blink/README.md#solution).
Bu eğitim için gerçek donanımda tam olarak neyin doğrulandığı için
yukarıdaki **Çözüm** bölümüne bakın.

## Ne Zaman Tamamlanmış Sayılır

`tdf.py monitor`, her 5 sn'de bir tekrarlanan `SHT40 (0x44): ACK` ve
`LIS2DE12 (0x19): ACK` gösterir — gerçek donanımda gerçekten
doğrulandı, bkz. **Çözüm**.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
