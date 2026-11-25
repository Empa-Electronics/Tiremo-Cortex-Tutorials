<p class="sphinxhide" align="right"><a href="../FAQs.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../images/banner.png" width="100%">
      <h1>Sıkça Sorulan Sorular</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Sıkça Sorulan Sorular

> Bu sayfa [İngilizce orijinalinin](../FAQs.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

- [Eğitimler nasıl düzenlenmiş?](#eğitimler-nasıl-düzenlenmiş)
- [Feature Tutorial ile Design Tutorial arasındaki fark nedir?](#feature-tutorial-ile-design-tutorial-arasındaki-fark-nedir)
- [Tiremo Cloud platform dökümanı nerede?](#tiremo-cloud-platform-dökümanı-nerede)
- [Çalışmamı nereden kontrol ederim?](#çalışmamı-nereden-kontrol-ederim)
- [Nasıl eğitim katkısında bulunurum?](#nasıl-eğitim-katkısında-bulunurum)

## Eğitimler nasıl düzenlenmiş?

Eğitimler konulara (klasörlere) ayrılmıştır. Her konu firmware yığınının
bir katmanını, kabaca derleme sırasına göre ele alır:

- [Getting Started](./Getting_Started): araç zinciri, ilk LED, UART loglama, buton, LED desenleri; ağ yok.
- [Sensors & Local Storage](./Sensors_And_Storage): SHT40, LIS2DE12, batarya, mikrofon, flash ayarları, yazılım saati.
- [ESP32 Connectivity](./ESP32_Connectivity): AT taşıma katmanı, WiFi bağlantısı, yeniden denemeler, NTP, BluFi provizyonu.
- [Cloud Messaging](./Cloud_Messaging): MQTT, mTLS kimlik bilgileri, Tiremo Cloud bağlantısı, güvenilir yoklama.
- [Tiremo Cloud](./Tiremo_Cloud): cihaz kimliği, JSON yardımcıları, filo provizyonu, cihaz shadow'u. Platform dökümanı: [docs.tiremo.ai](https://docs.tiremo.ai).
- [Intelligence](./Intelligence): eğitim verisi toplama, Data Sources, Experiments, Solutions ve Tiremo UI'dan Edge dağıtımı.
- [Developer Contributed](./Developer_Contributed): numaralı müfredata uymayan, topluluktan gelen her şey.

## Feature Tutorial ile Design Tutorial arasındaki fark nedir?

Bir Feature Tutorial tek bir kavramı tek başına öğretir: tek bir sürücü,
tek bir AT komut dizisi, tek bir bulut temel işlemi. Bir Design Tutorial,
daha önce tamamlanmış birkaç Feature Tutorial'ı çalışan tek bir alt
sisteme birleştirir. Design Tutorial'lar her zaman ön koşul olan Feature
Tutorial'ları listeler.

## Tiremo Cloud platform dökümanı nerede?

Firmware anlatımları bu depoda; platform kavramları
[docs.tiremo.ai](https://docs.tiremo.ai) adresinde:

- [Giriş](https://docs.tiremo.ai/) ve [mimari ve veri akışı](https://docs.tiremo.ai/introduction/phase-1)
- [Cihaz provizyonu](https://docs.tiremo.ai/device-provisioning/getting-started) (claim / bootstrap sertifikaları, filo provizyonu, [TLS SNI](https://docs.tiremo.ai/device-provisioning/tls-sni))
- [Device State Controller](https://docs.tiremo.ai/device-state-controller/introduction) (klasik Device Shadow)
- [Tiremo Jobs](https://docs.tiremo.ai/developer-guide/jobs/jobs-introduction)
- [Developer Guide](https://docs.tiremo.ai/developer-guide), [PoC rehberi](https://docs.tiremo.ai/poc-guide), [SSS](https://docs.tiremo.ai/faq)

Her [Tiremo Cloud](./Tiremo_Cloud/) eğitimi kendisiyle eşleşen sayfalara link verir.

## Çalışmamı nereden kontrol ederim?

Güncel liste için kök [README'nin Çözümler listesine](./README.md#çözümler)
bakın. Getting Started, Sensors & Local Storage ve ESP32 Connectivity'nin
hepsinde artık gerçek çözümler var; sonraki bölümler (Cloud Messaging ve
sonrası) hâlâ büyük ölçüde iskelet. Her çözüm, o eğitimin `README.md`'sinin
yanındaki yerel bir `Sw/` klasöründedir (ör.
[Getting_Started/01-Hello_Blink/Sw/](../Getting_Started/01-Hello_Blink/Sw/));
neyin gerçek donanımda doğrulandığını, neyin hâlâ kart başında birinin
elini gerektirdiğini tam olarak görmek için o eğitimin kendi **Çözüm**
bölümüne bakın. Derlenebilir çözümler, bu depoda
[`Generation/`](./Generation/) altında duran ABOV üretici SDK'sının tek
bir kopyasını paylaşır; bir çözümü derlemeye çalışmadan önce o klasörün
README'sine bakın.

## Nasıl eğitim katkısında bulunurum?

İlgili konunun altına (numaralı müfredata uymuyorsa
[Developer_Contributed](./Developer_Contributed) altına) numaralı bir
klasör ekleyen bir PR açın. Klasördeki `README.md` mevcut eğitimlerle aynı
yapıda olsun: Introduction, You Will Learn, Prerequisites, Solution, Step
Outline, Done When.

<p align="center"><img alt="" src="../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Eğitimleri — Empa Electronics</sub></p>
