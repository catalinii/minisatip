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

extern struct struct_opts opts;
#define ASSEMBLE_TIMEOUT 1000

SPMT *pmts[MAX_PMT];
SMutex pmts_mutex;

_Spmt_op ops[10];

SCW *cws[MAX_CW];
SMutex cws_mutex;

SFilter *filters[MAX_FILTERS];
SMutex filters_mutex;
int nfilters;

static inline SCW *get_cw(int id)
{
	if (id < 0 || id >= MAX_CW || !cws[id] || !cws[id]->enabled)
		return NULL;
	return cws[id];
}

static inline SPMT_op *get_op(int id)
{
	if (ops[id].enabled)
		return ops[id].op;
	return NULL;
}
int register_algo(SPMT_op *o)
{
	int i;
	for (i = 0; i < sizeof(ops); i++)
		if (!ops[i].enabled)
		{
			ops[i].op = o;
			ops[i].enabled = 1;
			return 0;
		}
	return 1;
}

#ifndef DISABLE_DVBCSA
void init_algo_csa();
#endif
#ifndef DISABLE_DVBAES
void init_algo_aes();
#endif

typedef void (*type_algo_init_func)();

type_algo_init_func algo_init_func[] =
	{
#ifndef DISABLE_DVBCSA
		&init_algo_csa,
#endif
#ifndef DISABLE_DVBAES
		&init_algo_aes,
#endif
		NULL};

void init_algo()
{
	int i;
	for (i = 0; i < sizeof(algo_init_func); i++)
		if (algo_init_func[i])
			algo_init_func[i]();
}

#define POINTER_TYPE_ECM ((void *)-TYPE_ECM)
#define POINTER_1 ((void *)1)

int add_filter(int aid, int pid, void *callback, void *opaque, int flags)
{
	uint8_t data[DMX_FILTER_SIZE], mask[DMX_FILTER_SIZE];
	memset(data, 0, sizeof(data));
	memset(mask, 0, sizeof(mask));
	add_filter_mask(aid, pid, callback, opaque, data, mask, flags);
}
int add_filter_mask(int aid, int pid, void *callback, void *opaque, uint8_t *data, uint8_t *mask, int flags)
{
	SFilter *f;
	int i, fid = 0;
	fid = add_new_lock((void **)filters, MAX_FILTERS, sizeof(Filter), &filters_mutex);
	if (fid == -1)
		LOG_AND_RETURN(-1, "%s failed", __FUNCTION__);
	f = filters[fid];
	f->id = fid;
	f->opaque = opaque;
	f->pid = pid;
	f->callback = callback;
	f->flags = flags;
	f->next_filter = -1;
	memcpy(f->data, data, sizeof(f->data));
	memcpy(f->mask, mask, sizeof(f->mask));
	if (i >= nfilters)
		nfilters = i + 1;
	for (i = 0; i < nfilters; i++)
		if (i != fid && filters[i] && filters[i]->enabled && filters[i]->pid == pid && filters[i]->next_filter == -1)
		{
			filters[i]->next_filter = fid;
			f->master_filter = filters[i]->master_filter;
		}
	mutex_unlock(&f->mutex);
}

int reset_master_filter(int adapter, int pid, it id)
{
	int nf = 0;
	for (i = 0; i < nfilters; i++)
		if ((filters[i] && filters[i]->enabled && filters[i]->adapter == adapter && filters[i]->pid == pid))
		{
			filters[i]->master_filter = id;
			nf++;
		}
	return nf;
}

int del_filter(int id)
{
	SFilter *f;
	LOG("deleting filter %d", id);
	if (id < 0 || id >= MAX_FILTERS || !filters[id] || !filters[id]->enabled)
		return 0;

	f = filters[id];
	mutex_lock(&f->mutex);
	if (!f->enabled)
	{
		mutex_unlock(&f->mutex);
		return 0;
	}

	if (id == f->master_filter)
	{
		int master_filter = f->next_filter;
		SFilter *m = get_filter(master_filter);
		if (!m) // double check there is no other filter
		{
			for (i = 0; i < nfilters; i++)
				if ((filters[i] && filters[i]->enabled && filters[i]->adapter == f->adapter && filters[i]->pid == pid))
				{
					m = filters[i];
					LOGL(3, "warning: filter %d was also found for pid %d", m->id, pid);
					break;
				}
		}
		if (m) // reset master_filter for all filters
			reset_master_filter(f->adapter, pid, m->id);
		else
		{
			SPid *p = find_pid(f->adapter, f->pid);
			if (p->sid[0] == -1 && filters[i]->flags == FILTER_ADD_PID)
				mark_pid_del(ad->id, f->pid);
		}
	}
	else
	{
		for (i = 0; i < nfilters; i++)
			if (filters[i] && filters[i]->enabled && filters[i]->adapter == f->adapter && filters[i]->pid == pid && filters[i]->next_filter == f->id)
			{
				filters[i]->next_filter = f->next_filter;
				break;
			}
	}

	mutex_destroy(&f->mutex);
	LOG("deleted filter %d", id);
	return 0;
}
int get_pid_filter(int aid, int pid)
{
	int i;
	for (i = 0; i < nfilters; i++)
		if (filters[i] && filters[i]->enabled && filters[i]->adapter == aid && filters[i]->pid == pid)

			return filters[i]->master_filter;
	return -1;
}

