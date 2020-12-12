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
#include "pmt.h"

#define DEFAULT_LOG LOG_PMT

SPMT *pmts[MAX_PMT];
SMutex pmts_mutex;
int npmts;

#define MAX_OPS 10
_SCW_op ops[MAX_OPS];

SCW *cws[MAX_CW];
SMutex cws_mutex;
int ncws;

SFilter *filters[MAX_FILTERS];
SMutex filters_mutex;
int nfilters;

static inline SCW *get_cw(int id)
{
	if (id < 0 || id >= MAX_CW || !cws[id] || !cws[id]->enabled)
		return NULL;
	return cws[id];
}

static inline SPMT *get_pmt_for_existing_pid(SPid *p)
{
	SPMT *pmt = NULL;
	if (p && p->pmt >= 0 && p->pmt < npmts && pmts[p->pmt] && pmts[p->pmt]->enabled)
	{
		pmt = pmts[p->pmt];
	}
	return pmt;
}

int register_algo(SCW_op *o)
{
	int i;
	for (i = 0; i < MAX_OPS; i++)
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

// software decryption should be last, use first hardware
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
	for (i = 0; algo_init_func[i]; i++)
		if (algo_init_func[i])
			algo_init_func[i]();
}

SCW_op *get_op_for_algo(int algo)
{
	int i;
	for (i = 0; i < MAX_OPS; i++)
		if (ops[i].enabled && ops[i].op->algo == algo)
			return ops[i].op;
	return NULL;
}

int get_mask_len(uint8_t *filter, int l)
{
	int len = 0, i;
	for (i = 0; i < l; i++)
		if (filter[i] != 0)
			len = i + 1;
	return len;
}

void dump_filters(int aid)
{
	int i;
	for (i = 0; i < nfilters; i++)
		if (filters[i] && filters[i]->enabled)
			LOG("filter %d A: %d: pid %d, master %d, next %d, flags %d, mask_len %d, filter -> %02X %02X %02X %02X, mask ->%02X %02X %02X %02x",
				filters[i]->id, filters[i]->adapter, filters[i]->pid, filters[i]->master_filter, filters[i]->next_filter, filters[i]->flags, filters[i]->mask_len, filters[i]->filter[0], filters[i]->filter[1], filters[i]->filter[2], filters[i]->filter[3], filters[i]->mask[0], filters[i]->mask[1], filters[i]->mask[2], filters[i]->mask[3]);
}

int add_filter(int aid, int pid, void *callback, void *opaque, int flags)
{
	uint8_t filter[FILTER_SIZE], mask[FILTER_SIZE];
	memset(filter, 0, sizeof(filter));
	memset(mask, 0, sizeof(mask));
	return add_filter_mask(aid, pid, callback, opaque, flags, filter, mask);
}
int add_filter_mask(int aid, int pid, void *callback, void *opaque, int flags, uint8_t *filter, uint8_t *mask)
{
	SFilter *f;
	int i, fid = 0;

	if (pid < 0 || pid > 8191)
		LOG_AND_RETURN(-1, "%s failed, pid %d", __FUNCTION__, pid);

	fid = add_new_lock((void **)filters, MAX_FILTERS, sizeof(SFilter), &filters_mutex);
	if (fid == -1)
		LOG_AND_RETURN(-1, "%s failed", __FUNCTION__);

	mutex_lock(&filters_mutex);
	f = filters[fid];
	f->id = fid;
	f->opaque = opaque;
	f->pid = pid;
	f->callback = callback;
	f->flags = 0;
	f->len = 0;
	f->next_filter = -1;
	f->adapter = aid;

	if (fid >= nfilters)
		nfilters = fid + 1;

	f->master_filter = f->id;

	// this should be syncronized using filters_mutex

	for (i = 0; i < nfilters; i++)
		if (i != fid && filters[i] && filters[i]->enabled && filters[i]->adapter == aid && filters[i]->pid == pid && filters[i]->next_filter == -1)
		{
			filters[i]->next_filter = fid;
			f->master_filter = filters[i]->master_filter;
		}
	set_filter_flags(fid, flags);
	set_filter_mask(fid, filter, mask);
	mutex_unlock(&filters_mutex);
	mutex_unlock(&f->mutex);

	LOG("new filter %d added for adapter %d, pid %d, flags %d, mask_len %d, master_filter %d",
		fid, f->adapter, pid, f->flags, f->mask_len, f->master_filter);

	SPid *p = find_pid(f->adapter, pid);
	if (p)
		p->filter = fid;

	return fid;
}

