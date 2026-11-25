<p class="sphinxhide" align="right"><a>English</a> | <a href="../../../../../docs-tr/Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/Sw/certs/README.md">Türkçe</a></p>
# certs/

| File | Committed? | What it is |
|---|---|---|
| `root_ca.pem` | Yes | Amazon Root CA 1 — public, the same for every Tiremo Cloud device, from `https://www.amazontrust.com/repository/AmazonRootCA1.pem`. Verifies Tiremo Cloud's server certificate. |
| `device_cert.pem.example` / `device_key.pem.example` | Yes | Placeholders showing the format. |
| `device_cert.pem` | **No** (gitignored) | The certificate from **Provisioning Certificates** on your product page in the Tiremo UI. |
| `device_key.pem` | **No** (gitignored) | Its private key, downloaded at the same time. Keep it like a password: whoever has this file can connect to Tiremo Cloud with your product's credentials — never share it or paste it anywhere public. |

The download gives you two files named after your product's identifier,
for example `myproduct.certificate.pem` and `myproduct.private.key`.
Put them into this folder and rename them:

| Downloaded file | Rename to |
|---|---|
| `<identifier>.certificate.pem` | `device_cert.pem` |
| `<identifier>.private.key` | `device_key.pem` |

The build only looks for these two names, and stops with
`Missing device credentials` if either is missing. Both names are
gitignored, and so is any `*.key` file in a `certs/` folder. Before you
commit, check `git status` anyway: neither file may ever appear in it.

The **Secret Key** shown on the same product page is not used in this
tutorial. See the tutorial's own `README.md` for the full walkthrough.
