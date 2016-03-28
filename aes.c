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
#include "dvbapi.h"
#include "adapter.h"
#include "tables.h"
#include "dvbapi.h"
#include "openssl/aes.h"

extern struct struct_opts opts;

void *dvbaes_create_key()
{
	return malloc1(sizeof(AES_KEY));
}

void dvbaes_delete_key(void *key)
{
	free1(key);
}

int dvbaes_batch_size() // make sure the number is divisible by 7
{
	return 64; // process 64 packets at a time
}

void dvbaes_set_cw(unsigned char *cw, void *key)
{
	AES_set_decrypt_key(cw, 128, (AES_KEY *) key);
}

void dvbaes_decrypt_stream(void *key, dvbapi_batch *batch, int max_len)
{
	int i, j, len;
	for (i = 0; batch[i].data && i < max_len; i++)
	{
		len = (batch[i].len / 16) * 16;
		for (j = 0; j < len; j++)
			AES_ecb_encrypt(batch[i].data + j, batch[i].data + j,
					(AES_KEY *) key, AES_DECRYPT);

	}
}

dvbapi_op aes_op =
{ .algo = CA_ALGO_AES128, .mode = CA_MODE_ECB,
		.create_cwkey = dvbaes_create_key, .delete_cwkey = dvbaes_delete_key,
		.batch_size = dvbaes_batch_size, .set_cw = dvbaes_set_cw,
		.decrypt_stream = dvbaes_decrypt_stream };

