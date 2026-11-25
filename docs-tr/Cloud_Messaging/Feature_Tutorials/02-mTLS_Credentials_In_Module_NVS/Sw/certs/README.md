<p class="sphinxhide" align="right"><a href="../../../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/Sw/certs/README.md">English</a> | <a>Türkçe</a></p>

# certs/

> Bu sayfa [İngilizce orijinalinin](../../../../../../Cloud_Messaging/Feature_Tutorials/02-mTLS_Credentials_In_Module_NVS/Sw/certs/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

| Dosya | Commit edilir mi? | Nedir |
|---|---|---|
| `root_ca.pem` | Evet | Kendinden imzalı bir TEST kök CA'sı (`CN=Tiremo Cortex Tutorial 02 TEST Root CA - not for real use`). Eğitim 03'te bu yuva sunucuyu doğrulayan gerçek CA'yı tutar. |
| `client_cert.pem.example` / `client_key.pem.example` | Evet | Biçimi gösteren yer tutucular. |
| `client_cert.pem` | **Hayır** (gitignore'lu) | Bu eğitimin NVS'e yazma / geri okuma testi için kendinden imzalı TEST istemci sertifikası. |
| `client_key.pem` | **Hayır** (gitignore'lu) | Eşleşen TEST özel anahtarı. Asla paylaşmayın. |

Derlemeden önce TEST çiftini üretin. Bu eğitimin `Sw/` klasöründen (tek
satır):

```sh
openssl req -x509 -newkey rsa:2048 -nodes -days 3650 -keyout certs/client_key.pem -out certs/client_cert.pem -subj "/CN=Tiremo Cortex Tutorial 02 TEST client - not for real use"
```

`openssl` Git for Windows ile birlikte gelir: komutu Git Bash'te
çalıştırın ve orada konuyu `"//CN=..."` olarak yazın (Git Bash baştaki
`/` işaretini Windows yolu olarak yeniden yazar). macOS ve Linux'ta
`openssl` genellikle zaten kuruludur. Bu iki dosya eksikse CMake
yapılandırmayı reddeder.

Bunlar yalnızca test dosyalarıdır. Modülün depolamasının çalıştığını
kanıtlarlar; hiçbir sunucu onlara güvenmez.
