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

// number of pids for each ddci adapter to be stored in the mapping table
#define MAX_CHANNELS_ON_CI 4
#define PIDS_FOR_ADAPTER 128
int ddci_adapters;
extern int dvbca_id;

int first_ddci = -1;

#define get_ddci(i) ((i >= 0 && i < MAX_ADAPTERS && ddci_devices[i] && ddci_devices[i]->enabled) ? ddci_devices[i] : NULL)
typedef struct ddci_device
{
	SMutex *mutex;
	int enabled;
	int fd;
	int pid_mapping[8192];
	int channels;
	int max_channels;
	int pmt[MAX_CHANNELS_ON_CI + 1];
} ddci_device_t;

int ddci_id;
static ddci_device_t *ddci_devices[MAX_ADAPTERS];

int mapping_table_pids;
typedef struct ddci_mapping_table
{
	int ad_pid;
	int ddci_adapter;
	int ddci_pid;
	char rewrite;
	int pmt[MAX_CHANNELS_ON_CI + 1];
	int npmt;
} ddci_mapping_table_t;

ddci_mapping_table_t *mapping_table;

int add_pid_ddci(int ddci_adapter, int pid, int ddci_pid, int idx)
{
	ddci_device_t *d = get_ddci(ddci_adapter);
	if (!d)
		LOG_AND_RETURN(-1, "ddci_adapter %d disabled", ddci_adapter);
	if (pid < 0 || pid > 8191)
		LOG_AND_RETURN(-1, "pid %d invalid", pid);

	if (ddci_pid >= 0 && d->pid_mapping[ddci_pid] == idx)
		return ddci_pid;

	for (ddci_pid = pid; (ddci_pid & 0xFFFF) < 8191; ddci_pid++)
	{
		if (d->pid_mapping[ddci_pid] == -1)
		{
			d->pid_mapping[ddci_pid] = idx;
			return ddci_pid;
		}
		if (ddci_pid == pid)
			ddci_pid = ad->id * 100;
	}
	return -1;
}
int add_pid_mapping_table(int ad, int pid, int pmt, int ddci_adapter)
{
	int ddci_pid = 0, i;
	int key = (ad << 16) | pid;
	if (!mapping_table)
		return -1;
	int idx = get_index_hash(&mapping_table[0].ad_pid, mapping_table_pids, sizeof(ddci_mapping_table_t), key, key);
	if (idx == -1)
		idx = get_index_hash(&mapping_table[0].ad_pid, mapping_table_pids, sizeof(ddci_mapping_table_t), key, (uint32_t)-1);
	if (idx == -1)
		return -1;

	ddci_pid = add_pid_ddci(ddci_adapter, pid, mapping_table[idx].ddci_pid, idx);
	if (ddci_pid == -1)
		LOG_AND_RETURN(-1, "could not map the pid %d (ad %d) to a ddci pid", ad, pid);
	mapping_table[idx].ad_pid = key;
	mapping_table[idx].ddci_adapter = ddci_adapter;
	mapping_table[idx].ddci_pid = ddci_pid;
	mapping_table[idx].rewrite = 0;
	if (pid != ddci_pid)
		mapping_table[idx].rewrite = 1;

	int add_pid = 1, add_pmt = 1;
	for (i = 0; i < mapping_table[idx].npmt; i++)
	{
		if (mapping_table[idx].pmt[i] >= 0)
			add_pid = 0;
		if (mapping_table[idx].pmt[i] == pmt)
			add_pmt = 0;
	}
	if (add_pmt)
		for (i = 0; i < mapping_table[idx].npmt; i++)
		{
			if (mapping_table[idx].pmt[i] < 0)
				add = 0;
		}
	if (add_pid)
		mark_pid_add(-1, ad, pid);

	return ddci_pid;
}

inline static int get_mapping_table(int ad, int pid, int *ddci_adapter)
{
	int key = (ad << 16) | pid;
	int idx = get_index_hash(&mapping_table[0].ad_pid, mapping_table_pids, sizeof(ddci_mapping_table_t), key, key);
	if (idx == -1)
		return -1;
	*ddci_adapter = mapping_table[idx].ddci_adapter;
	return mapping_table[idx].ddci_pid;
}

int del_pid_ddci(int ddci_adapter, int ddci_pid, int pmt)
{
	ddci_device_t *d = get_ddci(ddci_adapter);
	if (!d)
		LOG_AND_RETURN(-1, "ddci_adapter %d disabled", ddci_adapter);
	d->pid_mapping[ddci_pid] = -1;

	return 0;
}
int del_pid_mapping_table(int ad, int pid)
{
	int ddci_pid, ddci_adapter;
	int key = (ad << 16) | pid;
	int idx = get_index_hash(&mapping_table[0].ad_pid, mapping_table_pids, sizeof(ddci_mapping_table_t), key, key);
	if (idx == -1)
		return -1;
	ddci_pid = mapping_table[idx].ddci_pid;
	ddci_adapter = mapping_table[idx].ddci_adapter;
	mapping_table[idx].ad_pid = -1;
	mapping_table[idx].ddci_adapter = -1;
	mapping_table[idx].ddci_pid = -1;
	mapping_table[idx].rewrite = 0;

	int del_pid = 1, del_pmt = 1;
	for (i = 0; i < mapping_table[idx].npmt; i++)
	{
		if (mapping_table[idx].pmt[i] == pmt)
			mapping_table[idx].pmt[i] = -1;
		if (mapping_table[idx].pmt[i] >= 0)
			del_pid = 0;
	}
	if (del_pid)
	{
		SPid *p = find_pid(ad, pid);
		LOGM("No pmt found for ad %d pid %d, deleteing if not used %d", ad, pid, p ? p->sid[0] : -2);
		if (p && p->sid[0] == -1)
			mark_pid_deleted(-1, ad, pid);
	}
	return del_pid_ddci(ddci_adapter, ddci_pid);
}

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

