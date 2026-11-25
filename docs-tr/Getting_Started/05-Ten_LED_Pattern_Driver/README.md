<p class="sphinxhide" align="right"><a href="../../../Getting_Started/05-Ten_LED_Pattern_Driver/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Ten-LED Pattern Driver</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Ten-LED Pattern Driver

***Kart: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Bölüm: Getting Started · Zorluk: Kolay***

> Bu sayfa [İngilizce orijinalinin](../../../Getting_Started/05-Ten_LED_Pattern_Driver/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Üretim üretici yazılımı LED desenlerini shadow state'ten (buluttan
gelen) seçer, ama desen motorunun kendisi — on tane onboard LED
üzerinde solid/blink/pulse zamanlaması — inşa edilip test edilmek için
buluta ihtiyaç duymaz. Bu eğitim onu bağımsız olarak inşa eder: on
LED'in tamamı aynı üç deseni otomatik olarak döngüye sokar, her biri
görünür şekilde bağımsız olacak biçimde kademelendirilmiş, ve debounce
edilmiş bir USER butonu (Tutorial 04) döngüyü manuel olarak ilerletir.

## Neler Öğreneceksiniz

- Tek bir modülden (`Drivers/led`) on onboard LED'in tamamını sürmek —
  Tutorial 02'den beri kullanılan sadece iki tanesi değil
- Referans LED sürücüsünün neden yalnızca ham on/off/toggle'a sahip
  olduğu, ve solid/blink/pulse desen motorunun neden kopyalanacak bir
  referans uygulamasının olmadığı — bu, üzerine katmanlanmış orijinal
  kod
- Bir paylaşılan `TIREMO_SysTick_GetMs()` üzerinden çalışan on bağımsız
  zamanlayıcıya sahip, bloklamayan, LED-başına bir durum makinesi
- Yeni, genel üretici yazılımı mantığının nereye ait olduğu:
  `Common/`'a, uygulama koduna değil — doğrudan Tutorial 04'ün debounce
  mantığıyla karşılaştırıldı, o mantık `prv_user_code.c`'de kaldı çünkü
  tek bir çağırana özgüydü
- Tutorial 04'ün debounce edilmiş butonunu bu eğitimin desen motoruyla
  birleştirmek: bir buton basmasının yalnızca bir log satırı üretmek
  yerine uygulama durumunu sürmesi

## Ön Koşullar

