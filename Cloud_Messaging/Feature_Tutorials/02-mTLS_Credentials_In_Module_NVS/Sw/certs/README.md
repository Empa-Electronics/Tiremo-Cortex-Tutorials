# certs/

| File | Committed? | What it is |
|---|---|---|
| `root_ca.pem` | Yes | Public CA used to verify the server (Amazon Root CA 1 in later tutorials). |
| `client_cert.pem.example` / `client_key.pem.example` | Yes | Placeholders showing the format. |
| `client_cert.pem` | **No** (gitignored) | Self-signed TEST client certificate for this tutorial's NVS write/read-back. |
| `client_key.pem` | **No** (gitignored) | Matching TEST private key. Never share it. |

Copy the two `.example` files to `client_cert.pem` and `client_key.pem`
in this same folder and paste a locally generated test pair before
building (`openssl req -x509 -newkey rsa:2048 ...`, subject labeled
TEST / not for real use). CMake refuses to configure if those two
files are missing.
