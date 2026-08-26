Chipset Brief Description {#Chipset_Description}
=====

A31xxxx Sub Family
-----

### A31G21x

#### - Brief Description

 * CPU : ARM Cortex-M0+ Core
 * Interupt : Up to 32 peripheral interrupts
 * Code Flash 
  - A31G213 : 64KB Code Flash Memory
  - A31G212 : 32KB Code Flash Memory
 * SRAM : 6KB
 * Operation Frequency : Up To 48MHz
 * Clock
  - High Speed Internal oscillator (HSI) : 32MHz
  - Low Speed Internal oscillator (LSI) : 500KHz
  - High Speed External oscillator (HSE) : 2MHz ~ 16MHz
  - Low Speed External oscillator (LSE) : 32.768KHz
  - Phase-Locked Loop (PLL) : Up to 48MHz

#### - Support Peripheral List and Version

<table>
<tr>
    <th> <center>Group</center>                     <th> <center>Peripheral</center>                                                      <th> <center>Type</center>     <th> <center>Version</center> 
<tr> <td> System                                    <td> \ref HAL_SCU "System Control Unit (SCU)"                                         <td> <center>V</center>        <td> 01.00.01
<tr> <td> Port                                      <td> \ref HAL_PCU "Port Control Unit (PCU)"                                           <td> <center>V</center>        <td> 01.00.00
<tr> <td rowspan="2"> Memory                        <td> \ref HAL_CFMC "Code Flash Memory Controller (CFMC)"                              <td> <center>V</center>        <td> 02.00.00
<tr>                                                <td> \ref HPL_DMA "Direct Memory Access controller (DMA)"                             <td> <center>V</center>        <td> 01.00.00
<tr> <td rowspan="2"> Analog                        <td> \ref HAL_ADC "Analog to Digital Converter (ADC)"                                 <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_DAC "Digital to Analog Converter (DAC)"                                 <td> <center>V</center>        <td> 01.00.00
<tr> <td>             Security & Integrity          <td> \ref HAL_CRC "Cyclic Redundancy Check (CRC)"                                     <td> <center>V</center>        <td> 01.00.00
<tr> <td rowspan="4"> Connectivity & Communication  <td> \ref HAL_I2C "Inter-Integrated Circuit (I2C)"                                    <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_SPI "Serial Peripheral Interface (SPI)"                                 <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_UART "Universal Asynchronous Receiver Transmitter (UART)"               <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_USART "Universal Synchronous/Asynchronous Receiver Transmitter (USART)" <td> <center>F</center>        <td> 01.00.00
<tr> <td rowspan="5"> Timer                         <td> \ref HAL_TIMER1 "16-bit General Purpose Timer (TIMER1)"                          <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_TIMER2 "32-bit General Purpose Timer (TIMER2)"                          <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_TIMER3 "16-bit Multi-PWM Channel Timer (TIMER3)"                        <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_WDT "Watch Dog Timer (WDT)"                                             <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_WT "Watch Timer (WT)"                                                   <td> <center>F</center>        <td> 01.00.00
<tr> <td>             Display                       <td> \ref HAL_LED "Light Emitting Diode controller (LED)"                             <td> <center>V</center>        <td> 01.00.01
</table>

### A31G31x

#### - Brief Description

 * CPU : ARM Cortex-M0+ Core
 * Interupt : Up to 32 peripheral interrupts
 * Code Flash 
  - A31G316 : 256KB Code Flash Memory
  - A31G314 : 128KB Code Flash Memory
  - A31G313 : 64KB Code Flash Memory
 * SRAM : 16KB
 * Operation Frequency : Up To 48MHz
 * Clock
  - High Speed Internal oscillator (HSI) : 48MHz
  - Low Speed Internal oscillator (LSI) : 500KHz
  - High Speed External oscillator (HSE) : 2MHz ~ 16MHz
  - Low Speed External oscillator (LSE) : 32.768KHz
  - Phase-Locked Loop (PLL) : Up to 48MHz

#### - Support Peripheral List and Version

<table>
<tr>
    <th> <center>Group</center>                     <th> <center>Peripheral</center>                                                      <th> <center>Type</center>     <th> <center>Version</center> 
<tr> <td> System                                    <td> \ref HAL_SCU "System Control Unit (SCU)"                                         <td> <center>V</center>        <td> 01.00.00
<tr> <td> Port                                      <td> \ref HAL_PCU "Port Control Unit (PCU)"                                           <td> <center>V</center>        <td> 01.00.01
<tr> <td rowspan="2"> Memory                        <td> \ref HAL_CFMC "Code Flash Memory Controller (CFMC)"                              <td> <center>V</center>        <td> 02.00.00
<tr>                                                <td> \ref HPL_DMA "Direct Memory Access controller (DMA)"                             <td> <center>V</center>        <td> 01.00.01
<tr> <td rowspan="3"> Analog                        <td> \ref HAL_ADC "Analog to Digital Converter (ADC)"                                 <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_DAC "Digital to Analog Converter (DAC)"                                 <td> <center>V</center>        <td> 02.00.00
<tr>                                                <td> \ref HAL_CMP "Comparator (CMP)"                                                  <td> <center>V</center>        <td> 01.00.00
<tr> <td>             Security & Integrity          <td> \ref HAL_CRC "Cyclic Redundancy Check (CRC)"                                     <td> <center>V</center>        <td> 01.00.00
<tr> <td rowspan="3"> Connectivity & Communication  <td> \ref HAL_I2C "Inter-Integrated Circuit (I2C)"                                    <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_UART "Universal Asynchronous Receiver Transmitter (UART)"               <td> <center>V</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_USART "Universal Synchronous/Asynchronous Receiver Transmitter (USART)" <td> <center>F</center>        <td> 01.02.00
<tr> <td rowspan="5"> Timer                         <td> \ref HAL_TIMER1 "16-bit General Purpose Timer (TIMER1)"                          <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_TIMER2 "32-bit General Purpose Timer (TIMER2)"                          <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_TIMER3 "16-bit Multi-PWM Channel Timer (TIMER3)"                        <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_WDT "Watch Dog Timer (WDT)"                                             <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_WT "Watch Timer (WT)"                                                   <td> <center>F</center>        <td> 01.00.00
<tr> <td rowspan="2"> Display                       <td> \ref HAL_LED "Light Emitting Diode controller (LED)"                             <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_LCD "Liquid Crystal Display controller (LCD)"                           <td> <center>F</center>        <td> 01.00.00
</table>

### A31G32x

#### - Brief Description

 * CPU : ARM Cortex-M0+ Core
 * Interupt : Up to 32 peripheral interrupts
 * Code Flash 
  - A31G324 : 128KB Code Flash Memory
  - A31G323 : 64KB Code Flash Memory
 * SRAM : 16KB
 * Operation Frequency : Up To 48MHz
 * Clock
  - High Speed Internal oscillator (HSI) : 48MHz
  - Low Speed Internal oscillator (LSI) : 750KHz / 40KHz
  - High Speed External oscillator (HSE) : 2MHz ~ 16MHz
  - Low Speed External oscillator (LSE) : 32.768KHz
  - Phase-Locked Loop (PLL) : Up to 144MHz

#### - Support Peripheral List and Version

<table>
<tr>
    <th> <center>Group</center>                     <th> <center>Peripheral</center>                                                      <th> <center>Type</center>     <th> <center>Version</center> 
<tr> <td> System                                    <td> \ref HAL_SCU "System Control Unit (SCU)"                                         <td> <center>V</center>        <td> 01.00.03
<tr> <td> Port                                      <td> \ref HAL_PCU "Port Control Unit (PCU)"                                           <td> <center>V</center>        <td> 01.00.02
<tr> <td rowspan="3"> Memory                        <td> \ref HAL_CFMC "Code Flash Memory Controller (CFMC)"                              <td> <center>V</center>        <td> 02.00.00
<tr>                                                <td> \ref HPL_DMA "Direct Memory Access controller (DMA)"                             <td> <center>V</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_EBI "External Memory Interface (EBI)"                                   <td> <center>V</center>        <td> 01.00.00
<tr> <td rowspan="4"> Analog                        <td> \ref HAL_ADC "Analog to Digital Converter (ADC)"                                 <td> <center>V</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_DAC "Digital to Analog Converter (DAC)"                                 <td> <center>V</center>        <td> 02.00.00
<tr>                                                <td> \ref HAL_CMP "Comparator (CMP)"                                                  <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_TEMPSENS "Temperature Sensor (TempSens)"                                <td> <center>V</center>        <td> 01.00.00
<tr> <td>             Security & Integrity          <td> \ref HAL_CRC "Cyclic Redundancy Check (CRC)"                                     <td> <center>V</center>        <td> 01.00.00
<tr> <td rowspan="3"> Connectivity & Communication  <td> \ref HAL_I2C "Inter-Integrated Circuit (I2C)"                                    <td> <center>F</center>        <td> 01.01.00
<tr>                                                <td> \ref HAL_SPI "Serial Peripheral Interface (SPI)"                                 <td> <center>V</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_USART "Universal Synchronous/Asynchronous Receiver Transmitter (USART)" <td> <center>F</center>        <td> 01.01.00
<tr> <td rowspan="7"> Timer                         <td> \ref HAL_TIMER1 "16-bit General Purpose Timer (TIMER1)"                          <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_TIMER2 "32-bit General Purpose Timer (TIMER2)"                          <td> <center>F</center>        <td> 01.00.02
<tr>                                                <td> \ref HAL_TIMER3 "16-bit Multi-PWM Channel Timer (TIMER3)"                        <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_TIMER4 "16-bit Multi-Input Capture Timer (TIMER4)"                      <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_WDT "Watch Dog Timer (WDT)"                                             <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_WT "Watch Timer (WT)"                                                   <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_RTC "Real-Time Clock (RTC)"                                             <td> <center>V</center>        <td> 01.00.00
</table>

### A31G22x

#### - Brief Description

 * CPU : ARM Cortex-M0+ Core
 * Interupt : Up to 32 peripheral interrupts
 * Code Flash 
  - A31G226 : 256KB Code Flash Memory
  - A31G224 : 128KB Code Flash Memory
 * Data Flash : 32KB Data Flash Memory
 * SRAM : 20KB
 * Operation Frequency : Up To 48MHz
 * Clock
  - High Speed Internal oscillator (HSI) : 32MHz
  - Low Speed Internal oscillator (LSI) : 500KHz
  - High Speed External oscillator (HSE) : 1MHz ~ 16MHz
  - Low Speed External oscillator (LSE) : 32.768KHz
  - Phase-Locked Loop (PLL) : Up to 48MHz

#### - Support Peripheral List and Version

<table>
<tr>
    <th> <center>Group</center>                     <th> <center>Peripheral</center>                                                      <th> <center>Type</center>     <th> <center>Version</center> 
<tr> <td> System                                    <td> \ref HAL_SCU "System Control Unit (SCU)"                                         <td> <center>V</center>        <td> 01.00.02
<tr> <td> Port                                      <td> \ref HAL_PCU "Port Control Unit (PCU)"                                           <td> <center>F</center>        <td> 01.00.03
<tr> <td rowspan="2"> Memory                        <td> \ref HAL_FMC "Flash Memory Controller (FMC)"                                     <td> <center>V</center>        <td> 02.01.00
<tr>                                                <td> \ref HPL_DMA "Direct Memory Access controller (DMA)"                             <td> <center>V</center>        <td> 01.00.01
<tr> <td rowspan="4"> Analog                        <td> \ref HAL_ADC "Analog to Digital Converter (ADC)"                                 <td> <center>F</center>        <td> 01.01.01
<tr>                                                <td> \ref HAL_DAC "Digital to Analog Converter (DAC)"                                 <td> <center>V</center>        <td> 02.01.00
<tr>                                                <td> \ref HAL_CMP "Comparator (CMP)"                                                  <td> <center>V</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_TEMPSENS "Temperature Sensor (TempSens)"                                <td> <center>V</center>        <td> 01.00.01
<tr> <td>             Security & Integrity          <td> \ref HAL_CRC "Cyclic Redundancy Check (CRC)"                                     <td> <center>V</center>        <td> 02.00.01
<tr> <td rowspan="4"> Connectivity & Communication  <td> \ref HAL_I2C "Inter-Integrated Circuit (I2C)"                                    <td> <center>F</center>        <td> 01.02.00
<tr>                                                <td> \ref HAL_SPI "Serial Peripheral Interface (SPI)"                                 <td> <center>V</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_UART "Universal Asynchronous Receiver Transmitter (UART)"               <td> <center>V</center>        <td> 01.00.02
<tr>                                                <td> \ref HAL_USART "Universal Synchronous/Asynchronous Receiver Transmitter (USART)" <td> <center>F</center>        <td> 01.02.00
<tr> <td rowspan="5"> Timer                         <td> \ref HAL_TIMER1 "16-bit General Purpose Timer (TIMER1)"                          <td> <center>F</center>        <td> 01.01.00
<tr>                                                <td> \ref HAL_TIMER2 "32-bit General Purpose Timer (TIMER2)"                          <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_TIMER3 "16-bit Multi-PWM Channel Timer (TIMER3)"                        <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_WDT "Watch Dog Timer (WDT)"                                             <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_WT "Watch Timer (WT)"                                                   <td> <center>F</center>        <td> 01.00.00
<tr> <td>             Display                       <td> \ref HAL_LCD "Liquid Crystal Display controller (LCD)"                           <td> <center>F</center>        <td> 01.00.01
</table>

### A31G11x

#### - Brief Description

 * CPU : ARM Cortex-M0+ Core
 * Interupt : Up to 32 peripheral interrupts
 * Code Flash 
  - A31G111 : 16KB Code Flash Memory
  - A31G112 : 32KB Code Flash Memory
 * SRAM : 4KB
 * Operation Frequency : Up To 40MHz
 * Clock
  - High Speed Internal oscillator (HSI) : 2.5MHz ~ 40MHz
  - WDT Internal oscillator (LSI) : 40KHz
  - High Speed External oscillator (HSE) : 2MHz ~ 16MHz
  - Low Speed External oscillator (LSE) : 32.768KHz

#### - Support Peripheral List and Version

<table>
<tr>
    <th> <center>Group</center>                     <th> <center>Peripheral</center>                                                      <th> <center>Type</center>     <th> <center>Version</center> 
<tr> <td> System                                    <td> \ref HAL_SCU "System Control Unit (SCU)"                                         <td> <center>F</center>        <td> 01.00.00
<tr> <td> Port                                      <td> \ref HAL_PCU "Port Control Unit (PCU)"                                           <td> <center>F</center>        <td> 01.00.01
<tr> <td> Memory                                    <td> \ref HAL_CFMC "Code Flash Memory Controller (FMC)"                               <td> <center>F</center>        <td> 01.00.00
<tr> <td> Analog                                    <td> \ref HAL_ADC "Analog to Digital Converter (ADC)"                                 <td> <center>F</center>        <td> 01.00.02
<tr> <td> Security & Integrity                      <td> \ref HAL_CRC "Cyclic Redundancy Check (CRC)"                                     <td> <center>F</center>        <td> 01.00.00
<tr> <td rowspan="3"> Connectivity & Communication  <td> \ref HAL_I2C "Inter-Integrated Circuit (I2C)"                                    <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_UART "Universal Asynchronous Receiver Transmitter (UART)"               <td> <center>V</center>        <td> 01.00.05
<tr>                                                <td> \ref HAL_USART "Universal Synchronous/Asynchronous Receiver Transmitter (USART)" <td> <center>F</center>        <td> 01.00.01
<tr> <td rowspan="5"> Timer                         <td> \ref HAL_TIMER1 "16-bit General Purpose Timer (TIMER1)"                          <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_TIMER2 "32-bit General Purpose Timer (TIMER2)"                          <td> <center>F</center>        <td> 01.00.03
<tr>                                                <td> \ref HAL_TIMER3 "16-bit Multi-PWM Channel Timer (TIMER3)"                        <td> <center>F</center>        <td> 01.00.03
<tr>                                                <td> \ref HAL_WDT "Watch Dog Timer (WDT)"                                             <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_WT "Watch Timer (WT)"                                                   <td> <center>F</center>        <td> 01.00.01
<tr> <td>             Display                       <td> \ref HAL_LCD "Liquid Crystal Display controller (LCD)"                           <td> <center>F</center>        <td> 01.00.02
</table>

### A31G12x

#### - Brief Description

 * CPU : ARM Cortex-M0+ Core
 * Interupt : Up to 32 peripheral interrupts
 * Code Flash 
  - A31G122 : 32KB Code Flash Memory
  - A31G123 : 64KB Code Flash Memory
 * SRAM : 6KB
 * Operation Frequency : Up To 40MHz
 * Clock
  - High Speed Internal oscillator (HSI) : 2.5MHz ~ 40MHz
  - WDT Internal oscillator (LSI) : 40KHz
  - High Speed External oscillator (HSE) : 2MHz ~ 16MHz
  - Low Speed External oscillator (LSE) : 32.768KHz

#### - Support Peripheral List and Version

<table>
<tr>
    <th> <center>Group</center>                     <th> <center>Peripheral</center>                                                      <th> <center>Type</center>     <th> <center>Version</center> 
<tr> <td> System                                    <td> \ref HAL_SCU "System Control Unit (SCU)"                                         <td> <center>F</center>        <td> 01.00.00
<tr> <td> Port                                      <td> \ref HAL_PCU "Port Control Unit (PCU)"                                           <td> <center>F</center>        <td> 01.00.01
<tr> <td> Memory                                    <td> \ref HAL_CFMC "Code Flash Memory Controller (FMC)"                               <td> <center>F</center>        <td> 01.00.00
<tr> <td> Analog                                    <td> \ref HAL_ADC "Analog to Digital Converter (ADC)"                                 <td> <center>F</center>        <td> 01.00.02
<tr> <td> Security & Integrity                      <td> \ref HAL_CRC "Cyclic Redundancy Check (CRC)"                                     <td> <center>F</center>        <td> 01.00.00
<tr> <td rowspan="3"> Connectivity & Communication  <td> \ref HAL_I2C "Inter-Integrated Circuit (I2C)"                                    <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_UART "Universal Asynchronous Receiver Transmitter (UART)"               <td> <center>V</center>        <td> 01.00.05
<tr>                                                <td> \ref HAL_USART "Universal Synchronous/Asynchronous Receiver Transmitter (USART)" <td> <center>F</center>        <td> 01.00.01
<tr> <td rowspan="5"> Timer                         <td> \ref HAL_TIMER1 "16-bit General Purpose Timer (TIMER1)"                          <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_TIMER2 "32-bit General Purpose Timer (TIMER2)"                          <td> <center>F</center>        <td> 01.00.03
<tr>                                                <td> \ref HAL_TIMER3 "16-bit Multi-PWM Channel Timer (TIMER3)"                        <td> <center>F</center>        <td> 01.00.03
<tr>                                                <td> \ref HAL_WDT "Watch Dog Timer (WDT)"                                             <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_WT "Watch Timer (WT)"                                                   <td> <center>F</center>        <td> 01.00.01
<tr> <td>             Display                       <td> \ref HAL_LCD "Liquid Crystal Display controller (LCD)"                           <td> <center>F</center>        <td> 01.00.02
</table>

### A31T21x

#### - Brief Description

 * CPU : ARM Cortex-M0+ Core
 * Interupt : Up to 32 peripheral interrupts
 * Code Flash 
  - A31T214 : 128KB Code Flash Memory
  - A31T216 : 256KB Code Flash Memory
 * SRAM : 16KB
 * Operation Frequency : Up To 48MHz
 * Clock
  - High Speed Internal oscillator (HSI) : 32MHz
  - Low Speed Internal oscillator (LSI) : 500KHz
  - High Speed External oscillator (HSE) : 2MHz ~ 16MHz
  - Low Speed External oscillator (LSE) : 32.768KHz
  - Phase-Locked Loop (PLL) : Up to 48MHz

#### - Support Peripheral List and Version

<table>
<tr>
    <th> <center>Group</center>                     <th> <center>Peripheral</center>                                                      <th> <center>Type</center>     <th> <center>Version</center> 
<tr> <td> System                                    <td> \ref HAL_SCU "System Control Unit (SCU)"                                         <td> <center>V</center>        <td> 01.00.04
<tr> <td> Port                                      <td> \ref HAL_PCU "Port Control Unit (PCU)"                                           <td> <center>V</center>        <td> 01.00.04
<tr> <td rowspan="2"> Memory                        <td> \ref HAL_CFMC "Code Flash Memory Controller (CFMC)"                              <td> <center>V</center>        <td> 02.00.00
<tr>                                                <td> \ref HPL_DMA "Direct Memory Access controller (DMA)"                             <td> <center>V</center>        <td> 01.00.00
<tr> <td>             Analog                        <td> \ref HAL_ADC "Analog to Digital Converter (ADC)"                                 <td> <center>F</center>        <td> 01.00.00
<tr> <td>             Security & Integrity          <td> \ref HAL_CRC "Cyclic Redundancy Check (CRC)"                                     <td> <center>V</center>        <td> 01.00.00
<tr> <td rowspan="4"> Connectivity & Communication  <td> \ref HAL_I2C "Inter-Integrated Circuit (I2C)"                                    <td> <center>F</center>        <td> 01.02.00
<tr>                                                <td> \ref HAL_SPI "Serial Peripheral Interface (SPI)"                                 <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_UART "Universal Asynchronous Receiver Transmitter (UART)"               <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_USART "Universal Synchronous/Asynchronous Receiver Transmitter (USART)" <td> <center>F</center>        <td> 01.00.00
<tr> <td rowspan="5"> Timer                         <td> \ref HAL_TIMER1 "16-bit General Purpose Timer (TIMER1)"                          <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_TIMER2 "32-bit General Purpose Timer (TIMER2)"                          <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_TIMER3 "16-bit Multi-PWM Channel Timer (TIMER3)"                        <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_WDT "Watch Dog Timer (WDT)"                                             <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_WT "Watch Timer (WT)"                                                   <td> <center>F</center>        <td> 01.00.00
<tr> <td rowspan="2"> Display                       <td> \ref HAL_LED "Light Emitting Diode controller (LED)"                             <td> <center>V</center>        <td> 01.01.00
<tr>                                                <td> \ref HAL_LCD "Liquid Crystal Display controller (LCD)"                           <td> <center>F</center>        <td> 01.00.00
</table>

### A31L21x

#### - Brief Description

 * CPU : ARM Cortex-M0+ Core
 * Interupt : Up to 32 peripheral interrupts
 * Code Flash 
  - A31L213 : 64KB Code Flash Memory
  - A31L214 : 128KB Code Flash Memory
 * Data Flash : 4KB Data Flash Memory
 * SRAM : 20KB
 * Operation Frequency : Up To 32MHz
 * Clock
  - High Speed Internal oscillator (HSI) : 2MHz ~ 32MHz
  - WDT Internal oscillator (LSI) : 40KHz
  - High Speed External oscillator (HSE) : X-TAL(2MHz ~ 16MHz), External Clock (2MHz to 32MHz)
  - Low Speed External oscillator (LSE) : X-TAL 32.768KHz

#### - Support Peripheral List and Version

<table>
<tr>
    <th> <center>Group</center>                     <th> <center>Peripheral</center>                                                      <th> <center>Type</center>     <th> <center>Version</center> 
<tr> <td> System                                    <td> \ref HAL_SCU "System Control Unit (SCU)"                                         <td> <center>F</center>        <td> 01.01.00
<tr> <td> Port                                      <td> \ref HAL_PCU "Port Control Unit (PCU)"                                           <td> <center>F</center>        <td> 01.00.00
<tr> <td> Memory                                    <td> \ref HAL_FMC "Flash Memory Controller (FMC)"                                     <td> <center>F</center>        <td> 01.00.00
<tr> <td rowspan="3"> Analog                        <td> \ref HAL_ADC "Analog to Digital Converter (ADC)"                                 <td> <center>F</center>        <td> 02.00.00
<tr>                                                <td> \ref HAL_TEMPSENS "Temperature Sensor (TempSens)"                                <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_CMP "Comparator (CMP)"                                                  <td> <center>F</center>        <td> 01.00.00
<tr> <td rowspan="3"> Security & Integrity          <td> \ref HAL_CRC "Cyclic Redundancy Check (CRC)"                                     <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_AES "Advanced Encryption Standard (AES)"                                <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_TRNG "True Random Number Generator (TRNG)"                              <td> <center>F</center>        <td> 01.00.00
<tr> <td rowspan="5"> Connectivity & Communication  <td> \ref HAL_I2C "Inter-Integrated Circuit (I2C)"                                    <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_SPI "Serial Peripheral Interface (SPI)"                                 <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_UART "Universal Asynchronous Receiver Transmitter (UART)"               <td> <center>V</center>        <td> 01.00.05
<tr>                                                <td> \ref HAL_USART "Universal Synchronous/Asynchronous Receiver Transmitter (USART)" <td> <center>F</center>        <td> 01.03.00
<tr>                                                <td> \ref HAL_LPUART "Low Power Universal Asynchronous Receiver Transmitter (LPUART)" <td> <center>F</center>        <td> 01.00.00
<tr> <td rowspan="6"> Timer                         <td> \ref HAL_TIMER1 "16-bit General Purpose Timer (TIMER1)"                          <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_TIMER4E "16-bit 2-PWM Channel Timer (TIMER4E)"                          <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_TIMER5 "16-bit Multi-Capture Source Timer (TIMER5)"                     <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_TIMER6 "16-bit 100Hz Clock Source Timer (TIMER6)"                       <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_WDT "Watch Dog Timer (WDT)"                                             <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_RTC "Real-Time Clock (RTC)"                                             <td> <center>F</center>        <td> 01.00.00
<tr> <td> Display                                   <td> \ref HAL_LCD "Liquid Crystal Display controller (LCD)"                           <td> <center>F</center>        <td> 01.00.03
</table>

### A31L12x

#### - Brief Description

 * CPU : ARM Cortex-M0+ Core
 * Interupt : Up to 32 peripheral interrupts
 * Code Flash 
  - A31L122 : 32KB Code Flash Memory
  - A31L123 : 64KB Code Flash Memory
 * SRAM : 8KB
 * Operation Frequency : Up To 32MHz
 * Clock
  - High Speed Internal oscillator (HSI) : 2MHz ~ 32MHz
  - WDT Internal oscillator (LSI) : 40KHz
  - High Speed External oscillator (HSE) : X-TAL(2MHz ~ 16MHz), External Clock (2MHz to 32MHz)
  - Low Speed External oscillator (LSE) : X-TAL 32.768KHz

#### - Support Peripheral List and Version

<table>
<tr>
    <th> <center>Group</center>                     <th> <center>Peripheral</center>                                                      <th> <center>Type</center>     <th> <center>Version</center> 
<tr> <td> System                                    <td> \ref HAL_SCU "System Control Unit (SCU)"                                         <td> <center>F</center>        <td> 01.01.00
<tr> <td> Port                                      <td> \ref HAL_PCU "Port Control Unit (PCU)"                                           <td> <center>F</center>        <td> 01.00.00
<tr> <td> Memory                                    <td> \ref HAL_CFMC "Code Flash Memory Controller (FMC)"                               <td> <center>F</center>        <td> 01.00.00
<tr> <td rowspan="2"> Analog                        <td> \ref HAL_ADC "Analog to Digital Converter (ADC)"                                 <td> <center>F</center>        <td> 02.00.00
<tr>                                                <td> \ref HAL_CMP "Comparator (CMP)"                                                  <td> <center>F</center>        <td> 01.00.00
<tr> <td> Security & Integrity                      <td> \ref HAL_CRC "Cyclic Redundancy Check (CRC)"                                     <td> <center>F</center>        <td> 01.00.01
<tr> <td rowspan="5"> Connectivity & Communication  <td> \ref HAL_I2C "Inter-Integrated Circuit (I2C)"                                    <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_SPI "Serial Peripheral Interface (SPI)"                                 <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_UART "Universal Asynchronous Receiver Transmitter (UART)"               <td> <center>V</center>        <td> 01.00.05
<tr>                                                <td> \ref HAL_USART "Universal Synchronous/Asynchronous Receiver Transmitter (USART)" <td> <center>F</center>        <td> 01.03.00
<tr>                                                <td> \ref HAL_LPUART "Low Power Universal Asynchronous Receiver Transmitter (LPUART)" <td> <center>F</center>        <td> 01.00.00
<tr> <td rowspan="4"> Timer                         <td> \ref HAL_TIMER4E "16-bit 2-PWM Channel Timer (TIMER4E)"                          <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_TIMER5 "16-bit Multi-Capture Source Timer (TIMER5)"                     <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_WDT "Watch Dog Timer (WDT)"                                             <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_RTC "Real-Time Clock (RTC)"                                             <td> <center>F</center>        <td> 01.00.00
<tr> <td> Display                                   <td> \ref HAL_LCD "Liquid Crystal Display controller (LCD)"                           <td> <center>F</center>        <td> 01.00.02
</table>

### A31L22x

#### - Brief Description

 * CPU : ARM Cortex-M0+ Core
 * Interupt : Up to 32 peripheral interrupts
 * Code Flash 
  - A31L221 : 16KB Code Flash Memory
  - A31L222 : 32KB Code Flash Memory
 * SRAM : 4KB
 * Operation Frequency : Up To 32MHz
 * Clock
  - High Speed Internal oscillator (HSI) : 2MHz ~ 32MHz
  - WDT Internal oscillator (LSI) : 40KHz
  - High Speed External oscillator (HSE) : X-TAL(2MHz ~ 16MHz), External Clock (2MHz to 32MHz)
  - Low Speed External oscillator (LSE) : X-TAL 32.768KHz

#### - Support Peripheral List and Version

<table>
<tr>
    <th> <center>Group</center>                     <th> <center>Peripheral</center>                                                      <th> <center>Type</center>     <th> <center>Version</center> 
<tr> <td> System                                    <td> \ref HAL_SCU "System Control Unit (SCU)"                                         <td> <center>F</center>        <td> 01.01.00
<tr> <td> Port                                      <td> \ref HAL_PCU "Port Control Unit (PCU)"                                           <td> <center>F</center>        <td> 01.00.00
<tr> <td> Memory                                    <td> \ref HAL_CFMC "Code Flash Memory Controller (FMC)"                               <td> <center>F</center>        <td> 01.00.00
<tr> <td rowspan="3"> Analog                        <td> \ref HAL_ADC "Analog to Digital Converter (ADC)"                                 <td> <center>F</center>        <td> 02.00.00
<tr>                                                <td> \ref HAL_CMP "Comparator (CMP)"                                                  <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_TEMPSENS "Temperature Sensor (TempSens)"                                <td> <center>F</center>        <td> 01.00.00
<tr> <td> Security & Integrity                      <td> \ref HAL_CRC "Cyclic Redundancy Check (CRC)"                                     <td> <center>F</center>        <td> 01.00.01
<tr> <td rowspan="5"> Connectivity & Communication  <td> \ref HAL_I2C "Inter-Integrated Circuit (I2C)"                                    <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_SPI "Serial Peripheral Interface (SPI)"                                 <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_UART "Universal Asynchronous Receiver Transmitter (UART)"               <td> <center>V</center>        <td> 01.00.05
<tr>                                                <td> \ref HAL_USART "Universal Synchronous/Asynchronous Receiver Transmitter (USART)" <td> <center>F</center>        <td> 01.03.00
<tr>                                                <td> \ref HAL_LPUART "Low Power Universal Asynchronous Receiver Transmitter (LPUART)" <td> <center>F</center>        <td> 01.00.00
<tr> <td rowspan="5"> Timer                         <td> \ref HAL_TIMER4E "16-bit 2-PWM Channel Timer (TIMER4E)"                          <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_TIMER5 "16-bit Multi-Capture Source Timer (TIMER5)"                     <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_TIMER6 "16-bit 100Hz Clock Source Timer (TIMER6)"                       <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_WDT "Watch Dog Timer (WDT)"                                             <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_RTC "Real-Time Clock (RTC)"                                             <td> <center>F</center>        <td> 01.00.00
</table>

### A31C14x

#### - Brief Description

 * CPU : ARM Cortex-M0+ Core
 * Interupt : Up to 32 peripheral interrupts
 * Code Flash 
  - A31C144 : 128KB Code Flash Memory
  - A31C143 : 64KB Code Flash Memory
 * SRAM : 8KB
 * Operation Frequency : Up To 32MHz
 * Clock
  - High Speed Internal oscillator (HSI) : 32MHz
  - WDT Internal oscillator (LSI) : 32KHz
  - High Speed External oscillator (HSE) : 4MHz ~ 16MHz
  - Low Speed External oscillator (LSE) : 32.768KHz

#### - Support Peripheral List and Version

<table>
<tr>
    <th> <center>Group</center>                     <th> <center>Peripheral</center>                                                      <th> <center>Type</center>     <th> <center>Version</center> 
<tr> <td> System                                    <td> \ref HAL_SCU "System Control Unit (SCU)"                                         <td> <center>V</center>        <td> 04.00.00
<tr> <td> Port                                      <td> \ref HAL_PCU "Port Control Unit (PCU)"                                           <td> <center>V</center>        <td> 02.00.03
<tr> <td> Memory                                    <td> \ref HAL_CFMC "Code Flash Memory Controller (CFMC)"                              <td> <center>V</center>        <td> 03.01.01
<tr> <td rowspan="2"> Analog                        <td> \ref HAL_ADC "Analog to Digital Converter (ADC)"                                 <td> <center>V</center>        <td> 01.00.02
<tr>                                                <td> \ref HAL_CMP "Comparator (CMP)"                                                  <td> <center>V</center>        <td> 02.00.01
<tr> <td> Security & Integrity                      <td> \ref HAL_CRC "Cyclic Redundancy Check (CRC)"                                     <td> <center>V</center>        <td> 02.00.03
<tr> <td rowspan="3"> Connectivity & Communication  <td> \ref HAL_I2C "Inter-Integrated Circuit (I2C)"                                    <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_UART "Universal Asynchronous Receiver Transmitter (UART)"               <td> <center>V</center>        <td> 01.00.03
<tr>                                                <td> \ref HAL_USART "Universal Synchronous/Asynchronous Receiver Transmitter (USART)" <td> <center>F</center>        <td> 01.02.01
<tr> <td rowspan="3"> Timer                         <td> \ref HAL_TIMER1 "16-bit General Purpose Timer (TIMER1)"                          <td> <center>V</center>        <td> 01.02.00
<tr>                                                <td> \ref HAL_WDT "Watch Dog Timer (WDT)"                                             <td> <center>V</center>        <td> 01.00.02
<tr>                                                <td> \ref HAL_FRT "Free Run Timer (FRT)"                                             <td> <center>V</center>        <td> 01.00.00
</table>

### A31C12x

#### - Brief Description

 * CPU : ARM Cortex-M0+ Core
 * Interrupt : Up to 32 peripheral interrupts
 * Code Flash 
  - A31C122 : 32KB Code Flash Memory
  - A31C121 : 16KB Code Flash Memory
 * SRAM : 6KB
 * Operation Frequency : Up To 32MHz
 * Clock
  - High Speed Internal oscillator (HSI) : 32MHz
  - WDT Internal oscillator (LSI) : 32KHz
  - High Speed External oscillator (HSE) : 4MHz ~ 16MHz
  - Low Speed External oscillator (LSE) : 32.768KHz

#### - Support Peripheral List and Version

<table>
<tr>
    <th> <center>Group</center>                     <th> <center>Peripheral</center>                                                      <th> <center>Type</center>     <th> <center>Version</center> 
<tr> <td> System                                    <td> \ref HAL_SCU "System Control Unit (SCU)"                                         <td> <center>V</center>        <td> 04.00.00
<tr> <td> Port                                      <td> \ref HAL_PCU "Port Control Unit (PCU)"                                           <td> <center>V</center>        <td> 02.00.03
<tr> <td> Memory                                    <td> \ref HAL_CFMC "Code Flash Memory Controller (CFMC)"                              <td> <center>V</center>        <td> 03.01.01
<tr> <td> Analog                                    <td> \ref HAL_ADC "Analog to Digital Converter (ADC)"                                 <td> <center>V</center>        <td> 01.00.02
<tr> <td> Security & Integrity                      <td> \ref HAL_CRC "Cyclic Redundancy Check (CRC)"                                     <td> <center>V</center>        <td> 02.00.03
<tr> <td rowspan="3"> Connectivity & Communication  <td> \ref HAL_I2C "Inter-Integrated Circuit (I2C)"                                    <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_UART "Universal Asynchronous Receiver Transmitter (UART)"               <td> <center>V</center>        <td> 01.00.08
<tr>                                                <td> \ref HAL_USART "Universal Synchronous/Asynchronous Receiver Transmitter (USART)" <td> <center>F</center>        <td> 01.02.01
<tr> <td rowspan="3"> Timer                         <td> \ref HAL_TIMER1 "16-bit General Purpose Timer (TIMER1)"                          <td> <center>V</center>        <td> 01.02.00
<tr>                                                <td> \ref HAL_WDT "Watch Dog Timer (WDT)"                                             <td> <center>V</center>        <td> 01.00.02
<tr>                                                <td> \ref HAL_FRT "Free Run Timer (FRT)"                                             <td> <center>V</center>        <td> 01.00.00
</table>

### A31G33x

#### - Brief Description

 * CPU : ARM Cortex-M0+ Core
 * Interupt : Up to 32 peripheral interrupts
 * Code Flash 
  - A31G336 : 256KB Code Flash Memory
 * SRAM : 32KB
 * Operation Frequency : Up To 48MHz
 * Clock
  - High Speed Internal oscillator (HSI) : 32MHz
  - WDT Internal oscillator (LSI) : 40KHz
  - High Speed External oscillator (HSE) : 2MHz to 16MHz
  - Low Speed External oscillator (LSE) : 32.768KHz
  - Phase-Locked Loop (PLL) : Up to 48MHz

#### - Support Peripheral List and Version

<table>
<tr>
    <th> <center>Group</center>                     <th> <center>Peripheral</center>                                                      <th> <center>Type</center>     <th> <center>Version</center> 
<tr> <td> System                                    <td> \ref HAL_SCU "System Control Unit (SCU)"                                         <td> <center>F</center>        <td> 02.00.00
<tr> <td> Port                                      <td> \ref HAL_PCU "Port Control Unit (PCU)"                                           <td> <center>F</center>        <td> 01.00.02
<tr> <td> Memory                                    <td> \ref HAL_CFMC "Code Flash Memory Controller (CFMC)"                              <td> <center>F</center>        <td> 01.00.01
<tr> <td rowspan="3"> Analog                        <td> \ref HAL_ADC "Analog to Digital Converter (ADC)"                                 <td> <center>F</center>        <td> 02.00.01
<tr>                                                <td> \ref HAL_TEMPSENS "Temperature Sensor (TempSens)"                                <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_CMP "Comparator (CMP)"                                                  <td> <center>F</center>        <td> 01.00.01
<tr> <td> Security & Integrity                      <td> \ref HAL_CRC "Cyclic Redundancy Check (CRC)"                                     <td> <center>F</center>        <td> 01.00.00
<tr> <td rowspan="3"> Connectivity & Communication  <td> \ref HAL_I2C "Inter-Integrated Circuit (I2C)"                                    <td> <center>F</center>        <td> 01.00.02
<tr>                                                <td> \ref HAL_SPI "Serial Peripheral Interface (SPI)"                                 <td> <center>F</center>        <td> 01.00.02
<tr>                                                <td> \ref HAL_USART "Universal Synchronous/Asynchronous Receiver Transmitter (USART)" <td> <center>F</center>        <td> 01.03.01
<tr> <td rowspan="4"> Timer                         <td> \ref HAL_TIMER1 "16-bit General Purpose Timer (TIMER1)"                          <td> <center>F</center>        <td> 01.00.02
<tr>                                                <td> \ref HAL_TIMER2 "32-bit General Purpose Timer (TIMER2)"                          <td> <center>F</center>        <td> 01.00.04
<tr>                                                <td> \ref HAL_WDT "Watch Dog Timer (WDT)"                                             <td> <center>F</center>        <td> 01.00.02
<tr>                                                <td> \ref HAL_RTC "Real-Time Clock (RTC)"                                             <td> <center>F</center>        <td> 01.00.01
</table>

### A31S13x

#### - Brief Description

 * CPU : ARM Cortex-M0+ Core
 * Interupt : Up to 32 peripheral interrupts
 * Code Flash 
  - A31S134 : 128KB Code Flash Memory
 * Data Flash : 4KB Data Flash Memory
 * SRAM : 16KB
 * Operation Frequency : Up To 32MHz
 * Clock
  - High Speed Internal oscillator (HSI) : 32MHz
  - WDT Internal oscillator (LSI) : 40KHz
  - High Speed External oscillator (HSE) : 2MHz to 16MHz
  - Low Speed External oscillator (LSE) : 32.768KHz

#### - Support Peripheral List and Version

<table>
<tr>
    <th> <center>Group</center>                     <th> <center>Peripheral</center>                                                      <th> <center>Type</center>     <th> <center>Version</center> 
<tr> <td> System                                    <td> \ref HAL_SCU "System Control Unit (SCU)"                                         <td> <center>F</center>        <td> 01.00.01
<tr> <td> Port                                      <td> \ref HAL_PCU "Port Control Unit (PCU)"                                           <td> <center>F</center>        <td> 01.00.00
<tr> <td> Memory                                    <td> \ref HAL_FMC "Flash Memory Controller (FMC)"                                     <td> <center>F</center>        <td> 01.00.00
<tr> <td> Analog                                    <td> \ref HAL_ADC "Analog to Digital Converter (ADC)"                                 <td> <center>F</center>        <td> 01.00.02
<tr> <td> Security & Integrity                      <td> \ref HAL_CRC "Cyclic Redundancy Check (CRC)"                                     <td> <center>F</center>        <td> 01.00.00
<tr> <td rowspan="4"> Connectivity & Communication  <td> \ref HAL_I2C "Inter-Integrated Circuit (I2C)"                                    <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_SPI "Serial Peripheral Interface (SPI)"                                 <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_UART "Universal Asynchronous Receiver Transmitter (UART)"               <td> <center>V</center>        <td> 01.00.05
<tr>                                                <td> \ref HAL_USART "Universal Synchronous/Asynchronous Receiver Transmitter (USART)" <td> <center>F</center>        <td> 01.03.00
<tr> <td rowspan="4"> Timer                         <td> \ref HAL_TIMER1 "16-bit General Purpose Timer (TIMER1)"                          <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_TIMER2 "32-bit General Purpose Timer (TIMER2)"                          <td> <center>F</center>        <td> 01.00.03
<tr>                                                <td> \ref HAL_WDT "Watch Dog Timer (WDT)"                                             <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_WT "Watch Timer (WT)"                                                   <td> <center>F</center>        <td> 01.00.01
</table>

### A31G34x

#### - Brief Description

 * CPU : ARM Cortex-M0+ Core
 * Interupt : Up to 32 peripheral interrupts
 * Code Flash 
  - A31G346 : 256KB Code Flash Memory
 * SRAM : 32KB
 * Operation Frequency : Up To 48MHz
 * Clock
  - High Speed Internal oscillator (HSI) : 32MHz
  - WDT Internal oscillator (LSI) : 40KHz
  - High Speed External oscillator (HSE) : 2MHz to 16MHz
  - Low Speed External oscillator (LSE) : 32.768KHz
  - Phase-Locked Loop (PLL) : Up to 48MHz

#### - Support Peripheral List and Version

<table>
<tr>
    <th> <center>Group</center>                     <th> <center>Peripheral</center>                                                      <th> <center>Type</center>     <th> <center>Version</center> 
<tr> <td> System                                    <td> \ref HAL_SCU "System Control Unit (SCU)"                                         <td> <center>F</center>        <td> 02.00.00
<tr> <td> Port                                      <td> \ref HAL_PCU "Port Control Unit (PCU)"                                           <td> <center>F</center>        <td> 01.00.00
<tr> <td> Memory                                    <td> \ref HAL_CFMC "Code Flash Memory Controller (CFMC)"                              <td> <center>F</center>        <td> 01.00.01
<tr> <td rowspan="2"> Analog                        <td> \ref HAL_ADC "Analog to Digital Converter (ADC)"                                 <td> <center>F</center>        <td> 02.00.01
<tr>                                                <td> \ref HAL_TEMPSENS "Temperature Sensor (TempSens)"                                <td> <center>F</center>        <td> 01.00.01
<tr> <td> Security & Integrity                      <td> \ref HAL_CRC "Cyclic Redundancy Check (CRC)"                                     <td> <center>F</center>        <td> 01.00.00
<tr> <td rowspan="3"> Connectivity & Communication  <td> \ref HAL_I2C "Inter-Integrated Circuit (I2C)"                                    <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_SPI "Serial Peripheral Interface (SPI)"                                 <td> <center>F</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_USART "Universal Synchronous/Asynchronous Receiver Transmitter (USART)" <td> <center>F</center>        <td> 01.03.01
<tr> <td rowspan="4"> Timer                         <td> \ref HAL_TIMER1 "16-bit General Purpose Timer (TIMER1)"                          <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_TIMER2 "32-bit General Purpose Timer (TIMER2)"                          <td> <center>F</center>        <td> 01.00.03
<tr>                                                <td> \ref HAL_WDT "Watch Dog Timer (WDT)"                                             <td> <center>F</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_RTC "Real-Time Clock (RTC)"                                             <td> <center>F</center>        <td> 01.00.00
</table>

A33xxxx Sub Family
-----

### A33G52x

#### - Brief Description

 * CPU : ARM Cortex-M3 Core
 * Interupt : Up to 64 peripheral interrupts
 * Code Flash 
  - A33G527 : 384KB Code Flash Memory
  - A33G526 : 256KB Code Flash Memory
  - A33G524 : 128KB Code Flash Memory
 * Data Flash : 32KB Data Flash Memory
 * SRAM : 24KB
 * Operation Frequency : Up To 75MHz
 * Clock
  - High Speed Internal oscillator (HSI:IOSC16) : 16MHz
  - Low Speed Internal oscillator (LSI:RINGOSC) : 1MHz
  - High Speed External oscillator (HSE:MXOSC) : 4MHz ~ 10MHz
  - Low Speed External oscillator (LSE:SXOSC) : 32.768KHz
  - Phase-Locked Loop (PLL) : Up to 75MHz

#### - Support Peripheral List and Version

<table>
<tr>
    <th> <center>Group</center>                     <th> <center>Peripheral</center>                                         <th> <center>Type</center>     <th> <center>Version</center> 
<tr> <td> System                                    <td> \ref HAL_SCU "Power Management Unit (PMU/SCU)"                      <td> <center>V</center>        <td> 03.00.00
<tr> <td> Port                                      <td> \ref HAL_PCU "Port Control Unit (PCU)"                              <td> <center>V</center>        <td> 02.00.00
<tr> <td> Memory                                    <td> \ref HAL_FMC "Flash Memory Controller (FMC)"                        <td> <center>V</center>        <td> 01.00.00
<tr> <td> Analog                                    <td> \ref HAL_ADC "Analog to Digital Converter (ADC)"                    <td> <center>F</center>        <td> 01.00.01
<tr> <td rowspan="3"> Connectivity & Communication  <td> \ref HAL_I2C "Inter-Integrated Circuit (I2C)"                       <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_SPI "Serial Peripheral Interface (SPI)"                    <td> <center>V</center>        <td> 01.00.02
<tr>                                                <td> \ref HAL_UART "Universal Asynchronous Receiver Transmitter (UART)"  <td> <center>V</center>        <td> 01.00.04
<tr> <td rowspan="4"> Timer                         <td> \ref HAL_TIMER1 "16-bit General Purpose Timer (TIMER1)"             <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_PWM "Pulse Width Modulation (PWM)"                         <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_WDT "Watch Dog Timer (WDT)"                                <td> <center>V</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_FRT "Free Run Timer (FRT)"                                 <td> <center>V</center>        <td> 01.00.01
</table>

### A33M11x

#### - Brief Description

 * CPU : ARM Cortex-M3 Core
 * Interupt : Up to 86 peripheral interrupts
 * Code Flash 
  - A33M116 : 256KB Code Flash Memory
  - A33M114 : 128KB Code Flash Memory
 * Data Flash : 32KB Data Flash Memory
 * SRAM : 16KB
 * Operation Frequency : Up To 96MHz
 * Clock
  - High Speed Internal oscillator (HSI) : 32MHz
  - Low Speed Internal oscillator (LSI) : 500KHz
  - High Speed External oscillator (HSE) : 4MHz ~ 16MHz
  - Low Speed External oscillator (LSE) : 32.768KHz
  - Phase-Locked Loop (PLL) : Up to 96MHz

#### - Support Peripheral List and Version

<table>
<tr>
    <th> <center>Group</center>                     <th> <center>Peripheral</center>                                         <th> <center>Type</center>     <th> <center>Version</center> 
<tr> <td> System                                    <td> \ref HAL_SCU "System Control Unit (SCU)"                            <td> <center>V</center>        <td> 02.00.01
<tr> <td> Port                                      <td> \ref HAL_PCU "Port Control Unit (PCU)"                              <td> <center>V</center>        <td> 02.00.01
<tr> <td rowspan="2"> Memory                        <td> \ref HAL_FMC "Flash Memory Controller (FMC)"                        <td> <center>V</center>        <td> 03.00.00
<tr>                                                <td> \ref HPL_DMA "Direct Memory Access controller (DMA)"                <td> <center>V</center>        <td> 01.00.01
<tr> <td rowspan="2"> Analog                        <td> \ref HAL_ADC "Analog to Digital Converter (ADC)"                    <td> <center>V</center>        <td> 01.01.00
<tr>                                                <td> \ref HAL_AFE "Analog Front End (AFE)"                               <td> <center>V</center>        <td> 01.00.00
<tr> <td> Security & Integrity                      <td> \ref HAL_CRC "Cyclic Redundancy Check (CRC)"                        <td> <center>V</center>        <td> 02.00.00
<tr> <td rowspan="4"> Connectivity & Communication  <td> \ref HAL_I2C "Inter-Integrated Circuit (I2C)"                       <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_SPI "Serial Peripheral Interface (SPI)"                    <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_UART "Universal Asynchronous Receiver Transmitter (UART)"  <td> <center>V</center>        <td> 01.00.03
<tr>                                                <td> \ref HAL_QEI "Quadrature Encoder Interface (QEI)"                   <td> <center>V</center>        <td> 01.00.00
<tr> <td rowspan="4"> Timer                         <td> \ref HAL_TIMER1 "16-bit General Purpose Timer (TIMER1)"             <td> <center>V</center>        <td> 01.02.00
<tr>                                                <td> \ref HAL_MPWM "Motor Pulse Width Modulation (MPWM)"                 <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_WDT "Watch Dog Timer (WDT)"                                <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_FRT "Free Run Timer (FRT)"                                 <td> <center>V</center>        <td> 01.00.00
</table>

### A33G53x

#### - Brief Description

 * CPU : ARM Cortex-M3 Core
 * Interupt : Up to 64 peripheral interrupts
 * Code Flash 
  - A33G538 : 512KB Code Flash Memory (Dual Bank)
  - A33G539 : 768KB Code Flash Memory (Dual Bank)
 * Data Flash : 32KB Data Flash Memory
 * SRAM : 24KB
 * Operation Frequency : Up To 75MHz
 * Clock
  - High Speed Internal oscillator (HSI:IOSC16) : 16MHz
  - Low Speed Internal oscillator (LSI:RINGOSC) : 1MHz
  - High Speed External oscillator (HSE:MXOSC) : 4MHz ~ 10MHz
  - Low Speed External oscillator (LSE:SXOSC) : 32.768KHz
  - Phase-Locked Loop (PLL) : Up to 75MHz

#### - Support Peripheral List and Version

<table>
<tr>
    <th> <center>Group</center>                     <th> <center>Peripheral</center>                                         <th> <center>Type</center>     <th> <center>Version</center> 
<tr> <td> System                                    <td> \ref HAL_SCU "Power Management Unit (PMU/SCU)"                      <td> <center>V</center>        <td> 03.00.00
<tr> <td> Port                                      <td> \ref HAL_PCU "Port Control Unit (PCU)"                              <td> <center>V</center>        <td> 03.00.00
<tr> <td> Memory                                    <td> \ref HAL_FMC "Flash Memory Controller (FMC)"                        <td> <center>V</center>        <td> 01.01.00
<tr> <td> Analog                                    <td> \ref HAL_ADC "Analog to Digital Converter (ADC)"                    <td> <center>F</center>        <td> 01.00.01
<tr> <td rowspan="3"> Connectivity & Communication  <td> \ref HAL_I2C "Inter-Integrated Circuit (I2C)"                       <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_SPI "Serial Peripheral Interface (SPI)"                    <td> <center>V</center>        <td> 01.00.02
<tr>                                                <td> \ref HAL_UART "Universal Asynchronous Receiver Transmitter (UART)"  <td> <center>V</center>        <td> 01.00.04
<tr> <td rowspan="4"> Timer                         <td> \ref HAL_TIMER1 "16-bit General Purpose Timer (TIMER1)"             <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_PWM "Pulse Width Modulation (PWM)"                         <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_WDT "Watch Dog Timer (WDT)"                                <td> <center>V</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_FRT "Free Run Timer (FRT)"                                 <td> <center>V</center>        <td> 01.00.01
</table>

A34xxxx Sub Family
-----

### A34M41x

#### - Brief Description

 * CPU : ARM Cortex-M4F Core
 * Interupt : Up to 86 peripheral interrupts
 * Code Flash 
  - A34M418 : 512KB Code Flash Memory
  - A34M416 : 256KB Code Flash Memory
  - A34M414 : 128KB Code Flash Memory
 * Data Flash : 32KB Data Flash Memory
 * SRAM : 64KB / 32KB
 * Operation Frequency : Up To 120MHz
 * Clock
  - High Speed Internal oscillator (HSI) : 32MHz
  - Low Speed Internal oscillator (LSI) : 500KHz
  - High Speed External oscillator (HSE) : 4MHz ~ 16MHz
  - Low Speed External oscillator (LSE) : 32.768KHz
  - Phase-Locked Loop (PLL) : Up to 120MHz

#### - Support Peripheral List and Version

<table>
<tr>
    <th> <center>Group</center>                     <th> <center>Peripheral</center>                                         <th> <center>Type</center>     <th> <center>Version</center> 
<tr> <td> System                                    <td> \ref HAL_SCU "System Control Unit (SCU)"                            <td> <center>V</center>        <td> 02.00.00
<tr> <td> Port                                      <td> \ref HAL_PCU "Port Control Unit (PCU)"                              <td> <center>V</center>        <td> 02.00.00
<tr> <td rowspan="2"> Memory                        <td> \ref HAL_FMC "Flash Memory Controller (FMC)"                        <td> <center>V</center>        <td> 03.00.00
<tr>                                                <td> \ref HPL_DMA "Direct Memory Access controller (DMA)"                <td> <center>V</center>        <td> 01.00.00
<tr> <td rowspan="3"> Analog                        <td> \ref HAL_ADC "Analog to Digital Converter (ADC)"                    <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_CMP "Comparator (CMP)"                                     <td> <center>V</center>        <td> 02.00.00
<tr>                                                <td> \ref HAL_PGA "Programmable Gain Amplifier (PGA)"                    <td> <center>V</center>        <td> 01.00.00
<tr> <td rowspan="3"> Security & Integrity          <td> \ref HAL_CRC "Cyclic Redundancy Check (CRC)"                        <td> <center>V</center>        <td> 02.00.00
<tr>                                                <td> \ref HAL_AES "Advanced Encryption Standard (AES)"                   <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_RNG "Random Number Generator (RNG)"                        <td> <center>V</center>        <td> 01.00.00
<tr> <td rowspan="4"> Connectivity & Communication  <td> \ref HAL_I2C "Inter-Integrated Circuit (I2C)"                       <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_SPI "Serial Peripheral Interface (SPI)"                    <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_UART "Universal Asynchronous Receiver Transmitter (UART)"  <td> <center>V</center>        <td> 01.00.03
<tr>                                                <td> \ref HAL_QEI "Quadrature Encoder Interface (QEI)"                   <td> <center>V</center>        <td> 01.00.00
<tr> <td rowspan="4"> Timer                         <td> \ref HAL_TIMER1 "16-bit General Purpose Timer (TIMER1)"             <td> <center>V</center>        <td> 01.02.00
<tr>                                                <td> \ref HAL_MPWM "Motor Pulse Width Modulation (MPWM)"                 <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_WDT "Watch Dog Timer (WDT)"                                <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_FRT "Free Run Timer (FRT)"                                 <td> <center>V</center>        <td> 01.00.00
</table>

### A34M42x

#### - Brief Description

 * CPU : ARM Cortex-M4F Core
 * Interupt : Up to 86 peripheral interrupts
 * Code Flash 
  - A34M420 : 1024KB Code Flash Memory (Dual Bank)
 * Data Flash : 32KB Data Flash Memory
 * SRAM : 64KB
 * Operation Frequency : Up To 140MHz
 * Clock
  - High Speed Internal oscillator (HSI) : 32MHz
  - Low Speed Internal oscillator (LSI) : 500KHz
  - High Speed External oscillator (HSE) : 4MHz ~ 16MHz
  - Low Speed External oscillator (LSE) : 32.768KHz
  - Phase-Locked Loop (PLL) : Up to 140MHz

#### - Support Peripheral List and Version

<table>
<tr>
    <th> <center>Group</center>                     <th> <center>Peripheral</center>                                         <th> <center>Type</center>     <th> <center>Version</center> 
<tr> <td> System                                    <td> \ref HAL_SCU "System Control Unit (SCU)"                            <td> <center>V</center>        <td> 02.00.04
<tr> <td> Port                                      <td> \ref HAL_PCU "Port Control Unit (PCU)"                              <td> <center>V</center>        <td> 02.00.00
<tr> <td rowspan="2"> Memory                        <td> \ref HAL_FMC "Flash Memory Controller (FMC)"                        <td> <center>V</center>        <td> 03.00.01
<tr>                                                <td> \ref HPL_DMA "Direct Memory Access controller (DMA)"                <td> <center>V</center>        <td> 01.00.00
<tr> <td> Analog                                    <td> \ref HAL_ADC "Analog to Digital Converter (ADC)"                    <td> <center>V</center>        <td> 01.00.03
<tr> <td rowspan="2"> Security & Integrity          <td> \ref HAL_CRC "Cyclic Redundancy Check (CRC)"                        <td> <center>V</center>        <td> 02.00.00
<tr>                                                <td> \ref HAL_RNG "Random Number Generator (RNG)"                        <td> <center>V</center>        <td> 01.00.00
<tr> <td rowspan="3"> Connectivity & Communication  <td> \ref HAL_I2C "Inter-Integrated Circuit (I2C)"                       <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_SPI "Serial Peripheral Interface (SPI)"                    <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_UART "Universal Asynchronous Receiver Transmitter (UART)"  <td> <center>V</center>        <td> 01.00.03
<tr> <td rowspan="3"> Timer                         <td> \ref HAL_TIMER1 "16-bit General Purpose Timer (TIMER1)"             <td> <center>V</center>        <td> 01.02.00
<tr>                                                <td> \ref HAL_WDT "Watch Dog Timer (WDT)"                                <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_FRT "Free Run Timer (FRT)"                                 <td> <center>V</center>        <td> 01.00.00
</table>

### A34L71x

#### - Brief Description

 * CPU : ARM Cortex-M4F Core
 * Interupt : Up to 86 peripheral interrupts
 * Code Flash 
  - A34L716 : 256KB Code Flash Memory
 * Data Flash : 32KB Data Flash Memory
 * SRAM : 24KB
 * Operation Frequency : Up To 48MHz
 * Clock
  - High Speed Internal oscillator (HSI) : 32MHz
  - Low Speed Internal oscillator (LSI) : 40KHz
  - High Speed External oscillator (HSE) : 4MHz ~ 16MHz
  - Low Speed External oscillator (LSE) : 32.768KHz
  - Phase-Locked Loop (PLL) : Up to 48MHz

#### - Support Peripheral List and Version

<table>
<tr>
    <th> <center>Group</center>                     <th> <center>Peripheral</center>                                                       <th> <center>Type</center>     <th> <center>Version</center> 
<tr> <td> System                                    <td> \ref HAL_SCU "System Control Unit (SCU)"                                          <td> <center>V</center>        <td> 02.00.05
<tr> <td> Port                                      <td> \ref HAL_PCU "Port Control Unit (PCU)"                                            <td> <center>V</center>        <td> 02.00.04
<tr> <td rowspan="2"> Memory                        <td> \ref HAL_FMC "Flash Memory Controller (FMC)"                                      <td> <center>V</center>        <td> 03.00.02
<tr>                                                <td> \ref HPL_DMA "Direct Memory Access controller (DMA)"                              <td> <center>V</center>        <td> 01.00.00
<tr> <td rowspan="2"> Analog                        <td> \ref HAL_ADC "Analog to Digital Converter (ADC)"                                  <td> <center>V</center>        <td> 01.00.04
<tr>                                                <td> \ref HAL_DAC "Digital to Analog Converter (DAC)"                                  <td> <center>V</center>        <td> 03.00.00
<tr> <td> Security & Integrity                      <td> \ref HAL_CRC "Cyclic Redundancy Check (CRC)"                                      <td> <center>V</center>        <td> 02.00.03
<tr> <td rowspan="5"> Connectivity & Communication  <td> \ref HAL_I2C "Inter-Integrated Circuit (I2C)"                                     <td> <center>V</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_SPI "Serial Peripheral Interface (SPI)"                                  <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_UART "Universal Asynchronous Receiver Transmitter (UART)"                <td> <center>V</center>        <td> 01.00.07
<tr>                                                <td> \ref HAL_USART "Universal Synchronous/Asynchronous Receiver Transmitter (USART)"  <td> <center>F</center>        <td> 01.02.02
<tr>                                                <td> \ref HAL_LPUART "Low Power Universal Asynchronous Receiver Transmitter (LPUART)"  <td> <center>F</center>        <td> 01.00.01
<tr> <td rowspan="4"> Timer                         <td> \ref HAL_TIMER1 "16-bit General Purpose Timer (TIMER1)"                           <td> <center>V</center>        <td> 01.02.00
<tr>                                                <td> \ref HAL_WDT "Watch Dog Timer (WDT)"                                              <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_FRT "Free Run Timer (FRT)"                                               <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_RTC "Real-Time Clock (RTC)"                                              <td> <center>V</center>        <td> 01.00.01
<tr> <td> Display                                   <td> \ref HAL_LCD "Liquid Crystal Display controller (LCD)"                            <td> <center>F</center>        <td> 01.00.04
<tr> <td> Miscellaneous                             <td> \ref HAL_VREFBUF "Voltage Reference Buffer (VREFBUF)"                             <td> <center>V</center>        <td> 01.00.00
</table>

### A34G43x

#### - Brief Description

 * CPU : ARM Cortex-M4F Core
 * Interupt : Up to 86 peripheral interrupts
 * Code Flash 
  - A34G43A : 2048KB Code Flash Memory
  - A34G430 : 1024KB Code Flash Memory
 * Data Flash : 32KB Data Flash Memory
 * SRAM : 256KB
 * Operation Frequency : Up To 200MHz
 * Clock
  - High Speed Internal oscillator (HSI) : 16MHz
  - Low Speed Internal oscillator (LSI) : 500KHz
  - High Speed External oscillator (HSE) : 4MHz ~ 16MHz
  - Low Speed External oscillator (LSE) : 32.768KHz
  - Phase-Locked Loop (PLL) : Up to 200MHz

#### - Support Peripheral List and Version

<table>
<tr>
    <th> <center>Group</center>                     <th> <center>Peripheral</center>                                                       <th> <center>Type</center>     <th> <center>Version</center> 
<tr> <td> System                                    <td> \ref HAL_SCU "System Control Unit (SCU)"                                          <td> <center>V</center>        <td> 02.00.03
<tr> <td> Port                                      <td> \ref HAL_PCU "Port Control Unit (PCU)"                                            <td> <center>V</center>        <td> 02.00.05
<tr> <td rowspan="2"> Memory                        <td> \ref HAL_FMC "Flash Memory Controller (FMC)"                                      <td> <center>V</center>        <td> 03.00.03
<tr>                                                <td> \ref HPL_DMA "Direct Memory Access controller (DMA)"                              <td> <center>V</center>        <td> 01.00.00
<tr> <td rowspan="3"> Analog                        <td> \ref HAL_ADC "Analog to Digital Converter (ADC)"                                  <td> <center>V</center>        <td> 01.00.05
<tr>                                                <td> \ref HAL_DAC "Digital to Analog Converter (DAC)"                                  <td> <center>V</center>        <td> 03.00.01
<tr>                                                <td> \ref HAL_CMP "Comparator (CMP)"                                                   <td> <center>V</center>        <td> 02.00.02
<tr> <td rowspan="3"> Security & Integrity          <td> \ref HAL_CRC "Cyclic Redundancy Check (CRC)"                                      <td> <center>V</center>        <td> 02.00.03
<tr>                                                <td> \ref HAL_AES "Advanced Encryption Standard (AES)"                                 <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_RNG "Random Number Generator (RNG)"                                      <td> <center>V</center>        <td> 01.00.00
<tr> <td rowspan="4"> Connectivity & Communication  <td> \ref HAL_I2C "Inter-Integrated Circuit (I2C)"                                     <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_SPI "Serial Peripheral Interface (SPI)"                                  <td> <center>V</center>        <td> 01.01.00
<tr>                                                <td> \ref HAL_UART "Universal Asynchronous Receiver Transmitter (UART)"                <td> <center>V</center>        <td> 01.00.06
<tr>                                                <td> \ref HAL_QSPI "Quad Serial Peripheral Interface (QSPI)"                           <td> <center>V</center>        <td> 01.00.00
<tr> <td rowspan="4"> Timer                         <td> \ref HAL_TIMER1 "16-bit General Purpose Timer (TIMER1)"                           <td> <center>V</center>        <td> 01.02.00
<tr>                                                <td> \ref HAL_PWM "Pulse Width Modulation (PWM)"                                       <td> <center>V</center>        <td> 01.00.01
<tr>                                                <td> \ref HAL_WDT "Watch Dog Timer (WDT)"                                              <td> <center>V</center>        <td> 01.00.00
<tr>                                                <td> \ref HAL_FRT "Free Run Timer (FRT)"                                               <td> <center>V</center>        <td> 01.00.00
</table>
