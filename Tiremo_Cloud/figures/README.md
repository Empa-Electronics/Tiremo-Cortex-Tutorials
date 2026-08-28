# Tiremo UI figures

Screenshots for [Getting credentials from the Tiremo UI](../Feature_Tutorials/03-Fleet_Provisioning_By_Claim/#getting-credentials-from-the-tiremo-ui),
[Sending a shadow update in the Tiremo UI](../Feature_Tutorials/05-Live_Shadow_Delta_Handling/#sending-a-shadow-update-in-the-tiremo-ui),
[Seeing telemetry and alarms in the Tiremo UI](../Feature_Tutorials/06-Publishing_Alarms_And_Telemetry/#seeing-telemetry-and-alarms-in-the-tiremo-ui),
[Creating a job in the Tiremo UI](../Feature_Tutorials/07-Tiremo_Jobs/#creating-a-job-in-the-tiremo-ui),
and [Building a dashboard in the Tiremo UI](../Design_Tutorials/01-Sensor_Telemetry_And_Shadow_Control/#building-a-dashboard-in-the-tiremo-ui).

| File | What it shows |
|---|---|
| `tiremo_ui_products_menu.png` | Main menu — open **Products** |
| `tiremo_ui_select_product.png` | Products list — open the product to provision (skip if already on that product) |
| `tiremo_ui_product_credentials.png` | Product page — Identifier, Secret Key, and Provisioning Certificates |
| `tiremo_ui_state_controller_menu.png` | Main menu — open **State Controller** |
| `tiremo_ui_state_controller_commands.png` | Device State Controller — send a command |
| `tiremo_ui_send_state_command.png` | Send Device State Command — target device, **Send Command** |
| `tiremo_ui_device_shadow_state.png` | Device page — **Device Shadow State** |
| `tiremo_ui_device_shadow_delta.png` | Device page — `desired` / `reported` / `delta` |
| `tiremo_ui_devices_menu.png` | Main menu — open **Devices** |
| `tiremo_ui_select_device.png` | Devices list — open the publishing device by identifier |
| `tiremo_ui_device_telemetry_alarms.png` | Device page — **Last 5 Telemetry Data** and **Alarms** |
| `tiremo_ui_jobs_menu.png` | Main menu — open **Jobs** |
| `tiremo_ui_jobs_list.png` | Jobs list — **Create Job** |
| `tiremo_ui_job_create_step1.png` | Create job step 1 — **HEALTH_CHECK**, Device target |
| `tiremo_ui_job_create_step2.png` | Create job step 2 — Snapshot, **Next** |
| `tiremo_ui_job_create_step3.png` | Create job step 3 — **Submit** |
| `tiremo_ui_job_details_succeeded.png` | Job details — **Job Executions** **Succeeded** |
| `tiremo_ui_cortex_dashboard.png` | Dashboards — On/Off shadow widgets plus telemetry chart and climate |
| `tiremo_ui_dashboard_add_widget.png` | Dashboards — **Add Widget** catalog |

Credential values go into gitignored `certs/claim_cert.pem`, `certs/claim_key.pem`,
and `aws_iot_config.h` (`TIREMO_AWS_PRODUCT_IDENTIFIER`,
`TIREMO_AWS_PRODUCT_SECRET`). Never commit the real secret or claim key.

Platform docs: [Phase 1](https://docs.tiremo.ai/device-provisioning/first-phase)
(bootstrap certificate, identifier, secret),
[Device State Controller](https://docs.tiremo.ai/device-state-controller/introduction)
(desired updates and delta),
[Phase 3](https://docs.tiremo.ai/device-provisioning/third-phase)
(telemetry and alarm publish topics),
[Jobs](https://docs.tiremo.ai/developer-guide/jobs/jobs-introduction).
