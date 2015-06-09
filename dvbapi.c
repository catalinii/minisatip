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
#include "search.h"
#include "tables.h"


extern struct struct_opts opts;
const int64_t DVBAPI_ITEM = 0x1000000000000;
int dvbapi_sock = -1;
int sock;
int dvbapi_is_enabled = 0;
int haveDvbapi = 0;
int batchSize;
SKey keys[MAX_KEYS];
unsigned char read_buffer[1500];


#define TEST_WRITE(a) if((a)<=0){LOG("write to dvbapi socket failed, closing socket %d",sock);sockets_del(dvbapi_sock);sock = 0;dvbapi_sock = -1;dvbapi_is_enabled = 0;}
#define POINTER_TYPE_ECM ((void *)-TYPE_ECM)
#define POINTER_1 ((void *)1)
int have_dvbapi()
{
	return haveDvbapi;
}

int dvbapi_enabled()
{
	return dvbapi_is_enabled;
}

void invalidate_adapter(int ad)
{
	int64_t pk_key = DVBAPI_ITEM + ((1+ ad) << 24) + 1;
	SKey **pid_to_key = (SKey **)getItem(pk_key);				
	if(pid_to_key)
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
	unsigned int op, version, _pid;
	int k_id, a_id = 0, s_id, pos = 0;
	int demux, filter;
	SPid *p;
	if(s->rlen == 0)
	{
		send_client_info(s);	
		return 0;
	}
	while(pos < s->rlen)
	{
		int op1;
		b = s->buf + pos;
		copy32r(op, b, 0);
		op1 = op & 0xFFFFFF;
		change_endianness = 0;
		if(op1 == CA_SET_DESCR_X || op1 == CA_SET_DESCR_AES_X || op1 == CA_SET_PID_X || op1 == DMX_STOP_X || op1 == DMX_SET_FILTER_X)
		{ // change endianness
			op = 0x40000000 | ((op1 & 0xFF) << 16) | (op1 & 0xFF00) | ((op1 & 0xFF0000) >> 16);
			if(!(op & 0xFF0000 ))
				op &= 0xFFFFFF;
			LOG("Changing endianness from %06X to %08X -> %02X %02X %02X %02X %02X %02X %02X %02X %02X", op1, op, b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8]);
			//b ++;
			//pos ++;
			change_endianness = 1;
		} 
		LOGL(3, "dvbapi read from socket %d the following data (%d bytes), pos = %d, op %08X, adapter %d", s->sock, s->rlen, pos, op, b[4]);
		switch(op){
		
		case DVBAPI_SERVER_INFO:
		
			if(s->rlen < 6)
				return;
			dvbapi_copy16r(version, b, 4);
			LOG("DVBAPI server version %d found, name = %s", version, b+7);
			pos = 6 + strlen(b+6) + 1;
			break;
		
		case DVBAPI_DMX_SET_FILTER:
		{
			int data;
			int i = 0, k_id;
			SKey *k;
			pos += 65;
			if(s->rlen < 9)
				return;
			dvbapi_copy16r(_pid, b, 7);
			_pid &= 0x1FFF;
			k_id = b[4];
			k = get_key(k_id);
			a_id = -1;
			if(k)
				a_id = k->adapter;			
	
			demux = b[5];
			filter = b[6];
			LOG("DVBAPI requested set filter for pid %04X (%d), key %d, demux %d, filter %d", _pid, _pid, k_id, demux, filter);
			if(!(p = find_pid(a_id, _pid)))
			{
				mark_pid_add(-1, a_id, _pid);
				update_pids(a_id);		
				p = find_pid(a_id, _pid);
			}	
			if(!p)
				break;	
//			if(k && p->key != k_id)
			if(p->filter == 255)
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
			int ad;
			k_id = b[4];
			pos += 9;
			k = get_key(k_id);
			if(!k)
				break;
			a_id = k->adapter;
			dvbapi_copy16r(_pid, b, 7 )
			_pid &= 0x1FFF;
			LOG("Received from DVBAPI server DMX_STOP for key %d, adapter %d, pid %X (%d)", k_id, a_id, _pid, _pid);
			if((p=find_pid(a_id, _pid)) && (p->key == k_id))
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

			pos += 21;
			k_id = b[4];
			dvbapi_copy32r(index, b, 5);
			dvbapi_copy32r(parity, b, 9);			
			cw = b + 13;			
			k = get_key(k_id);
			if(k && (parity < 2))
			{
				LOG("received DVBAPI_CA_SET_DESCR, key %d parity %d, index %d, CW: %02X %02X %02X %02X %02X %02X %02X %02X", k_id, parity, index, cw[0], cw[1], cw[2], cw[3], cw[4], cw[5], cw[6], cw[7]);			
				dvbcsa_bs_key_set(cw, k->key[parity]);
				k->key_ok[parity] = 1;
				invalidate_adapter(k->adapter);
				p = find_pid(k->adapter, k->pmt_pid);
				if(p)
					p->type |= CLEAN_PMT;
				
			} else   LOG("invalid DVBAPI_CA_SET_DESCR, key %d parity %d, k %p, index %d, CW: %02X %02X %02X %02X %02X %02X %02X %02X", k_id, parity, k, index, cw[0], cw[1], cw[2], cw[3], cw[4], cw[5], cw[6], cw[7]);
			break; 
		}
		default: pos = s->rlen;
		}
	}
	s->rlen = 0;
	return 0;
}


