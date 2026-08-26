<p class="sphinxhide" align="right"><a href="../Hardware.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../images/banner.png" width="100%">
      <h1>Donanım Genel Bakış</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + AWS IoT üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Donanım Genel Bakış

> Bu sayfa [İngilizce orijinalinin](../Hardware.md) Türkçe çevirisidir;
> içerik farklılaşırsa İngilizce sürüm esas alınır.

Kart seviyesinde referans: Tiremo Cortex kartında neler olduğu, MCU'ya
nasıl kablolandığı, ve bir host bilgisayarın kartla nasıl konuştuğu.
Burada üretici yazılımı yok — bu donanım üzerine kod yazmaya başlamak
için [Getting Started](../Getting_Started/README.md)'a bakın.

## Kart Blok Diyagramı

```mermaid
graph TB
    classDef mcu fill:#2b6cb0,stroke:#1a365d,color:#ffffff,stroke-width:3px
    classDef onboard fill:#bee3f8,stroke:#2b6cb0,color:#1a365d,stroke-width:1px
    classDef external fill:#feebc8,stroke:#c05621,color:#652b06,stroke-width:1px
    classDef debug fill:#e9d8fd,stroke:#553c9a,color:#322659,stroke-width:1px
    classDef internal fill:#f0fff4,stroke:#38a169,color:#22543d,stroke-width:1px,stroke-dasharray: 4 3

    HOST(["Host Bilgisayar"]):::debug
    PROBE["Onboard CMSIS-DAP<br/>Debug Probu"]:::debug

    MCU{{"ABOV A34G43x MCU<br/>Cortex-M4<br/>A34G43A YLN-A Starter Kit"}}:::mcu

    subgraph USERIO["Kullanıcı G/Ç"]
        LEDS["10x LED<br/>active-low"]:::onboard
        BTN["USER Butonu<br/>active-low"]:::onboard
    end

    subgraph SENSORS["Sensörler — I2C Bus 2"]
        SHT["SHT40<br/>Sıcaklık / Nem"]:::onboard
        LIS["LIS2DE12<br/>İvmeölçer"]:::onboard
    end

    MIC["MP23ABS1 Mikrofon<br/>analog"]:::onboard
    PWR["Besleme İzleyici<br/>VCORE bandgap"]:::internal
    ESP["ESP32 Modülü<br/>WiFi (AT komutları)"]:::external

    HOST <-->|USB| PROBE
    PROBE <-->|SWD| MCU
    PROBE <-->|"UART0, 115200 8N1"| MCU
    MCU -->|GPIO| LEDS
    BTN -->|GPIO| MCU
    MCU <-->|I2C2| SHT
    MCU <-->|I2C2| LIS
    LIS -.->|"INT1 / INT2"| MCU
    MIC -->|ADC0| MCU
    PWR -.->|dahili| MCU
    MCU <-->|UART2| ESP
```

<sub>Altıgen = MCU · yuvarlatılmış stadyum = kart dışı host · kesikli
kenarlık = yalnızca dahili, harici pin yok · tam port/pin numaraları
yukarıda tekrar edilmez, aşağıdaki **Pin Haritası Referansı**
tablosundadır.</sub>

## Host Bağlantısı

Bir geliştiricinin bilgisayarının MCU'ya gerçekte nasıl ulaştığı — ayrı
bir programlayıcıya gerek yok, debug probu kartın üzerinde. Tek bir USB
kablosu aynı anda iki yolu taşır: flaşlama/hata ayıklama için SWD, ve
UART0 log konsolu için sanal bir seri port.

```mermaid
graph LR
    classDef mcu fill:#2b6cb0,stroke:#1a365d,color:#ffffff,stroke-width:3px
    classDef debug fill:#e9d8fd,stroke:#553c9a,color:#322659,stroke-width:1px

    HOST(["Host Bilgisayar"]):::debug -->|USB kablosu| PROBE["Onboard CMSIS-DAP<br/>Debug Probu"]:::debug
    PROBE -->|SWD| MCU{{"ABOV A34G43x MCU"}}:::mcu
    PROBE -->|"USB-seri köprü<br/>UART0, 115200 8N1"| MCU
```

Tek bir USB kablosu aynı anda iki yolu taşır: flaşlama/hata ayıklama
için SWD, ve UART0 log konsolu için sanal bir seri port.

## Pin Haritası Referansı

| Port · Pin | İşlev | Notlar |
|---|---|---|
| PA0 | Mikrofon analog girişi (AN0) | ADC0 kanal 0, Timer1 tetiklemeli burst yakalama |
| PA4 | LIS2DE12 INT1 | İvmeölçer kesmesi |
| PA5 | LIS2DE12 INT2 | İvmeölçer kesmesi |
| PA8 | ESP32 UART2 RX | ESP32 TX → MCU RX |
| PA9 | ESP32 UART2 TX | MCU TX → ESP32 RX |
| PA12 | ESP32 güç kapısı | Active-high (HIGH = güçlü) |
| PB4 | LED1 | Active-low |
| PB5 | LED4 | Active-low |
| PB6 | I2C Bus 2 SCL | Paylaşılan: SHT40 + LIS2DE12 |
| PB7 | I2C Bus 2 SDA | Paylaşılan: SHT40 + LIS2DE12 |
| PB9 | LED2 | Active-low |
| PB10 | LED5 | Active-low |
| PB11 | LED6 | Active-low |
| PB12 | LED7 | Active-low |
| PB13 | LED8 | Active-low |
| PB14 | LED9 | Active-low |
| PB15 | LED10 | Active-low |
| PC9 | USER butonu | Active-low |
| PF7 | LED3 | Active-low |
| — | Besleme voltajı izleyici | Dahili ADC1 kanal 23 (VCORE bandgap); harici pin yok |
| — | Debug UART (UART0) | Starter kit'in onboard USB-seri köprüsü tarafından sabitlenmiş, kart üzerinden yapılandırılamaz |

<sub>Doğruluk kaynağı: referans üretici yazılımının `board_config.h`'i.
Kendi kart revizyonunuz farklıysa, bu sayfa değil o dosya kodunuzun
takip etmesi gereken şeydir.</sub>

<p align="center"><img alt="" src="../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
