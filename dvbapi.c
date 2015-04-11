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
#ifndef DISABLE_DVBCSA 


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

extern struct struct_opts opts;
const int64_t DVBAPI_ITEM = 0x1000000000000;
int dvbapi_sock;
int sock;
int isEnabled = 0;
int batchSize;
SKey keys[MAX_KEYS];
unsigned char read_buffer[1500];


#define MAX_DATA 1500
#define MAX_SINFO 100

typedef struct tmpinfo
{
	unsigned char enabled;
	uint16_t len;
	int64_t key;
	uint16_t id;
	unsigned char *data;
} STmpinfo;
STmpinfo sinfo[MAX_SINFO];

STmpinfo *getItemPos(int64_t key)
{
	static STmpinfo *last;	
	int i;
	if(last && (last->enabled) && (last->key==key))
		return last;
	for (i=0;i<MAX_SINFO;i++)
		if(sinfo[i].enabled && sinfo[i].key==key)
		{
			last = sinfo + i;
			return last;
		}
	return NULL;
}

STmpinfo *getFreeItemPos(int64_t key)
{
	int i;
	for (i=0;i<MAX_SINFO;i++)
		if(!sinfo[i].enabled)
		{
			sinfo[i].id = i;
			LOGL(2,"Requested new Item for key %llX, returning %d", key, i);
			return sinfo + i;
		}
	return NULL;
}

unsigned char *getItem(int64_t key)
{
	STmpinfo *s = getItemPos(key);
	return s?s->data:NULL;
}

int getItemLen(int64_t key)
{
	STmpinfo *s = getItemPos(key);
	return s? s->len : 0;
}


int setItem(int64_t key, unsigned char *data, int len, int pos)   // pos = -1 -> append, owerwrite the existing key
{
	STmpinfo *s = getItemPos(key);
	if(!s)
		s = getFreeItemPos(key);
	if(!s)
		return -1;
	
	if(!s->data)
		s->data = malloc1(MAX_DATA);
	if(!s->data)
		return -1;
	s->enabled = 1;
	s->key = key;	
	if(pos == -1)
		pos = s->len;
	if(pos + len >= MAX_DATA) // make sure we do not overflow the data buffer
		len = MAX_DATA - pos;
		
	s->len = pos + len;
	
	memcpy(s->data + pos, data, len);			
}

int delItem(int64_t key)
{
	STmpinfo *s = getItemPos(key);
	if(!s)
		return 0;
	s->enabled = 0;
	s->len = 0;
	s->key = 0;
	LOG("Deleted Item Pos %d", s->id);
	return 0;
}

int have_dvbapi()
{
	return isEnabled;
}


int dvbapi_close(sockets * s)
{
	sock = 0;
	isEnabled = 0;
	return 0;
}


