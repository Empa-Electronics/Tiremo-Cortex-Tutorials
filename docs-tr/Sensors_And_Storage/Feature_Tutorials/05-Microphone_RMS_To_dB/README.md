<p class="sphinxhide" align="right"><a href="../../../../Sensors_And_Storage/Feature_Tutorials/05-Microphone_RMS_To_dB/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>Microphone: RMS Capture to dB</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Microphone: RMS Capture to dB

***Kart: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Bölüm: Sensors & Local Storage / Feature Tutorials · Zorluk: Zor***

> Bu sayfa [İngilizce orijinalinin](../../../../Sensors_And_Storage/Feature_Tutorials/05-Microphone_RMS_To_dB/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Yerel sensör eğitimlerinin en zoru: Timer1 ile tetiklenen bir ADC burst'ü, DMA ile bir tampona aktarım, DC ofset giderme, RMS ve RMS→dB — artı seviyeyi `HIGH_VOICE` / `NORMAL` / `SILENCE` alarmlarına çeviren küçük bir durum makinesi. `Tiremo_Hal` yerine doğrudan satıcının `hal_adc` / `hal_timer1` / `hal_pcu` sürücüleriyle konuşan ilk eğitimdir.

## Neler Öğreneceksiniz

- Burst-DMA yakalamanın neden `Tiremo_Hal_ADC`'de olmadığı (o sarmalayıcı tek bir yoklamalı dönüşümdür)
- ADC tetikleyicisi olarak Timer1, HPL DMA kurma/servis etme, PA0 analog mux
- DC ofset giderme + yüksek geçiren filtre, RMS ve tamsayı dB olarak `20·log10(rms)` (göreli, kalibre edilmiş SPL değil)
- Ses alarmları için histerezisli bir durum makinesi
- Bu SDK'nın `abov_module_config.h` dosyasında `CONFIG_HAL_DMAC`'ın zaten `1` olduğu

## Ön Koşullar

- [04-Battery_Voltage_ADC](../04-Battery_Voltage_ADC/README.md) — aynı ADC çevre birimi (orada tekli mod, burada burst mod)
- Seviyeleri izlemek için: `tdf monitor` (kurulum için [Getting_Started/04-Button_Read_And_Debounce'un Çözüm bölümüne](../../../Getting_Started/04-Button_Read_And_Debounce/README.md#çözüm) bakın) ya da kartın USB seri portunda 115200 8N1'de herhangi bir seri terminal (Tera Term, PuTTY, screen, minicom, …)
- Bilgisayarınıza USB kablosuyla bağlı bir Tiremo Cortex kartı (ABOV A34G43A MCU — bkz. [Donanım Genel Bakışı](../../../Hardware.md)). Kartın üzerindeki CMSIS-DAP hata ayıklayıcısı SWD üzerinden flaşlamayı yapar; ayrı bir programlayıcı gerekmez.

## Sinyal yolu

**MP23ABS1** analog MEMS mikrofonu ve onu yükselten **TS922** op-amp katı, 2,7 V'luk **VDDA** hattından beslenir (bu, [04-Battery_Voltage_ADC](../04-Battery_Voltage_ADC/README.md)'nin ölçtüğü hatla aynıdır). Op-amp çıkışı (`MIC_IN2`), üretici yazılımının **ADC0, kanal 0** üzerinden okuduğu **PA0 / AN0** pinine gider.

## Atlamayı (bypass) anlamak

[`Sw/Drivers/mic/`](../../../../Sensors_And_Storage/Feature_Tutorials/05-Microphone_RMS_To_dB/Sw/Drivers/mic/) referans üretici yazılımından kopyalanmıştır (`tiremo_mic_bsp.c` + `tiremo_mic_app.c` + `tiremo_mic_adc_reg.h`). BSP, `TIREMO_HAL_ADC_ReadChannel()`'ın hiçbir zaman açmadığı ADC/Timer1/DMA register'larına yazar. Bu yüzden `board_config.h` satıcı türlerini (`ADC_ID_0`, `TIMER1_ID_0`, `PCU_ID_A`) kullanır — bu, o başlıkta belirtilmiştir.

`PRV_ADC_Init()`, ADC0'ı kanal 0'da burst + Timer1 tetiklemesiyle yapılandırır; `PRV_Timer1_Init()`, ADC tetik çıkışını etkinleştirir. BSP, Timer1'i **16 kHz** örneklemeye ayarlar, bir DMA tamponuna yakalamayı başlatır ve `TIREMO_MIC_Service()` tamamlanan DMA bloklarını yoklar.

## İşlemeyi anlamak

[`tiremo_mic_app.c`](../../../../Sensors_And_Storage/Feature_Tutorials/05-Microphone_RMS_To_dB/Sw/Drivers/mic/Src/tiremo_mic_app.c) içinde her 12-bit örnek şu adımlardan geçer:

1. **DC ofset:** sabit bir ofset (`TIREMO_MIC_ADC_OFFSET`, `1705` sayım) çıkarılır.
2. **Ses düzeyi ve kazanç:** sinyal giriş ses düzeyiyle (varsayılan 100 üzerinden `80`) ve sabit `8` dijital kazançla ölçeklenir.
3. **Yüksek geçiren filtre:** tek kutuplu bir yüksek geçiren filtre kalan DC'yi temizler; böylece op-amp'in bias noktasındaki küçük farklar önemli olmaz.
4. **RMS**, **1024 örneklik** dairesel bir tampon üzerinden (16 kHz'de 64 ms), tamsayı karekökle hesaplanır.
5. **Zarf:** seviye yükselişleri hızlı (attack ÷2), düşüşleri yavaş (release ÷16) takip eder; böylece kısa tepeler de kaydedilir.

**dB**, `TIREMO_MIC_RmsToDb()`'dir: `20·log10(rms) ≈ 6,0206 · log2(rms)`, ikinin kuvvetleri arasında doğrusal ara değerlemeyle tamsayı olarak. Göreli bir seviyedir, kalibre edilmiş SPL değildir.

**Ses alarmları** — `TIREMO_MIC_OnVoiceAlert()` yalnızca durum değiştiğinde çağrılır:

| Durum | Seviye şu olduğunda girilir… | Seviye şu olduğunda çıkılır… |
|---|---|---|
| `HIGH_VOICE` | art arda 2 tampon boyunca ≥ 1000 | art arda 8 tampon boyunca < 700 (≤ 180 ise → `SILENCE`, değilse `NORMAL`) |
| `SILENCE` | art arda 8 tampon boyunca ≤ 180 | > 450 (→ `NORMAL`), ya da 2 tampon boyunca ≥ 1000 (→ `HIGH_VOICE`) |
| `NORMAL` | aradaki her şey | — |

Giriş ve çıkış eşikleri arasındaki boşluk (histerezis), seviye bir eşiğin etrafında gezinirken alarmın titremesini önler.

## Çözüm

Depo kökünden:

```sh
cd Sensors_And_Storage/Feature_Tutorials/05-Microphone_RMS_To_dB/Sw
tdf build
tdf flash
tdf monitor
```

(`tdf`'nin ne olduğu için [Getting_Started/01-Hello_Blink'in Çözüm bölümüne](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) bakın. macOS/Linux'ta `tdf.py` takma ad olarak da çalışır; Windows'ta `tdf` kullanın. `tdf monitor`'ü çalıştırmadan önce kartın seri portundaki diğer terminalleri kapatın — portu aynı anda yalnızca bir program açabilir.)

**Derleme notları:** [01-Hello_Blink'in Çözüm bölümünde](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) açıklanan beklenen ve zararsız `_close` / `_lseek` / `_read` / `_write is not implemented` linker uyarılarının yanında, bu derleme satıcı ADC HAL başlığından (`hal_adc_v_01_00_05.h`) iki `-Wenum-compare` uyarısı da basar. Bunlar değiştirilmemiş satıcı kodundan gelir ve zararsızdır.

**Gerçek donanımda doğrulandı:** `tdf build`, macOS'ta ve Windows 11'de derlenip bağlanıyor (`.text` yaklaşık 18 KB; kesin boyut toolchain sürümüne göre biraz değişir). `tdf flash`, gerçek, bağlı bir Tiremo Cortex kartına karşı SWD üzerinden tam bir programlama-ve-doğrulama döngüsünü tamamladı (`** Verified OK **`). Açılış satırı ve `MIC: capturing (talk near the mic vs silence)` satırından sonra her saniye bir `MIC: blocks=… rms=… dB=…` durum satırı, durum her değiştiğinde de bir `MIC alert: …` satırı gelir. Gerçek bir karttan — önce sessiz, sonra birkaç kelime, sonra yine sessiz — bir kesit:

```text
[INFO] MIC: blocks=921850  rms=12  dB=21
[INFO] MIC: blocks=930398  rms=10  dB=19
[INFO] MIC: blocks=938991  rms=23  dB=26
[INFO] MIC alert: NORMAL  rms=806  dB=57
[INFO] MIC alert: HIGH_VOICE  rms=1282  dB=61
[INFO] MIC alert: NORMAL  rms=475  dB=53
[INFO] MIC alert: SILENCE  rms=133  dB=42
```

Sessiz bir odada seviye rms 10–40 civarındadır (yaklaşık 20–35 dB); mikrofona yakından konuşmak onu rms 1000–3500'e (yaklaşık 60–70 dB) çıkarır. DMA yakalaması çalıştığı sürece `blocks` artmaya devam eder — durursa yakalama takılmış demektir. Bir alarm satırındaki `rms`, durumun değiştiği andaki seviyedir; bu yüzden tepe değerde değil, bir eşiğin yakınında durur. Mikrofon kurulamazsa log `[ERROR] MIC: init failed` ya da `[ERROR] MIC: start failed` gösterir ve program orada durur.

## Adım Adım

### 1. Kart yapılandırması — satıcı kimlikleri

[`Sw/Board/board_config.h`](../../../../Sensors_And_Storage/Feature_Tutorials/05-Microphone_RMS_To_dB/Sw/Board/board_config.h): `TIREMO_BOARD_MIC_ADC_ID` = `ADC_ID_0`, kanal `0`, `TIREMO_BOARD_MIC_TIMER_ID` = `TIMER1_ID_0`, ve PA0 analog pin mux'ı (`PCU_ID_A`, `PCU_PIN_ID_0`, `PCU_ALT_7`).

### 2. `main.c` — `PRV_ADC_Init()` + `PRV_Timer1_Init()`

[`Sw/Src/main.c`](../../../../Sensors_And_Storage/Feature_Tutorials/05-Microphone_RMS_To_dB/Sw/Src/main.c), `PRV_PORT_Init()`, `PRV_SCU_CLK_Init()`, `PRV_UART_Init()`, `PRV_ADC_Init()` (ADC0 burst + Timer1 tetikleme) ve `PRV_Timer1_Init()`'i (ADC tetik çıkışı), ardından `PRV_USER_Code()`'u çağırır.

### 3. Döngü

[`Sw/Src/prv_user_code.c`](../../../../Sensors_And_Storage/Feature_Tutorials/05-Microphone_RMS_To_dB/Sw/Src/prv_user_code.c), `TIREMO_MIC_Init()` ve `TIREMO_MIC_Start()`'ı çağırır, ardından `TIREMO_MIC_Service()`'i olabildiğince sık çağıran bir döngüde saniyede bir `blocks` / `rms` / `dB` loglar. Ayrıca her `HIGH_VOICE` / `NORMAL` / `SILENCE` değişikliğini loglamak için zayıf (weak) `TIREMO_MIC_OnVoiceAlert()` geri çağırmasını geçersiz kılar.

### 4. Derle, flaşla ve izle

```sh
tdf build
tdf flash
tdf monitor
```

(Ya da tek komutta zincirleyerek: `tdf build flash monitor`.) Çıktının nasıl görünmesi gerektiği için yukarıdaki **Çözüm** bölümüne bakın.

## Ne Zaman Tamamlanmış Sayılır

`tdf monitor`, `blocks`'un her saniye arttığını, oda sessizken düşük bir seviye (rms onlarca, ~20–35 dB) ve bir `SILENCE` alarmı, mikrofona yakından konuştuğunuzda ise rms'si 1000'in üzerinde (~60 dB ya da daha fazla) bir `HIGH_VOICE` alarmı gösterir.


<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
