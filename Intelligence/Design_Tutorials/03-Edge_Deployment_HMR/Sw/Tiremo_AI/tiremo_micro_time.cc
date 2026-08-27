// GENERATED FILE — do not edit.
// config hash: da4f5bc5c01c82ef  model: ff8c7ffa-ca80-4f3a-81cb-fc39f90f1c74  generated: 2026-08-27T21:32:01Z
#include "tensorflow/lite/micro/micro_time.h"

namespace tflite {

// Portable no-op timing — MicroProfiler is not used by tiremo_api.cc.
// Replaces any TFLM micro_time.cc so the package stays ISA-neutral.
uint32_t ticks_per_second() { return 0; }
uint32_t GetCurrentTimeTicks() { return 0; }

}  // namespace tflite
