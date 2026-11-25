<p class="sphinxhide" align="right"><a href="../../../../../../Tiremo_Cloud/Feature_Tutorials/03-Fleet_Provisioning_By_Claim/Sw/certs/README.md">English</a> | <a>Türkçe</a></p>

# certs/

> Bu sayfa [İngilizce orijinalinin](../../../../../../Tiremo_Cloud/Feature_Tutorials/03-Fleet_Provisioning_By_Claim/Sw/certs/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

Düzen [Cloud Messaging 03](../../../../../Cloud_Messaging/Feature_Tutorials/03-MQTT_mTLS_To_AWS_IoT/Sw/certs/README.md) ile aynıdır: gitignore'lu çift, Tiremo UI'daki ürün sayfanızın **Provisioning Certificates** bölümünden gelen claim sertifikası ve anahtarıdır.

| Dosya | Commit edilir mi? | Nedir |
|---|---|---|
| `root_ca.pem` | Evet | Amazon Root CA 1 — herkese açık, her Tiremo Cloud cihazı için aynı, `https://www.amazontrust.com/repository/AmazonRootCA1.pem` adresinden. Tiremo Cloud'un sunucu sertifikasını doğrular. |
| `claim_cert.pem.example` / `claim_key.pem.example` | Evet | Biçimi gösteren yer tutucular. |
| `claim_cert.pem` | **Hayır** (gitignore'lu) | Ürün sayfanızdaki **Provisioning Certificates** altındaki claim sertifikası. İlkesi yalnızca filo provizyonuna (`CreateKeysAndCertificate` / `RegisterThing`) izin verir. |
| `claim_key.pem` | **Hayır** (gitignore'lu) | Claim sertifikasının özel anahtarı. Asla paylaşmayın veya herkese açık bir yere yapıştırmayın. |

İndirilen dosyalar ürününüzün tanımlayıcısıyla adlandırılmış iki dosyadır.
Onları bu klasöre koyup yeniden adlandırın:

| İndirilen dosya | Yeni adı |
|---|---|
| `<identifier>.certificate.pem` | `claim_cert.pem` |
| `<identifier>.private.key` | `claim_key.pem` |

Onları Cloud Messaging 03'te zaten `device_cert.pem` / `device_key.pem`
olarak koyduysanız, o iki dosyayı yeni adlarıyla buraya kopyalayın; aynı
sertifika ve anahtardır. Dosyalardan biri eksikse derleme `Missing claim credentials` ile durur.
İki ad da gitignore'ludur, bir `certs/` klasöründeki her `*.key` dosyası da
öyle; yine de commit etmeden önce `git status` ile kontrol edin.

Bu değerlerin Tiremo UI'da nerede olduğu için eğitimin
[Tiremo UI'dan kimlik bilgileri](../../README.md#tiremo-uidan-kimlik-bilgileri)
bölümüne bakın. Platform dökümanı:
[Phase 1](https://docs.tiremo.ai/device-provisioning/first-phase),
[filo provizyonu](https://docs.tiremo.ai/device-provisioning/second-phase),
[TLS SNI](https://docs.tiremo.ai/device-provisioning/tls-sni).

Verilen cihaza özel sertifika ve anahtar bu klasörde başlamaz: Tiremo
Cloud onları MQTT üzerinden döndürür ve firmware doğrudan ESP32'nin
`mqtt_cert` / `mqtt_key` SYSMFG yuvalarına yazar.