SKey *get_active_key(SPid *p)
{
	SKey *k;
	int key = p->key;	
	if(key==255)
		return NULL;
	if(p->type & TYPE_ECM)
		return POINTER_TYPE_ECM;
	if(p->type & TYPE_PMT)
		return NULL;
	k = get_key(key);
	if(k==NULL)
		return NULL;
	LOGL(3, "get_active_key: searching key for pid %d, starting key %d parity %d ok %d %d", p->pid, key, k->parity, k->key_ok[0], k->key_ok[1]);
	while(k && k->enabled)
	{
		if(k->key_ok[0] || k->key_ok[1])
		{
			LOGL(1, "get_active_key: returning key %d for pid %d", k->id, p->pid);
			return k; 
		}
		if(k->next_key<keys || k->next_key>keys+MAX_KEYS)
		{
			LOG("get_active_key: invalid next_key for key %d: %p", k->id, k->next_key);
			k->next_key = NULL;
		}
		k = k->next_key;
		LOGL(3, "get_active_key: trying next key for pid %d, key %d parity %d ok %d %d", p->pid,k?k->id:-1, k?k->parity:-1, k?k->key_ok[0]:-1, k?k->key_ok[1]:-1);

	}
	LOGL(1, "get_active_key: returning NULL for pid %d, starting key %d", p->pid, key);
	return NULL;
}

void set_next_key(int k1, int k2)
{
	SKey *key1, *key2;
	key1 = get_key(k1);
	key2 = get_key(k2);
	if(!key1 || !key2)
		return;
	LOG("Next key %d set to key %d", key1->id, key2->id);
	key1->next_key = key2;
}

int decrypt_batch(SKey *k)
{
	int oldb1 = -1, oldb2 = -1;
	unsigned char *b = NULL, *oldb = NULL;
	int bl, i, pid = 0;
	if(!k)
		LOG_AND_RETURN(-1, "Unable to decrypt k=NULL for key %d", k->id);
	if(k->blen<=0 || (k->parity==-1) || (!k->key_ok[k->parity]))
		LOG_AND_RETURN(-1, "Unable to decrypt blen = %d, parity = %d, key_ok %d for key %d", 
			k->blen, k->parity, (k->parity>=0)?k->key_ok[k->parity]:0, k->id);
	b = k->batch[0].data - 4;
	pid = (b[1] & 0x1F)*256 + b[2];
	k->batch[k->blen].data = NULL;
	k->batch[k->blen].len = 0;	
	dvbcsa_bs_decrypt(k->key[k->parity], k->batch, 184);
	LOGL(5, "dvbapi: decrypted key %d parity %d at len %d, channel_id %d (pid %d) %p", 
		k->id, k->parity, k->blen, k->sid, pid, k->batch[0].data); //0x99
	k->blen = 0;
	k->parity = -1;
	memset(k->batch, 0, sizeof(int *)*128);
	return 0;
}

