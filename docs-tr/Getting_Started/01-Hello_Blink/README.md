<p class="sphinxhide" align="right"><a href="../../../Getting_Started/01-Hello_Blink/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Hello Blink</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + AWS IoT üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Hello Blink

***Kart: Tiremo Cortex (ESP32) · Bölüm: Getting Started / Getting Started · Zorluk: Kolay***

> Bu sayfa [İngilizce orijinalinin](../../../Getting_Started/01-Hello_Blink/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

WiFi veya AWS ile konuşmadan önce, flaşlanan bir derlemeye ve gözle görülür
bir şey yapan bir karta ihtiyacınız var. Bu eğitim, araç zincirinin, OpenOCD
bağlantısının ve kart devreye almanın çalıştığını kanıtlamak için LED1'i
yakıp söndürür.

## Neler Öğreneceksiniz

- Aktif-düşük LED kablolaması ve `TIREMO_BOARD_LED*` pin makroları
- `Tiremo_Hal` GPIO sarmalayıcısı (`tiremo_hal_gpio.c`)
- `tdf.py build` / `flash` / `clean-device` — bu deponun CLI aracı, ve altında gerçekte ne olduğu (CMake + Ninja + OpenOCD)

## Ön Koşullar

- `PATH` üzerinde `arm-none-eabi-gcc`, `cmake` (3.20+) ve `ninja` — eksik olanları görmek ve kurulum komutlarını almak için depo kökünden [`./install.sh`](../../../install.sh) çalıştırın (burada Arm GNU Toolchain 15.2.1, CMake 4.3, Ninja 1.13'e karşı derlenip test edildi; makul ölçüde güncel herhangi bir sürüm çalışmalı)
- **Windows:** aşağıdaki komutlar cmd.exe, PowerShell, bash ya da zsh'de aynı şekilde çalışır — Git Bash veya WSL gerekmez. (Bu deponun daha önceki bir sürümü `build.sh`/`flash.sh` bash betikleri kullanıyordu ve bunlardan birini gerektiriyordu; CMake'e geçiş özellikle bu gereksinimi kaldırmak için yapıldı — bkz. aşağıdaki [Çözüm](#çözüm).) `install.sh`'ın kendisi bash'tir — Windows'ta ya Git Bash/WSL'den bir kez çalıştırın, ya da üç aracı kendiniz kurun.
- SWD üzerinden bağlı bir ABOV A34G43A Starter Kit

> **Windows'ta henüz doğrulanmadı.** Aşağıdakilerin tamamı macOS'ta (Arm GNU Toolchain 15.2.1, CMake 4.3.3, Ninja 1.13.2) temiz şekilde derlenip, bağlanıp, flaşlanıyor; CMake toolchain dosyasının Windows dalı (paketlenmiş `win32` OpenOCD'yi seçen) yazıldı ama gerçek Windows donanımında uçtan uca test edilmedi.

## GPIO HAL'ı Anlamak

Bu kod tabanındaki her çevre birimi sürücüsü — bu eğitimin LED'i, sonraki
eğitimlerin ivmeölçer kesme pinleri, I2C veri yolu, USER butonu — ABOV'un
satıcı PCU API'sine doğrudan dokunmak yerine `Tiremo_Hal` üzerinden geçer.
Bu, tek bir dosyalık bir yalıtımdır: bu kart bir gün başka bir MCU
üreticisine geçse, yalnızca `tiremo_hal_gpio.c` değişmesi gerekir, üzerine
kurulu hiçbir sürücü değil.

**API yüzeyi** ([`Tiremo_Hal/Include/tiremo_hal_gpio.h`](../../../Getting_Started/01-Hello_Blink/Sw/Tiremo_Hal/Include/tiremo_hal_gpio.h))
küçük, enum tabanlı bir sarmalayıcıdır. Port ve pinler genel tanımlayıcılardır:

```c
typedef enum { TIREMO_HAL_GPIO_PORT_A = 0, ..., TIREMO_HAL_GPIO_PORT_G } TIREMO_HAL_GPIO_Port_t;
typedef enum { TIREMO_HAL_GPIO_PIN_0 = 0, ..., TIREMO_HAL_GPIO_PIN_15 } TIREMO_HAL_GPIO_Pin_t;
```

Hello Blink bundan tam olarak üç fonksiyon kullanır
([`Tiremo_Hal/Src/tiremo_hal_gpio.c`](../../../Getting_Started/01-Hello_Blink/Sw/Tiremo_Hal/Src/tiremo_hal_gpio.c)):

```c
TIREMO_HAL_GPIO_Status_t TIREMO_HAL_GPIO_SetMode(TIREMO_HAL_GPIO_Port_t port,
                                                 TIREMO_HAL_GPIO_Pin_t pin,
                                                 TIREMO_HAL_GPIO_Mode_t mode);
TIREMO_HAL_GPIO_Status_t TIREMO_HAL_GPIO_WritePin(TIREMO_HAL_GPIO_Port_t port,
                                                  TIREMO_HAL_GPIO_Pin_t pin,
                                                  TIREMO_HAL_GPIO_Level_t level);
TIREMO_HAL_GPIO_Status_t TIREMO_HAL_GPIO_TogglePin(TIREMO_HAL_GPIO_Port_t port,
                                                   TIREMO_HAL_GPIO_Pin_t pin);
```

İçeride, her biri genel enum'u ABOV'un satıcı PCU türüne düz bir cast ile
çevirir, ardından doğrudan satıcı HAL'ini çağırır:

```c
vendorStatus = HAL_PCU_SetInOutMode((PCU_ID_e)port, (PCU_PIN_ID_e)pin, vendorMode);
```

Soyutlamanın tamamı bu kadar — register erişimi yok, sihir yok, sadece bir
tarafında kararlı, satıcıdan bağımsız bir isim taşıyan bir çeviri katmanı.

**Bilinmesi gereken bir ayrıntı:** `TIREMO_HAL_GPIO_TogglePin()` atomik bir
donanım toggle'ı değildir. Uygulamasına bakın, sadece bir okuma ve ardından
bir yazmadır:

```c
TIREMO_HAL_GPIO_Status_t TIREMO_HAL_GPIO_TogglePin(...)
{
    TIREMO_HAL_GPIO_Level_t level;
    TIREMO_HAL_GPIO_ReadPin(port, pin, &level);
    level = (level == TIREMO_HAL_GPIO_LEVEL_HIGH) ? TIREMO_HAL_GPIO_LEVEL_LOW
                                                   : TIREMO_HAL_GPIO_LEVEL_HIGH;
    return TIREMO_HAL_GPIO_WritePin(port, pin, level);
}
```

1 Hz'lik bir yanıp sönme için sorun değil. İleride bir eğitim bir pini bir
kesme işleyicisinden toggle'larsa bunu hatırlamakta fayda var — orada bir
oku-değiştir-yaz yarışı teorik değil, gerçek bir hatadır.

**Aktif-düşük kablolama:** LED1, `LEVEL_LOW` onu yakacak, `LEVEL_HIGH`
söndürecek şekilde kablolanmıştır — "HIGH = açık" sezgisinin tersine.
[`Board/board_config.h`](../../../Getting_Started/01-Hello_Blink/Sw/Board/board_config.h)
bunu `TIREMO_BOARD_LED_ACTIVE_LOW` ile belgeler, ama — kodu daha sonra
okurken kafa karışıklığının yaygın bir kaynağı olduğu için burada net
olmakta fayda var — bu makro bu eğitimin kodunda yalnızca açıklayıcıdır.
Mantığı tersine çevirmek için değerini okuyan hiçbir şey yoktur;
`prv_user_code.c` "kapalı" durumu için doğrudan `LEVEL_HIGH`'ı sabit
olarak yazar, makronun belgelediğiyle eşleşir ama ona başvurmaz. (Bu,
tam üretim üretici yazılımının kendi LED sürücüsünün çalışma şeklini de
yansıtır.)

## Çözüm

[`Sw/`](../../../Getting_Started/01-Hello_Blink/Sw/) gerçek, derlenebilir bir uygulamaya sahip. Sadece ilk seferde, depo kökünden:

```sh
./install.sh          # arm-none-eabi-gcc, cmake, ninja'yı kontrol eder / kurmayı teklif eder
source ./export.sh     # bu shell oturumu için tdf.py'yi PATH'e ekler (isteğe bağlı ama önerilir)
```

Sonra, `Sw/` içinden:

```sh
cd Sw
tdf.py build
tdf.py flash
```

**`tdf.py` aslında ne:** [`tools/tdf.py`](../../../tools/tdf.py) içindeki
ince bir Python sarmalayıcı — ESP-IDF için `idf.py`'nin oynadığı aynı
rol. Kendi başına hiçbir OpenOCD ya da derleyici mantığı taşımaz;
`tdf.py build` tam olarak `cmake -G Ninja -B build
-DCMAKE_TOOLCHAIN_FILE=<depo>/cmake/toolchain-arm-none-eabi.cmake &&
cmake --build build`'dir, ve `tdf.py flash` bunun ardına `cmake --build
build --target flash` ekler. Kendi dosya yolundan depoyu kendisi bulur,
bu yüzden `python tools/tdf.py build` `export.sh` hiç çalıştırılmadan
da her yerden aynı şekilde çalışır — o betik yalnızca çıplak `tdf.py`
yazabilme kolaylığını ekler. Ayrıca `tdf.py clean` (`build/`'i kaldırır)
ve `tdf.py clean-device` (bağlı kartın çip flash'ını toptan siler —
önce sorar) de var. Gerçekten okunacak bir UART ortaya çıkınca eklenen
`tdf.py monitor` (bkz. [04-Button_Read_And_Debounce'un Çözüm bölümü](../../../Getting_Started/04-Button_Read_And_Debounce/README.md#solution)),
kartı SWD üzerinden resetler ve log'u akıtır — saf CMake devretmesi
olmayan tek alt komut, çünkü ayrıca `pyserial` de gerektiriyor;
`install.sh` tarafından ayrı bir `tdf_venv/` içine kurulur, `export.sh`
tarafından etkinleştirilir.

Bu depodaki paylaşılan satıcı SDK'sına karşı derlenir
([`Generation/`](../../../Generation/), depo kökünde — neden tek seferde,
paylaşılan olarak kontrol altına alındığını, her eğitime kopyalanmak
yerine, o klasöre bakın) ve tek bir ~5 KB imaj bağlar: sensör yok, radyo
yok, debug UART yok, sadece port init, saat init ve GPIO.

**Neden CMake, elle yazılmış bir Makefile değil:** bu eğitim başlangıçta
düz bir Makefile ile inşa edildi — referans üretici yazılımının kendi
derleme sistemiyle eşleşerek — ve çalıştı. Ama Windows'ta Git Bash ya da
WSL gerektiriyordu (artı ayrı bir `make` kurulumu, çünkü Git Bash bir
tane içermiyor), ve gerçek bir hatayla karşılaştı: `..` bileşenleri
içeren bir satıcı yolu, bu eğitimlerin her Makefile'ının kullandığı saf
`$(patsubst %.c,$(BUILD_DIR)/obj/%.o,$(SRCS))` deseninden geçirildiğinde,
bu `..`'ların `build/obj/`'den kaçmasına ve nesne dosyalarının derleme
klasörünün tamamen dışında, başıboş bir dizin ağacına yazılmasına izin
veriyordu. Make sürümü bunu bir `vpath` numarasıyla aşıyordu; CMake'in
`add_executable()`'ı sorunun tamamını atlıyor, çünkü ağaç-dışı kaynaklar
için nesne dosyası yerleşimini kendi içinde yönetiyor. Windows shell
bağımlılığını kaldırmak ile bir hata sınıfını tamamen kaldırmak arasında,
CMake kazandı — bkz. [`Sw/CMakeLists.txt`](../../../Getting_Started/01-Hello_Blink/Sw/CMakeLists.txt).
CMake'in kendi çağrısının bile bir öğrenme eğrisi var yine de
(`cmake -G Ninja -B build -DCMAKE_TOOLCHAIN_FILE=...` kimsenin her
eğitim için ezberlemesi ya da yeniden yazması gereken bir şey değil) —
`tdf.py`'nin var olma sebebi de bu.

**Dürüstlük notu:** `tdf.py build` temiz şekilde derlenip bağlanıyor
(bu makinede doğrulandı, Arm GNU Toolchain 15.2.1, CMake 4.3.3, Ninja
1.13.2, macOS/arm64 — eski Make tabanlı derlemeyle aynı `.text`/`.data`/`.bss`
boyutları). `tdf.py flash`, gerçek, bağlı bir ABOV
A34G43A Starter Kit'e karşı SWD üzerinden tam bir programlama-ve-doğrulama
döngüsünü tamamladı (`** Verified OK **`) — bu eğitim artık gerçekten
flaşlanmış durumda, sadece bağlanmış değil. `tdf.py clean-device` de
gerçekten denendi: kartı sildi (silme sonrası halt durumunun
`pc: 0xfffffffe` — klasik boş-flash imzası — okunmasıyla doğrulandı),
sonra `tdf.py flash` onu normal önyüklemeye geri getirdi.
`Sw/Src/prv_user_code.c` içindeki `BLINK_DELAY_LOOPS`, kalibre edilmiş bir zaman tabanı değil,
elle seçilmiş bir tahmindir — kartınızda yanıp sönme hızı gözle görülür
şekilde farklıysa yukarı veya aşağı ayarlayın.

## Adım Adım

### 1. Kart yapılandırması — pini adlandırma

[`Sw/Board/board_config.h`](../../../Getting_Started/01-Hello_Blink/Sw/Board/board_config.h)
LED1'in fiziksel konumuna, kodun geri kalanının çıplak bir port/pin çifti
yerine kullanabileceği bir isim verir:

```c
#define TIREMO_BOARD_LED_ACTIVE_LOW    (1U)
#define TIREMO_BOARD_LED1_PORT         TIREMO_HAL_GPIO_PORT_B
#define TIREMO_BOARD_LED1_PIN          TIREMO_HAL_GPIO_PIN_4
```

Yeni bir donanım parçası ekleyen her sonraki eğitim aynı deseni izler:
ne olduğunu ve nerede yaşadığını burada tanımlayın, sonra her yerde ismi
kullanın — bu dosyanın dışında hiçbir yerde çıplak bir port/pin çifti
kullanmayın.

### 2. Giriş noktası — kodunuz çalışmadan önce ne önyükleniyor

[`Sw/Src/main.c`](../../../Getting_Started/01-Hello_Blink/Sw/Src/main.c),
satıcı tarafından üretilen şablon giriş noktasının bilinçli olarak
sadeleştirilmiş bir sürümüdür:

```c
int main(void)
{
    PRV_PORT_Init();      /* satıcı tarafından üretilen pin muxing, startup_a34g43x.h içinde static inline */
    PRV_SCU_CLK_Init();   /* satıcı tarafından üretilen saat yapılandırması, şablondan değiştirilmedi */
    PRV_USER_Code();      /* bu eğitimin kodu -- bkz. adım 3 */
    return 0;
}
```

Tam satıcı şablonu burada ayrıca `PRV_UART_Init()`, `PRV_ADC_Init()`,
`PRV_Timer1_Init()` ve `PRV_I2C_Init()` çağırır, artı isteğe bağlı debug
UART kurulumu. Bu eğitim bunların hiçbirine henüz ihtiyaç duymuyor — UART
loglama [03. Eğitim](../../../Getting_Started/03-Debug_UART_Logging/README.md)'de
— bu yüzden `main.c` yalnızca GPIO-only bir programın gerçekten ihtiyaç
duyduğu iki çağrıyı tutar: pin muxing ve saat kurulumu. Kullanılmayan
çevre birimi init'lerini dışarıda bırakmak sadece derlilik değil — aynı
zamanda aşağıdaki CMakeLists.txt'in neden `hal_uart.c`, `hal_adc.c`,
`hal_timer1.c` ya da `hal_i2c.c` derlemesine hiç gerek duymadığının da
nedenidir.

### 3. Yanıp sönme döngüsü

[`Sw/Src/prv_user_code.c`](../../../Getting_Started/01-Hello_Blink/Sw/Src/prv_user_code.c)
programın tamamıdır:

```c
#define BLINK_DELAY_LOOPS (600000UL)

static void prvDelay(void)
{
    volatile uint32_t i;
    for (i = 0UL; i < BLINK_DELAY_LOOPS; i++) { /* empty */ }
}

void PRV_USER_Code(void)
{
    TIREMO_HAL_GPIO_SetMode(TIREMO_BOARD_LED1_PORT, TIREMO_BOARD_LED1_PIN,
                            TIREMO_HAL_GPIO_MODE_OUTPUT_PP);
    TIREMO_HAL_GPIO_WritePin(TIREMO_BOARD_LED1_PORT, TIREMO_BOARD_LED1_PIN,
                             TIREMO_HAL_GPIO_LEVEL_HIGH);  /* start off */

    for (;;)
    {
        TIREMO_HAL_GPIO_TogglePin(TIREMO_BOARD_LED1_PORT, TIREMO_BOARD_LED1_PIN);
        prvDelay();
    }
}
```

Döngü sayacındaki `volatile` süsleme değil, taşıyıcı bir gerekliliktir:
`-Og` seviyesinde derleyici aksi halde döngünün gözlemlenebilir bir etkisi
olmadığını fark edip tamamen silmekte özgürdür. Bu fonksiyonun tamamı,
[02. Eğitim](../../../Getting_Started/02-SysTick_Timing/README.md)'in
tam olarak değiştirdiği şeydir — aynı LED, aynı fikir, ama elle ayarlanmış
boş bir döngü yerine SysTick donanım zamanlayıcısına göre zamanlanmış;
böylece tek bir döngüden iki LED bağımsız, doğru hızlarda yanıp
sönebilir.

### 4. Derleme — tek hedef, iki kaynak grubu

[`Sw/CMakeLists.txt`](../../../Getting_Started/01-Hello_Blink/Sw/CMakeLists.txt)
tek bir `add_executable(HelloBlink ...)` bildirir; bu eğitimin kendi
kaynaklarını (`Src/`, `Tiremo_Hal/`) paylaşılan
[`Generation/`](../../../Generation/) SDK'sından üç dizin yukarıdan
(`../../../Generation/AUDK32_A34xxxx-1.0.12`) çekilen altı satıcı
dosyasıyla birlikte listeler — bu eğitimin orijinal Make tabanlı
sürümünün aksine ayrı bir kaynak-grubu muhasebesi gerekmez (tam olarak
nelerin işin içinde olduğu için yukarıdaki [Çözüm](#çözüm) bölümündeki
**Neden CMake** notuna bakın). `target_include_directories()` ve
`target_compile_definitions()`, eski Makefile'ın elle geçirdiği aynı
`-I`/`-D` bayraklarını taşır; çapraz derleyicinin kendisi (bayraklar, CPU
hedefi, linker özellikleri) depo kökündeki
[`cmake/toolchain-arm-none-eabi.cmake`](../../../cmake/toolchain-arm-none-eabi.cmake)
dosyasında tek bir yerde yaşar, her `CMakeLists.txt`'te tekrarlanmak
yerine her eğitim tarafından paylaşılır.

### 5. Flaşlama

```sh
tdf.py build   # -> build/HelloBlink (.elf) + .bin, boyut raporu
tdf.py flash
```

`CMakeLists.txt`'teki `flash` özel hedefi (`tdf.py flash`'ın arkasında
sadece `cmake --build build --target flash` çalıştırdığı), işletim
sisteminize uyan önceden derlenmiş OpenOCD ikili dosyasını seçer (satıcı
SDK'sı Linux/macOS/Windows için birer tane gönderir,
`CMAKE_HOST_SYSTEM_NAME`/`_PROCESSOR` ile tespit edilir) ve tek bir
bütün imajı adres 0'a programlar. Bunun yerine tüm çipi silen bir
`erase-device` hedefi de var (`tdf.py clean-device`) — bir kartı
flaşlamadan önce gerçekten boş olduğunu kendinize kanıtlamak için
kullanışlı. Windows'a
özel kurulum için yukarıdaki Ön Koşullar'a, gerçek donanımda gerçekte
neyin doğrulanıp doğrulanmadığı için yukarıdaki Çözüm bölümündeki
Dürüstlük notuna bakın.

## Ne Zaman Tamamlanmış Sayılır

Taze bir flaşlamadan sonra, hiçbir debugger bağlı değilken LED1 1 Hz'de
yanıp söner.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
