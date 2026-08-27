# HMR 3-axis datasets

Same recording, three sample rates. Header on every file:

```text
acc_x,acc_y,acc_z,class
```

Values are milli-g. Classes: `CIRCLE`, `HORIZONTAL`, `STANDBY`, `TRIANGLE`, `VERTICAL`. 8 Hz is keep-every-16th of 128 Hz; 4 Hz is keep-every-32nd. Wall-clock length is ~58.6 minutes for all three.

| File | Rate | Rows |
|---|---|---:|
| `Dataset_HMR_3Axis_TiremoCortex_128Hz.csv` | 128 Hz | 450 048 |
| `Dataset_HMR_3Axis_TiremoCortex_8Hz.csv` | 8 Hz | 28 128 |
| `Dataset_HMR_3Axis_TiremoCortex_4Hz.csv` | 4 Hz | 14 064 |

Why they exist, and which to import for Edge window size 8, is in the [design tutorial README](../README.md).
