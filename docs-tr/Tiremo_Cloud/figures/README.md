<p class="sphinxhide" align="right"><a href="../../../Tiremo_Cloud/figures/README.md">English</a> | <a>Türkçe</a></p>

# Tiremo UI ekran görüntüleri

> Bu sayfa [İngilizce orijinalinin](../../../Tiremo_Cloud/figures/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

Şu bölümlerin ekran görüntüleri: [Tiremo UI'dan kimlik bilgileri](../Feature_Tutorials/03-Fleet_Provisioning_By_Claim/#tiremo-uidan-kimlik-bilgileri),
[Tiremo UI'da shadow güncellemesi gönderme](../Feature_Tutorials/05-Live_Shadow_Delta_Handling/#tiremo-uida-shadow-güncellemesi-gönderme),
[Tiremo UI'da telemetri ve alarmlar](../Feature_Tutorials/06-Publishing_Alarms_And_Telemetry/#tiremo-uida-telemetri-ve-alarmlar),
[Tiremo UI'da iş oluşturma](../Feature_Tutorials/07-Tiremo_Jobs/#tiremo-uida-iş-oluşturma)
ve [Tiremo UI'da pano oluşturma](../Design_Tutorials/01-Sensor_Telemetry_And_Shadow_Control/#tiremo-uida-pano-oluşturma).

| Dosya | Ne gösteriyor |
|---|---|
| `tiremo_ui_products_menu.png` | Ana menü — **Products**'ı açın |
| `tiremo_ui_select_product.png` | Ürün listesi — provizyon yapılacak ürünü açın (zaten o üründeyseniz atlayın) |
| `tiremo_ui_product_credentials.png` | Ürün sayfası — Identifier, Secret Key ve Provisioning Certificates |
| `tiremo_ui_state_controller_menu.png` | Ana menü — **State Controller**'ı açın |
| `tiremo_ui_state_controller_commands.png` | Device State Controller — komut gönderme |
| `tiremo_ui_send_state_command.png` | Send Device State Command — hedef cihaz, **Send Command** |
| `tiremo_ui_device_shadow_state.png` | Cihaz sayfası — **Device Shadow State** |
| `tiremo_ui_device_shadow_delta.png` | Cihaz sayfası — `desired` / `reported` / `delta` |
| `tiremo_ui_devices_menu.png` | Ana menü — **Devices**'ı açın |
| `tiremo_ui_select_device.png` | Cihaz listesi — yayın yapan cihazı tanımlayıcısından açın |
| `tiremo_ui_device_telemetry_alarms.png` | Cihaz sayfası — **Last 5 Telemetry Data** ve **Alarms** |
| `tiremo_ui_jobs_menu.png` | Ana menü — **Jobs**'u açın |
| `tiremo_ui_jobs_list.png` | İş listesi — **Create Job** |
| `tiremo_ui_job_create_step1.png` | İş oluşturma adım 1 — **HEALTH_CHECK**, Device hedefi |
| `tiremo_ui_job_create_step2.png` | İş oluşturma adım 2 — Snapshot, **Next** |
| `tiremo_ui_job_create_step3.png` | İş oluşturma adım 3 — **Submit** |
| `tiremo_ui_job_details_succeeded.png` | İş ayrıntısı — **Job Executions** **Succeeded** |
| `tiremo_ui_cortex_dashboard.png` | Dashboards — On/Off shadow widget'ları, telemetri grafiği ve iklim |
| `tiremo_ui_dashboard_add_widget.png` | Dashboards — **Add Widget** kataloğu |

Kimlik bilgisi değerleri gitignore'lu `certs/claim_cert.pem`, `certs/claim_key.pem`
ve `aws_iot_config.h` (`TIREMO_AWS_PRODUCT_IDENTIFIER`,
`TIREMO_AWS_PRODUCT_SECRET`) dosyalarına girer. Gerçek gizli anahtarı veya
claim anahtarını asla commit etmeyin.

Platform dökümanı: [Phase 1](https://docs.tiremo.ai/device-provisioning/first-phase)
(bootstrap sertifikası, tanımlayıcı, gizli anahtar),
[Device State Controller](https://docs.tiremo.ai/device-state-controller/introduction)
(desired güncellemeleri ve delta),
[Phase 3](https://docs.tiremo.ai/device-provisioning/third-phase)
(telemetri ve alarm yayın topic'leri),
[Jobs](https://docs.tiremo.ai/developer-guide/jobs/jobs-introduction).