int change_filter_mask(int id, uint8_t *data, uint8_t *mask)
{
	SFilter *f = get_filter(id);
	if (f)
	{
		memcpy(f->data, data, sizeof(f->data));
		memcpy(f->mask, mask, sizeof(f->mask));
	}
	else
		LOGL(3, "Filter %d not found", id);
	return f ? 0 : 1;
}

void process_filters(SFilter *f, unsigned char *b)
{
}
void process_filters(adapter *ad, unsigned char *b, SPid *p)
{
	int pid = PID_FROM_TS(b);
	SFilter *f;
	int filter = p->filter;
	f = get_filter(filter);
	if (!f || f->master_filter != filter)
	{
		filter = get_pid_filter(ad->id, pid);
		p->filter = filter;
		f = get_filter(filter);
	}
	while (f)
	{
		process_filter(f, b);
		f = get_filter(f->next_filter);
	}
}

void update_cw(SPMT *pmt)
{
	SPMT *master = get_pmt(pmt->master_pmt);
	SCW *cw = NULL;
	SPMT_op *op = NULL;
	int i = 0;
	if (!master)
	{
		LOGL(3, "Master PMT %d does not exist", pmt->master_pmt);
		return;
	}
	if (!pmt->invalidated && !master->invalidated)
	{
		LOGL(3, "PMT %d (master %d) not invalidated, ignoring", pmt->id, master->id);
		return;
	}
	pmt->cw = NULL;
	pmt->op = NULL;
	for (i = 0; i < MAX_CW; i++)
		if (cws[i] && cws[i]->enabled && (pmt->parity == cws[i]->parity) && (cws[i]->pmt == pmt->id || cws[i]->pmt == master->id))
		{
			if (!cw)
			{
				cw = cws[i];
				continue;
			}
		}
	if (cw)
		op = get_op(cw->op_id);
	if (cw && op)
	{
		mutex_lock(&pmt->mutex);
		pmt->cw = cw;
		pmt->op = op;
		pmt->invalidated = 0;
		mutex_unlock(&pmt->mutex);

		mutex_lock(&master->mutex);
		master->cw = cw;
		master->op = op;
		master->invalidated = 0;
		master->parity = pmt->parity;
		mutex_unlock(&master->mutex);
	}
}

int send_cw(int pmt_id, int type, int parity, uint8_t *cw)
{
	LOG("got CW for PMT %d, type %d, parity %d: %02X %02X %02X %02X %02X %02X %02X %02X", pmt_id, type, parity,
		cw[0], cw[1], cw[2], cw[3], cw[4], cw[5], cw[6], cw[7]);
	return 0;
}

int decrypt_batch(SPMT *pmt)
{
	int oldb1 = -1, oldb2 = -1;
	unsigned char *b = NULL, *oldb = NULL;
	int bl, i, pid = 0;

	mutex_lock(&pmt->mutex);
	if (!pmt->cw || !pmt->op)
	{
		LOG("No CW found or OP found for pmt %d pid %d", pmt->id, pmt->pid);
		mutex_unlock(&pmt->mutex);
		return 1;
	}
	b = pmt->batch[0].data - 4;
	pid = (b[1] & 0x1F) * 256 + b[2];
	pmt->batch[pmt->blen].data = NULL;
	pmt->batch[pmt->blen].len = 0;
	pmt->op->decrypt_stream(pmt->cw->key, pmt->batch, 184);
	LOGL(6,
		 "tables: decrypted key %d parity %d at len %d, channel_id %d (pid %d) %p",
		 pmt->id, pmt->parity, pmt->blen, pmt->sid, pid, pmt->batch[0].data); //0x99
	pmt->blen = 0;
	memset(pmt->batch, 0, sizeof(int *) * 128);
	mutex_unlock(&pmt->mutex);
	return 0;
}