int reset_master_filter(int adapter, int pid, int id)
{
	int i, nf = 0;
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
	int i, pid;
	int adapter = -1;
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
	mutex_lock(&filters_mutex);
	set_filter_flags(id, 0); // remote all pids if any
	pid = f->pid;
	adapter = f->adapter;
	if (id == f->master_filter)
	{
		int master_filter = f->next_filter;
		SFilter *m = get_filter(master_filter);
		if (!m) // double check there is no other filter
		{
			for (i = 0; i < nfilters; i++)
				if (i != id && (filters[i] && filters[i]->enabled && filters[i]->adapter == f->adapter && filters[i]->pid == pid))
				{
					m = filters[i];
					LOGM("warning: filter %d was also found for pid %d", m->id, pid);
					break;
				}
			SPid *p = find_pid(f->adapter, pid);
			if (p)
				p->filter = -1;
		}
		if (m) // reset master_filter for all filters
			reset_master_filter(f->adapter, pid, m->id);
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
	i = MAX_FILTERS;
	while (--i >= 0)
		if (filters[i] && filters[i]->enabled)
			break;
	nfilters = i + 1;
	f->pid = -1;
	f->enabled = 0;
	mutex_unlock(&filters_mutex);
	mutex_destroy(&f->mutex);
	LOG("deleted filter %d, ad %d, pid %d, max filters %d", id, adapter, pid, nfilters);
	return 0;
}
int get_pid_filter(int aid, int pid)
{
	int i;
	for (i = 0; i < nfilters; i++)
		if (filters[i] && filters[i]->enabled && filters[i]->adapter == aid && filters[i]->pid == pid)
		{
			LOGM("found filter %d for pid %d, master %d (%d)", i, pid, filters[i]->master_filter, nfilters);
			return filters[i]->master_filter;
		}
	return -1;
}

int get_active_filters_for_pid(int master_filter, int aid, int pid, int flags)
{
	SFilter *f;
	int add_remove = 0;
	if (master_filter == -1)
		master_filter = get_pid_filter(aid, pid);
	for (f = get_filter(master_filter); f; f = get_filter(f->next_filter))
		if (f->flags & flags)
			add_remove++;
	LOGM("Found %d filters for adapter %d pid %d master %d with flags %d", add_remove, aid, pid, master_filter, flags);
	return add_remove;
}

int set_filter_flags(int id, int flags)
{
	SFilter *f = get_filter(id);
	if (!f)
		LOG_AND_RETURN(1, "Filter %d not found", id)
	f->flags = flags;
	if (flags & FILTER_ADD_REMOVE)
	{
		SPid *p = find_pid(f->adapter, f->pid);
		if (!p)
			mark_pid_add(-1, f->adapter, f->pid);
	}
	else if (flags == 0)
	{
		int add_remove = get_active_filters_for_pid(f->master_filter, f->adapter, f->pid, FILTER_ADD_REMOVE | FILTER_PERMANENT);
		if (!add_remove)
		{
			SPid *p = find_pid(f->adapter, f->pid);
			if (p && p->flags != 3 && p->sid[0] == -1)
			{
				mark_pid_deleted(f->adapter, -1, f->pid, p);
				update_pids(f->adapter);
			}
			else
				LOGM("pid not found or pid in use by sid %d", p ? p->sid[0] : -1);
		}
	}
	return 0;
}

int set_filter_mask(int id, uint8_t *filter, uint8_t *mask)
{
	SFilter *f = get_filter(id);
	if (f)
	{
		memcpy(f->filter, filter, sizeof(f->filter));
		memcpy(f->mask, mask, sizeof(f->mask));
		f->mask_len = get_mask_len(f->mask, sizeof(f->mask));
	}
	else
		LOGM("Filter %d not found", id);
	return f ? 0 : 1;
}

void delete_filter_for_adapter(int aid)
{
	int i;
	for (i = 0; i < nfilters; i++)
		if (filters[i] && filters[i]->enabled && (filters[i]->adapter == aid) && !(filters[i]->flags & FILTER_PERMANENT))
			del_filter(i);
	return;
}
int match_filter(SFilter *f, unsigned char *b)
{
	int i, match = 1, idx, filter;
	for (i = 0; match && (i < f->mask_len); i++)
	{
		if (i == 0)
			idx = i;
		else
			idx = i + 2;
		filter = f->filter[i] & f->mask[i];
		if ((b[idx] & f->mask[i]) != filter)
		{
			DEBUGM("filter %d, pid %d, index %d did not match: %02X & %02X != %02X inital filter: %02X", f->id, f->pid, i, b[i + 5], f->mask[i], filter, f->filter[i]);
			match = 0;
		}
	}
	if (f->flags & FILTER_REVERSE)
		match = 1 - match;
	DEBUGM("filter %smatch: id %d: pid %d, flags %d, mask_len %d, filter -> %02X %02X %02X %02X %02X, mask ->%02X %02X %02X %02X %02X -> data %02X [%02x %02x] %02X %02X %02X %02X",
		   match ? "" : "not ", f->id, f->pid, f->flags, f->mask_len, f->filter[0], f->filter[1], f->filter[2], f->filter[3], f->filter[4], f->mask[0], f->mask[1], f->mask[2], f->mask[3], f->mask[4], b[0], b[1], b[2], b[3], b[4], b[5], b[6]);
	return match;
}

void process_filter(SFilter *f, unsigned char *b)
{
	int match = 0;
	if (!f || !f->enabled || mutex_lock(&f->mutex))
	{
		LOGM("%s: filter %d not enabled", __FUNCTION__, f->id);
		return;
	}

	if ((b[1] & 0x40))
	{
		if (!(f->flags & FILTER_EMM))
			match = match_filter(f, b + 5);
		DEBUGM("matching pid %d, filter %d, match %d, flags %d, isEMM %d", f->pid, f->id, match, f->flags, (f->flags & FILTER_EMM) > 0);
		f->match = match;
	}
	if (f->match || (f->flags & FILTER_EMM))
	{
		int len = assemble_packet(f, b);
		DEBUGM("assemble_packet returned %d for pid %d", len, f->pid);
		if (!len)
		{
			mutex_unlock(&f->mutex);
			return;
		}
		if (!(f->flags & FILTER_EMM))
			f->callback(f->id, f->data, len, f->opaque);
		else
		{
			int i = 0, cl;
			unsigned char *data = f->data;
			while (i < len)
			{
				if (data[i] < 0x80 || data[i] > 0x8F)
					break;
				cl = (data[i + 1] & 0xF) * 256 + data[i + 2];
				match = match_filter(f, data + i);

				DEBUGM("EMM: match %d id: %d len: %d: %02X %02X %02X %02X %02X", match, f->id, cl + 3, data[i], data[i + 1], data[i + 2], data[i + 3], data[i + 4]);

				if (match)
					f->callback(f->id, data + i, cl + 3, f->opaque);
				i += cl + 3;
			}
		}
	}

	mutex_unlock(&f->mutex);
}
void process_filters(adapter *ad, unsigned char *b, SPid *p)
{
	int pid = PID_FROM_TS(b);
	SFilter *f;
	int filter = p->filter;
	f = get_filter(filter);
	//	DEBUGM("got filter %d for pid (%d) %d master filter %d", filter, pid, p->pid, f ? f->master_filter : -1);
	if (!f || f->master_filter != filter || pid != f->pid)
	{
		p->filter = get_pid_filter(ad->id, pid);
		f = get_filter(p->filter);
	}
	while (f)
	{
		if (f->pid == pid)
			process_filter(f, b);
		else
		{
			LOG("filter %d with pid %d is wrong for pid %d", f->id, f->pid, pid);
			dump_filters(ad->id);
			p->filter = get_pid_filter(ad->id, pid);
		}
		f = get_filter(f->next_filter);
	}
}

int get_filter_pid(int filter)
{
	SFilter *f = get_filter(filter);
	if (f)
		return f->pid;
	return -1;
}
int get_filter_adapter(int filter)
{
	SFilter *f = get_filter(filter);
	if (f)
		return f->adapter;
	return -1;
}

void clear_cw_for_pmt(int master_pmt, int parity)
{
	int i, dcw = 0;
	int64_t ctime = getTick();
	for (i = 0; i < ncws; i++)
		if (cws[i] && cws[i]->enabled && cws[i]->pmt == master_pmt && cws[i]->parity == parity)
		{
			LOG("disabling CW %d, parity %d created %jd ms ago", i, cws[i]->parity, ctime - cws[i]->time);
			if (cws[i]->op->stop_cw)
				cws[i]->op->stop_cw(cws[i], get_pmt(master_pmt));
			cws[i]->enabled = 0;
			dcw++;
		}
	i = MAX_CW;
	while (--i >= 0)
		if (cws[i] && cws[i]->enabled)
			break;
	ncws = i + 1;
}

void expire_cw_for_pmt(int master_pmt, int parity, int64_t min_expiry)
{
	int i;
	int64_t ctime = getTick();
	for (i = 0; i < ncws; i++)
		if (cws[i] && cws[i]->enabled && cws[i]->pmt == master_pmt && cws[i]->parity == parity)
		{
			SCW *cw = cws[i];
			// CW expired
			if (ctime > cw->expiry)
				continue;

			// Wait at least min_expiry after the CW is set
			if (!cw->set_time || ctime - cw->set_time < min_expiry)
				continue;

			LOG("Expiring CW %d, PMT %d, parity %d created %jd ms ago, CW: %02X %02X", i, master_pmt, cws[i]->parity, ctime - cws[i]->time, cws[i]->cw[0], cws[i]->cw[1]);
			cws[i]->expiry = getTick() - 1000;
		}
}

void disable_cw(int master_pmt)
{
	SPMT *pmt = get_pmt(master_pmt);
	if (pmt)
	{
		pmt->cw = NULL;
		pmt->invalidated = 1;
	}
	//	LOGM("disabled %d CWs for PMT %d, valid %d, parity %d", dcw, master_pmt, pmt != NULL, parity);
}
void update_cw(SPMT *pmt)
{
	SPMT *master = get_pmt(pmt->master_pmt);
	SCW *cw = NULL;
	int64_t ctime = getTick();
	int i = 0;
	if (!master)
	{
		LOGM("Master PMT %d does not exist", pmt->master_pmt);
		return;
	}
	if (!pmt->invalidated && !master->invalidated)
	{
		LOGM("PMT %d or pmt %d invalidated", pmt->id, pmt->master_pmt);
		return;
	}
	if (pmt->cw)
	{
		LOGM("Valid CW %d for PMT %d, validating the PMT", pmt->cw->id, pmt->master_pmt);
		pmt->invalidated = 0;
		master->invalidated = 0;
		return;
	}
	LOGM("%s: pmt %d, parity %d, CW %d", __FUNCTION__, pmt->id, pmt->parity, pmt->cw ? pmt->cw->id : -1);
	pmt->cw = NULL;
	for (i = 0; i < ncws; i++)
		if (cws[i] && cws[i]->enabled && (pmt->parity == cws[i]->parity) && (cws[i]->pmt == pmt->id || cws[i]->pmt == master->id))
		{
			int change = 0;
			if (ctime > cws[i]->expiry)
				continue;

			if (!cw)
			{
				cw = cws[i];
				LOGM("candidate CW %d, prio %d, time %jd ms ago, expiry in %jd s, parity %d, pmt %d, change %d", i, cws[i]->prio, ctime - cws[i]->time, cws[i]->expiry - ctime, pmt->parity, cws[i]->pmt, 1);
				continue;
			}
			if (cw->low_prio)
			{
				cw->low_prio = 0;
				cw->prio++;
			}
			if (cws[i]->low_prio)
			{
				cws[i]->low_prio = 0;
				cws[i]->prio++;
			}

			if (cw->prio < cws[i]->prio)
				change = 1;
			// newest CW
			if ((cw->prio == cws[i]->prio) && (cw->time < cws[i]->time))
				change = 1;

			LOGM("candidate CW %d, prio %d, time %jd ms ago, expiry in %jd ms, parity %d, pmt %d, change %d", i, cws[i]->prio, ctime - cws[i]->time, cws[i]->expiry - ctime, pmt->parity, cws[i]->pmt, change);
			if (change)
				cw = cws[i];
		}
	char buf[300];
	sprintf(buf, "not found");
	if (cw)
		sprintf(buf, "%02X %02X %02X %02X %02X %02X %02X %02X", cw->cw[0], cw->cw[1], cw->cw[2], cw->cw[3], cw->cw[4], cw->cw[5], cw->cw[6], cw->cw[7]);
	if (cw && cw->iv[0])
		sprintf(buf + strlen(buf), ", IV: %02X %02X %02X %02X %02X %02X %02X %02X", cw->iv[0], cw->iv[1], cw->iv[2], cw->iv[3], cw->iv[4], cw->iv[5], cw->iv[6], cw->iv[7]);

	LOG("found CW: %d for %s PMT %d, pid %d, master %d, %jd ms ago, parity %d: %s", cw ? cw->id : -1, pmt->name, pmt->id, pmt->pid, master->id, cw ? (getTick() - cw->time) : 0, pmt->parity, buf);
	if (cw)
	{
		mutex_lock(&pmt->mutex);
		pmt->cw = cw;
		pmt->invalidated = 0;
		mutex_unlock(&pmt->mutex);

		mutex_lock(&master->mutex);
		master->cw = cw;
		master->invalidated = 0;
		master->parity = pmt->parity;
		master->invalidated = 0;
		cw->op->set_cw(cw, master);
		mutex_unlock(&master->mutex);
		cw->pmt = master->id;
		if (!cw->set_time)
			cw->set_time = getTick();
	}
	else
	{
		pmt->invalidated = 0;
		master->invalidated = 0;
	}
}

int send_cw(int pmt_id, int algo, int parity, uint8_t *cw, uint8_t *iv, int64_t expiry)
{
	int i, master_pmt;
	char buf[400];
	SCW_op *op = get_op_for_algo(algo);
	buf[0] = 0;
	if (iv)
	{
		sprintf(buf, ", IV: %02X %02X %02X %02X %02X %02X %02X %02X", iv[0], iv[1], iv[2], iv[3], iv[4], iv[5], iv[6], iv[7]);
	}
	SPMT *pmt = get_pmt(pmt_id);
	LOGM("got CW for PMT %d, master %d, algo %d, parity %d: %02X %02X %02X %02X %02X %02X %02X %02X %s", pmt_id, pmt ? pmt->master_pmt : -2, algo, parity,
		 cw[0], cw[1], cw[2], cw[3], cw[4], cw[5], cw[6], cw[7], buf);
	if (!pmt)
		LOG_AND_RETURN(1, "%s: pmt not found %d", __FUNCTION__, pmt_id);
	master_pmt = pmt->master_pmt;
	pmt = get_pmt(master_pmt);
	if (!pmt)
	{
		LOG("%s: master pmt not found %d for pmt %d", __FUNCTION__, master_pmt, pmt_id);
		pmt = get_pmt(pmt_id);
		if (!pmt)
			LOG_AND_RETURN(2, "%s: pmt %d and master pmt not found %d ", __FUNCTION__, pmt_id, master_pmt);
	}
	if (!op)
		LOG_AND_RETURN(3, "op not found for algo %d", algo);

	for (i = 0; i < MAX_CW; i++)
		if (cws[i] && cws[i]->enabled && cws[i]->pmt == master_pmt && cws[i]->parity == parity && !memcmp(cw, cws[i]->cw, cws[i]->cw_len))
			LOG_AND_RETURN(1, "cw already exist at position %d: %02X %02X %02X %02X %02X %02X %02X %02X", i, cw[0], cw[1], cw[2], cw[3], cw[4], cw[5], cw[6], cw[7]);

	int64_t ctime = getTick();
	mutex_lock(&cws_mutex);
	for (i = 0; i < MAX_CW; i++)
		if (!cws[i] || (!cws[i]->enabled && cws[i]->algo == algo) ||
			(cws[i]->enabled && cws[i]->algo == algo && (ctime > cws[i]->expiry)))
			break;
	if (i == MAX_CW)
	{
		LOG("CWS is full %d", i);
		mutex_unlock(&cws_mutex);
		return 1;
	}

	if (!cws[i])
	{
		cws[i] = malloc1(sizeof(SCW));
		if (!cws[i])
		{
			LOG("CWS: could not allocate memory");
			mutex_unlock(&cws_mutex);
			return 2;
		}
		memset(cws[i], 0, sizeof(SCW));
		op->create_cw(cws[i]);
	}
	SCW *c = cws[i];

	if (c->enabled && c->op->stop_cw)
		c->op->stop_cw(cw, pmt);

	c->id = i;
	c->adapter = pmt->adapter;
	c->parity = parity;
	c->pmt = master_pmt;
	c->cw_len = 16;
	c->prio = 0;
	c->low_prio = 0;
	c->time = getTick();
	c->set_time = 0;
	if (expiry == 0)
		c->expiry = c->time + MAX_CW_TIME;
	else
		c->expiry = c->time + expiry * 1000;

	if ((pmt->parity == parity) && (ctime - pmt->last_parity_change > 2000))
	{
		c->prio = -1;
		c->low_prio = 1;
	}
	if (algo < 2)
		c->cw_len = 8;
	c->algo = algo;
	memcpy(c->cw, cw, c->cw_len);
	if (iv)
		memcpy(c->iv, iv, c->cw_len);
	else
		memset(c->iv, 0, sizeof(c->iv));
	c->op = op;
	c->enabled = 1;
	if (i >= ncws)
		ncws = i + 1;

	mutex_unlock(&cws_mutex);
	pmt->invalidated = 1;
	LOG("CW %d for %s PMT %d, master %d, pid %d, expiry in %jd ms, algo %d, parity %d: %02X %02X %02X %02X %02X %02X %02X %02X", c->id, pmt->name, pmt_id, master_pmt, pmt->pid, cws[i]->expiry - getTick(), algo, parity,
		cw[0], cw[1], cw[2], cw[3], cw[4], cw[5], cw[6], cw[7]);
	return 0;
}

#ifndef DISABLE_TABLES

int set_all_pmt_encrypted(int aid, int pid, int status)
{
	int i;
	SPMT *pmt;
	for (i = 0; i < npmts; i++)
		if (pmts[i] && pmts[i]->enabled && pmts[i]->adapter == aid && pmts[i]->first_active_pid == pid && pmts[i]->encrypted != status)
		{
			pmt = pmts[i];
			pmt->encrypted = status;
			tables_update_encrypted_status(get_adapter(aid), pmt);
		}
	return 0;
}

void check_packet_encrypted(adapter *ad)
{
	int i, cp, pid;
	int64_t ctime = getTick();
	uint16_t enc[8192], dec[8192];
	uint8_t *b;
	memset(enc, 0, sizeof(enc));
	memset(dec, 0, sizeof(dec));
	for (i = 0; i < ad->rlen; i += DVB_FRAME)
	{
		b = ad->buf + i;
		if (b[1] & 0x80)
			return;
		if (b[0] == 0x47 && (b[1] & 0x40))
		{
			int pid = PID_FROM_TS(b);
			int start = 4;
			cp = ((b[3] & 0x40) > 0);
			if (b[3] & 0x20)
				start = 5 + b[4];
			if (start + 3 > 188)
				return;
			if (b[start] != 0 && (b[start + 1] != 0 || b[start + 2] != 1))
			{
				enc[pid]++;
				DEBUGM("decryption failed for pid %d, parity %d, start %d: %02X %02X %02X %02X %02X %02X %02X %02X", pid, cp, start, b[start], b[start + 1], b[start + 2], b[start + 3], b[start + 4], b[start + 5], b[start + 6], b[start + 7])
			}
			else if (b[start] && b[start + 1] && (b[start + 2] == 1))
			{
				dec[pid]++;
				DEBUGM("decryption worked for pid %d, parity %d, start %d: [%02X %02X %02X %02X] %02X %02X %02X %02X %02X %02X %02X %02X", pid, cp, start, b[0], b[1], b[2], b[3], b[start], b[start + 1], b[start + 2], b[start + 3], b[start + 4], b[start + 5], b[start + 6], b[start + 7])
			}
		}
	}
	for (pid = 0; pid < 8192; pid++)
		if ((enc[pid] > 2 && dec[pid] == 0) || (enc[pid] == 0 && dec[pid] > 2))
		{
			SPid *p = find_pid(ad->id, pid);
			// Skip PMT pids or pids without pmt
			if (p && p->pmt < 0)
				continue;
			SPMT *pmt = get_pmt_for_existing_pid(p);
			if (!pmt)
				continue;
			int64_t grace_time = pmt->grace_time - ctime;

			LOGM("Found pid %d dec %d enc %d pmt %d first pid %d, grace time %jd ms",
				 pid, dec[pid], enc[pid], pmt->id, pmt->first_active_pid, grace_time > 0 ? grace_time : 0);

			if (pmt->first_active_pid != pid)
				continue;

			if (grace_time > 0)
				continue;

			set_all_pmt_encrypted(ad->id, pid, enc[pid] > 0 ? TABLES_CHANNEL_ENCRYPTED : TABLES_CHANNEL_DECRYPTED);
		}
}

#endif

int decrypt_batch(SPMT *pmt)
{
	unsigned char *b = NULL;
	int pid = 0;
	if (pmt->blen <= 0)
		return 0;
	mutex_lock(&pmt->mutex);
	if (!pmt->cw)
	{
		LOG("No CW found for pmt %d parity %d pid %d, blen %d", pmt->id, pmt->parity, pmt->pid, pmt->blen);
		pmt->blen = 0;
		mutex_unlock(&pmt->mutex);
		return 1;
	}
	b = pmt->batch[0].data - 4;
	pid = PID_FROM_TS(b);
	pmt->batch[pmt->blen].data = NULL;
	pmt->batch[pmt->blen].len = 0;
	pmt->cw->op->decrypt_stream(pmt->cw, pmt->batch, 184);
	DEBUGM("pmt: decrypted key %d, CW %d, parity %d at len %d, channel_id %d (pid %d) %p",
		   pmt->id, pmt->cw->id, pmt->parity, pmt->blen, pmt->sid, pid, pmt->batch[0].data);
	pmt->blen = 0;
	//	memset(pmt->batch, 0, sizeof(int *) * 128);
	mutex_unlock(&pmt->mutex);
	return 0;
}

int pmt_decrypt_stream(adapter *ad)
{
	SPMT *pmt = NULL, *master = NULL;
	int adapt_len;
	int batchSize = 0;
	// max batch
	int i = 0;
	unsigned char *b;
	SPid *p = NULL;
	int pid;
	int cp;
	int rlen = ad->rlen;

	for (i = 0; i < rlen; i += 188)
	{
		b = ad->buf + i;
		pid = PID_FROM_TS(b);
		if (b[3] & 0x80)
		{
			p = find_pid(ad->id, pid);
			pmt = get_pmt_for_existing_pid(p);
			if (!pmt)
			{
				DEBUGM("PMT not found for pid %d, id %d, packet %d, pos %d", pid, p ? p->pmt : -3, i / 188, i);
				continue; // cannot decrypt
			}
			master = get_pmt(pmt->master_pmt);
			if (!master)
				master = pmt;

			cp = ((b[3] & 0x40) > 0);
			if (pmt->parity == -1)
				pmt->parity = cp;

			if (!pmt->cw || !pmt->cw->enabled || pmt->cw->pmt != pmt->master_pmt || master->invalidated || pmt->invalidated)
			{
				if (pmt->invalidated || master->invalidated)
					update_cw(pmt);
			}
			if (!pmt->cw)
			{
				DEBUGM("pmt %d channel %s CW not found, parity %d, packet parity %d", pmt->id, pmt->name, pmt->parity, cp);
				p->dec_err++;
				if (pmt->parity == cp) // allow the parity change to be processed if the CW is invalid
					continue;
			}

			if (!batchSize && pmt->cw)
			{
				batchSize = pmt->cw->op->batch_size();
				if (batchSize > MAX_BATCH_SIZE)
					batchSize = MAX_BATCH_SIZE;
			}
			if ((pmt->parity != cp) || (pmt->blen >= batchSize)) // partiy change or batch buffer full
			{
				int old_parity = pmt->parity;
				decrypt_batch(pmt);
				if (old_parity != cp)
				{
					int64_t ctime = getTick();
					LOG("Parity change for %s PMT %d, new active parity %d pid %d [%02X %02X %02X %02X], last_parity_change %jd",
						pmt->name, pmt->id, cp, pid, b[0], b[1], b[2], b[3],
						pmt->last_parity_change);
					pmt->last_parity_change = ctime;
					master->last_parity_change = ctime;
					pmt->parity = cp;
					disable_cw(pmt->master_pmt);
					update_cw(pmt);
				}
			}

			if (b[3] & 0x20)
			{
				adapt_len = (b[4] < 183) ? b[4] + 5 : 188;
				//				DEBUGM("Adaptation for pid %d, len %d, packet %d",
				//					   pid, adapt_len, i / 188);
			}
			else
				adapt_len = 4;
			if (adapt_len < 188)
			{
				pmt->batch[pmt->blen].data = b + adapt_len;
				pmt->batch[pmt->blen++].len = 188 - adapt_len;
			}
			//			DEBUGM("clear encrypted flags for pid %d", pid);
			b[3] &= 0x3F; // clear the encrypted flags
		}
	}

	for (i = 0; i < npmts; i++) // decrypt everything that's left
		if (pmts[i] && pmts[i]->enabled && (pmts[i]->blen > 0) && (pmts[i]->adapter == ad->id))
			decrypt_batch(pmts[i]);
	return 0;
}

int pmt_process_stream(adapter *ad)
{
	SPid *p;
	int i, pid;
	uint8_t *b;

	int rlen = ad->rlen;
	ad->null_packets = 0;

	for (i = 0; i < rlen; i += DVB_FRAME)
	{
		b = ad->buf + i;
		pid = PID_FROM_TS(b);
		p = find_pid(ad->id, pid);
		if (p && (p->filter != -1))
		{
			process_filters(ad, b, p);
		}
		if (opts.emulate_pids_all && pid == 0)
		{
			p = find_pid(ad->id, 8192);
			if (p)
				process_filters(ad, b, p);
		}
	}
#ifndef DISABLE_TABLES

	if (ad->ca_mask == 0) // no CA enabled on this adapter
		return 0;

	tables_ca_ts(ad);
	pmt_decrypt_stream(ad);

	check_packet_encrypted(ad);

	if (ad->ca_mask && ad->drop_encrypted)
	{
		for (i = 0; i < ad->rlen; i += DVB_FRAME)
		{
			b = ad->buf + i;
			pid = PID_FROM_TS(b);
			if ((b[3] & 0x80) == 0x80)
			{
				if (opts.debug & (DEFAULT_LOG | LOG_DMX))
					LOG("Marking PID %d packet %d pos %d as NULL", pid, i / 188, i);
				b[1] |= 0x1F;
				b[2] |= 0xFF;
				ad->dec_err++;
				ad->null_packets = 1;
			}
			//			else
			//				DEBUGL(LOG_DMX, "PID %d packet %d pos %d not marked: %02X %02X %02X %02X", pid, i / 188, i, b[0], b[1], b[2], b[3]);
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
	pmt->version = -1;
	pmt->invalidated = 1;
	pmt->skip_first = 1;
	pmt->active = 0;
	pmt->cw = NULL;
	pmt->opaque = NULL;
	pmt->first_active_pid = -1;
	pmt->ca_mask = pmt->disabled_ca_mask = 0;
	pmt->name[0] = pmt->provider[0] = 0;
	pmt->clean_pos = pmt->clean_cc = 0;

	pmt->active_pids = 0;
	memset(pmt->active_pid, 0, sizeof(pmt->active_pids));
	pmt->stream_pids = 0;
	memset(pmt->stream_pid, 0, sizeof(pmt->stream_pid));

	if (i >= npmts)
		npmts = i + 1;

	mutex_unlock(&pmt->mutex);
	LOG("returning new pmt %d for adapter %d, pmt pid %d sid %d %04X", i, adapter,
		pmt_pid, sid, sid);

	return i;
}

int pmt_del(int id)
{
	int aid, i;
	SPMT *pmt;
	int master_pmt;
	pmt = get_pmt(id);
	if (!pmt)
		return 0;

#ifndef DISABLE_TABLES
	close_pmt_for_cas(get_adapter(pmt->adapter), pmt);
#endif

	mutex_lock(&pmt->mutex);
	if (!pmt->enabled)
	{
		mutex_unlock(&pmt->mutex);
		return 0;
	}
	LOG("deleting PMT %d, master PMT %d, name %s ", pmt->id, pmt->master_pmt, pmt->name);
	master_pmt = pmt->master_pmt;

	if (master_pmt == id)
	{
		clear_cw_for_pmt(master_pmt, 0);
		clear_cw_for_pmt(master_pmt, 1);
	}

	aid = pmt->adapter;
	pmt->enabled = 0;

	pmt->sid = 0;
	pmt->pid = 0;
	pmt->adapter = -1;

	i = MAX_PMT;
	while (--i >= 0)
		if (pmts[i] && pmts[i]->enabled)
			break;
	npmts = i + 1;

	mutex_destroy(&pmt->mutex);
	if (master_pmt != id) // delete all linked pmts
	{
		int i;
		for (i = 0; i < npmts; i++)
			if (pmts[i] && pmts[i]->enabled && (pmts[i]->adapter == aid) && (pmts[i]->id == master_pmt || pmts[i]->master_pmt == master_pmt))
				pmt_del(i);
	}
	return 0;
}

int clear_pmt_for_adapter(int aid)
{
	uint8_t filter[FILTER_SIZE], mask[FILTER_SIZE];
	adapter *ad = get_adapter(aid);
	delete_pmt_for_adapter(aid);
	delete_filter_for_adapter(aid);
	if (ad)
	{
		ad->pat_processed = 0;
		ad->transponder_id = 0;
		ad->pat_ver = 0;
		memset(filter, 0, FILTER_SIZE);
		memset(mask, 0, FILTER_SIZE);
		set_filter_mask(ad->pat_filter, filter, mask);
		set_filter_flags(ad->pat_filter, FILTER_PERMANENT | FILTER_CRC);
		set_filter_mask(ad->sdt_filter, filter, mask);
		set_filter_flags(ad->sdt_filter, FILTER_PERMANENT | FILTER_CRC);
	}
	return 0;
}

int delete_pmt_for_adapter(int aid)
{
	int i;
	for (i = 0; i < npmts; i++)
		if (pmts[i] && pmts[i]->enabled && pmts[i]->adapter == aid)
			pmt_del(i);
	return 0;
}

SPMT *get_pmt_for_sid(int aid, int sid)

{
	int i;
	for (i = 0; i < npmts; i++)
		if (pmts[i] && pmts[i]->enabled && pmts[i]->adapter == aid && pmts[i]->sid == sid)
			return pmts[i];
	return NULL;
}

void mark_pid_null(uint8_t *b)
{
	b[1] |= 0x1F;
	b[2] |= 0xFF;
}

int clean_psi_buffer(uint8_t *pmt, uint8_t *clean, int clean_size)
{
	uint8_t *n, *o;
	int nlen = 0, i, j, es_len, desc_len;
	uint32_t crc;
	memset(clean, -1, clean_size);
	memcpy(clean, pmt, 12);
	int pi_len = ((pmt[10] & 0xF) << 8) + pmt[11];
	int pmt_len = ((pmt[1] & 0xF) << 8) + pmt[2];
	int ver = (clean[5] & 0x3e) >> 1;
	ver = (ver + 1) & 0xF;
	clean[5] = (0xC0 & clean[5]) | (ver << 1);
	n = clean;
	o = pmt + pi_len + 12;
	nlen = 12;
	n[10] &= 0xF0; // pi_len => 0
	n[11] &= 0x0;
	if (pi_len > 1500)
		LOG_AND_RETURN(0, "Program Info Length is too big %d", pi_len);

	for (i = 0; i < pmt_len - pi_len - 17; i += (es_len) + 5) // reading streams
	{
		uint8_t *t = o + i;
		int init_len = nlen + 5;
		es_len = (o[i + 3] & 0xF) * 256 + o[i + 4];
		DEBUGM("es: copy 5 bytes from %d -> %d : %02X %02X %02X %02X %02X",
			   i, nlen, t[0], t[1], t[2], t[3], t[4]);
		memcpy(n + nlen, o + i, 5);
		nlen += 5;
		for (j = 0; j < es_len; j += desc_len) // reading program info
		{
			desc_len = o[i + 5 + j + 1] + 2;
			if (o[i + 5 + j] != 9)
			{
				t = o + i + 5 + j;
				DEBUGM("desc copy %d bytes from %d -> %d : %02X %02X %02X",
					   desc_len, i + 5 + j, nlen, t[0], t[1], t[2]);
				memcpy(n + nlen, o + i + 5 + j, desc_len);
				nlen += desc_len;
			}
		}
		int nes_len = nlen - init_len;
		DEBUGM("clean_psi: setting the new es_len %d at position %d",
			   nes_len, init_len - 2);
		n[init_len - 2] = (n[init_len - 2] & 0xF0) | ((nes_len >> 8) & 0xF);
		n[init_len - 1] = (nes_len)&0xFF;
	}
	nlen += 4 - 3;
	DEBUGM("new len is %d, old len is %d", nlen, pmt_len);
	n[1] &= 0xF0;
	n[1] |= (nlen >> 8);
	n[2] = nlen & 0xFF;
	n[5] ^= 0x3F; // change version

	crc = crc_32(n, nlen - 1);
	copy32(n, nlen - 1, crc);
	return nlen + 3;
}

void clean_psi(adapter *ad, uint8_t *b)
{
	int pid = PID_FROM_TS(b);
	int pmt_len;
	int clean_size = 1500;
	uint8_t *clean, *pmt;
	SPid *p;
	SPMT *cpmt;

	p = find_pid(ad->id, pid);
	if (!p || !VALID_SID(p->sid[0])) // no need to fix this PMT as it not requested by any stream
		return;

	if (!(cpmt = get_pmt(-p->pmt))) // no key associated with PMT - most likely the channel is clear
		return;

	if (!(cpmt->cw) || !cpmt->running)
	{
		//		mark_pid_null(b);
		return;
	}
	if (!cpmt->clean)
		cpmt->clean = malloc1(clean_size + 10);
	clean = cpmt->clean;

	pmt_len = cpmt->pmt_len;
	pmt = cpmt->pmt;

	if (!clean)
	{
		//		mark_pid_null(b);
		return;
	}

	if (pmt) // populate clean psi
	{
		clean_psi_buffer(pmt, clean, clean_size);
	}

	if (clean)
	{
		pmt_len = ((clean[1] & 0xF) << 8) + clean[2];
		if (b[1] & 0x40)
			cpmt->clean_pos = 0;
		if (cpmt->clean_pos > pmt_len || cpmt->clean_pos < 0)
		{
			mark_pid_null(b);
			return;
		}
		if (cpmt->clean_pos == 0)
		{
			memcpy(b + 5, clean, 183);
			cpmt->clean_pos = 183;
		}
		else
		{
			memcpy(b + 4, clean + cpmt->clean_pos, 184);
			cpmt->clean_pos += 184;
		}
		cpmt->clean_cc = (cpmt->clean_cc + 1) & 0xF;
		b[3] = (b[3] & 0xF0) | cpmt->clean_cc;
	}
}

int getEMMlen(unsigned char *b, int len)
{
	int i = 0, cl, emms = 0;
	while (i < len)
	{
		if (b[i] < 0x80 || b[i] > 0x8F)
			break;
		cl = (b[i + 1] & 0xF) * 256 + b[i + 2];
		i += cl + 3;
		emms++;
	}
	DEBUGM("returning EMM len %d (%X) from %d, found %d emms", i, i, len, emms);
	return i;
}

int assemble_emm(SFilter *f, uint8_t *b)
{
	int len = 0;
	if (b[4] == 0 && (b[5] >= 0x82 && b[5] <= 0x8F))
	{
		f->len = 0;
		memset(f->data, 0, FILTER_PACKET_SIZE);
		memcpy(f->data + f->len, b + 5, 183);
		f->len += 183;
	}
	else
	{
		// f >1500
		if (f->len + 183 > FILTER_PACKET_SIZE)
		{
			LOG("%s: data too large %d", __FUNCTION__, f->len + 184);
			f->len = 0;
			return 0;
		}
		memcpy(f->data + f->len, b + 5, 183);
		f->len += 183;
	}
	//	hexdump("emm: ", b, 188);
	len = getEMMlen(f->data, f->len);
	if (f->len < len)
		return 0;
	//	hexdump("emm full: ", f->data + len - 188, 190);

	return len;
}

int assemble_normal(SFilter *f, uint8_t *b)
{
	int len = 0, pid;
	pid = PID_FROM_TS(b);
	if ((b[1] & 0x40) == 0x40)
	{
		len = ((b[6] & 0xF) << 8) + b[7];
		len = len + 8 - 5; // byte 8 - 5 bytes that we skip
		f->len = 0;
		memset(f->data, 0, FILTER_PACKET_SIZE);
	}
	if ((len > 1500 || len < 0))
		LOG_AND_RETURN(0,
					   "assemble_packet: len %d not valid for pid %d [%02X %02X %02X %02X %02X %02X]",
					   len, pid, b[3], b[4], b[5], b[6], b[7], b[8]);

	if (len > 183)
	{
		memcpy(f->data + f->len, b + 5, 183);
		f->len += 183;
		return 0;
	}
	else if (len > 0)
	{
		memcpy(f->data + f->len, b + 5, len);
		f->len += len;
	}
	else // pmt_len == 0 - next part from the pmt
	{
		if (f->len + 184 > FILTER_PACKET_SIZE)
		{
			LOG("%s: data too large %d", __FUNCTION__, f->len + 184);
			f->len = 0;
			return 0;
		}
		memcpy(f->data + f->len, b + 4, 184);
		f->len += 184;
		len = ((f->data[1] & 0xF) << 8) + f->data[2];
		len += 3;
		if (f->len < len)
			return 0;
	}
	return len;
}

int assemble_packet(SFilter *f, uint8_t *b)
{
	int len = 0, pid;
	uint32_t crc;

	if ((b[0] != 0x47)) // make sure we are dealing with TS
		return 0;

	pid = PID_FROM_TS(b);
	if (f->flags & FILTER_EMM)
		len = assemble_emm(f, b);
	else
		len = assemble_normal(f, b);

	b = f->data;
	if ((len > 0) && (f->flags & FILTER_CRC)) // check the crc for PAT and PMT
	{
		int current_crc;
		if (len < 4 || len > FILTER_PACKET_SIZE)
			LOG_AND_RETURN(0, "assemble_packet: CRC check: flags %d len %d not valid for pid %d [%02X %02X %02X %02X %02X %02X]",
						   f->flags, len, pid, b[0], b[1], b[2], b[3], b[4], b[5]);
		crc = crc_32(b, len - 4);
		copy32r(current_crc, b, len - 4);
		if (crc != current_crc)
		{
			LOG("pid %d (%04X) CRC failed %08X != %08X len %d",
				pid, pid, crc, current_crc, len);
			hexdump("packet failed CRC ", b, len);
			return 0;
		}
	}
	return len;
}

int process_pat(int filter, unsigned char *b, int len, void *opaque)
{
	int pat_len = 0, i, tid = 0, pid, sid, ver;
	adapter *ad = (adapter *)opaque;
	uint8_t new_filter[FILTER_SIZE], new_mask[FILTER_SIZE];
	uint8_t sids[0xFFFF];
	int new_version = 0;
	pat_len = len - 4; // remove crc
	tid = b[3] * 256 + b[4];
	ver = (b[5] & 0x3e) >> 1;
	//	LOGM("PAT: tid %d (%d) ver %d (%d) pat_len %d : %02X %02X %02X %02X %02X %02X %02X %02X", tid, ad ? ad->transponder_id : -1, ver, ad ? ad->pat_ver : -1, pat_len, b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7]);
	if (b[0] != 0)
		return 0;

	if (!ad->enabled)
		LOG_AND_RETURN(0, "Adapter %d no longer enabled, not processing PAT", ad->id);

	if ((ad->transponder_id == tid) && (ad->pat_ver == ver)) //pat already processed
		LOG_AND_RETURN(0, "PAT AD %d: tsid %d version %d", ad->id, tid, ver);

	memset(sids, 0, sizeof(sids));

	if (ad->pat_processed && ad->transponder_id == tid && ad->pat_ver != ver)
	{
		LOG("PAT AD %d new version for transponder %d, version %d", ad->id, ad->transponder_id, ver);
		new_version = 1;
	}

	if (ad->pat_processed && ad->transponder_id != tid)
	{
		LOG("PAT AD %d new transponder %d (old %d), version %d", ad->id, tid, ad->transponder_id, ad->pat_ver);
		clear_pmt_for_adapter(ad->id);
		ad->pat_processed = 0;
	}

	ad->pat_ver = ver;
	ad->transponder_id = tid;

	memset(new_filter, 0, sizeof(new_filter));
	memset(new_mask, 0, sizeof(new_mask));
	new_filter[1] = b[3];
	new_mask[1] = 0xFF;
	new_filter[2] = b[4];
	new_mask[2] = 0xFF;
	new_filter[3] = b[5];
	new_mask[3] = 0xFF;
	set_filter_mask(filter, new_filter, new_mask);
	set_filter_flags(filter, FILTER_PERMANENT | FILTER_REVERSE | FILTER_CRC);
	pat_len -= 9;
	b += 8;
	LOG("PAT Adapter %d, Transponder ID %d, version %d, len %d", ad->id,
		tid, ad->pat_ver, pat_len);
	if (pat_len > 1500)
		return 0;

	for (i = 0; i < pat_len; i += 4)
	{
		int new_pmt = 1;
		sid = b[i] * 256 + b[i + 1];
		pid = (b[i + 2] & 0x1F) * 256 + b[i + 3];
		sids[sid] = 1;
		LOG("Adapter %d, PMT sid %d (%04X), pid %d", ad->id, sid, sid, pid);

		if (new_version)
			new_pmt = !get_pmt_for_sid(ad->id, sid);

		if (sid > 0 && new_pmt)
		{
			int pmt_id = pmt_add(-1, ad->id, sid, pid);
			SPid *p = find_pid(ad->id, pid);
			SPMT *pmt = get_pmt(pmt_id);
			if (pmt && p) // already added PMTs are processed first
			{
				pmt->skip_first = 0;
			}
			memset(new_filter, 0, sizeof(new_filter));
			memset(new_mask, 0, sizeof(new_mask));
			new_filter[1] = b[i];
			new_mask[1] = 0xFF;
			new_filter[2] = b[i + 1];
			new_mask[2] = 0xFF;
			if (pmt)
				pmt->filter = add_filter_mask(ad->id, pid, (void *)process_pmt, pmt, opts.pmt_scan ? FILTER_ADD_REMOVE | FILTER_CRC : 0, new_filter, new_mask);
			else
				LOG("could not add PMT pid %d sid %d (%X) for processing", pid, sid, sid);
		}
	}

	if (new_version)
	{
		for (i = 0; i < npmts; i++)
			if (pmts[i] && pmts[i]->enabled && pmts[i]->adapter == ad->id && sids[pmts[i]->sid] == 0)
			{
				LOG("Deleting PMT %d (%s) and filter %d as it is not present in the new PAT", i, pmts[i]->name, pmts[i]->filter);
				del_filter(pmts[i]->filter);
				pmt_del(i);
			}
	}

	update_pids(ad->id);
	ad->pat_processed = 1;
	return 0;
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
		if (es[i] == 0x6A || es[i] == 0x7A)
			isAC3 = 1;
	}

	return isAC3;
}

void find_pi(SPMT *pmt, unsigned char *es, int len)
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
		if (!pi_exist(capid, caid, pmt->pi, pmt->pi_len))
		{
			if (pmt->pi_len + es_len > sizeof(pmt->pi) - 2)
			{
				LOG("PI is too small %zd", sizeof(pmt->pi));
				return;
			}
			LOG("PI pos %d caid %04X => pid %04X (%d), index %d", pmt->pi_len, caid, capid,
				capid, pmt->caids);
			memcpy(pmt->pi + pmt->pi_len, es + i, es_len);
			pmt->pi_len += es_len;
			if (pmt->caids < MAX_CAID - 1)
			{
				pmt->caid[pmt->caids] = caid;
				pmt->capid[pmt->caids++] = capid;
			}
			else
				LOG("Too many CAIDs for pmt %d, discarding %04X", pmt->id, caid);
		}
	}
	return;
}

