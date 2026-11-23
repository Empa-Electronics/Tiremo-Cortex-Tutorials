<p class="sphinxhide" align="right"><a href="../../../../Sensors_And_Storage/Feature_Tutorials/04-Battery_Voltage_ADC/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>Battery Voltage via ADC</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Battery Voltage via ADC

***Kart: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Bölüm: Sensors & Local Storage / Feature Tutorials · Zorluk: Orta***

> Bu sayfa [İngilizce orijinalinin](../../../../Sensors_And_Storage/Feature_Tutorials/04-Battery_Voltage_ADC/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Bu kartta ayrılmış bir batarya-ölçüm pini yoktur. Bunun yerine üretici yazılımı, çipin dahili VCORE referansını MCU'nun **analog beslemesine (AVDD, VDDA hattı)** karşı ölçerek bu besleme gerilimini hesaplar. Üretici yazılımının geri kalanının "batarya" olarak raporladığı besleme izleyicisi budur — bu yüzden gerçekte neyi ölçtüğü konusunda net olmakta fayda var (aşağıya bakın). Bu aynı zamanda, mikrofon eğitimi aynı çevre birimini burst modunda kullanmadan önce `Tiremo_Hal_ADC`'ye iyi bir giriştir.

## Neler Öğreneceksiniz

- `Tiremo_Hal_ADC` sarmalayıcısı (`TIREMO_HAL_ADC_ReadChannel`)
- Dahili bir referans kanalının ADC'nin kendi beslemesini nasıl hesaplattığı: `AVDD_mV = (VCORE_REF_MV * 4096) / vcoreRaw`, `VCORE_REF_MV = 1000` ile
- Bu sayının batarya hakkında neyi söylediği — ve neyi söylemediği

## Ön Koşullar

- Kavramsal olarak [03-LIS2DE12_Accelerometer](../03-LIS2DE12_Accelerometer/README.md); bu `Sw/` yeni bir projedir (ADC var, I2C yok)
- Okumaları izlemek için: `tdf monitor` (kurulum için [Getting_Started/04-Button_Read_And_Debounce'un Çözüm bölümüne](../../../Getting_Started/04-Button_Read_And_Debounce/README.md#çözüm) bakın) ya da kartın USB seri portunda 115200 8N1'de herhangi bir seri terminal (Tera Term, PuTTY, screen, minicom, …)
- Bilgisayarınıza USB kablosuyla bağlı bir Tiremo Cortex kartı (ABOV A34G43A MCU — bkz. [Donanım Genel Bakışı](../../../Hardware.md)). Kartın üzerindeki CMSIS-DAP hata ayıklayıcısı SWD üzerinden flaşlamayı yapar; ayrı bir programlayıcı gerekmez.
- İsteğe bağlı: sonucu gerçek VDDA hattıyla karşılaştırmak için bir multimetre

## Referans yöntemini anlamak

[`Sw/Drivers/battery/Src/tiremo_battery.c`](../../../../Sensors_And_Storage/Feature_Tutorials/04-Battery_Voltage_ADC/Sw/Drivers/battery/Src/tiremo_battery.c) referans sürücüden kopyalanmıştır. ADC1'in 23. kanalı dahilidir — dışarıya açılan bir pini yoktur — ve çipin VCORE referansına (~1,0 V) bağlıdır. 12-bit dönüştürücü her girişi kendi beslemesi olan AVDD'nin bir oranı olarak ölçer; yani:

```text
vcoreRaw = VCORE / AVDD * 4096   ->   AVDD = VCORE * 4096 / vcoreRaw
```

Bu yüzden daha büyük bir ham değer, *daha küçük* bir AVDD anlamına gelir. `VCORE_REF_MV = 1000` ile `1520` ham değeri `1000 * 4096 / 1520 = 2694 mV` verir.

**Bu kartta neyi ölçer.** AVDD (MCU'nun 11. pini), analog mikrofon için **LDK130M-R** ayarlanabilir LDO'sunun **2,7 V**'ta ürettiği **VDDA** hattından beslenir (`R1 = 40K + 7,5K`, `R2 = 20K`, `VO = 0,8 V × (1 + R1/R2) = 2,70 V`). Bu LDO'nun kendisi 3,3 V'luk `VDD` hattından beslenir (USB 5 V → LD1117S33). Dolayısıyla USB'den beslenen bir kartta bu eğitim sabit ~2,7 V okur: analog beslemenin sağlıklı olduğunu söyler, bataryanın ne kadar dolu olduğunu değil. Bir batarya ancak regülatörlerin çıkışlarını tutamayacağı kadar düştüğünde burada görünür.

`TIREMO_BAT_Init()`, dahili referansın oturması için ilk okumadan önce meşgul-bekleme yapar (500 000 döngü). `PRV_ADC_Init()` (satıcı tarafından üretilen) ADC1'i tekli / yazılım-tetikli modda devreye alır — bu eğitimin kullandığı dönüştürücü budur. Ayrıca sonraki eğitim için ADC0'ı burst/Timer1 modunda devreye alır; bu eğitim onu hiç başlatmaz.

## Çözüm

Depo kökünden:

```sh
cd Sensors_And_Storage/Feature_Tutorials/04-Battery_Voltage_ADC/Sw
tdf build
tdf flash
tdf monitor
```

(`tdf`'nin ne olduğu için [Getting_Started/01-Hello_Blink'in Çözüm bölümüne](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) bakın. macOS/Linux'ta `tdf.py` takma ad olarak da çalışır; Windows'ta `tdf` kullanın. `tdf monitor`'ü çalıştırmadan önce kartın seri portundaki diğer terminalleri kapatın — portu aynı anda yalnızca bir program açabilir.)

**Derleme notları:** [01-Hello_Blink'in Çözüm bölümünde](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) açıklanan beklenen ve zararsız `_close` / `_lseek` / `_read` / `_write is not implemented` linker uyarılarının yanında, bu derleme satıcı ADC HAL başlığından (`hal_adc_v_01_00_05.h`) iki `-Wenum-compare` uyarısı da basar. Bunlar değiştirilmemiş satıcı kodundan gelir ve zararsızdır.

**Gerçek donanımda doğrulandı:** `tdf build`, macOS'ta ve Windows 11'de derlenip bağlanıyor (`.text` yaklaşık 14 KB; kesin boyut toolchain sürümüne göre biraz değişir). `tdf flash`, gerçek, bağlı bir Tiremo Cortex kartına karşı SWD üzerinden tam bir programlama-ve-doğrulama döngüsünü tamamladı (`** Verified OK **`). Log:

```text
[INFO] Battery_Voltage_ADC: boot
[INFO] BAT: VCORE raw=1520  AVDD=2694 mV
[INFO] BAT: VCORE raw=1520  AVDD=2694 mV
[INFO] BAT: VCORE raw=1520  AVDD=2694 mV
```

her saniye yeni bir satır gelir. Aynı kartın VDDA hattında (C16 ya da MCU'nun 11. pini) multimetre **2,69 V** okudu — üretici yazılımının sonucu gerçek hatla örtüşüyor.

## Adım Adım

### 1. Kart yapılandırması — ADC1, kanal 23

[`Sw/Board/board_config.h`](../../../../Sensors_And_Storage/Feature_Tutorials/04-Battery_Voltage_ADC/Sw/Board/board_config.h): `TIREMO_BOARD_BATTERY_ADC_ID` = `TIREMO_HAL_ADC_ID_1`, `TIREMO_BOARD_BATTERY_VCORE_CHANNEL` = `23`. Hiç GPIO kullanılmaz — kanal dahilidir.

### 2. `main.c` — `PRV_ADC_Init()`

[`Sw/Src/main.c`](../../../../Sensors_And_Storage/Feature_Tutorials/04-Battery_Voltage_ADC/Sw/Src/main.c), `PRV_USER_Code()`'dan önce `PRV_PORT_Init()`, `PRV_SCU_CLK_Init()`, `PRV_UART_Init()` ve ardından `PRV_ADC_Init()`'i (satıcı tarafından üretilen ADC devreye alma) çağırır. Bu projede I2C yoktur.

### 3. Döngü — 1 Hz `TIREMO_BAT_Read()`

[`Sw/Src/prv_user_code.c`](../../../../Sensors_And_Storage/Feature_Tutorials/04-Battery_Voltage_ADC/Sw/Src/prv_user_code.c), `TIREMO_BAT_Init()`'i bir kez çağırır (başarısız olursa `[ERROR] BAT: init failed` loglayıp durur), ardından `TIREMO_BAT_Read()`'i açılışta ve her 1000 ms'de çağırarak hem ham değeri hem hesaplanan AVDD'yi loglar. Başarısız bir okuma `[WARN] BAT: read failed` loglar.

### 4. Derle, flaşla ve izle

```sh
tdf build
tdf flash
tdf monitor
```

(Ya da tek komutta zincirleyerek: `tdf build flash monitor`.) Çıktının nasıl görünmesi gerektiği için yukarıdaki **Çözüm** bölümüne bakın.

## Ne Zaman Tamamlanmış Sayılır

`tdf monitor` saniyede bir `BAT: VCORE raw=…  AVDD=… mV` gösterir ve AVDD 2,7 V civarındadır; multimetreniz varsa değer, C16'da ya da MCU'nun 11. pininde ölçülen VDDA hattından ~50 mV içindedir.


<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
