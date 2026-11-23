<p class="sphinxhide" align="right"><a href="../../../../Sensors_And_Storage/Design_Tutorials/01-Standalone_Environmental_Logger/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>Standalone Environmental Logger</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Standalone Environmental Logger

***Kart: Tiremo Cortex (ABOV A34G43A, Cortex-M4) · Bölüm: Sensors & Local Storage / Design Tutorials · Zorluk: Orta***

> Bu sayfa [İngilizce orijinalinin](../../../../Sensors_And_Storage/Design_Tutorials/01-Standalone_Environmental_Logger/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır (`Sw/` klasörü yalnızca
> İngilizce eğitim klasöründe yaşar — burada kopyalanmaz); içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Tek bir satır ağ kodu eklemeden önce tüm sensör yığınının birlikte çalıştığını kanıtlayın: SHT40 + LIS2DE12 + batarya + mikrofon bir zamanlamayla örneklenir, son N okuma flash'a kaydedilir ve açılışta UART'a dökülür.

## Neler Öğreneceksiniz

- Feature Tutorial 02–06 sürücülerini tek bir SysTick döngüsünde birleştirmek (bulut yok)
- Son 8 örneğin flash halkası (`ENV1` magic + CRC32), açılışta en eskiden başlayarak yeniden oynatılır
- Kalıcı zaman damgalarının neden eğitim 07'nin RAM'deki duvar saati değil de `seq` + `uptimeMs` olduğu (o saat resetle birlikte ölür)
- İki sensör aynı döngüde aynı veri yolunu paylaşınca I2C katmanında nelerin değiştiği

## Ön Koşullar

- [Sensors & Local Storage / Feature Tutorials](../../Feature_Tutorials/) eğitimlerinin tamamı (01–07)
- [Getting Started](../../../Getting_Started/) kurulum betikleri çalıştırılmış olmalı, böylece `tdf` kullanılabilir
- Log'u izlemek için: `tdf monitor` ya da kartın USB seri portunda 115200 8N1'de herhangi bir seri terminal (Tera Term, PuTTY, screen, minicom, …), **en az 100 sütun genişliğinde** (aşağıdaki banner'a bakın)
- USB kabloyla bilgisayarınıza bağlı bir Tiremo Cortex kartı (ABOV A34G43A MCU — bkz. [Donanıma Genel Bakış](../../../Hardware.md)). Kart üzerindeki CMSIS-DAP hata ayıklama probu SWD ile programlamayı yapar; ayrı bir programlayıcı gerekmez.

## Açılış banner'ı

Design Tutorials, Tiremo Cortex logosuyla açılır; böylece boş bir terminal bile herhangi bir sensör çıktısından önce kartı ve çalışan uygulamayı tanıtır. Feature Tutorials sessiz kalır — banner, bir eğitimin tek çevre birimi alıştırması değil bütünleşik bir uygulama olduğunun işaretidir.

[`Sw/Common/Src/tiremo_banner.c`](../../../../Sensors_And_Storage/Design_Tutorials/01-Standalone_Environmental_Logger/Sw/Common/Src/tiremo_banner.c) `TIREMO_LOG_INFO` yerine `TIREMO_Debug_PutsLine` ile doğrudan UART'a yazar; böylece `[INFO]` öneki çizimi bölmez. Çalışma zamanı log seviyesi INFO'nun altına düştüğünde banner basılmaz, yani `tdf build -DTIREMO_DEBUG_LEVEL=2` sessiz bir açılış verir.

```text
              ####
   #####      ####
   #####
   #####
############ #####   ###########   ###########     ############### ###########       ############
############ #####   ########### ###############   ############################    ###############
   #####     #####   ######     #####       #####  ######     #######     #####   ######      #####
   #####     #####   #####      #################  #####       #####       ##### ######       ######
   #####     #####   #####      #################  #####       #####       ##### #####         #####
   #####     #####   #####      #####              #####       #####       #####  #####       ######
   ######### #####   #####      ######    ######   #####       #####       #####  #######   #######
    ######## #####   #####        #############    #####       #####       #####    #############
      ###### #####   #####          #########      #####       #####       #####      #########


                                                                    ###
                                   ######       ######    ###  ## ########    #####    ####    #####
                                 ##########  ###########  ####### ######## ###########  ####  ####
                                ####         ###     #### ####      ###   ####     ####   #######
                                ###         ####      ### ####      ###   #############    ####
                                ####        ####      ### ####      ###   ####            #######
                                 ##########  ###########  ####      #####  ###########  ####  ####
                                   #######     #######    ####       ####    #######   ####    #####

  Standalone Environmental Logger
  Design Tutorial - Empa Electronics
```

Çizimi iki kısıt şekillendirdi. Genişliği **100 sütundur**; terminalinizi en az bu genişliğe ayarlayın, yoksa her satır kayar ve logo dağılır. "Boş" satırlar ise gerçekten boş değil, tek bir boşluk içerir: `tdf monitor` boş satırları atlar ve iki kelime arasındaki boşluk yok olurdu.

`tdf monitor` önce seri portu açar, sonra kartı resetler; bu yüzden banner'ı ve `history` dökümü dahil açılışın tamamını görürsünüz. Başka bir terminal (Tera Term, PuTTY, …) kullanıyorsanız önce bağlanın, sonra RESET'e basın.

## Halkayı anlamak

[`Sw/Common/Src/tiremo_envlog.c`](../../../../Sensors_And_Storage/Design_Tutorials/01-Standalone_Environmental_Logger/Sw/Common/Src/tiremo_envlog.c) veri-flash'ın **sektör 0**'ını kullanır — eğitim 06'nın `bootCount` için kullandığı sektörün aynısı; yani bu logger'ı çalıştırmak o kaydın üzerine yazar (tersi de geçerli). Düzen: magic `"ENV1"` + sürüm + sayaç + `nextSeq` + 8 adet `TIREMO_EnvSample_t` + CRC32 (06 ile aynı polinom). Sektörde geçerli bir `ENV1` kaydı yoksa — örneğin logger'ı ilk kez çalıştırdığınızda — geçmiş boş başlar.

`seq` resetler boyunca monoton artar. `uptimeMs` yalnızca *bu* açılıştan beri geçen SysTick süresidir. Eğitim 07'nin `TIREMO_Clock`'u yalnızca RAM'de yaşar, bu yüzden kaydedilen o değildir.

`flags` bitleri: `0x01` SHT40, `0x02` ivme, `0x04` batarya. `0x07` üç okumanın da başarılı olduğu anlamına gelir; eksik bir bit o okumanın başarısız olduğunu ve alanlarının `0` olarak loglandığını gösterir. Mikrofon dB değeri DMA yolundan en iyi çabayla alınır (eğitim 05).

Ana döngü her turda `TIREMO_MIC_Service()`'i çağırır ve her `SAMPLE_INTERVAL_MS` (3000 ms) sürede bir tam örnek alır.

**Flash aşınması — günlerce çalışır halde bırakmayın.** Her ekleme tüm sektörü silip yeniden yazar; 3 saniyede bir örnekle sektör saatte yaklaşık 1.200 kez (günde ~28.800 kez) silinir. Veri-flash sektörleri yalnızca sınırlı sayıda silme döngüsü için derecelendirilmiştir (A34G43A veri sayfasına bakın), bu yüzden gece gündüz log tutan bir kart sektörü birkaç gün içinde aşındırabilir. Kısa bir eğitim çalışması için sorun değildir; gerçek bir ürün daha seyrek log tutar, yazmadan önce birkaç örneği RAM'de biriktirir ya da yazmaları birden fazla sektöre yayar (wear levelling).

## Aynı I2C veri yolunda iki sensör

SHT40 ve LIS2DE12 I2C2'yi paylaşır ve her 3 saniyede art arda okunur. Bu durum [`Sw/Tiremo_Hal/Src/tiremo_hal_i2c.c`](../../../../Sensors_And_Storage/Design_Tutorials/01-Standalone_Environmental_Logger/Sw/Tiremo_Hal/Src/tiremo_hal_i2c.c) içinde, tek sensörlü bir demonun göz ardı edebileceği üç ayrıntıyı ortaya çıkarır; bu projedeki I2C HAL kopyası üçünü de ele alır:

- **Son bayta NACK.** Okuma sırasında bir baytın ACK/NACK'i, master o baytı almaya başladığında belirlenir; bu yüzden ACK, son bayt saatlenmeden *önce* kapatılır. Son bayt ACK'lenirse sensör, master STOP göndermeye çalışırken SDA'ya bir sonraki baytı sürmeye devam eder ve veri yolu takılı kalır.
- **STOP'u gerçekten sürmek.** STOP istendikten sonra bekleyen durum bayrağı temizlenir; böylece çevre birimi SCL'yi bırakır ve STOP koşulu veri yoluna ulaşır.
- **Yalnızca okuma transferleri.** `TIREMO_HAL_I2C_WriteRead()` `txLength = 0` kabul eder ve cihazı doğrudan okuma için adresler. SHT40 sürücüsü hem komut yazması hem de 6 baytlık okuması için `WriteRead()` kullanır; böylece her transfer, üreticinin bloklayan `HAL_I2C_Transmit` / `HAL_I2C_Receive` çağrıları yerine HAL'in zaman aşımından geçer.

Bunlar olmadan belirtiler şunlardır: SHT40 okumaları her iki örnekte bir başarısız olur (`T=0.000` ile `flags=0x04` / `0x05`) ve LIS2DE12 hiç başlatılamaz (`acc=0/0/0`).

## Çözüm

Repo kök dizininden:

```sh
cd Sensors_And_Storage/Design_Tutorials/01-Standalone_Environmental_Logger/Sw
tdf build
tdf flash
tdf monitor
```

(`tdf`'nin ne olduğu için [Getting_Started/01-Hello_Blink'in Çözüm bölümüne](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) bakın. macOS/Linux'ta `tdf.py` takma ad olarak da çalışır; Windows'ta `tdf` kullanın. `tdf monitor`'ü çalıştırmadan önce kartın seri portundaki diğer terminalleri kapatın — portu aynı anda yalnızca bir program açabilir.)

**Derleme notları:** [01-Hello_Blink'in Çözüm bölümünde](../../../Getting_Started/01-Hello_Blink/README.md#çözüm) açıklanan beklenen ve zararsız `_close` / `_lseek` / `_read` / `_write is not implemented` linker uyarılarına ek olarak bu derleme, üreticinin ADC HAL başlık dosyasından (`hal_adc_v_01_00_05.h`) `-Wenum-compare` uyarıları da basar. Bunlar değiştirilmemiş üretici kodundan gelir ve zararsızdır.

**Gerçek donanımda doğrulandı:** `tdf build`, macOS'ta ve Windows 11'de hatasız derlenip bağlanıyor (`.text` yaklaşık 28,5 KB; kesin boyut toolchain sürümüne göre biraz değişir). `tdf flash`, gerçek, bağlı bir Tiremo Cortex kartına karşı SWD üzerinden tam bir programlama-ve-doğrulama döngüsünü tamamladı (`** Verified OK **`). Banner'dan sonra, boş bir veri-flash ile ilk açılış şöyle görünür:

```text
[INFO] Standalone_Environmental_Logger: boot
[INFO] history: (empty)
[INFO] live seq=0 up=4326 ms  T=25.476 C  RH=49.711 %RH  acc=-128/-226/924 mg  AVDD=2647 mV  mic=21 dB  flags=0x07
[INFO] live seq=1 up=7326 ms  T=25.530 C  RH=49.677 %RH  acc=-124/-234/932 mg  AVDD=2649 mV  mic=33 dB  flags=0x07
[INFO] live seq=2 up=10326 ms  T=25.543 C  RH=49.566 %RH  acc=-132/-234/936 mg  AVDD=2647 mV  mic=31 dB  flags=0x07
```

Her satırda `flags=0x07` var; `AVDD` 2,65–2,7 V civarında (VDDA hattı — bkz. [04-Battery_Voltage_ADC](../../Feature_Tutorials/04-Battery_Voltage_ADC/README.md)); ivmeölçer büyüklüğü yaklaşık 1 g. Şimdi RESET'e basın (ya da `tdf monitor`'ü yeniden çalıştırın). Açılış `history: 8 sample(s), oldest first` basar, son sekiz örneği orijinal `seq` ve `up` değerleriyle `hist` etiketiyle yeniden oynatır, ardından yeni `live` örnekler bir sonraki `seq`'ten devam eder — kalıcılığın kanıtı budur.

## Adım Adım İnceleme

1. [`main.c`](../../../../Sensors_And_Storage/Design_Tutorials/01-Standalone_Environmental_Logger/Sw/Src/main.c) — olağan port / saat / UART başlatmasının üzerine `PRV_I2C_Init`, `PRV_ADC_Init`, `PRV_Timer1_Init`
2. [`board_config.h`](../../../../Sensors_And_Storage/Design_Tutorials/01-Standalone_Environmental_Logger/Sw/Board/board_config.h) — eğitim 02–05'in birleşimi
3. [`prv_user_code.c`](../../../../Sensors_And_Storage/Design_Tutorials/01-Standalone_Environmental_Logger/Sw/Src/prv_user_code.c) — banner, flash log'unu ve tüm sensörleri başlatma (başarısız bir sensör `[WARN]` basar ve logger onsuz devam eder), halkayı yeniden oynatma, ardından her 3 saniyede örnekle + ekle
4. [`tiremo_banner.c`](../../../../Sensors_And_Storage/Design_Tutorials/01-Standalone_Environmental_Logger/Sw/Common/Src/tiremo_banner.c) — bunu yeni bir Design Tutorial'ın `Common/` klasörüne kopyalayın
5. Feature eğitimlerinden kopyalanan sürücüler, artı yukarıdaki **Aynı I2C veri yolunda iki sensör** bölümünde anlatılan I2C HAL ayrıntıları

## Tamamlanma Ölçütü

Her `live` satırı `flags=0x07` gösteriyor; bir resetten sonra açılış son örnekleri `hist` satırları olarak yeniden oynatıyor ve ardından bir sonraki `seq` ile `live`'a devam ediyor — resetten önceki örneklerin hiçbiri kaybolmuyor.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
