# Implementation

1. Include the public header:

```c
#include "tiremo_api.h"
```

2. Fill `float features[TIREMO_INPUT_SIZE]`. Model input shape: `[24]` (flat vector). `tiremo_ai_run` takes a buffer of length `TIREMO_INPUT_SIZE` = `24`. Collection is already done.

3. Call `tiremo_ai_run`:

```c
float features[TIREMO_INPUT_SIZE];  /* [24] */
features[i] = sample[i];  /* i = 0 .. TIREMO_INPUT_SIZE - 1 */
TiremoInferenceResult result;
int rc = tiremo_ai_run(features, &result);
if (rc != TIREMO_OK) {
    /* tiremo_ai_strerror(rc) */
}
```

4. Time the call with the device clock. Pass the elapsed milliseconds as `inference_time_ms`.

5. Call `tiremo_ai_format_mqtt_json`. Window timestamps are Unix epoch milliseconds. Solution and deployment IDs are already in `tiremo_config.h`.

```c
char mqtt_json[1024];
rc = tiremo_ai_format_mqtt_json(
    &result, window_start_unix_ms, window_finish_unix_ms,
    inference_time_ms, mqtt_json, sizeof mqtt_json);
```

6. Publish the JSON yourself on `pub/<product_id>/<device_id>/inference`.

7. Next to `CC`, set the C++ compiler with the same prefix as `gcc`:

```makefile
CXX := $(CROSS)g++
```

8. After `OBJS` is defined, include the build fragment:

```makefile
include $(TIREMO_DIR)/Tiremo_AI/tiremo_makefile.mk
```

9. Link the ELF with `$(CXX)` instead of `$(CC)`.

10. Application `.c` files stay C; they only `#include "tiremo_api.h"`.

## Target

This package uses `float` arithmetic. **Enable the FPU** on the MCU
and compile/link with a hardware-float ABI. Do not mix `-mfloat-abi=soft` with
this package. Put the same CPU/FPU flags on the compiler and the linker.

Example (Cortex-M4; pick `-mfpu` for your core — M33/M7 differ):

```makefile
CPU_FLAGS := -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS   += $(CPU_FLAGS)
CXXFLAGS += $(CPU_FLAGS)
LDFLAGS  += $(CPU_FLAGS)
```

If the BSP does not already do so, enable the FPU in startup (Cortex-M: coprocessor
access via `SCB->CPACR`) before calling `tiremo_ai_run`.

