<p class="sphinxhide" align="right"><a href="../../../../../../Intelligence/Design_Tutorials/03-Edge_Deployment_HMR/Sw/Tiremo_AI/IMPLEMENTATION.md">English</a> | <a>Türkçe</a></p>

# Uygulama

> Bu sayfa [İngilizce orijinalinin](../../../../../../Intelligence/Design_Tutorials/03-Edge_Deployment_HMR/Sw/Tiremo_AI/IMPLEMENTATION.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

1. Genel başlık dosyasını ekleyin:

```c
#include "tiremo_api.h"
```

2. `float features[TIREMO_INPUT_SIZE]` dizisini doldurun. Model giriş biçimi: `[24]` (düz vektör). `tiremo_ai_run`, uzunluğu `TIREMO_INPUT_SIZE` = `24` olan bir tampon alır. Veri toplama zaten yapılmıştır.

3. `tiremo_ai_run`'ı çağırın:

```c
float features[TIREMO_INPUT_SIZE];  /* [24] */
features[i] = sample[i];  /* i = 0 .. TIREMO_INPUT_SIZE - 1 */
TiremoInferenceResult result;
int rc = tiremo_ai_run(features, &result);
if (rc != TIREMO_OK) {
    /* tiremo_ai_strerror(rc) */
}
```

4. Çağrının süresini cihaz saatiyle ölçün. Geçen milisaniyeyi `inference_time_ms` olarak verin.

5. `tiremo_ai_format_mqtt_json`'ı çağırın. Pencere zaman damgaları Unix epoch milisaniyesidir. Solution ve deployment kimlikleri zaten `tiremo_config.h` içindedir.

```c
char mqtt_json[1024];
rc = tiremo_ai_format_mqtt_json(
    &result, window_start_unix_ms, window_finish_unix_ms,
    inference_time_ms, mqtt_json, sizeof mqtt_json);
```

6. JSON'u `pub/<product_id>/<device_id>/inference` üzerinde kendiniz yayınlayın.

7. `CC`'nin yanına, `gcc` ile aynı önekle C++ derleyicisini ayarlayın:

```makefile
CXX := $(CROSS)g++
```

8. `OBJS` tanımlandıktan sonra derleme parçasını ekleyin:

```makefile
include $(TIREMO_DIR)/Tiremo_AI/tiremo_makefile.mk
```

9. ELF'i `$(CC)` yerine `$(CXX)` ile bağlayın.

10. Uygulamanın `.c` dosyaları C olarak kalır; yalnızca `#include "tiremo_api.h"` yaparlar.

## Hedef

Bu paket `float` aritmetiği kullanır. MCU'da **FPU'yu etkinleştirin** ve
donanım float ABI'si ile derleyip bağlayın. Bu paketle `-mfloat-abi=soft`
kullanmayın. Derleyiciye ve bağlayıcıya aynı CPU/FPU bayraklarını verin.

Örnek (Cortex-M4; çekirdeğinize göre `-mfpu` seçin — M33/M7 farklıdır):

```makefile
CPU_FLAGS := -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS   += $(CPU_FLAGS)
CXXFLAGS += $(CPU_FLAGS)
LDFLAGS  += $(CPU_FLAGS)
```

BSP bunu zaten yapmıyorsa, `tiremo_ai_run`'ı çağırmadan önce açılışta FPU'yu
etkinleştirin (Cortex-M: `SCB->CPACR` ile yardımcı işlemci erişimi).
