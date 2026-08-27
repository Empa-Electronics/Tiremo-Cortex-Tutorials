# certs/

Same layout as [Cloud Messaging 03](../../../../../Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/Sw/certs/README.md), but the gitignored pair is the shared **claim** identity, not a per-Thing device certificate.

| File | Committed? | What it is |
|---|---|---|
| `root_ca.pem` | Yes | Amazon Root CA 1 — public, identical for every AWS account, fetched from `https://www.amazontrust.com/repository/AmazonRootCA1.pem`. Verifies AWS IoT's server certificate. |
| `claim_cert.pem.example` / `claim_key.pem.example` | Yes | Placeholders showing the format. |
| `claim_cert.pem` | **No** (gitignored) | The shared claim certificate attached to the restricted fleet-provisioning policy. |
| `claim_key.pem` | **No** (gitignored) | The claim certificate's private key. Never share this or paste it anywhere public. |

Copy the two `.example` files to `claim_cert.pem` and `claim_key.pem`
in this same folder and fill in the real claim certificate and key
before building. See the tutorial's own `README.md` for the full walkthrough.

After [tutorial 03](../../03-Fleet_Provisioning_By_Claim/) the issued
identity already lives in the ESP32's `mqtt_cert` / `mqtt_key` slots.
This tutorial still embeds the claim PEMs so a factory-fresh board (or
a 10 s USER factory reset) can claim again before the shadow exchange.

Do **not** copy Cloud Messaging 03's `device_cert.pem` / `device_key.pem`
here — those are a hand-made Thing identity. The claim certificate is a
different AWS object, with a policy that is only allowed to call
CreateKeysAndCertificate / RegisterThing.
