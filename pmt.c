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

int enabledKeys = 0;

SPMT *pmts[MAX_PMT];
SMutex pmts_mutex;

_Spmt_op ops[10];

SCW *cws[MAX_CW];
SMutex cws_mutex;


SCW *get_cw(int id)
{
	if(id <0 || id >= MAX_CW || !cws[id] || !cws[id]->enabled)
		return NULL;
	return cws[id];

}

SPMT *get_pmt(int id)
{
	if(id <0 || id >= MAX_KEYS || !pmts[id] || !pmts[id]->enabled)
		return NULL;
	return pmts[id];
}

int register_algo(SPMT_op *o)
{
	int i;
	for (i=0; i<sizeof(ops); i++)
		if(!ops[i].enabled)
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
	NULL
}

void init_algo()
{
	int i;
	for(i=0; i<sizeof(algo_init_func); i++)
		if(algo_init_func[i])
			algo_init_func[i]();
}

#define POINTER_TYPE_ECM ((void *)-TYPE_ECM)
#define POINTER_1 ((void *)1)

void invalidate_adapter(int aid)
{
	adapter *ad = get_adapter(aid);
	if(ad)
		ad->invalidated = 1;
}

/*
   SKey *get_active_key1(SPid *p)
   {
   SKey *k;
   adapter *ad;
   int key = p->key;
   int counter = 0;
   int64_t ctime = getTick();
   uint8_t nullcw[16] =
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   if (key == 255)
   return NULL;
   if (p->type & TYPE_ECM)
   return POINTER_TYPE_ECM;
   if (p->type & TYPE_PMT)
   return NULL;
   k = get_key(key);
   if (k == NULL)
   return NULL;
   ad = get_adapter(k->adapter);

   LOGL(3, "get_active_key: searching key for pid %d, starting key %d parity %d ok %d %d",
      p->pid, key, k->parity, k->key_ok[0], k->key_ok[1]);
   while (k && k->enabled && (counter++ < 10))
   {
   if ((k->parity != -1) && k->key_ok[k->parity]
      && (ctime - k->cw_time[k->parity] > MAX_KEY_TIME)) // key expired
   {
   k->key_ok[k->parity] = 0;
   LOGL(2, "get_active_key: Invalidating key %d, parity %d, as the CW expired, pid %d",
        k->id, k->parity, p->pid);
   }

   if ((k->parity != -1) && (ctime - k->cw_time[k->parity] > 5000)
      && (memcmp(nullcw, k->next_cw[k->parity], k->key_len) != 0))
   {
   char *cw = k->next_cw[k->parity];
   LOGL(2, "get_active_key: Setting the queued CW as active CW for key %d, parity %d: %02X %02X %02X ...",
        k->id, k->parity, cw[0], cw[1], cw[2]);
   memcpy(k->cw[k->parity], k->next_cw[k->parity], k->key_len);
   memset(k->next_cw[k->parity], 0, k->key_len);
   k->op->set_cw(k->cw[k->parity], k->key[k->parity]);

   }

   if (((k->parity != -1) && k->key_ok[k->parity])
 || ((k->parity == -1) && (k->key_ok[0] || k->key_ok[1])))
   {
   char buf[200];
   buf[0] = 0;
   LOGL(1,
        "get_active_key: returning key %d for pid %d, d/c errs %d/%d, adapter pid errs %d",
        k->id, p->pid, p->dec_err, p->err, ad ? ad->pid_err : -1);

   return k;
   }

   k = k->next_key;
   LOGL(3, "get_active_key: trying next key for pid %d, key %d parity %d ok %d %d",
       p->pid, k ? k->id : -1, k ? k->parity : -1, k ? k->key_ok[0] : -1, k ? k->key_ok[1] : -1);

   }
   LOGL(1, "get_active_key: returning NULL for pid %d, starting key %d",
      p->pid, key);
   return NULL;
   }

   int8_t get_active_key(SPMT *_pmt)
   {
   adapter *ad = get_adapter(pmt->adapter);
   int counter = 0;
   int8_t next_pmt = 0;
   SPMT *pmt;
   SCW *acw[2];
   int64_t ctime = getTick();
   uint8_t nullcw[16] =
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   SPid *p = get_pid(_pmt->pid);
   if(!p)
   return -1;
   _pmt->p = p;
   if (p->type & TYPE_ECM)
   return -TYPE_ECM;
   if (p->type & TYPE_PMT)
   return -TYPE_PMT;

   LOGL(3, "get_active_key: searching CW for pid %d, starting pmt %d",
      pmt->pid, pmt->id);
   acw[0] = NULL;
   acw[1] = NULL;
   for(next_pmt = pmt->id; next_pmt >= 0; next_pmt = pmt->next_pmt)
   {
   pmt = ad->pmt[next_pmt];
   int i = 0;
   for ( i = 0; i< sizeof(ad->pmt->cw); i++)
   {
   CW *cw = ad->pmt->cw + i;
   parity = i & 1;

   if ((ctime - cw->cw_time > MAX_KEY_TIME)) // key expired
   {
    cw->enabled = 0;
    LOGL(2, "get_active_key: Invalidating key %d, parity %d, as the CW expired, pid %d",
         i, parity, p->pid);
   }

   if(!acw[parity] && cw->enabled)
    acw[parity] = cw;

   int new_cw = 0;

   if(acw[parity].prio < cw->prio)
    new_cw = 1;

   if(acw[parity].cw_time > cw->cw_time)
    new_cw = 1;


   if(new_cw)
    acw[parity] = cw;

   k = k->next_key;
   LOGL(3, "get_active_key: trying next key for pid %d, key %d parity %d ok %d %d",
        p->pid, k ? k->id : -1, k ? k->parity : -1, k ? k->key_ok[0] : -1, k ? k->key_ok[1] : -1);

   }
   }
   int new_cw = 0;
   for(i=0; i<2; i++)
   if(acw[i] != _pmt->active_cw[i])
   {
   _pmt->active_cw[i] = acw[i];
   acw[i]->op->set_cw(acw[i]->cw, acw[i]->key);
   new_cw++;

   }
   return !new_cw;
   }

   void set_next_pmt(int p1, int p2)
   {
   SPMT *pmt1, *pmt2;
   key1 = get_pmt(p1);
   key2 = get_pmt(p2);
   if (!pmt1 || !pmt2)
   return;
   LOG("Next pmt %d set to pmt %d", pmt1->id, pmt2->id);
   pmt1->next_key = pmt2;
   }
 */
