<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../images/banner.png" width="100%">
      <h1>Intelligence Tutorials</h1>
      <sub>Tiremo Cortex Tutorials: ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>

# Design Tutorials

Design tutorials take the Intelligence objects from the Feature Tutorials and combine them into working subsystems, closer to what actually ships. Flash them in order: live `acc_x`/`acc_y`/`acc_z` telemetry, a local 8-sample window, then the HMR Edge package on that same window.

 <table style="width:100%">
 <tr>
 <td width="35%" align="center"><b>Tutorial</b>
 <td width="65%" align="center"><b>Description</b>
 </tr>
 <tr>
 <td align="center"><a href="./01-Accelerometer_Telemetry/">Accelerometer Telemetry</a></td>
 <td>LIS2DE12 at 8 Hz software pace; one MQTT JSON <code>{"acc_x","acc_y","acc_z"}</code> per sample on <code>…/telemetry</code>.</td>
 </tr>
 <tr>
 <td align="center"><a href="./02-Accelerometer_Windowing/">Accelerometer Windowing</a></td>
 <td>Same sensor and pace; tumbling 8×3 milli-g buffer dumped over UART. No cloud, no <code>Tiremo_AI</code>.</td>
 </tr>
 <tr>
 <td align="center"><a href="./03-HMR_Edge_Deployment/">HMR Gesture Classifier on the Edge</a></td>
 <td>Take the 3-axis HMR capture from 128 Hz through 8 Hz and 4 Hz, train a five-class classifier in Intelligence, and run the generated <code>Tiremo_AI/</code> package on the Cortex (<code>Sw/</code> publishes inference MQTT).</td>
 </tr>
 </table>

<p align="center"><img alt="" src="../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials, Empa Electronics</sub></p>
