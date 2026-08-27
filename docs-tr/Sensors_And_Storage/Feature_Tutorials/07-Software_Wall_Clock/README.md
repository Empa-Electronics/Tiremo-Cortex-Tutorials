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


> Bu sayfa [İngilizce orijinalinin](../../../../Sensors_And_Storage/Feature_Tutorials/07-Software_Wall_Clock/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

# Software Wall Clock

***Kart: Tiremo Cortex (ESP32) · Bölüm: Sensors & Local Storage / Feature Tutorials · Zorluk: Orta***

## Giriş

A34G43x'te donanım RTC yok. Duvar saati `epoch_at_sync + (SysTick_now - SysTick_at_sync) / 1000`.

## Çözüm

`tiremo_clock.c` referanstan kopyalandı. **Doğrulandı:** `.text` 12344, flaş OK, epoch saniyede bir artıyor.

```text
[INFO] clock: epoch=1704067201  elapsed=1 s
[INFO] clock: epoch=1704067202  elapsed=2 s
```


<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
