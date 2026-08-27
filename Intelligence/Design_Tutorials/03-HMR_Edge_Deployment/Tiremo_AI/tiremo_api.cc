// GENERATED FILE — do not edit.
// config hash: da4f5bc5c01c82ef  model: ff8c7ffa-ca80-4f3a-81cb-fc39f90f1c74  generated: 2026-08-27T21:32:01Z
#include "tiremo_api.h"
#include "tiremo_config.h"
#include "tiremo_ops.h"
#include "model.h"

#include <math.h>

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/schema/schema_generated.h"

namespace {
    const tflite::Model* model = nullptr;
    alignas(16) uint8_t tensor_arena[TIREMO_ARENA_SIZE];
    tflite::MicroInterpreter* interpreter = nullptr;
    TfLiteTensor* input_tensor = nullptr;
    TfLiteTensor* output_tensor = nullptr;

    // Storage for the interpreter object
    alignas(alignof(tflite::MicroInterpreter)) uint8_t interpreter_buf[sizeof(tflite::MicroInterpreter)];
}

extern "C" tiremo_status_t tiremo_init(void) {
    model = tflite::GetModel(tiremo_model_data);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        MicroPrintf("Model schema version mismatch: got %lu, expected %d",
                    model->version(), TFLITE_SCHEMA_VERSION);
        return TIREMO_ERROR_INIT;
    }

    static auto op_resolver = tiremo_get_op_resolver();

    interpreter = new (interpreter_buf) tflite::MicroInterpreter(
        model, op_resolver, tensor_arena, TIREMO_ARENA_SIZE);

    if (interpreter->AllocateTensors() != kTfLiteOk) {
        MicroPrintf("AllocateTensors() failed");
        return TIREMO_ERROR_INIT;
    }

    input_tensor = interpreter->input(0);
    output_tensor = interpreter->output(0);
    return TIREMO_OK;
}

extern "C" tiremo_status_t tiremo_invoke(const float* input, int input_len) {
    if (!interpreter || !input) return TIREMO_ERROR_ARG;
    if (input_len != TIREMO_INPUT_SIZE) return TIREMO_ERROR_ARG;

    float* input_data = tflite::GetTensorData<float>(input_tensor);
    for (int i = 0; i < input_len; ++i) {
        input_data[i] = input[i];
    }

    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
        return TIREMO_ERROR_INVOKE;
    }
    return TIREMO_OK;
}

extern "C" tiremo_status_t tiremo_get_scores(float* out_scores, int out_len) {
    if (!interpreter || !out_scores) return TIREMO_ERROR_ARG;
    int copy_len = out_len < TIREMO_OUTPUT_SIZE ? out_len : TIREMO_OUTPUT_SIZE;

    const float* out_data = tflite::GetTensorData<float>(output_tensor);
    for (int i = 0; i < copy_len; ++i) {
        out_scores[i] = out_data[i];
    }

    return TIREMO_OK;
}

extern "C" int tiremo_get_predicted_class(void) {
    if (!interpreter) return -1;

    const float* out_data = tflite::GetTensorData<float>(output_tensor);
    int best = 0;
    float best_val = out_data[0];
    for (int i = 1; i < TIREMO_OUTPUT_SIZE; ++i) {
        if (out_data[i] > best_val) { best_val = out_data[i]; best = i; }
    }

    return best;
}

extern "C" const char* tiremo_get_class_name(int class_id) {
    if (class_id < 0 || class_id >= TIREMO_NUM_CLASSES) return "UNKNOWN";
    return TIREMO_CLASS_NAMES[class_id];
}

extern "C" const char* tiremo_get_predicted_class_name(void) {
    return tiremo_get_class_name(tiremo_get_predicted_class());
}

static int tiremo_ai_ensure_init(void)
{
    if (interpreter != NULL) {
        return TIREMO_OK;
    }
    return tiremo_init();
}

#define TIREMO_MQTT_SCALAR float

/* Convert logits to probabilities in place. Skip when the vector already
 * looks like a calibrated distribution (all in ~[0, 1], sum ~ 1). */
static void tiremo_ai_softmax(float *x, size_t n)
{
    size_t i;
    float max_v;
    float sum;

    if (n == 0) {
        return;
    }
    max_v = x[0];
    for (i = 1; i < n; i++) {
        if (x[i] > max_v) {
            max_v = x[i];
        }
    }
    sum = (float)0.0f;
    for (i = 0; i < n; i++) {
        x[i] = expf(x[i] - max_v);
        sum += x[i];
    }
    if (sum <= (float)0.0f) {
        float uniform = (float)1.0f / (float)n;
        for (i = 0; i < n; i++) {
            x[i] = uniform;
        }
        return;
    }
    for (i = 0; i < n; i++) {
        x[i] /= sum;
    }
}

