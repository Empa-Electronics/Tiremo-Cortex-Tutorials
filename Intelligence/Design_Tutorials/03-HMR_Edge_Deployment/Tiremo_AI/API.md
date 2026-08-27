# Tiremo_AI API

## Input

| Macro | Value |
|---|---|
| `TIREMO_WINDOW_SIZE` | 8 |
| `TIREMO_NUM_FEATURES` | 3 |
| `TIREMO_INPUT_SIZE` | 24 |

Model input shape: `[24]` (flat vector). `tiremo_ai_run` takes a buffer of length `TIREMO_INPUT_SIZE` = `24`.

Element type: `float`. Fill as:

```c
float features[TIREMO_INPUT_SIZE];  /* [24] */
features[i] = sample[i];  /* i = 0 .. TIREMO_INPUT_SIZE - 1 */
```

## Output (`tiremo_ai_run`)

Fills `TiremoInferenceResult`. Status is `TIREMO_OK` or a negative `tiremo_status_t`.

| Field | Meaning |
|---|---|
| `predicted_class` | Argmax index into `TIREMO_CLASS_NAMES` |
| `predicted_class_label` | `TIREMO_CLASS_NAMES[predicted_class]` |
| `class_probabilities[TIREMO_N_CLASSES]` | Per-class probabilities in `[0, 1]` (softmax applied when the model output is logits) |

`TIREMO_N_CLASSES` / `TIREMO_NUM_CLASSES` = 5. Labels (`TIREMO_CLASS_NAMES[i]`):

| `i` | Name |
|---|---|
| `0` | `CIRCLE` |
| `1` | `HORIZONTAL` |
| `2` | `STANDBY` |
| `3` | `TRIANGLE` |
| `4` | `VERTICAL` |

## MQTT JSON (`tiremo_ai_format_mqtt_json`)

Writes the EDGE JSON body only (no network, no topic). Required arguments:

| Argument | Meaning |
|---|---|
| `inference_result` | Filled `TiremoInferenceResult` from `tiremo_ai_run` |
| `window_start_unix_ms` | Unix epoch ms (UTC) of the first sample in the window |
| `window_finish_unix_ms` | Unix epoch ms (UTC) of the last sample in the window |
| `inference_time_ms` | Elapsed ms of `tiremo_ai_run` (not Unix time) |
| `mqtt_json_out` / length | Caller buffer; output is NUL-terminated |

Identity (`TIREMO_SOLUTION_IDENTIFIER`, `TIREMO_DEPLOYMENT_IDENTIFIER`,
`TIREMO_DEPLOYMENT_ALIAS`) comes from `tiremo_config.h`.

Topic suffix: `TIREMO_MQTT_TOPIC_SUFFIX` (`"inference"`).
Publish on `pub/<product_id>/<device_id>/inference`.
