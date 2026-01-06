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
#include "adapter.h"
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

void dvbaes_create_key(SCW *cw) {
    if (!(cw->key = (void *)EVP_CIPHER_CTX_new()))
        LOG("EVP_CIPHER_CTX_new failed");
}

void dvbaes_delete_key(SCW *cw) {
    if (cw->key) {
        EVP_CIPHER_CTX_free((EVP_CIPHER_CTX *)cw->key);
        cw->key = NULL;
    }
}

void dvbaes_set_cw(SCW *cw, SPMT *pmt) {
    EVP_CIPHER_CTX *ctx = (EVP_CIPHER_CTX *)cw->key;
    if (!ctx)
        return;

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, cw->cw, NULL))
        LOG("EVP_DecryptInit_ex failed for ECB mode");
}

void dvbaes_decrypt_stream(SCW *cw, SPMT_batch *batch, int batch_len) {
    EVP_CIPHER_CTX *ctx = (EVP_CIPHER_CTX *)cw->key;
    int i, j, len, out_len;

    if (!ctx)
        return;

    for (i = 0; i < batch_len; i++) {
        len = (batch[i].len / 16) * 16;
        for (j = 0; j < len; j += 16) {
            if (1 != EVP_DecryptUpdate(ctx, batch[i].data + j, &out_len,
                                       batch[i].data + j, 16))
                LOG("EVP_DecryptUpdate failed");
        }
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
    int i;
    uint32_t len;
    uint8_t decryptedtext[400];
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
        if (new_len > (int)batch[i].len)
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
    register_algo(&aes_op);
    register_algo(&aes_cbc_op);
}
