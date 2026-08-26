Release Note {#Release_Note}
=====

1.0.12 (April. 17, 2026)
-----

### New Features

 * Supported Chipsets
   - Cortex-M0+  : A31xxxx Series
     * A31C14x
     * A31C12x

### Enhancements
 N/A

### Bug Fixes
 N/A

### Tool
 N/A

### Documentation
 N/A

### Miscellaneous
 N/A

1.0.11 (Feb. 2, 2026)
-----

### New Features

 * Supported Chipsets
   - Cortex-M4  : A34xxxx Series
     * A34G43x

### Enhancements
 * New Modules.
   - QSPI (Quad Serial Peripheral Interface) 

### Bug Fixes
 N/A

### Tool
 N/A

### Documentation
 N/A

### Miscellaneous
 N/A

1.0.10 (Dec. 24, 2025)
-----

### New Features

 * Supported Chipsets
   - Cortex-M4  : A34xxxx Series
     * A34L71x

### Enhancements
 * New Modules.
   - VREFBUF (Voltage Reference Buffer) 

### Bug Fixes
 N/A

### Tool
 N/A

### Documentation
 N/A

### Miscellaneous
 N/A

1.0.9 (Nov. 24, 2025)
-----

### New Features

 * Supported Chipsets
   - Cortex-M0+ : A31xxxx Series
     * A31G34x
   - Cortex-M4  : A34xxxx Series
     * A34M42x

### Enhancements
 N/A

### Bug Fixes
  * I2C
    - Modified Master rx stop sequence.
    - Modified to send START condition when address NACK occurs in master mode.
  * WDT
    - Modified to control counter bitfield on WDT starts/stops.

### Tool
 N/A

### Documentation
 N/A

### Miscellaneous
 N/A

1.0.8 (Sep. 23, 2025)
-----

### New Features

 * Supported Chipsets
   - Cortex-M0+ : A31xxxx Series
     * A31G33x
     * A31S13x

### Enhancements
 N/A

### Bug Fixes
  * SCU Clock
    - Fixed calculated delay count when using Low clock source as Main clock.
  * USART
    - Added to control 9bit data tx/rx for tx/rx buffer size 9bit.

### Tool
 N/A

### Documentation
 N/A

### Miscellaneous
 N/A

1.0.7 (Jun. 30, 2025)
-----

### New Features

 * Supported Chipsets
   - Cortex-M0+ : A31xxxx Series
     * A31L12x (Low Power)
     * A31L22x (Low Power)

### Enhancements

 * New Modules.

 * Changes
   - Integrating Touch Library (v1.10) and Touch Application for use with Keil, IAR, and Eclipse environments utilized by A31T21x.

### Bug Fixes
  * Hardware Private Layer (NMI)
    - Modified the size of internal contexts. (occurred the stack crash issue)
  * SCU Clock
    - Added stable delay almost 2 and 2.5 sec between enabling LSE clock and setting Main clock source.
    - Added to check PLL ready bit when setting PLL for A31T21x.
  * LCD
    - Modified to apply the divider value when the LCD clock source is set to the main clock.
  * CFMC (Code Flash)
    - Fixed to calculate the writing page length.

### Tool
 N/A

### Documentation
 N/A

### Miscellaneous
 N/A

1.0.6 (Feb. 28, 2025)
-----

### New Features

 * Supported Chipsets
   - Cortex-M0+ : A31xxxx Series
     * A31L21x (Low Power)

### Enhancements

 * New Modules.
   - LPUART (Low Power UART) 
   - TRANG (True Random Number Generator)
   - Timer4E (16bit 2-phase complmementary PWM)
   - Timer5 (16bit multi input capture source)
   - Timer6 (16bit 100Hz source clock)

 * Changes
   - Analog group (ADC)
     * Refactoring the internal logic of the ADC HAL module.
   - Hardware Private Layer (DMA)
     * Adding the internal interrupt logic of the DMA HPL module. (for only A31L21x)

### Bug Fixes
 N/A

### Tool
 N/A

### Documentation
 N/A

### Miscellaneous
 N/A

1.0.5 (July. 01, 2024)
-----

### New Features

 * Supported Chipsets
   - Cortex-M3 : A33xxxx Series
     * A33G53x

### Enhancements

 * Add AutoTest scripts based on Tera-term.

### Bug Fixes
 N/A

### Tool
 N/A

### Documentation
 * Updated the document content to align with version 1.0.3.
   - STD10-HB_AUDK32 Guide_Eng.pdf
   - STD10-HB_AUDK32 CLI Example_Eng.pdf

### Miscellaneous
 N/A

1.0.4 (March. 29, 2024)
-----

### New Features

 * Supported Chipsets
   - Cortex-M0+ : A31xxxx Series
     * A31G11x
     * A31G12x
     * A31T21x

 * Integrating Touch Library (v1.09) and Touch Application for use with Keil, IAR, and Eclipse environments utilized by A31T21x.
  - Touch Library Guide Doc : STKS_Shield Board Quick Guide_A31T216RLN_Eng.pdf
    (URL : https://www.abov.co.kr/kr/products/product_documentation.php?category=home_appliance_touch_solution_series&device=A31T214RLN&type=past)

### Enhancements

 * Re-factoring CLI-based example source code.

### Bug Fixes
 N/A

### Tool
 N/A

### Documentation
 * Updated the document content to align with version 1.0.3.
   - STD10-HB_AUDK32 Guide_Eng.pdf
   - STD10-HB_AUDK32 CLI Example_Eng.pdf

### Miscellaneous
 N/A

1.0.3 (Dec. 29, 2023)
-----

### New Features

 * Adding Bare/STK Eclipse workspace and project in both of Windows and Linux environments.
 * Adding flash downloader and debugger based on OpenOCD 0.12.0.

### Enhancements

### Bug Fixes
 N/A

### Tool
 N/A

### Documentation
 * Updated the document content to align with version 1.0.3.
   - STD10-HB_AUDK32 Guide_Eng.pdf
   - STD10-HB_AUDK32 CLI Example_Eng.pdf

### Miscellaneous
 N/A

1.0.2 (May. 12, 2023)
-----

### New Features

 * Supported Chipsets
   - Cortex-M3 : A33xxxx Series
     * A33G52x
     * A33M11x

### Enhancements

 * New Modules.
   - Pulse Width Modulation (PWM)
   - Analog Front End (AFE)

 * Changes
   - Memory group (CFMC, FMC)
     * Refactoring the internal logic of the CFMC and FMC HAL module.
   - Timer group (WDT)
     * Refactoring the internal logic of the WDT module.
   - Analog group (ADC)
     * Realigned the internal module version.
   - System (SCU CLK)
     * Added a pre/post main clock divider struct member to the main clock struct.
   - Port (PCU/GPIO)
     * Added the 'un8IntNum' input argument to the 'HAL_PCU_SetIntrPort' API.

### Bug Fixes
 N/A

### Tool
 N/A

### Documentation
 * Modified the document content to match version 1.0.2.
   - STD10-HB_AUDK32 Guide_Eng.pdf
   - STD10-HB_AUDK32 CLI Example_Eng.pdf

### Miscellaneous
 * Added brief descriptions of A33G52x and A33M11x chipsets in the markdown file.
   - Chipset_Description.md

1.0.1 (Dec. 30, 2022)
-----

### New Features

 * Supported Chipsets
   - Cortex-M4 : A34xxxx Series
     * A34M41x

### Enhancements

 * New Modules.
   - Motor Pulse Width Modulation (MPWM)
   - Quadrature Encoder Interface (QEI)
   - Random Number Generator (RNG)
   - Advances Encryption Standard (AES)
   - Programmable Gain Amplifier (PGA)
   - Free Run Timer (FRT)

 * Changes
   - Connectivity module (I2C, SPI, UART, USART)
     * Supported only polling mode among enforced operation option in transmit and receive standard api.

### Bug Fixes
 N/A

### Tool
 N/A

### Documentation
 * Changed the file name of the document 
   - STD10-HB_AUDK32 Getting Started_Eng.pdf to STD10-HB_AUDK32 Guide_Eng.pdf
   - STD10-HB_AUDK32 Firmware Example_Eng.pdf to STD10-HB_AUDK32 CLI Example_Eng.pdf
 * Modified description more clearly.

### Miscellaneous
 * Added chipset brief description markdown file for doxygen.
   - Chipset_Description.md

1.0.0 (Sep. 30, 2022)
-----

### New Features

 * Initial version of AUDK32.
 * Supported Chipsets
   - Cortex-M0/M0+ : A31xxxx Series
     * A31G21x
     * A31G31x
     * A31G32x
     * A31G22x

### Enhancements

 * Implmeneted Hardware Abstraction Layer(HAL) and Hardware Private Layer(HPL).
 * Applied USART/UART based Command-Line Input(CLI) environment for the operation of each module example.
 * Added Tera Term Language(TTL)-based module operation script files for each module.

### Bug Fixes
 N/A

### Tool
 * Added Flash Programming Algorithm (FPA) files for downloading firmware image through CMSIS-DAP Debugger.
   - .flm files for Keil IDE tool
   - .board/.flash/.out files for IAR IDE tool
 * Added System View Description(SVD) files based on XML and SFR files.

### Documentation
 * All the latest documentation can be downloaded from the [ABOV website](https://www.abovsemi.com).
   - STD10-HB_AUDK32 Getting Started_Eng.pdf
   - STD10-HB_AUDK32 Firmware Example_Eng.pdf
   - AUDK32.chm (AUDK32 Microsoft Compiled HTML Help file).

### Miscellaneous
 N/A
