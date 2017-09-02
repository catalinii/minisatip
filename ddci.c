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
#include <fcntl.h>
#include <ctype.h>

#include <linux/dvb/ca.h>
#include "dvb.h"
#include "socketworks.h"
#include "minisatip.h"
#include "ddci.h"
#include "utils.h"

#define DEFAULT_LOG LOG_DVBCA

typedef struct ddci_device
{
	int enabled;
	int fd;

} ddci_device_t;

int ddci_id;
static ddci_device_t *ddci_devices[MAX_ADAPTERS];

int ddci_init_dev(adapter *ad)
{
	return TABLES_RESULT_OK;
}

int ddci_close_dev(adapter *ad)
{
	return TABLES_RESULT_OK;
}
SCA_op ddci;

void ddci_close_device(ddci_device_t *c)
{
}

int ddci_close_all()
{
	int i;
	for (i = 0; i < MAX_ADAPTERS; i++)
		if (ddci_devices[i] && ddci_devices[i]->enabled)
		{
			ddci_close_device(ddci_devices[i]);
		}
	return 0;
}

int ddci_close(adapter *a)
{
	return 0;
}

int ddci_ts(adapter *ad)
{
	return 0;
}

void ddci_init() // you can search the devices here and fill the ddci_devices, then open them here (for example independent CA devices), or use ddci_init_dev to open them (like in this module)
{
	memset(&ddci, 0, sizeof(ddci));
	ddci.ca_init_dev = ddci_init_dev;
	ddci.ca_close_dev = ddci_close_dev;
	ddci.ca_add_pmt = NULL;
	ddci.ca_del_pmt = NULL;
	ddci.ca_close_ca = ddci_close;
	ddci.ca_ts = ddci_ts;
	ddci_id = add_ca(&ddci, 0xFFFFFFFF);
}
int ddci_set_pid(adapter *a, int i_pid)
{
	return 100;
}

int ddci_del_filters(adapter *ad, int fd, int pid)
{
	return 0;
}

int ddci_read_sec_data(sockets *s)
{
	read_dmx(s);
	LOG("done read_dmx")
	return 0;
}

void ddci_post_init(adapter *ad)
{
	sockets *s = get_sockets(ad->sock);
	s->action = (socket_action)ddci_read_sec_data;
}

int ddci_open_device(adapter *ad)
{
	char buf[100];
	int read_fd, write_fd;
	LOG("DDCI opening [%d] adapter %d and frontend %d", ad->id, ad->pa, ad->fn);
	sprintf(buf, "/dev/dvb/adapter%d/sec%d", ad->pa, ad->fn);
#ifdef DDCI_TEST
	write_fd = open(buf, O_WRONLY);
	if (write_fd < 0)
	{
		LOG("%s: could not open %s in WRONLY mode error %d: %s", __FUNCTION__, buf, errno, strerror(errno));
		return 1;
	}

	read_fd = open(buf, O_RDONLY);
	if (read_fd < 0)
	{
		LOG("%s: could not open %s in RDONLY mode error %d: %s", __FUNCTION__, buf, errno, strerror(errno));
		if (write_fd >= 0)
			close(write_fd);
		ad->fe = -1;
		return 1;
	}
#else
	int fd[2];
	if (pipe(fd) == -1)
	{
		LOG("pipe failed errno %d: %s", errno, strerror(errno));
		return 1;
	}
	read_fd = fd[0];
	write_fd = fd[1];

#endif
	ad->fe = write_fd;
	ad->dvr = read_fd;
	ad->type = ADAPTER_DVB;
	ad->dmx = -1;
	ad->sys[0] = 0;
	LOG("opened DVB adapter %d fe:%d dvr:%d", ad->id, ad->fe, ad->dvr);

	return 0;
}

fe_delivery_system_t ddci_delsys(int aid, int fd, fe_delivery_system_t *sys)
{
	return 0;
}

void find_ddci_adapter(adapter **a)
{
	int na = 0;
	char buf[100];
	int cnt;
	int i = 0, j = 0;
	adapter *ad;
	if (opts.disable_dvb)
	{
		LOG("DVBCI device detection disabled");
		return;
	}

	for (i = 0; i < MAX_ADAPTERS; i++)
		if (a[i])
			na = i;
	na++;
	LOGM("Starting %s with index %d", __FUNCTION__, na);

	for (i = 0; i < MAX_ADAPTERS; i++)
		for (j = 0; j < MAX_ADAPTERS; j++)
		{
			cnt = 0;
			sprintf(buf, "/dev/dvb/adapter%d/ca%d", i, j);
			if (!access(buf, R_OK))
				cnt++;

			sprintf(buf, "/dev/dvb/adapter%d/sec%d", i, j);
			if (!access(buf, R_OK))
				cnt++;
#ifdef DDCI_TEST
			cnt = 2;
#endif
			if (cnt == 2)
			{
				LOGM("%s: adding %d %d to the list of devices", __FUNCTION__, i, j);
				if (!a[na])
					a[na] = adapter_alloc();

				ad = a[na];
				ad->pa = i;
				ad->fn = j;

				ad->open = (Open_device)ddci_open_device;
				ad->commit = (Adapter_commit)NULL;
				ad->tune = (Tune)NULL;
				ad->delsys = (Dvb_delsys)ddci_delsys;
				ad->post_init = (Adapter_commit)ddci_post_init;
				ad->close = (Adapter_commit)ddci_close;
				ad->get_signal = (Device_signal)NULL;
				ad->set_pid = (Set_pid)ddci_set_pid;
				ad->del_filters = (Del_filters)ddci_del_filters;
				ad->type = ADAPTER_DVB;

				na++;
				a_count = na; // update adapter counter
				if (na == MAX_ADAPTERS)
					return;
#ifdef DDCI_TEST
				break;
#endif
			}
		}
	for (; na < MAX_ADAPTERS; na++)
		if (a[na])
			a[na]->pa = a[na]->fn = -1;
}