int pmt_decrypt_stream(adapter *ad)
{
	struct iovec *iov;
	SPMT *pmt = NULL;
	int adapt_len;
	int len, pmt_id;
	int batchSize = 0;
	// max batch
	int i = 0, j = 0;
	unsigned char *b;
	SPid *p;
	int pid;
	int cp;
	int rlen = ad->rlen;

	for (i = 0; i < rlen; i += 188)
	{
		b = ad->buf + i;
		pid = (b[1] & 0x1F) * 256 + b[2];
		if (b[3] & 0x80)
		{
			p = find_pid(ad->id, pid);
			if (p->pmt > 0 && p->pmt < MAX_PMT && pmts[p->pmt])
				pmt = pmts[p->pmt];
			else
				pmt = NULL;
			if (!pmt)
			{
				continue; // cannot decrypt
			}
			if (!pmt->op || !pmt->cw || !pmt->cw->enabled || pmt->cw->pmt != pmt->master_pmt)
			{
				update_cw(pmt);
			}
			if (!pmt->op || !pmt->cw)
			{
				LOG("CW %x not found (%d) or OP not found %x", pmt->op, pmt->cw_id, pmt->cw);
				continue;
			}

			cp = ((b[3] & 0x40) > 0);
			if (pmt->parity == -1)
				pmt->parity = cp;

			if (!batchSize)
				batchSize = pmt->op->batch_size();

			if ((pmt->parity != cp) || (pmt->blen >= batchSize)) // partiy change or batch buffer full
			{
				int old_parity = pmt->parity;
				decrypt_batch(pmt);
				if (old_parity != cp)
				{
					int64_t ctime = getTick();
					LOGL(2,
						 "Parity change for key %d, new active parity %d pid %d [%02X %02X %02X %02X], last_parity_change %jd",
						 pmt->id, cp, pid, b[0], b[1], b[2], b[3],
						 pmt->last_parity_change);
					pmt->last_parity_change = ctime;
					pmt->parity = cp;
					pmt->invalidated = 1;
					pmt->cw->enabled = 0;
					pmt->op = NULL;
					update_cw(pmt);
				}
			}

			if (b[3] & 0x20)
			{
				adapt_len = (b[4] < 183) ? b[4] + 5 : 188;
				LOGL(5, "Adaptation for pid %d, specified len %d, final len %d",
					 pid, b[4], adapt_len);
			}
			else
				adapt_len = 4;
			if (adapt_len < 188)
			{
				pmt->batch[pmt->blen].data = b + adapt_len;
				pmt->batch[pmt->blen++].len = 188 - adapt_len;
			}
			b[3] &= 0x3F; // clear the encrypted flags
		}
	}

	for (i = 0; i < MAX_PMT; i++) // decrypt everything that's left
		if (pmts[i] && pmts[i]->enabled && (pmts[i]->blen > 0) && (pmts[i]->adapter == ad->id))
			decrypt_batch(pmts[i]);
}

int pmt_process_stream(adapter *ad)
{
	SPid *p;
	int i, pid;
	uint8_t *b;

	int rlen = ad->rlen;

	for (i = 0; i < rlen; i += DVB_FRAME)
	{
		b = ad->buf + i;
		pid = PID_FROM_TS(b);
		p = find_pid(ad->id, pid);
		if (p && (p->type & TYPE_FILTER))
		{
			process_filters(ad, b, p);
		}
	}
#ifndef DISABLE_TABLES

	if (ad->ca_mask == 0) // no CA enabled on this adapter
		return 0;

	pmt_decrypt_stream(ad);
	run_ca_action(CA_TS, ad, &rlen);
	if (ad->ca_mask && opts.drop_encrypted)
	{
		for (i = 0; i < rlen; i += DVB_FRAME)
		{
			b = ad->buf + i;
			pid = PID_FROM_TS(b);
			p = find_pid(ad->id, pid);
			ad->dec_err++;
			if (p)
				p->dec_err++;

			if ((b[3] & 0x80) == 0x80)
			{
				b[1] |= 0x1F;
				b[2] |= 0xFF;
			}
		}
	}

#endif

	return 0;
}

int pmt_add(int i, int adapter, int sid, int pmt_pid)
{

	SPMT *pmt;
	if (i == -1)
		i = add_new_lock((void **)pmts, MAX_PMT, sizeof(SPMT), &pmts_mutex);
	else
	{
		if (pmts[i])
			mutex_lock(&pmts[i]->mutex);
		else
		{
			pmts[i] = malloc1(sizeof(SPMT));
			if (!pmts[i])
				LOG_AND_RETURN(-1, "Could not allocate memory for the pmt %d", i);
			mutex_init(&pmts[i]->mutex);
			mutex_lock(&pmts[i]->mutex);
		}
	}
	if (i == -1 || !pmts[i])
	{
		LOG_AND_RETURN(-1, "PMT buffer is full, could not add new pmts");
	}

	pmt = pmts[i];

	pmt->parity = -1;
	pmt->sid = sid;
	pmt->pid = pmt_pid;
	pmt->adapter = adapter;
	pmt->master_pmt = i;
	pmt->id = i;
	pmt->blen = 0;
	pmt->enabled = 1;
	pmt->ver = -1;
	pmt->invalidated = 1;
	mutex_unlock(&pmt->mutex);
	LOG("returning new key %d for adapter %d, pmt pid %d sid %04X", i, adapter,
		pmt_pid, sid);

	return i;
}

