<p class="sphinxhide" align="right"><a href="../../../Getting_Started/03-Debug_UART_Logging/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Debug UART Logging</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Debug UART Logging

***Kart: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Bölüm: Getting Started · Zorluk: Kolay***

> Bu sayfa [İngilizce orijinalinin](../../../Getting_Started/03-Debug_UART_Logging/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Buradan sonraki her eğitim bir debugger breakpoint'i değil, bir UART
log'u üzerinden hata ayıklanacak — referans üretici yazılımının kendisi
de böyle geliştirildi. `TIREMO_LOG_*`'ı şimdi çalışır hale getirin,
Tutorial 02'nin iki LED döngüsünün üzerine: her toggle artık sessizce
olmak yerine UART üzerinden anlatılır.

## Neler Öğreneceksiniz

- `TIREMO_LOG_ERROR/WARN/INFO/DEBUG` ve `TIREMO_DEBUG_LEVEL` derleme
  zamanı tavanı
- CMake configure zamanında `-DTIREMO_DEBUG_LEVEL=N` ile ayrıntı
  seviyesini geçersiz kılma — ve bunu sessizce bozabilen bir önişlemci
  tuzağı
- Çalışma zamanı `TIREMO_Debug_SetLevel()`'ın neden yalnızca daha sessize
  gidebildiği, hiç daha yükseğe gidemediği
- Tutorial 01 ve 02 boyunca aynı kalan `main.c`'in neden nihayet bu
  eğitimde değiştiği

## Ön Koşullar

- [02-SysTick_Timing](../../../Getting_Started/02-SysTick_Timing/README.md) — bu eğitimin LED döngüsü, üzerine loglama eklenmiş Tutorial 02'nin ta kendisi
- `PATH` üzerinde `arm-none-eabi-gcc`, `cmake` (3.20+) ve `ninja` (bkz. [01-Hello_Blink'in Ön Koşulları](../../../Getting_Started/01-Hello_Blink/README.md#prerequisites) — burada aynısı geçerli, Windows Git Bash/WSL gerektirmemesi dahil)
- Log çıktısını görmek için 115200 8N1'de bir seri terminal: `tdf monitor` (bkz. [Çözüm](#çözüm)), Tera Term, PuTTY, screen, minicom ya da IDE'nizin yerleşik olanı. Ayrı bir USB-seri adaptör gerekmez: kartın üzerindeki CMSIS-DAP hata ayıklayıcısı aynı USB kablosu üzerinden bir seri port sağlar (Windows'ta örneğin `USB Serial Device (COMx)`).
- Bilgisayarınıza USB kablosuyla bağlı bir Tiremo Cortex kartı (ABOV A34G43A MCU — bkz. [Donanım Genel Bakışı](../../Hardware.md)). Kartın üzerindeki CMSIS-DAP hata ayıklayıcısı SWD üzerinden flaşlamayı yapar; ayrı bir programlayıcı gerekmez.

## Bir Önişlemci Tuzağı: Seviyeler Neden Aynı Zamanda Makro

Bu eğitimin ikinci "Neler Öğreneceksiniz" maddesi — daha düşük bir
`TIREMO_DEBUG_LEVEL`'da yeniden derleyip ayrıntı seviyesinin gerçekten
değiştiğini doğrulamak — `tiremo_debug.h`'de kolayca yanlış yapılabilen
bir ayrıntıya dayanır. Başlığın ilk akla gelen, doğal bir sürümü seviye
enum'unu bildirdikten hemen sonra şöyle görünür:

```c
typedef enum { TIREMO_DEBUG_LEVEL_NONE = 0, ..., TIREMO_DEBUG_LEVEL_DEBUG = 4 } TIREMO_Debug_Level_t;

#define TIREMO_DEBUG_LEVEL  TIREMO_DEBUG_LEVEL_INFO   /* <- koşulsuz */
#ifndef TIREMO_DEBUG_LEVEL
#define TIREMO_DEBUG_LEVEL           TIREMO_DEBUG_LEVEL_INFO
#endif
```

Burada iki bağımsız hata üst üste biniyor. Birincisi, `#ifndef`
korumasının üzerindeki koşulsuz `#define` her zaman kazanıyor —
altındaki koruma asla tetiklenemiyor, bu yüzden derleme zamanında
verilen bir `-DTIREMO_DEBUG_LEVEL=2` her seferinde sessizce `INFO`'ya
geri yazılıyor. Sadece bunu düzeltmek yeterli değil — daha derin sorun,
`TIREMO_DEBUG_LEVEL_ERROR`, `_WARN`, `_INFO`, `_DEBUG`'ın **enum
sabitleri olması, önişlemci makrosu olmaması**. C standardı, önişlemcinin
makro olarak tanımadığı herhangi bir tanımlayıcının bir `#if` ifadesi
içinde `0` olarak değerlendirileceğini söyler — yani dosyadaki her
koruma, örneğin:

```c
#if (TIREMO_DEBUG_LEVEL >= TIREMO_DEBUG_LEVEL_DEBUG)
#define TIREMO_LOG_DEBUG(...) TIREMO_Debug_Log(TIREMO_DEBUG_LEVEL_DEBUG, __VA_ARGS__)
#else
#define TIREMO_LOG_DEBUG(...) ((void)0)
#endif
```

sessizce `0 >= 0`'a düşüyor — her zaman doğru — hangi seviye ayarlanmış
olursa olsun. Bunu önişlemciyle görebilirsiniz (`arm-none-eabi-gcc -E`):
`TIREMO_LOG_DEBUG(...)`, `-DTIREMO_DEBUG_LEVEL=2` geçilse bile gerçek,
canlı bir `TIREMO_Debug_Log()` çağrısına genişler. Her log makrosu her
zaman derlemeye dahil edilir; gerçekten filtreleme yapan tek şey
`TIREMO_Debug_Log()` içindeki *çalışma zamanı* kontrolüdür (gerçek C,
önişlemci `#if`'i değil).

**Bu depoda kullanılan çözüm**, [`Sw/Common/Include/tiremo_debug.h`](../../../Getting_Started/03-Debug_UART_Logging/Sw/Common/Include/tiremo_debug.h)
içinde: her seviyeyi, bir `#if` içinde onlara herhangi bir şey başvurmadan
önce, enum'dan hemen sonra düz sayısal bir makro olarak yeniden bildirmek:

```c
#define TIREMO_DEBUG_LEVEL_NONE     0
#define TIREMO_DEBUG_LEVEL_ERROR    1
#define TIREMO_DEBUG_LEVEL_WARN     2
#define TIREMO_DEBUG_LEVEL_INFO     3
#define TIREMO_DEBUG_LEVEL_DEBUG    4
```

Bu özellikle enum typedef'inden *sonra* geldiği için güvenli — enum'un
kendi bildirimi bu `#define`'lar var olduğunda zaten ayrıştırılmış
durumda, bu yüzden hiçbir şey onu bozmuyor. O noktadan sonra ise, bu
isimlerden her biri — hem bu başlığın kendi `#if` korumalarında, hem de
`TIREMO_Debug_Log(TIREMO_DEBUG_LEVEL_ERROR, ...)` gibi herhangi bir
`.c` dosyasının çağrılarında — derleyici onu görmeden önce eşleşen düz
tam sayıyla değiştiriliyor; bu hem `#if` korumalarını çalışır kılan hem
de mevcut her çağrı noktasına tamamen zararsız olan şey, çünkü sayılar
enum'un kendi değerleriyle tam olarak eşleşiyor.

**Kendiniz nasıl kontrol edersiniz:** bu eğitimin `Sw/`'sini iki kez
derleyin, biri varsayılan seviyede, biri `tdf build -DTIREMO_DEBUG_LEVEL=2`
ile. `strings build/DebugUartLogging | grep "toggle @"` (macOS/Linux) ya
da `Select-String -Path build\DebugUartLogging -Pattern "toggle @"`
(PowerShell) varsayılan derlemede INFO log biçim dizelerini iki kez
bulur, seviye 2'de ise **sıfır** kez — gerçekten dışarıda bırakılmış,
sadece çalışma zamanında bastırılmış değil. İkisi arasında ikili `.text`
boyutu yaklaşık dörtte bir azalır (örneğin Arm GNU Toolchain 14.2.1 ile
yaklaşık 12,6 KB'tan yaklaşık 9,3 KB'a; kesin sayılar toolchain sürümüne
göre biraz değişir).

## Çözüm

[`Sw/`](../../../Getting_Started/03-Debug_UART_Logging/Sw/) gerçek, derlenebilir bir uygulamaya sahip. Depo kökünden:

```sh
cd Getting_Started/03-Debug_UART_Logging/Sw
tdf build
tdf flash
```

(`tdf`'nin ne olduğu ve `./install.sh` + `source ./export.sh` — Windows'ta
`install.ps1` + `. .\export.ps1` ya da `install.bat` + `export.bat` — ile
nasıl `PATH`'e ekleneceği için [01-Hello_Blink'in Çözüm bölümüne](../01-Hello_Blink/README.md#çözüm)
bakın. macOS/Linux'ta `tdf.py` takma ad olarak da çalışır; Windows'ta
`tdf` kullanın.)

Tutorial 01/02 ile aynı paylaşılan satıcı SDK'sı
([`Generation/`](../../../Generation/), depo kökünde), bu depodaki her
eğitimle aynı CMake derlemesi (neden CMake olduğu için
[01-Hello_Blink'in Çözüm bölümüne](../01-Hello_Blink/README.md#çözüm)
bakın). Satıcı kaynak listesinin belirgin şekilde büyüdüğü ilk eğitim
bu: UART'ı devreye almak satıcı UART HAL'ini (`hal_uart.c`) ve şablonun
UART kurulum kodunu (`user_uart.c`, `user_uart_isr.c`) ekler; bunlar da
DMA ve NMI satıcı HAL modüllerini (`hpl_dma.c`, `hpl_nmi.c`) beraberinde
getirir — bkz. [`Sw/CMakeLists.txt`](../../../Getting_Started/03-Debug_UART_Logging/Sw/CMakeLists.txt).

**Gerçek donanımda doğrulandı:** `tdf build`, macOS'ta ve Windows 11'de
her debug seviyesinde hatasız derlenip bağlanıyor (linker,
[01-Hello_Blink'in Çözüm bölümünde](../01-Hello_Blink/README.md#çözüm)
açıklanan beklenen ve zararsız `_close` / `_lseek` / `_read` / `_write is
not implemented` uyarılarını burada da basar). `tdf flash`, gerçek, bağlı
bir Tiremo Cortex kartına karşı SWD üzerinden tam bir
programlama-ve-doğrulama döngüsünü tamamladı (`** Verified OK **`) ve
flaşlanan imaj, Tutorial 02'nin yanıp sönme desenine uyan
`[INFO] LED1 toggle @ ... ms` / `LED2 toggle @ ... ms` satırlarını
sürekli olarak akıtıyor.

Log'u izlemek için çalıştırın:

```sh
tdf monitor
```

Kartı SWD üzerinden resetler ve log'u seviyeye göre renk kodlu olarak
akıtır (çıkmak için Ctrl+C). `pyserial` gerektirir; `install.sh` /
`install.ps1` / `install.bat` bunu sizin için kurar. Seri portu aynı anda
yalnızca bir program açabilir, bu yüzden önce o porttaki diğer terminalleri
(Tera Term, PuTTY, …) kapatın. `tdf monitor`,
[04-Button_Read_And_Debounce'un Çözüm bölümünde](../04-Button_Read_And_Debounce/README.md#çözüm)
daha ayrıntılı anlatılıyor; burada kullanmak için Tutorial 04'ü bitirmeniz
gerekmez.

## Adım Adım

### 1. Kart yapılandırması — Tutorial 02'den değişmedi

[`Sw/Board/board_config.h`](../../../Getting_Started/03-Debug_UART_Logging/Sw/Board/board_config.h),
Tutorial 02'ninkiyle aynıdır (LED1 + LED2). Debug UART'ın fiziksel portu
(`TIREMO_HAL_UART_PORT_0`), `tiremo_debug.c`'nin kendi içinde sabit
kodlanmıştır, bu eğitimin kart yapılandırmasının adlandırması gereken
bir şey değildir.

### 2. Giriş noktası — nihayet değişen

[`Sw/Src/main.c`](../../../Getting_Started/03-Debug_UART_Logging/Sw/Src/main.c),
Tutorial 01/02'nin `PRV_SCU_CLK_Init()`'i ile `PRV_USER_Code()`'u
arasına bir çağrı ekler:

```c
PRV_PORT_Init();
PRV_SCU_CLK_Init();
PRV_UART_Init();     /* yeni: 3 satıcı UART'ının tamamını 115200 8N1'de devreye alır */
PRV_USER_Code();
```

`PRV_UART_Init()` satıcı tarafından üretilmiştir, değiştirilmemiştir —
bu eğitimin kodu onları kullansın ya da kullanmasın, üç fiziksel UART
çevre biriminin tamamını no-op yer tutucu kesme işleyicileriyle
başlatır. [`Common/Src/tiremo_debug.c`](../../../Getting_Started/03-Debug_UART_Logging/Sw/Common/Src/tiremo_debug.c)
daha sonra `TIREMO_Debug_Init()` içinde UART 0'ın kesme işleyicisini
Tiremo'nun kendi gerçek işleyicisiyle yeniden kaydeder — çatışma yok,
sadece o port için satıcı yer tutucusunun üzerine yazan ikinci bir
`HAL_UART_SetIRQ()` çağrısı. `main.c`'in burada değişip Tutorial
01/02'de değişmemesinin nedeni bu: GPIO ve SysTick hiç satıcı çevre
birimi devreye alma çağrısı gerektirmiyordu; UART gerçekten gerektiriyor.

### 3. Mevcut yanıp sönme döngüsünü loglama

[`Sw/Src/prv_user_code.c`](../../../Getting_Started/03-Debug_UART_Logging/Sw/Src/prv_user_code.c),
Tutorial 02'nin döngüsüne iki ekleme yapılmış halidir: başta
`TIREMO_Debug_Init()`, ve her toggle için bir `TIREMO_LOG_INFO()`
çağrısı:

```c
TIREMO_SysTick_Init();
TIREMO_Debug_Init();
TIREMO_LOG_INFO("Debug_UART_Logging: boot");

/* ... Tutorial 02 ile aynı LED kurulumu ... */

for (;;)
{
    now = TIREMO_SysTick_GetMs();
    if ((now - led1LastMs) >= LED1_INTERVAL_MS)
    {
        TIREMO_HAL_GPIO_TogglePin(TIREMO_BOARD_LED1_PORT, TIREMO_BOARD_LED1_PIN);
        TIREMO_LOG_INFO("LED1 toggle @ %lu ms", (unsigned long)now);
        led1LastMs = now;
    }
    /* ... LED2, aynı şekilde ... */
}
```

`TIREMO_LOG_INFO(...)`, derlemeye dahil edildiğinde
`TIREMO_Debug_Log(TIREMO_DEBUG_LEVEL_INFO, ...)`'a genişler,
`[INFO] <mesaj>\r\n` yazdırır, ve bloklayıcıdır
(`prvDebugTransmitBlocking()` UART'ın gerçekten göndermesini bekler) —
müfredatta ilerde sıkı bir zamanlama döngüsüne loglama eklemeden önce
bilinmeye değer, tıpkı Tutorial 01'in `TogglePin()`'in oku-değiştir-yaz
olduğunu ilerisi için işaretlemesi gibi.

### 4. Seviye makroları

Yukarıda tam olarak ele alındı — bkz. **Bir Önişlemci Tuzağı**. Kısa
versiyon: [`Sw/Common/Include/tiremo_debug.h`](../../../Getting_Started/03-Debug_UART_Logging/Sw/Common/Include/tiremo_debug.h)
içinde enum'dan hemen sonra gelen beş `#define` (`TIREMO_DEBUG_LEVEL_NONE`
… `_DEBUG`), böylece `#if` korumaları seviye değerlerini görebilir.

### 5. Derleme ve flaşlama

```sh
tdf build              # varsayılan seviye: INFO ve üzeri
tdf flash

# yalnızca WARN ve üzeri -- INFO satırları derlemeden çıkarılır.
# TIREMO_DEBUG_LEVEL bir CMake *cache* değişkenidir, configure zamanında
# ayarlanır: farklı bir -D ile yeniden geçirene kadar bu build/
# klasöründeki yeniden derlemeler boyunca kalıcıdır (ya da seviye başına
# ayrı bir build klasörü kullanın).
# tdf, -D'yi doğrudan cmake configure'a iletir:
tdf build -DTIREMO_DEBUG_LEVEL=2
tdf flash

# Seviyeler: 0=NONE (tüm loglar derlemeden çıkarılır), 1=ERROR, 2=WARN,
# 3=INFO, 4=DEBUG. Varsayılana (INFO) dönmek için boş değer verin:
tdf build -DTIREMO_DEBUG_LEVEL=
```

Tutorial 01/02 ile aynı işletim sistemi tespiti, aynı paketlenmiş
OpenOCD akışı, sadece `tdf` üzerinden ulaşılıyor — bunun ne olduğu
için [01-Hello_Blink'in Çözüm bölümüne](../01-Hello_Blink/README.md#çözüm)
bakın. Bu eğitim için gerçek donanımda tam olarak neyin doğrulandığı
için yukarıdaki **Çözüm** bölümüne bakın.

## Ne Zaman Tamamlanmış Sayılır

Boot log'u, 115200 8N1 bir seri bağlantı üzerinden önyükleme ve her LED
toggle'ı için `[INFO]` satırları gösterir, ve `-DTIREMO_DEBUG_LEVEL=2`
ile yeniden yapılandırıp derlemek ve flaşlamak bunları tamamen
kaybettirir — bunu seri çıktıda `tdf monitor` ile, ve yukarıda anlatılan
`strings` / ikili boyut kontrolüyle doğrulayın.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
