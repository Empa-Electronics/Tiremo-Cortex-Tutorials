<p class="sphinxhide" align="right"><a href="../../../../ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/README.md">English</a> | <a>Türkçe</a></p>
<table class="sphinxhide" width="100%">
  <tr>
    <td align="center">
      <picture>
        <source media="(prefers-color-scheme: dark)" srcset="../../../../images/tiremo_sub_brand_1_mono_white.png">
        <img alt="Tiremo Cortex" src="../../../../images/tiremo_sub_brand_1_mono_black.png" width="260">
      </picture>
      <br>
      <img alt="" src="../../../../images/banner.png" width="100%">
      <h1>Connect/Retry State Machine</h1>
      <sub>Tiremo Cortex Eğitimleri — ESP32 + AWS IoT üretici yazılımı, ilk LED yanıp sönmesinden filoya hazır bir cihaza</sub>
    </td>
  </tr>
</table>


# Connect/Retry State Machine

***Kart: Tiremo Cortex (ESP32) · Bölüm: ESP32 Connectivity / Feature Tutorials · Zorluk: Orta***

> Bu sayfa [İngilizce orijinalinin](../../../../ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

## Giriş

Eğitim 02 bir kez bağlanır, başarısız olursa asılı kalır. Gerçek AP'ler paket düşürür, DHCP gecikir, kayıtlı ağ kapsama dışında olabilir. Bu eğitim o join'i **önce tarama**, **üç deneme** ve **MCU flash'ta son iyi kimlik bilgisi** ile sarar — neyin işe yaradığını host hatırlar, ESP32 NVS'i değil.

NTP ve BluFi sonradır. Buraya eklemeyin.

## Neler Öğreneceksiniz

- `AT+CWLAP` ile "bu SSID havada mı?" fail-fast kontrolü
- `JoinAP`'ın bool değil **got-IP / ERROR / timeout** dönmesi
- Son iyi kaydı **silmek** ile **silmeden yeniden denemek**
- Sensors eğitim 06 flash düzenini **farklı magic** ile kullanmak (`WF01`, `TRM1` değil)

## Önkoşullar

- [02-WiFi_Station_Join](../../../../ESP32_Connectivity/Feature_Tutorials/02-WiFi_Station_Join/)
- [06-Flash_Backed_Config](../../../../Sensors_And_Storage/Feature_Tutorials/06-Flash_Backed_Config/)
- Eğitim 02'deki `wifi_secrets.h` (örneği bu `Sw/` altına da kopyalayın)
- [Getting Started](../../../../Getting_Started/) içindeki `tdf.py`

## Önce tara, sonra bağlan

`AT+CWJAP` AP yokken 20 s bekleyebilir. `AT+CWLAP` birkaç saniyede görünen 2.4 GHz SSID'leri basar. Dump'ta **tırnaklı** adı (`"YourSSID"`) eşlemek: ağ havada değilse `CWJAP` hiç başlamaz.

```c
(void)snprintf(expected, sizeof(expected), "\"%s\"", pSsid);

return TIREMO_ESP32_SendCommand("AT+CWLAP\r\n", expected, timeoutMs);
```

RX halkası ve yakalama tamponu burada **512** bayt. `CWLAP` dökümü `AT`/`OK`'dan çok daha büyük. Liste çok uzunsa SSID tamponun sonuna sığmayabilir — bu eğitimin tampon sınırı, ESP-AT'nin değil.

## ERROR ile timeout

Eğitim 02'de `JoinAP` bool idi. **Yanlış şifre** ve **yavaş DHCP** yalnızca başarı etiketini beklerseniz ikisi de "false" görünür.

Taşıma, çerçeveli `\r\nERROR\r\n` görünce beklemeyi keser (o dize beklenen değilse). `JoinAP` sonucu şöyle ayırır:

| Sonuç | Anlamı | Uygulama |
|-------|--------|----------|
| `TIREMO_ESP32_JOIN_GOT_IP` | `WIFI GOT IP` | kimlik bilgisini kaydet, heartbeat |
| `TIREMO_ESP32_JOIN_ERROR` | modül join'i reddetti (yanlış şifre, …) | flash'ı **sil**, idle |
| `TIREMO_ESP32_JOIN_TIMEOUT` | 20 s, GOT IP yok, ERROR yok | en fazla 3 kez dene; üçü de fail olana kadar flash'ı **tut** |

Bir timeout glitch'tir. Üst üste üç timeout "bu kayıtlı AP kullanılabilir değil"dir — sonra sil. Çerçeveli ERROR kesindir: aynı şifreyi tekrar denemek işe yaramaz.

## Son iyi SSID (`WF01`)

[`tiremo_persist.c`](../../../../ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/Sw/Common/Src/tiremo_persist.c) Sensors 06 kalıbı: DFMC sektör 0, magic + sürüm + CRC32. Magic **`0x57463031` (`WF01`)**. Logger kaydı ile WiFi kaydı karışmaz.

Hâlâ **aynı sektör 0**. Bu eğitimi flaşlamak Sensors 06 / çevre kaydedicinin oradaki kaydını **üzerine yazar**. Beklenen davranış: görüntü başına bir sahip.

SSID sırası: geçerli `WF01` ve boş olmayan SSID → o; değilse `wifi_secrets.h`.

Join başarısı → `Persist_Save`. Flash'tan gelen SSID taramada yoksa, ERROR veya üç timeout → `Persist_Clear`. Secrets SSID taramada yoksa flash silinmez.

`AT+CWAUTOCONN=0` ve `AT+CWQAP` 02'deki gibi önce çalışır: "ne işe yaradı"yı **MCU** tutar.

## Kimlik bilgileri (şifre commit etmeyin)

```sh
cd Sw
cp wifi_secrets.h.example wifi_secrets.h
```

`wifi_secrets.h` [`.gitignore`](../../../../ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/Sw/.gitignore) içindedir. Log SSID yazar, şifreyi asla.

## Çözüm

```sh
cd Sw
cp wifi_secrets.h.example wifi_secrets.h
tdf.py build
tdf.py flash
tdf.py monitor
```

**Doğrulandı:** `.text` 19440, flaş OK. Join sonrası UART `ESP32: heartbeat still connected` gösterdi (SWD reset hâlâ CMSIS-DAP seri portunu düşürür; `tdf.py monitor` tarama/join'i kaçırabilir).

İlk boot (boş `WF01`):

```text
[INFO] Connect_Retry_State_Machine: boot
[INFO] persist: no saved SSID, using wifi_secrets.h "YourSSID"
[INFO] ESP32: scanning for "YourSSID"
[INFO] ESP32: SSID visible
[INFO] ESP32: join attempt 1/3
[INFO] ESP32: WIFI GOT IP
[INFO] persist: saved SSID "YourSSID" (password not logged)
[INFO] ESP32: heartbeat still connected
```

İkinci boot flash'tan yüklemeli:

```text
[INFO] persist: loaded SSID "YourSSID"
```

Kapsama dışı:

```text
[WARN] ESP32: SSID not visible, skip join (fail fast)
[INFO] persist: cleared last-good SSID    # SSID flash'tansa
[INFO] ESP32: idle, not connected
```

## Adım adım

1. [`main.c`](../../../../ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/Sw/Src/main.c) — 01 ile aynı
2. [`wifi_secrets.h.example`](../../../../ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/Sw/wifi_secrets.h.example) — yedek kimlik bilgisi
3. [`tiremo_persist`](../../../../ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/Sw/Common/Include/tiremo_persist.h) — sektör 0, `WF01`
4. [`tiremo_esp32.c`](../../../../ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/Sw/Drivers/esp32/Src/tiremo_esp32.c) — `ScanForSsid`, `JoinStatus`
5. [`prv_user_code.c`](../../../../ESP32_Connectivity/Feature_Tutorials/03-Connect_Retry_State_Machine/Sw/Src/prv_user_code.c) — persist veya secrets → tarama → 1..3 join → kaydet veya sil → heartbeat / idle

## Bitti sayılır

`CWLAP`'ta olmayan SSID `CWJAP` başlatmaz. Kapsamadaki AP üç denemede bağlanır; sonraki reset `persist: loaded SSID` yazar. Çerçeveli `ERROR` kalan denemeleri yakmaz, idle'a geçer.

<p align="center"><img alt="" src="../../../../images/banner.png" width="100%"></p>

<p class="sphinxhide" align="center"><sub>Tiremo Cortex Tutorials — Empa Electronics</sub></p>
