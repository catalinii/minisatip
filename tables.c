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
#include "dvbapi.h"
#include "tables.h"

#ifndef DISABLE_TABLES

extern struct struct_opts opts;


#define MAX_DATA 1500 // 16384
#define MAX_SINFO 100


#define TEST_WRITE(a) if((a)<=0){LOG("%s:%d: write to dvbapi socket failed, closing socket %d",__FILE__,__LINE__,sock);sockets_del(dvbapi_sock);sock = 0;dvbapi_sock = -1;isEnabled = 0;}


typedef struct tmpinfo
{
	unsigned char enabled;
	uint16_t len;
	int64_t key;
	uint16_t id;
	uint16_t max_size;
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

int setItemSize(int64_t key, int max_size)
{
	STmpinfo *s = getItemPos(key);
	if(!s)
		return -1;
	s->max_size = max_size + 100;
	if(s->data)
		free1(s->data);
	s->data = malloc1(s->max_size);
	if(!s->data)
		return -1;
	return 0;
}

int setItem(int64_t key, unsigned char *data, int len, int pos)   // pos = -1 -> append, owerwrite the existing key
{
	STmpinfo *s = getItemPos(key);
	if(!s)
		s = getFreeItemPos(key);
	if(!s)
		return -1;
	
	if(s->max_size==0)
		s->max_size = MAX_DATA;
	if(!s->data)
		s->data = malloc1(s->max_size);
	if(!s->data)
		return -1;
	s->enabled = 1;
	s->key = key;	
	if(pos == -1)
		pos = s->len;
	if(pos + len >= s->max_size) // make sure we do not overflow the data buffer
		len = s->max_size - pos;
		
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


int process_pat(unsigned char *b, adapter *ad)
{
	int pat_len = 0, i, tid, sid, pid, csid = 0;
	int64_t pid_key = TABLES_ITEM + ((1+ ad->id) << 24) + 0;
	int16_t *pids;
	int64_t item_key = TABLES_ITEM + (ad->id << 16) + pid;	

	SPid *p;
	if(ad->pat_processed)
		return;

	if(((b[1] & 0x1F) != 0) || (b[2]!=0))
		return 0;
		
	if((b[0]==0x47) && (b[1]==0x40) && (b[2]==0) && (b[5]==0)) // make sure we are dealing with PAT
		pat_len = ((b[6] & 0xF) << 8) + b[7];

	if(pat_len > 183)
	{
		setItem(item_key, b+5, 183, 0);
		return 0;

	} else if(pat_len > 0)
	{		
		b = b + 5;
	}else // pmt_len == 0 - next part from the pmt
	{
		setItem(item_key, b+4, 184, -1);
		b = getItem(item_key);
		pat_len = ((b[1] & 0xF) << 8) + b[2];
		if(getItemLen(item_key) < pat_len)
			return 0;
		
	}
	
	pat_len = ((b[1] & 0xF) << 8) + b[2];
	tid = b[3] * 256 + b[4];
	
//	LOG("tid %d pat_len %d: %02X %02X %02X %02X %02X %02X %02X %02X", tid, pat_len, b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7]);
	
	setItem(pid_key, b, 1, 0);
	setItemSize(pid_key, 8192*sizeof(*pids));
	pids = (int16_t *)getItem(pid_key);
	memset(pids, 0, 8192*sizeof(*pids));
	pat_len -= 9;
	b += 8;	
	LOGL(2, "PAT Adapter %d, Transponder ID %d, len %d", ad->id, tid, pat_len);
	if(pat_len>1500)
		return 0;
	for(i=0;i < pat_len; i+=4)
	{
		sid = b[i]*256 + b[i + 1];
		pid = (b[i + 2] & 0x1F)*256 + b[i + 3];
		LOGL(2, "Adapter %d, PMT sid %d, pid %d", ad->id, sid, pid);
		if(sid>0)
		{
			pids[pid] = -TYPE_PMT;
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
	int program_id = 0;
	int64_t item_key;
	int prio = 0;
	unsigned char *pmt, *pi;
	int caid, capid, pid, spid, stype;
	SPid *p, *cp;
	SKey *k = NULL;
	int64_t pid_key = TABLES_ITEM + ((1+ ad->id) << 24) + 0;
	int16_t *pids;
	int old_pmt;

	if((b[0]!=0x47)) // make sure we are dealing with TS
		return 0;
	
	pid = (b[1] & 0x1F)* 256 + b[2];
	if(!(p = find_pid(ad->id, pid)))
		return -1;
		
	if(p->sid[0] != -1) // the PMT pid is requested by the stream
		prio = 1;

	if(!p || (p->type & PMT_COMPLETE) || (p->type == 0))
		return 0;
	
	p->type |= TYPE_PMT;
	
	if((b[5] == 2) && ((b[1] &0x40) == 0x40))
		pmt_len = ((b[6] & 0xF) << 8) + b[7];
	
	if(p && (k = get_key(p->key)))
	{
		program_id = b[8]* 256 + b[9];
		if((pmt_len > 0) && (program_id == k->sid))
				p->type |= PMT_COMPLETE;
		LOG_AND_RETURN(0, "Trying to add existing key %d for PMT pid %d sid %d k->sid %d", p->key, pid, program_id, k->sid);
	}

	item_key = TABLES_ITEM + (ad->id << 16) + pid;	
	
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

	program_id = b[3]* 256 + b[4];
	pmt_len = ((b[1] & 0xF) << 8) + b[2];
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
	pids[pid] = -TYPE_PMT;
	for(i = 0; i < pmt_len - pi_len - 17; i+= (es_len) + 5) // reading streams
	{
		es_len = (pmt[i+3] & 0xF)*256 + pmt[i+4];
		stype = pmt[i];
		spid = (pmt[i+1] & 0x1F)*256 + pmt[i+2];		
		LOG("PMT pid %d - stream pid %04X (%d), type %d, es_len %d, pos %d, pi_len %d old pmt %d, old pmt for this pid %d", pid, spid, spid, stype, es_len, i, pi_len, pids[pid], pids[spid]);
		if((es_len + i>pmt_len) || (es_len == 0))
			break;
		if(!pi_len)
			pi = find_pi(pmt + i + 5, es_len, &pi_len);
		
		if(pi_len == 0)
			continue;
			
		old_pmt = pids[spid];
		pids[spid] = pid; 
		if((old_pmt > 0) && (old_pmt != -pid))  // this pid is associated with another PMT - link this PMT with the old one (if not linked already)
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
	delItem(item_key);
	return 0;
}

#endif