int pmt_del(int id)
{
	int aid, j;
	SPMT *pmt;
	int master_pmt;
	pmt = get_pmt(id);
	if (!pmt)
		return 0;

	mutex_lock(&pmt->mutex);
	if (!pmt->enabled)
	{
		mutex_unlock(&pmt->mutex);
		return 0;
	}
	master_pmt = pmt->master_pmt;
	aid = pmt->adapter;
	pmt->enabled = 0;

	pmt->sid = 0;
	pmt->pid = 0;
	pmt->adapter = -1;
	mutex_destroy(&pmt->mutex);
	if (master_pmt != id) // delete all linked pmts
	{
		int i;
		for (i = 0; i < MAX_PMT; i++)
			if (pmts[i] && pmts[i]->enabled && (pmts[i]->id == master_pmt || pmts[i]->master_pmt == master_pmt))
				pmt_del(i);
	}
	return 0;
}

int pmt_enabled_channels(int id)
{
}

static uint32_t crc_tab[256] =
	{0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
	 0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
	 0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
	 0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
	 0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
	 0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
	 0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
	 0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
	 0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
	 0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
	 0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
	 0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
	 0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
	 0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
	 0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
	 0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	 0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
	 0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
	 0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
	 0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
	 0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
	 0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
	 0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
	 0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
	 0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
	 0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
	 0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
	 0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
	 0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
	 0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
	 0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
	 0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
	 0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
	 0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
	 0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
	 0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	 0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
	 0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
	 0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
	 0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
	 0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
	 0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
	 0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4};

uint32_t crc_32(const uint8_t *data, int datalen)
{
	uint32_t crc = 0xFFFFFFFF;
	if (datalen < 0)
		return crc;
	while (datalen--)
		crc = (crc << 8) ^ crc_tab[((crc >> 24) ^ *data++) & 0xff];

	return crc;
}

void mark_pid_null(uint8_t *b)
{
	b[1] |= 0x1F;
	b[2] |= 0xFF;
}

