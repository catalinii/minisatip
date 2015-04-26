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
#include "ca.h"

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
	uint32_t max_size;
	uint32_t timeout;
	uint32_t last_updated;
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
	int tick = getTick();
	for (i=0;i<MAX_SINFO;i++)
		if(!sinfo[i].enabled || (sinfo[i].timeout && (tick - sinfo[i].last_updated > sinfo[i].timeout)))
		{
			sinfo[i].id = i;
			sinfo[i].timeout = 0;
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

int getItemSize(int64_t key)
{
	STmpinfo *s = getItemPos(key);
	if(!s)
		return 0;
	return s->max_size;
}


int setItemSize(int64_t key, uint32_t max_size)
{
	STmpinfo *s = getItemPos(key);
	if(!s)
		return -1;
	if(s->max_size == max_size)
		return 0;
	s->max_size = max_size;
	if(s->data)
		free1(s->data);
	s->data = malloc1(s->max_size + 100);
	if(!s->data)
		return -1;
	return 0;
}

int setItemTimeout(int64_t key, int tmout)
{
	STmpinfo *s = getItemPos(key);
	if(!s)
		return -1;
	s->timeout = tmout;
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
	s->last_updated = getTick();
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


	SPid *p;
	if(ad->pat_processed)
		return;

	if(((b[1] & 0x1F) != 0) || (b[2]!=0))
		return 0;

	if(!(pat_len = assemble_packet(&b,ad)))
		return 0;

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
#ifndef DISABLE_DVBCSA
			if(!p)
				mark_pid_add(-1, ad->id, pid);
#endif
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

#define ASSEMBLE_TIMEOUT 1000

int assemble_packet(uint8_t **b1, adapter *ad)
{
	int len = 0, pid;
	int64_t item_key;
	uint8_t *b = *b1;
	
	if((b[0]!=0x47)) // make sure we are dealing with TS
		return 0;
	
	pid = (b[1] & 0x1F)* 256 + b[2];
	
	if((b[1] &0x40) == 0x40)
		len = ((b[6] & 0xF) << 8) + b[7];
	
	if(len > 1500)
		return 0;
		
	item_key = TABLES_ITEM + (ad->id << 16) + pid;	
	
	if(len > 183)
	{
		setItem(item_key, b+5, 183, 0);
		setItemTimeout(item_key, ASSEMBLE_TIMEOUT);
		return 0;

	} else if(len > 0)
	{		
		b = b + 5;
	}else // pmt_len == 0 - next part from the pmt
	{
		setItem(item_key, b+4, 184, -1);
		setItemTimeout(item_key, ASSEMBLE_TIMEOUT);
		b = getItem(item_key);
		len = ((b[1] & 0xF) << 8) + b[2];
		if(getItemLen(item_key) < len)
			return 0;		
	}	
	*b1 = b;
	return len;
}

void free_assemble_packet(int pid, adapter *ad)
{
	delItem(TABLES_ITEM + (ad->id << 16) + pid);
}

int process_stream(adapter *ad,int rlen)
{	
	SPid *p;
	int i, isPMT, pid;
	uint8_t *b;
	
	int64_t pid_key = TABLES_ITEM + ((1+ ad->id) << 24) + 0;	
	int16_t *pids = (int16_t *)getItem(pid_key);
	
	for(i=0;i<rlen;i+=188)
	{
		b = ad->buf + i;
		pid = PID_FROM_TS(b);
		isPMT = 0;
		if(pid == 0)
		{
			process_pat(b, ad);
			continue;
		}
		
		if(pids && pids[pid]<0)
			isPMT = 1;
/*		else if(!pids)
		{
			p = find_pid(ad->id, pid);
			if(p && (p->type & TYPE_PMT))
				isPMT = 1;
		}
*/	
		if(isPMT)
		{
#ifndef DISABLE_DVBCA
			dvbca_process_pmt(b, ad);
#endif				
#ifndef DISABLE_DVBCSA
			dvbapi_process_pmt(b, ad);
#endif				
		}
	}	
#ifndef DISABLE_DVBCSA
	decrypt_stream(ad, rlen);
#endif	

}

