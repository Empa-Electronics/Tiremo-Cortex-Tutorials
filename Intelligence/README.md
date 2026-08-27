<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../images/banner.png" width="100%">
      <h1>Intelligence</h1>
      <sub>Tiremo Cortex Tutorials: ESP32 + Tiremo Cloud firmware, from first blink to a fleet-ready device</sub>
    </td>
  </tr>
</table>

# Intelligence

Collect telemetry from a live Tiremo Cortex (or any Thing on the product), turn it into a trainable Data Source, search models in an Experiment, and deploy a Solution, including an on-device Edge package, from the **Tiremo UI** at [`iot.tiremo.ai`](https://iot.tiremo.ai). Feature Tutorials teach those Intelligence objects with no new `Sw/` firmware. The Design Tutorials then apply the same chain to one complete Human Motion Recognition (HMR) case, from 128 Hz capture through a generated `Tiremo_AI/` Edge library, and ship three Cortex binaries (telemetry, windowing, on-device inference).

The platform objects sit in a chain. Data Explorer is *not* under the Intelligence menu; it is how you inspect and export what a device already published, but it is the usual first step toward a Data Source:

```mermaid
flowchart LR
    A[Device telemetry] --> B[Data Explorer]
    B --> C[CSV export / labels]
    C --> D[Data Source]
    D --> E[Data Studio]
    E --> D
    D --> F[Experiment]
    F --> G[Training Record]
    G --> H[Solution]
    H --> I[Edge deployment]
    I --> J["Tiremo_AI/ on the MCU"]
```

- The [**Feature Tutorials**](./Feature_Tutorials/) cover one Intelligence capability each: Data Collection (Data Explorer or any preferred method), Data Sources, Experiments, Solutions. Worked examples (for instance accelerometer keys for motion classification) appear in the details, not as the only purpose of the object.
- The [**Design Tutorials**](./Design_Tutorials/) combine those objects into one shipped HMR case and flash three LIS2DE12 firmwares.

UI screenshots live in [`figures/`](./figures/). Design-tutorial captures (gestures, UART inference) live next to [`03-Edge_Deployment_HMR/figures/`](./Design_Tutorials/03-Edge_Deployment_HMR/figures/).

<p align="center"><img alt="" src="../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials, Empa Electronics</sub></p>
