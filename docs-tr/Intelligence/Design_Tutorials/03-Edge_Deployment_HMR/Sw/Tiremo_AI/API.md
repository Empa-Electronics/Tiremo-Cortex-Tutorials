<p class="sphinxhide" align="right"><a href="../../../../../../Intelligence/Design_Tutorials/03-Edge_Deployment_HMR/Sw/Tiremo_AI/API.md">English</a> | <a>Türkçe</a></p>

# Tiremo_AI API

> Bu sayfa [İngilizce orijinalinin](../../../../../../Intelligence/Design_Tutorials/03-Edge_Deployment_HMR/Sw/Tiremo_AI/API.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

| Makro | Değer |
|---|---|
| `TIREMO_WINDOW_SIZE` | 8 |
| `TIREMO_NUM_FEATURES` | 3 |
| `TIREMO_INPUT_SIZE` | 24 |

Model giriş biçimi: `[24]` (düz vektör). `tiremo_ai_run`, uzunluğu `TIREMO_INPUT_SIZE` = `24` olan bir tampon alır.

Eleman tipi: `float`. Şöyle doldurun:

```c
float features[TIREMO_INPUT_SIZE];  /* [24] */
features[i] = sample[i];  /* i = 0 .. TIREMO_INPUT_SIZE - 1 */
```

## Çıkış (`tiremo_ai_run`)

`TiremoInferenceResult`'ı doldurur. Durum `TIREMO_OK` veya negatif bir `tiremo_status_t` değeridir.

| Alan | Anlamı |
|---|---|
| `predicted_class` | `TIREMO_CLASS_NAMES` içindeki argmax indeksi |
| `predicted_class_label` | `TIREMO_CLASS_NAMES[predicted_class]` |
| `class_probabilities[TIREMO_N_CLASSES]` | `[0, 1]` aralığında sınıf başına olasılıklar (model çıkışı logit ise softmax uygulanır) |

`TIREMO_N_CLASSES` / `TIREMO_NUM_CLASSES` = 5. Etiketler (`TIREMO_CLASS_NAMES[i]`):

| `i` | Ad |
|---|---|
| `0` | `CIRCLE` |
| `1` | `HORIZONTAL` |
| `2` | `STANDBY` |
| `3` | `TRIANGLE` |
| `4` | `VERTICAL` |

## MQTT JSON (`tiremo_ai_format_mqtt_json`)

Yalnızca EDGE JSON gövdesini yazar (ağ yok, topic yok). Gerekli argümanlar:

| Argüman | Anlamı |
|---|---|
| `inference_result` | `tiremo_ai_run`'dan gelen, doldurulmuş `TiremoInferenceResult` |
| `window_start_unix_ms` | Penceredeki ilk örneğin Unix epoch ms değeri (UTC) |
| `window_finish_unix_ms` | Penceredeki son örneğin Unix epoch ms değeri (UTC) |
| `inference_time_ms` | `tiremo_ai_run`'ın geçen süresi, ms (Unix zamanı değil) |
| `mqtt_json_out` / uzunluk | Çağıranın tamponu; çıktı NUL ile sonlanır |

Kimlik (`TIREMO_SOLUTION_IDENTIFIER`, `TIREMO_DEPLOYMENT_IDENTIFIER`,
`TIREMO_DEPLOYMENT_ALIAS`) `tiremo_config.h`'tan gelir.

Topic soneki: `TIREMO_MQTT_TOPIC_SUFFIX` (`"inference"`).
`pub/<product_id>/<device_id>/inference` üzerinde yayınlayın.
