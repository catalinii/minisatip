/*
 * Copyright (C) 2014-2020 Catalin Toda <catalinii@yahoo.com>
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
#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>
#include <linux/dvb/ca.h>
#include <fcntl.h>
#include <ctype.h>
#include "dvb.h"
#include "socketworks.h"
#include "minisatip.h"
#include "dvbapi.h"
#include "adapter.h"
#include "tables.h"

extern struct struct_opts opts;

const int64_t DVBAPI_ITEM = 0x1000000000000;
int dvbapi_sock = -1;
int sock;
int dvbapi_is_enabled = 0;
int batchSize;
int dvbapi_protocol_version = DVBAPI_PROTOCOL_VERSION;

SKey keys[MAX_KEYS];
unsigned char read_buffer[1500];

#define TEST_WRITE(a) if((a)<=0){LOG("write to dvbapi socket failed, closing socket %d",sock);sockets_del(dvbapi_sock);sock = 0;dvbapi_sock = -1;dvbapi_is_enabled = 0;}
#define POINTER_TYPE_ECM ((void *)-TYPE_ECM)
#define POINTER_1 ((void *)1)

void invalidate_adapter(int aid)
{
	int64_t pk_key = DVBAPI_ITEM + ((1 + aid) << 24) + 1;
	SKey **pid_to_key = (SKey **) getItem(pk_key);
	if (pid_to_key)
		pid_to_key[0] = POINTER_1; // invalidate the cache
}

#define dvbapi_copy32r(v, a, i) if(change_endianness)copy32rr(v, a, i) else copy32r(v, a, i)
#define dvbapi_copy16r(v, a, i) if(change_endianness)copy16rr(v, a, i) else copy16r(v, a, i)

void send_client_info(sockets *s);
int dvbapi_reply(sockets * s)
{
	unsigned char *b = s->buf;
	SKey *k;
	int change_endianness = 0;
	unsigned int op, _pid;
	int k_id, a_id = 0, pos = 0;
	int demux, filter;
	SPid *p;
	if (s->rlen == 0)
	{
		send_client_info(s);
		return 0;
	}
	while (pos < s->rlen)
	{
		int op1;
		b = s->buf + pos;
		copy32r(op, b, 0);
		op1 = op & 0xFFFFFF;
		change_endianness = 0;
		if (op1 == CA_SET_DESCR_X || op1 == CA_SET_DESCR_AES_X
				|| op1 == CA_SET_PID_X || op1 == DMX_STOP_X
				|| op1 == DMX_SET_FILTER_X)
		{ // change endianness
			op = 0x40000000 | ((op1 & 0xFF) << 16) | (op1 & 0xFF00)
					| ((op1 & 0xFF0000) >> 16);
			if (!(op & 0xFF0000))
				op &= 0xFFFFFF;
			LOG("dvbapi: changing endianness from %06X to %08X", op1, op);
			//b ++;
			//pos ++;
			b[4] = b[0];
			change_endianness = 1;
		}
		LOGL(3,
				"dvbapi read from socket %d the following data (%d bytes), pos = %d, op %08X, key %d",
				s->sock, s->rlen, pos, op, b[4]);
//		LOGL(3, "dvbapi read from socket %d the following data (%d bytes), pos = %d, op %08X, key %d -> %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", s->sock, s->rlen, pos, op, b[4], b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10]);

		switch (op)
		{

		case DVBAPI_SERVER_INFO:

			if (s->rlen < 6)
				return 0;
			dvbapi_copy16r(dvbapi_protocol_version, b, 4)
			;
			LOG("dvbapi: server version %d found, name = %s",
					dvbapi_protocol_version, b + 7)
			;
			if (dvbapi_protocol_version > DVBAPI_PROTOCOL_VERSION)
				dvbapi_protocol_version = DVBAPI_PROTOCOL_VERSION;

			register_dvbapi();
			dvbapi_is_enabled = 1;
			pos = 6 + strlen(b + 6) + 1;
			break;

		case DVBAPI_DMX_SET_FILTER:
		{
			SKey *k;
			if (change_endianness)
				pos += 2;  // for some reason the packet is longer with 2 bytes
			pos += 65;
			if (s->rlen < 9)
				return 0;
			dvbapi_copy16r(_pid, b, 7);
			_pid &= 0x1FFF;
			k_id = b[4];
			k = get_key(k_id);
			a_id = -1;
			if (k)
				a_id = k->adapter;

			demux = b[5];
			filter = b[6];
			LOG(
					"dvbapi requested set filter for pid %04X (%d), key %d, demux %d, filter %d",
					_pid, _pid, k_id, demux, filter);
			if (!(p = find_pid(a_id, _pid)))
			{
				mark_pid_add(-1, a_id, _pid);
				update_pids(a_id);
				p = find_pid(a_id, _pid);
			}
			if (!p)
				break;
//			if(k && p->key != k_id)
			if (p->filter == 255)
			{
				p->filter = filter;
				k->demux = demux;
				p->type = TYPE_ECM;
				p->key = k_id;
				p->ecm_parity = 255;
				invalidate_adapter(k->adapter);

			}
//			else p->ecm_parity = 255;

			break;
		}
		case DVBAPI_DMX_STOP:
		{
			k_id = b[4];
			demux = b[5];
			filter = b[6];
			pos += 9;
			k = get_key(k_id);
			if (!k)
				break;
			a_id = k->adapter;
			dvbapi_copy16r(_pid, b, 7)
			_pid &= 0x1FFF;
			LOG(
					"dvbapi: received DMX_STOP for key %d, adapter %d, demux %d, filter %d, pid %X (%d)",
					k_id, a_id, demux, filter, _pid, _pid);
			if ((p = find_pid(a_id, _pid)) && (p->key == k_id))
			{
				p->type = 0;
				p->key = p->filter = 255;
				invalidate_adapter(k->adapter);
			}

			break;
		}
		case DVBAPI_CA_SET_PID:
		{
			LOG("received DVBAPI_CA_SET_PID");
			pos += 13;
			break;
		}
		case DVBAPI_CA_SET_DESCR:
		{
			int index, parity, k_id;
			SKey *k;
			SPid *p;
			unsigned char *cw;
			uint32_t ctime = getTick();

			pos += 21;
			k_id = b[4];
			dvbapi_copy32r(index, b, 5);
			dvbapi_copy32r(parity, b, 9);
			cw = b + 13;
			k = get_key(k_id);
			if (k && (parity < 2))
			{
				char *queued = "";
				int do_queue = 0;
				mutex_lock(&k->mutex);
				k->key_len = 8;
				if (parity == k->parity && k->key_ok[k->parity]
						&& (memcmp(cw, k->cw[k->parity], k->key_len) != 0))
					do_queue = 1;
				if (ctime - k->last_parity_change < 5000) // CW received in the first 5s after key change
					do_queue = 0;
				if (do_queue)
				{
					memcpy(k->next_cw[parity], cw, k->key_len);
					queued = "[queued]";
				}
				else
				{
					dvbcsa_bs_key_set(cw, k->key[parity]);
					memcpy(k->cw[parity], cw, k->key_len);
				}
				k->key_ok[parity] = 1;
				k->cw_time[parity] = ctime;
				LOG(
						"dvbapi: received DVBAPI_CA_SET_DESCR, key %d parity %d, index %d, CW: %02X %02X %02X %02X %02X %02X %02X %02X %s",
						k_id, parity, index, cw[0], cw[1], cw[2], cw[3], cw[4],
						cw[5], cw[6], cw[7], queued);

				mutex_unlock(&k->mutex);
				invalidate_adapter(k->adapter);
				p = find_pid(k->adapter, k->pmt_pid);
				if (p)
					p->type |= CLEAN_PMT;

			}
			else
				LOG(
						"dvbapi: invalid DVBAPI_CA_SET_DESCR, key %d parity %d, k %p, index %d, CW: %02X %02X %02X %02X %02X %02X %02X %02X",
						k_id, parity, k, index, cw[0], cw[1], cw[2], cw[3],
						cw[4], cw[5], cw[6], cw[7]);
			break;
		}

		case DVBAPI_ECM_INFO:
		{
			int pos1 = s->rlen - pos;
			SKey *k = get_key(b[4]);
			char cardsystem[255];
			char reader[255];
			char from[255];
			char protocol[255];
			unsigned char len = 0;
			char *msg[5] =
			{ cardsystem, reader, from, protocol, NULL };
			int i = 5, j = 0;

			if (k)
			{
				int64_t e_key = DVBAPI_ITEM + ((1 + k->adapter) << 24) + 2;
				setItem(e_key, b, 1, 0);
				k->ecm_info = getItem(e_key);
				k->cardsystem = k->ecm_info;
				k->reader = k->ecm_info + 256;
				k->from = k->ecm_info + 512;
				k->protocol = k->ecm_info + 768;
				msg[0] = k->cardsystem;
				msg[1] = k->reader;
				msg[2] = k->from;
				msg[3] = k->protocol;

			}

			uint16_t sid;

			copy16r(sid, b, i);
			if (k)
			{
				copy16r(k->caid, b, i + 2);
				copy16r(k->info_pid, b, i + 4);
				copy32r(k->prid, b, i + 6);
				copy32r(k->ecmtime, b, i + 10);
			}
			i += 14;
			while (msg[j] && i < pos1)
			{
				len = b[i++];
				memcpy(msg[j], b + i, len);
				msg[j][len] = 0;
				i += len;
				j++;
			}
			if (i < pos1 && k)
				k->hops = b[i++];
			pos += i;
			LOG(
					"dvbapi: ECM_INFO: key %d, SID = %04X, CAID = %04X (%s), PID = %04X, ProvID = %06X, ECM time = %d ms, reader = %s, from = %s, protocol = %s, hops = %d",
					k ? k->id : -1, sid, k ? k->caid : 0, msg[0],
					k ? k->info_pid : 0, k ? k->prid : 0, k ? k->ecmtime : 0,
					msg[1], msg[2], msg[3], k ? k->hops : 0);
			break;
		}

		default:
		{
			LOG(
					"dvbapi: unknown operation: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
					b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9],
					b[10]);
			pos = s->rlen;

		}
		}
	}
	s->rlen = 0;
	return 0;
}

SKey *get_active_key(SPid *p)
{
	SKey *k;
	adapter *ad;
	int key = p->key;
	uint32_t ctime = getTick();
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

	LOGL(3,
			"get_active_key: searching key for pid %d, starting key %d parity %d ok %d %d",
			p->pid, key, k->parity, k->key_ok[0], k->key_ok[1]);
	while (k && k->enabled)
	{
		if ((k->parity != -1) && k->key_ok[k->parity]
				&& (ctime - k->cw_time[k->parity] > MAX_KEY_TIME)) // key expired
		{
			k->key_ok[k->parity] = 0;
			LOGL(2,
					"get_active_key: Invalidating key %d, parity %d, as the CW expired, pid %d",
					k->id, k->parity, p->pid);
		}

		if ((k->parity != -1) && (ctime - k->cw_time[k->parity] > 5000)
				&& (memcmp(nullcw, k->next_cw[k->parity], k->key_len) != 0))
		{
			char *cw = k->next_cw[k->parity];
			LOGL(2,
					"get_active_key: Setting the queued CW as active CW for key %d, parity %d: %02X %02X %02X ...",
					k->id, k->parity, cw[0], cw[1], cw[2]);
			memcpy(k->cw[k->parity], k->next_cw[k->parity], k->key_len);
			memset(k->next_cw[k->parity], 0, k->key_len);
			dvbcsa_bs_key_set(k->cw[k->parity], k->key[k->parity]);

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

		if (k->next_key < keys || k->next_key > keys + MAX_KEYS)
		{
			LOGL(3, "get_active_key: invalid next_key for key %d: %p", k->id,
					k->next_key);
			k->next_key = NULL;
		}
		k = k->next_key;
		LOGL(3,
				"get_active_key: trying next key for pid %d, key %d parity %d ok %d %d",
				p->pid, k ? k->id : -1, k ? k->parity : -1,
				k ? k->key_ok[0] : -1, k ? k->key_ok[1] : -1);

	}
	LOGL(1, "get_active_key: returning NULL for pid %d, starting key %d",
			p->pid, key);
	return NULL;
}

void set_next_key(int k1, int k2)
{
	SKey *key1, *key2;
	key1 = get_key(k1);
	key2 = get_key(k2);
	if (!key1 || !key2)
		return;
	LOG("Next key %d set to key %d", key1->id, key2->id);
	key1->next_key = key2;
}

void update_pid_key(adapter *ad)
{
	int i = 0;

	int64_t pk_key = DVBAPI_ITEM + ((1 + ad->id) << 24) + 1;
	SKey **pid_to_key = (SKey **) getItem(pk_key);
	if (!pid_to_key || pid_to_key[0]) // cache pid_to_key
	{
		setItem(pk_key, (uint8_t *) &i, 0, -1);
		setItemSize(pk_key, 8192 * sizeof(*pid_to_key));
		pid_to_key = (SKey **) getItem(pk_key);
		memset(pid_to_key, 0, getItemSize(pk_key));

		for (i = 0; i < MAX_PIDS; i++)
			if (ad->pids[i].flags == 1)
				pid_to_key[ad->pids[i].pid] = get_active_key(&ad->pids[i]);
	}

}

int decrypt_batch(SKey *k)
{
	int oldb1 = -1, oldb2 = -1;
	unsigned char *b = NULL, *oldb = NULL;
	int bl, i, pid = 0;
	if (!k)
		LOG_AND_RETURN(-1, "Unable to decrypt k=NULL for key %d", k->id);
	mutex_lock(&k->mutex);
	if (k->blen <= 0 || (k->parity == -1) || (!k->key_ok[k->parity]))
	{
		LOG("Unable to decrypt blen = %d, parity = %d, key_ok %d for key %d",
				k->blen, k->parity, (k->parity >= 0) ? k->key_ok[k->parity] : 0,
				k->id);
		mutex_unlock(&k->mutex);
		return -1;
	}
	b = k->batch[0].data - 4;
	pid = (b[1] & 0x1F) * 256 + b[2];
	k->batch[k->blen].data = NULL;
	k->batch[k->blen].len = 0;
	dvbcsa_bs_decrypt(k->key[k->parity], k->batch, 184);
	LOGL(5,
			"dvbapi: decrypted key %d parity %d at len %d, channel_id %d (pid %d) %p",
			k->id, k->parity, k->blen, k->sid, pid, k->batch[0].data); //0x99
	k->blen = 0;
	memset(k->batch, 0, sizeof(int *) * 128);
	mutex_unlock(&k->mutex);
	return 0;
}

void mark_decryption_failed(unsigned char *b, SKey *k, adapter *ad)
{
	SPid *p;
	int pid;
	if (!ad)
		return;
	pid = (b[1] & 0x1F) * 256 + b[2];
	LOGL(4,
			"NOT DECRYPTING for key %d drop_encrypted=%d parity %d pid %d key_ok %d",
			k ? k->id : -1, opts.drop_encrypted, k ? k->parity : -1, pid,
			k ? k->key_ok[k->parity] : -1);
	ad->dec_err++;
	p = find_pid(ad->id, pid);
	if (p)
		p->dec_err++;

	if (opts.drop_encrypted)
	{
		b[1] |= 0x1F;
		b[2] |= 0xFF;
	}
}

int decrypt_stream(adapter *ad, void *arg)
{
	struct iovec *iov;
	SKey *k = NULL;
	int adapt_len;
	int len;
	// max batch
	int i = 0, j = 0;
	unsigned char *b;
	SPid *p;
	int pid;
	int cp;
	int16_t *pids;
	SKey **pid_to_key;
	int64_t pid_key = TABLES_ITEM + ((1 + ad->id) << 24) + 0;
	int64_t pk_key = DVBAPI_ITEM + ((1 + ad->id) << 24) + 1;
	int rlen = *(int *) arg;

	if (!dvbapi_is_enabled)
		return 0;

	if (!batchSize)
		batch_size();

	pids = (int16_t *) getItem(pid_key);
	update_pid_key(ad);
	pid_to_key = (SKey **) getItem(pk_key);

	for (i = 0; i < rlen; i += 188)
	{
		b = ad->buf + i;
		pid = (b[1] & 0x1F) * 256 + b[2];
		if (b[3] & 0x80)
		{
			if (dvbapi_is_enabled && pid_to_key)
				k = pid_to_key[pid];
			else
				k = NULL;
			if (!k)
			{
				mark_decryption_failed(b, k, ad);
				continue;
			}
			else if (k == POINTER_TYPE_ECM) // it is an ECM - we should get here
				continue;
			else if (k == POINTER_1)
				continue;
			cp = ((b[3] & 0x40) > 0);
			if (k->parity == -1)
				k->parity = cp;
			if ((k->parity != cp) || (k->blen >= batchSize)) // partiy change or batch buffer full
			{
				int old_parity = k->parity;
				decrypt_batch(k);
				if (old_parity != cp)
				{
					int ctime = getTick();
					LOGL(2,
							"Parity change for key %d, new parity %d pid %d [%02X %02X %02X %02X], last_parity_change %d",
							k->id, cp, pid, b[0], b[1], b[2], b[3],
							k->last_parity_change);
//					if(ctime - k->last_parity_change> 1000)
//						k->key_ok[old_parity] = 0;
					k->last_parity_change = ctime;
					k->parity = cp;
					invalidate_adapter(ad->id);
					update_pid_key(ad);
				}

			}

			if (!k->key_ok[cp])
			{
				mark_decryption_failed(b, k, ad);
				continue;
			}
			if (b[3] & 0x20)
			{
				adapt_len = (b[4] < 183) ? b[4] + 5 : 0;
				LOGL(4, "Adaptation for pid %d, specified len %d, final len %d",
						pid, b[4], adapt_len);
			}
			else
				adapt_len = 4;
			k->batch[k->blen].data = b + adapt_len;
			k->batch[k->blen++].len = 188 - adapt_len;
			b[3] &= 0x3F; // clear the encrypted flags
		}
//		else
//			if (pid_to_key && pid_to_key[pid] == POINTER_TYPE_ECM)
//		{
//			send_ecm(b, ad);
//			continue;
//		}

	}

	for (i = 0; i < MAX_KEYS; i++)  // decrypt everything that's left
		if (keys[i].enabled && (keys[i].blen > 0)
				&& (keys[i].adapter == ad->id))
			decrypt_batch(&keys[i]);
//	else 
//		if((parity != -1) && k)LOGL(5, "NOT DECRYPTING sid %d, parity %d, %d %d, ctime %d last_key %d", sid->sid, parity, key_ok[parity], j, ctime, k->last_key[parity]);

}

int dvbapi_send_pmt(SKey *k)
{
	unsigned char buf[1500];
	int len;

	LOG(
			"Sending pmt to dvbapi server for pid %d, Channel ID %04X, key %d, using socket %d",
			k->pmt_pid, k->sid, k->id, sock);

	copy32(buf, 0, AOT_CA_PMT);
	buf[6] = CAPMT_LIST_UPDATE;
	copy16(buf, 7, k->sid);
	buf[9] = 1;

	copy32(buf, 12, 0x01820200);
	buf[15] = k->id;
	buf[16] = k->id;
	memcpy(buf + 17, k->pi, k->pi_len + 2);
	len = 17 - 6 + k->pi_len + 2;
	copy16(buf, 4, len);
	copy16(buf, 10, len - 11);
	TEST_WRITE(write(sock, buf, len + 6));
	return 0;
}

int dvbapi_close(sockets * s)
{
	int i;
	LOG("requested dvbapi close for sock %d, sock_id %d", sock, dvbapi_sock,
			s->sock);
	sock = 0;
	dvbapi_is_enabled = 0;
	unregister_dvbapi();
	SKey *k;
	for (i = 0; i < MAX_KEYS; i++)
		if (keys[i].enabled)
		{
			k = get_key(i);
			if (!k)
				continue;
//			reset_pids_type(k->adapter);
			keys_del(i);
		}
	unregister_dvbapi();
	return 0;
}

int connect_dvbapi(void *arg)
{
	int ctime;

	ctime = getTick();
	if ((sock > 0) && dvbapi_is_enabled)  // already connected
		return sock;

	dvbapi_is_enabled = 0;

	if (!opts.dvbapi_port || !opts.dvbapi_host)
		return 0;

	if (sock <= 0)
	{
		int err;
		sock = tcp_connect(opts.dvbapi_host, opts.dvbapi_port, NULL, 1);
		dvbapi_sock = sockets_add(sock, NULL, -1, TYPE_TCP | TYPE_CONNECT,
				(socket_action) dvbapi_reply, (socket_action) dvbapi_close,
				NULL);
		set_socket_buffer(dvbapi_sock, read_buffer, sizeof(read_buffer));

		return 0;
	}
	return 0;
}

int poller_sock;
void init_dvbapi()
{
	poller_sock = sockets_add(SOCK_TIMEOUT, NULL, -1, TYPE_UDP,
	NULL, NULL, (socket_action) connect_dvbapi);
	sockets_timeout(poller_sock, 5000); // try to connect every 5s
	set_sockets_rtime(poller_sock, -5 * 1000);

}

void send_client_info(sockets *s)
{
	unsigned char buf[1000];
	unsigned char len;
	copy32(buf, 0, DVBAPI_CLIENT_INFO);
	copy16(buf, 4, dvbapi_protocol_version);
	len = sprintf(buf + 7, "%s/%s", app_name, version);
	buf[6] = len;
	dvbapi_is_enabled = 1;
	TEST_WRITE(write(s->sock, buf, len + 7));
}

int send_ecm(adapter *ad, void *arg)
{
	unsigned char buf[1500];
	unsigned char *b = (unsigned char *) arg;
	SPid *p;
	SKey *k = NULL;
	int pid, len = 0;
	int filter, demux;
	int old_parity;

	if (!dvbapi_is_enabled)
		return 0;

	if (b[0] != 0x47)
		return 0;
	if ((b[1] & 0x40) && ((b[4] != 0) || ((b[5] & 0xFE) != 0x80)))
		return 0;

	pid = (b[1] & 0x1F) * 256 + b[2];
	p = find_pid(ad->id, pid);
	if (p)
		k = get_key(p->key);
	if (!k)
//		LOG_AND_RETURN(0, "key is null pid %d for p->key %d", pid, p?p->key:-1);
		return 0;

	demux = k->demux;
	filter = p->filter;

	if (!(len = assemble_packet(&b, ad, 0)))
		return 0;

	if ((b[0] & 1) == p->ecm_parity)
		return 0;
	old_parity = p->ecm_parity;
	p->ecm_parity = b[0] & 1;

	len = ((b[1] & 0xF) << 8) + b[2];
	len += 3;
	LOG(
			"dvbapi: sending ECM key %d for pid %04X (%d), ecm_parity = %d, new parity %d, demux = %d, filter = %d, len = %d [%02X %02X %02X %02X]",
			k->id, pid, pid, old_parity, b[0] & 1, demux, filter, len, b[0],
			b[1], b[2], b[3]);

	if (demux < 0)
		return 0;

	if (len > 559 + 3)
		return -1;

	copy32(buf, 0, DVBAPI_FILTER_DATA);
	buf[4] = demux;
	buf[5] = filter;
	// filter id
	memcpy(buf + 6, b, len + 6);
	TEST_WRITE(write(sock, buf, len + 6));
}

void *create_key()
{
	return (void *) dvbcsa_bs_key_alloc();
}

void free_key(void *key)
{
	dvbcsa_key_free(key);
}

int batch_size() // make sure the number is divisible by 7
{
	batchSize = dvbcsa_bs_batch_size();
//	batchSize = (batchSize / 7) * 7;
	return batchSize;
}

int keys_add(int adapter, int sid, int pmt_pid)
{
	int i;
	SKey *k;
	for (i = 0; i < MAX_KEYS; i++)
	{
		mutex_lock(&k->mutex);		
		if (!keys[i].enabled)
			break;
		mutex_unlock(&k->mutex);

	}
	if (i == MAX_KEYS)
		LOG_AND_RETURN(-1, "Key buffer is full, could not add new keys");
	k = &keys[i];
//	mutex_init(&k->mutex, &k->mutex0);
	if (!k->key[0])
		k->key[0] = dvbcsa_bs_key_alloc();
	if (!k->key[1])
		k->key[1] = dvbcsa_bs_key_alloc();

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
	memset(k->cw[0], 0, 16);
	memset(k->cw[1], 0, 16);
	memset(k->next_cw[0], 0, 16);
	memset(k->next_cw[1], 0, 16);
	mutex_unlock(&k->mutex);
	invalidate_adapter(adapter);
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
	if ((i < 0) || (i >= MAX_KEYS) || (!keys[i].enabled))
		return 0;
	k = &keys[i];
	mutex_lock(&k->mutex);
	aid = k->adapter;
	k->enabled = 0;
//	buf[7] = k->demux;
	buf[7] = i;
	LOG("Stopping DEMUX %d, removing key %d, sock %d, pmt pid %d", buf[7], i,
			sock, k->pmt_pid);
	if ((buf[7] != 255) && (sock > 0))
		TEST_WRITE(write(sock, buf, sizeof(buf)));
	mutex_lock(&k->mutex);
	k->sid = 0;
	k->pmt_pid = 0;
	k->adapter = -1;
	k->demux = -1;
	reset_pids_type_for_key(aid, i);
	if (k->next_key)
		keys_del(k->next_key->id);
	k->next_key = NULL;
	invalidate_adapter(aid);
	ek = 0;
	delItemP(k->ecm_info);
	k->ecm_info = k->cardsystem = k->reader = NULL;
	k->from = k->protocol = NULL;
	k->hops = k->caid = k->info_pid = k->prid = k->ecmtime = 0;
	buf[7] = 0xFF;
	for (j = 0; j < MAX_KEYS; j++)
		if (keys[j].enabled)
			ek++;
//	if(!ek && sock>0)
//		TEST_WRITE(write(sock, buf, sizeof(buf)));
	mutex_unlock(&k->mutex);
	mutex_destroy(&k->mutex);
	return 0;
}

SKey *get_key(int i)
{
	if (i < 0 || i >= MAX_KEYS || !keys[i].enabled)
		return NULL;
	return &keys[i];
}

void dvbapi_add_pid(adapter *ad, void *arg)
{
	invalidate_adapter(ad->id);
}

void dvbapi_del_pid(adapter *ad, void *arg)
{
	invalidate_adapter(ad->id);
}

void dvbapi_add_pmt(adapter *ad, void *arg)
{
	SPMT *spmt = (SPMT *) arg;
	SKey *k;
	SPid *p;
	int key, pid = spmt->pid;
	p = find_pid(ad->id, pid);
	if (!p)
		return;

	key = p->key;
	if (!get_key(p->key))
		key = keys_add(ad->id, spmt->program_id, pid);
	k = get_key(key);
	k->pi_len = spmt->pi_len;
	k->pi = spmt->pi;
	dvbapi_send_pmt(k);
	p->key = key;
	set_next_key(p->key, spmt->old_pmt);

}
void dvbapi_del_pmt(adapter *ad, void *arg)
{
	int pid = *(int *) arg;
	SPid *p = find_pid(ad->id, pid);
	keys_del(p->key);
}

SCA dvbapi;

void register_dvbapi()
{
	dvbapi.enabled = 1; // ignore it anyway
	memset(dvbapi.action, 0, sizeof(dvbapi.action));
	dvbapi.action[CA_ADD_PID] = (ca_action) &dvbapi_add_pid;
	dvbapi.action[CA_DEL_PID] = (ca_action) &dvbapi_del_pid;
	dvbapi.action[CA_ADD_PMT] = (ca_action) &dvbapi_add_pmt;
	dvbapi.action[CA_DEL_PMT] = (ca_action) &dvbapi_del_pmt;
	dvbapi.action[CA_ECM] = &send_ecm;
	dvbapi.action[CA_TS] = &decrypt_stream;
	add_ca(&dvbapi);
}

void unregister_dvbapi()
{
	del_ca(&dvbapi);
}

void dvbapi_delete_keys_for_adapter(int aid)
{
	int i;
	for (i = 0; i < MAX_KEYS; i++)
		if (keys[i].enabled && keys[i].adapter == aid)
			keys_del(i);
}

_symbols dvbapi_sym[] =
{
		{ "key_enabled", VAR_ARRAY_INT8, &keys[0].enabled, 1, MAX_KEYS,
				sizeof(keys[0]) },
		{ "key_hops", VAR_ARRAY_INT8, &keys[0].hops, 1, MAX_KEYS,
				sizeof(keys[0]) },
		{ "key_ecmtime", VAR_ARRAY_INT, &keys[0].ecmtime, 1, MAX_KEYS,
				sizeof(keys[0]) },
		{ "key_pmt", VAR_ARRAY_INT, &keys[0].pmt_pid, 1, MAX_KEYS,
				sizeof(keys[0]) },
		{ "key_cardsystem", VAR_ARRAY_PSTRING, &keys[0].cardsystem, 1, MAX_KEYS,
				sizeof(keys[0]) },
		{ "key_reader", VAR_ARRAY_PSTRING, &keys[0].reader, 1, MAX_KEYS,
				sizeof(keys[0]) },
		{ "key_from", VAR_ARRAY_PSTRING, &keys[0].from, 1, MAX_KEYS,
				sizeof(keys[0]) },
		{ "key_protocol", VAR_ARRAY_PSTRING, &keys[0].protocol, 1, MAX_KEYS,
				sizeof(keys[0]) },

		{ NULL, 0, NULL, 0, 0 } };