int get_master_pmt_for_pid(int aid, int pid)
{
	int i, j;
	SPMT *pmt;
	for (i = 0; i < npmts; i++)
		if (pmts[i] && pmts[i]->enabled && pmts[i]->adapter == aid)
		{
			pmt = pmts[i];
			DEBUGM("searching pid %d ad %d in pmt %d, active pids %d", pid, aid, pmt->id, pmt->active_pids);
			for (j = 0; j < pmt->active_pids && pmt->active_pid[j] > 0; j++)
			{
				DEBUGM("comparing with pid %d", pmt->active_pid[j]);
				if (pmt->active_pid[j] == pid)
				{
					LOGM("%s: ad %d found pid %d in master pmt %d", __FUNCTION__, aid, pid, pmt->master_pmt);
					return pmt->master_pmt;
				}
			}
		}
	LOGM("%s: no pmt found for pid %d adapter %d", __FUNCTION__, pid, aid);
	return -1;
}

int process_pmt(int filter, unsigned char *b, int len, void *opaque)
{
	int pi_len = 0, isAC3, pmt_len = 0, i, es_len, ver;
	int enabled_channels = 0;
	unsigned char *pmt_b, *pi;
	int pid, spid, stype;
	SPid *p, *cp, *p_all;
	SFilter *f;
	adapter *ad;
	int opmt;
	SPMT *pmt = (void *)opaque;

	if (b[0] != 2)
		return 0;

	if (!pmt || !pmt->enabled)
	{
		LOG("PMT %d does not exist", pmt ? pmt->id : -1);
		return 0;
	}
	if (pmt->skip_first)
	{
		pmt->skip_first = 0;
		return 0;
	}

	pid = get_filter_pid(filter);
	ad = get_adapter(pmt->adapter);
	ver = (b[5] & 0x3e) >> 1;

	f = get_filter(filter);
	if (f && f->adapter != pmt->adapter)
		LOG("Adapter mismatch %d != %d", f->adapter, pmt->adapter);

	if (pmt->version == ver)
	{
#ifndef DISABLE_TABLES
		if (ad && pmt->pi_len && pmt->running && ad->ca_mask)
			send_pmt_to_cas(ad, pmt);
#endif
		// just for testing purposes
		p = find_pid(pmt->adapter, pid);
		if (p)
			p->pmt = -pmt->id;

		if (ad && p && opts.clean_psi)
			clean_psi(ad, b);

		return 0;
	}

	if (!ad)
	{
		LOG("Adapter %d does not exist", pmt->adapter);
		return 0;
	}

	memset(pmt->pi, 0, sizeof(pmt->pi));
	memset(pmt->pmt, 0, sizeof(pmt->pmt));
	memcpy(pmt->pmt, b, len);
	pmt->pmt_len = len;
	pmt->pi_len = 0;

	if (!(p = find_pid(ad->id, pid)))
		return -1;

	p_all = find_pid(ad->id, 8192);

	pmt_len = len - 4;

	pi_len = ((b[10] & 0xF) << 8) + b[11];

	pmt->sid = b[3] * 256 + b[4];
	pmt->version = ver;

	mutex_lock(&pmt->mutex);
	LOG("new PMT %d AD %d, pid: %04X (%d), len %d, pi_len %d, ver %d, sid %04X (%d) %s %s", pmt->id, ad->id, pid, pid,
		pmt_len, pi_len, ver, pmt->sid, pmt->sid, pmt->name[0] ? "channel:" : "", pmt->name);
	pi = b + 12;
	pmt_b = pi + pi_len;

	if (pi_len > pmt_len)
		pi_len = 0;

	pmt->caids = 0;
	pmt->stream_pids = 0;

	if (pi_len > 0)
		find_pi(pmt, pi, pi_len);

	es_len = 0;
	pmt->active_pids = 0;
	pmt->active = 1;
	for (i = 0; i < pmt_len - pi_len - 12; i += (es_len) + 5) // reading streams
	{
		es_len = (pmt_b[i + 3] & 0xF) * 256 + pmt_b[i + 4];
		stype = pmt_b[i];
		spid = (pmt_b[i + 1] & 0x1F) * 256 + pmt_b[i + 2];
		isAC3 = 0;
		if (stype == 6)
			isAC3 = is_ac3_es(pmt_b + i + 5, es_len);
		else if (stype == 129)
			isAC3 = 1;

		if (pmt->stream_pids < MAX_PMT_PIDS - 1)
		{
			pmt->stream_pid[pmt->stream_pids].type = stype;
			pmt->stream_pid[pmt->stream_pids++].pid = spid;
		}
		else
			LOG("Too many pids for pmt %d, discarding pid %d", pmt->id, spid);

		LOG("PMT pid %d - stream pid %04X (%d), type %d%s, es_len %d, pos %d, pi_len %d",
			pid, spid, spid, stype, isAC3 ? " [AC3]" : "", es_len, i, pmt->pi_len);

		if ((es_len + i + 5 > pmt_len) || (es_len < 0))
		{
			LOGM("pmt processing complete, es_len + i %d, len %d, es_len %d", es_len + i, pmt_len, es_len);
			break;
		}
		int is_video = (stype == 2) || (stype == 27) || (stype == 36) || (stype == 15);
		int is_audio = isAC3 || (stype == 3) || (stype == 4);
		if (!is_audio && !is_video)
			continue;

		// is video stream
		if (pmt->first_active_pid < 0 && is_video)
			pmt->first_active_pid = spid;

		find_pi(pmt, pmt_b + i + 5, es_len);

		opmt = get_master_pmt_for_pid(ad->id, spid);
		if (opmt != -1 && opmt != pmt->master_pmt)
		{
			pmt->master_pmt = opmt;
			LOG("master pmt %d set for pmt %d", opmt, pmt->id);
		}

		if (pmt->active_pids < MAX_ACTIVE_PIDS - 1)
			pmt->active_pid[pmt->active_pids++] = spid;

		if ((cp = find_pid(ad->id, spid))) // the pid is already requested by the client
		{
			enabled_channels++;
			pmt->running = 1;
			cp->pmt = pmt->master_pmt;
		}
	}

	if (pmt->first_active_pid < 0)
		pmt->first_active_pid = pmt->active_pid[0];

	if (p_all && opts.emulate_pids_all)
	{
		int i, j;
		for (i = 0; i < MAX_STREAMS_PER_PID; i++)
			if (p_all->sid[i] >= 0)
				for (j = 0; j < pmt->stream_pids; j++)
					mark_pid_add(p_all->sid[i], ad->id, pmt->stream_pid[j].pid);

		enabled_channels = 1;
		pmt->running = 1;
		update_pids(ad->id);
	}

	if ((pmt->pi_len > 0) && enabled_channels) // PMT contains CA descriptor and there are active pids
	{
#ifndef DISABLE_TABLES
		if (pmt->sid > 0)
			start_pmt(pmt, ad);
		else
			LOG("PMT %d, SID is 0, not running ca_action", pid);
#endif
	}

	if (!pmt->running)
		set_filter_flags(filter, 0);

	mutex_unlock(&pmt->mutex);

	return 0;
}

