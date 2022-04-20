/*
   - * Copyright (C) 2014-2020 Catalin Toda <catalinii@yahoo.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 *
 */

#include "openssl/aes.h"
#include "adapter.h"
#include "aes.h"
#include "dvb.h"
#include "minisatip.h"
#include "pmt.h"
#include "socketworks.h"
#include "tables.h"
#include "utils.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_LOG LOG_PMT

void dvbaes_create_key(SCW *cw) { cw->key = malloc1(sizeof(AES_KEY)); }

void dvbaes_delete_key(SCW *cw) { free1(cw->key); }

void dvbaes_set_cw(SCW *cw, SPMT *pmt) {
    AES_set_decrypt_key(cw->cw, 128, (AES_KEY *)cw->key);
}

void dvbaes_decrypt_stream(SCW *cw, SPMT_batch *batch, int batch_len) {
    int i, j, len;
    for (i = 0; i < batch_len; i++) {
        len = (batch[i].len / 16) * 16;
        for (j = 0; j < len; j++)
            AES_ecb_encrypt(batch[i].data + j, batch[i].data + j,
                            (AES_KEY *)cw->key, AES_DECRYPT);
    }
}

SCW_op aes_op = {.algo = CA_ALGO_AES128_ECB,
                 .create_cw = (Create_CW)dvbaes_create_key,
                 .delete_cw = (Delete_CW)dvbaes_delete_key,
                 .set_cw = (Set_CW)dvbaes_set_cw,
                 .stop_cw = NULL,
                 .decrypt_stream = (Decrypt_Stream)dvbaes_decrypt_stream};

void dvbaes_cbc_create_key(SCW *cw) {
    if (!(cw->key = (void *)EVP_CIPHER_CTX_new()))
        LOG("EVP_CIPHER_CTX_new failed");
}

void dvbaes_cbc_delete_key(SCW *cw) {
    if (cw->key) {
        EVP_CIPHER_CTX_free((EVP_CIPHER_CTX *)cw->key);
        cw->key = NULL;
    }
}

void dvbaes_cbc_set_cw(SCW *cw, SPMT *pmt) {}
void dvbaes_cbc_stop_cw(SCW *cw, SPMT *pmt) {}

int decrypt(void *context, unsigned char *ciphertext, int ciphertext_len,
            unsigned char *key, unsigned char *iv, unsigned char *plaintext) {
    EVP_CIPHER_CTX *ctx = (EVP_CIPHER_CTX *)context;
    int plaintext_len = 0;
    if (!ctx)
        LOG_AND_RETURN(0, "AES CBC context is null");

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv))
        LOG_AND_RETURN(0, "EVP_DecryptInit_ex failed");

    if (1 != EVP_DecryptUpdate(ctx, plaintext, &plaintext_len, ciphertext,
                               ciphertext_len))
        LOG_AND_RETURN(0, "EVP_DecryptUpdate failed");

    return plaintext_len;
}

void dvbaes_cbc_decrypt_stream(SCW *cw, SPMT_batch *batch, int batch_len) {
    int i, len;
    uint8_t decryptedtext[300]; //, ciphertext[300];
    for (i = 0; i < batch_len; i++) {

        // memset(decryptedtext, 0, sizeof(decryptedtext));
        //		memset(ciphertext, 0, sizeof(ciphertext));
        //		memcpy(ciphertext, batch[i].data, batch[i].len);
        len = (batch[i].len / 16 + 1) * 16;
        if (len > sizeof(decryptedtext)) {
            LOG("WARNING: requested AES decryption of a large payload with "
                "size %d",
                len);
            len = sizeof(decryptedtext) / 16 * 16;
        }
        int new_len = decrypt(cw->key, /*ciphertext*/ batch[i].data, len,
                              cw->cw, cw->iv, decryptedtext);
        if (new_len > batch[i].len)
            new_len = batch[i].len;

        if (new_len > 0)
            memcpy(batch[i].data, decryptedtext, new_len);
    }
}

SCW_op aes_cbc_op = {.algo = CA_ALGO_AES128_CBC,
                     .create_cw = (Create_CW)dvbaes_cbc_create_key,
                     .delete_cw = (Delete_CW)dvbaes_cbc_delete_key,
                     .set_cw = (Set_CW)dvbaes_cbc_set_cw,
                     .stop_cw = (Set_CW)dvbaes_cbc_stop_cw,
                     .decrypt_stream =
                         (Decrypt_Stream)dvbaes_cbc_decrypt_stream};

void init_algo_aes() {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OPENSSL_config(NULL);
#endif

    register_algo(&aes_op);
    register_algo(&aes_cbc_op);
}

///// AES_XCBC_MAC