int find_ddci_for_pmt(SPMT *pmt)
{
	int i;
	ddci_device_t *d;
	for (i = 0; i < MAX_ADAPTERS; i++)
		if ((d = get_ddci(i)))
		{
			int j, send = 0;
			for (j = 0; j < ca[dvbca_id].ad_info[i].caids; j++)
				if (match_caid(pmt, ca[dvbca_id].ad_info[i].caid[j], ca[dvbca_id].ad_info[i].mask[j]))
				{
					LOG("DDCI CAID %04X and mask %04X matched PMT %d", ca[dvbca_id].ad_info[i].caid[j], ca[dvbca_id].ad_info[i].mask[j], pmt->id);
					if (d->channels < d->max_channels)
						return d->id;
				}
		}
	return -1;
}

// determine if the pids from this PMT needs to be added to the virtual adapter, also adds the PIDs to the translation table
int ddci_process_pmt(adapter *ad, SPMT *pmt)
{
	int i, ddid = 0;
	int rv = TABLES_RESULT_ERROR_NORETRY;
	ddid = find_ddci_for_pmt(pmt);
#ifdef DDCI_TEST
	ddid = first_ddci;
#endif
	ddci_device_t *d = get_ddci(ddid);
	if (d)
		return TABLES_RESULT_ERROR_NORETRY;
	mutex_lock(&d->mutex);

	if (d->channels == 0)
	{
		ddci_add_pid(d, ad->id, 1); // add pid 1
	}

	ddci_add_pid(d, ad->id, pmt->pid);
	for (i = 0; i < pmt->all_pids; i++)
		ddci_add_pid(d, ad->id, pmt->all_pid[i]);

	for (i = 0; i < pmt->caids; i++)
		ddci_add_pid(d, ad->id, pmt->capid[i]);

	rv = TABLES_RESULT_OK;
done:
	mutex_unlock(&d->mutex);
	return rv;
}

// if the PMT is used by the adapter, the pids will be removed from the translation table
int ddci_del_pmt(adapter *ad, SPMT *spmt)
{
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
	ddci.ca_add_pmt = ddci_process_pmt;
	ddci.ca_del_pmt = ddci_del_pmt;
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
	ddci_device_t *d = ddci_devices[ad->id];
	if (!d)
	{
		d = ddci_devices[ad->id] = malloc1(sizeof(ddci_device_t));
		if (!d)
			return -1;
		mutex_init(&d->mutex);
	}
	LOG("DDCI opening [%d] adapter %d and frontend %d", ad->id, ad->pa, ad->fn);
	sprintf(buf, "/dev/dvb/adapter%d/sec%d", ad->pa, ad->fn);
#ifndef DDCI_TEST
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
	mutex_lock(&d->mutex);
	ad->fe = write_fd;
	ad->dvr = read_fd;
	ad->type = ADAPTER_DVB;
	ad->dmx = -1;
	ad->sys[0] = 0;
	memset(d->pid_mapping, -1, sizeof(d->pid_mapping));
	d->max_channels = MAX_CHANNELS_ON_CI;
	d->channels = 0;
	mutex_unlock(&d->mutex);
	LOG("opened DDCI adapter %d fe:%d dvr:%d", ad->id, ad->fe, ad->dvr);

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

	ddci_adapters = 0;
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

				ddci_adapters++;
				na++;
				a_count = na; // update adapter counter
				if (na == MAX_ADAPTERS)
					return;
				if (first_ddci == -1)
					first_ddci = na;
#ifdef DDCI_TEST
				mapping_table_pids = ddci_adapters * PIDS_FOR_ADAPTER;
				mapping_table = malloc1(mapping_table_pids * sizeof(ddci_mapping_table_t));
				if (mapping_table)
					memset(mapping_table, -1, mapping_table_pids * sizeof(ddci_mapping_table_t));
				return;
#endif
			}
		}
	for (; na < MAX_ADAPTERS; na++)
		if (a[na])
			a[na]->pa = a[na]->fn = -1;

	mapping_table_pids = ddci_adapters * PIDS_FOR_ADAPTER;
	mapping_table = malloc1(mapping_table_pids * sizeof(ddci_mapping_table_t));
	if (!mapping_table)
		LOG("could not allocate memory for mapping table");

	memset(mapping_table, -1, mapping_table_pids * sizeof(ddci_mapping_table_t));
}
