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
#include <dvbcsa/dvbcsa.h>

extern struct struct_opts opts;

void *dvbcsa_create_key()
{
	return (void *) dvbcsa_bs_key_alloc();
}

void dvbcsa_delete_key(void *key)
{
	dvbcsa_key_free(key);
}

int dvbcsa_batch_size() // make sure the number is divisible by 7
{
	int batchSize = dvbcsa_bs_batch_size();
//	batchSize = (batchSize / 7) * 7;
	return batchSize;
}

void dvbcsa_set_cw(unsigned char *cw, void *key)
{
	dvbcsa_bs_key_set(cw, key);
}

void dvbcsa_decrypt_stream(void *key, dvbapi_batch *batch, int max_len)
{
	dvbcsa_bs_decrypt((const struct dvbcsa_bs_key_s *) key,
			(const struct dvbcsa_bs_batch_s *) batch, max_len);
}

dvbapi_op csa_op =
{ .algo = CA_ALGO_DVBCSA, .mode = 0, .create_cwkey = dvbcsa_create_key,
		.delete_cwkey = dvbcsa_delete_key, .batch_size = dvbcsa_batch_size,
		.set_cw = dvbcsa_set_cw, .decrypt_stream = dvbcsa_decrypt_stream };