#if OPENSSL_VERSION_NUMBER < 0x10100000L
int DH_set0_pqg(DH *dh, BIGNUM *p, BIGNUM *q, BIGNUM *g) {
    /* If the fields p and g in d are NULL, the corresponding input
     * parameters MUST be non-NULL.  q may remain NULL.
     */
    if ((dh->p == NULL && p == NULL) || (dh->g == NULL && g == NULL))
        return 0;

    if (p != NULL) {
        BN_free(dh->p);
        dh->p = p;
    }
    if (q != NULL) {
        BN_free(dh->q);
        dh->q = q;
    }
    if (g != NULL) {
        BN_free(dh->g);
        dh->g = g;
    }

    if (q != NULL) {
        dh->length = BN_num_bits(q);
    }

    return 1;
}

void DH_get0_key(const DH *dh, const BIGNUM **pub_key,
                 const BIGNUM **priv_key) {
    if (pub_key != NULL)
        *pub_key = dh->pub_key;
    if (priv_key != NULL)
        *priv_key = dh->priv_key;
}

void DH_set_flags(DH *dh, int flags) { dh->flags |= flags; }
#endif

int aes_xcbc_mac_init(struct aes_xcbc_mac_ctx *ctx, const uint8_t *key) {
    AES_KEY aes_key;
    int y, x;

    memset(&aes_key, 0, sizeof(aes_key));

    AES_set_encrypt_key(key, 128, &aes_key);

    for (y = 0; y < 3; y++) {
        for (x = 0; x < 16; x++)
            ctx->K[y][x] = y + 1;
        AES_ecb_encrypt(ctx->K[y], ctx->K[y], &aes_key, 1);
    }

    /* setup K1 */
    AES_set_encrypt_key(ctx->K[0], 128, &ctx->key);

    memset(ctx->IV, 0, 16);
    ctx->buflen = 0;

    return 0;
}

int aes_xcbc_mac_process(struct aes_xcbc_mac_ctx *ctx, const uint8_t *in,
                         unsigned int len) {
    while (len) {
        if (ctx->buflen == 16) {
            AES_ecb_encrypt(ctx->IV, ctx->IV, &ctx->key, 1);
            ctx->buflen = 0;
        }
        ctx->IV[ctx->buflen++] ^= *in++;
        --len;
    }

    return 0;
}

int aes_xcbc_mac_done(struct aes_xcbc_mac_ctx *ctx, uint8_t *out) {
    int i;

    if (ctx->buflen == 16) {
        /* K2 */
        for (i = 0; i < 16; i++)
            ctx->IV[i] ^= ctx->K[1][i];
    } else {
        ctx->IV[ctx->buflen] ^= 0x80;
        /* K3 */
        for (i = 0; i < 16; i++)
            ctx->IV[i] ^= ctx->K[2][i];
    }

    AES_ecb_encrypt(ctx->IV, ctx->IV, &ctx->key, 1);
    memcpy(out, ctx->IV, 16);

    return 0;
}

////// END_AES_XCBC_MAC
////// DH_RSA_MISC

int pkcs_1_mgf1(const uint8_t *seed, unsigned long seedlen, uint8_t *mask,
                unsigned long masklen) {
    unsigned long hLen, x;
    uint32_t counter;
    uint8_t *buf;

    /* get hash output size */
    hLen = 20; /* SHA1 */

    /* allocate memory */
    buf = malloc(hLen);
    if (buf == NULL) {
        LOG("error mem");
        return -1;
    }

    /* start counter */
    counter = 0;

    while (masklen > 0) {
        /* handle counter */
        copy32(buf, 0, counter);
        ++counter;

        /* get hash of seed || counter */
        unsigned char buffer[0x18];
        memcpy(buffer, seed, seedlen);
        memcpy(buffer + 0x14, buf, 4);
        SHA1(buffer, 0x18, buf);

        /* store it */
        for (x = 0; x < hLen && masklen > 0; x++, masklen--)
            *mask++ = buf[x];
    }

    free(buf);
    return 0;
}

