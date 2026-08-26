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
      <sub>Tiremo Cortex Eğitimleri — ESP32 + AWS IoT üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Ten-LED Pattern Driver

***Kart: Tiremo Cortex (ESP32) · Bölüm: Getting Started / Getting Started · Zorluk: Kolay***

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
- Tutorial 04 ve 05'i birleştirmek: debounce edilmiş bir buton basmasının
  yalnızca bir log satırı üretmek yerine uygulama durumunu sürmesi

## Ön Koşullar

- [04-Button_Read_And_Debounce](../../../Getting_Started/04-Button_Read_And_Debounce/README.md) — bu eğitim onun debounce durum makinesini ve `Drivers/button`'unu birebir yeniden kullanır
- `PATH` üzerinde `arm-none-eabi-gcc`, `cmake` (3.20+) ve `ninja` (bkz. [01-Hello_Blink'in Ön Koşulları](../../../Getting_Started/01-Hello_Blink/README.md#prerequisites))
- Sahne değişikliklerini izlemek için `tdf.py monitor` (kurulum için [04-Button_Read_And_Debounce'un Çözüm bölümüne](../../../Getting_Started/04-Button_Read_And_Debounce/README.md#solution) bakın — `pyserial` gerektiriyor, `./install.sh`'ın `tdf_venv/`'i üzerinden)
- On LED'in tamamına ve USER butonuna erişilebilir, SWD üzerinden bağlı bir ABOV A34G43A Starter Kit

## Desen Motorunu Anlamak (Kopyalanacak Bir Referans Yok)

Önceki her eğitimin yeni modülü, referans üretici yazılımındaki gerçek
bir dosyadan başladı, kırpılmış ya da birebir kopyalanmış. Bu farklı.
[`Sw/Drivers/led/Src/tiremo_led.c`](../../../Getting_Started/05-Ten_LED_Pattern_Driver/Sw/Drivers/led/Src/tiremo_led.c)
— birebir kopyalandı, şimdiye kadarki her `Drivers/` modülü gibi —
yalnızca `TIREMO_LED_On/Off/Toggle/SetState/GetState/AllOn/AllOff`'a
sahip. Ham, anlık, tek seferlik çağrılar. Referans üretici yazılımının
hiçbir yerinde bir `tiremo_led_pattern.c` yok; üretim sisteminin LED
desenleri daha üst seviye shadow state tarafından (bulut tarafında, bu
eğitimin kapsamı dışında ve burada ele alınmıyor) sürülüyor, ve
"2 Hz'de yanıp sön"ü çalışan bir zamanlayıcıya çeviren şey her ne ise, bu
deponun erişimi olduğu kodda yok.

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

[`Sw/`](../../../Getting_Started/05-Ten_LED_Pattern_Driver/Sw/) gerçek, derlenebilir bir uygulamaya sahip:

```sh
cd Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

(`tdf.py`'nin ne olduğu için [01-Hello_Blink'in Çözüm bölümüne](../../../Getting_Started/01-Hello_Blink/README.md#solution),
özellikle `monitor` için [04-Button_Read_And_Debounce'un Çözüm bölümüne](../../../Getting_Started/04-Button_Read_And_Debounce/README.md#solution)
bakın.)

Önceki her eğitimle aynı paylaşılan satıcı SDK'sı ve aynı CMake derleme
şekli. Hiç yeni satıcı kaynağı gerekmedi — on LED'in tamamı, Tutorial
01'den beri `hal_pcu.c` tarafından zaten kapsanan saf GPIO — bu yüzden
yeni derleme girdileri bu eğitimin kendi `Drivers/led/Src/tiremo_led.c`'i
(kopyalandı) ve `Common/Src/tiremo_led_pattern.c`'i (orijinal), Tutorial
04'ün buton kaynaklarının yanına `CMakeLists.txt`'e eklendi.

**Bağlı kartta, gerçekten, gerçek hayatta doğrulandı:** `tdf.py build`
temiz şekilde derlenip bağlanıyor (`.text` 13796 bayt). `tdf.py flash`,
SWD üzerinden tam bir programlama-ve-doğrulama döngüsünü tamamladı
(`** Verified OK **`). Ardından, önce resetleyen `tdf.py monitor`, temiz
bir önyükleme-canlıya log yakaladı:

```text
[INFO] scene -> SOLID @ 3 ms
[INFO] scene -> BLINK (auto) @ 8003 ms
[INFO] scene -> PULSE (auto) @ 16003 ms
```

Her otomatik ilerlemede tam olarak tasarlanan 8000 ms
`SCENE_INTERVAL_MS`, gerçek donanım zaman damgalarından doğrulandı,
varsayılmadı. **Gerçekten doğrulanmamış olan:** on LED'in tamamının
gerçekten görünür şekilde kademeli ve doğru kablolanmış olup olmadığı
(fiziksel kartı göremiyorum), ve gerçek bir USER buton basmasının
sahneyi erken ilerletip `(auto)` yerine `(button)` loglayıp
loglamadığı — bu bir insan parmağı gerektiriyor, Tutorial 04 ile aynı
sınırlama. Bunu kendiniz kontrol ediyorsanız: `tdf.py monitor`'ı
çalıştırın, BLINK ve PULSE sahneleri sırasında on LED'in tamamının
kademelendiğini izleyin (LED_1 en hızlı 300 ms'de, LED_10 en yavaş
1650 ms'de), ve USER butonuna basarak hemen bir sonraki sahneye
atladığını, `scene -> ... (button)` olarak loglandığını doğrulayın.

## Adım Adım

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
tdf.py build
tdf.py flash
tdf.py monitor
```

Önceki her eğitimle aynı işletim sistemi tespiti, aynı paketlenmiş
OpenOCD akışı — bkz. [01-Hello_Blink'in Çözüm bölümü](../../../Getting_Started/01-Hello_Blink/README.md#solution).
Bu eğitim için gerçek donanımda tam olarak neyin doğrulandığı için
yukarıdaki **Çözüm** bölümüne bakın.

## Ne Zaman Tamamlanmış Sayılır

`tdf.py monitor`, 8 saniyelik aralıklarla `scene -> SOLID`, ardından
`scene -> BLINK (auto)`, ardından `scene -> PULSE (auto)` gösterir —
gerçekten doğrulandı, bkz. **Çözüm**. On LED'in tamamı görünür şekilde
açık olmalı (SOLID), ardından on farklı yanıp sönme hızında kademeli
(BLINK), ardından kısa kalp atışı darbeleri olarak kademeli (PULSE), ve
herhangi bir anda bir USER buton basması hemen bir sonraki sahneye
atlamalı, `(auto)` yerine `(button)` olarak loglanmalı. Görsel/fiziksel
kısımlar benim tarafımdan gerçek bir kartta henüz doğrulanmadı — bkz.
**Çözüm** bölümündeki dürüstlük notu.

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
