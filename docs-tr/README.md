<p class="sphinxhide" align="right"><a href="../README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../images/banner.png" width="100%">
      <h1>Tiremo Cortex — Ayrıntılı Eğitimler</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + Tiremo Cloud üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>

> Bu sayfa [ana İngilizce README](../README.md) dosyasının Türkçe çevirisidir ve yalnızca gezinme
> içindir — eğitimlerin kendisi (klasör adları, dosya içerikleri, kod) İngilizce kalır.
> İçerik farklılaşırsa İngilizce sürüm esas alınır.

## Bu Kart Hakkında

Bu depo, **Tiremo Cortex** için üretici yazılımının adım adım inşa edilmiş halidir:
ESP32 tabanlı bir IoT sensör kartı. Tek radyo WiFi'dir (istasyon bağlantısı +
BluFi BLE ile kurulum). Kart **Tiremo Cloud**'a [`iot.tiremo.ai`](https://iot.tiremo.ai) üzerinde,
ESP32'nin kendi AT komut seti üzerinden karşılıklı TLS ile MQTT ile bağlanmak
üzere tasarlanmıştır. Operatörler **Tiremo UI** kullanır. Platform kavramları
(kurulum, Device Shadow, işler, telemetri) [Tiremo belgelendirmesinde](https://docs.tiremo.ai)dir.

Buradaki her eğitim, tam bir üretici yazılımı derlemesinin daha küçük,
bağımsız bir dilimidir — mevcut durum için aşağıdaki
[Çözümler](#çözümler) bölümüne bakın.

## Nereden Başlamalı

Bu koda yeni misiniz? [Getting Started](../Getting_Started/README.md) ile
başlayın — bir LED yakıp söndürün, UART loglamayı devreye alın, butonu okuyun,
on LED'lik bankayı sürün. Bu bölüm için WiFi veya bulut hesabı gerekmez.

Oradan sonra eğitimler [Eğitimler](#eğitimler) altında konuya göre
düzenlenmiştir, kabaca gerçek bir devreye almanın izleyeceği sırayla: önce
yerel sensörler, sonra ESP32 WiFi, sonra bulut mesajlaşma, sonra Tiremo Cloud,
sonra Intelligence (telemetri toplama, Tiremo UI'da eğitim, bir
Solution dağıtma).

Her konu iki tür eğitime ayrılır:

- **Feature Tutorials** (Özellik Eğitimleri) her biri tek bir kavramı ele alır
  — tek bir sürücü, tek bir AT komut dizisi, tek bir bulut ilkesi.
- **Design Tutorials** (Tasarım Eğitimleri) birkaç özellik eğitimini, gerçekte
  gönderilene daha yakın, çalışan tek bir alt sistemde birleştirir.

## Yardım Almak

- Önce bir eğitimin **Solution** (Çözüm) bölümüne bakın — o an için bir
  çözümün yazılıp yazılmadığını orada görürsünüz.
- Bulut platformu (MQTT, filo kurulumu, Device Shadow, işler) için
  [Tiremo belgelendirmesi](https://docs.tiremo.ai) — her
  [Tiremo Cloud](../Tiremo_Cloud/) eğitimi eşleşen sayfaları bağlar.
- Bir eğitimle ilgili sorularınız veya sorunlarınız için bir
  [Issue](https://github.com/Empa-Electronics/Tiremo-Cortex-Tutorials/issues)
  açın.

## Depoyu İndirme

```sh
git clone https://github.com/Empa-Electronics/Tiremo-Cortex-Tutorials.git
```

## Donanım

[Donanım Genel Bakış](./Hardware.md) — kart blok diyagramı, host bağlantı diyagramı, ve pin haritası referansı. Üretici yazılımı yok, yalnızca kartın kendisi.

## İlgili Depolar

[CRA-ready ESP-AT](https://github.com/Empa-Electronics/Cra-Ready-Esp-At) — bu
eğitimlerin ESP32 modülünde konuştuğu AT komut setini sağlayan Espressif
ESP-AT üretici yazılımını, AB **Siber Dayanıklılık Yasası (CRA)** ve RED
Yetkilendirilmiş Tüzüğü (EN 18031) tarafından istenen güvenlik özellikleriyle
derler: Secure Boot v2, flash ve NVS şifreleme, şifreli BLE ve sıkılaştırılmış
varsayılan ayarlar. Host tarafı aynı AT komutlarını göndermeye devam eder.

## Sürüm Notları

[Değişiklik Günlüğü](../CHANGELOG.md)

## Eğitimler

<table border="0" width="100%">
    <colgroup><col width="50%"><col width="50%"></colgroup>
    <tbody>
        <tr>
            <td colspan="2" height="24" align="center">
                <strong><a href="../Getting_Started">Getting Started</a></strong>
            </td>
        </tr>
        <tr>
            <td colspan="2" height="48">
                Buradan başlayın! Bir LED yakıp söndürün, log alın, butonu okuyun ve on LED'in tamamını sürün — henüz WiFi veya bulut gerekmiyor.
            </td>
        </tr>
        <tr>
            <td><a href="./Getting_Started/01-Hello_Blink/">Hello Blink</a></td>
            <td><a href="./Getting_Started/02-SysTick_Timing/">SysTick Timing</a></td>
        </tr>
        <tr>
            <td><a href="./Getting_Started/03-Debug_UART_Logging/">Debug UART Logging</a></td>
            <td><a href="./Getting_Started/04-Button_Read_And_Debounce/">Button Read & Debounce</a></td>
        </tr>
        <tr>
            <td><a href="./Getting_Started/05-Ten_LED_Pattern_Driver/">Ten-LED Pattern Driver</a></td>
            <td></td>
        </tr>
    </tbody>
</table>

<table border="0" width="100%">
    <colgroup><col width="50%"><col width="50%"></colgroup>
    <tbody>
        <tr>
            <td colspan="2" height="24" align="center">
                <strong><a href="../Sensors_And_Storage/">Sensors & Local Storage</a></strong>
            </td>
        </tr>
        <tr>
            <td colspan="2" height="48">
                Kart üzerindeki her sensörü ve sürücüyü devreye alın — SHT40, LIS2DE12, pil, mikrofon — ayrıca flash tabanlı yapılandırma kalıcılığı ve yazılım saatini de ekleyin; henüz ağ bağlantısı yok.
            </td>
        </tr>
        <tr>
            <td height="24"><strong>Feature Tutorials</strong></td>
            <td height="24"><strong>Design Tutorials</strong></td>
        </tr>
        <tr>
            <td><a href="../Sensors_And_Storage/Feature_Tutorials/01-I2C_Bus_Bringup/">I2C Bus Bring-up</a></td>
            <td><a href="../Sensors_And_Storage/Design_Tutorials/01-Standalone_Environmental_Logger/">Standalone Environmental Logger</a></td>
        </tr>
        <tr>
            <td><a href="../Sensors_And_Storage/Feature_Tutorials/02-SHT40_Temperature_Humidity/">SHT40 Temperature / Humidity</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../Sensors_And_Storage/Feature_Tutorials/03-LIS2DE12_Accelerometer/">LIS2DE12 Accelerometer</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../Sensors_And_Storage/Feature_Tutorials/04-Battery_Voltage_ADC/">Battery Voltage via ADC</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../Sensors_And_Storage/Feature_Tutorials/05-Microphone_RMS_To_dB/">Microphone: RMS Capture to dB</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../Sensors_And_Storage/Feature_Tutorials/06-Flash_Backed_Config/">Flash-Backed Config</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../Sensors_And_Storage/Feature_Tutorials/07-Software_Wall_Clock/">Software Wall Clock</a></td>
            <td></td>
        </tr>
    </tbody>
</table>

<table border="0" width="100%">
    <colgroup><col width="50%"><col width="50%"></colgroup>
    <tbody>
        <tr>
            <td colspan="2" height="24" align="center">
                <strong><a href="../ESP32_Connectivity/">ESP32 Connectivity</a></strong>
            </td>
        </tr>
        <tr>
            <td colspan="2" height="48">
                ESP32 AT komut aktarımını devreye alın, yeniden denemelerle WiFi'ye katılın, NTP üzerinden saati eşitleyin ve BluFi BLE ile kimlik bilgilerini kurun — bu kartın internete çıkmak için kullandığı bağlantı zincirinin tamamı.
            </td>
        </tr>
        <tr>
            <td height="24"><strong>Feature Tutorials</strong></td>
            <td height="24"><strong>Design Tutorials</strong></td>
        </tr>
        <tr>
            <td><a href="../ESP32_Connectivity/Feature_Tutorials/01-AT_Command_Transport/">AT Command Transport</a></td>
            <td><a href="../ESP32_Connectivity/Design_Tutorials/01-Boot_Cascade_Saved_WiFi_Then_BluFi/">Boot Cascade: Saved WiFi → BluFi</a></td>
        </tr>
        <tr>
            <td><a href="../ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/">WiFi Station Join</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/">Connect/Retry State Machine</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../ESP32_Connectivity/Feature_Tutorials/04-NTP_Time_Sync/">NTP Time Sync</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../ESP32_Connectivity/Feature_Tutorials/05-BluFi_Provisioning/">BluFi BLE Provisioning</a></td>
            <td></td>
        </tr>
    </tbody>
</table>

<table border="0" width="100%">
    <colgroup><col width="50%"><col width="50%"></colgroup>
    <tbody>
        <tr>
            <td colspan="2" height="24" align="center">
                <strong><a href="../Cloud_Messaging/">Cloud Messaging (MQTT & TLS)</a></strong>
            </td>
        </tr>
        <tr>
            <td colspan="2" height="48">
                MQTT'yi baştan sona çalışır hale getirin: önce TLS'siz genel bir broker, sonra ESP32'nin kendi NVS'sinde tutulan karşılıklı TLS kimlik bilgileri, sonra gerçek bir Tiremo Cloud oturumu.
            </td>
        </tr>
        <tr>
            <td height="24"><strong>Feature Tutorials</strong></td>
            <td height="24"><strong>Design Tutorials</strong></td>
        </tr>
        <tr>
            <td><a href="../Cloud_Messaging/Feature_Tutorials/01-MQTT_To_Public_Broker/">MQTT to a Public Broker</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/">mTLS Credentials in Module NVS</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/">MQTT mTLS to Tiremo Cloud</a></td>
            <td></td>
        </tr>
    </tbody>
</table>

<table border="0" width="100%">
    <colgroup><col width="50%"><col width="50%"></colgroup>
    <tbody>
        <tr>
            <td colspan="2" height="24" align="center">
                <strong><a href="../Tiremo_Cloud/">Tiremo Cloud</a></strong>
            </td>
        </tr>
        <tr>
            <td colspan="2" height="48">
                Cihaz kimliği, bu üretici yazılımının tam bir ayrıştırma kütüphanesi yerine kullandığı minimal JSON yardımcıları, iddia sertifikası ile filo kurulumu, cihaz shadow kontrol döngüsü, telemetri ve alarmlar, Tiremo Jobs ve OTA üretici yazılımı güncellemeleri.
            </td>
        </tr>
        <tr>
            <td height="24"><strong>Feature Tutorials</strong></td>
            <td height="24"><strong>Design Tutorials</strong></td>
        </tr>
        <tr>
            <td><a href="../Tiremo_Cloud/Feature_Tutorials/01-Device_Identity_From_MAC/">Device Identity from MAC</a></td>
            <td><a href="../Tiremo_Cloud/Design_Tutorials/01-Sensor_Telemetry_And_Shadow_Control/">Sensor Telemetry and Shadow Control</a></td>
        </tr>
        <tr>
            <td><a href="../Tiremo_Cloud/Feature_Tutorials/02-Minimal_JSON_Helpers/">Minimal JSON Helpers</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../Tiremo_Cloud/Feature_Tutorials/03-Fleet_Provisioning_By_Claim/">Fleet Provisioning by Claim Certificate</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../Tiremo_Cloud/Feature_Tutorials/04-Shadow_Get_Apply_Report/">Shadow Get + Apply + Report</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../Tiremo_Cloud/Feature_Tutorials/05-Live_Shadow_Delta_Handling/">Live Shadow Delta Handling</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../Tiremo_Cloud/Feature_Tutorials/06-Publishing_Alarms_And_Telemetry/">Publishing Alarms and Telemetry</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../Tiremo_Cloud/Feature_Tutorials/07-Tiremo_Jobs/">Tiremo Jobs</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../Tiremo_Cloud/Feature_Tutorials/08-OTA_Firmware_Update/">OTA Firmware Update</a></td>
            <td></td>
        </tr>
    </tbody>
</table>

<table border="0" width="100%">
    <colgroup><col width="50%"><col width="50%"></colgroup>
    <tbody>
        <tr>
            <td colspan="2" height="24" align="center">
                <strong><a href="./Intelligence/">Intelligence</a></strong>
            </td>
        </tr>
        <tr>
            <td colspan="2" height="48">
                Eğitim verisini toplayın (Data Explorer veya tercih ettiğiniz herhangi bir yol), bir Data Source'a dönüştürün, bir Experiment eğitin ve bir Solution dağıtın, cihaz üzerindeki bir Edge paketi dahil, Tiremo UI'dan. Özellik eğitimleri yalnızca UI'dır. Üç tasarım eğitimi LIS2DE12 üretici yazılımı yükler: örnek başına telemetri, yerel 8 örnekli pencere, ardından HMR Edge çıkarımı.
            </td>
        </tr>
        <tr>
            <td height="24"><strong>Feature Tutorials</strong></td>
            <td height="24"><strong>Design Tutorials</strong></td>
        </tr>
        <tr>
            <td><a href="./Intelligence/Feature_Tutorials/01-Data_Collection/">Data Collection</a></td>
            <td><a href="./Intelligence/Design_Tutorials/01-Accelerometer_Telemetry/">Accelerometer Telemetry</a></td>
        </tr>
        <tr>
            <td><a href="./Intelligence/Feature_Tutorials/02-Data_Sources/">Data Sources</a></td>
            <td><a href="./Intelligence/Design_Tutorials/02-Accelerometer_Windowing/">Accelerometer Windowing</a></td>
        </tr>
        <tr>
            <td><a href="./Intelligence/Feature_Tutorials/03-Experiments/">Experiments</a></td>
            <td><a href="./Intelligence/Design_Tutorials/03-Edge_Deployment_HMR/">HMR Gesture Classifier on the Edge</a></td>
        </tr>
        <tr>
            <td><a href="./Intelligence/Feature_Tutorials/04-Solutions/">Solutions</a></td>
            <td></td>
        </tr>
    </tbody>
</table>

<table border="0" width="100%">
    <colgroup><col width="50%"><col width="50%"></colgroup>
    <tbody>
        <tr>
            <td colspan="2" height="24" align="center">
                <strong><a href="../Developer_Contributed">Developer Contributed</a></strong>
            </td>
        </tr>
        <tr>
            <td colspan="2" height="48">
                Numaralı müfredata girmeyen topluluk ve şirket içi eğitimler — devreye alma notları,
                alternatif arka uçlar, tezgah araçları. Katkılar memnuniyetle karşılanır.
            </td>
        </tr>
    </tbody>
</table>


## Çözümler

Üretici yazılımı içeren her eğitim, `README.md`'sinin hemen yanındaki
yerel bir `Sw/` klasöründe eksiksiz, derlenebilir bir çözüm sunar; o
eğitimin kendi **Çözüm** bölümü nasıl derlenip flaşlanacağını ve çıktının
nasıl görünmesi gerektiğini gösterir. (Dört
[Intelligence Feature Tutorial](../Intelligence/Feature_Tutorials/)
Tiremo Intelligence web arayüzünün anlatımlarıdır; bu yüzden `Sw/`
klasörleri yoktur.)

Derlenebilir eğitimler, bu depoya tek kopya olarak eklenmiş tek bir ABOV
satıcı SDK'sını ([`Generation/`](../Generation/)) ve tek bir
derleme/flaşlama CLI'ını paylaşır: [`tools/tdf.py`](../tools/tdf.py)
("Tiremo Development Framework" — ESP-IDF için `idf.py`'nin oynadığı ile
aynı rol). Yalnızca ilk seferde:

```sh
# macOS / Linux
./install.sh
source ./export.sh
```

```powershell
# Windows (PowerShell)
.\install.ps1
. .\export.ps1
```

```bat
:: Windows (cmd.exe)
install.bat
export.bat
```

`tdf.py`'nin kendisi düz Python'dır ve üç platformda da aynı çalışır —
işletim sistemine özgü tek parça bu kurulum betikleridir (toolchain'i
kontrol eder ve `tdf_venv/` + `PATH`'i bağlar). Windows'ta Git Bash veya
WSL gerekmez. `python tools\tdf.py build` hiçbir kurulum adımı olmadan
çalışır.

Ardından, herhangi bir eğitimin `Sw/` dizininden (örneğin depo kökünden
`cd Getting_Started/01-Hello_Blink/Sw`):

```sh
tdf build              # yapılandır + derle
tdf flash               # derle, sonra SWD üzerinden flaşla
tdf monitor             # kartı resetle, UART log'unu akıt (renk kodlu)
tdf flash monitor       # zincir: flaşla, sonra izle
tdf build flash monitor
tdf clean                # build/'i kaldır
tdf clean-device         # kartın çip flash'ını topluca sil (önce sorar)
tdf create-project <Name>   # yeni, boş bir Sw/ projesi oluştur
tdf help                    # komut özeti
tdf help flash              # bir komutun ayrıntılı yardımı
```

macOS/Linux'ta `tdf.py` takma ad olarak da çalışır; Windows'ta `tdf` kullanın. `source ./export.sh` (bash/zsh) ve `. .\export.ps1` (PowerShell) bu komut adları için tab tamamlamayı da kurar.

Her komutun gerçekte ne yaptığı, neden elle yazılmış bir Makefile
yerine CMake kullanıldığı, ve Windows'a özgü notlar için
[01-Hello_Blink'in Çözüm bölümüne](./Getting_Started/01-Hello_Blink/README.md#çözüm)
bakın.

<p align="center"><img alt="" src="../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