int pkcs_1_pss_encode(const uint8_t *msghash, unsigned int msghashlen,
                      unsigned long saltlen, unsigned long modulus_bitlen,
                      uint8_t *out, unsigned int outlen) {
    unsigned char *DB, *mask, *salt, *hash;
    unsigned long x, y, hLen, modulus_len;
    int err = -1;
    unsigned char *hashbuf;
    unsigned int hashbuflen;

    hLen = 20; /* SHA1 */
    modulus_len = (modulus_bitlen >> 3) + (modulus_bitlen & 7 ? 1 : 0);

    /* allocate ram for DB/mask/salt/hash of size modulus_len */
    DB = malloc(modulus_len);
    mask = malloc(modulus_len);
    salt = malloc(modulus_len);
    hash = malloc(modulus_len);

    hashbuflen = 8 + msghashlen + saltlen;
    hashbuf = malloc(hashbuflen);

    if (!(DB && mask && salt && hash && hashbuf)) {
        LOG("out of memory");
        goto LBL_ERR;
    }

    /* generate random salt */
    if (saltlen > 0) {
        if (get_random(salt, saltlen) != (long)saltlen) {
            LOG("rnd failed");
            goto LBL_ERR;
        }
    }

    /* M = (eight) 0x00 || msghash || salt, hash = H(M) */
    memset(hashbuf, 0, 8);
    memcpy(hashbuf + 8, msghash, msghashlen);
    memcpy(hashbuf + 8 + msghashlen, salt, saltlen);
    SHA1(hashbuf, hashbuflen, hash);

    /* generate DB = PS || 0x01 || salt, PS == modulus_len - saltlen - hLen - 2
     * zero bytes */
    x = 0;
    memset(DB + x, 0, modulus_len - saltlen - hLen - 2);
    x += modulus_len - saltlen - hLen - 2;
    DB[x++] = 0x01;
    memcpy(DB + x, salt, saltlen);
    x += saltlen;

    err = pkcs_1_mgf1(hash, hLen, mask, modulus_len - hLen - 1);
    if (err)
        goto LBL_ERR;

    /* xor against DB */
    for (y = 0; y < (modulus_len - hLen - 1); y++)
        DB[y] ^= mask[y];

    /* output is DB || hash || 0xBC */
    if (outlen < modulus_len) {
        err = -1;
        LOG("error overflow");
        goto LBL_ERR;
    }

    /* DB len = modulus_len - hLen - 1 */
    y = 0;
    memcpy(out + y, DB, modulus_len - hLen - 1);
    y += modulus_len - hLen - 1;

    /* hash */
    memcpy(out + y, hash, hLen);
    y += hLen;

    /* 0xBC */
    out[y] = 0xBC;

    /* now clear the 8*modulus_len - modulus_bitlen most significant bits */
    out[0] &= 0xFF >> ((modulus_len << 3) - (modulus_bitlen - 1));

    err = 0;
LBL_ERR:
    free(hashbuf);
    free(hash);
    free(salt);
    free(mask);
    free(DB);

    return err;
}

/* DH */

int dh_gen_exp(uint8_t *dest, int dest_len, uint8_t *dh_g, int dh_g_len,
               uint8_t *dh_p, int dh_p_len) {
    DH *dh;
    BIGNUM *p, *g;
    const BIGNUM *priv_key;
    int len;
    unsigned int gap;

    dh = DH_new();

    p = BN_bin2bn(dh_p, dh_p_len, 0);
    g = BN_bin2bn(dh_g, dh_g_len, 0);
    DH_set0_pqg(dh, p, NULL, g);
    DH_set_flags(dh, DH_FLAG_NO_EXP_CONSTTIME);

    DH_generate_key(dh);

    DH_get0_key(dh, NULL, &priv_key);
    len = BN_num_bytes(priv_key);
    if (len > dest_len) {
        LOG("len > dest_len");
        return -1;
    }

    gap = dest_len - len;
    memset(dest, 0, gap);
    BN_bn2bin(priv_key, &dest[gap]);

    DH_free(dh);

    return 0;
}

/* dest = base ^ exp % mod */
int dh_mod_exp(uint8_t *dest, int dest_len, uint8_t *base, int base_len,
               uint8_t *mod, int mod_len, uint8_t *exp, int exp_len) {
    BIGNUM *bn_dest, *bn_base, *bn_exp, *bn_mod;
    BN_CTX *ctx;
    int len;
    unsigned int gap;

    bn_base = BN_bin2bn(base, base_len, NULL);
    bn_exp = BN_bin2bn(exp, exp_len, NULL);
    bn_mod = BN_bin2bn(mod, mod_len, NULL);
    ctx = BN_CTX_new();

    bn_dest = BN_new();
    BN_mod_exp(bn_dest, bn_base, bn_exp, bn_mod, ctx);
    BN_CTX_free(ctx);

    len = BN_num_bytes(bn_dest);
    if (len > dest_len) {
        LOG("len > dest_len");
        return -1;
    }

    gap = dest_len - len;
    memset(dest, 0, gap);
    BN_bn2bin(bn_dest, &dest[gap]);

    BN_free(bn_dest);
    BN_free(bn_mod);
    BN_free(bn_exp);
    BN_free(bn_base);

    return 0;
}

