/*
 * Copyright (C) 2016 Catalin Toda <catalinii@yahoo.com>
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
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fcntl.h>
#include <ctype.h>

#include "dvb.h"
#include "socketworks.h"
#include "minisatip.h"
#include "t2mi.h"
#include "utils.h"
#include "tables.h"

#define DEFAULT_LOG LOG_STREAM

t2mi_device_t *t2[MAX_ADAPTERS];

t2mi_device_t *get_or_alloc_t2mi(int id)
{
	if (t2[id])
		return t2[id];

	if (id < 0 || id >= MAX_ADAPTERS)
		return NULL;

	LOG("Allocating t2mi for device %d", id);
	t2[id] = malloc1(sizeof(t2mi_device_t));
	return t2[id];
}

void detect_t2mi(adapter *ad)
{
	int i, pid;
	int rlen = ad->rlen;
	uint8_t *b;

	for (i = 0; i < rlen; i += DVB_FRAME)
	{
		b = ad->buf + i;
		pid = PID_FROM_TS(b);
		if (pid != T2MI_PID)
		{
			ad->is_t2mi = -1; // no t2mi
			return;
		}
	}
	for (i = 0; i < rlen; i += DVB_FRAME)
	{
		b = ad->buf + i;
		pid = PID_FROM_TS(b);
		if (pid == T2MI_PID)
		{
			ad->is_t2mi = 1; // t2mi
			return;
		}
	}
}

// unpack t2mi stream
void t2mi_process_t2mi(t2mi_device_t *t, adapter *ad, uint8_t *b)
{
}

int t2mi_process_ts(adapter *ad)
{
	int i, pid;
	int rlen = ad->rlen;
	uint8_t *b;

	if (ad->is_t2mi == 0)
		detect_t2mi(ad);

	if (ad->is_t2mi <= 0)
		return 0;

	t2mi_device_t *t = get_or_alloc_t2mi(ad->id);
	for (i = 0; i < rlen; i += DVB_FRAME)
	{
		b = ad->buf + i;
		pid = PID_FROM_TS(b);
		if (pid == T2MI_PID)
			t2mi_process_t2mi(t, ad, b);
	}
	return 0;
}

void free_t2mi()
{
	int i;
	for(i=0;i<MAX_ADAPTERS;i++)
	if(t2[i])
	{
		free1(t2[i]);
		t2[i] = NULL;
	}
}
