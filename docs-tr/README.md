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
      <sub>Tiremo Cortex Eğitimleri — ESP32 + AWS IoT üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>

> Bu sayfa [ana İngilizce README](../README.md) dosyasının Türkçe çevirisidir ve yalnızca gezinme
> içindir — eğitimlerin kendisi (klasör adları, dosya içerikleri, kod) İngilizce kalır.
> İçerik farklılaşırsa İngilizce sürüm esas alınır.

## Bu Kart Hakkında

Bu depo, **Tiremo Cortex** için üretici yazılımının adım adım inşa edilmiş halidir:
ESP32 tabanlı bir IoT sensör kartı. Tek radyo WiFi'dir (istasyon bağlantısı +
BluFi BLE ile kurulum) ve AWS IoT Core'a, ESP32'nin kendi AT komut seti
üzerinden karşılıklı TLS ile MQTT ile ulaşılır.

Buradaki her eğitim, tam bir üretici yazılımı derlemesinin daha küçük,
bağımsız bir dilimidir — mevcut durum için aşağıdaki
[Çözümler](#çözümler) bölümüne bakın.

## Nereden Başlamalı

Bu koda yeni misiniz? [Getting Started](../Getting_Started/README.md) ile
başlayın — bir LED yakıp söndürün, UART loglamayı devreye alın, butonu okuyun,
on LED'lik bankayı sürün. Bu bölüm için WiFi veya bulut hesabı gerekmez.

Oradan sonra eğitimler [Eğitimler](#eğitimler) altında konuya göre
düzenlenmiştir, kabaca gerçek bir devreye almanın izleyeceği sırayla: önce
yerel sensörler, sonra ESP32 WiFi, sonra bulut mesajlaşma, sonra AWS IoT Core,
sonra çalışan bir prototipi filoya hazır bir cihaza dönüştüren üretim
davranışları (shadow tabanlı LED'ler, alarmlar, telemetri, işler, fabrika
sıfırlama).

Her konu iki tür eğitime ayrılır:

- **Feature Tutorials** (Özellik Eğitimleri) her biri tek bir kavramı ele alır
  — tek bir sürücü, tek bir AT komut dizisi, tek bir bulut ilkesi.
- **Design Tutorials** (Tasarım Eğitimleri) birkaç özellik eğitimini, gerçekte
  gönderilene daha yakın, çalışan tek bir alt sistemde birleştirir.

## Yardım Almak

- Önce bir eğitimin **Solution** (Çözüm) bölümüne bakın — o an için bir
  çözümün yazılıp yazılmadığını orada görürsünüz.
- Bir eğitimle ilgili sorularınız veya sorunlarınız için bir
  [Issue](https://github.com/Empa-Electronics/Tiremo-Cortex-Tutorials/issues)
  açın.

## Depoyu İndirme

```sh
git clone https://github.com/Empa-Electronics/Tiremo-Cortex-Tutorials.git
```

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
            <td><a href="../Getting_Started/04-Button_Read_And_Debounce/">Button Read & Debounce</a></td>
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
                MQTT'yi baştan sona çalışır hale getirin: önce TLS'siz genel bir broker, sonra ESP32'nin kendi NVS'sinde tutulan karşılıklı TLS kimlik bilgileri, sonra gerçek bir AWS IoT Core oturumu, sonra üretim üretici yazılımının dayandığı işleyici tabanlı yoklama deseni.
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
            <td><a href="../Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/">MQTT mTLS to AWS IoT Core</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../Cloud_Messaging/Feature_Tutorials/04-Reliable_Inbound_Message_Draining/">Reliable Inbound Message Draining</a></td>
            <td></td>
        </tr>
    </tbody>
</table>

<table border="0" width="100%">
    <colgroup><col width="50%"><col width="50%"></colgroup>
    <tbody>
        <tr>
            <td colspan="2" height="24" align="center">
                <strong><a href="../AWS_IoT_Core/">AWS IoT Core</a></strong>
            </td>
        </tr>
        <tr>
            <td colspan="2" height="48">
                Cihaz kimliği, iddia sertifikası ile filo kurulumu, bu üretici yazılımının tam bir ayrıştırma kütüphanesi yerine kullandığı minimal JSON yardımcıları ve cihaz shadow kontrol döngüsü — get, apply, report ve canlı delta.
            </td>
        </tr>
        <tr>
            <td height="24"><strong>Feature Tutorials</strong></td>
            <td height="24"><strong>Design Tutorials</strong></td>
        </tr>
        <tr>
            <td><a href="../AWS_IoT_Core/Feature_Tutorials/01-Device_Identity_From_MAC/">Device Identity from MAC</a></td>
            <td><a href="../AWS_IoT_Core/Design_Tutorials/01-Provision_And_Sync_A_Real_Thing/">Provision and Sync a Real Thing</a></td>
        </tr>
        <tr>
            <td><a href="../AWS_IoT_Core/Feature_Tutorials/02-Fleet_Provisioning_By_Claim/">Fleet Provisioning by Claim Certificate</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../AWS_IoT_Core/Feature_Tutorials/03-Minimal_JSON_Helpers/">Minimal JSON Helpers</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../AWS_IoT_Core/Feature_Tutorials/04-Shadow_Get_Apply_Report/">Shadow Get + Apply + Report</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../AWS_IoT_Core/Feature_Tutorials/05-Live_Shadow_Delta_Handling/">Live Shadow Delta Handling</a></td>
            <td></td>
        </tr>
    </tbody>
</table>

<table border="0" width="100%">
    <colgroup><col width="50%"><col width="50%"></colgroup>
    <tbody>
        <tr>
            <td colspan="2" height="24" align="center">
                <strong><a href="../Production_Firmware/">Production Firmware</a></strong>
            </td>
        </tr>
        <tr>
            <td colspan="2" height="48">
                Çalışan bir prototipi filoya hazır bir cihaza dönüştüren davranışlar: shadow tabanlı LED'ler, histerezisli kenar alarmları, gruplanmış telemetri, AWS IoT Jobs, fabrika sıfırlama ve bağlantı kurtarma — hepsi tam kapsamlı bir yapı ile taçlanır.
            </td>
        </tr>
        <tr>
            <td height="24"><strong>Feature Tutorials</strong></td>
            <td height="24"><strong>Design Tutorials</strong></td>
        </tr>
        <tr>
            <td><a href="../Production_Firmware/Feature_Tutorials/01-Shadow_Driven_LED_State_Machine/">Shadow-Driven LED State Machine</a></td>
            <td><a href="../Production_Firmware/Design_Tutorials/01-Full_Event_Loop_Integration/">Full Event Loop Integration (Capstone)</a></td>
        </tr>
        <tr>
            <td><a href="../Production_Firmware/Feature_Tutorials/02-Edge_Alarms_With_Hysteresis/">Edge Alarms with Hysteresis</a></td>
            <td><a href="../Production_Firmware/Design_Tutorials/02-Add_A_New_Sensor_End_To_End/">Add a New Sensor End-to-End</a></td>
        </tr>
        <tr>
            <td><a href="../Production_Firmware/Feature_Tutorials/03-Batched_Telemetry_Publisher/">Batched Telemetry Publisher</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../Production_Firmware/Feature_Tutorials/04-AWS_IoT_Jobs_Health_Check/">AWS IoT Jobs — Health Check</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../Production_Firmware/Feature_Tutorials/05-Reject_Unsupported_Jobs/">Reject Unsupported Jobs (OTA Stub)</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../Production_Firmware/Feature_Tutorials/06-Factory_Reset/">Factory Reset</a></td>
            <td></td>
        </tr>
        <tr>
            <td><a href="../Production_Firmware/Feature_Tutorials/07-Link_Recovery_And_Backoff/">Link Recovery & Backoff</a></td>
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

Eğitimlerin çoğunun henüz yazılmış bir çözümü yok — bu depo hâlâ büyük
ölçüde bir müfredat iskeleti (yapı, öğrenme hedefleri, adım anahatları),
tamamlanmış bir kurs değil. Her eğitim, varsa kendi çalışan çözümünü,
`README.md`'sinin hemen yanındaki yerel bir `Sw/` klasöründe sunar;
güncel durum için o eğitimin kendi **Solution** (İngilizce) bölümüne
bakın.

Derlenebilir eğitimler, bu depoya tek kopya olarak eklenmiş tek bir ABOV
satıcı SDK'sını paylaşır ([`Generation/`](../Generation/), ~107 MB — neden
her eğitim için ayrı ayrı değil de paylaşılan tek kopya olduğu için o
klasörün kendi notuna, satıcının BSD-3-Clause şartları için paketlenmiş
`Document/LICENSE.md`'sine bakın), ve tek bir derleme/flaşlama CLI'ı
paylaşırlar: [`tools/tdf.py`](../tools/tdf.py) ("Tiremo Development
Framework" — ESP-IDF için `idf.py`'nin oynadığı ile aynı rol). Yalnızca
ilk seferde:

```sh
./install.sh          # arm-none-eabi-gcc, cmake, ninja'yı ve pyserial'li bir tdf_venv/'i kontrol eder / kurmayı önerir
source ./export.sh     # bu shell oturumu için tdf.py'yi PATH'e ekler ve tdf_venv/'i etkinleştirir (isteğe bağlı)
```

Ardından, herhangi bir eğitimin `Sw/` dizininden: `tdf.py build`,
`tdf.py flash`, `tdf.py monitor` (kartı SWD üzerinden resetler, sonra
UART log'unu seviyeye göre renk kodlu akıtır), `tdf.py clean`, `tdf.py
clean-device` (bağlı kartın çip flash'ını topluca siler). `tdf.py
build`/`flash`/`clean`/`clean-device` ince sarmalayıcılardır — kendi
başlarına hiç OpenOCD ya da derleyici mantığı taşımazlar, her komut
yalnızca o eğitimin `CMakeLists.txt`'indeki eşleşen `cmake --build build
[--target ...]`'i çalıştırır, ve `tdf.py` kendi dosya konumundan depoyu
bulur, bu yüzden `python tools/tdf.py build`, `export.sh` olsun ya da
olmasın aynı şekilde çalışır. `monitor` tek istisna: canlı bir UART'ı
okumak gerçek bir seri port gerektiriyor, ki CMake/OpenOCD'nin bu
konuda hiçbir kavramı yok, bu yüzden o kısım `pyserial` kullanıyor —
düz bir `pip install` yerine (çoğu güncel Python kurulumunda doğrudan
başarısız olur — PEP 668) `install.sh` tarafından ayrı bir `tdf_venv/`
içine kurulur, `export.sh` tarafından etkinleştirilir. cmd.exe,
PowerShell, bash ya da zsh'de aynı şekilde çalışır; Windows'ta
derlemek, flaşlamak ya da izlemek için Git Bash ya da WSL gerekmez
(`install.sh`/`export.sh`'in kendisi bash'tir — Windows kullanıcıları
bunları bir kez Git Bash/WSL'den çalıştırır, ya da araçları elle kurup
doğrudan `python tools\tdf.py build`'e geçer). Tam hikaye, ve bu deponun
neden elle yazılmış bir Makefile yerine CMake kullandığı için
[01-Hello_Blink'in Çözüm bölümüne](../Getting_Started/01-Hello_Blink/README.md#solution)
bakın.

Bir eğitimi takip etmek yerine kendi projenizi mi başlatıyorsunuz?
`tdf.py create-project <Name> [--dir PARENT_DIR]`, yeni, boş,
derlenebilir bir `<Name>/Sw/` oluşturur — `CMakeLists.txt`,
`Src/main.c`, ve boş bir `Src/prv_user_code.c`, bilinçli olarak minimal
(`board_config.h` yok, `Tiremo_Hal` yok — ihtiyaç duydukça bir
eğitimden kopyalayın). Bu depodaki paylaşılan `Generation/` SDK'sına
geri dönen göreli yolu, yeni klasörün gerçekte nereye indiğine göre
hesaplar, her `Getting_Started` eğitiminin kullandığı sabit üç-seviye-derin
düzeni varsaymak yerine, bu yüzden yalnızca `Getting_Started/` altında
değil, bu deponun içindeki her yerde çalışır.

- [x] [Getting_Started/01-Hello_Blink](../Getting_Started/01-Hello_Blink/) — derlenir, bağlanır, gerçek bir ABOV A34G43A Starter Kit'e SWD üzerinden flaşlandı ve doğrulandı
- [x] [Getting_Started/02-SysTick_Timing](../Getting_Started/02-SysTick_Timing/) — derlenir, bağlanır, gerçek bir ABOV A34G43A Starter Kit'e flaşlandı ve doğrulandı: iki LED de belirgin şekilde farklı hızlarda yanıp sönüyor
- [x] [Getting_Started/03-Debug_UART_Logging](../Getting_Started/03-Debug_UART_Logging/) — derlenir, bağlanır (iki debug seviyesinde, farklı ikili boyutları doğrulandı), SWD üzerinden flaşlandı ve doğrulandı, ve — Tutorial 04'te eklenen `tdf.py monitor` ile — gerçek UART çıktısı da gerçekten doğrulandı
- [x] [Getting_Started/04-Button_Read_And_Debounce](../Getting_Started/04-Button_Read_And_Debounce/) — derlenir, bağlanır, SWD üzerinden flaşlandı + doğrulandı; `tdf.py monitor`'ın önyükleme/LED log satırları gerçekten doğrulandı; buton-basma/debounce davranışının kendisi hâlâ bir insan parmağı gerektiriyor, henüz doğrulanmadı
- [x] [Getting_Started/05-Ten_LED_Pattern_Driver](../Getting_Started/05-Ten_LED_Pattern_Driver/) — derlenir, bağlanır, SWD üzerinden flaşlandı + doğrulandı; `tdf.py monitor`, SOLID→BLINK→PULSE otomatik döngüsünü gerçekten doğruladı, tam olarak 8 sn'lik aralığında; on LED'in tamamının görünür şekilde kademeli olup olmadığı ve buton ile tetiklenen ilerlemenin çalışıp çalışmadığı hâlâ kartı gözle görmek ve bir insan parmağı gerektiriyor

<p align="center"><img alt="" src="../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