void update_pid_key(adapter *ad)
{
	int i = 0;

	for (i = 0; i < MAX_CW; i++)
		if(cws[i] && cws[i]->enabled && cws[i]->adapter == ad->id)
		{

		}
}

int send_cw(int pmt_id, int type, int parity, uint8_t *cw)
{
	LOG("got CW for PMT %d, type %d, parity %d: %02X %02X %02X %02X %02X %02X %02X %02X", pmt_id, type, parity,
					cw[0], cw[1], cw[2], cw[3], cw[4], cw[5], cw[6], cw[7]);
	return 0;

}

int decrypt_batch(SPMT *p)
{
	int oldb1 = -1, oldb2 = -1;
	unsigned char *b = NULL, *oldb = NULL;
	int bl, i, pid = 0;
	if (!p)
		LOG_AND_RETURN(-1, "Unable to decrypt k=NULL for key %d", p->id);
	mutex_lock(&p->mutex);
	if (k->blen <= 0 || (p->parity == -1) || (!p->key_ok[p->parity]))
	{
		LOG("Unable to decrypt blen = %d, parity = %d, key_ok %d for key %d",
						p->blen, p->parity, (p->parity >= 0) ? p->key_ok[p->parity] : 0,
						p->id);
		mutex_unlock(&p->mutex);
		return -1;
	}
	b = p->batch[0].data - 4;
	pid = (b[1] & 0x1F) * 256 + b[2];
	p->batch[p->blen].data = NULL;
	p->batch[p->blen].len = 0;
	p->op->decrypt_stream(p->key[p->parity], p->batch, 184);
	LOGL(6,
						"dvbapi: decrypted key %d parity %d at len %d, channel_id %d (pid %d) %p",
						p->id, p->parity, p->blen, p->sid, pid, p->batch[0].data); //0x99
	p->blen = 0;
	memset(p->batch, 0, sizeof(int *) * 128);
	mutex_unlock(&p->mutex);
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

	update_pid_key(ad);

	for (i = 0; i < rlen; i += 188)
	{
		b = ad->buf + i;
		pid = (b[1] & 0x1F) * 256 + b[2];
		if (b[3] & 0x80)
		{
			p = get_pid(ad->id, pid);
			if (p->pmt > 0 && p->pmt < MAX_PMTS && pmts[p->pmt])
				pmt = pmts[p->pmt];
			else
				pmt = NULL;
			if (!pmt)
			{
				continue; // cannot decrypt
			}

			cp = ((b[3] & 0x40) > 0);
			if (pmt->parity == -1)
				pmt->parity = cp;

			if (!batchSize)
				batchSize = k->op->batch_size();

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
					invalidate_adapter(ad->id);
					update_pid_key(ad);
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

	for (i = 0; i < MAX_PMT; i++)  // decrypt everything that's left
		if (pmts[i] && pmts[i]->enabled && (pmts[i]->blen > 0)
						&& (pmts[i]->adapter == ad->id))
			decrypt_batch(pmts[i]);

}


void process_filter(adapter *ad, unsigned char *b)
{

}

int pmt_process_stream(adapter *ad)
{
	SPid *p;
	int i, pid;
	uint8_t *b;

	int64_t pid_key = TABLES_ITEM + ((1 + ad->id) << 24) + 0;
	int16_t *pids;
	int rlen = ad->rlen;

	if (nca == 0)
		return 0;

	if (ad->ca_mask == 0) // no CA enabled on this adapter
		return 0;

	pids = (int16_t *) getItem(pid_key);

	for (i = 0; i < rlen; i += 188)
	{
		b = ad->buf + i;
		pid = PID_FROM_TS(b);
		p = find_pid(ad->id, pid);
		if(p && (p->type & TYPE_FILTER))
		{
			process_filter(ad, b);
//			continue;
		}
//		if (p && p->type == TYPE_ECM)
//			run_ca_action(CA_ECM, ad, b);

	}
	pmt_decrypt_stream(ad);
	run_ca_action(CA_TS, ad, &rlen);
	if(opts.drop_encrypted)
	{
		for (i = 0; i < rlen; i += 188)
		{
			b = ad->buf + i;
			pid = PID_FROM_TS(b);
			p = find_pid(ad->id, pid);
			ad->dec_err++;
			if (p)
				p->dec_err++;

			if((b[3] & 0x80) == 0x80)
			{
				b[1] |= 0x1F;
				b[2] |= 0xFF;
			}
		}
	}


	return 0;
}



void create_cw_keys(SKey *k)
{
	if (!k->key[0])
		k->key[0] = k->op->create_cwkey();
	if (!k->key[1])
		k->key[1] = k->op->create_cwkey();
}

void delete_cw_keys(SKey *k)
{
	if (!k->key[0])
		k->op->delete_cwkey(k->key[0]);
	if (!k->key[1])
		k->op->delete_cwkey(k->key[1]);
	k->key[0] = k->key[1] = NULL;
}

int set_algo(SKey *k, int algo, int mode)
{
	int i;
	dvbapi_op *op = NULL;
	if (k->op->algo == algo && k->op->mode == mode)
		return 1;

	for (i = 0; ops[i]; i++)
		if (ops[i]->algo == algo && ops[i]->mode == mode)
			op = ops[i];

	if (!op)
	{
		LOG("%s: key %d: no matching algorithm found for algo %d and mode %d",
						__FUNCTION__, k->id, algo, mode);
		return 2;
	}
	mutex_lock(&k->mutex);
	delete_cw_keys(k);
	k->op = op;
	create_cw_keys(k);
	k->op->set_cw(k->cw[0], k->key[0]);
	k->op->set_cw(k->cw[1], k->key[1]);
	mutex_unlock(&k->mutex);
	return 0;
}

int keys_add(int i, int adapter, int sid, int pmt_pid)
{

	SKey *k;
	if( i == -1)
		i = add_new_lock((void **) keys, MAX_KEYS, sizeof(SKey), &keys_mutex);
	else
	{
		if(keys[i])
			mutex_lock(&keys[i]->mutex);
		else
		{
			keys[i] = malloc(sizeof(SKey));
			if(!keys[i])
				LOG_AND_RETURN(-1, "Could not allocate memory for the key %d", i);
			memset(keys[i], 0, sizeof(SKey));
			mutex_init(&keys[i]->mutex);
			mutex_lock(&keys[i]->mutex);
		}

	}
	if (i == -1 || !keys[i])
	{
		LOG_AND_RETURN(-1, "Key buffer is full, could not add new keys");
	}

	k = keys[i];

	k->op = ops[0];
	create_cw_keys(k);

	if (!k->key[0] || !k->key[1])
	{
		mutex_unlock(&k->mutex);
		LOG_AND_RETURN(-1, "Count not allocate dvbcsa key, keys_add failed");
	}
	k->parity = -1;
	k->key_ok[0] = k->key_ok[1] = 0;
	k->sid = sid;
	k->pmt_pid = pmt_pid;
	k->adapter = adapter;
	k->demux = -1;
	k->id = i;
	k->blen = 0;
	k->enabled = 1;
	k->ver = -1;
	k->next_key = NULL;
	k->ecm_info = k->cardsystem = k->reader = k->from = k->protocol = NULL;
	k->cw_time[0] = k->cw_time[1] = 0;
	k->key_len = 8;
	k->ecms = 0;
	k->last_dmx_stop = 0;
	memset(k->cw[0], 0, 16);
	memset(k->cw[1], 0, 16);
	memset(k->next_cw[0], 0, 16);
	memset(k->next_cw[1], 0, 16);
	mutex_unlock(&k->mutex);
	invalidate_adapter(adapter);
	enabledKeys++;
	LOG("returning new key %d for adapter %d, pmt pid %d sid %04X", i, adapter,
					pmt_pid, sid);

	return i;
}

int keys_del(int i)
{
	int aid, j, ek;
	SKey *k;
	unsigned char buf[8] =
	{ 0x9F, 0x80, 0x3f, 4, 0x83, 2, 0, 0 };
	k = get_key(i);
	if (!k)
		return 0;

	mutex_lock(&k->mutex);
	if (!k->enabled)
	{
		mutex_unlock(&k->mutex);
		return 0;
	}
	aid = k->adapter;
	k->enabled = 0;
//	buf[7] = k->demux;
	buf[7] = i;
	LOG("Stopping DEMUX %d, removing key %d, sock %d, pmt pid %d", buf[7], i,
					sock, k->pmt_pid);
	if ((buf[7] != 255) && (sock > 0))
		TEST_WRITE(write(sock, buf, sizeof(buf)));

	delete_cw_keys(k);

	k->sid = 0;
	k->pmt_pid = 0;
	k->adapter = -1;
	k->demux = -1;
	k->last_dmx_stop = 0;
	reset_ecm_type_for_key(aid, i);
//	if (k->next_key)
//		keys_del(k->next_key->id);
//		tables_pid_del(get_adapter(k->next_key->adapter), k->next_key->pmt_pid);
	k->next_key = NULL;
	invalidate_adapter(aid);
	ek = 0;
	delItemP(k->ecm_info);
	k->ecm_info = k->cardsystem = k->reader = NULL;
	k->from = k->protocol = NULL;
	k->hops = k->caid = k->info_pid = k->prid = k->ecmtime = 0;
	buf[7] = 0xFF;
	for (j = 0; j < MAX_KEYS; j++)
		if (keys[j] && keys[j]->enabled)
			ek++;
	enabledKeys = ek;
	if (!ek && sock > 0)
		TEST_WRITE(write(sock, buf, sizeof(buf)));
	mutex_destroy(&k->mutex);
	return 0;
}

SKey *get_key(int i)
{
	if (i < 0 || i >= MAX_KEYS || !keys[i] || !keys[i]->enabled)
		return NULL;
	return keys[i];
}

void dvbapi_add_pid(adapter *ad, void *arg)
{
	invalidate_adapter(ad->id);
}

void dvbapi_del_pid(adapter *ad, void *arg)
{
	invalidate_adapter(ad->id);
}


int pmt_enabled_channels(int id)
{
}

SCA dvbapi;


void free_all_pmts(void)
{
	SPMT *p;
	int i;
	for (i = 0; i < MAX_PMTS; i++) {
		if (pmts[i]) {
			mutex_destroy(&pmts[i]->mutex);
			free(pmts[i]);
		}
	}
	mutex_destroy(&pmts_mutex);
}


_symbols dvbapi_sym[] =
{
	{ "pmt_enabled", VAR_AARRAY_INT8, pmts, 1, MAX_PMTS, offsetof(SPMT, enabled) },
	{ "pmt_adapter", VAR_AARRAY_INT, pmts, 1, MAX_PMTS, offsetof(SPMT, adapter) },

	{ NULL, 0, NULL, 0, 0 }
};
