// GENERATED FILE — do not edit.
// config hash: da4f5bc5c01c82ef  model: ff8c7ffa-ca80-4f3a-81cb-fc39f90f1c74  generated: 2026-08-27T21:32:01Z
#ifndef TIREMO_API_H_
#define TIREMO_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include "tiremo_config.h"

typedef enum {
    TIREMO_OK             =  0,
    TIREMO_ERROR_INIT     =  1,
    TIREMO_ERROR_INVOKE   =  2,
    TIREMO_ERROR_ARG      =  3,
    TIREMO_ERR_NULL_PTR   = -1,
    TIREMO_ERR_INPUT_LEN  = -2,
    TIREMO_ERR_OUTPUT_LEN = -3,
    TIREMO_ERR_TASK_TYPE  = -4,
    TIREMO_ERR_BUFFER     = -5
} tiremo_status_t;

typedef struct {
    double      score;
    int         is_anomaly;
    const char *label;
} TiremoAnomalyResult;

typedef struct {
    int          predicted_class;
    const char  *predicted_class_label;
    float       class_probabilities[TIREMO_N_CLASSES];
} TiremoInferenceResult;


tiremo_status_t tiremo_init(void);
tiremo_status_t tiremo_invoke(const float* input, int input_len);
tiremo_status_t tiremo_get_scores(float* out_scores, int out_len);
int             tiremo_get_predicted_class(void);
const char*     tiremo_get_class_name(int class_id);
const char*     tiremo_get_predicted_class_name(void);

/**
 * Run this package's task and fill inference_result.
 * Calls tiremo_init() on first use. Time this call in application code
 * (elapsed milliseconds).
 */
int tiremo_ai_run(const float            *features,
                  TiremoInferenceResult *result_out);

/**
 * Format an EDGE MQTT JSON document. Does not run inference.
 *
 * @param window_start_unix_ms  Unix epoch milliseconds (UTC), first sample in window
 * @param window_finish_unix_ms Unix epoch milliseconds (UTC), last sample in window
 * @param inference_time_ms     Elapsed milliseconds for tiremo_ai_run() (not Unix time)
 *
 * Recommended mqtt_json_out size: 512–1024 bytes (more if many classes).
 */
int tiremo_ai_format_mqtt_json(
    const TiremoInferenceResult *inference_result,
    int64_t                      window_start_unix_ms,
    int64_t                      window_finish_unix_ms,
    int32_t                      inference_time_ms,
    char                        *mqtt_json_out,
    size_t                       mqtt_json_out_len);

const char *tiremo_ai_strerror(int error_code);

#ifdef __cplusplus
}
#endif

#endif  // TIREMO_API_H_
