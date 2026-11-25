<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../../../docs-tr/Tiremo_Cloud/Design_Tutorials/01-Sensor_Telemetry_And_Shadow_Control/Sw/certs/README.md">Türkçe</a></p>
# certs/

Same layout as [Cloud Messaging 03](../../../../../Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/Sw/certs/README.md): the gitignored pair is the **Provisioning Certificates** (claim certificate and key) from your product page in the Tiremo UI.

| File | Committed? | What it is |
|---|---|---|
| `root_ca.pem` | Yes | Amazon Root CA 1 — public, the same for every Tiremo Cloud device, from `https://www.amazontrust.com/repository/AmazonRootCA1.pem`. Verifies Tiremo Cloud's server certificate. |
| `claim_cert.pem.example` / `claim_key.pem.example` | Yes | Placeholders showing the format. |
| `claim_cert.pem` | **No** (gitignored) | The claim certificate from **Provisioning Certificates** on your product page. Its policy only allows fleet provisioning (`CreateKeysAndCertificate` / `RegisterThing`). |
| `claim_key.pem` | **No** (gitignored) | The claim certificate's private key. Never share it or paste it anywhere public. |

The download gives you two files named after your product's identifier.
Put them into this folder and rename them:

| Downloaded file | Rename to |
|---|---|
| `<identifier>.certificate.pem` | `claim_cert.pem` |
| `<identifier>.private.key` | `claim_key.pem` |

If you already put them into Cloud Messaging 03 as `device_cert.pem` /
`device_key.pem`, copy those two files here under the new names — they
are the same certificate and key. The build stops with
`Missing claim credentials` if either file is missing. Both names are
gitignored, and so is any `*.key` file in a `certs/` folder; check
`git status` before you commit anyway.

See the tutorial's own `README.md` and
[tutorial 03](../../../../Feature_Tutorials/03-Fleet_Provisioning_By_Claim/#getting-credentials-from-the-tiremo-ui)
for where these are in the Tiremo UI (platform:
[Phase 1](https://docs.tiremo.ai/device-provisioning/first-phase)).

During provisioning the board receives its **own** certificate and
stores it in the ESP32's `mqtt_cert` / `mqtt_key` slots. The claim PEMs
stay embedded so a factory-fresh board (or one reset with a 10 s USER
hold) can provision again.