void clean_psi(adapter *ad, uint8_t *b)
{
	int pid = PID_FROM_TS(b);
	int pmt_len;
	int64_t clean_key = TABLES_ITEM + ((1 + ad->id) << 24) + pid;
	int64_t item_key = TABLES_ITEM + (ad->id << 16) + pid;
	uint8_t *clean, *pmt;
	SPid *p;
	int pi_len, i, j, es_len, desc_len;
	int8_t *cc, _cc;

	p = find_pid(ad->id, pid);
	if (!p || p->sid[0] == -1) // no need to fix this PMT as it not requested by any stream
		return;

#ifndef DISABLE_DVBAPI
	if (!get_pmt(p->pmt)) // no key associated with PMT - most likely the channel is clear
		return;
#else
	return;
#endif

	if (!(p->type & CLEAN_PMT))
	{
		//		mark_pid_null(b);
		return;
	}
	clean = getItem(clean_key);
	if (!(pmt = getItem(item_key)))
	{
		pmt_len = ((b[6] & 0xF) << 8) + b[7];
		if ((b[1] & 0x40) && (pmt_len < 183))
			pmt = b + 5;
	}

	if (!clean && !pmt)
	{
		mark_pid_null(b);
		return;
	}

	if (!clean && pmt) // populate clean psi
	{
		uint8_t *n, *o;
		int nlen = 0;
		uint32_t crc;
		setItem(clean_key, pmt, 1, 0);
		if (getItemSize(clean_key) < 1500)
			setItemSize(clean_key, 1500);
		clean = getItem(clean_key);
		if (!clean)
		{
			mark_pid_null(b);
			return;
		}
		setItemTimeout(clean_key, ASSEMBLE_TIMEOUT);
		memset(clean, -1, getItemSize(clean_key));
		setItem(clean_key, pmt, 12, 0);
		pi_len = ((pmt[10] & 0xF) << 8) + pmt[11];
		pmt_len = ((pmt[1] & 0xF) << 8) + pmt[2];
		LOG("Cleaning PMT for pid %d, pmt_len %d, pi_len %d, pmt %p", pid,
			pmt_len, pi_len, pmt);
		n = clean;
		o = pmt + pi_len + 12;
		nlen = 12;
		n[10] &= 0xF0; // pi_len => 0
		n[11] &= 0x0;

		for (i = 0; i < pmt_len - pi_len - 17; i += (es_len) + 5) // reading streams
		{
			uint8_t *t = o + i;
			int init_len = nlen + 5;
			es_len = (o[i + 3] & 0xF) * 256 + o[i + 4];
			LOGL(4, "es: copy 5 bytes from %d -> %d : %02X %02X %02X %02X %02X",
				 i, nlen, t[0], t[1], t[2], t[3], t[4]);
			memcpy(n + nlen, o + i, 5);
			nlen += 5;
			for (j = 0; j < es_len; j += desc_len) // reading program info
			{
				desc_len = o[i + 5 + j + 1] + 2;
				if (o[i + 5 + j] != 9)
				{
					t = o + i + 5 + j;
					LOGL(4, "desc copy %d bytes from %d -> %d : %02X %02X %02X",
						 desc_len, i + 5 + j, nlen, t[0], t[1], t[2]);
					memcpy(n + nlen, o + i + 5 + j, desc_len);
					nlen += desc_len;
				}
			}
			int nes_len = nlen - init_len;
			LOGL(4, "clean_psi: setting the new es_len %d at position %d",
				 nes_len, init_len - 2);
			n[init_len - 2] = (n[init_len - 2] & 0xF0) | ((nes_len >> 8) & 0xF);
			n[init_len - 1] = (nes_len)&0xFF;
		}
		nlen += 4 - 3;
		LOGL(4, "new len is %d, old len is %d", nlen, pmt_len);
		n[1] &= 0xF0;
		n[1] |= (nlen >> 8);
		n[2] = nlen & 0xFF;
		n[5] ^= 0x3F; // change version

		crc = crc_32(n, nlen - 1);
		copy32(n, nlen - 1, crc);
		copy16(n, 1498, nlen + 1); // set the position at the end of the pmt
		_cc = b[3] & 0xF;		   // continuity counter
		_cc = (_cc - 1) & 0xF;
		cc = (uint8_t *)n + 1497;
		*cc = _cc;
	}

	if (clean)
	{
		uint16_t *pos = (uint16_t *)clean + 1498;
		pmt_len = ((clean[1] & 0xF) << 8) + clean[2];
		cc = (uint8_t *)clean + 1497;
		if (b[1] & 0x40)
			*pos = 0;
		if (*pos > pmt_len)
		{
			mark_pid_null(b);
			return;
		}
		if (*pos == 0)
		{
			memcpy(b + 5, clean, 183);
			*pos = 183;
		}
		else
		{
			memcpy(b + 4, clean + *pos, 184);
			*pos += 184;
		}
		*cc = (*cc + 1) & 0xF;
		b[3] = (b[3] & 0xF0) | *cc;
		return;
	}
	mark_pid_null(b);
}

int assemble_packet(uint8_t **b1, adapter *ad, int check_crc)
{
	int len = 0, pid;
	uint32_t crc, current_crc;
	int64_t item_key;
	uint8_t *b = *b1;

	if ((b[0] != 0x47)) // make sure we are dealing with TS
		return 0;

	pid = (b[1] & 0x1F) * 256 + b[2];

	if ((b[1] & 0x40) == 0x40)
		len = ((b[6] & 0xF) << 8) + b[7];

	if (len > 1500 || len < 0)
		LOG_AND_RETURN(0,
					   "assemble_packet: len %d not valid for pid %d [%02X %02X %02X %02X %02X %02X]",
					   len, pid, b[3], b[4], b[5], b[6], b[7], b[8]);

	item_key = TABLES_ITEM + (ad->id << 16) + pid;

	if (!getItem(item_key) && !len)
		return 0;

	if (len > 180)
	{
		setItem(item_key, b + 5, 183, 0);
		setItemTimeout(item_key, ASSEMBLE_TIMEOUT);
		return 0;
	}
	else if (len > 0)
	{
		b = b + 5;
	}
	else // pmt_len == 0 - next part from the pmt
	{
		setItem(item_key, b + 4, 184, -1);
		//		setItemTimeout(item_key, ASSEMBLE_TIMEOUT);
		b = getItem(item_key);
		len = ((b[1] & 0xF) << 8) + b[2];
		if (getItemLen(item_key) < len)
			return 0;
	}
	*b1 = b;
	if (check_crc) // check the crc for PAT and PMT
	{
		if (len < 4 || len > 1500)
			LOG_AND_RETURN(0, "assemble_packet: len %d not valid for pid %d",
						   len, pid);
		crc = crc_32(b, len - 1);
		copy32r(current_crc, b, len - 1) if (crc != current_crc)
			LOG_AND_RETURN(0, "pid %d (%04X) CRC failed %08X != %08X len %d",
						   pid, pid, crc, current_crc, len);
	}
	return len;
}

