# certs/

| File | Committed? | What it is |
|---|---|---|
| `root_ca.pem` | Yes | Amazon Root CA 1 — public, identical for every AWS account, fetched from `https://www.amazontrust.com/repository/AmazonRootCA1.pem`. Verifies AWS IoT's server certificate. |
| `device_cert.pem.example` / `device_key.pem.example` | Yes | Placeholders showing the format. |
| `device_cert.pem` | **No** (gitignored) | Your AWS IoT Thing's own certificate, downloaded from the AWS IoT console when you created it. |
| `device_key.pem` | **No** (gitignored) | Your AWS IoT Thing's own private key, downloaded at the same time — AWS only shows it once. Never share this or paste it anywhere public. |

Copy the two `.example` files to `device_cert.pem` and `device_key.pem`
in this same folder and fill in your real Thing's certificate and key
before building. See the tutorial's own `README.md` for the full walkthrough.