void mark_decryption_failed(unsigned char *b, SKey *k, adapter *ad)
{
	SPid *p;
	if(!ad)
		return;
	LOGL(4, "NOT DECRYPTING for key %d drop_encrypted=%d parity %d pid %d key_ok %d", k?k->id:-1, opts.drop_encrypted, k?k->parity:-1, 
		(b[1] & 0x1F)*256 + b[2], k?k->key_ok[k->parity]:-1);
	if(opts.drop_encrypted)
	{
		b[1] |= 0x1F;
		b[2] |= 0xFF; 
	}
}


int decrypt_stream(adapter *ad,int rlen)
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
	int64_t pid_key = TABLES_ITEM + ((1+ ad->id) << 24) + 0;
	int64_t pk_key = DVBAPI_ITEM + ((1+ ad->id) << 24) + 1;
	
	
	if(!have_dvbapi())
		return 0;
	if(!dvbapi_is_enabled)
	{
		init_dvbapi();
//		if(!dvbapi_is_enabled)
//			return 0;
	}	
	if(!batchSize)
		batch_size();
	
	pids = (int16_t *)getItem(pid_key);
	pid_to_key = (SKey **)getItem(pk_key);
	if(!pid_to_key || pid_to_key[0]) // cache pid_to_key
	{
		setItem(pk_key, b, 0, -1);
		setItemSize(pk_key, 8192*sizeof(*pid_to_key));
		pid_to_key = (SKey **)getItem(pk_key);
		memset(pid_to_key, 0, getItemSize(pk_key));
	
		for(i=0;i<MAX_PIDS;i++)
			if(ad->pids[i].flags == 1)
				pid_to_key[ad->pids[i].pid]=get_active_key(&ad->pids[i]);
	}

	for(i=0;i<rlen;i+=188)
	{
		b = ad->buf + i;
		pid = (b[1] & 0x1F)*256 + b[2]; 			
		if(b[3] & 0x80)
		{				
			if(dvbapi_is_enabled && pid_to_key)
				k = pid_to_key[pid];
			else k = NULL;
			if(!k)
			{
				mark_decryption_failed(b, k, ad);
				continue;
			} else if(k == POINTER_TYPE_ECM) // it is an ECM - we should get here
				continue;
			else if (k == POINTER_1)
				continue;
			cp = ((b[3] & 0x40) > 0);			
			if(k->parity == -1)
				k->parity = cp;
			if((k->parity != cp) || (k->blen >=batchSize))  // partiy change or batch buffer full
			{
				int old_parity = k->parity;
				decrypt_batch(k);
				if(old_parity != cp)
				{
					int ctime = getTick();
					LOGL(2, "Parity change for key %d, old parity %d, new parity %d pid %d [%02X %02X %02X %02X], last_parity_change %d", k->id, old_parity, cp, pid, b[0], b[1], b[2], b[3], k->last_parity_change);
					if(ctime - k->last_parity_change> 1000)
						k->key_ok[old_parity] = 0;
					k->last_parity_change = ctime;
					invalidate_adapter(ad->id);
				}
				k->parity = cp;
			}
			
			if(!k->key_ok[cp])
			{
				mark_decryption_failed(b, k, ad);
				continue;
			}
			if(b[3] & 0x20)
			{
				adapt_len = (b[4]<183)?b[4]+5:0;
				LOGL(4, "Adaptation for pid %d, specified len %d, final len %d", pid, b[4], adapt_len);
			}	
			else adapt_len = 4;
			k->batch[k->blen].data = b + adapt_len;
			k->batch[k->blen++].len = 188 - adapt_len;
			b[3] &= 0x3F; // clear the encrypted flags
		}
		else if (pid_to_key && pid_to_key[pid]==POINTER_TYPE_ECM)
		{
			send_ecm(b,ad);
			continue;
		}

	}
	
	for(i=0; i < MAX_KEYS; i++)  // decrypt everything that's left
		if(keys[i].enabled && (keys[i].blen > 0))
			decrypt_batch(&keys[i]);
