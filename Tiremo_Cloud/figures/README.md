# Tiremo UI figures

Screenshots for [Getting credentials from the Tiremo UI](../Feature_Tutorials/03-Fleet_Provisioning_By_Claim/#getting-credentials-from-the-tiremo-ui).

| File | What it shows |
|---|---|
| `tiremo_ui_products_menu.png` | Main menu — open **Products** |
| `tiremo_ui_select_product.png` | Products list — open the product to provision (skip if already on that product) |
| `tiremo_ui_product_credentials.png` | Product page — Identifier, Secret Key, and Provisioning Certificates |

Those values go into gitignored `certs/claim_cert.pem`, `certs/claim_key.pem`,
and `aws_iot_config.h` (`TIREMO_AWS_PRODUCT_IDENTIFIER`,
`TIREMO_AWS_PRODUCT_SECRET`). Never commit the real secret or claim key.

Platform docs: [Phase 1](https://docs.tiremo.ai/device-provisioning/first-phase)
(bootstrap certificate, identifier, secret).
