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

***Kart: Tiremo Cortex (ESP32) · Bölüm: Sensors & Local Storage / Design Tutorials · Zorluk: Orta***

> Bu sayfa [İngilizce orijinalinin](../../../../Sensors_And_Storage/Design_Tutorials/01-Standalone_Environmental_Logger/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Ağ kodu eklemeden önce tüm sensör yığınının birlikte çalıştığını kanıtlayın: SHT40 + LIS2DE12 + batarya + mikrofona sabit aralıkla örnek, son N okuma flash'ta, açılışta UART'a döküm.

## Açılış banner'ı

Design Tutorials, Tiremo Cortex logosunu açılışta UART'a basar; böylece boş bir terminal bile kartı ve çalışan uygulamayı tanıtır. Feature Tutorials'da banner yoktur — banner, bir eğitimin tek çevre birimi alıştırması değil bütünleşik bir uygulama olduğunun işaretidir.

[`Sw/Common/Src/tiremo_banner.c`](../../../../Sensors_And_Storage/Design_Tutorials/01-Standalone_Environmental_Logger/Sw/Common/Src/tiremo_banner.c) `TIREMO_LOG_INFO` yerine doğrudan `TIREMO_Debug_PutsLine` kullanır; böylece `[INFO]` öneki çizimi bölmez. Çalışma zamanı log seviyesi INFO'nun altındaysa banner basılmaz (`tdf.py build -D TIREMO_DEBUG_LEVEL=2` sessiz açılış verir).

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

Çizimi iki kısıt şekillendirdi. Genişliği **100 sütundur**; terminaliniz en az bu kadar geniş değilse her satır kaydırılır ve logo dağılır. "Boş" satırlar ise gerçekten boş değil, tek boşluk içerir: `tdf.py monitor` boş satırları atar (`tools/tdf.py:214`) ve iki kelime arasındaki boşluk yok olurdu.

`tdf.py monitor` önce kartı resetleyip sonra seri portu açtığı için banner'ı genellikle kaçırır — ilk `hist` satırlarını yutan yarışın aynısı. Açılışı baştan izlemek için portu önce açın, resetlemeyi başka bir terminalden `tdf.py reset-device` ile yapın.

## Çözüm

```sh
cd Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

**Doğrulandı:** `.text` 29752, flaş OK, `flags=0x07` canlı örnek, reset sonrası `hist` + artan `seq`.

```text
[INFO] live seq=155 up=4245 ms  T=32.913 C  RH=54.287 %RH  acc=128/8/1012 mg  AVDD=2015 mV  mic=37 dB  flags=0x07
```

`flags=0x01` ve `acc=0/0/0` / `AVDD=0 mV`: LIS2DE12 ve batarya `Init()` görmemiş demektir. SHT40 `Read()` init olmadan da çalışır; diğer ikisi sıfır kalır.

Bu eğitim veri-flash sektör 0'ı kullanır (eğitim 06'daki `bootCount` kaydının üzerine yazar).

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