//	else 
//		if((parity != -1) && k)LOGL(5, "NOT DECRYPTING sid %d, parity %d, %d %d, ctime %d last_key %d", sid->sid, parity, key_ok[parity], j, ctime, k->last_key[parity]);

}

int dvbapi_send_pi(SKey *k)
{
	unsigned char buf[1500];
	int len;
	
	LOG("Sending pmt to dvbapi server for pid %d, Channel ID %04X, key %d, using socket %d", k->pmt_pid, k->sid, k->id, sock);


	copy32(buf, 0, AOT_CA_PMT);
	buf[6] = CAPMT_LIST_ONLY;
	copy16(buf, 7, k->sid);
	buf[9] = 1;
	
	copy32(buf, 12, 0x01820200);
	buf[15] = k->id;
	buf[16] = k->id;
	memcpy(buf + 17, k->pi, k->pi_len + 2);
	len = 17  - 6 + k->pi_len + 2;
	copy16(buf, 4, len);
	copy16(buf, 10, len - 11);
	TEST_WRITE(write(sock, buf, len + 6 ));
}
static int last_retry = -5000;

int dvbapi_close(sockets * s)
{
	int i;
	LOG("requested dvbapi close for sock %d, sock_id %d", sock, dvbapi_sock, s->sock);
	sock = 0;
	dvbapi_is_enabled = 0;
	last_retry = getTick();
	SKey *k;
	for(i=0;i<MAX_KEYS;i++)
		if(keys[i].enabled)
		{
			k = get_key(i);
			if(!k) 
				continue;
			reset_pids_type(k->adapter);
			keys_del(i);
		}
	return 0;
}
	
int init_dvbapi()
{
	int ctime;

	ctime = getTick();
	if((sock>0) && dvbapi_is_enabled)  // already connected
			return sock;
	
	if(ctime - last_retry < 5000) // wait 5 seconds before trying again
		return 0;
		
	dvbapi_is_enabled = 0;
		
	haveDvbapi = 0;
	if(!opts.dvbapi_port  || !opts.dvbapi_host)
		return 0;
		
	haveDvbapi = 1;

	if((sock<=0) && (ctime - last_retry < 5000)) // retry every 5s
		return 0;
		
	if(sock<= 0)
	{
		int err;
		last_retry = ctime;
		sock = tcp_connect(opts.dvbapi_host, opts.dvbapi_port, NULL, 1);
		dvbapi_sock = sockets_add (sock, NULL, -1, TYPE_TCP | TYPE_CONNECT, (socket_action) dvbapi_reply, (socket_action) dvbapi_close, NULL);
		set_socket_buffer (dvbapi_sock , read_buffer, sizeof(read_buffer));
		
		return 0;
	}
	return 0;
}
void send_client_info(sockets *s)
{	
	unsigned char buf[1000];
	unsigned char len;
	copy32(buf, 0, DVBAPI_CLIENT_INFO);
	copy16(buf, 4, DVBAPI_PROTOCOL_VERSION);
	len = sprintf(buf + 7, "minisatip/%s", VERSION);
	buf[6] = len;
	dvbapi_is_enabled = 1;
	TEST_WRITE(write(s->sock, buf, len+7));	
}