void copy_en300468_string(char *dest, int dest_len, char *src, int len)
{
	int start = (src[0] < 0x20) ? 1 : 0;
	int charset = 0;
	int i;
	if (src[0] == 0x10)
		start += 2;

	if (src[0] < 0x20)
		charset = src[0];

	for (i = start; (i < len) && (--dest_len > 0); i++)
	{
		int c = (unsigned char)src[i];
		c |= (charset << 8);

		switch (c)
		{ // default latin -> charset = 0
		case 0x80 ... 0x85:
		case 0x88 ... 0x89:
		case 0x8B ... 0x9F:
		case 0x8A:
		case 0x1680 ... 0x1685: //ISO/IEC 10646 [16]
		case 0x1688 ... 0x1689:
		case 0x168B ... 0x169F:
		case 0x168A:
			*dest++ = '\n';
			continue;
		case 0x86 ... 0x87: // ignore emphasis
			continue;
		case 0xC2:
		case 0x16E0:
			continue;
		}
		*dest++ = src[i];
	}
	*dest = 0;
}

int process_sdt(int filter, unsigned char *sdt, int len, void *opaque)
{
	int i, j, tsid, sdt_len, sid, desc_loop_len, desc_len;
	SPMT *pmt;
	unsigned char *b;
	uint8_t new_filter[FILTER_SIZE], new_mask[FILTER_SIZE];

	if (sdt[0] != 0x42)
		return 0;

	adapter *ad = (void *)opaque;
	tsid = sdt[3] * 256 + sdt[4];

	// make sure the PAT is processed first and the PMTs are created
	if (ad->transponder_id != tsid)
		return 0;

	memset(new_filter, 0, sizeof(new_filter));
	memset(new_mask, 0, sizeof(new_mask));
	new_filter[1] = sdt[3];
	new_mask[1] = 0xFF;
	new_filter[2] = sdt[4];
	new_mask[2] = 0xFF;
	new_filter[3] = (sdt[5] & 0x3E);
	new_mask[3] = 0x3E;
	set_filter_mask(filter, new_filter, new_mask);
	set_filter_flags(filter, FILTER_PERMANENT | FILTER_REVERSE | FILTER_CRC);
	sdt_len = (sdt[1] & 0xF) * 256 + sdt[2];
	i = 11;
	LOG("Processing SDT for transponder %d (%x) with length %d, sdt[5] %02X", tsid, tsid, sdt_len, sdt[5]);

	for (i = 11; i < sdt_len - 1; i += desc_loop_len)
	{
		b = sdt + i;
		sid = b[0] * 256 + b[1];
		desc_loop_len = (b[3] & 0xF) * 256 + b[4];
		desc_loop_len += 5;
		pmt = get_pmt_for_sid(ad->id, sid);
		LOGM("Detected service ID %d (%X), pos %d len %d", sid, sid, i, desc_loop_len);
		if (!pmt)
		{
			LOG("%s: no PMT found for sid %d (%X)", __FUNCTION__, sid, sid);
			continue;
		}
		for (j = 5; j < desc_loop_len; j += desc_len)
		{
			unsigned char *c = b + j;
			desc_len = c[1];
			desc_len += 2;
			if (c[0] == 0x48)
			{
				int name_size = sizeof(pmt->name) - 1;
				c += 3;
				copy_en300468_string(pmt->provider, name_size, (char *)c + 1, c[0]);
				c += c[0] + 1;
				copy_en300468_string(pmt->name, name_size, (char *)c + 1, c[0]);
				LOG("SDT PMT %d: name %s provider %s, sid: %d (%X)", pmt->id, pmt->name, pmt->provider, sid, sid);
			}
		}
	}
	return 0;
}

