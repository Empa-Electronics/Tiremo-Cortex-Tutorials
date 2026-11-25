<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../../../docs-tr/Tiremo_Cloud/Feature_Tutorials/05-Live_Shadow_Delta_Handling/Sw/certs/README.md">Türkçe</a></p>
# certs/

Same layout as [tutorial 03](../../../03-Fleet_Provisioning_By_Claim/Sw/certs/README.md): the gitignored pair is the **Provisioning Certificates** (claim certificate and key) from your product page in the Tiremo UI.

| File | Committed? | What it is |
|---|---|---|
| `root_ca.pem` | Yes | Amazon Root CA 1 — public, the same for every Tiremo Cloud device, from `https://www.amazontrust.com/repository/AmazonRootCA1.pem`. Verifies Tiremo Cloud's server certificate. |
| `claim_cert.pem.example` / `claim_key.pem.example` | Yes | Placeholders showing the format. |
| `claim_cert.pem` | **No** (gitignored) | The claim certificate from **Provisioning Certificates** on your product page. Its policy only allows fleet provisioning (`CreateKeysAndCertificate` / `RegisterThing`). |
| `claim_key.pem` | **No** (gitignored) | The claim certificate's private key. Never share it or paste it anywhere public. |

The easiest way to fill this folder is to copy `claim_cert.pem` and
`claim_key.pem` from tutorial 03's `Sw/certs/`. If you start from the
download instead, it gives you two files named after your product's
identifier. Put them into this folder and rename them:

| Downloaded file | Rename to |
|---|---|
| `<identifier>.certificate.pem` | `claim_cert.pem` |
| `<identifier>.private.key` | `claim_key.pem` |

The build stops with `Missing claim credentials` if either file is
missing. Both names are gitignored, and so is any `*.key` file in a
`certs/` folder; check `git status` before you commit anyway.

After [tutorial 03](../../../03-Fleet_Provisioning_By_Claim/README.md)
the issued identity already lives in the ESP32's `mqtt_cert` /
`mqtt_key` slots, and this firmware connects with it. The claim PEMs are
still built in so a factory-fresh board (or one reset with a 10 s USER
hold) can run the claim exchange again before the shadow exchange.

See tutorial 03's
[Getting credentials from the Tiremo UI](../../../03-Fleet_Provisioning_By_Claim/README.md#getting-credentials-from-the-tiremo-ui)
section for where these are in the Tiremo UI. Platform docs:
[Phase 1](https://docs.tiremo.ai/device-provisioning/first-phase).