int send_ecm(unsigned char *b, adapter *ad)
{
	unsigned char buf[1500];
	SPid *p;
	SKey *k = NULL;
	int pid, len = 0;
	int filter, demux;
	int old_parity;
	
	if(!dvbapi_is_enabled)
		return 0;
	
	pid = (b[1] & 0x1F)* 256 + b[2];
	p = find_pid(ad->id, pid);
	if(p)
		k = get_key(p->key);
	if(!k)
//		LOG_AND_RETURN(0, "key is null pid %d for p->key %d", pid, p?p->key:-1);
		return 0;
		
	demux = k->demux;
	filter = p->filter;	
	
	if(!(len = assemble_packet(&b,ad)))
		return 0;
	
	if((b[0] & 1) == p->ecm_parity)
		return 0;
	old_parity = p->ecm_parity;
	p->ecm_parity = b[0] & 1;	

	len = ((b[1] & 0xF) << 8) + b[2];
	len += 3;	
	LOG("Sending DVBAPI_FILTER_DATA key %d for pid %04X (%d), ecm_parity = %d, new parity %d, demux = %d, filter = %d, len = %d [%02X %02X %02X %02X]", k->id, pid, pid, old_parity, b[0] & 1,demux, filter, len, b[0], b[1], b[2], b[3]);
	
	if(demux<0)
		return 0;
	
	if(len>559+3)
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
	return (void *)dvbcsa_bs_key_alloc();
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
	for(i=0;i<MAX_KEYS;i++)
		if(!keys[i].enabled)
			break;
	if(i==MAX_KEYS)
		LOG_AND_RETURN(-1,"Key buffer is full, could not add new keys");
	
	if(!keys[i].key[0])
		keys[i].key[0] = dvbcsa_bs_key_alloc();
	if(!keys[i].key[1])
		keys[i].key[1] = dvbcsa_bs_key_alloc();
	
	if(!keys[i].key[0] || !keys[i].key[1])
		LOG_AND_RETURN(-1,"Count not allocate dvbcsa key, keys_add failed");
	keys[i].parity = -1;
	keys[i].key_ok[0] = keys[i].key_ok[1] = 0;
	keys[i].sid = sid;
	keys[i].pmt_pid = pmt_pid;
	keys[i].adapter = adapter;
	keys[i].demux = -1;
	keys[i].id = i;
	keys[i].blen = 0;
	keys[i].enabled = 1;
	keys[i].enabled_channels = 0;
	keys[i].ver = -1;
	keys[i].next_key = NULL;
	invalidate_adapter(adapter);
	LOG("returning new key %d for adapter %d, pmt pid %d sid %04X", i, adapter, pmt_pid, sid);
	
	return i;
}

int keys_del(int i)
{
	int aid, j, ek;
	unsigned char buf[8]={0x9F,0x80,0x3f,4,0x83,2,0,0};
	if((i<0) || (i>=MAX_KEYS) || (!keys[i].enabled))
		return 0;
	aid = keys[i].adapter;
	keys[i].enabled = 0;
//	buf[7] = keys[i].demux;
	buf[7] = i;
	LOG("Stopping DEMUX %d, removing key %d, sock %d, pmt pid %d", buf[7], i, sock, keys[i].pmt_pid);
	if((buf[7] != 255) && (sock>0))
		TEST_WRITE(write(sock, buf, sizeof(buf)));
	keys[i].sid = 0;
	keys[i].pmt_pid = 0;
	keys[i].adapter = -1;
	keys[i].demux = -1;
	keys[i].enabled_channels = 0;
	reset_pids_type_for_key(aid, i);
	if(keys[i].next_key)
		keys_del(keys[i].next_key->id);
	keys[i].next_key = NULL;
	invalidate_adapter(aid);
	ek = 0;
	buf[7] = 0xFF;
	for(j = 0;j < MAX_KEYS; j++)
		if(keys[j].enabled)
			ek++;
//	if(!ek && sock>0)
//		TEST_WRITE(write(sock, buf, sizeof(buf)));
}

SKey *get_key(int i)
{
	if(i<0 || i>=MAX_KEYS || !keys[i].enabled)
		return NULL;
	return &keys[i];
}

