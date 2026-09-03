<p class="sphinxhide" align="right"><a href="../../../Getting_Started/04-Button_Read_And_Debounce/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Button Read And Debounce</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Button Read And Debounce

***Kart: Tiremo Cortex (ESP32) · Bölüm: Getting Started / Getting Started · Zorluk: Kolay***

> Bu sayfa [İngilizce orijinalinin](../../../Getting_Started/04-Button_Read_And_Debounce/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

USER butonu, üretim üretici yazılımında yalnızca tek bir şey için
kullanılır — 10 saniyelik bir fabrika sıfırlama basılı tutma işlemi —
ama bu, o özelliği daha sonra inşa edebilmeden önce temiz, debounce
edilmiş basma/tutma algılamasına ihtiyacınız olduğu anlamına gelir. Bu
eğitim, USER butonunu Tutorial 03'ün UART loglamasının üzerine ekler:
her basma, bırakma ve uzun-tutma eşiği geçişi kendi log satırını alır,
tek bir fiziksel basmanın birkaç basma gibi yanlış okunmaması için
debounce edilmiş şekilde.

## Neler Öğreneceksiniz

- `Drivers/button` katmanıyla sarmalanmış, `Tiremo_Hal_GPIO` üzerinden
  buton okuma — bu depodaki yalnızca `Tiremo_Hal`/`Common`'a değil,
  `Drivers/`'a da dokunan ilk eğitim
- Referans buton sürücüsünün kendisinin **hiç debounce mantığı
  olmamasının** nedeni, ve bunun tek gerçek çağıranı için neden bilinçli
  ve savunulabilir bir seçim olduğu
- Tutorial 02'nin aynı bloklamayan `TIREMO_SysTick_GetMs()` deyimini
  kullanan, ham sürücünün üzerine uygulama kodunda inşa edilen küçük bir
  debounce durum makinesi
- Bir uzun-tutma eşiğini, bırakmayı beklemeden, aşıldığı anda algılama —
  referans üretici yazılımının fabrika sıfırlama kontrolünün kullandığı
  ile aynı şekil
- `tdf.py monitor` — bu eğitimde eklenen yeni bir alt komut; kartı SWD
  üzerinden resetler ve UART log'unu seviyeye göre renk kodlu olarak
  akıtır, böylece buton olayları yalnızca teorik olarak loglanmak yerine
  gerçekten izlenebilir hale gelir

## Ön Koşullar

- [03-Debug_UART_Logging](../../../Getting_Started/03-Debug_UART_Logging/README.md) — bu eğitimin LED döngüsü ve loglaması, buton işleme eklenmiş Tutorial 03'ün ta kendisi
- `PATH` üzerinde `arm-none-eabi-gcc`, `cmake` (3.20+) ve `ninja` (bkz. [01-Hello_Blink'in Ön Koşulları](../../../Getting_Started/01-Hello_Blink/README.md#prerequisites))
- Basma/bırakma/tutma log satırlarını görmek için: ya `tdf.py monitor`
  (aşağıdaki **Çözüm**'e bakın — `pyserial` gerektiriyor, `./install.sh` /
  `install.ps1` / `install.bat` bunu ayrı bir `tdf_venv/` içinde kurar), ya
  da 115200 8N1'de herhangi bir USB-seri terminal
- USER butonuna erişilebilir, SWD üzerinden bağlı bir ABOV A34G43A Starter Kit

## Debounce'u Anlamak (ve Referans Sürücünün Neden Hiç Debounce'u Olmadığı)

Mekanik bir buton, açıktan kapalıya temiz bir şekilde geçmez. Kontaklar
birkaç milisaniye boyunca fiziksel olarak sekiyor, bu yüzden tek bir
basma, oturana kadar birkaç hızlı `LOW`/`HIGH`/`LOW` geçişi olarak
okunabilir. Ham bir GPIO seviyesini yeterince hızlı yoklarsanız, bu
gürültüyü tek bir parmak basması için birden fazla "basma" olarak
görürsünüz.

Yine de bu eğitimin
[`Sw/Drivers/button/Src/tiremo_button.c`](../../../Getting_Started/04-Button_Read_And_Debounce/Sw/Drivers/button/Src/tiremo_button.c)
dosyasına (referans sürücüden birebir kopyalanmıştır) bakın —
`TIREMO_Button_IsPressed()` sadece şudur:

```c
level = prvButtonReadLevel(buttonId);   /* tek bir ham TIREMO_HAL_GPIO_ReadPin() */
#if (TIREMO_BOARD_BUTTON_ACTIVE_LOW == 1U)
return (level == TIREMO_HAL_GPIO_LEVEL_LOW);
#else
return (level == TIREMO_HAL_GPIO_LEVEL_HIGH);
#endif
```

Hiç filtreleme yok, hiç debounce zamanlayıcısı yok, hiçbir şey yok. Bu
bir gözden kaçırma değil — çünkü sürücünün üretim üretici yazılımındaki
tek gerçek çağıranı, `TIREMO_APP_ServiceFactoryResetHold()`, tek bir
şeyi önemser: buton *kesintisiz* olarak tam 10.000 ms basılı tutulmuş
mu. Bir basmanın başında birkaç milisaniyelik kontak sekmesi, etkin
başlangıç zamanını 10 saniyelik bir eşiğin gürültüsünde tamamen kaybolan
bir miktar kaydırır — eşik, sekme penceresinden dört büyüklük mertebesi
daha büyük olduğu için bu, gerçek bir 10 saniyelik tutmayı asla yanlış
negatife, ya da gerçek bir kısa basmayı asla yanlış pozitife
çeviremez. Sürücü seviyesinde debounce yapmak, o belirli çağıranın
sahip olmadığı bir soruna harcanan bir çaba olurdu.

Ancak bu eğitimin kendi döngüsünde bu sorun *var* — her basma ve bırakmayı
ayrı, ayrık bir olay olarak loglar, ve sekme gürültüsü tek bir fiziksel
basmayı birkaç milisaniye arayla sahte "basıldı" / "bırakıldı" çiftleri
patlamasına dönüştürür. Bu yüzden debounce mantığı gerçekten ihtiyaç
duyulan yerde yaşar: bu eğitimin
[`Sw/Src/prv_user_code.c`](../../../Getting_Started/04-Button_Read_And_Debounce/Sw/Src/prv_user_code.c)
dosyası içindeki `PollButton()`'da, sürücünün bir katman üstünde — tam
olarak referans mimarisinin, çevre birimine genel değil tek bir çağırana
özgü politikayı koyduğu yerde.

## Çözüm

[`Sw/`](../../../Getting_Started/04-Button_Read_And_Debounce/Sw/) gerçek, derlenebilir bir uygulamaya sahip:

```sh
cd Sw
tdf.py build
tdf.py flash
```

(`tdf.py`'nin ne olduğu ve `./install.sh` + `source ./export.sh` — Windows'ta
`install.ps1` + `. .\export.ps1` ya da `install.bat` + `export.bat` — ile
nasıl `PATH`'e ekleneceği için [01-Hello_Blink'in Çözüm bölümüne](../../../Getting_Started/01-Hello_Blink/README.md#solution)
bakın.)

Önceki her eğitimle aynı paylaşılan satıcı SDK'sı ve aynı CMake derleme
şekli ([`Generation/`](../../../Generation/), depo kökünde). Hiç yeni
satıcı kaynağı gerekmedi — buton, Tutorial 01'den beri `hal_pcu.c`
tarafından zaten kapsanan saf bir GPIO — bu yüzden tek yeni derleme
girdisi, bu eğitimin kendi `Drivers/button/Src/tiremo_button.c`'i,
Tutorial 03'ün UART/debug kaynaklarının yanına `CMakeLists.txt`'e
eklendi.

**Flaşlandığı doğrulandı, gözlemlendiği de doğrulandı — geriye dürüst
bir açık kaldı:** `tdf.py build` temiz şekilde derlenip bağlanıyor
(`.text` 13216 bayt). `tdf.py flash`, gerçek, bağlı bir ABOV A34G43A
Starter Kit'e karşı SWD üzerinden tam bir programlama-ve-doğrulama
döngüsünü tamamladı (`** Verified OK **`). UART log'unu okumak daha önce
burada engelliydi — bu ortamda seri terminal yoktu — bu yüzden bu eğitim
`tdf.py monitor`'ı da ekliyor:

```sh
tdf.py monitor
```

Bu, kartı SWD üzerinden resetler (yeni bir `reset-device` CMake hedefi,
`init; reset run; shutdown` — aynı OpenOCD, yeniden programlama yok) ve
ardından log'u seviyeye göre renk kodlu olarak akıtır (`[ERROR]` kırmızı,
`[WARN]` sarı, `[INFO]` yeşil, `[DEBUG]` camgöbeği — Windows'ta `tdf.py
monitor` konsolun ANSI işlemesini açar, böylece renkler görünür). Gerçek,
yeni bir bağımlılık gerektirdi: `pyserial` — düz bir `pip install` yerine
`./install.sh` tarafından ayrı bir `tdf_venv/` içine kuruldu, çünkü
çoğu güncel Python kurulumu (Homebrew, çoğu Linux dağıtımı) artık
sistem geneline pip kurulumlarını doğrudan reddediyor (PEP 668);
`source ./export.sh` o venv'i otomatik olarak etkinleştirir. İlk seferde
tam kurulum:

```sh
# macOS / Linux
./install.sh          # tdf_venv/ oluşturmayı + içine pyserial kurmayı da önerir
source ./export.sh     # bu shell oturumu için tdf_venv/'i etkinleştirir
```

```powershell
# Windows (PowerShell)
.\install.ps1
. .\export.ps1
```

```bat
:: Windows (cmd.exe)
install.bat
export.bat
```

**Bununla, bağlı kartta, gerçekten doğrulanan:** önyükleme satırı ve
sürekli `LED1`/`LED2 toggle @ ... ms` satırları doğru şekilde, yeşil
renkte akıyor, Tutorial 02/03'ün yanıp sönme desenine uyuyor — hem
Tutorial 03'ün hem bu eğitimin kendi belgelerinde daha önce
doğrulanmamış olarak işaretlenmişti, şimdi kapatıldı. **Hâlâ gerçekten
doğrulanmamış olan:** buton-basma davranışının kendisi, gerçek kartta
bir insan parmağı gerektiriyor, bunu buradan yapamıyorum — `tdf.py
monitor` log borusunun çalıştığını kanıtlar, bir basmanın doğru log
satırını ürettiğini değil. Bunu kendiniz kontrol ediyorsanız: `tdf.py
monitor`'ı çalıştırın, sonra USER butonuna basıp basılı tutun — bir
basma için bir sarı `USER button: pressed` satırı (birkaç tane değil),
10 saniyeyi aşan bir tutma için bir yeşil `USER button: long hold
reached (10000 ms)` satırı, ve bırakmada bir sarı `USER button: released
after N ms` satırı görmelisiniz. (Basma/bırakma özellikle `INFO` değil
`WARN` seviyesinde loglanıyor, böylece bunu gerçekten canlı izlerken
sürekli yeşil LED-toggle gürültüsüne karşı sarı renkte öne çıkıyorlar.)

## Adım Adım

### 1. Kart yapılandırması — USER butonunu ekler

[`Sw/Board/board_config.h`](../../../Getting_Started/04-Button_Read_And_Debounce/Sw/Board/board_config.h),
Tutorial 03'ün LED1/LED2 makrolarını korur ve referans
`board_config.h`'den kopyalanan butonun pin haritasını ekler:

```c
#define TIREMO_BOARD_BUTTON_ACTIVE_LOW     (1U)
#define TIREMO_BOARD_BUTTON_COUNT          (1U)

#define TIREMO_BOARD_USER_BUTTON_PORT      TIREMO_HAL_GPIO_PORT_C
#define TIREMO_BOARD_USER_BUTTON_PIN       TIREMO_HAL_GPIO_PIN_9
#define TIREMO_BOARD_USER_BUTTON_IRQ_PRIO  (3U)
```

`TIREMO_BOARD_USER_BUTTON_IRQ_PRIO`, yalnızca bu eğitimin hiç
çağırmadığı `TIREMO_Button_IntrInit()` tarafından kullanılır — belirli
bir çağıran kesmeleri kullansın ya da kullanmasın ona başvuran birebir
sürücü dosyasıyla eşleşmesi için yine de burada tutulur.

### 2. Yeni bir katman: `Drivers/button`

Şimdiye kadarki her eğitim doğrudan `Tiremo_Hal` (GPIO, SysTick, UART)
ya da `Common` (SysTick servisi, debug loglama) ile konuştu. Bu,
`Drivers/`'ı devreye alan ilk eğitim — bir HAL'i genel bir pin yerine
adlandırılmış, belirli bir çevre birimine saran katman.
`Sw/Drivers/button/`, referanstan birebir kopyalanan
`tiremo_button.h`/`.c`'yi barındırır. `main.c`'in kendisi Tutorial
03'ten **değişmedi** — buton hiç satıcı çevre birimi devreye alma
çağrısı gerektirmiyor, Tutorial 01'deki GPIO ile aynı.

`CMakeLists.txt`, tek yeni kaynak dosyayı ve include dizinini ekler:

```cmake
add_executable(ButtonReadAndDebounce
    ...                                     # Tutorial 03'ün sahip olduğu her şey
    Drivers/button/Src/tiremo_button.c
)
target_include_directories(ButtonReadAndDebounce PRIVATE
    ...
    Drivers/button/Include
)
```

### 3. Debounce ve tutma algılama

[`Sw/Src/prv_user_code.c`](../../../Getting_Started/04-Button_Read_And_Debounce/Sw/Src/prv_user_code.c),
Tutorial 03'ün iki-LED döngüsünü birebir korur ve aynı `for(;;)`'e bir
tane daha bağımsız, bloklamayan şey ekler: `PollButton(now)`'a bir çağrı.

```c
static void PollButton(uint32_t now)
{
    static bool stablePressed = false;
    static bool candidatePressed = false;
    static uint32_t candidateSinceMs = 0U;
    static uint32_t pressStartMs = 0U;
    static bool holdLogged = false;
    bool rawPressed = TIREMO_Button_IsPressed(TIREMO_BUTTON_USER);

    if (rawPressed != candidatePressed)
    {
        candidatePressed = rawPressed;   /* ham seviye değişti -- debounce zamanlayıcısını yeniden başlat */
        candidateSinceMs = now;
    }

    if ((candidatePressed != stablePressed) &&
        ((now - candidateSinceMs) >= BUTTON_DEBOUNCE_MS))
    {
        stablePressed = candidatePressed;   /* yeterince uzun süre sabit kaldı -- kabul et */
        if (stablePressed) { pressStartMs = now; holdLogged = false; /* WARN ile "pressed" logla */ }
        else { /* WARN ile "released after N ms" logla */ }
    }

    if (stablePressed && !holdLogged && ((now - pressStartMs) >= BUTTON_HOLD_THRESHOLD_MS))
    {
        holdLogged = true;   /* eşik aşıldığı anda bir kez tetiklenir */
        /* "long hold reached" logla */
    }
}
```

Tutorial 02'deki aynı işaretsiz-çıkarma deyimini kullanan iki bağımsız
zamanlayıcı: `candidateSinceMs`, bir ham okumanın güvenilmeden önce ne
kadar sabit kalması gerektiğini kapılar (`BUTTON_DEBOUNCE_MS`, 30 ms —
tipik mekanik sekmenin rahatça üzerinde, gerçek bir basma süresinin
rahatça altında), ve `pressStartMs` uzun-tutma kontrolünü kapılar
(`BUTTON_HOLD_THRESHOLD_MS`, referansın fabrika sıfırlama sabitiyle
eşleşen 10000 ms). Tutma kontrolü, buton basılı tutulduğu her döngü
yinelemesinde, debounce durum makinesinden bağımsız olarak çalışır, bu
yüzden 10 saniyeye ulaşıldığı anda tetiklenir — nihai bırakmayı beklemez,
referans üretici yazılımındaki `TIREMO_APP_ServiceFactoryResetHold()`'un
kullandığı ile aynı şekil.

### 4. Canlı izleme: `tdf.py monitor`

[`tools/tdf.py`](../../../tools/tdf.py) yeni bir `monitor` alt komutu
kazanıyor. `build`/`flash`/`clean`/`clean-device`'ın aksine, altında
saf CMake devretme değil — kartı resetlemek yine öyle (`cmake --build
build --target reset-device`, her eğitimin `CMakeLists.txt`'ine
`erase-device`'in hemen yanına eklenen bir hedef), ama sonrasında
UART'ı okumak gerçek bir seri port gerektiriyor, ki CMake ve OpenOCD'nin
bu konuda hiçbir kavramı yok. O kısım `pyserial`: kartın seri portunu
otomatik algıla (birden fazla aday çıkarsa `--port`'a düş), 115200
8N1'de aç, ve her satırı `[LEVEL]` önekinden seçilen bir ANSI renk
koduna sararak yazdır:

```python
_LEVEL_COLORS = {
    "[ERROR]": "\033[31m",  # kırmızı
    "[WARN]": "\033[33m",   # sarı
    "[INFO]": "\033[32m",   # yeşil
    "[DEBUG]": "\033[36m",  # camgöbeği
}

def colorize(line):
    color = _LEVEL_COLORS.get(line.split(" ", 1)[0])
    return f"{color}{line}\033[0m" if color else line
```

Önce resetlemek, seri portu açmadan önce, bilinçli bir seçim: her
`tdf.py monitor` oturumunun, kartın zaten yapmakta olduğu bir şeyin
ortasına düşmek yerine, taze bir önyüklemeden başlamasını garanti eder.
Bilinmeye değer, dürüst bir uç durum: canlı bir UART bağlantısını
resetlemek, tam reset sınırında bir satırı bazen ikiye yırtabilir
(test sırasında bir kez gözlemlendi — başıboş, bozuk bir kısmi satır,
hemen ardından 0 ms'ye yakın yeniden başlayan temiz çıktı). Zararsız, ve
bir eğitim aracı için etrafında mühendislik yapmaya değmez.

### 5. Derle, flaşla, izle

```sh
tdf.py build
tdf.py flash
tdf.py monitor
```

`build`/`flash` için önceki her eğitimle aynı işletim sistemi tespiti,
aynı paketlenmiş OpenOCD akışı — bkz.
[01-Hello_Blink'in Çözüm bölümü](../../../Getting_Started/01-Hello_Blink/README.md#solution).
Bu eğitim için gerçek donanımda tam olarak neyin doğrulandığı, ve
`monitor`'ın tam hikayesi için yukarıdaki **Çözüm** bölümüne bakın.

## Ne Zaman Tamamlanmış Sayılır

`tdf.py monitor`, önyükleme satırını ve sürekli `LED1`/`LED2 toggle @
... ms` satırlarını yeşil renkte akıtır — gerçekten doğrulandı, bkz.
**Çözüm**. USER butonuna basıp basılı tutmak şunları eklemeli: her
fiziksel basma için bir sarı `USER button: pressed` satırı (birkaç tane
değil — debounce'un çalıştığının kanıtı), bir tutma 10 saniyeyi aştığı
anda bir yeşil `USER button: long hold reached (10000 ms)` satırı, ve
bırakmada `N`'in kısa bir basmayı uzun bir tutmadan ayırt ettiği bir
sarı `USER button: released after N ms` satırı. Özellikle buton-basma
kısmı benim tarafımdan gerçek bir kartta henüz doğrulanmadı — bkz.
**Çözüm** bölümündeki dürüstlük notu.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
