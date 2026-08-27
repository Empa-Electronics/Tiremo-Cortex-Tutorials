// GENERATED FILE — do not edit.
// config hash: da4f5bc5c01c82ef  model: ff8c7ffa-ca80-4f3a-81cb-fc39f90f1c74  generated: 2026-08-27T21:32:01Z
#ifndef TIREMO_OPS_H_
#define TIREMO_OPS_H_

#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"

inline tflite::MicroMutableOpResolver<4> tiremo_get_op_resolver() {
    tflite::MicroMutableOpResolver<4> resolver;
    resolver.AddFullyConnected();
    resolver.AddMul();
    resolver.AddSub();
    resolver.AddTanh();
    return resolver;
}

#endif  // TIREMO_OPS_H_
