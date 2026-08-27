<p class="sphinxhide" align="right"><a>English</a> | <a href="docs-tr/Hardware.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="images/banner.png" width="100%">
      <h1>Hardware Overview</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Hardware Overview

Board-level reference: what's on the Tiremo Cortex board, how it's wired
to the MCU, and how a host PC talks to it. No firmware here — see
[Getting Started](./Getting_Started/) to start writing code against
this hardware.

## Board Block Diagram

```mermaid
graph TB
    classDef mcu fill:#2b6cb0,stroke:#1a365d,color:#ffffff,stroke-width:3px
    classDef onboard fill:#bee3f8,stroke:#2b6cb0,color:#1a365d,stroke-width:1px
    classDef external fill:#feebc8,stroke:#c05621,color:#652b06,stroke-width:1px
    classDef debug fill:#e9d8fd,stroke:#553c9a,color:#322659,stroke-width:1px
    classDef internal fill:#f0fff4,stroke:#38a169,color:#22543d,stroke-width:1px,stroke-dasharray: 4 3

    HOST(["Host PC"]):::debug
    PROBE["Onboard CMSIS-DAP<br/>Debug Probe"]:::debug

    MCU{{"ABOV A34G43x MCU<br/>Cortex-M4<br/>A34G43A YLN-A Starter Kit"}}:::mcu

    subgraph USERIO["User I/O"]
        LEDS["10x LED<br/>active-low"]:::onboard
        BTN["USER Button<br/>active-low"]:::onboard
    end

    subgraph SENSORS["Sensors — I2C Bus 2"]
        SHT["SHT40<br/>Temp / Humidity"]:::onboard
        LIS["LIS2DE12<br/>Accelerometer"]:::onboard
    end

    MIC["MP23ABS1 Mic<br/>analog"]:::onboard
    PWR["Supply Monitor<br/>VCORE bandgap"]:::internal
    ESP["ESP32 Module<br/>WiFi (AT commands)"]:::external

    HOST <-->|USB| PROBE
    PROBE <-->|SWD| MCU
    PROBE <-->|"UART0, 115200 8N1"| MCU
    MCU -->|GPIO| LEDS
    BTN -->|GPIO| MCU
    MCU <-->|I2C2| SHT
    MCU <-->|I2C2| LIS
    LIS -.->|"INT1 / INT2"| MCU
    MIC -->|ADC0| MCU
    PWR -.->|internal| MCU
    MCU <-->|UART2| ESP
```

<sub>Hexagon = MCU · rounded stadium = off-board host · dashed border =
internal-only, no external pin · exact port/pin numbers are in the
**Pinout Reference** table below, not repeated here.</sub>

## Host Connection

How a developer's PC actually reaches the MCU — no separate programmer
needed, the debug probe is on-board. One USB cable carries both paths
at once: SWD for flashing/debugging, and a virtual serial port for the
UART0 log console.

```mermaid
graph LR
    classDef mcu fill:#2b6cb0,stroke:#1a365d,color:#ffffff,stroke-width:3px
    classDef debug fill:#e9d8fd,stroke:#553c9a,color:#322659,stroke-width:1px

    HOST(["Host PC"]):::debug -->|USB cable| PROBE["Onboard CMSIS-DAP<br/>Debug Probe"]:::debug
    PROBE -->|SWD| MCU{{"ABOV A34G43x MCU"}}:::mcu
    PROBE -->|"USB-serial bridge<br/>UART0, 115200 8N1"| MCU
```

## Pinout Reference

| Port · Pin | Function | Notes |
|---|---|---|
| PA0 | Microphone analog input (AN0) | ADC0 channel 0, Timer1-triggered burst capture |
| PA4 | LIS2DE12 INT1 | Accelerometer interrupt |
| PA5 | LIS2DE12 INT2 | Accelerometer interrupt |
| PA8 | ESP32 UART2 RX | ESP32 TX → MCU RX |
| PA9 | ESP32 UART2 TX | MCU TX → ESP32 RX |
| PA12 | ESP32 power gate | Active-high (HIGH = powered) |
| PB4 | LED1 | Active-low |
| PB5 | LED4 | Active-low |
| PB6 | I2C Bus 2 SCL | Shared: SHT40 + LIS2DE12 |
| PB7 | I2C Bus 2 SDA | Shared: SHT40 + LIS2DE12 |
| PB9 | LED2 | Active-low |
| PB10 | LED5 | Active-low |
| PB11 | LED6 | Active-low |
| PB12 | LED7 | Active-low |
| PB13 | LED8 | Active-low |
| PB14 | LED9 | Active-low |
| PB15 | LED10 | Active-low |
| PC9 | USER button | Active-low |
| PF7 | LED3 | Active-low |
| — | Supply voltage monitor | Internal ADC1 channel 23 (VCORE bandgap); no external pin |
| — | Debug UART (UART0) | Fixed by the starter kit's onboard USB-serial bridge, not board-configurable |

<sub>Source of truth: the reference firmware's `board_config.h`. If your
own board revision differs, that file — not this page — is what your
code should follow.</sub>

<p align="center"><img alt="" src="images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