void start_pmt(SPMT *pmt, adapter *ad)
{
	LOGM("starting PMT %d master %d, pid %d, sid %d for channel: %s", pmt->id, pmt->master_pmt, pmt->pid, pmt->sid, pmt->name);
	pmt->running = 1;
	pmt->encrypted = 0;
	// give 1s to initialize decoding or override for each CA
	pmt->grace_time = getTick() + 2000;
	set_filter_flags(pmt->filter, FILTER_ADD_REMOVE | FILTER_CRC);
#ifndef DISABLE_TABLES
	send_pmt_to_cas(ad, pmt);
#endif
}

void stop_pmt(SPMT *pmt, adapter *ad)
{
	if (!pmt->running)
		return;
	LOGM("stopping PMT %d pid %d sid %d master %d for channel %s", pmt->id, pmt->pid, pmt->sid, pmt->master_pmt, pmt->name);
	pmt->running = 0;
	set_filter_flags(pmt->filter, 0);
#ifndef DISABLE_TABLES
	close_pmt_for_cas(ad, pmt);
#endif
}

void emulate_add_all_pids(adapter *ad)
{
	char pids[8193];
	SPid *p_all = find_pid(ad->id, 8192);
	int i, j, k;
	int updated = 0;
	memset(pids, 0, sizeof(pids));
	for (i = 0; i < MAX_PIDS; i++)
		if (ad->pids[i].flags > 0 && ad->pids[i].flags < 3)
			pids[i] = 1;

	for (i = 0; i < MAX_STREAMS_PER_PID; i++)
		if (p_all->sid[i] >= 0)
		{
			for (j = 0; j < MAX_PMT; j++)
				if (pmts[j] && pmts[j]->enabled && pmts[j]->adapter == ad->id)
					for (k = 0; k < pmts[j]->stream_pids; k++)
						if (!pids[pmts[j]->stream_pid[k].pid])
						{
							LOG("%s: adding pid %d to emulate all pids", __FUNCTION__, pmts[j]->stream_pid[k].pid)
							mark_pid_add(p_all->sid[i], ad->id, pmts[j]->stream_pid[k].pid);
							pids[pmts[j]->stream_pid[k].pid] = 1;
							updated = 1;
						}

			int forced_pids[] = {EMU_PIDS_ALL_ENFORCED_PIDS_LIST};
			int i_forced = sizeof(forced_pids) / sizeof(int);
			for (j = 0; j < i_forced; j++)
			{
				int fpid = forced_pids[j];
				LOG("%s: adding (enforced) pid %d to emulate all pids", __FUNCTION__, fpid);
				mark_pid_add(p_all->sid[i], ad->id, fpid);
				updated = 1;
			}

		}
	if (updated)
		update_pids(ad->id);
}

