<p class="sphinxhide" align="right"><a href="../../../../../../Tiremo_Cloud/Design_Tutorials/01-Sensor_Telemetry_And_Shadow_Control/Sw/certs/README.md">English</a> | <a>Türkçe</a></p>

# certs/

> Bu sayfa [İngilizce orijinalinin](../../../../../../Tiremo_Cloud/Design_Tutorials/01-Sensor_Telemetry_And_Shadow_Control/Sw/certs/README.md)
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

Bu değerlerin Tiremo UI'da nerede olduğu için eğitimin kendi `README.md`'sine ve
[eğitim 03](../../../../Feature_Tutorials/03-Fleet_Provisioning_By_Claim/#tiremo-uidan-kimlik-bilgileri)'e
bakın (platform:
[Phase 1](https://docs.tiremo.ai/device-provisioning/first-phase)).

Provizyon sırasında kart **kendi** sertifikasını alır ve ESP32'nin
`mqtt_cert` / `mqtt_key` yuvalarına kaydeder. Claim PEM'leri gömülü
kalır; böylece fabrikadan yeni çıkmış bir kart (veya USER'a 10 s basılarak
sıfırlanmış bir kart) yeniden provizyon yapabilir.
