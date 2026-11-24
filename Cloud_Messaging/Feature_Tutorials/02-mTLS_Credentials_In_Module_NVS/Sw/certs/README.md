# certs/

| File | Committed? | What it is |
|---|---|---|
| `root_ca.pem` | Yes | A self-signed TEST root CA (`CN=Tiremo Cortex Tutorial 02 TEST Root CA - not for real use`). In tutorial 03 this slot holds the real CA that verifies the server. |
| `client_cert.pem.example` / `client_key.pem.example` | Yes | Placeholders showing the format. |
| `client_cert.pem` | **No** (gitignored) | Self-signed TEST client certificate for this tutorial's NVS write/read-back. |
| `client_key.pem` | **No** (gitignored) | Matching TEST private key. Never share it. |

Before building, generate the TEST pair. From this tutorial's `Sw/`
folder (one line):

```sh
openssl req -x509 -newkey rsa:2048 -nodes -days 3650 -keyout certs/client_key.pem -out certs/client_cert.pem -subj "/CN=Tiremo Cortex Tutorial 02 TEST client - not for real use"
```

`openssl` comes with Git for Windows: run the command in Git Bash, and
write the subject as `"//CN=..."` there (Git Bash rewrites a leading `/`
as a Windows path). On macOS and Linux, `openssl` is usually already
installed. CMake refuses to configure if those two files are missing.

These are test files only. They prove that the module's storage works;
no server trusts them.
