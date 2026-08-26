<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../images/banner.png" width="100%" height="16">
      <h1>Getting Started</h1>
      <sub>Tiremo Cortex Tutorials — ESP32 + AWS IoT firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>


# Getting Started

Start here. No experience with this codebase required — by the end of this section you'll have a board that blinks, logs over UART, reads its button, and drives all ten LEDs, using nothing but `Tiremo_Hal`.

* **[Hello Blink](./01-Hello_Blink/)**: Toggle one onboard LED — first successful build/flash round-trip.
* **[SysTick Timing](./02-SysTick_Timing/)**: Replace busy-wait delays with a millisecond tick so later loops can stay non-blocking.
* **[Debug UART Logging](./03-Debug_UART_Logging/)**: Bring up leveled UART logging so every later tutorial can narrate itself.
* **[Button Read & Debounce](./04-Button_Read_And_Debounce/)**: Poll the USER button and print press/release/hold-duration events.
* **[Ten-LED Pattern Driver](./05-Ten_LED_Pattern_Driver/)**: Drive all ten LEDs with solid/blink/pulse patterns as a standalone module.

<p align="center"><img alt="" src="../images/banner.png" width="100%" height="16"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
