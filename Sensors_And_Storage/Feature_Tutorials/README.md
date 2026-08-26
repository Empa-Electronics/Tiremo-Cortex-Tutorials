<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <h1>Sensors & Local Storage Tutorials</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Feature Tutorials

Feature tutorials cover one concept each — a single driver, a single AT command sequence, a single cloud primitive. Work through them in order; each one names its prerequisite.

 <table style="width:100%">
 <tr>
 <td width="35%" align="center"><b>Tutorial</b>
 <td width="65%" align="center"><b>Description</b>
 </tr>
 <tr>
 <td align="center"><a href="./01-I2C_Bus_Bringup/">I2C Bus Bring-up</a></td>
 <td>Bring up I2C2 and scan/probe it before trusting any sensor driver on top.</td>
 </tr>
 <tr>
 <td align="center"><a href="./02-SHT40_Temperature_Humidity/">SHT40 Temperature / Humidity</a></td>
 <td>Read temperature and humidity from the SHT40 over I2C2.</td>
 </tr>
 <tr>
 <td align="center"><a href="./03-LIS2DE12_Accelerometer/">LIS2DE12 Accelerometer</a></td>
 <td>Read X/Y/Z acceleration in mg and make scale/ODR runtime-configurable.</td>
 </tr>
 <tr>
 <td align="center"><a href="./04-Battery_Voltage_ADC/">Battery Voltage via ADC</a></td>
 <td>Measure AVDD supply voltage using the internal VCORE bandgap reference.</td>
 </tr>
 <tr>
 <td align="center"><a href="./05-Microphone_RMS_To_dB/">Microphone: RMS Capture to dB</a></td>
 <td>Capture the MP23ABS1 mic via ADC burst + DMA and convert an RMS envelope to a relative dB level.</td>
 </tr>
 <tr>
 <td align="center"><a href="./06-Flash_Backed_Config/">Flash-Backed Config</a></td>
 <td>Persist a small struct (magic + CRC32) to data flash so settings survive reboot.</td>
 </tr>
 <tr>
 <td align="center"><a href="./07-Software_Wall_Clock/">Software Wall Clock</a></td>
 <td>Keep epoch time as epoch-at-sync + SysTick offset — no RTC peripheral on this chip.</td>
 </tr>
 </table>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
