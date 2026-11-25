<p class="sphinxhide" align="right"><a href="../../../../../Intelligence/Design_Tutorials/03-Edge_Deployment_HMR/Datasets/README.md">English</a> | <a>Türkçe</a></p>

# HMR 3 eksenli veri setleri

> Bu sayfa [İngilizce orijinalinin](../../../../../Intelligence/Design_Tutorials/03-Edge_Deployment_HMR/Datasets/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

Aynı kayıt, üç örnekleme hızı. Her dosyanın başlığı:

```text
acc_x,acc_y,acc_z,class
```

Değerler mili-g cinsindendir. Sınıflar: `CIRCLE`, `HORIZONTAL`, `STANDBY`, `TRIANGLE`, `VERTICAL`. 8 Hz, 128 Hz'in her 16. örneğinin alınmasıyla; 4 Hz her 32. örneğin alınmasıyla elde edilir. Üçünün de gerçek süresi yaklaşık 58,6 dakikadır.

| Dosya | Hız | Satır |
|---|---|---:|
| `Dataset_HMR_3Axis_TiremoCortex_128Hz.csv` | 128 Hz | 450 048 |
| `Dataset_HMR_3Axis_TiremoCortex_8Hz.csv` | 8 Hz | 28 128 |
| `Dataset_HMR_3Axis_TiremoCortex_4Hz.csv` | 4 Hz | 14 064 |

Neden var oldukları ve Edge pencere boyutu 8 için hangisinin içe aktarılacağı [design eğitiminin README'sinde](../README.md) anlatılır.