void dvbapi_pid_add(adapter *ad,int pid, SPid *cp)
{
	int64_t pid_key = TABLES_ITEM + ((1+ ad->id) << 24) + 0;
	int16_t *pids = NULL;
	SKey *k;
	SPid *p;
	if(!ad)
		return;
	invalidate_adapter(ad->id);
	pids = (int16_t *)getItem(pid_key);
	if(pid==0)
	{
		return;
	}
	if(pids && pids[pid]<=-TYPE_PMT)
	{
		p = find_pid(ad->id, pid);
		if(p && p->type == 0)
		{
			LOG("Adding PMT pid %d to list of pids for adapter %d", pid, ad->id);
			p->type = TYPE_PMT;
		}
		return;
	}
		
	if(pids && (pids[pid]>0))
	{
		p = find_pid(ad->id, pids[pid]);
		LOGL(2, "dvbapi_pid_add: adding pid %d adapter %d, pmt pid %d, pmt pid type %d, pmt pid key %d", pid, ad->id, pids[pid], p?p->type:-1, p?p->key:-1 );
		if(p && (p->type & PMT_COMPLETE))
		{
			cp->key =  p->key;
			if(k=get_key(cp->key))
				k->enabled_channels++;
			return;
		}	
		cp->key = 255;

		if(!p || (p->type == 0))
		{
			int i, next_pmt;
			LOG("Detected pid %d adapter %d without the PMT added to the list of pids",pid, ad->id);
			for(i=pids[pid];i != -TYPE_PMT; )
			{
				next_pmt = abs(i);
				LOG("Adding PMT pid %d to the list of pids, next pid %d", next_pmt, -pids[next_pmt]);
				mark_pid_add(-1, ad->id, next_pmt);
				p = find_pid(ad->id, next_pmt);
				if(!p)
					continue;
				p->type |= TYPE_PMT;
				i=pids[next_pmt];
				pids[next_pmt] = -TYPE_PMT;
			}
		}
	}
}

void dvbapi_pid_del(adapter *ad,int pid, SPid *cp)
{
	int64_t pid_key = TABLES_ITEM + ((1+ ad->id) << 24) + 0;
	int16_t *pids = NULL;
	SPid *p;
	SKey *k;
	if(!ad)
		return;
	invalidate_adapter(ad->id);
	pids = (int16_t *)getItem(pid_key);
	k = get_key(cp->key);
	if(cp->key != 255)
		LOGL(2, "Requested delete pid %d adapter %d key %d pids %d enabled_channels %d", pid, ad->id, cp->key, pids?pids[pid]:-1, k?k->enabled_channels:-1);
	if(cp->type & TYPE_PMT) // if(pids && (pids[pid]< 0))
	{
		if(k && k->enabled_channels == 0)
			keys_del(cp->key);
		return;
	}
	if(k && (cp->type == 0))
	{
		if(k->enabled_channels>0)
			k->enabled_channels --;
		if(k->enabled_channels == 0)
			keys_del(k->id);
	}
}
unsigned char *find_pi(unsigned char *es, int len, int *pi_len)
{
	
	int es_len,caid, capid;
	int i;
	int start_pi = -1;
	*pi_len = 0;
	
        for(i = 0; i < len; i+= es_len) // reading program info
        {
		es_len = es[i+1] + 2;
		if((es[i]==9) && (start_pi==-1))
			start_pi = i;
		if((es[i]!=9) && (start_pi != -1))
			break;
		if(start_pi>=0)
		{
			caid = es[i+2] * 256 + es[i+3];
			capid = (es[i+4] & 0x1F) *256 + es[i+5];
			LOG("PI pos %d caid %04X => pid %04X (%d)", i, caid, capid, capid);
		}
        }
	if(start_pi == -1)
		return NULL;
	LOG("start_pi: %d %d es[0]-> %d", start_pi, i, es[0]);
	*pi_len = i - start_pi;
	return es + start_pi;
} 

