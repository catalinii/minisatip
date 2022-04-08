#ifndef AES_H
#define AES_H
#include <openssl/aes.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

struct cert_ctx {
    X509_STORE *store;

    /* Host */
    X509 *cust_cert;
    X509 *device_cert;

    /* Module */
    X509 *ci_cust_cert;
    X509 *ci_device_cert;
};

struct aes_xcbc_mac_ctx {
    uint8_t K[3][16];
    uint8_t IV[16];
    AES_KEY key;
    int buflen;
};

int aes_xcbc_mac_init(struct aes_xcbc_mac_ctx *ctx, const uint8_t *key);
int aes_xcbc_mac_process(struct aes_xcbc_mac_ctx *ctx, const uint8_t *in,
                         unsigned int len);
int aes_xcbc_mac_done(struct aes_xcbc_mac_ctx *ctx, uint8_t *out);
int dh_gen_exp(uint8_t *dest, int dest_len, uint8_t *dh_g, int dh_g_len,
               uint8_t *dh_p, int dh_p_len);
int dh_mod_exp(uint8_t *dest, int dest_len, uint8_t *base, int base_len,
               uint8_t *mod, int mod_len, uint8_t *exp, int exp_len);
int dh_dhph_signature(uint8_t *out, uint8_t *nonce, uint8_t *dhph, RSA *r);
/*

int pkcs_1_mgf1(const uint8_t *seed, unsigned long seedlen, uint8_t *mask,
                unsigned long masklen);

int pkcs_1_pss_encode(const uint8_t *msghash, unsigned int msghashlen,
                      unsigned long saltlen, unsigned long modulus_bitlen,
                      uint8_t *out, unsigned int outlen);
int verify_cb(int ok, X509_STORE_CTX *ctx);
*/
RSA *rsa_privatekey_open(const char *filename);
X509 *certificate_load_and_check(struct cert_ctx *ctx, const char *filename);
X509 *certificate_import_and_check(struct cert_ctx *ctx, const uint8_t *data,
                                   int len);

#endif
