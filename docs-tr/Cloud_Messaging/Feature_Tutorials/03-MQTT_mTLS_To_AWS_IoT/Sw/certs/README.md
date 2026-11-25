<p class="sphinxhide" align="right"><a href="../../../../../../Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/Sw/certs/README.md">English</a> | <a>Türkçe</a></p>

# certs/

> Bu sayfa [İngilizce orijinalinin](../../../../../../Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/Sw/certs/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

| Dosya | Commit edilir mi? | Nedir |
|---|---|---|
| `root_ca.pem` | Evet | Amazon Root CA 1 — herkese açık, her Tiremo Cloud cihazı için aynı, `https://www.amazontrust.com/repository/AmazonRootCA1.pem` adresinden. Tiremo Cloud'un sunucu sertifikasını doğrular. |
| `device_cert.pem.example` / `device_key.pem.example` | Evet | Biçimi gösteren yer tutucular. |
| `device_cert.pem` | **Hayır** (gitignore'lu) | Tiremo UI'daki ürün sayfanızın **Provisioning Certificates** bölümündeki sertifika. |
| `device_key.pem` | **Hayır** (gitignore'lu) | Aynı anda indirilen özel anahtarı. Onu bir şifre gibi saklayın: bu dosyaya sahip olan herkes ürününüzün kimlik bilgileriyle Tiremo Cloud'a bağlanabilir; asla paylaşmayın veya herkese açık bir yere yapıştırmayın. |

İndirilen dosyalar ürününüzün tanımlayıcısıyla adlandırılmış iki dosyadır,
örneğin `myproduct.certificate.pem` ve `myproduct.private.key`. Onları bu
klasöre koyup yeniden adlandırın:

| İndirilen dosya | Yeni adı |
|---|---|
| `<identifier>.certificate.pem` | `device_cert.pem` |
| `<identifier>.private.key` | `device_key.pem` |

Derleme yalnızca bu iki adı arar ve biri eksikse
`Missing device credentials` ile durur. İki ad da gitignore'ludur, bir
`certs/` klasöründeki her `*.key` dosyası da öyle. Commit etmeden önce
yine de `git status` ile kontrol edin: iki dosya da orada asla görünmemeli.

Aynı ürün sayfasında gösterilen **Secret Key** bu eğitimde kullanılmaz.
Tam anlatım için eğitimin kendi `README.md`'sine bakın.
