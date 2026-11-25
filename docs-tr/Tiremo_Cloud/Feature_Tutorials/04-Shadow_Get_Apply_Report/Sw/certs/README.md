<p class="sphinxhide" align="right"><a href="../../../../../../Tiremo_Cloud/Feature_Tutorials/04-Shadow_Get_Apply_Report/Sw/certs/README.md">English</a> | <a>Türkçe</a></p>

# certs/

> Bu sayfa [İngilizce orijinalinin](../../../../../../Tiremo_Cloud/Feature_Tutorials/04-Shadow_Get_Apply_Report/Sw/certs/README.md)
> Türkçe çevirisidir. Kod ve dosya yolları İngilizce kalır; içerik farklılaşırsa
> İngilizce sürüm esas alınır.

Düzen [eğitim 03](../../../03-Fleet_Provisioning_By_Claim/Sw/certs/README.md) ile aynıdır: gitignore'lu çift, Tiremo UI'daki ürün sayfanızın **Provisioning Certificates** bölümünden gelen claim sertifikası ve anahtarıdır.

| Dosya | Commit edilir mi? | Nedir |
|---|---|---|
| `root_ca.pem` | Evet | Amazon Root CA 1 — herkese açık, her Tiremo Cloud cihazı için aynı, `https://www.amazontrust.com/repository/AmazonRootCA1.pem` adresinden. Tiremo Cloud'un sunucu sertifikasını doğrular. |
| `claim_cert.pem.example` / `claim_key.pem.example` | Evet | Biçimi gösteren yer tutucular. |
| `claim_cert.pem` | **Hayır** (gitignore'lu) | Ürün sayfanızdaki **Provisioning Certificates** altındaki claim sertifikası. İlkesi yalnızca filo provizyonuna (`CreateKeysAndCertificate` / `RegisterThing`) izin verir. |
| `claim_key.pem` | **Hayır** (gitignore'lu) | Claim sertifikasının özel anahtarı. Asla paylaşmayın veya herkese açık bir yere yapıştırmayın. |

Bu klasörü doldurmanın en kolay yolu `claim_cert.pem` ve `claim_key.pem`
dosyalarını eğitim 03'ün `Sw/certs/` klasöründen kopyalamaktır. Bunun
yerine indirilen dosyalardan başlarsanız, ürününüzün tanımlayıcısıyla
adlandırılmış iki dosya alırsınız. Onları bu klasöre koyup yeniden
adlandırın:

| İndirilen dosya | Yeni adı |
|---|---|
| `<identifier>.certificate.pem` | `claim_cert.pem` |
| `<identifier>.private.key` | `claim_key.pem` |

Dosyalardan biri eksikse derleme `Missing claim credentials` ile durur.
İki ad da gitignore'ludur, bir `certs/` klasöründeki her `*.key` dosyası da
öyle; yine de commit etmeden önce `git status` ile kontrol edin.

[Eğitim 03](../../../03-Fleet_Provisioning_By_Claim/README.md)'ten sonra
verilen kimlik zaten ESP32'nin `mqtt_cert` / `mqtt_key` yuvalarındadır ve
bu firmware onunla bağlanır. Claim PEM'leri yine de gömülü kalır; böylece
fabrikadan yeni çıkmış bir kart (veya USER'a 10 s basılarak sıfırlanmış
bir kart) shadow alışverişinden önce claim alışverişini yeniden yapabilir.

Bu değerlerin Tiremo UI'da nerede olduğu için eğitim 03'ün
[Tiremo UI'dan kimlik bilgileri](../../../03-Fleet_Provisioning_By_Claim/README.md#tiremo-uidan-kimlik-bilgileri)
bölümüne bakın. Platform dökümanı:
[Phase 1](https://docs.tiremo.ai/device-provisioning/first-phase).
