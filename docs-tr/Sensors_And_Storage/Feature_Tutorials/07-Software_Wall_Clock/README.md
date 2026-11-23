<p class="sphinxhide" align="right"><a href="../../../../Sensors_And_Storage/Feature_Tutorials/07-Software_Wall_Clock/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>Software Wall Clock</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Software Wall Clock

***Kart: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Bölüm: Sensors & Local Storage / Feature Tutorials · Zorluk: Orta***

> Bu sayfa [İngilizce orijinalinin](../../../../Sensors_And_Storage/Feature_Tutorials/07-Software_Wall_Clock/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Bu karttaki A34G43A'nın pille desteklenen bir donanım RTC'si yoktur; bu yüzden duvar saati zamanı yazılımla hesaplanır: `epoch_at_sync + (SysTick_now - SysTick_at_sync) / 1000`. Bu eğitim bilinen bir epoch'u elle yerleştirir; [NTP Time Sync](../../ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/README.md) eğitimi daha sonra aynı API'ye gerçek bir NTP zamanı verir.

## Neler Öğreneceksiniz

- Bir senkronizasyon noktası ile milisaniyelik SysTick sayacının birlikte nasıl çalışan bir duvar saati verdiği
- Yazılım saatinin ödünleşimleri: yalnızca RAM'dedir (her yeniden başlatma senkronsuz başlar), CPU saatiyle birlikte kayar ve 32-bit milisaniye sayacı taşar — bu yüzden düzenli olarak yeniden senkronize edilmesi gerekir
- [`Sw/Common/Src/tiremo_clock.c`](../../../../Sensors_And_Storage/Feature_Tutorials/07-Software_Wall_Clock/Sw/Common/Src/tiremo_clock.c) referans üretici yazılımından kopyalanmıştır

## Ön Koşullar

- Kavramsal olarak [06-Flash_Backed_Config](../06-Flash_Backed_Config/README.md); bu `Sw/` yalnızca SysTick + UART kullanır
- Log'u izlemek için: `tdf monitor` (kurulum için [Getting_Started/04-Button_Read_And_Debounce'un Çözüm bölümüne](../../../Getting_Started/04-Button_Read_And_Debounce/README.md#çözüm) bakın) ya da kartın USB seri portunda 115200 8N1'de herhangi bir seri terminal (Tera Term, PuTTY, screen, minicom, …)
- Bilgisayarınıza USB kablosuyla bağlı bir Tiremo Cortex kartı (ABOV A34G43A MCU — bkz. [Donanım Genel Bakışı](../../../Hardware.md)). Kartın üzerindeki CMSIS-DAP hata ayıklayıcısı SWD üzerinden flaşlamayı yapar; ayrı bir programlayıcı gerekmez.

## Ofseti anlamak

[`tiremo_clock.c`](../../../../Sensors_And_Storage/Feature_Tutorials/07-Software_Wall_Clock/Sw/Common/Src/tiremo_clock.c) tek bir senkronizasyon noktası saklar ve o andan beri geçen süreyi ekler:

```c
void TIREMO_Clock_SetEpoch(uint32_t epochSeconds)
{
    s_epochAtSync = epochSeconds;
    s_msAtSync = TIREMO_SysTick_GetMs();
    s_synced = true;
}

/* TIREMO_Clock_GetEpoch() içinde: */
elapsedMs = TIREMO_SysTick_GetMs() - s_msAtSync;
*pEpochSeconds = s_epochAtSync + (elapsedMs / 1000U);
```

`GetEpoch()`, `SetEpoch()` bir kez çağrılana kadar `false` döndürür. Burada NTP yoktur: [`prv_user_code.c`](../../../../Sensors_And_Storage/Feature_Tutorials/07-Software_Wall_Clock/Sw/Src/prv_user_code.c) açılışta bir kez `SetEpoch(1704067200)` (2024-01-01 00:00:00 UTC) çağırır, ardından her saniye `GetEpoch()`'u loglar.

## Yazılım saatinin sınırları

Bu yöntem basittir ve zaman damgaları için yeterlidir; yeter ki şu iki sınırı akılda tutun:

- **Kayma.** CPU saati (dolayısıyla SysTick), çipin **dahili RC osilatörüyle (HSI)** beslenen PLL'den gelir — bu kartta XIN/XOUT'ta kristal yoktur. Bir RC osilatör kristalden çok daha az hassastır ve sıcaklıkla değişir; bu yüzden saat gerçek zamandan yavaş yavaş uzaklaşır. Bir fikir vermek için: %0,5'lik bir hata günde yaklaşık 7 dakikadır. Tek bir senkronizasyona güvenmek yerine periyodik olarak NTP'den yeniden senkronize edin.
- **Sayaç taşması.** `TIREMO_SysTick_GetMs()`, 2³² ms ≈ **49,7 gün** sonra taşan 32-bit bir milisaniye sayacıdır. İşaretsiz çıkarma tek bir taşmayı doğru ele alır; ama son `SetEpoch()`'tan bu yana 49,7 günden fazla geçerse geçen süre sıfıra döner ve saat geri atlar. Bu pencerenin rahatça içinde (örneğin günlük) yeniden senkronize etmek iki sorunu da önler.

## Çözüm

Depo kökünden:

```sh
cd Sensors_And_Storage/Feature_Tutorials/07-Software_Wall_Clock/Sw
tdf build
tdf flash
tdf monitor
```

(`tdf`'nin ne olduğu için [Getting_Started/01-Hello_Blink'in Çözüm bölümüne](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) bakın. macOS/Linux'ta `tdf.py` takma ad olarak da çalışır; Windows'ta `tdf` kullanın. `tdf monitor`'ü çalıştırmadan önce kartın seri portundaki diğer terminalleri kapatın — portu aynı anda yalnızca bir program açabilir.)

**Gerçek donanımda doğrulandı:** `tdf build`, macOS'ta ve Windows 11'de hatasız derlenip bağlanıyor (`.text` yaklaşık 12 KB; kesin boyut toolchain sürümüne göre biraz değişir, ve linker [01-Hello_Blink'in Çözüm bölümünde](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) açıklanan beklenen ve zararsız `_close` / `_lseek` / `_read` / `_write is not implemented` uyarılarını basar). `tdf flash`, gerçek, bağlı bir Tiremo Cortex kartına karşı SWD üzerinden tam bir programlama-ve-doğrulama döngüsünü tamamladı (`** Verified OK **`). Log:

```text
[INFO] Software_Wall_Clock: boot
[INFO] clock: SetEpoch(1704067200)
[INFO] clock: epoch=1704067201  elapsed=1 s
[INFO] clock: epoch=1704067202  elapsed=2 s
[INFO] clock: epoch=1704067203  elapsed=3 s
```

her saniye yeni bir satır gelir; `elapsed`, `SetEpoch()`'tan bu yana geçen saniye sayısıdır. Saat herhangi bir senkronizasyondan önce okunsaydı log bunun yerine `[WARN] clock: not synced` gösterirdi.

## Adım Adım

### 1. `tiremo_clock.c`

[`Sw/Common/Src/tiremo_clock.c`](../../../../Sensors_And_Storage/Feature_Tutorials/07-Software_Wall_Clock/Sw/Common/Src/tiremo_clock.c): `TIREMO_Clock_SetEpoch()`, `TIREMO_Clock_GetEpoch()` ve `TIREMO_Clock_IsSynced()` — [Getting_Started/02-SysTick_Timing](../../../Getting_Started/02-SysTick_Timing/README.md)'deki `TIREMO_SysTick_GetMs()` üzerine kurulu üç fonksiyon ve üç statik değişken.

### 2. `prv_user_code.c`

[`Sw/Src/prv_user_code.c`](../../../../Sensors_And_Storage/Feature_Tutorials/07-Software_Wall_Clock/Sw/Src/prv_user_code.c): açılışta bir kez `SetEpoch(1704067200)`, ardından `epoch` ve `elapsed` değerlerini loglayan 1 saniyelik bloklamayan bir döngü.

### 3. Derle, flaşla ve izle

```sh
tdf build
tdf flash
tdf monitor
```

(Ya da tek komutta zincirleyerek: `tdf build flash monitor`.) Çıktının nasıl görünmesi gerektiği için yukarıdaki **Çözüm** bölümüne bakın.

## Ne Zaman Tamamlanmış Sayılır

`SetEpoch(1704067200)` satırından sonra `tdf monitor`, `epoch`'un her saniye bir arttığını ve `elapsed`'in 1, 2, 3, … diye saydığını gösterir — ve gerçek bir ürünün bu saati neden periyodik olarak yeniden senkronize etmesi gerektiğini (kayma ve 49,7 günlük taşma) açıklayabilirsiniz.


<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
