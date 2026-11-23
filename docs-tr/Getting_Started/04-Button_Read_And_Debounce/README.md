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

***Kart: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Bölüm: Getting Started · Zorluk: Kolay***

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
- `tdf monitor`'ün nasıl çalıştığı — kartı SWD üzerinden resetleyen ve
  UART log'unu seviyeye göre renk kodlu olarak akıtan depo aracı komutu;
  böylece buton olaylarını canlı izleyebilirsiniz

## Ön Koşullar

- [03-Debug_UART_Logging](../../../Getting_Started/03-Debug_UART_Logging/README.md) — bu eğitimin LED döngüsü ve loglaması, buton işleme eklenmiş Tutorial 03'ün ta kendisi
- `PATH` üzerinde `arm-none-eabi-gcc`, `cmake` (3.20+) ve `ninja` (bkz. [01-Hello_Blink'in Ön Koşulları](../../../Getting_Started/01-Hello_Blink/README.md#prerequisites))
- Basma/bırakma/tutma log satırlarını görmek için: ya `tdf monitor`
  (aşağıdaki **Çözüm**'e bakın — `pyserial` gerektiriyor, `./install.sh` /
  `install.ps1` / `install.bat` bunu ayrı bir `tdf_venv/` içinde kurar), ya
  da 115200 8N1'de herhangi bir seri terminal (Tera Term, PuTTY, screen,
  minicom, …). Ayrı bir USB-seri adaptör gerekmez: kartın üzerindeki
  CMSIS-DAP hata ayıklayıcısı aynı USB kablosu üzerinden bir seri port
  sağlar (Windows'ta örneğin `USB Serial Device (COMx)`).
- Bilgisayarınıza USB kablosuyla bağlı, USER butonuna erişilebilen bir Tiremo Cortex kartı (ABOV A34G43A MCU — bkz. [Donanım Genel Bakışı](../../Hardware.md)). Kartın üzerindeki CMSIS-DAP hata ayıklayıcısı SWD üzerinden flaşlamayı yapar; ayrı bir programlayıcı gerekmez.

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

[`Sw/`](../../../Getting_Started/04-Button_Read_And_Debounce/Sw/) gerçek, derlenebilir bir uygulamaya sahip. Depo kökünden:

```sh
cd Getting_Started/04-Button_Read_And_Debounce/Sw
tdf build
tdf flash
```

(`tdf`'nin ne olduğu ve `./install.sh` + `source ./export.sh` — Windows'ta
`install.ps1` + `. .\export.ps1` ya da `install.bat` + `export.bat` — ile
nasıl `PATH`'e ekleneceği için [01-Hello_Blink'in Çözüm bölümüne](../01-Hello_Blink/README.md#çözüm)
bakın. macOS/Linux'ta `tdf.py` takma ad olarak da çalışır; Windows'ta
`tdf` kullanın.)

Önceki her eğitimle aynı paylaşılan satıcı SDK'sı ve aynı CMake derleme
şekli ([`Generation/`](../../../Generation/), depo kökünde). Hiç yeni
satıcı kaynağı gerekmedi — buton, Tutorial 01'den beri `hal_pcu.c`
tarafından zaten kapsanan saf bir GPIO — bu yüzden tek yeni derleme
girdisi, bu eğitimin kendi `Drivers/button/Src/tiremo_button.c`'i,
Tutorial 03'ün UART/debug kaynaklarının yanına `CMakeLists.txt`'e
eklendi.

**Gerçek donanımda doğrulandı:** `tdf build`, macOS'ta ve Windows 11'de
hatasız derlenip bağlanıyor (`.text` yaklaşık 13 KB; kesin boyut
toolchain sürümüne göre biraz değişir, ve linker
[01-Hello_Blink'in Çözüm bölümünde](../01-Hello_Blink/README.md#çözüm)
açıklanan beklenen ve zararsız `_close` / `_lseek` / `_read` / `_write is
not implemented` uyarılarını basar). `tdf flash`, gerçek, bağlı bir
Tiremo Cortex kartına karşı SWD üzerinden tam bir programlama-ve-doğrulama
döngüsünü tamamladı (`** Verified OK **`). Log'u izlemek için çalıştırın:

```sh
tdf monitor
```

Bu, kartı SWD üzerinden resetler (`reset-device` CMake hedefi,
`init; reset run; shutdown` — aynı OpenOCD, yeniden programlama yok) ve
ardından log'u seviyeye göre renk kodlu olarak akıtır (`[ERROR]` kırmızı,
`[WARN]` sarı, `[INFO]` yeşil, `[DEBUG]` camgöbeği — Windows'ta `tdf
monitor` konsolun ANSI işlemesini açar, böylece renkler görünür). Çıkmak
için Ctrl+C. Seri portu aynı anda yalnızca bir program açabilir, bu
yüzden önce o porttaki diğer terminalleri (Tera Term, PuTTY, …) kapatın.
`monitor`, `pyserial` gerektirir; bu, düz bir `pip install` yerine kurulum
betikleri tarafından ayrı bir `tdf_venv/` içine kurulur, çünkü çoğu güncel
Python kurulumu (Homebrew, çoğu Linux dağıtımı) sistem geneline pip
kurulumlarını reddediyor (PEP 668); `export` o venv'i otomatik olarak
etkinleştirir. İlk seferde tam kurulum:

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

**Ne görmelisiniz:** önyükleme satırı ve sürekli `LED1`/`LED2 toggle @
... ms` satırları yeşil renkte, Tutorial 02/03'ün yanıp sönme desenine
uygun şekilde akar. USER butonuna basınca her fiziksel basma için tam
olarak bir sarı `[WARN] USER button: pressed` satırı (birkaç tane değil),
bırakınca bir sarı `[WARN] USER button: released after N ms` satırı
gelir. 10 saniyeden uzun basılı tutarsanız eşik aşıldığı anda yeşil bir
`[INFO] USER button: long hold reached (10000 ms)` satırı görünür;
bırakma satırı da `(was a long hold)` ile biter. Örneğin gerçek bir
kartta iki kısa basma:

```text
[INFO] LED1 toggle @ 37003 ms
[WARN] USER button: pressed
[INFO] LED1 toggle @ 38003 ms
[WARN] USER button: released after 1821 ms
[INFO] LED1 toggle @ 39003 ms
```

Basma/bırakma `WARN`, uzun tutma satırı `INFO` seviyesinde loglanır:
basma/bırakma kenarları sürekli yeşil LED-toggle satırlarına karşı sarı
renkte öne çıkar. İpucu: LED satırlarını tamamen gizlemek için
`tdf build -DTIREMO_DEBUG_LEVEL=2` ile derleyin — yalnızca `WARN`
basma/bırakma satırları kalır (o seviyede `INFO` uzun-tutma satırı da
derlemeden çıkarılır).

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

### 4. Canlı izleme: `tdf monitor`

[`tools/tdf.py`](../../../tools/tdf.py) bir `monitor` alt komutuna
sahiptir. `build`/`flash`/`clean`/`clean-device`'ın aksine, altında
saf CMake devretme değil — kartı resetlemek yine öyle (`cmake --build
build --target reset-device`, her eğitimin `CMakeLists.txt`'inde
`erase-device`'in hemen yanında tanımlı bir hedef), ama sonrasında
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

Sıra bilinçlidir: `monitor` önce seri portu açar ve bilgisayarda zaten
tamponlanmış eski baytları atar, *sonra* kartı resetler. Böylece her
`tdf monitor` oturumu taze bir önyüklemeden başlar ve port açılırken
önyükleme satırı kaybolmaz. Bilinmeye değer bir uç durum: canlı bir UART
bağlantısını resetlemek, tam reset sınırında bir satırı bazen ikiye
yırtabilir (başıboş, bozuk bir kısmi satır, hemen ardından 0 ms'ye yakın
yeniden başlayan temiz çıktı). Zararsızdır.

### 5. Derle, flaşla, izle

```sh
tdf build
tdf flash
tdf monitor
```

(Ya da tek komutta zincirleyerek: `tdf build flash monitor`.)
`build`/`flash` için önceki her eğitimle aynı işletim sistemi tespiti,
aynı paketlenmiş OpenOCD akışı — bkz.
[01-Hello_Blink'in Çözüm bölümü](../01-Hello_Blink/README.md#çözüm).
Çıktının nasıl görünmesi gerektiği için yukarıdaki **Çözüm** bölümüne
bakın.

## Ne Zaman Tamamlanmış Sayılır

`tdf monitor`, önyükleme satırını ve sürekli `LED1`/`LED2 toggle @
... ms` satırlarını yeşil renkte akıtır. USER butonuna basıp basılı
tutmak şunları ekler: her fiziksel basma için bir sarı `USER button:
pressed` satırı (birkaç tane değil — debounce'un çalıştığının kanıtı),
bir tutma 10 saniyeyi aştığı anda bir yeşil `USER button: long hold
reached (10000 ms)` satırı, ve bırakmada `N`'in kısa bir basmayı uzun bir
tutmadan ayırt ettiği bir sarı `USER button: released after N ms` satırı.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