int process_pat(int filter, unsigned char *b, int len, void *opaque)
{
	int pat_len = 0, i, tid = 0, sid, pid, ver, csid = 0;
	int16_t *pids;
	unsigned char *init_b = b;
	SPid *p;
	adapter *ad = (adapter *)opaque;

	if (((b[1] & 0x1F) != 0) || (b[2] != 0))
		return 0;

	if (b[0] != 0x47)
		return 0;

	if ((b[1] & 0x40) && ((b[4] != 0) || (b[5] != 0)))
		return 0;

	if (ad->pat_processed && ((b[1] & 0x40) == 0))
		return 0;

	//	p = find_pid(ad->id, 0);
	//	if(!p)
	//		return 0;

	tid = b[8] * 256 + b[9];
	ver = b[10] & 0x3E;

	if ((ad->transponder_id == tid) && (ad->pat_ver == ver)) //pat already processed
		return 0;

	if (!(pat_len = assemble_packet(&b, ad, 1)))
		return 0;

	tid = b[3] * 256 + b[4];
	ver = b[5] & 0x3E;
	if (((ad->transponder_id != tid) || (ad->pat_ver != ver)) && (pat_len > 0) && (pat_len < 1500))
	{
		ad->pat_processed = 0;
	}

	if (ad->pat_processed)
		return 0;

	ad->pat_ver = ver;
	ad->transponder_id = tid;
#ifndef DISABLE_DVBAPI
	dvbapi_delete_keys_for_adapter(ad->id);
#endif
	//	LOG("tid %d pat_len %d: %02X %02X %02X %02X %02X %02X %02X %02X", tid, pat_len, b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7]);
	pids = NULL;
	memset(pids, 0, 8192 * sizeof(*pids));
	pat_len -= 9;
	b += 8;
	LOGL(2, "PAT Adapter %d, Transponder ID %d, len %d, version %d", ad->id,
		 tid, pat_len, ad->pat_ver);
	if (pat_len > 1500)
		return 0;

	for (i = 0; i < pat_len; i += 4)
	{
		sid = b[i] * 256 + b[i + 1];
		pid = (b[i + 2] & 0x1F) * 256 + b[i + 3];
		LOGL(2, "Adapter %d, PMT sid %d (%04X), pid %d", ad->id, sid, sid, pid);
		if (sid > 0)
		{
			pids[pid] = -TYPE_PMT;
			p = find_pid(ad->id, pid);
			if (!p)
			{
				mark_pid_add(-1, ad->id, pid);
				p = find_pid(ad->id, pid);
				if (p)
					p->type = PMT_SKIPFIRST;
			}
			if ((p = find_pid(ad->id, pid)))
			{
				p->type |= TYPE_PMT;
				csid = pid;
				if (p->flags == 3)
					p->flags = 1;
			}
		}
	}
	update_pids(ad->id);
	ad->pat_processed = 1;
	return csid;
}

int pi_exist(int ecapid, int ecaid, unsigned char *es, int len)
{
	int es_len, caid, capid;
	int i;

	for (i = 0; i < len; i += es_len) // reading program info
	{
		es_len = es[i + 1] + 2;
		caid = es[i + 2] * 256 + es[i + 3];
		capid = (es[i + 4] & 0x1F) * 256 + es[i + 5];
		if (caid == ecaid && capid == ecapid)
			return 1;
	}
	return 0;
}

int is_ac3_es(unsigned char *es, int len)
{
	int i, es_len, isAC3 = 0;
	for (i = 0; i < len; i += es_len)
	{
		es_len = es[i + 1] + 2;
		if (es[i] == 0x6A)
			isAC3 = 1;
	}

	return isAC3;
}

void find_pi(unsigned char *es, int len, unsigned char *pi, int *pi_len)
{

	int es_len, caid, capid;
	int i;

	for (i = 0; i < len; i += es_len) // reading program info
	{
		es_len = es[i + 1] + 2;
		if (es[i] != 9)
			continue;
		caid = es[i + 2] * 256 + es[i + 3];
		capid = (es[i + 4] & 0x1F) * 256 + es[i + 5];
		if (!pi_exist(capid, caid, pi, *pi_len))
		{
			if (*pi_len + es_len > MAX_PI_LEN)
				return;
			LOG("PI pos %d caid %04X => pid %04X (%d)", *pi_len, caid, capid,
				capid);
			memcpy(pi + *pi_len, es + i, es_len);
			*pi_len += es_len;
		}
	}
	return;
}