static int tiremo_ai_looks_like_probabilities(const float *x, size_t n)
{
    size_t i;
    float sum;
    float delta;
    const float atol_bound = (float)1e-3f;
    const float atol_sum = (float)1e-2f;

    if (n < 2) {
        return 0;
    }
    sum = (float)0.0f;
    for (i = 0; i < n; i++) {
        if (x[i] < -atol_bound || x[i] > (float)1.0f + atol_bound) {
            return 0;
        }
        sum += x[i];
    }
    delta = sum - (float)1.0f;
    if (delta < (float)0.0f) {
        delta = -delta;
    }
    return delta <= atol_sum;
}

static void tiremo_ai_ensure_probabilities(float *x, size_t n)
{
    if (n < 2) {
        return;
    }
    if (tiremo_ai_looks_like_probabilities(x, n)) {
        return;
    }
    tiremo_ai_softmax(x, n);
}


static int tiremo_ai_classify(const float *input,
                              size_t       n_classes,
                              float       *probabilities_out,
                              int         *predicted_class_out)
{
    if (input == NULL || probabilities_out == NULL)     return TIREMO_ERR_NULL_PTR;
    if (n_classes == 0)                                 return TIREMO_ERR_INPUT_LEN;

    tiremo_status_t status = tiremo_invoke(input, TIREMO_INPUT_SIZE);
    if (status != TIREMO_OK) {
        return status;
    }

    status = tiremo_get_scores(probabilities_out, (int)n_classes);
    if (status != TIREMO_OK) {
        return status;
    }

    if (predicted_class_out != NULL) {
        *predicted_class_out = tiremo_get_predicted_class();
    }
    tiremo_ai_ensure_probabilities(probabilities_out, n_classes);
    return TIREMO_OK;
}

static int tiremo_ai_regress(const float *input, float *value_out)
{
    if (input == NULL || value_out == NULL)             return TIREMO_ERR_NULL_PTR;

    tiremo_status_t status = tiremo_invoke(input, TIREMO_INPUT_SIZE);
    if (status != TIREMO_OK) {
        return status;
    }
    return tiremo_get_scores(value_out, 1);
}

static int tiremo_ai_detect_anomaly(const float        *input,
                                    TiremoAnomalyResult *result_out)
{
    if (input == NULL || result_out == NULL)             return TIREMO_ERR_NULL_PTR;
    if (!TIREMO_IS_ANOMALY_MODEL)                        return TIREMO_ERR_TASK_TYPE;

    tiremo_status_t status = tiremo_invoke(input, TIREMO_INPUT_SIZE);
    if (status != TIREMO_OK) {
        return status;
    }

#if TIREMO_OUTPUT_SIZE <= 1
    float score = 0.0f;
    status = tiremo_get_scores(&score, 1);
    if (status != TIREMO_OK) {
        return status;
    }
    result_out->score = (double)score;
#if TIREMO_ANOMALY_COMPARISON == TIREMO_ANOMALY_COMPARE_ABOVE
    result_out->is_anomaly = result_out->score > TIREMO_ANOMALY_THRESHOLD;
#else
    result_out->is_anomaly = result_out->score < TIREMO_ANOMALY_THRESHOLD;
#endif
#else
    float scores[TIREMO_OUTPUT_SIZE];
    status = tiremo_get_scores(scores, TIREMO_OUTPUT_SIZE);
    if (status != TIREMO_OK) {
        return status;
    }
    {
        int predicted = tiremo_get_predicted_class();
        if (predicted < 0) {
            predicted = 0;
        }
        result_out->score = (double)scores[predicted];
        result_out->is_anomaly = predicted != 0;
    }
#endif

#if TIREMO_N_CLASSES >= 2
    result_out->label = TIREMO_CLASS_NAMES[result_out->is_anomaly ? 1 : 0];
#else
    result_out->label = NULL;
#endif

    return TIREMO_OK;
}