- [04-Button_Read_And_Debounce](../04-Button_Read_And_Debounce/README.md) — bu eğitim onun debounce durum makinesini ve `Drivers/button`'unu birebir yeniden kullanır
- `PATH` üzerinde `arm-none-eabi-gcc`, `cmake` (3.20+) ve `ninja` (bkz. [01-Hello_Blink'in Ön Koşulları](../01-Hello_Blink/README.md#ön-koşullar))
- Sahne değişikliklerini log'da izlemek için: `tdf monitor` (kurulum için [04-Button_Read_And_Debounce'un Çözüm bölümüne](../04-Button_Read_And_Debounce/README.md#çözüm) bakın — `pyserial` gerektiriyor, `./install.sh` / `install.ps1` / `install.bat`'ın oluşturduğu `tdf_venv/` üzerinden), ya da kartın USB seri portunda 115200 8N1'de herhangi bir seri terminal (Tera Term, PuTTY, screen, minicom, …). LED'lerin kendisi terminal olmadan da çalışır.
- Bilgisayarınıza USB kablosuyla bağlı, on LED'in tamamına ve USER butonuna erişilebilen bir Tiremo Cortex kartı (ABOV A34G43A MCU — bkz. [Donanım Genel Bakışı](../../Hardware.md)). Kartın üzerindeki CMSIS-DAP hata ayıklayıcısı SWD üzerinden flaşlamayı yapar; ayrı bir programlayıcı gerekmez.

## Desen Motorunu Anlamak (Kopyalanacak Bir Referans Yok)

Önceki her eğitimin yeni modülü, referans üretici yazılımındaki gerçek
bir dosyadan başladı, kırpılmış ya da birebir kopyalanmış. Bu farklı.
[`Sw/Drivers/led/Src/tiremo_led.c`](../../../Getting_Started/05-Ten_LED_Pattern_Driver/Sw/Drivers/led/Src/tiremo_led.c)
— birebir kopyalandı, şimdiye kadarki her `Drivers/` modülü gibi —
yalnızca `TIREMO_LED_On/Off/Toggle/SetState/GetState/AllOn/AllOff`'a
sahip. Ham, anlık, tek seferlik çağrılar. Referans üretici yazılımının
hiçbir yerinde bir `tiremo_led_pattern.c` yok; üretim sisteminin LED
desenleri daha üst seviye shadow state tarafından (bulut tarafında, bu
eğitimin kapsamı dışında ve burada ele alınmıyor) sürülüyor; bu yüzden
"2 Hz'de yanıp sön"ü çalışan bir zamanlayıcıya çeviren zamanlama katmanı
bu eğitimde sıfırdan inşa ediliyor.

Bu yüzden
[`Sw/Common/Src/tiremo_led_pattern.c`](../../../Getting_Started/05-Ten_LED_Pattern_Driver/Sw/Common/Src/tiremo_led_pattern.c)
orijinal: LED-başına bir durum makinesi — desen türü, aralık, son
değişim zaman damgası, mevcut açık/kapalı faz — 02-SysTick_Timing'den
beri her eğitimin kullandığı aynı `(now - last) >= interval` deyimiyle,
her döngü tikinde bloklamadan servis edilir, sadece aynı anda çalışan
bir ya da iki tane yerine on bağımsız kopyası.

**Nerede yaşadığı, ve bunun neden rastgele değil gerçek bir karar
olduğu:** Tutorial 04'ün buton-debounce durum makinesi
`prv_user_code.c` içinde — uygulama kodunda — kaldı, çünkü o tek
eğitimin döngüsünün ihtiyaç duyduğu şeye özgüydü. Bu desen motoru
bunun yerine `Common/`'a gidiyor, `tiremo_debug.c` ve
`tiremo_systick.c`'nin yanına, çünkü genel: gelecekteki herhangi bir
çağıran, o LED'de şu anda ne çalıştığını bilmeye ya da önemsemeye
gerek kalmadan `TIREMO_LEDPattern_Set()`'i çağırabilir, tıpkı herhangi
bir çağıranın altındaki UART hakkında hiçbir şey bilmeden
`TIREMO_LOG_INFO()`'ya başvurabilmesi gibi. `Common/` burada belirleyici
faktör — bir donanım sürücüsü üzerinde yeniden kullanılabilir servis
katmanı, tek bir çağıran için tek seferlik mantığa karşı.

**PWM yok, bu yüzden "pulse" bir fade değil.** `TIREMO_HAL_GPIO`
yalnızca dijital — üzerine gerçek bir fade in/out inşa etmek için bir
karartma ilkeli yok. Buradaki `PULSE`, `BLINK`'in eşit 50/50
bölünmesinden farklı, kısa parlak bir patlama (periyodun %20'si)
ardından uzun bir dinlenme anlamına geliyor — bir parlaklık rampası
değil, bir duty-cycle kalp atışı. PWM özellikli LED kanallarına sahip
donanımda "pulse"ın ne anlama geleceğini varsaymadan önce bilinmeye
değer.

## Çözüm

[`Sw/`](../../../Getting_Started/05-Ten_LED_Pattern_Driver/Sw/) gerçek, derlenebilir bir uygulamaya sahip. Depo kökünden:

```sh
cd Getting_Started/05-Ten_LED_Pattern_Driver/Sw
tdf build
tdf flash
tdf monitor
```

(`tdf`'nin ne olduğu için [01-Hello_Blink'in Çözüm bölümüne](../01-Hello_Blink/README.md#çözüm),
özellikle `monitor` için [04-Button_Read_And_Debounce'un Çözüm bölümüne](../04-Button_Read_And_Debounce/README.md#çözüm)
bakın. macOS/Linux'ta `tdf.py` takma ad olarak da çalışır; Windows'ta
`tdf` kullanın. `tdf monitor`'ü çalıştırmadan önce kartın seri portundaki
diğer terminalleri kapatın — portu aynı anda yalnızca bir program
açabilir.)

Önceki her eğitimle aynı paylaşılan satıcı SDK'sı ve aynı CMake derleme
şekli. Hiç yeni satıcı kaynağı gerekmedi — on LED'in tamamı, Tutorial
01'den beri `hal_pcu.c` tarafından zaten kapsanan saf GPIO — bu yüzden
yeni derleme girdileri bu eğitimin kendi `Drivers/led/Src/tiremo_led.c`'i
(kopyalandı) ve `Common/Src/tiremo_led_pattern.c`'i (orijinal), Tutorial
04'ün buton kaynaklarının yanına `CMakeLists.txt`'e eklendi.

**Gerçek donanımda doğrulandı:** `tdf build`, macOS'ta ve Windows 11'de
hatasız derlenip bağlanıyor (`.text` yaklaşık 13,5 KB; kesin boyut
toolchain sürümüne göre biraz değişir, ve linker
[01-Hello_Blink'in Çözüm bölümünde](../01-Hello_Blink/README.md#çözüm)
açıklanan beklenen ve zararsız `_close` / `_lseek` / `_read` / `_write is
not implemented` uyarılarını basar). `tdf flash`, gerçek, bağlı bir
Tiremo Cortex kartına karşı SWD üzerinden tam bir programlama-ve-doğrulama
döngüsünü tamamladı (`** Verified OK **`) ve taze bir önyüklemeden
itibaren log şöyle görünür:

```text
[INFO] Ten_LED_Pattern_Driver: boot
[INFO] scene -> SOLID @ 3 ms
[INFO] scene -> BLINK (auto) @ 8003 ms
[INFO] scene -> PULSE (auto) @ 16003 ms
[INFO] scene -> SOLID (auto) @ 24003 ms
```

(PULSE'tan sonra döngü SOLID'e geri döner ve her 8 saniyede bir tekrar eder.)

Zaman damgaları, her otomatik ilerleme arasında tam olarak tasarlanan
8000 ms `SCENE_INTERVAL_MS`'i gösterir. **Kartta ne görmelisiniz:** SOLID
sırasında on LED'in tamamı yanar; BLINK ve PULSE sırasında on farklı
hızda kademeli olarak yanıp söner (LED_1 en hızlı, 300 ms periyotla;
LED_10 en yavaş, 1650 ms ile). Herhangi bir anda USER butonuna basarsanız
kart hemen bir sonraki sahneye geçer: sarı bir `[WARN] USER button:
pressed` satırını sarı bir `[WARN] scene -> ... (button) @ ... ms`
satırı izler ve 8 saniyelik otomatik ilerleme zamanlayıcısı o andan
itibaren yeniden başlar.

## Adım Adım İnceleme

### 1. Kart yapılandırması — LED bankasının geri kalanı

[`Sw/Board/board_config.h`](../../../Getting_Started/05-Ten_LED_Pattern_Driver/Sw/Board/board_config.h),
Tutorial 04'ün LED1/LED2/buton makrolarını korur ve
`TIREMO_BOARD_LED3_PORT`/`PIN`'den `LED10`'a kadar, artı
`TIREMO_BOARD_LED_COUNT`'u (10) ekler, referans `board_config.h`'den
kopyalandı. `Drivers/led/Src/tiremo_led.c` doğrudan bu sayıdan
indeksliyor, bu yüzden onu doğru almak, `TIREMO_LED_Init()`'in sessizce
yalnızca ikisi yerine on pinin tamamını devreye almasını sağlayan şey.

### 2. Desen motoru

Yukarıda tam olarak ele alındı — bkz. **Desen Motorunu Anlamak**. Kısa
versiyon, genel API'si:

```c
void TIREMO_LEDPattern_Init(void);
void TIREMO_LEDPattern_Set(TIREMO_LED_Id_t ledId, TIREMO_LEDPattern_Type_t pattern, uint32_t intervalMs, uint32_t now);
void TIREMO_LEDPattern_Service(uint32_t now);
```

`Set()` hemen etkiye giriyor (OFF/SOLID pini hemen yazıyor; BLINK/PULSE
ilk fazını `now`'dan zamanlanmış olarak başlatıyor, bir ilk-tik sentinel
hilesinden kaçınarak). `Service()`, her döngü yinelemesinde on LED'in
tamamını dolaşıyor; OFF/SOLID orada hiçbir şeye mal olmuyor, yalnızca
BLINK/PULSE tik-başına zamanlama işi yapıyor.

### 3. Demo: kademeli sahneler, otomatik ve manuel ilerleme

[`Sw/Src/prv_user_code.c`](../../../Getting_Started/05-Ten_LED_Pattern_Driver/Sw/Src/prv_user_code.c),
Tutorial 02/03/04'ün özel LED1/LED2 döngüsünü emekliye ayırıyor —
şimdi onlar on LED'in arasında sadece `TIREMO_LED_1`/`_2`. `ApplyScene()`
her LED'i aynı desen türüne ama farklı bir aralığa ayarlıyor:

```c
for (index = 0U; index < TIREMO_BOARD_LED_COUNT; index++)
{
    intervalMs = PATTERN_BASE_INTERVAL_MS + (index * PATTERN_STEP_MS); /* 300, 450, ..., 1650 */
    TIREMO_LEDPattern_Set((TIREMO_LED_Id_t)index, pattern, intervalMs, now);
}
```

böylece BLINK/PULSE sahneleri, on LED'in tamamı mükemmel bir uyum
içinde değişmek yerine kart boyunca görünür şekilde kademeleniyor —
gösterilmek istenen nokta, *bağımsız olarak adreslenebilir*
oldukları, ve senkronize bir yanıp sönmenin, bir LED'in on kez
kablolanmasından farklı görünmeyeceği. Ana döngü sahneyi her
`SCENE_INTERVAL_MS`'de (8000 ms) otomatik olarak ilerletiyor, ve ayrı
olarak, `PollButton()` — Tutorial 04'ün tam debounce durum makinesi,
şimdi taze bir debounce edilmiş basmayı algıladığı tikte `true`
döndürüyor — gerçek bir buton basmasında hemen ilerletiyor, her
durumda otomatik-ilerleme zamanlayıcısını sıfırlayarak:

```c
if ((now - sceneLastMs) >= SCENE_INTERVAL_MS)         { /* ilerlet, "(auto)" logla */ }
if (PollButton(now))                                    { /* ilerlet, "(button)" logla */ }
```

Tutorial 02–04'ün aksine, burada bireysel LED toggle'ları loglanmıyor —
aynı anda çalışan on bağımsız zamanlı LED ile, toggle-başına loglama
UART'ı sel basar ve sahne değişikliklerini izleme amacını boşa
çıkarırdı. Yalnızca sahne geçişleri loglanıyor, her biri tetikleyicisiyle
(`auto` ya da `button`) ve bir milisaniye zaman damgasıyla birlikte.

### 4. Derle, flaşla, izle

```sh
tdf build
tdf flash
tdf monitor
```

(Ya da tek komutta zincirleyerek: `tdf build flash monitor`.) Önceki her
eğitimle aynı işletim sistemi tespiti, aynı paketlenmiş OpenOCD akışı —
bkz. [01-Hello_Blink'in Çözüm bölümü](../01-Hello_Blink/README.md#çözüm).
Çıktının nasıl görünmesi gerektiği için yukarıdaki **Çözüm** bölümüne
bakın.

## Tamamlanma Ölçütü

`tdf monitor`, 8 saniyelik aralıklarla `scene -> SOLID`, ardından
`scene -> BLINK (auto)`, ardından `scene -> PULSE (auto)` gösterir ve
sonra `SOLID (auto)`'ya geri döner. On LED'in tamamı görünür şekilde
açıktır (SOLID), ardından on farklı yanıp sönme hızında kademelenir
(BLINK), ardından kısa kalp atışı darbeleri olarak kademelenir (PULSE),
ve herhangi bir anda bir USER buton basması hemen bir sonraki sahneye
atlar, `(auto)` yerine `(button)` olarak loglanır.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
