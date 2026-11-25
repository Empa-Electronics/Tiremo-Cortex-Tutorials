<p class="sphinxhide" align="right"><a href="../../../Sensors_And_Storage/Feature_Tutorials/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Feature Tutorials</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Feature Tutorials

> Bu sayfa [İngilizce orijinalinin](../../../Sensors_And_Storage/Feature_Tutorials/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

Feature eğitimlerinin her biri tek bir kavramı ele alır: tek bir sürücü, tek bir AT komut dizisi, tek bir bulut temel işlemi. Sırayla ilerleyin; her eğitim ön koşulunu belirtir.

 <table style="width:100%">
 <tr>
 <td width="35%" align="center"><b>Eğitim</b>
 <td width="65%" align="center"><b>Açıklama</b>
 </tr>
 <tr>
 <td align="center"><a href="./01-I2C_Bus_Bringup/">I2C Bus Bring-up</a></td>
 <td>Üzerine bir sensör sürücüsü koymadan önce I2C2'yi devreye alıp taramak ve yoklamak.</td>
 </tr>
 <tr>
 <td align="center"><a href="./02-SHT40_Temperature_Humidity/">SHT40 Temperature / Humidity</a></td>
 <td>SHT40'tan I2C2 üzerinden sıcaklık ve nem okumak.</td>
 </tr>
 <tr>
 <td align="center"><a href="./03-LIS2DE12_Accelerometer/">LIS2DE12 Accelerometer</a></td>
 <td>X/Y/Z ivmesini mg cinsinden okumak; ölçek ve ODR'yi çalışırken ayarlanabilir yapmak.</td>
 </tr>
 <tr>
 <td align="center"><a href="./04-Battery_Voltage_ADC/">Battery Voltage via ADC</a></td>
 <td>AVDD besleme gerilimini dahili VCORE bandgap referansıyla ölçmek.</td>
 </tr>
 <tr>
 <td align="center"><a href="./05-Microphone_RMS_To_dB/">Microphone: RMS Capture to dB</a></td>
 <td>MP23ABS1 mikrofonunu ADC burst + DMA ile yakalamak ve RMS zarfını göreli dB seviyesine çevirmek.</td>
 </tr>
 <tr>
 <td align="center"><a href="./06-Flash_Backed_Config/">Flash-Backed Config</a></td>
 <td>Küçük bir yapıyı (magic + CRC32) veri flash'ına kaydetmek, böylece ayarlar yeniden başlatmadan sonra da korunsun.</td>
 </tr>
 <tr>
 <td align="center"><a href="./07-Software_Wall_Clock/">Software Wall Clock</a></td>
 <td>Epoch zamanını senkronizasyon anındaki epoch + SysTick ofseti olarak tutmak; bu çipte RTC çevre birimi yok.</td>
 </tr>
 </table>

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Eğitimleri — Empa Electronics</sub></p>