extern "C" {

#include <stdio.h>
#include <stdarg.h>

static int tiremo_mqtt_append(char **cur, const char *end, const char *fmt, ...)
{
    size_t avail;
    int    n;
    va_list ap;

    if (cur == NULL || *cur == NULL || end == NULL || *cur >= end) {
        return -1;
    }
    avail = (size_t)(end - *cur);
    va_start(ap, fmt);
    n = vsnprintf(*cur, avail, fmt, ap);
    va_end(ap);
    if (n < 0 || (size_t)n >= avail) {
        return -1;
    }
    *cur += n;
    return 0;
}

static int tiremo_mqtt_append_escaped(char **cur, const char *end, const char *s)
{
    if (s == NULL) {
        s = "";
    }
    for (; *s != '\0'; s++) {
        if (*s == '"' || *s == '\\') {
            if (tiremo_mqtt_append(cur, end, "\\%c", *s) != 0) {
                return -1;
            }
        } else if ((unsigned char)*s < 0x20U) {
            if (tiremo_mqtt_append(cur, end, "\\u%04x", (unsigned)(unsigned char)*s) != 0) {
                return -1;
            }
        } else {
            if (tiremo_mqtt_append(cur, end, "%c", *s) != 0) {
                return -1;
            }
        }
    }
    return 0;
}

static int tiremo_mqtt_append_common_prefix(
    char                       **cur,
    const char                  *end,
    const char                  *task_type_display,
    int32_t                      inference_time_ms)
{
    if (tiremo_mqtt_append(cur, end, "{\"solutionIdentifier\":\"") != 0 ||
        tiremo_mqtt_append_escaped(cur, end, TIREMO_SOLUTION_IDENTIFIER) != 0 ||
        tiremo_mqtt_append(cur, end, "\",") != 0) {
        return -1;
    }
    if (tiremo_mqtt_append(cur, end, "\"deploymentIdentifier\":\"") != 0 ||
        tiremo_mqtt_append_escaped(cur, end, TIREMO_DEPLOYMENT_IDENTIFIER) != 0 ||
        tiremo_mqtt_append(cur, end, "\",") != 0) {
        return -1;
    }
    if (tiremo_mqtt_append(cur, end, "\"deploymentAlias\":\"") != 0 ||
        tiremo_mqtt_append_escaped(cur, end, TIREMO_DEPLOYMENT_ALIAS) != 0 ||
        tiremo_mqtt_append(cur, end, "\",") != 0) {
        return -1;
    }
    if (tiremo_mqtt_append(
            cur,
            end,
            "\"deploymentType\":\"EDGE\","
            "\"success\":true,"
            "\"failureIssue\":null,"
            "\"failureReason\":null,"
            "\"severity\":null,"
            "\"inferenceTimeMs\":%d,"
            "\"taskType\":\"%s\","
            "\"results\":",
            (int)inference_time_ms,
            task_type_display) != 0) {
        return -1;
    }
    return 0;
}

/* Copies caller-supplied Unix epoch milliseconds. Does not generate time.
 * Manual decimal print — newlib-nano often mishandles %lld (emits "ld"). */
static int tiremo_mqtt_append_i64(char **cur, const char *end, int64_t v)
{
    char     tmp[24];
    char    *p = tmp + (int)sizeof(tmp);
    uint64_t u;
    int      neg = 0;

    *--p = '\0';
    if (v < 0) {
        neg = 1;
        u = (uint64_t)(-(v + 1)) + 1ULL;
    } else {
        u = (uint64_t)v;
    }
    if (u == 0ULL) {
        *--p = '0';
    } else {
        while (u > 0ULL) {
            *--p = (char)('0' + (unsigned)(u % 10ULL));
            u /= 10ULL;
        }
    }
    if (neg != 0) {
        *--p = '-';
    }
    return tiremo_mqtt_append(cur, end, "%s", p);
}

/* Fixed 4 digits after the decimal. */
static int tiremo_mqtt_append_f(char **cur, const char *end, double v)
{
    char     tmp[6];
    double   a;
    uint64_t scaled;
    unsigned frac;

    if (v != v || v > 1.0e15 || v < -1.0e15) {
        return tiremo_mqtt_append(cur, end, "0.0000");
    }
    a = v < 0.0 ? -v : v;
    scaled = (uint64_t)(a * 10000.0 + 0.5);
    if (v < 0.0 && scaled != 0ULL) {
        if (tiremo_mqtt_append(cur, end, "-") != 0) {
            return -1;
        }
    }
    if (tiremo_mqtt_append_i64(cur, end, (int64_t)(scaled / 10000ULL)) != 0) {
        return -1;
    }
    frac = (unsigned)(scaled % 10000ULL);
    tmp[0] = '.';
    tmp[1] = (char)('0' + (frac / 1000U));
    tmp[2] = (char)('0' + ((frac / 100U) % 10U));
    tmp[3] = (char)('0' + ((frac / 10U) % 10U));
    tmp[4] = (char)('0' + (frac % 10U));
    tmp[5] = '\0';
    return tiremo_mqtt_append(cur, end, "%s", tmp);
}

static int tiremo_mqtt_append_results_open(
    char   **cur,
    const char *end,
    int64_t window_start_unix_ms,
    int64_t window_finish_unix_ms)
{
    if (tiremo_mqtt_append(cur, end, "{\"windowStartTimestamp\":") != 0 ||
        tiremo_mqtt_append_i64(cur, end, window_start_unix_ms) != 0 ||
        tiremo_mqtt_append(cur, end, ",\"windowFinishTimestamp\":") != 0 ||
        tiremo_mqtt_append_i64(cur, end, window_finish_unix_ms) != 0 ||
        tiremo_mqtt_append(cur, end, ",") != 0) {
        return -1;
    }
    return 0;
}


int tiremo_ai_run(const float            *features,
                  TiremoInferenceResult *result_out)
{
    int status;
    int predicted = 0;

    if (features == NULL || result_out == NULL) {
        return TIREMO_ERR_NULL_PTR;
    }

    status = tiremo_ai_ensure_init();
    if (status != TIREMO_OK) {
        return status;
    }

    status = tiremo_ai_classify(
        features, TIREMO_N_CLASSES, result_out->class_probabilities, &predicted);
    if (status != TIREMO_OK) {
        return status;
    }

    result_out->predicted_class = predicted;
    result_out->predicted_class_label =
        (predicted >= 0 && (size_t)predicted < TIREMO_N_CLASSES)
            ? TIREMO_CLASS_NAMES[predicted]
            : "";
    return TIREMO_OK;
}


int tiremo_ai_format_mqtt_json(
    const TiremoInferenceResult *inference_result,
    int64_t                      window_start_unix_ms,
    int64_t                      window_finish_unix_ms,
    int32_t                      inference_time_ms,
    char                        *mqtt_json_out,
    size_t                       mqtt_json_out_len)
{
    const char *label;
    char       *cur;
    const char *end;
    size_t      i;
    int         predicted;

    if (inference_result == NULL || mqtt_json_out == NULL) {
        return TIREMO_ERR_NULL_PTR;
    }
    if (mqtt_json_out_len == 0 || TIREMO_N_CLASSES == 0) {
        return TIREMO_ERR_BUFFER;
    }

    predicted = inference_result->predicted_class;
    label = inference_result->predicted_class_label != NULL
                ? inference_result->predicted_class_label
                : "";

    cur = mqtt_json_out;
    end = mqtt_json_out + mqtt_json_out_len;
    if (tiremo_mqtt_append_common_prefix(&cur, end, "CLASSIFICATION",
                                         inference_time_ms) != 0) {
        return TIREMO_ERR_BUFFER;
    }
    if (tiremo_mqtt_append_results_open(&cur, end, window_start_unix_ms,
                                        window_finish_unix_ms) != 0) {
        return TIREMO_ERR_BUFFER;
    }
    if (tiremo_mqtt_append(&cur, end, "\"predictions\":%d,\"predictedClassLabel\":\"", predicted) != 0 ||
        tiremo_mqtt_append_escaped(&cur, end, label) != 0 ||
        tiremo_mqtt_append(&cur, end, "\",\"probabilities\":{") != 0) {
        return TIREMO_ERR_BUFFER;
    }
    for (i = 0; i < TIREMO_N_CLASSES; i++) {
        if (i > 0) {
            if (tiremo_mqtt_append(&cur, end, ",") != 0) {
                return TIREMO_ERR_BUFFER;
            }
        }
        if (tiremo_mqtt_append(&cur, end, "\"") != 0 ||
            tiremo_mqtt_append_escaped(&cur, end, TIREMO_CLASS_NAMES[i]) != 0 ||
            tiremo_mqtt_append(&cur, end, "\":") != 0 ||
            tiremo_mqtt_append_f(&cur, end, inference_result->class_probabilities[i]) != 0) {
            return TIREMO_ERR_BUFFER;
        }
    }
    if (tiremo_mqtt_append(&cur, end, "}}}") != 0) {
        return TIREMO_ERR_BUFFER;
    }
    return TIREMO_OK;
}


const char *tiremo_ai_strerror(int error_code)
{
    switch (error_code) {
        case TIREMO_OK:             return "OK";
        case TIREMO_ERROR_INIT:     return "Interpreter initialization failed";
        case TIREMO_ERROR_INVOKE:   return "Interpreter invoke failed";
        case TIREMO_ERROR_ARG:      return "Invalid argument";
        case TIREMO_ERR_NULL_PTR:   return "NULL pointer passed to tiremo API";
        case TIREMO_ERR_INPUT_LEN:  return "Input buffer is smaller than expected";
        case TIREMO_ERR_OUTPUT_LEN: return "Output buffer is smaller than expected";
        case TIREMO_ERR_TASK_TYPE:  return "API is not valid for this model task type";
        case TIREMO_ERR_BUFFER:     return "Output buffer is too small for MQTT JSON";
        default:                    return "Unknown error";
    }
}

}
