# certs/

Same layout as [Cloud Messaging 03](../../../../../Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/Sw/certs/README.md), but the gitignored pair is the shared **claim** identity, not a per-Thing device certificate.

| File | Committed? | What it is |
|---|---|---|
| `root_ca.pem` | Yes | Amazon Root CA 1 — public, identical for every Tiremo Cloud tenant, fetched from `https://www.amazontrust.com/repository/AmazonRootCA1.pem`. Verifies Tiremo Cloud's server certificate. |
| `claim_cert.pem.example` / `claim_key.pem.example` | Yes | Placeholders showing the format. |
| `claim_cert.pem` | **No** (gitignored) | The shared claim certificate attached to the restricted fleet-provisioning policy. |
| `claim_key.pem` | **No** (gitignored) | The claim certificate's private key. Never share this or paste it anywhere public. |

Copy the two `.example` files to `claim_cert.pem` and `claim_key.pem`
in this same folder and paste the real claim certificate and key from
the **Tiremo UI** before building. See the tutorial's
[Getting credentials from the Tiremo UI](../../README.md#getting-credentials-from-the-tiremo-ui)
section (Products menu → your product → Identifier, Secret Key,
Provisioning Certificates). Platform docs:
[Phase 1](https://docs.tiremo.ai/device-provisioning/first-phase),
[fleet provisioning](https://docs.tiremo.ai/device-provisioning/second-phase),
[TLS SNI](https://docs.tiremo.ai/device-provisioning/tls-sni).

Do **not** copy Cloud Messaging 03's `device_cert.pem` / `device_key.pem`
here — those are a hand-made Thing identity. This tutorial's claim
certificate is a different platform object, with a policy that is only
allowed to call CreateKeysAndCertificate / RegisterThing.

The issued per-device certificate/key do not start in this folder: Tiremo Cloud
returns them over MQTT and the firmware writes them directly to the
ESP32's `mqtt_cert` / `mqtt_key` SYSMFG slots.