void pmt_pid_add(adapter *ad, int pid, int existing)
{
	int i;
	SPid *cp;
	SPMT *pmt;
	if (!ad)
		return;
	cp = find_pid(ad->id, pid);
	if (!cp)
		return;

	if (opts.emulate_pids_all && pid == 8192)
	{
		emulate_add_all_pids(ad);
		return;
	}

	cp->filter = get_pid_filter(ad->id, pid);

	int pmt_id = get_master_pmt_for_pid(ad->id, pid);
	if (pmt_id >= 0)
		cp->pmt = pmt_id;

	for (i = 0; i < npmts; i++)
		if (pmts[i] && pmts[i]->enabled && pmts[i]->adapter == ad->id && pmts[i]->master_pmt == pmt_id)
		{
			pmt = pmts[i];
			if (!pmt->running)
				start_pmt(pmt, ad);
#ifndef DISABLE_TABLES
			tables_add_pid(ad, pmt, pid);
#endif
		}
}

void pmt_pid_del(adapter *ad, int pid)
{
	int ep;
	SPid *p;
	if (!ad) // || ad->do_tune)
		return;

	// filter code

	int i;
	p = find_pid(ad->id, pid);
	if (!p)
		return;
	SPMT *pmt = get_pmt(p->pmt);
	if (pmt)
		LOGM("%s: pid %d adapter %d pmt %d, master %d, channel %s", __FUNCTION__, pid, ad->id, p->pmt, pmt->master_pmt, pmt->name)
	else
		return;

	for (i = 0; i < npmts; i++)
		if (pmts[i] && pmts[i]->enabled && pmts[i]->adapter == ad->id && pmts[i]->master_pmt == pmt->master_pmt && pmts[i]->running)
#ifndef DISABLE_TABLES
			tables_del_pid(ad, pmts[i], pid);
#endif

	ep = 0;
	for (i = 0; i < pmt->active_pids; i++)
		if (pmt->active_pid[i] != pid && (p = find_pid(ad->id, pmt->active_pid[i])) && (p->flags == 1 || p->flags == 2))
		{
			LOGM("found active pid %d for pmt id %d, pid %d", pmt->active_pid[i], pmt->id, pmt->pid);
			ep++;
		}

	if (!ep)
	{
		for (i = 0; i < npmts; i++)
			if (pmts[i] && pmts[i]->enabled && pmts[i]->running && pmts[i]->adapter == ad->id && pmts[i]->master_pmt == pmt->master_pmt)
				stop_pmt(pmts[i], ad);
	}
}

