<p class="sphinxhide" align="right"><a href="../../../Getting_Started/02-SysTick_Timing/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>SysTick Timing</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# SysTick Timing

***Kart: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Bölüm: Getting Started · Zorluk: Kolay***

> Bu sayfa [İngilizce orijinalinin](../../../Getting_Started/02-SysTick_Timing/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Sonraki her eğitim — WiFi yeniden denemeleri, MQTT yoklaması, alarm
titreşim giderme (debounce) — bir busy-wait yerine serbest çalışan bir
milisaniye saatine ihtiyaç duyar. Bu eğitim SysTick'i bir kez devreye
alır, böylece sonraki hiçbir eğitim onu yeniden türetmek zorunda kalmaz —
ve bunu Tutorial 01'in busy-wait döngüsünün yapısal olarak yapamayacağı
bir şeyle kanıtlar: tek bir döngüden iki LED'i iki farklı, doğru hızda
çalıştırmak.

## Neler Öğreneceksiniz

- `TIREMO_SysTick_Init()` / `TIREMO_SysTick_GetMs()` / `TIREMO_SysTick_DelayMs()`
- Neden geçen-zaman matematiğinin (`now - last >= interval`) iç içe
  gecikmelerden daha güvenli olduğu ve milisaniye sayacının etrafından
  dolanmasına (wraparound) nasıl dayandığı
- `Tiremo_Hal` (satıcıya dönük) katmanının üzerine yeni bir `Common`
  (uygulamaya dönük) katman: Tutorial 01 GPIO için `Tiremo_Hal`'i tanıttı;
  bu eğitim `Common/` klasörünü ilk kez, zamanlamayla başlayarak ekliyor

## Ön Koşullar

- [01-Hello_Blink](../01-Hello_Blink/README.md) — bu eğitim onun GPIO HAL'ini ve board_config.h desenini değişmeden yeniden kullanır
- `PATH` üzerinde `arm-none-eabi-gcc`, `cmake` (3.20+) ve `ninja` (bkz. [01-Hello_Blink'in Ön Koşullar bölümü](../01-Hello_Blink/README.md#ön-koşullar) — burada aynısı geçerli, Windows'un bu derleme sistemiyle neden Git Bash/WSL gerektirmediği dahil)
- Bilgisayarınıza USB kablosuyla bağlı bir Tiremo Cortex kartı (ABOV A34G43A MCU — bkz. [Donanım Genel Bakışı](../../Hardware.md)). Kartın üzerindeki CMSIS-DAP hata ayıklayıcısı SWD üzerinden flaşlamayı yapar; ayrı bir programlayıcı gerekmez.

## Bloklamayan Zamanlamayı Anlamak

Tutorial 01'in `prvDelay()`'i bloklayan bir busy-wait'ti: CPU, sayım
bitene kadar başka hiçbir şey yapmadan boş bir döngüde oturur. Bu, tek
bir LED için sorun değil. Ama yapısal olarak ikinci bir LED'i farklı bir
hızda süremez, bir butona hizmet edemez, başka bir şeyi yoklayamaz — CPU
saymakla meşgul, sıkışmış durumda. Buradan sonraki her eğitim, döngü
başına birden fazla şey yapmak zorunda, bu yüzden bunun değişmesi
gereken nokta burası.

**Donanım tarafı** ([`Tiremo_Hal/Src/tiremo_hal_systick.c`](../../../Getting_Started/02-SysTick_Timing/Sw/Tiremo_Hal/Src/tiremo_hal_systick.c))
Cortex-M4 çekirdeğinin kendi SysTick çevre birimini yapılandırır —
bir ABOV satıcı zamanlayıcısı değil, her Cortex-M çekirdeğinin standart
bir parçası — milisaniyede bir kesme üretmesi için:

```c
reloadTicks = SystemCoreClock / 1000UL;   /* 1000 Hz = 1 ms periyot */
SysTick_Config(reloadTicks);              /* CMSIS çekirdek çağrısı, satıcı HAL'i değil */
```

`SystemCoreClock`, Tutorial 01'in `PRV_SCU_CLK_Init()`'inin PLL'i
yapılandırmasının bir yan etkisi olarak zaten ayarladığı aynı global
değişkendir — bu eğitim için hiç yeni satıcı kaynak dosyası gerekmez,
sadece bu küçük CMSIS tabanlı dosya artı üzerine kurulu milisaniye sayacı
mantığı.

**Uygulama tarafı** ([`Common/Src/tiremo_systick.c`](../../../Getting_Started/02-SysTick_Timing/Sw/Common/Src/tiremo_systick.c))
sonraki her eğitimin gerçekte çağırdığı şeydir. Yukarıdaki donanım
katmanına bir geri çağırma (callback) kaydeder ve her 1 ms kesmede
serbest çalışan bir sayacı artırır (burada kısaltılmıştır; gerçek
işleyici ayrıca `TIREMO_SysTick_DelayMs()`'in gecikme sayacını azaltır ve
aşağıda anlatılan `TIREMO_SysTick_OnTick1ms()` kancasını çağırır):

```c
static volatile uint32_t s_msTick = 0U;

static void prvSysTickHandleTick(void)
{
    /* ... */
    s_msTick++;
    /* ... */
}

uint32_t TIREMO_SysTick_GetMs(void)
{
    return s_msTick;
}
```

Bu başlık ayrıca `TIREMO_SysTick_DelayMs()`'i de bildirir — `prvDelay()`
ile aynı şekilde kurulmuş, sadece doğru olan *bloklayan* bir gecikme —
ve gelecekteki milisaniye başına uygulama işi için zayıf (weak) bir
`TIREMO_SysTick_OnTick1ms()` kancası. Bu eğitimde ikisi de kullanılmıyor:
buradaki asıl mesele bloklamayan desendir, bloklamanın daha doğru bir
sürümü değil.

**Bu depodaki her bloklamayan döngünün izlediği desen**, izlenen her şey
için kendi "son ateşlenme" zaman damgasıyla tek bir karşılaştırmadır:

```c
now = TIREMO_SysTick_GetMs();
if ((now - lastMs) >= interval)
{
    /* işi yap */
    lastMs = now;
}
```

`now - lastMs`, bunu sonsuza kadar güvenli kılan ayrıntı olan işaretsiz
(unsigned) çıkarmadır — sadece sayaç dolanana kadar değil: `UINT32_MAX + 1`'de
sayaç `0`'a geri döner, ama işaretsiz dolanma aritmetiği `now - lastMs`'in
özel bir durum gerekmeden doğru geçen zamanı hesaplamasını sağlar. Bu,
tam üretim üretici yazılımının bağlantı-yeniden deneme ve telemetri-aralık
mantığı boyunca kullandığı tam olarak aynı deyimdir.

## Çözüm

[`Sw/`](../../../Getting_Started/02-SysTick_Timing/Sw/) gerçek, derlenebilir bir uygulamaya sahip. Depo kökünden:

```sh
cd Getting_Started/02-SysTick_Timing/Sw
tdf build
tdf flash
```

(Yeni bir shell'de ilk seferinde, depo kökünden: `source export.sh`
(bash/zsh), `. .\export.ps1` (PowerShell) ya da `export.bat` (cmd.exe)
`tdf`'yi `PATH`'e ekler. macOS/Linux'ta `tdf.py` takma ad olarak da
çalışır; Windows'ta `tdf` kullanın, ya da export adımı olmadan `Sw\`
içinden `python ..\..\..\tools\tdf.py build`. `tdf`'nin aslında ne olduğu
ve bu deponun neden ham `cmake` çağrıları yerine onu kullandığı için
[01-Hello_Blink'in Çözüm bölümüne](../01-Hello_Blink/README.md#çözüm)
bakın.)

Tutorial 01 ile aynı paylaşılan satıcı SDK'sı
([`Generation/`](../../Generation/), depo kökünde), bu depodaki her
eğitimle aynı CMake derlemesi (neden CMake olduğu için
[01-Hello_Blink'in Çözüm bölümüne](../01-Hello_Blink/README.md#çözüm)
bakın) — bu sefer Tutorial 01'in GPIO HAL'inin üzerine SysTick katmanı
eklenmiş tek bir ~5.5 KB imaj bağlanıyor.

**Gerçek donanımda doğrulandı.** `tdf build`, macOS'ta ve Windows 11'de
hatasız derlenip bağlanıyor. Linker, [01-Hello_Blink'in Çözüm
bölümünde](../01-Hello_Blink/README.md#çözüm) açıklanan beklenen ve
zararsız `_close` / `_lseek` / `_read` / `_write is not implemented`
uyarılarını burada da basar. `tdf flash`, gerçek, bağlı bir Tiremo
Cortex kartına karşı SWD üzerinden tam bir programlama-ve-doğrulama
döngüsünü tamamladı (`** Verified OK **`) ve her iki LED de belirgin
şekilde farklı hızlarda yanıp sönüyor. `Sw/Src/prv_user_code.c`
içindeki `LED1_INTERVAL_MS` / `LED2_INTERVAL_MS` değerleri, iki hız bir
bakışta ayırt edilsin diye `1000` / `5000` (2 s ve 10 s tam döngü)
olarak ayarlanmıştır.

## Adım Adım İnceleme

### 1. Kart yapılandırması — ikinci bir LED ekleme

[`Sw/Board/board_config.h`](../../../Getting_Started/02-SysTick_Timing/Sw/Board/board_config.h),
Tutorial 01'in LED1'inin yanına, aynı desen, aynı dosya ile LED2'yi ekler:

```c
#define TIREMO_BOARD_LED1_PORT  TIREMO_HAL_GPIO_PORT_B
#define TIREMO_BOARD_LED1_PIN   TIREMO_HAL_GPIO_PIN_4
#define TIREMO_BOARD_LED2_PORT  TIREMO_HAL_GPIO_PORT_B
#define TIREMO_BOARD_LED2_PIN   TIREMO_HAL_GPIO_PIN_9
```

### 2. Giriş noktası — Tutorial 01'den değişmedi

[`Sw/Src/main.c`](../../../Getting_Started/02-SysTick_Timing/Sw/Src/main.c),
Tutorial 01'inkiyle harfi harfine aynı şekildedir: `PRV_PORT_Init()`,
`PRV_SCU_CLK_Init()`, `PRV_USER_Code()`. SysTick devreye alma burada
değil `PRV_USER_Code()` içinde gerçekleşir — bu, kart seviyesinde önyükleme
sıralamasına bir değişiklik değil, bu eğitimin yeni uygulama davranışıdır,
bu yüzden `main.c`'in henüz değişmesi için bir sebep yok. (Değişecek,
bir sonraki eğitim uygulamadan önce önyüklenen şeyi gerçekten değiştirmesi
gerektiğinde — örneğin Tutorial 03 buraya UART init ekler.)

### 3. Tek döngüde iki bağımsız zamanlayıcı

[`Sw/Src/prv_user_code.c`](../../../Getting_Started/02-SysTick_Timing/Sw/Src/prv_user_code.c)
programın tamamıdır:

```c
#define LED1_INTERVAL_MS (1000U)  /* -> 2 s tam yanıp sönme döngüsü */
#define LED2_INTERVAL_MS (5000U)  /* -> 10 s tam döngü -- bilinçli olarak LED1'den çok daha yavaş */

void PRV_USER_Code(void)
{
    uint32_t led1LastMs, led2LastMs, now;

    TIREMO_SysTick_Init();
    /* ... her iki LED'i çıkış olarak ayarla, ikisi de kapalı başlar ... */

    led1LastMs = TIREMO_SysTick_GetMs();
    led2LastMs = led1LastMs;

    for (;;)
    {
        now = TIREMO_SysTick_GetMs();

        if ((now - led1LastMs) >= LED1_INTERVAL_MS)
        {
            TIREMO_HAL_GPIO_TogglePin(TIREMO_BOARD_LED1_PORT, TIREMO_BOARD_LED1_PIN);
            led1LastMs = now;
        }
        if ((now - led2LastMs) >= LED2_INTERVAL_MS)
        {
            TIREMO_HAL_GPIO_TogglePin(TIREMO_BOARD_LED2_PORT, TIREMO_BOARD_LED2_PIN);
            led2LastMs = now;
        }
    }
}
```

İki `if`'ten hiçbiri asla bloklamaz, bu yüzden döngü her iki kontrolü de
saniyede binlerce kez dönmekte serbesttir — bu, buradan sonraki her
eğitimin üzerine kurduğu şekildir: tek bir `for (;;)`, içinde birkaç
bağımsız zamanlanmış şey, hiçbiri başka bir şeyi beklemez.

### 4. Derleme — bu sefer yeni satıcı kaynağı yok

[`Sw/CMakeLists.txt`](../../../Getting_Started/02-SysTick_Timing/Sw/CMakeLists.txt),
Tutorial 01'in `CMakeLists.txt`'i artı iki satırdır: `add_executable()`'e
eklenen `Tiremo_Hal/Src/tiremo_hal_systick.c` ve `Common/Src/tiremo_systick.c`,
ve `target_include_directories()`'a eklenen bir `Common/Include`. Satıcı
dosya listesi hiç değişmiyor: SysTick bir ABOV HAL modülü değil, CMSIS
üzerinden erişilen bir çekirdek Cortex-M4 çevre birimidir, bu yüzden
eklenecek satıcı `.c` dosyası gerekmiyor. Tutorial 01'in derleme
yazısıyla karşılaştırmaya değer — orada satıcı kaynak listesini doğru
almak (ve gerçek bir Make hatasını aşmak) zorluğun büyük kısmıydı, burada
kolay kısımdı.

### 5. Flaşlama

```sh
tdf build
tdf flash
```

Tutorial 01 ile aynı işletim sistemi tespiti, aynı paketlenmiş OpenOCD
akışı, sadece tam `cmake --build build --target flash` yazmak yerine
`tdf` üzerinden ulaşılıyor — bunun gerçek donanıma gerçekten
ulaştığının onayı için yukarıdaki Çözüm bölümüne bakın.

## Tamamlanma Ölçütü

Hiçbir debugger bağlı değilken, tek bir bloklamayan döngüden iki LED
bağımsız, gözle görülür farklı hızlarda yanıp söner: LED1 her saniye
(2 s tam döngü), LED2 her 5 saniyede bir (10 s tam döngü) durum değiştirir.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