int dvbapi_process_pmt(unsigned char *b, adapter *ad)
{
	int pi_len = 0, ver, pmt_len = 0, i, _pid, es_len, len;
	int program_id = 0;
	int prio = 0;
	unsigned char *pmt, *pi;
	int caid, capid, pid, spid, stype;
	SPid *p, *cp;
	SKey *k = NULL;
	int64_t pid_key = TABLES_ITEM + ((1+ ad->id) << 24) + 0;
	int16_t *pids;
	int old_pmt;

	if(!dvbapi_is_enabled)
		return 0;

	if((b[0]!=0x47)) // make sure we are dealing with TS
		return 0;
	
	pid = (b[1] & 0x1F)* 256 + b[2];
	if(!(p = find_pid(ad->id, pid)))
		return -1;
		
	if(p->sid[0] != -1) // the PMT pid is requested by the stream
		prio = 1;

	k = get_key(p->key);
	if(k && (b[5]==2))
	{
		ver = b[10] & 0x3F;
		program_id = b[8]* 256 + b[9];
		if(k->ver != ver || k->program_id != program_id) // pmt version changed
		{ 
//			keys_del(p->key); 
//			p->key = 255;
//			p->type = TYPE_PMT;
		}
	}
	
	if(!p || (p->type & PMT_COMPLETE) || (p->type == 0))
		return 0;
	
	p->type |= TYPE_PMT;
	
	if((b[5] == 2) && ((b[1] &0x40) == 0x40))
		pmt_len = ((b[6] & 0xF) << 8) + b[7];
	
	if(p && k)
	{
		program_id = b[8]* 256 + b[9];
		if((pmt_len > 0) && (program_id == k->sid))
				p->type |= PMT_COMPLETE;
		LOG_AND_RETURN(0, "Trying to add existing key %d for PMT pid %d sid %d k->sid %d", p->key, pid, program_id, k->sid);
	}

	if(!(pmt_len = assemble_packet(&b,ad)))
		return 0;

	program_id = b[3]* 256 + b[4];
	pi_len = ((b[10] & 0xF) << 8) + b[11];

	LOG("PMT pid: %04X (%d), pmt_len %d, pi_len %d, channel id %04X (%d)", pid, pid, pmt_len, pi_len, program_id, program_id);
	pi = b + 12;
	pmt = pi + pi_len;	
	
	if(pmt_len > 1500)
		return 0;

	if(pi_len > pmt_len)
		pi_len = 0;

	if(pi_len > 0) 
		pi=find_pi(pi, pi_len, &pi_len);

	es_len = 0;
	pids = (int16_t *)getItem(pid_key);
	if(!pids)
		return 0;
	pids[pid] = -TYPE_PMT;
	for(i = 0; i < pmt_len - pi_len - 17; i+= (es_len) + 5) // reading streams
	{
		es_len = (pmt[i+3] & 0xF)*256 + pmt[i+4];
		stype = pmt[i];
		spid = (pmt[i+1] & 0x1F)*256 + pmt[i+2];		
		LOG("PMT pid %d - stream pid %04X (%d), type %d, es_len %d, pos %d, pi_len %d old pmt %d, old pmt for this pid %d", pid, spid, spid, stype, es_len, i, pi_len, pids[pid], pids[spid]);
		if((es_len + i>pmt_len) || (es_len == 0))
			break;
		if(stype != 2 && stype != 3 && stype != 4 && stype != 6 && stype != 27 || spid < 64)
			continue;
		if(!pi_len)
			pi = find_pi(pmt + i + 5, es_len, &pi_len);
		
		if(pi_len == 0)
			continue;
			
		old_pmt = pids[spid];
		pids[spid] = pid; 
		if((old_pmt > 0) && (abs(old_pmt) != abs(pid)))  // this pid is associated with another PMT - link this PMT with the old one (if not linked already)
		{
			if(pids[pid]==-TYPE_PMT)
			{
				pids[pid] = -old_pmt;
				LOG("Linking PMT pid %d with PMT pid %d for pid %d, adapter %d", pid, old_pmt, spid, ad->id);
			}
		}
		
		if(cp = find_pid(ad->id, spid)) // the pid is already requested by the client
		{
			old_pmt = cp->key;

			if(!k)
			{
				p->key = keys_add(ad->id, program_id, pid); // we add a new key for this pmt pid
				k = get_key(p->key);
				if(!k)
					break;				
				set_next_key(k->id, old_pmt);
			}
			cp->key = p->key;
			cp->type = 0;
			k->ver = b[5] & 0x3F;
			k->program_id = program_id;
			k->enabled_channels ++;
		}
	}
	
	if((pi_len > 0) && k && k->enabled_channels)
	{	
			
		k->pi_len = pi_len;
		k->pi = pi;	
//		p = find_pid(ad->id, pid);
		dvbapi_send_pi(k);
		p->type |= PMT_COMPLETE;
	} else 
		p->type = 0; // we do not need this pmt pid anymore
//	free_assemble_packet(pid, ad);
	return 0;
}