int dvbapi_reply(sockets * s)
{
	unsigned char *b = s->buf;
	SKey *k;
	unsigned int op, version, _pid;
	
	int k_id, a_id = 0, s_id, pos = 0;
	int demux, filter;
	SPid *p;
	while(pos < s->rlen)
	{
		b = s->buf + pos;
		copy32r(op, b, 0);
		LOGL(3, "dvbapi read from socket %d the following data (%d bytes), pos = %d, op %08X: ", s->sock, s->rlen, pos, op);
		switch(op){
		case DVBAPI_SERVER_INFO:
		
			if(s->rlen < 6)
				return;
			copy16r(version, b, 4);
			LOG("DVBAPI server version %d found, name = %s", version, b+6);
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
			copy16r(_pid, b, 7);
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
			if(p->filter == 255)
			{
				p->filter = filter;
				k->demux = demux;
				p->type = TYPE_ECM;
				p->key = k_id;
			} 
//				else k->parity = -1;
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
			copy16r(_pid, b, 7 )
			LOG("Received from DVBAPI server DMX_STOP for adapter %d, pid %X (%d)", a_id, _pid, _pid);
			if((p=find_pid(a_id, _pid)))
				p->type = 0;
				
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
			int index, parity;
			SKey *k;
			SPid *p;
			unsigned char *cw;
			pos += 21;
			copy32r(index, b, 5);
			copy32r(parity, b, 9);			
			cw = b + 13;			
			k = get_key(index);
			if(k && (parity < 2))
			{
				LOG("received DVBAPI_CA_SET_DESCR, key: %d parity: %d, CW: %02X %02X %02X %02X %02X %02X %02X %02X", index, parity, cw[0], cw[1], cw[2], cw[3], cw[4], cw[5], cw[6], cw[7]);			
				dvbcsa_bs_key_set(cw, k->key[parity]);
				k->key_ok[parity] = 1;
			}
			break; 
		}
		default: pos = s->rlen;
		}
	}
	s->rlen = 0;
	return 0;
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
	if(opts.log > 4)
	{
		b = k->batch[0].data + k->batch[0].len - 188;
		pid = (b[1] & 0x1F)*256 + b[2];
		for(i=1;i<k->blen;i++)
		{
			b = k->batch[i].data + k->batch[i].len - 188;
			if(((b[1] & 0x1F)*256 + b[2]) == pid)
			{
				if((b[1] & 0x40) && (oldb1>=0))
					break;
				oldb1 = b[186];
				oldb2 = b[187];
				oldb = b;
			}
		}
		if(i>=k->blen)
			b = NULL;
		else b = oldb;
				
	}
		
	k->batch[k->blen].data = NULL;
	k->batch[k->blen].len = 0;	
	dvbcsa_bs_decrypt(k->key[k->parity], k->batch, 184);
	LOGL(5, "dvbapi: decrypted key %d parity %d at len %d, channel_id %d (pid %d) padding: [%02X %02X] -> [%02X %02X]", 
		k->id, k->parity, k->blen, k->sid, pid, oldb1, oldb2, b?b[186]:153, b?b[187]:153); //0x99
	k->blen = 0;
	k->parity = -1;
	return 0;
}
int decrypt_stream(adapter *ad,int rlen)
{
	struct iovec *iov;
	SKey *k = NULL;
	int adapt_len;
	short pid_to_key[8192];
	int len;	
	 // max batch
	int i = 0, j = 0;
	unsigned char *b;
	SPid *p;
	int pid;
	int cp;
	
	if(!isEnabled)
	{
		init_dvbapi();
		if(!isEnabled)
			return 0;
	}	
	if(!batchSize)
		batch_size();
	
	memset(pid_to_key, -1, sizeof(pid_to_key));
	for(i=0;i<MAX_PIDS;i++)
		if(ad->pids[i].flags == 1)
			if(ad->pids[i].type==0)
				pid_to_key[ad->pids[i].pid]=ad->pids[i].key;
			else if (ad->pids[i].type==TYPE_ECM)
				pid_to_key[ad->pids[i].pid] = -TYPE_ECM - 1;
			else if (ad->pids[i].type==TYPE_PMT)
                                pid_to_key[ad->pids[i].pid] = -TYPE_PMT - 1;
	for(i=0;i<rlen;i+=188)
	{
		b = ad->buf + i;
		pid = (b[1] & 0x1F)*256 + b[2];
		if(pid_to_key[pid]==-TYPE_PMT-1)
			process_pmt(b,ad);
		else if (pid_to_key[pid]==-TYPE_ECM-1)
                        send_ecm(b,ad);
		else if (pid == 0)
			process_pat(b, ad);
	}	
	for(i=0;i<rlen;i+=188)
	{
		b = ad->buf + i;
//		LOG("--> %02X %02X %02X %02X ", b[0], b[1], b[2], b[3]);
		if(b[3] & 0x80)
		{
			pid = (b[1] & 0x1F)*256 + b[2]; 
			k = get_key(pid_to_key[pid]);
			if(!k)
			{
//				LOGL(4, "stream_decrypt: key is null for pid %d", pid)
				continue;
			}
//			LOG("decrypting index %d j = %d", i, j);
			cp = ((b[3] & 0x40) > 0);			
			if(k->parity == -1)
				k->parity = cp;
			if((k->parity != cp) || (k->blen >=batchSize))  // partiy change or batch buffer full
			{
				int old_parity = k->parity;
				decrypt_batch(k);
				if(old_parity != cp)
				{
					LOGL(2, "Parity change for key %d, old parity %d, new parity %d pid %d [%02X %02X %02X %02X]", k->id, old_parity, cp, pid, b[0], b[1], b[2], b[3]);
					k->key_ok[old_parity] = 0;
				}
				k->parity = cp;
			}

			if(!k->key_ok[cp])
			{
				LOGL(4, "NOT DECRYPTING for key %d drop_encrypted=%d parity %d", k->id, opts.drop_encrypted, cp);
				if(opts.drop_encrypted)
				{
					b[1] |= 0x1F;
					b[2] |= 0xFF; 
				}
				continue;
			}
			
			if(b[3] & 0x20)
			{
				adapt_len = (b[4]<183)?b[4]+5:0;
//				LOGL(2, "Adaptation for pid %d, specified len %d, final len %d", pid, b[4], adapt_len);
			}	
			else adapt_len = 4;
			k->batch[k->blen].data = b + adapt_len;
			k->batch[k->blen++].len = 188 - adapt_len;
			b[3] &= 0x3F; // clear the encrypted flags
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
	
	LOG("Starting sending pmt for pid %d, Channel ID %d, key %d, using socket %d", k->pmt_pid, k->sid, k->id, sock);


	copy32(buf, 0, AOT_CA_PMT);
	buf[6] = CAPMT_LIST_ONLY;
	copy16(buf, 7, k->sid);
	buf[9] = 1;
	
	copy32(buf, 12, 0x01820200);
	buf[16] = k->id;
	memcpy(buf + 17, k->pi, k->pi_len + 2);
	len = 17  - 6 + k->pi_len + 2;
	copy16(buf, 4, len);
	copy16(buf, 10, len - 11);
	write(sock, buf, len + 6 );
}

int init_dvbapi()
{
	unsigned char buf[1000];
	unsigned char len;
	int ctime;
	static int last_retry = -5000;
	if(sock>0)
			return sock;
	isEnabled = 0;
	if(!opts.dvbapi_port  || !opts.dvbapi_host)
		return 0;
	
	ctime = getTick();
	if(ctime - last_retry < 5000) // retry every 5s
		return 0;
	last_retry = ctime;
	sock = tcp_connect(opts.dvbapi_host, opts.dvbapi_port, NULL);
	if(sock < 0)
		return 0;
	dvbapi_sock = sockets_add (sock, NULL, -1, TYPE_TCP, (socket_action) dvbapi_reply, (socket_action) dvbapi_close, NULL);
	set_socket_buffer (dvbapi_sock , read_buffer, sizeof(read_buffer));
	isEnabled = 1;
	
	copy32(buf, 0, DVBAPI_CLIENT_INFO);
	copy16(buf, 4, DVBAPI_PROTOCOL_VERSION);
	len = sprintf(buf + 7, "minisatip/%s", VERSION);
	buf[6] = len;
	write(sock, buf, len+7);	
}

int process_pat(unsigned char *b, adapter *ad)
{
	int pat_len, i, tid, sid, pid, csid = 0;
	SPid *p;
	if(ad->pat_processed)
		return;
	if((b[0]!=0x47) || (b[1]!=0x40) || (b[2]!=0) || (b[5]!=0)) // make sure we are dealing with PAT
		return 0;
	pat_len = ((b[6] & 0xF) << 8) + b[7];
	tid = b[8] * 256 + b[9];
//	LOG("tid %d pat_len %d: %02X %02X %02X %02X %02X %02X %02X %02X", tid, pat_len, b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7]);
	
	pat_len -= 9;
	b += 13;
	for(i=0;i < pat_len; i+=4)
	{
		sid = b[i]*256 + b[i + 1];
		pid = (b[i + 2] & 0x1F)*256 + b[i + 3];
		LOGL(2, "Adapter %d, Transponder ID %d, PMT sid %d, pid %d", ad->id, tid, sid, pid);
		if(sid>0)
		{
			p = find_pid(ad->id, pid);
			if(!p)
				mark_pid_add(-1, ad->id, pid);

			if((p = find_pid(ad->id, pid)))
			{
				p->type = TYPE_PMT;
				csid = pid;
			}
		}
	}	
	update_pids(ad->id);
	ad->pat_processed = 1;
	return csid;
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

int process_pmt(unsigned char *b, adapter *ad)
{
	int pi_len = 0, pmt_len = 0, i, _pid, es_len;
	int program_id = 0, enabled_channels = 0;
	int64_t item_key;
	unsigned char *pmt, *pi;
	int caid, capid, spid, stype;
	SPid *p, *cp;
	SKey *k = NULL;

	if((b[0]!=0x47)) // make sure we are dealing with TS
		return 0;
	
	_pid = (b[1] & 0x1F)* 256 + b[2];
	if(!(p = find_pid(ad->id, _pid)))
		return -1;

	if(!p || (p->type & PMT_COMPLETE))
		return 0;

	if((b[5] == 2) && ((b[1] &0x40) == 0x40))
		pmt_len = ((b[6] & 0xF) << 8) + b[7];

	item_key = DVBAPI_ITEM + (ad->id << 16) + _pid;
	
	if(pmt_len > 183)
	{
		setItem(item_key, b+5, 183, 0);
		return 0;

	} else if(pmt_len > 0)
	{		
		b = b + 5;
	}else // pmt_len == 0 - next part from the pmt
	{
		setItem(item_key, b+4, 184, -1);
		b = getItem(item_key);
		pmt_len = ((b[1] & 0xF) << 8) + b[2];
		if(getItemLen(item_key) < pmt_len)
			return 0;
		
	}

	program_id = (b[3] & 0x1F)* 256 + b[4];
	pmt_len = ((b[1] & 0xF) << 8) + b[2];
	pi_len = ((b[10] & 0xF) << 8) + b[11];
	LOGL(5,"PMT pid: %04X (%d), pmt_len %d, pi_len %d, channel id %d len %d", _pid, _pid, pmt_len, pi_len, program_id, pmt_len);

	pi = b + 12;	
	pmt = pi + pi_len;
	if(pi_len > 0) 
		pi=find_pi(pi, pi_len, &pi_len);
	
	es_len = 0;	
	for(i = 0; i < pmt_len - pi_len - 17; i+= (es_len) + 5) // reading streams
	{
		es_len = (pmt[i+3] & 0xF)*256 + pmt[i+4];
		stype = pmt[i];
		spid = (pmt[i+1] & 0x1F)*256 + pmt[i+2];		
		LOG("PMT pid %d - stream pid %04X (%d), type %d, es_len = %d", _pid, spid, spid, stype, es_len);
		if(es_len>384)
			return -1;
		if(cp = find_pid(ad->id, spid)) // the pid is already requested by the client
		{
			if(cp->key!=255)
				continue;

			if(!k)
			{
				p->key = keys_add(ad->id, program_id, _pid); // we add a new key for this pmt pid
				k = get_key(p->key);
				if(!k)
					return 0;
			}
			cp->key = p->key;
			enabled_channels ++;
		}
		if(!pi_len)
			pi = find_pi(pmt + i + 5, es_len, &pi_len);
	}
	if((pi_len > 0) && enabled_channels)
	{	
			
		k->pi_len = pi_len;
		k->pi = pi;	
		p = find_pid(ad->id, _pid);
		dvbapi_send_pi(k);
	}	
	delItem(item_key);
	p->type |= PMT_COMPLETE;
	return 0;
}


int send_ecm(unsigned char *b, adapter *ad)
{
	unsigned char buf[1500];
	SPid *p;
	SKey *k = NULL;
	int64_t item_key;
	int pid, len = 0;
	int filter, demux;
	int old_parity;
	pid = (b[1] & 0x1F)* 256 + b[2];
	p = find_pid(ad->id, pid);
	if(p)
		k = get_key(p->key);
	if(!k)
		LOG_AND_RETURN(0, "key is null pid %d for p->key %d", pid, p?p->key:-1);
		
	demux = k->demux;
	filter = p->filter;	
	if((b[1] & 0x40) == 0x40)
	{
		len = ((b[6] & 0xF) << 8) + b[7];
		len += 3;
	}
	
	item_key = DVBAPI_ITEM + (k->id << 16) + pid;	
	if(len > 559)
		return -1;
	if(len> 183)
	{
		setItem(item_key, b+5, 183, 0);
		return 0;
	}else if(len > 0)
	{		
		b += 5;
	}else // len == 0 - next part from the pmt
	{
		setItem(item_key, b+4, 184, -1);				
		b = getItem(item_key);
	}
	
	if((b[0] & 1) == p->ecm_parity)
		return 0;
	old_parity = p->ecm_parity;
	p->ecm_parity = b[0] & 1;	

	len = ((b[1] & 0xF) << 8) + b[2];
	len += 3;	
	LOG("Sending DVBAPI_FILTER_DATA key %d for pid %04X (%d), ecm_parity = %d, new parity %d, demux = %d, filter = %d, len = %d", k->id, pid, pid, old_parity, b[0] & 1,demux, filter, len);
	if(len>559+3)
		return -1;
	copy32(buf, 0, DVBAPI_FILTER_DATA);
	buf[4] = demux;
	buf[5] = filter;
	 // filter id
	memcpy(buf + 6, b, len + 6);
	write(sock, buf, len + 6);	
	delItem(item_key);

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
	LOG("returning new key %d for adapter %d, pmt pid %d sid %d", i, adapter, pmt_pid, sid);
	return i;
}

int keys_del(int i)
{
	unsigned char buf[8]={0x9F,0x80,0x3f,4,0x83,2,0,0};
	if((i<0) || (i>=MAX_KEYS) || (!keys[i].enabled))
		return 0;
	keys[i].enabled = 0;
	//buf[7] = keys[i].demux;
	buf[7] = i;
	LOG("Stopping DEMUX %d, removing key %d", buf[7], i);
	if(buf[7] != 255)
		write(sock, buf, sizeof(buf));
	keys[i].sid = 0;
	keys[i].pmt_pid = 0;
	keys[i].adapter = -1;
	keys[i].demux = -1;
}

SKey *get_key(int i)
{
	if(i<0 || i>=MAX_KEYS || !keys[i].enabled)
		return NULL;
	return &keys[i];
}


#endif
