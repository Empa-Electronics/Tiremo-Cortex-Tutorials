<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../docs-tr/Intelligence/Design_Tutorials/02-Accelerometer_Windowing/README.md">Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../images/banner.png" width="100%">
      <h1>Accelerometer Windowing</h1>
      <sub>Tiremo Cortex Tutorials: ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>

# Accelerometer Windowing

***Board: Tiremo Cortex (ESP32) · Section: Intelligence / Design Tutorials · Difficulty: Medium***

## What this binary does

No network. After the design-tutorial boot banner, the firmware configures the LIS2DE12 for ±2 g / 10 Hz, samples every **125 ms**, and fills a tumbling window of **8 samples × 3 axes** (`float[24]`, milli-g, interleaved xyz). When the window is full it dumps the 24 values over UART and starts the next window. Same layout the Edge model in [03](../03-HMR_Edge_Deployment/) expects; this binary does not call `tiremo_ai_run`.

## Build and flash

From `Sw/` (no WiFi secrets or claim PEMs):

```text
cd Intelligence/Design_Tutorials/02-Accelerometer_Windowing/Sw
tdf.py build
tdf.py flash
tdf.py monitor
```

<p align="center"><img alt="" src="../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials, Empa Electronics</sub></p>
