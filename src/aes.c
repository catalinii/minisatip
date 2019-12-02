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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <net/if.h>
#include <fcntl.h>
#include <ctype.h>
#include "utils.h"
#include "dvb.h"
#include "socketworks.h"
#include "minisatip.h"
#include "pmt.h"
#include "adapter.h"
#include "tables.h"
#include "openssl/aes.h"
#include <openssl/evp.h>
#include <openssl/conf.h>
#include <openssl/err.h>

#define DEFAULT_LOG LOG_PMT

void dvbaes_create_key(SCW *cw)
{
	cw->key = malloc1(sizeof(AES_KEY));
}

void dvbaes_delete_key(SCW *cw)
{
	free1(cw->key);
}

int dvbaes_batch_size() // make sure the number is divisible by 7
{
	return 128; // process 64 packets at a time
}

void dvbaes_set_cw(SCW *cw, SPMT *pmt)
{
	AES_set_decrypt_key(cw->cw, 128, (AES_KEY *)cw->key);
}

void dvbaes_decrypt_stream(SCW *cw, SPMT_batch *batch, int max_len)
{
	int i, j, len;
	for (i = 0; i < max_len && batch[i].data; i++)
	{
		len = (batch[i].len / 16) * 16;
		for (j = 0; j < len; j++)
			AES_ecb_encrypt(batch[i].data + j, batch[i].data + j,
							(AES_KEY *)cw->key, AES_DECRYPT);
	}
}

SCW_op aes_op =
	{.algo = CA_ALGO_AES128_ECB,
	 .create_cw = (Create_CW)dvbaes_create_key,
	 .delete_cw = (Delete_CW)dvbaes_delete_key,
	 .batch_size = dvbaes_batch_size,
	 .set_cw = (Set_CW)dvbaes_set_cw,
	 .stop_cw = NULL,
	 .decrypt_stream = (Decrypt_Stream)dvbaes_decrypt_stream};

void dvbaes_cbc_create_key(SCW *cw)
{
	if (!(cw->key = (void *)EVP_CIPHER_CTX_new()))
		LOG("EVP_CIPHER_CTX_new failed");
}

void dvbaes_cbc_delete_key(SCW *cw)
{
	if (cw->key)
	{
		EVP_CIPHER_CTX_free((EVP_CIPHER_CTX *)cw->key);
		cw->key = NULL;
	}
}

void dvbaes_cbc_set_cw(SCW *cw, SPMT *pmt)
{
}
void dvbaes_cbc_stop_cw(SCW *cw, SPMT *pmt)
{
}

int decrypt(void *context, unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
			unsigned char *iv, unsigned char *plaintext)
{
	EVP_CIPHER_CTX *ctx = (EVP_CIPHER_CTX *)context;
	int plaintext_len = 0;
	if (!ctx)
		LOG_AND_RETURN(0, "AES CBC context is null");

	if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv))
		LOG_AND_RETURN(0, "EVP_DecryptInit_ex failed");

	if (1 != EVP_DecryptUpdate(ctx, plaintext, &plaintext_len, ciphertext, ciphertext_len))
		LOG_AND_RETURN(0, "EVP_DecryptUpdate failed");

	return plaintext_len;
}

void dvbaes_cbc_decrypt_stream(SCW *cw, SPMT_batch *batch, int max_len)
{
	int i, len;
	uint8_t decryptedtext[300]; //, ciphertext[300];
	for (i = 0; i < max_len && batch[i].data; i++)
	{

		//memset(decryptedtext, 0, sizeof(decryptedtext));
		//		memset(ciphertext, 0, sizeof(ciphertext));
		//		memcpy(ciphertext, batch[i].data, batch[i].len);
		len = (batch[i].len / 16 + 1) * 16;
		len = decrypt(cw->key, /*ciphertext*/ batch[i].data, len, cw->cw, cw->iv,
					  decryptedtext);
		if (len > batch[i].len)
			len = batch[i].len;

		if (len > 0)
			memcpy(batch[i].data, decryptedtext, len);
	}
}

SCW_op aes_cbc_op =
	{.algo = CA_ALGO_AES128_CBC,
	 .create_cw = (Create_CW)dvbaes_cbc_create_key,
	 .delete_cw = (Delete_CW)dvbaes_cbc_delete_key,
	 .batch_size = dvbaes_batch_size,
	 .set_cw = (Set_CW)dvbaes_cbc_set_cw,
	 .stop_cw = (Set_CW)dvbaes_cbc_stop_cw,
	 .decrypt_stream = (Decrypt_Stream)dvbaes_cbc_decrypt_stream};

void init_algo_aes()
{
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	OPENSSL_config(NULL);
#endif

	register_algo(&aes_op);
	register_algo(&aes_cbc_op);
}