int get_master_pmt_for_pid(int aid, int pid)
{
	int i, j;
	SPMT *pmt;
	for (i = 0; i < MAX_PMT; i++)
		if (pmts[i] && pmts[i]->enabled && pmts[i].adapter == aid)
		{
			pmt = pmts[i];
			for (j = 0; j < MAX_ACTIVE_PIDS && pmt->active_pid[j] > 0; j++)
				if (pmt->active_pid[j] == pid)
					return pmt->master_pmt;
		}
	return -1;
}

int process_pmt(int filter, unsigned char *b, int len, void *opaque)
{
	int pi_len = 0, isAC3, ver, pmt_len = 0, i, _pid, es_len, init_pi_len;
	int program_id = 0;
	int prio = 0;
	int enabled_channels = 0;
	unsigned char *pmt_b, *pi, tmp_pi[MAX_PI_LEN];
	unsigned char *init_b = b;
	int caid, capid, pid, spid, stype;
	uint16_t pid_list[MAX_PIDS];
	int npl = 0;
	SPid *p, *cp;
	int64_t pid_key = 0;
	int16_t *pids;
	adapter *ad;
	int opmt, old_key;
	int pmt_id = (int)opaque;

	SPMT *pmt = get_pmt(pmt_id);
	SFilter *f = get_filter(filter);
	if (!pmt)
	{
		LOG("PMT %d does not exist", pmt_id);
		return 0;
	}

	ad = get_adapter(pmt->adapter);
	if (!ad)
	{
		LOG("Adapter %d does not exist", pmt->adapter);
		return 0;
	}
	pid = f->pid;

	if (!(p = find_pid(ad->id, pid)))
		return -1;

	if (!p || (p->type & PMT_COMPLETE) || (p->type == 0))
		return 0;

	pi_len = ((b[10] & 0xF) << 8) + b[11];

	program_id = pmt->sid = b[3] * 256 + b[4];
	ver = pmt->version = b[5] & 0x3F;

	LOG("PMT pid: %04X (%d), pmt_len %d, pi_len %d, sid %04X (%d)", pid, pid,
		pmt_len, pi_len, program_id, program_id);
	pi = b + 12;
	pmt_b = pi + pi_len;

	if (pmt_len > 1500)
		return 0;

	if (pi_len > pmt_len)
		pi_len = 0;

	init_pi_len = pi_len;
	pi_len = 0;

	if (init_pi_len > 0)
		find_pi(pi, init_pi_len, tmp_pi, &pi_len);

	pi = tmp_pi;

	es_len = 0;
	pids = (int16_t *)getItem(pid_key);
	if (!pids)
		return 0;

	p->type |= TYPE_PMT;
	pids[pid] = -TYPE_PMT;

	for (i = 0; i < pmt_len - init_pi_len - 17; i += (es_len) + 5) // reading streams
	{
		es_len = (pmt_b[i + 3] & 0xF) * 256 + pmt_b[i + 4];
		stype = pmt_b[i];
		spid = (pmt_b[i + 1] & 0x1F) * 256 + pmt_b[i + 2];
		isAC3 = 0;
		if (stype == 6)
			isAC3 = is_ac3_es(pmt_b + i + 5, es_len);

		LOG("PMT pid %d - stream pid %04X (%d), type %d%s, es_len %d, pos %d, pi_len %d old pmt %d, old pmt for this pid %d",
			pid, spid, spid, stype, isAC3 ? " [AC3]" : "", es_len, i, pi_len, pids[pid], pids[spid]);
		if ((es_len + i > pmt_len) || (init_pi_len + es_len == 0))
			break;

		if (stype != 2 && stype != 3 && stype != 4 && !isAC3 && stype != 27 && stype != 36)
			continue;

		find_pi(pmt_b + i + 5, es_len, pi, &pi_len);

		if (pi_len == 0)
			continue;

		opmt = pids[spid];
		pids[spid] = pid;
		if ((opmt > 0) && (abs(opmt) != abs(pid))) // this pid is associated with another PMT - link this PMT with the old one (if not linked already)
		{
			if (pids[pid] == -TYPE_PMT)
			{
				pids[pid] = -opmt;
				LOG("Linking PMT pid %d with PMT pid %d for pid %d, adapter %d",
					pid, opmt, spid, ad->id);
			}
		}

		if ((cp = find_pid(ad->id, spid))) // the pid is already requested by the client
		{
			enabled_channels++;
			pid_list[npl++] = spid;
			//			old_key = cp->key;
		}
	}

	if ((pi_len > 0) && enabled_channels) // PMT contains CA descriptor and there are active pids
	{
		if (program_id > 0)
			run_ca_action(CA_ADD_PMT, ad, &pmt);
		else
			LOG("PMT %d, SID is 0, not running ca_action", pid);

		for (i = 0; i < npl; i++)
		{
			cp = find_pid(ad->id, pid_list[i]);
			//			if (cp)
			//				cp->key = p->key;
		}
		p->type |= PMT_COMPLETE;
	}
	else
	{
		p->type = 0; // we do not need this pmt pid anymore
		mark_pid_deleted(ad->id, 99, p->pid, p);
		do_dump_pids = 0;
		update_pids(ad->id);
		do_dump_pids = 1;
	}
	//	free_assemble_packet(pid, ad);
	if (opts.clean_psi && p->sid[0] != -1)
		clean_psi(ad, b);

	return 0;
}

