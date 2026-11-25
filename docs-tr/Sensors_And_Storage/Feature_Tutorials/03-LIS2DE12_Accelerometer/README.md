<p class="sphinxhide" align="right"><a href="../../../../Sensors_And_Storage/Feature_Tutorials/03-LIS2DE12_Accelerometer/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>LIS2DE12 Accelerometer</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# LIS2DE12 Accelerometer

***Kart: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Bölüm: Sensors & Local Storage / Feature Tutorials · Zorluk: Orta***

> Bu sayfa [İngilizce orijinalinin](../../../../Sensors_And_Storage/Feature_Tutorials/03-LIS2DE12_Accelerometer/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

SHT40'ın aksine LIS2DE12'nin gerçek bir yapılandırma yüzeyi vardır — tam ölçek ve çıkış veri hızı, sonraki düşme/şok/titreşim alarmları için önemlidir; bu yüzden sürücüyü ilk günden bunları parametre olarak alacak şekilde yazın. Eğitim 01, çipin `0x19`'da ACK ettiğini zaten kanıtladı; bu eğitim register haritasıyla `WriteRead()` üzerinden konuşur.

## Neler Öğreneceksiniz

- LIS2DE12 register haritası: `WHO_AM_I` (`0x0F` = `0x33`), `CTRL_REG1` (ODR), `CTRL_REG4` (ölçek), `STATUS_REG` (yeni veri bayrağı), `OUT_X/Y/Z_H`
- Bu çipin neden SHT40'ın STOP-sonra-okuma yöntemi değil, `TIREMO_HAL_I2C_WriteRead()` (tekrarlanmış START) gerektirdiği
- Sol hizalı 8-bit örnekleri, datasheet tam ölçek hassasiyetiyle (`15,6 / 31,2 / 62,5 / 187,5` mg/digit) yalnızca tamsayı matematiği kullanarak mg'ye dönüştürmek

## Ön Koşullar

- [02-SHT40_Temperature_Humidity](../02-SHT40_Temperature_Humidity/README.md) — aynı I2C HAL ve `PRV_I2C_Init()`
- Okumaları izlemek için: `tdf monitor` (kurulum için [Getting_Started/04-Button_Read_And_Debounce'un Çözüm bölümüne](../../../Getting_Started/04-Button_Read_And_Debounce/README.md#çözüm) bakın) ya da kartın USB seri portunda 115200 8N1'de herhangi bir seri terminal (Tera Term, PuTTY, screen, minicom, …)
- Bilgisayarınıza USB kablosuyla bağlı, LIS2DE12'nin I2C2 üzerinde bulunduğu bir Tiremo Cortex kartı (ABOV A34G43A MCU — bkz. [Donanım Genel Bakışı](../../../Hardware.md)). Kartın üzerindeki CMSIS-DAP hata ayıklayıcısı SWD üzerinden flaşlamayı yapar; ayrı bir programlayıcı gerekmez.

## WriteRead'i ve mg dönüşümünü anlamak

LIS2DE12, bir işlemin ilk baytını bir *register işaretçisi* olarak ele alır. O yazma ile veri okuma arasındaki bir STOP işaretçiyi sıfırlar — bu yüzden HAL'in `WriteRead()`'i bunun yerine tekrarlanmış bir START gönderir. Çok baytlı okumalar ayrıca alt adresin 7. bitini (`0x80`) ayarlayarak `OUT_X_H` … `OUT_Z_H` arasında otomatik artış yapar.

Bu ABOV çevre biriminde yazmalar, denetleyicinin çerçeveyi temiz bitirmesi için ardından bir baytlık boş bir okuma yapan aynı `WriteRead()`'i kullanır — bu, referans I2C adaptöründen kopyalanmış bu HAL'e özgü bir durumdur, bir datasheet kuralı değil.

8-bit örnekler 16-bit bir sözcükte sola hizalıdır; bu yüzden ham 16-bit değer 256'ya bölünür ve datasheet hassasiyetiyle çarpılır. Yalnızca tamsayıyla kalmak için sürücü hassasiyeti mg/digit'in onda biri cinsinden tutar ve `256 × 10 = 2560`'a böler:

| Tam ölçek | Hassasiyet (datasheet) | Saklanan değer |
|---|---|---|
| ±2 g | 15,6 mg/digit | `156` |
| ±4 g | 31,2 mg/digit | `312` |
| ±8 g | 62,5 mg/digit | `625` |
| ±16 g | 187,5 mg/digit | `1875` |

```c
mg = (raw16 * sens_tenth_mg) / 2560;   /* ±2g -> 15,6 mg/digit */
```

Her `ReadAccel()` çağrısı dört kez yeni bir örnek bekler (`STATUS_REG` `ZYXDA` biti, 200 ms zaman aşımı) ve ortalamayı döndürür; bu, 8-bit niceleme gürültüsünü yumuşatır. 100 Hz'de bu, çağrı başına yaklaşık 40 ms bloklar — bu eğitimin 500 ms'lik döngüsü için sorun değildir.

`Configure(scale_g, data_rate_hz)`, `CTRL_REG4` FS bitlerini ve `CTRL_REG1` ODR nibble'ı + düşük güç XYZ etkinleştirmesini (`0x0F`) programlar, sonra yazmayı doğrulamak için `CTRL_REG1`'i geri okur. Desteklenen hızlar arasındaki bir ODR bir sonrakine yukarı yuvarlanır (1, 10, 25, 50, 100, 200, 400 Hz).

Üretim üretici yazılımı ST'nin PID kütüphanesini sarmalar. Bu eğitim yapmaz — öğrenme hedefi bu birkaç register'dır.

## Çözüm

Depo kökünden:

```sh
cd Sensors_And_Storage/Feature_Tutorials/03-LIS2DE12_Accelerometer/Sw
tdf build
tdf flash
tdf monitor
```

(`tdf`'nin ne olduğu için [Getting_Started/01-Hello_Blink'in Çözüm bölümüne](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) bakın. macOS/Linux'ta `tdf.py` takma ad olarak da çalışır; Windows'ta `tdf` kullanın. `tdf monitor`'ü çalıştırmadan önce kartın seri portundaki diğer terminalleri kapatın — portu aynı anda yalnızca bir program açabilir.)

Eğitim 01/02'nin ötesinde yeni satıcı kaynağı yok. Yeni kod `Drivers/lis2de12`'dir.

**Gerçek donanımda doğrulandı:** `tdf build`, macOS'ta ve Windows 11'de hatasız derlenip bağlanıyor (`.text` yaklaşık 15,5 KB; kesin boyut toolchain sürümüne göre biraz değişir, ve linker [01-Hello_Blink'in Çözüm bölümünde](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) açıklanan beklenen ve zararsız `_close` / `_lseek` / `_read` / `_write is not implemented` uyarılarını basar). `tdf flash`, gerçek, bağlı bir Tiremo Cortex kartına karşı SWD üzerinden tam bir programlama-ve-doğrulama döngüsünü tamamladı (`** Verified OK **`). Açılış ve `WHO_AM_I` satırlarından sonra her 500 ms'de bir `X=… mg  Y=… mg  Z=… mg` satırı gelir. Gerçek bir karttan, önce bir kenarına, sonra başka bir kenarına yatırılıp ardından tekrar düz bırakıldığı bir kesit:

```text
[INFO] LIS2DE12_Accelerometer: boot
[INFO] LIS2DE12: WHO_AM_I=0x33  FS=2g  ODR=100 Hz
...
[INFO] LIS2DE12: X=206 mg  Y=861 mg  Z=507 mg
...
[INFO] LIS2DE12: X=975 mg  Y=-222 mg  Z=-159 mg
...
[INFO] LIS2DE12: X=113 mg  Y=-175 mg  Z=963 mg
[INFO] LIS2DE12: X=117 mg  Y=-171 mg  Z=967 mg
```

Kart düz dururken yerçekimi Z'de yaklaşık +1000 mg'dır ve X/Y küçük kalır (yüz mg civarı normaldir — kart nadiren tam yataydır; yukarıdaki son satırda toplam büyüklük √(117² + 171² + 967²) ≈ 989 mg, yani 1 g). Kartı sırayla her kenarına yatırın; ~1000 mg o eksene geçer (yatırdığınız yöne göre pozitif ya da negatif); iki eksenin arasındayken ikisine paylaşılır. Sensör açılışta yanıt vermezse log `[ERROR] LIS2DE12: init failed (WHO_AM_I or Configure)` gösterir ve program orada durur; sonraki başarısız bir okuma `[WARN] LIS2DE12: read failed` loglar.

## Adım Adım İnceleme

### 1. Kart yapılandırması — I2C2'de `0x19`

[`Sw/Board/board_config.h`](../../../../Sensors_And_Storage/Feature_Tutorials/03-LIS2DE12_Accelerometer/Sw/Board/board_config.h): yalnızca `TIREMO_BOARD_LIS2DE12_I2C_BUS` / `_ADDRESS`.

### 2. `main.c` — 01/02 ile aynı `PRV_I2C_Init()`

[`Sw/Src/main.c`](../../../../Sensors_And_Storage/Feature_Tutorials/03-LIS2DE12_Accelerometer/Sw/Src/main.c), Eğitim 02'den değişmedi: `PRV_PORT_Init()`, `PRV_SCU_CLK_Init()`, `PRV_UART_Init()`, `PRV_I2C_Init()` (I2C2, 100 kHz master), ardından `PRV_USER_Code()`. LIS2DE12, I2C2'yi SHT40 ile paylaşır; bu yüzden yeni bir satıcı devreye alma çağrısı gerekmez.

### 3. Init, Configure, ReadAccel

[`Sw/Drivers/lis2de12/Src/tiremo_lis2de12.c`](../../../../Sensors_And_Storage/Feature_Tutorials/03-LIS2DE12_Accelerometer/Sw/Drivers/lis2de12/Src/tiremo_lis2de12.c) `WHO_AM_I`'yi okur (en fazla 10 deneme, 10 ms arayla) ve `0x33` bekler, ardından `Configure(2, 100)` çağırır. [`prv_user_code.c`](../../../../Sensors_And_Storage/Feature_Tutorials/03-LIS2DE12_Accelerometer/Sw/Src/prv_user_code.c) içindeki döngü ortalaması alınmış mg değerlerini her 500 ms'de loglar.

### 4. Derle, flaşla ve izle

```sh
tdf build
tdf flash
tdf monitor
```

(Ya da tek komutta zincirleyerek: `tdf build flash monitor`.) Çıktının nasıl görünmesi gerektiği için yukarıdaki **Çözüm** bölümüne bakın.

## Tamamlanma Ölçütü

`tdf monitor` `WHO_AM_I=0x33` gösterir, ardından kart düz dururken Z'de yaklaşık +1000 mg görünür ve kartı sırayla her kenarına yatırdığınızda ~1000 mg X'e ve Y'ye geçer.


<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