int pmt_init_device(adapter *ad)
{
#ifndef DISABLE_TABLES
	tables_init_device(ad);
#endif
	return 0;
}

int pmt_close_device(adapter *ad)
{
#ifndef DISABLE_TABLES
	tables_close_device(ad);
#endif
	return 0;
}
int pmt_tune(adapter *ad)
{
	if (ad->pat_filter == -1)
		ad->pat_filter = add_filter(ad->id, 0, (void *)process_pat, ad, FILTER_PERMANENT | FILTER_CRC);

	if (ad->sdt_filter == -1)
		ad->sdt_filter = add_filter(ad->id, 17, (void *)process_sdt, ad, FILTER_PERMANENT | FILTER_CRC);

	// to comment
	clear_pmt_for_adapter(ad->id);
	return 0;
}

char *get_channel_for_adapter(int aid, char *dest, int max_size)
{
	int i, len;
	adapter *ad;
	dest[0] = 0;
	len = 0;
	ad = get_adapter_nw(aid);
	if (!ad)
		return dest;

	for (i = 0; i < npmts; i++)
		if (pmts[i] && pmts[i]->enabled && pmts[i]->adapter == aid && pmts[i]->running && pmts[i]->name[0])
		{
			len += snprintf(dest + len, max_size - len - 1, "%s,", pmts[i]->name);
		}
	if (len > 0)
		dest[len - 1] = 0;
	return dest;
}