// clean psi from CA info

void pmt_pid_add(adapter *ad, int pid, int existing)
{
	int i;
	SPid *p, *cp;
	SPMT *pmt;
	if (!ad)
		return;
	cp = find_pid(ad->id, pid);
	if (!cp)
		return;

	cp->filter = get_pid_filter(ad->id, pid);
	if (cp->filter != -1)
		cp->type |= TYPE_FILTER;

#ifndef DISABLE_TABLES

	run_ca_action(CA_ADD_PID, ad, &pid);
	//	invalidate_adapter(ad->id);
	int pmt_pid = get_master_pmt_for_pid(ad->id, pid);
	cp = find_pid(ad->id, pmt_pid);
	if (!cp)
	{
		for (i = 0; i < MAX_PMT; i++)
			if (pmts[i] && pmts[i]->enabled && pmts[i]->adapter == ad->id && pmts[i]->master_pmt == pmt_pid)
			{
				pmt = pmts[i];
				SPid *mp = find_pid(ad->id, pmt->pid);
				if (!mp)
					mark_pid_add(-1, ad->id, pmt->pid);
				SPid *mp = find_pid(ad->id, pmt->pid);
				if (mp && !(mp->type & TYPE_PMT))
				{
					mp->type |= TYPE_PMT;
					run_ca_action(CA_ADD_PMT, ad, &pmt);
				}
			}
	}

#endif
}

void pmt_pid_del(adapter *ad, int pid)
{
	int ep;
	SPid *p;
	if (!ad)
		return;

// filter code

#ifndef DISABLE_TABLES
	int i;
	run_ca_action(CA_DEL_PID, ad, &pid);
	p = find_pid(ad->id, pid);
	if (!p)
		return;
	SPMT *pmt = get_pmt(p->pmt);
	if (pmt)
		LOGL(2, "tables_pid_del: pid %d adapter %d pmt %d",
			 pid, ad->id, p->pmt)
	else
		return;
	ep = 0;
	for (i = 0; i < MAX_ACTIVE_PIDS && pmt->active_pid[i]; i++)
		if (pmt->active[i] != pid && find_pid(ad->id, pmt->active_pid[i]))
			ep++;

	if (!ep)
	{

		for (i = 0; i < MAX_PMT; i++)
			if (pmts[i] && pmts[i]->enabled && pmts[i]->adapter == ad->id && pmts[i]->master_pmt == pmt->master_pmt)
			{
				run_ca_action(CA_DEL_PMT, ad, pmt);
			}
	}
#endif
}

int pmt_init_device(adapter *ad)
{
#ifndef DISABLE_TABLES
	tables_init_device(ad);
#endif
}

int pmt_close_device(adapter *ad)
{
#ifndef DISABLE_TABLES
	tables_close_device(ad);
#endif
}
int pmt_tune(adapter *ad)
{
	int i;
	SFilter *f;

	if (ad->pat_filter == -1)
	{
		ad->pat_filter = add_filter(ad->id, 0, (void *)process_pat, ad, FILTER_PERMANENT);
		SPid *p = find_pid(ad->id, 0);
		p->type |= TYPE_FILTER;
		p->filter = ad->pat_filter;
	}

	for (i = 0; i < nfilters; i++)
		if ((f = get_filter(i)) && (f->adapter == ad->id) && !(f->flags & FILTER_PERMANENT))
			del_filter(i);
}

void free_all_pmts(void)
{
	SPMT *p;
	int i;
	for (i = 0; i < MAX_PMT; i++)
	{
		if (pmts[i])
		{
			mutex_destroy(&pmts[i]->mutex);
			free(pmts[i]);
		}
	}
	mutex_destroy(&pmts_mutex);
}

_symbols pmt_sym[] =
	{
		{"pmt_enabled", VAR_AARRAY_INT8, pmts, 1, MAX_PMT, offsetof(SPMT, enabled)},
		{"pmt_adapter", VAR_AARRAY_INT, pmts, 1, MAX_PMT, offsetof(SPMT, adapter)},

		{NULL, 0, NULL, 0, 0}};
