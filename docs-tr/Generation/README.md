<p class="sphinxhide" align="right"><a href="../../Generation/README.md">English</a> | <a>Türkçe</a></p>

# Generation/ — Ortak Üretici SDK'sı

> Bu sayfa [İngilizce orijinalinin](../../Generation/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

Bu, ABOV Semiconductor'ın A34G43x için AUDK32 SDK'sıdır (CMSIS cihaz
paketi, HAL, linker script ve Linux, macOS ve Windows için paketlenmiş bir
OpenOCD derlemesi: `AUDK32_A34xxxx-1.0.12/Tool/OpenOCD/`). [Referans
firmware](../referance_only/Tiremo-Secure-Iot-Sw/README.md)'in derlendiği
üretici ağacının aynısıdır; tam üretim firmware'inin bunun üzerine neler
eklediğini o projenin kendi README'sinde görebilirsiniz.

## Neden burada, bir kez, ortak olarak duruyor

Bu depoda gerçek, derlenebilir bir `Sw/` çözüm klasörü olan her eğitim bu
tek kopyaya karşı derlenir ve ona göreli yol ile başvurur (genellikle bir
`<Topic>/<Section>/<NN-Tutorial>/Sw/Makefile` içinden
`../../../Generation/AUDK32_A34xxxx-1.0.12`). Her eğitime **kopyalanmaz**:
yaklaşık 107 MB'lık (2000'den fazla dosya, üç önceden derlenmiş OpenOCD
ikilisi dahil) bu ağacı 38 eğitim klasörünün hepsine kopyalamak depoyu
birkaç gigabayta şişirirdi ve hiçbir faydası olmazdı; çünkü her eğitim
aynı kartı ve aynı MCU'yu hedefler.

## Lisans

ABOV'un kendi [`AUDK32_A34xxxx-1.0.12/Document/LICENSE.md`](../../Generation/AUDK32_A34xxxx-1.0.12/Document/LICENSE.md)
dosyası bu ağacın tamamını kapsar: BSD-3-Clause; kaynak ve ikili biçimde
yeniden dağıtıma, atıf korunarak izin verilir. Tek tek kaynak dosyalar da
aynı dosyaya işaret eden ABOV telif başlığını taşır.

## Yeniden üretme / güncelleme

ABOV daha yeni bir AUDK32 SDK sürümü yayınlarsa, içindeki dosyaları elle
düzenlemek yerine bu dizini bütünüyle değiştirin (referans firmware'in
"üretici yapıştırıcısı ince kalır" tasarım ilkesiyle uyumlu). Her eğitimin
`Sw/Makefile`'ı bu ağacın içindeki kararlı göreli yollara bağlıdır, bu tam
SDK sürümüne özgü hiçbir şeye değil.
