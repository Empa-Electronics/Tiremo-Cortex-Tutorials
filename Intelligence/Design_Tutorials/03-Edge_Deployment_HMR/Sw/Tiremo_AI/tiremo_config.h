// GENERATED FILE — do not edit.
// config hash: da4f5bc5c01c82ef  model: ff8c7ffa-ca80-4f3a-81cb-fc39f90f1c74  generated: 2026-08-27T21:32:01Z
/*
 * Model metadata:
 *   modelIdentifier:       ff8c7ffa-ca80-4f3a-81cb-fc39f90f1c74
 *   algorithmName: ANN
 *   generated:     2026-08-27T21:33:28Z
 */

#ifndef TIREMO_CONFIG_H_
#define TIREMO_CONFIG_H_

/* Model input shape: [24] (flat). TIREMO_INPUT_SIZE = 24.
 * Fill as: features[i] = sample[i]  (i = 0 .. TIREMO_INPUT_SIZE-1)
 * Feature order: not provided in deployment metadata */
/* Task type identifier */
#define TIREMO_TASK_TYPE        "CLASSIFICATION"

#define TIREMO_INPUT_SIZE       24
#define TIREMO_WINDOW_SIZE      8
#define TIREMO_NUM_FEATURES     3
#define TIREMO_N_FEATURES       TIREMO_NUM_FEATURES
#define TIREMO_OUTPUT_SIZE      5
#define TIREMO_NUM_CLASSES      5
#define TIREMO_N_CLASSES        TIREMO_NUM_CLASSES


#define TIREMO_SAMPLING_FREQUENCY_HZ    0.0f
#define TIREMO_INFERENCE_INTERVAL_S     0.0f

/* Deployment identity (baked at package generation; used by MQTT JSON) */
#define TIREMO_SOLUTION_IDENTIFIER    "c7eb0868-cb8e-4d75-ae63-350f1429ee33"
#define TIREMO_DEPLOYMENT_IDENTIFIER  "9540b526-fbd3-4ec3-b7c4-15cdba0f9f76"
#define TIREMO_DEPLOYMENT_ALIAS       "edge-deployment-1"

#define TIREMO_MQTT_TOPIC_SUFFIX      "inference"
/* Topic: pub/<product_id>/<device_id>/inference */

/* Anomaly decision metadata */
#define TIREMO_IS_ANOMALY_MODEL       0
#define TIREMO_ANOMALY_THRESHOLD      0.0
#define TIREMO_ANOMALY_COMPARE_BELOW  0
#define TIREMO_ANOMALY_COMPARE_ABOVE  1
#define TIREMO_ANOMALY_COMPARISON     TIREMO_ANOMALY_COMPARE_BELOW

#define TIREMO_INPUT_DTYPE_FLOAT32   1
#define TIREMO_OUTPUT_DTYPE_FLOAT32  1


#ifndef TIREMO_ARENA_SIZE
#define TIREMO_ARENA_SIZE       7168
#endif

static const char* const TIREMO_CLASS_NAMES[TIREMO_NUM_CLASSES] = {
    "CIRCLE", "HORIZONTAL", "STANDBY", "TRIANGLE", "VERTICAL"
};

#endif  // TIREMO_CONFIG_H_