int dh_dhph_signature(uint8_t *out, uint8_t *nonce, uint8_t *dhph, RSA *r) {
    unsigned char dest[302];
    uint8_t hash[20];
    unsigned char dbuf[256];

    dest[0x00] = 0x00; /* version */
    dest[0x01] = 0x00;
    dest[0x02] = 0x08; /* len (bits) */
    dest[0x03] = 0x01; /* version data */

    dest[0x04] = 0x01; /* msg_label */
    dest[0x05] = 0x00;
    dest[0x06] = 0x08; /* len (bits) */
    dest[0x07] = 0x02; /* message data */

    dest[0x08] = 0x02; /* auth_nonce */
    dest[0x09] = 0x01;
    dest[0x0a] = 0x00; /* len (bits) */
    memcpy(&dest[0x0b], nonce, 32);

    dest[0x2b] = 0x04; /* DHPH - DH public key host */
    dest[0x2c] = 0x08;
    dest[0x2d] = 0x00; /* len (bits) */
    memcpy(&dest[0x2e], dhph, 256);

    SHA1(dest, 0x12e, hash);

    if (pkcs_1_pss_encode(hash, 20, 20, 0x800, dbuf, sizeof(dbuf))) {
        LOG("pss encode failed");
        return -1;
    }

    RSA_private_encrypt(sizeof(dbuf), dbuf, out, r, RSA_NO_PADDING);

    return 0;
}

int verify_cb(int ok, X509_STORE_CTX *ctx) {
    if (X509_STORE_CTX_get_error(ctx) == X509_V_ERR_CERT_NOT_YET_VALID) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        if (t->tm_year < 2015) {
            LOG("seems our rtc is wrong - ignore!");
            return 1;
        }
    }

    if (X509_STORE_CTX_get_error(ctx) == X509_V_ERR_CERT_HAS_EXPIRED)
        return 1;
    return 0;
}

RSA *rsa_privatekey_open(const char *filename) {
    FILE *fp;
    RSA *r = NULL;

    fp = fopen(filename, "r");
    if (!fp) {
        LOG("can not open %s", filename);
        return NULL;
    }

    PEM_read_RSAPrivateKey(fp, &r, NULL, NULL);
    if (!r) {
        LOG("read error");
    }

    fclose(fp);

    return r;
}

X509 *certificate_open(const char *filename) {
    FILE *fp;
    X509 *cert;

    fp = fopen(filename, "r");
    if (!fp) {
        LOG("can not open %s", filename);
        return NULL;
    }

    cert = PEM_read_X509(fp, NULL, NULL, NULL);
    if (!cert) {
        LOG("can not read cert");
    }

    fclose(fp);

    return cert;
}

int certificate_validate(struct cert_ctx *ctx, X509 *cert) {
    X509_STORE_CTX *store_ctx;
    int ret;

    store_ctx = X509_STORE_CTX_new();

    X509_STORE_CTX_init(store_ctx, ctx->store, cert, NULL);
    X509_STORE_CTX_set_verify_cb(store_ctx, verify_cb);
    X509_STORE_CTX_set_flags(store_ctx, X509_V_FLAG_IGNORE_CRITICAL);

    ret = X509_verify_cert(store_ctx);

    if (ret != 1) {
        LOG("%s",
            X509_verify_cert_error_string(X509_STORE_CTX_get_error(store_ctx)));
    }

    X509_STORE_CTX_free(store_ctx);

    if (ret == 1)
        return 1;
    else
        return 0;
}

X509 *certificate_load_and_check(struct cert_ctx *ctx, const char *filename) {
    X509 *cert;

    if (!ctx->store) {
        /* we assume this is the first certificate added - so its root-ca */
        ctx->store = X509_STORE_new();
        if (!ctx->store) {
            LOG("can not create cert_store");
            exit(-1);
        }

        if (X509_STORE_load_locations(ctx->store, filename, NULL) != 1) {
            LOG("load of first certificate (root_ca) failed");
            exit(-1);
        }

        return NULL;
    }

    cert = certificate_open(filename);
    if (!cert) {
        LOG("can not open certificate %s", filename);
        return NULL;
    }

    if (!certificate_validate(ctx, cert)) {
        LOG("can not vaildate certificate");
        X509_free(cert);
        return NULL;
    }

    /* push into store - create a chain */
    if (X509_STORE_load_locations(ctx->store, filename, NULL) != 1) {
        LOG("load of certificate failed");
        X509_free(cert);
        return NULL;
    }

    return cert;
}

X509 *certificate_import_and_check(struct cert_ctx *ctx, const uint8_t *data,
                                   int len) {
    X509 *cert;

    cert = d2i_X509(NULL, &data, len);
    if (!cert) {
        LOG("can not read certificate");
        return NULL;
    }

    if (!certificate_validate(ctx, cert)) {
        LOG("can not vaildate certificate");
        X509_free(cert);
        return NULL;
    }

    X509_STORE_add_cert(ctx->store, cert);

    return cert;
}