void free_all_pmts(void)
{
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

int pmt_init()
{
	mutex_init(&pmts_mutex);
	mutex_init(&cws_mutex);
	init_algo();
#ifndef DISABLE_TABLES
	tables_init();
#endif
	return 0;
}

int pmt_destroy()
{
#ifndef DISABLE_TABLES
	tables_destroy();
#endif
	mutex_destroy(&cws_mutex);
	mutex_destroy(&pmts_mutex);
	return 0;
}

_symbols pmt_sym[] =
	{
		{"pmt_enabled", VAR_AARRAY_INT8, pmts, 1, MAX_PMT, offsetof(SPMT, enabled)},
		{"pmt_adapter", VAR_AARRAY_INT, pmts, 1, MAX_PMT, offsetof(SPMT, adapter)},
		{"pmt_name", VAR_AARRAY_STRING, pmts, 1, MAX_PMT, offsetof(SPMT, name)},
		{"pmt_pid", VAR_AARRAY_INT, pmts, 1, MAX_PMT, offsetof(SPMT, pid)},
		{"pmt_sid", VAR_AARRAY_INT, pmts, 1, MAX_PMT, offsetof(SPMT, sid)},
		{"pmt_running", VAR_AARRAY_UINT8, pmts, 1, MAX_PMT, offsetof(SPMT, running)},
		{"ad_channel", VAR_FUNCTION_STRING, (void *)&get_channel_for_adapter, 0, MAX_ADAPTERS, 0},

		{NULL, 0, NULL, 0, 0, 0}};
