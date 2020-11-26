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
#include "adapter.h"
#include "tables.h"
#include "pmt.h"
#include <dvbcsa/dvbcsa.h>

#define DEFAULT_LOG LOG_DVBCA

void dvbcsa_create_key(SCW *cw)
{
	cw->key = dvbcsa_bs_key_alloc();
}

void dvbcsa_delete_key(SCW *cw)
{
	dvbcsa_key_free(cw->key);
}

int dvbcsa_batch_size()
{
	int batchSize = dvbcsa_bs_batch_size();
	return batchSize;
}

void dvbcsa_set_cw(SCW *cw, SPMT *pmt)
{
	dvbcsa_bs_key_set((unsigned char *)cw->cw, cw->key);
}

void dvbcsa_decrypt_stream(SCW *cw, SPMT_batch *batch, int max_len)
{
	if (cw->key)
		dvbcsa_bs_decrypt((const struct dvbcsa_bs_key_s *)cw->key,
						  (const struct dvbcsa_bs_batch_s *)batch, max_len);
	else
		LOG("%s: cw->key is null", __FUNCTION__);
}

SCW_op csa_op = {.algo = CA_ALGO_DVBCSA,
				 .create_cw = (Create_CW)dvbcsa_create_key,
				 .delete_cw = (Delete_CW)dvbcsa_delete_key,
				 .batch_size = dvbcsa_batch_size,
				 .set_cw = (Set_CW)dvbcsa_set_cw,
				 .stop_cw = NULL,
				 .decrypt_stream = (Decrypt_Stream)dvbcsa_decrypt_stream};

void init_algo_csa()
{
	register_algo(&csa_op);
}
