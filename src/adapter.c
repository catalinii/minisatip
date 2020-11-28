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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
//#include <linux/ioctl.h>
#include <sys/ioctl.h>

#include "socketworks.h"
#include "dvb.h"
#include "adapter.h"
#include "dvbapi.h"
#include "utils.h"
#include "pmt.h"
#include "stream.h"

#ifndef DISABLE_SATIPCLIENT
#include "satipc.h"
#endif

#ifndef DISABLE_NETCVCLIENT
#include "netceiver.h"
#endif

#ifdef AXE
#include "axe.h"
#endif

#ifndef DISABLE_DDCI
#include "ddci.h"
#endif

#include "t2mi.h"

#define DEFAULT_LOG LOG_ADAPTER

adapter *a[MAX_ADAPTERS];
int a_count;
char disabled[MAX_ADAPTERS]; // disabled adapters
int sock_signal;
char do_dump_pids = 1;
uint64_t source_map[MAX_SOURCES];

SMutex a_mutex;

int tuner_s2, tuner_t, tuner_c, tuner_t2, tuner_c2, tuner_at, tuner_ac;
char fe_map[2 * MAX_ADAPTERS];
void find_dvb_adapter(adapter **a);

adapter *adapter_alloc()
{
	adapter *ad = malloc1(sizeof(adapter));
	memset(ad, 0, sizeof(adapter));

	/* diseqc setup */
	ad->diseqc_param.fast = opts.diseqc_fast;
	ad->diseqc_param.addr = opts.diseqc_addr;
	ad->diseqc_param.committed_no = opts.diseqc_committed_no;
	ad->diseqc_param.uncommitted_no = opts.diseqc_uncommitted_no;

	/* diseqc default timing */
	ad->diseqc_param.before_cmd = opts.diseqc_before_cmd;
	ad->diseqc_param.after_cmd = opts.diseqc_after_cmd;
	ad->diseqc_param.after_repeated_cmd = opts.diseqc_after_repeated_cmd;
	ad->diseqc_param.after_switch = opts.diseqc_after_switch;
	ad->diseqc_param.after_burst = opts.diseqc_after_burst;
	ad->diseqc_param.after_tone = opts.diseqc_after_tone;

	/* diseqc state control */
	ad->old_diseqc = -1;
	ad->old_hiband = -1;
	ad->old_pol = -1;
	ad->dmx_source = -1;
	ad->master_source = -1;
	ad->diseqc_multi = opts.diseqc_multi;

	/* enable all sources */
	ad->debug_pos[0] = '\0';
	ad->debug_src = 0;
	int i;
	for (i = 0; i <= MAX_SOURCES; i++)
	{
		ad->sources_pos[i] = 1;
	}

	/* LOF setup */
	ad->diseqc_param.lnb_low = opts.lnb_low;
	ad->diseqc_param.lnb_high = opts.lnb_high;
	ad->diseqc_param.lnb_circular = opts.lnb_circular;
	ad->diseqc_param.lnb_switch = opts.lnb_switch;

	ad->adapter_timeout = opts.adapter_timeout;

	ad->strength_multiplier = opts.strength_multiplier;
	ad->snr_multiplier = opts.snr_multiplier;

	ad->drop_encrypted = opts.drop_encrypted;

#ifndef DISABLE_PMT
	// filter for pid 0
	ad->pat_filter = -1;
	ad->sdt_filter = -1;
#endif
	return ad;
}

void find_adapters()
{
	static int init_find_adapter;
	if (init_find_adapter == 1)
		return;
	init_find_adapter = 1;
	a_count = 0;
#ifndef DISABLE_LINUXDVB
	find_dvb_adapter(a);
#endif
#ifndef DISABLE_SATIPCLIENT
	find_satip_adapter(a);
#endif
#ifndef DISABLE_NETCVCLIENT
	find_netcv_adapter(a);
#endif
#ifdef AXE
	find_axe_adapter(a);
#endif
#ifndef DISABLE_DDCI
	find_ddci_adapter(a);
#endif
}

// avoid adapter close unless all the adapters can be closed
int adapter_timeout(sockets *s)
{
	int do_close = 1;
	adapter *ad = get_adapter(s->sid);
	if (!ad)
		return 1;

	if (ad->force_close)
		return 1;

	if (ad->adapter_timeout == 0)
	{
		LOG("keeping the adapter %d open as the initialization is slow", ad->id);
		s->rtime = getTick();
		return 0;
	}

	if (get_streams_for_adapter(ad->id) > 0)
	{
		ad->rtime = getTick();
		s->rtime = ad->rtime;
		LOG("Keeping the adapter %d open as there are active streams", ad->id);
		return 0;
	}

#ifndef AXE
	int64_t rtime = getTick(), max_close = rtime - ad->adapter_timeout + 2000;
	int i;
	if (opts.no_threads)
	{
		adapter *ad1;
		for (i = 0; i < MAX_ADAPTERS; i++)
			if ((ad1 = get_adapter_nw(i)))
			{
				if (rtime - ad1->rtime < s->timeout_ms)
					do_close = 0;
				if (max_close < ad1->rtime)
				{
					max_close = ad1->rtime;
					LOGM("max_close set to %jd for adapter %d", max_close, i);
				}
			}
	}
	for (i = 0; i < MAX_ADAPTERS; i++)
		if (a[i] && (a[i]->master_source == ad->id) && ad->enabled && a[i]->enabled)
		{
			LOG("adapter %d is already used by a slave adapters %d, used %jx", ad->id, a[i]->id, a[i]->used);
			do_close = 0;
			break;
		}

	LOG("Requested adapter %d close due to timeout %d sec, result %d max_rtime %jd",
		s->sid, ad->adapter_timeout / 1000, do_close, max_close);
	if (!do_close)
		s->rtime = max_close;
#endif
	return do_close;
}

void request_adapter_close(adapter *ad)
{
	ad->force_close = 1;
	sockets_force_close(ad->sock);
}

int close_adapter_for_socket(sockets *s)
{
	int aid = s->sid;
	adapter *ad = get_adapter(aid);
	LOG("closing DVR socket %d pos %d aid %d", s->sock, s->id, aid);
	if (ad && (s->sid == ad->sock))
	{
		ad->sock = -1;
		s->sid = -1;
	}
	if (!ad)
		return 1;

	ad->rtime = getTick();
	return close_adapter(aid);
}

void adapter_set_dvr(adapter *ad)
{
	ad->sock = sockets_add(ad->dvr, NULL, ad->id, TYPE_DVR, (socket_action)read_dmx,
						   (socket_action)close_adapter_for_socket,
						   (socket_action)adapter_timeout);
	memset(ad->buf, 0, opts.adapter_buffer + 1);
	set_socket_buffer(ad->sock, (unsigned char *)ad->buf, opts.adapter_buffer);
	if (ad->adapter_timeout > 0)
		sockets_timeout(ad->sock, ad->adapter_timeout);
	if (opts.th_priority > 0)
		set_thread_prio(get_socket_thread(ad->sock), opts.th_priority);
}

int init_complete = 0;
int num_adapters = 0;

int init_hw(int i)
{
	int64_t st, et;
	adapter *ad;
	int rv = 0;
	if (i < 0 || i >= MAX_ADAPTERS)
		return 3;

	if (a[i] && a[i]->enabled)
		return 0;

	if (!a[i])
		return 2;

	ad = a[i];
	mutex_init(&ad->mutex);
	mutex_lock(&ad->mutex);
	if (is_adapter_disabled(i))
	{
		rv = 3;
		goto NOK;
	}
	if (ad->enabled)
	{
		rv = 4;
		goto NOK;
	}

	ad->id = i;
	ad->fe_sock = -1;
	ad->sock = -1;
	ad->force_close = 0;
	ad->err = 0;

	if (opts.max_pids)
		ad->max_pids = opts.max_pids;

	st = getTick();
	if (!ad->open)
	{
		rv = 5;
		goto NOK;
	}

#ifdef ENIGMA
	if (ad->dmx_source == -1)
		ad->dmx_source = ad->id;
#endif

	if ((rv = ad->open(ad)))
	{
		init_complete = 0;
		LOG("Opening adapter %d failed with error %d", ad->id, rv);
		rv |= 0x600;
		goto NOK;
	}
	ad->enabled = 1;

	if (!ad->buf)
	{
		ad->lbuf = opts.adapter_buffer;
		ad->buf = malloc1(ad->lbuf + 10);
	}
	if (!ad->buf)
	{
		LOG(
			"memory allocation failed for %d bytes failed, adapter %d, trying %d bytes",
			opts.adapter_buffer, i, ADAPTER_BUFFER);
		opts.adapter_buffer = ADAPTER_BUFFER;
		ad->buf = malloc1(opts.adapter_buffer + 10);
		if (!ad->buf)
		{
			LOG("memory allocation failed for %d bytes failed, adapter %d",
				opts.adapter_buffer, i);
			close_adapter(i);
		}
		rv = 7;
		goto NOK;
	}
	memset(ad->buf, 0, opts.adapter_buffer + 1);
	init_dvb_parameters(&ad->tp);
	mark_pids_deleted(i, -1, NULL);
	update_pids(i);

	if (!ad->sys[0])
		ad->delsys(i, ad->fe, ad->sys);
	ad->master_sid = -1;
	ad->sid_cnt = 0;
	ad->pid_err = ad->dec_err = 0;
	ad->new_gs = 0;
	ad->force_close = 0;
	ad->ca_mask = 0;
	ad->tune_time = 0;
	ad->threshold = opts.udp_threshold;
	ad->updating_pids = 0;
	ad->wait_new_stream = 0;
	ad->rtime = getTick();
	adapter_set_dvr(ad);
	snprintf(ad->name, sizeof(ad->name), "AD%d", i);
	ad->thread = start_new_thread(ad->name);
	set_socket_thread(ad->sock, ad->thread);
#ifndef DISABLE_PMT
	pmt_init_device(ad);
#endif
	if (ad->post_init)
		ad->post_init(ad);

	et = getTick();

	if (et - st > 1000000)
	{
		LOG("Slow adapter %d detected", ad->id);
		ad->adapter_timeout = 0;
	}

	//	set_sock_lock(ad->sock, &ad->mutex); // locks automatically the adapter on reading from the DVR

	LOG("done opening adapter %i delivery systems: %s %s %s %s", i,
		get_delsys(ad->sys[0]), get_delsys(ad->sys[1]),
		get_delsys(ad->sys[2]), get_delsys(ad->sys[3]));
	getAdaptersCount();

	mutex_unlock(&ad->mutex);

	if ((ad->master_source >= 0) && (ad->master_source < MAX_ADAPTERS))
	{
		return init_hw(ad->master_source);
	}

	return 0;

NOK:
	LOG("opening adapter %i failed with exit code %d", ad->id, rv);
	mutex_unlock(&ad->mutex);
	return 1;
}

int init_all_hw()
{
	int i, rv;

	LOG("starting init_all_hw %d", init_complete);
	if (init_complete)
		return num_adapters;
	mutex_init(&a_mutex);
	mutex_lock(&a_mutex);
	find_adapters();
	num_adapters = 0;
	init_complete = 1;
	for (i = 0; i < MAX_ADAPTERS; i++)
		if (!a[i] || ((!a[i]->enabled || a[i]->fe <= 0) && ((a[i]->pa >= 0 && a[i]->fn >= 0) || a[i]->type > ADAPTER_DVB))) // ADAPTER is intialized and not DVB
		{
			if (!(rv = init_hw(i)))
				num_adapters++;
			else if (rv != 2)
				LOG("Failed to init device %d with return value %d", i, rv);
		}
		else if (a[i]->enabled)
			num_adapters++;
	if (num_adapters == 0)
		init_complete = 0;
	LOG("done init_hw %d", init_complete);
	mutex_unlock(&a_mutex);
	return num_adapters;
}

// this method needs to close the adapter otherwise the adapter will be in a half closed state

int close_adapter(int na)
{
	adapter *ad;
	init_complete = 0;
	int sock, i;

	ad = get_adapter_nw(na);
	if (!ad)
		return 1;

	mutex_lock(&ad->mutex);
	if (!ad->enabled)
	{
		mutex_unlock(&ad->mutex);
		return 1;
	}

	LOG("closing adapter %d  -> fe:%d dvr:%d, sock:%d, fe_sock:%d", na, ad->fe, ad->dvr, ad->sock, ad->fe_sock);
	//close all streams attached to this adapter
	//	close_streams_for_adapter (na, -1);
	mark_pids_deleted(na, -1, NULL);
	update_pids(na);
	ad->enabled = 0;
	if (ad->close)
		ad->close(ad);
	//      if(ad->dmx>0)close(ad->dmx);
	if (ad->fe > 0)
		close(ad->fe);
#ifndef DISABLE_PMT
	if (ad->ca_mask > 0)
		pmt_close_device(ad);
#endif
	ad->ca_mask = 0;
	ad->fe = -1;
	ad->dvr = 0;
	ad->strength = 0;
	ad->snr = 0;
#ifndef AXE
	ad->old_diseqc = -1;
	ad->old_hiband = -1;
	ad->old_pol = -1;
#endif
	sock = ad->sock;
	ad->sock = -1;
	if (sock >= 0) // avoid locking the socket after the adapter
	{
		sockets_force_close(sock);
		set_sockets_sid(sock, -1);
	}
	mutex_destroy(&ad->mutex);
	//      if(a[na]->buf)free1(a[na]->buf);a[na]->buf=NULL;
	LOG("done closing adapter %d", na);
	for (i = 0; i < MAX_ADAPTERS; i++)
		if (a[i] && (a[i]->master_source == ad->id) && ad->enabled && a[i]->enabled)
		{
			LOG("Slave adapter %d of adapter %d will be closed", i, ad->id);
			request_adapter_close(a[i]);
		}
	return 1;
}

int getAdaptersCount()
{
	int i, j, k;
	adapter *ad;
	int ts2 = 0, tc2 = 0, tt2 = 0, tc = 0, tt = 0, tac = 0, tat = 0;
	int fes[MAX_DVBAPI_SYSTEMS][MAX_ADAPTERS];
	int ifes[MAX_DVBAPI_SYSTEMS];
	char order[] =
		{SYS_DVBS2, SYS_DVBT, SYS_DVBC_ANNEX_A, SYS_DVBT2, SYS_DVBC2, SYS_ATSC, SYS_DVBC_ANNEX_B};

	memset(&ifes, 0, sizeof(ifes));

	if (opts.force_sadapter)
		tuner_s2 = opts.force_sadapter;
	if (opts.force_tadapter)
		tuner_t = opts.force_tadapter;
	if (opts.force_cadapter)
		tuner_c = opts.force_cadapter;
	for (i = 0; i < MAX_ADAPTERS; i++)
		if ((ad = a[i]))
		{
			// Note: Adapter 0 uses map bit 0, sources_pos ranges from 0..MAX_SOURCES+1 and source_map ranges from 0..SRC-1 (SRC=0 can't be configured)
			for (j = 0; j < MAX_SOURCES; j++)
			{
				ad->sources_pos[j + 1] = (source_map[j] >> i) & 1;
			}

			if (!opts.force_sadapter && (delsys_match(ad, SYS_DVBS) || delsys_match(ad, SYS_DVBS2)))
			{
				ts2++;
				fes[SYS_DVBS2][ifes[SYS_DVBS2]++] = i;
			}

			if (!opts.force_tadapter && delsys_match(ad, SYS_DVBT) && !delsys_match(ad, SYS_DVBT2))
			{
				tt++;
				fes[SYS_DVBT][ifes[SYS_DVBT]++] = i;
			}

			if (!opts.force_cadapter && delsys_match(ad, SYS_DVBC_ANNEX_A) && !delsys_match(ad, SYS_DVBC2))
			{
				tc++;
				fes[SYS_DVBC_ANNEX_A][ifes[SYS_DVBC_ANNEX_A]++] = i;
			}

			if (delsys_match(ad, SYS_DVBT2))
			{
				tt2++;
				fes[SYS_DVBT2][ifes[SYS_DVBT2]++] = i;
			}

			if (delsys_match(ad, SYS_DVBC2))
			{
				tc2++;
				fes[SYS_DVBC2][ifes[SYS_DVBC2]++] = i;
			}
			if (delsys_match(ad, SYS_DVBC_ANNEX_B))
			{
				tac++;
				fes[SYS_DVBC_ANNEX_B][ifes[SYS_DVBC_ANNEX_B]++] = i;
			}
			if (delsys_match(ad, SYS_ATSC))
			{
				tat++;
				fes[SYS_ATSC][ifes[SYS_ATSC]++] = i;
			}
		}
	if (tuner_s2 != ts2 || tuner_c2 != tc2 || tt2 != tuner_t2 || tc != tuner_c || tt != tuner_t || tac != tuner_ac || tat != tuner_at)
	{
		if (!opts.force_sadapter)
			tuner_s2 = ts2;
		tuner_c2 = tc2;
		tuner_t2 = tt2;
		if (!opts.force_cadapter)
			tuner_c = tc;
		if (!opts.force_tadapter)
			tuner_t = tt;
		tuner_at = tat;
		tuner_ac = tac;

		memset(&fe_map, -1, sizeof(fe_map));

		k = 0;
		for (i = 0; i < ARRAY_SIZE(order); i++)
		{
			int sys = order[i];
			for (j = 0; j < ifes[sys]; j++)
			{
				fe_map[k++] = fes[sys][j];
				LOG("FE %d mapped to Adapter %d, sys %s", k, fes[sys][j],
					get_delsys(sys));
			}
		}
	}
	char lsrc[256];
	for (i = 0; i < MAX_ADAPTERS; i++)
		if ((ad = a[i]))
		{
			ad->debug_pos[0] = '\0';
			ad->debug_src = 0;
			lsrc[0] = '\0';
			for (j = 1; j <= MAX_SOURCES; j++)
			{
				if (ad->sources_pos[j])
				{
					char usrc[8];
					sprintf(usrc, ",%d", j);
					strcat(lsrc, usrc);

					strcat(ad->debug_pos, "X");
					ad->debug_src += (unsigned long)1 << (j - 1);
				}
				else
				{
					strcat(ad->debug_pos, ".");
				}
			}
			if (strlen(lsrc) > 0)
				lsrc[0] = ' ';
			DEBUGM("Adpater %d enabled sources:%s", i, lsrc);
			DEBUGM("Adpater %d debug sources: %s (%lu)", i, ad->debug_pos, ad->debug_src);
		}
	return tuner_s2 + tuner_c2 + tuner_t2 + tuner_c + tuner_t;
}

void dump_adapters()
{
	int i;
	adapter *ad;
	if (!opts.log)
		return;
	LOG("Dumping adapters:");
	for (i = 0; i < MAX_ADAPTERS; i++)
		if ((ad = get_adapter_nw(i)))
			LOG("%d|f: %d sid_cnt:%d master_sid:%d master_source:%d del_sys: %s,%s,%s src_map: %s (%lu)", i,
				ad->tp.freq, ad->sid_cnt, ad->master_sid, ad->master_source,
				get_delsys(ad->sys[0]), get_delsys(ad->sys[1]),
				get_delsys(ad->sys[2]), ad->debug_pos, ad->debug_src);
	dump_streams();
}

void dump_pids(int aid)
{
	int i, dp = 1;

	if (!opts.log)
		return;

	if (!do_dump_pids)
		return;
	adapter *p = get_adapter(aid);
	if (!p)
		return;
	for (i = 0; i < MAX_PIDS; i++)
		if (p->pids[i].flags > 0)
		{
			if (dp)
				LOG("Dumping pids table for adapter %d, number of unknown pids: %d", aid, p->pid_err);
			dp = 0;
			LOG("pid %d, fd %d, packets %d, d/c errs %d/%d, flags %d, pmt %d, filter %d, sock %d, sids: %d %d %d %d %d %d %d %d",
				p->pids[i].pid, p->pids[i].fd,
				p->pids[i].packets, p->pids[i].dec_err, p->pids[i].cc_err,
				p->pids[i].flags, p->pids[i].pmt, p->pids[i].filter, p->pids[i].sock, p->pids[i].sid[0],
				p->pids[i].sid[1], p->pids[i].sid[2], p->pids[i].sid[3],
				p->pids[i].sid[4], p->pids[i].sid[5], p->pids[i].sid[6],
				p->pids[i].sid[7]);
		}
}

// makes sure the slave and the master adapter have the same polarity, band and diseqc
int compare_slave_parameters(adapter *ad, transponder *tp)
{
	adapter *master = NULL;

	if (!ad)
		return 0;
	// is not a slave and does not have a slave adapter
	if ((ad->master_source < 0) && !ad->used)
		return 0;
	// is slave and the switch is UNICABLE/JESS - we do not care about pol and band
	if ((ad->diseqc_param.switch_type == SWITCH_JESS) || (ad->diseqc_param.switch_type == SWITCH_UNICABLE))
		return 0;

	// master adapter is used by other
	int diseqc = (tp->diseqc > 0) ? tp->diseqc - 1 : 0;
	int pol = (tp->pol - 1) & 1;
	int hiband = 0;
	int freq = tp->freq;

	if (tp->pol > 2 && tp->diseqc_param.lnb_circular > 0)
		hiband = 0;
	else if (freq < tp->diseqc_param.lnb_switch)
		hiband = 0;
	else
		hiband = 1;

	if (ad->master_source >= 0 && ad->master_source < MAX_ADAPTERS)
		master = a[ad->master_source];

	// master adapter used by slave adapters, check slave parameters if they match
	if (ad && ad->used)
	{
		int i;
		for (i = 0; i < MAX_ADAPTERS; i++)
			if (ad->used & (1L << i))
			{
				adapter *ad2 = get_adapter(i);
				if (!ad2)
				{
					LOG("adapter %d used is set for adapter %d but it is disabled, clearing", ad->id, i);
					ad->used &= ~(1 << i);
					continue;
				}
				if (ad2->old_pol != pol || ad2->old_hiband != hiband || ad2->old_diseqc != diseqc)
					return 1; // slave parameters matches with the required parameters
			}
	}

	// master adapter used by slave adapters
	if (master)
	{
		if (master->old_pol != pol || master->old_hiband != hiband || master->old_diseqc != diseqc)
			return 1; // master parameters matches with the required parameters
	}
	LOGM("%s: adapter %d used %ld master %d used %ld (pol %d, band %d, diseqc %d) not compatible with freq %d, pol %d band %d diseqc %d", __FUNCTION__, ad->id, ad->used, master ? master->id : ad->master_source, master ? master->used : -1, ad->old_pol, ad->old_hiband, ad->old_diseqc, freq, pol, hiband, diseqc);
	return 0;
}

int get_free_adapter(transponder *tp)
{
	int i;
	int match = 0;
	int msys = tp->sys;
	int fe = tp->fe;

	adapter *ad = a[0];

	if ((fe > 0) && (fe <= ARRAY_SIZE(fe_map)) && (fe_map[fe - 1] >= 0))
	{
		fe = fe_map[fe - 1];
		ad = a[fe];
	}
	else
		fe = -1;

	if (ad)
		LOG(
			"get free adapter %d - a[%d] => e:%d m:%d sid_cnt:%d src:%d f:%d pol=%d sys: %s %s",
			tp->fe, ad->id, ad->enabled, ad->master_sid, ad->sid_cnt,
			ad->tp.diseqc, ad->tp.freq, ad->tp.pol, get_delsys(ad->sys[0]),
			get_delsys(ad->sys[1]))
	else
		LOG("get free adapter %d msys %s requested %s", fe, get_delsys(fe),
			get_delsys(msys));

	if (fe >= 0)
	{
		if (ad && delsys_match(ad, msys))
		{
			match = 0;
			if (ad->sid_cnt == 0)
				match = 1;
			if (!ad->enabled || !compare_tunning_parameters(ad->id, tp))
				match = 1;
			if (match && ad->sources_pos[tp->diseqc] && !init_hw(fe))
				return fe;
		}
		goto noadapter;
	}
	// provide an already existing adapter
	for (i = 0; i < MAX_ADAPTERS; i++)
		if ((ad = get_adapter_nw(i)) && delsys_match(ad, msys) && ad->sources_pos[tp->diseqc])
			if (!compare_tunning_parameters(ad->id, tp))
				return i;

	for (i = 0; i < MAX_ADAPTERS; i++)
	{
		//first free adapter that has the same msys
		if ((ad = get_adapter_nw(i)) && ad->sid_cnt == 0 && delsys_match(ad, msys) && !compare_slave_parameters(ad, tp) && ad->sources_pos[tp->diseqc])
			return i;
		if (!ad && delsys_match(a[i], msys) && !compare_slave_parameters(a[i], tp)) // device is not initialized
		{
			ad = a[i];
			if (ad->sources_pos[tp->diseqc] && !init_hw(i))
				return i;
		}
	}

noadapter:
	LOG("no adapter found for src:%d f:%d pol:%d msys:%d", tp->diseqc, tp->freq, tp->pol, tp->sys);
	dump_adapters();
	return -1;
}

void adapter_update_threshold(adapter *ad)
{
	streams *sid = NULL;
	int i, threshold = opts.udp_threshold;
	if (ad->sid_cnt == 1 && ad->master_sid >= 0)
	{
		sid = get_stream(ad->master_sid);
		if (sid && (sid->type == STREAM_RTSP_TCP || sid->type == STREAM_HTTP))
			threshold = opts.tcp_threshold;
	}
	else if (ad->sid_cnt > 0)
	{
		for (i = 0; i < MAX_STREAMS; i++)
			if ((sid = get_stream(i)))
				if (sid->type == STREAM_RTSP_UDP)
					break;
		if (i >= MAX_STREAMS)
			threshold = opts.tcp_threshold;
	}

	if (threshold <= 0 || threshold >= 200)
		threshold = 200;

	ad->threshold = threshold;
}

int set_adapter_for_stream(int sid, int aid)
{
	adapter *ad;
	if (!(ad = get_adapter(aid)))
		return -1;
	mutex_lock(&ad->mutex);

	if (ad->master_sid == -1)
		ad->master_sid = sid;
	if (ad->sid_cnt++ == 0) // tune always first time for a stream
		ad->tp.freq = 0;

	if (ad->master_source >= 0 && ad->master_source < MAX_ADAPTERS)
	{
		adapter *ad2 = a[ad->master_source];
		ad2->used |= (1 << ad->id);
	}
	LOG("set adapter %d for sid %d m:%d s:%d", aid, sid, ad->master_sid, ad->sid_cnt);
	adapter_update_threshold(ad);
	mutex_unlock(&ad->mutex);

	return 0;
}

void close_adapter_for_stream(int sid, int aid, int close_stream)
{
	adapter *ad;
	streams *s = get_sid(sid);
	if (!(ad = get_adapter(aid)))
		return;

	mutex_lock(&ad->mutex);

	if (s && s->adapter == aid)
		s->adapter = -1;
	else if (s && s->adapter != aid)
		LOG("%s adapter mismatch: got %d expected %d", __FUNCTION__, s->adapter, aid);

	if (ad->master_sid == sid)
	{
		ad->master_sid = -1;
		fix_master_sid(aid);
	}
	if (ad->sid_cnt > 0)
		ad->sid_cnt--;
	LOG("closed adapter %d for sid %d m:%d sid_cnt:%d", aid, sid, ad->master_sid, ad->sid_cnt);
	// delete the attached PIDs as well
	if (ad->sid_cnt == 0)
	{
		ad->master_sid = -1;
		mark_pids_deleted(aid, -1, NULL);
		init_dvb_parameters(&ad->tp);
		if (ad->standby && close_stream)
			ad->standby(ad);
#ifdef AXE
		free_axe_input(ad);
#endif
		if ((ad->master_source >= 0) && (ad->master_source < MAX_ADAPTERS))
		{
			adapter *ad2 = a[ad->master_source];
			if (ad2)
				ad2->used &= ~(1 << ad->id);
			LOGM("adapter %d freed from slave adapter %d, used %jd", ad2 ? ad2->id : -1, ad->id, ad2 ? ad2->used : -1);
		}
	}
	else
		mark_pids_deleted(aid, sid, NULL);
	update_pids(aid);
	adapter_update_threshold(ad);
	mutex_unlock(&ad->mutex);
}

int update_pids(int aid)
{
	int i, dp = 1;
	adapter *ad;
	ad = get_adapter(aid);
	if (!ad || ad->updating_pids)
		return 1;

	if (ad->err)
	{
		LOG("adapter %d in error state %d", aid, ad->err);
		return 1;
	}
	ad->updating_pids = 1;
#ifndef DISABLE_PMT
	for (i = 0; i < MAX_PIDS; i++)
		if (ad->pids[i].flags == 3)
			pmt_pid_del(ad, ad->pids[i].pid);

	for (i = 0; i < MAX_PIDS; i++)
		if (ad->pids[i].flags == 2)
			pmt_pid_add(ad, ad->pids[i].pid, 0);
#endif

	for (i = MAX_PIDS - 1; i >= 0; i--)
		if (ad->pids[i].flags == 3)
		{
			if (dp)
				dump_pids(aid);
			dp = 0;
			if (ad->pids[i].fd > 0)
			{
				if (ad->active_pids > 0)
					ad->active_pids--;
				ad->del_filters(ad, ad->pids[i].fd, ad->pids[i].pid);
			}
			ad->pids[i].fd = 0;
			ad->pids[i].filter = -1;
			ad->pids[i].pmt = -1;
			ad->pids[i].flags = 0;
		}

	for (i = 0; i < MAX_PIDS; i++)
		if (ad->pids[i].flags == 2)
		{
			if (ad->max_pids && (ad->max_pids < ad->active_pids))
			{
				LOG("maximum number of pids %d out of %d reached", ad->active_pids, ad->max_pids);
				break;
			}

			if (dp)
				dump_pids(aid);
			dp = 0;
			if (ad->pids[i].fd <= 0)
			{
				int pid = ad->pids[i].pid;
				// For pids=all emulation add just the PAT pid. process_pmt will add
				// the other pids
				if (opts.emulate_pids_all && pid == 8192)
					pid = 0;
				if ((ad->pids[i].fd = ad->set_pid(ad, pid)) < 0)
				{
					ad->max_pids = ad->max_active_pids - 1;
					LOG0("Maximum pid filter reached, lowering the value to %d", opts.max_pids);
					break;
				}
				ad->active_pids++;
				if (ad->max_active_pids < ad->active_pids)
					ad->max_active_pids = ad->active_pids;
			}
			ad->pids[i].flags = 1;
			if (ad->pids[i].pid == 0)
				ad->pat_processed = 0;
			ad->pids[i].packets = 0;
			ad->pids[i].cc = -1;
			ad->pids[i].cc1 = -1;
			ad->pids[i].cc_err = 0;
			ad->pids[i].dec_err = 0;
		}
	if (ad->commit)
		ad->commit(ad);

	ad->updating_pids = 0;
	return 0;
}

void post_tune(adapter *ad)
{
#if !defined(DISABLE_PMT) || !defined(DISABLE_T2MI)
	int aid = ad->id;
#endif
#ifndef DISABLE_PMT
	SPid *p = find_pid(aid, 0);
	SPid *p_all = find_pid(aid, 8192);
	if ((!p || p->flags == 3) && (!p_all || p_all->flags == 3)) // add pid 0 if not explicitly added
	{
		LOG(
			"Adding pid 0 to the list of pids as not explicitly added for adapter %d",
			aid);
		mark_pid_add(-1, aid, 0);
	}
	pmt_tune(ad);
#endif
#ifndef DISABLE_T2MI
	SPid *p_t2mi = find_pid(aid, T2MI_PID);
	if (!p_t2mi)
	{
		mark_pid_add(-1, aid, T2MI_PID);
	}
#endif
}

int tune(int aid, int sid)
{
	adapter *ad = get_adapter(aid);
	int rv = 0, flush_data = 0;

	if (!ad)
		return -400;

	mutex_lock(&ad->mutex);

	ad->last_sort = getTick();
	if (sid == ad->master_sid && ad->do_tune)
	{
		ad->tp.diseqc_param = ad->diseqc_param;

		rv = ad->tune(ad->id, &ad->tp);
		ad->status = -1;
		ad->status_cnt = 0;
		ad->wait_new_stream = 1;
		ad->strength = 0;
		ad->snr = 0;
		flush_data = 1;
		ad->is_t2mi = 0;
		set_socket_pos(ad->sock, 0); // flush the existing buffer
		ad->rlen = 0;
		if (ad->sid_cnt > 1) // the master changed the frequency
		{
			close_streams_for_adapter(aid, sid);
			if (update_pids(aid))
			{
				ad->do_tune = 0;
				mutex_unlock(&ad->mutex);
				return -503;
			}
		}
		post_tune(ad);
	}
	else
		LOG("not tuning for sid %d (do_tune=%d, master_sid=%d)", sid,
			ad->do_tune, ad->master_sid);
	if (rv < 0)
		mark_pids_deleted(aid, sid, NULL);
	if (update_pids(aid))
	{
		ad->do_tune = 0;
		mutex_unlock(&ad->mutex);
		return -503;
	}
	if (flush_data)
	{
		ad->tune_time = getTick();
		get_socket_iteration(ad->sock, 0);
	}
	ad->do_tune = 0;
	mutex_unlock(&ad->mutex);
	return rv;
}

SPid *find_pid(int aid, int p)
{
	int i;
	adapter *ad = get_adapter(aid);

	if (!ad)
		return NULL;

	for (i = 0; i < MAX_PIDS; i++)
		if ((ad->pids[i].flags > 0) && (ad->pids[i].pid == p))
			return &ad->pids[i];
	return NULL;
}

void mark_pid_deleted(int aid, int sid, int _pid, SPid *p)
{
	int j;
	int cnt = 0, sort = 0;
	if (!p)
		p = find_pid(aid, _pid);
	if (!p)
		return;

	if (sid == -1) // delete all sids and the pid
	{
		if (p->flags != 0)
			p->flags = 3;
		for (j = 0; j < MAX_STREAMS_PER_PID; j++)
			if (sid == -1) // delete all pids if sid = -1
				p->sid[j] = -1;
		return;
	}
	// sid != -1
	for (j = 0; j < MAX_STREAMS_PER_PID; j++)
		if (p->sid[j] == sid) // delete all pids where .sid == sid
		{
			p->sid[j] = -1;
			if ((j + 1 < MAX_STREAMS_PER_PID) && (p->sid[j + 1] >= 0))
				sort = 1;
		}

	for (j = 0; j < MAX_STREAMS_PER_PID; j++)
		if (p->sid[j] >= 0)
			cnt++;
	int keep = 0;

#ifndef DISABLE_PMT
	if (cnt == 0 && p->filter != -1 && p->flags > 0)
		keep = get_active_filters_for_pid(p->filter, aid, _pid, FILTER_ADD_REMOVE); // 0 - no filter with type ADD_REMOVE
#endif

	if ((cnt == 0) && (p->flags != 0) && !keep)
		p->flags = 3;

	if (sort)
	{
		for (j = 0; j < MAX_STREAMS_PER_PID - 1; j++)
			if (p->sid[j + 1] > p->sid[j])
			{
				unsigned char t = p->sid[j];
				p->sid[j] = p->sid[j + 1];
				p->sid[j + 1] = t;
			}
	}
}

void mark_pids_deleted(int aid, int sid, char *pids) //pids==NULL -> delete all pids
{
	int i, la, pid;
	adapter *ad;
	char *arg[MAX_PIDS];

	ad = get_adapter(aid);
	if (!ad)
		return;

	LOG("deleting pids on adapter %d, sid %d, pids=%s", aid, sid,
		pids ? pids : "NULL");

	// If "delpids=all" provided, delete all the pids for this stream
	if (pids && strstr(pids, "all"))
	{
		LOG("deleting all the pids on adapter %d, sid %d, pids=%s", aid, sid, pids);
		pids = NULL;
	}

	if (pids)
	{
		la = split(arg, pids, ARRAY_SIZE(arg), ',');
		for (i = 0; i < la; i++)
		{
			pid = map_int(arg[i], NULL);
			mark_pid_deleted(aid, sid, pid, NULL);
		}
		return;
	}

	for (i = 0; i < MAX_PIDS; i++)
		mark_pid_deleted(aid, sid, ad->pids[i].pid, &ad->pids[i]);
	//	if(sid == -1)
	//		reset_pids_type(aid);
	dump_pids(aid);
}

int mark_pid_add(int sid, int aid, int _pid)
{
	adapter *ad;
	int k, i;
	ad = get_adapter(aid);
	int found = 0;
	SPid *p;
	if (!ad)
		return -1;
	// check if the pid already exists, if yes add the sid
	if ((p = find_pid(aid, _pid)))
	{
		LOG("found already existing pid %d flags %d", _pid, p->flags);
		for (k = 0; k < MAX_STREAMS_PER_PID; k++)
			if (p->sid[k] == -1 || p->sid[k] == sid)
			{
				if (p->flags == 3)
					p->flags = 2;
				p->sid[k] = sid;
				found = 1;
				break;
			}
		if (!found)
		{
			LOG("too many streams for PID %d adapter %d", _pid, aid);
			return -1;
		}
		return 0;
	}
	// add the new pid in a new position
	for (i = 0; i < MAX_PIDS; i++)
		if (ad->pids[i].flags <= 0)
		{
			ad->pids[i].flags = 2;
			ad->pids[i].pid = _pid;
			ad->pids[i].sid[0] = sid;
			ad->pids[i].pmt = -1;
			ad->pids[i].filter = -1;
			ad->pids[i].sock = -1;
			return 0;
		}
	LOG("MAX_PIDS (%d) reached for adapter %d in adding PID: %d", MAX_PIDS, aid,
		_pid);
	dump_pids(aid);
	dump_adapters();
	return -1;
}

int mark_pids_add(int sid, int aid, char *pids)
{
	int i, la;
	adapter *ad;
	char *arg[MAX_PIDS + 2];
	int pid;

	ad = get_adapter(aid);
	if (!ad)
		return -1;

	if (!pids)
		return -1;
	LOG("adding pids to adapter %d, sid %d, pids=%s", aid, sid,
		pids ? pids : "NULL");

	la = split(arg, pids, ARRAY_SIZE(arg), ',');
	for (i = 0; i < la; i++)
	{
		pid = map_intd(arg[i], NULL, -1);
		if (pid < 0 || pid > 8192)
			continue;
		if (mark_pid_add(sid, aid, pid) < 0)
			return -1;
	}
	dump_pids(aid);
	return 0;
}

int compare_tunning_parameters(int aid, transponder *tp)
{
	adapter *ad = get_adapter(aid);
	if (!ad)
		return -1;

	LOGM("new parameters: f:%d, plp/isi:%d, diseqc:%d, pol:%d, sr:%d, mtype:%d",
		 tp->freq, tp->plp_isi, tp->diseqc, tp->pol, tp->sr, tp->mtype);
	LOGM("old parameters: f:%d, plp/isi:%d, diseqc:%d, pol:%d, sr:%d, mtype:%d",
		 ad->tp.freq, ad->tp.plp_isi, ad->tp.diseqc, ad->tp.pol, ad->tp.sr, ad->tp.mtype);
	if (tp->freq != ad->tp.freq || tp->plp_isi != ad->tp.plp_isi || tp->diseqc != ad->tp.diseqc || (tp->pol > 0 && tp->pol != ad->tp.pol) || (tp->sr > 1000 && tp->sr != ad->tp.sr) || (tp->mtype != 6 && tp->mtype != ad->tp.mtype))

		return 1;

	return 0;
}

int set_adapter_parameters(int aid, int sid, transponder *tp)
{
	adapter *ad = get_adapter(aid);

	if (!ad)
		return -1;

	LOG("setting DVB parameters for adapter %d - master_sid %d sid %d old f:%d",
		aid, ad->master_sid, sid, ad->tp.freq);
	mutex_lock(&ad->mutex);
	if (ad->master_sid == -1)
		ad->master_sid = sid; // master sid was closed

	ad->do_tune = 0;
	if (compare_tunning_parameters(aid, tp))
	{
		if (sid != ad->master_sid) // slave sid requesting to tune to a different frequency
		{
			mutex_unlock(&ad->mutex);
			LOG(
				"secondary stream requested tune, not gonna happen ad: f:%d sr:%d pol:%d plp/isi:%d src:%d mod %d -> \
			new: f:%d sr:%d pol:%d plp/isi:%d src:%d mod %d",
				ad->tp.freq, ad->tp.sr, ad->tp.pol, ad->tp.plp_isi,
				ad->tp.diseqc, ad->tp.mtype, tp->freq, tp->sr, tp->pol,
				tp->plp_isi, tp->diseqc, tp->mtype);
			return -1;
		}
		ad->do_tune = 1;
		mark_pids_deleted(aid, -1, NULL);
		if (update_pids(aid))
		{
			ad->do_tune = 0;
			mutex_unlock(&ad->mutex);
			return -1;
		}
	}

	copy_dvb_parameters(tp, &ad->tp);

	if (ad->tp.pids) // pids can be specified in SETUP and then followed by a delpids in PLAY, make sure the behaviour is right
	{
		mark_pids_deleted(aid, sid, NULL); // delete all the pids for this
		if (mark_pids_add(sid, aid, ad->tp.pids) < 0)
		{
			mutex_unlock(&ad->mutex);
			return -1;
		}
	}

	if (ad->tp.dpids)
	{
		mark_pids_deleted(aid, sid, ad->tp.dpids);
	}
	if (ad->tp.apids)
	{
		if (mark_pids_add(sid, aid, ad->tp.apids ? ad->tp.apids : ad->tp.pids) < 0)
		{
			mutex_unlock(&ad->mutex);
			return -1;
		}
	}

	if (0 && (ad->tp.apids || ad->tp.pids || ad->tp.dpids))
		dump_pids(aid);
	mutex_unlock(&ad->mutex);
	return 0;
}

adapter *
get_adapter1(int aid, char *file, int line)
{
	if (aid < 0 || aid >= MAX_ADAPTERS || !a[aid] || !a[aid]->enabled)
	{
		LOG("%s:%d: get_adapter returns NULL for adapter_id %d", file, line,
			aid);
		return NULL;
	}
	return a[aid];
}

adapter *
get_configured_adapter1(int aid, char *file, int line)
{
	if (aid < 0 || aid >= MAX_ADAPTERS || !a[aid] || disabled[aid])
	{
		LOG("%s:%d: get_configured_adapter returns NULL for adapter_id %d",
			file, line, aid);
		return NULL;
	}
	return a[aid];
}

char *itoa_positive(char *dest, int val)
{
	dest[0] = 0;
	if (val >= 0)
		sprintf(dest, "%d", val);
	return dest;
}

char *get_stream_pids(int s_id, char *dest, int max_size);
char *
describe_adapter(int sid, int aid, char *dad, int ld)
{
	int use_ad, len;
	transponder *t;
	adapter *ad;
	streams *ss;
	char plp_isi[10], ds[10];
	int status = 1, strength = 255, snr = 15;

	ss = get_sid(sid);

	use_ad = 1;
	if (!(ad = get_adapter_nw(aid)) || (ss && !ss->do_play))
	{
		if (aid < 0)
			aid = 0;
		if (!ss)
			return "";
		t = &ss->tp;
		use_ad = 0;
	}
	else
		t = &ad->tp;
	memset(dad, 0, ld);

	if (use_ad)
	{
		if (ad->status < 0)
		{
			status = strength = snr = 0;
		}
		else
		{
			strength = ad->strength;
			snr = ad->snr;
			if (snr > 15)
				snr = snr >> 4;
			status = (ad->status & FE_HAS_LOCK) > 0;

			if (strength > 255 || strength < 0)
				strength = 1;
			if (snr > 15 || snr < 0)
				snr = 1;
		}
	}
	if (t->sys == 0)
		len = snprintf(dad, ld, "ver=1.0;src=1;tuner=%d,0,0,0,0,,,,,,,;pids=",
					   aid + 1);
	else if (t->sys == SYS_DVBS || t->sys == SYS_DVBS2)
		len =
			snprintf(dad, ld,
					 "ver=1.0;src=%d;tuner=%d,%d,%d,%d,%d,%s,%s,%s,%s,%s,%d,%s;pids=",
					 t->diseqc, (ad && ad->tp.fe > 0) ? ad->tp.fe : aid + 1, strength, status, snr,
					 t->freq / 1000, get_pol(t->pol),
					 get_delsys(t->sys), get_modulation(t->mtype), get_pilot(t->plts),
					 get_rolloff(t->ro), t->sr / 1000, get_fec(t->fec));
	else if (t->sys == SYS_DVBT || t->sys == SYS_DVBT2)
		len =
			snprintf(dad, ld, "ver=1.1;tuner=%d,%d,%d,%d,%.2f,%d,%s,%s,%s,%s,%s,%s,%d,%d;pids=",
					 (ad && ad->tp.fe > 0) ? ad->tp.fe : aid + 1, strength, status, snr,
					 (double)t->freq / 1000.0, t->bw / 1000000, get_delsys(t->sys),
					 get_tmode(t->tmode), get_modulation(t->mtype),
					 get_gi(t->gi), get_fec(t->fec), itoa_positive(plp_isi, t->plp_isi),
					 t->t2id, t->sm);
	else
		len =
			snprintf(dad, ld, "ver=1.2;tuner=%d,%d,%d,%d,%.2f,%d,%s,%s,%d,%d,%s,%s,%s;pids=",
					 (ad && ad->tp.fe > 0) ? ad->tp.fe : aid + 1, strength, status, snr,
					 (double)t->freq / 1000, t->bw / 1000000, get_delsys(t->sys),
					 get_modulation(t->mtype), t->sr / 1000, t->c2tft, itoa_positive(ds, t->ds),
					 itoa_positive(plp_isi, t->plp_isi), get_inversion(t->inversion));

	if (use_ad)
	{
		int len1 = len;
		len += strlen(get_stream_pids(sid, dad + len, ld - len));
		if (len == len1)
			strlcatf(dad, ld, len, "%s", "none");
	}

	if (!use_ad && (t->apids || t->pids))
	{
		if (t->pids && strstr(t->pids, "8192"))
			strlcatf(dad, ld, len, "%s", "all");
		else
			strlcatf(dad, ld, len, "%s", t->pids ? t->pids : t->apids);
	}
	else if (!use_ad)
		strlcatf(dad, ld, len, "%s", "none");

	LOGM("describe_adapter: sid %d, aid %d => %s", sid, aid, dad);

	return dad;
}

// sorting the pid list in order to get faster the pids that are frequestly used
void sort_pids(int aid)
{
	int b, i;
	SPid pp;
	SPid *p;
	adapter *ad = get_adapter(aid);

	if (!ad)
		return;
	p = ad->pids;
	b = 1;
	while (b)
	{
		b = 0;
		for (i = 0; i < MAX_PIDS - 1; i++)
			if (p[i].packets < p[i + 1].packets)
			{
				b = 1;
				memcpy(&pp, &p[i], sizeof(pp));
				memcpy(&p[i], &p[i + 1], sizeof(pp));
				memcpy(&p[i + 1], &pp, sizeof(pp));
			}
	}
}

void free_all_adapters()
{
	int i;
	sockets_del(sock_signal);
	for (i = 0; i < MAX_ADAPTERS; i++)
		if (a[i] && a[i]->enabled)
		{
			a[i]->adapter_timeout = opts.adapter_timeout;
			close_adapter(i);
		}

	for (i = 0; i < MAX_ADAPTERS; i++)
		if (a[i])
		{
			if (a[i]->buf)
				free1(a[i]->buf);
			free(a[i]);
			a[i] = NULL;
		}

#ifndef DISABLE_NETCVCLIENT
	fprintf(stderr, "\n\nREEL: recv_exit\n");
	if (opts.netcv_if && recv_exit())
		LOG("Netceiver exit failed");
#endif
}
char is_adapter_disabled(int i)
{
	if (i >= 0 && i < MAX_ADAPTERS)
		return disabled[i];
	return 1;
}

void set_disable(int i, int v)
{
	if (i >= 0 && i < MAX_ADAPTERS)
		disabled[i] = v;
}

void enable_adapters(char *o)
{
	int i, la, st, end, j;
	char buf[1000], *arg[40], *sep;
	for (i = 0; i < MAX_ADAPTERS; i++)
		set_disable(i, 1);
	SAFE_STRCPY(buf, o);

	la = split(arg, buf, ARRAY_SIZE(arg), ',');
	for (i = 0; i < la; i++)
	{
		sep = strchr(arg[i], '-');
		if (sep == NULL)
		{
			st = map_int(arg[i], NULL);
			set_disable(st, 0);
		}
		else
		{
			st = map_int(arg[i], NULL);
			end = map_int(sep + 1, NULL);
			for (j = st; j <= end; j++)
				set_disable(j, 0);
		}
	}
}

void set_unicable_adapters(char *o, int type)
{
	int i, la, a_id, slot, freq, pin, o13v;
	char buf[1000], *arg[40], *sep1, *sep2, *sep3;
	adapter *ad;
	SAFE_STRCPY(buf, o);
	la = split(arg, buf, ARRAY_SIZE(arg), ',');
	for (i = 0; i < la; i++)
	{
		a_id = map_intd(arg[i], NULL, -1);
		if (a_id < 0 || a_id >= MAX_ADAPTERS)
			continue;

		if (!a[a_id])
			a[a_id] = adapter_alloc();
		ad = a[a_id];

		sep1 = strchr(arg[i], ':');
		sep2 = strchr(arg[i], '-');

		if (!sep1 || !sep2)
			continue;
		if ((o13v = (sep2[1] == '*')) != 0)
			sep2++;
		slot = map_intd(sep1 + 1, NULL, -1);
		freq = map_intd(sep2 + 1, NULL, -1);
		if (slot < 0 || freq < 0)
			continue;
		sep3 = strchr(sep2 + 1, '-');
		pin = TP_VALUE_UNSET;
		if (sep3)
			pin = map_intd(sep3 + 1, NULL, TP_VALUE_UNSET);

		ad->diseqc_param.uslot = slot;
		ad->diseqc_param.ufreq = freq;
		ad->diseqc_param.switch_type = type;
		ad->diseqc_param.pin = pin;
		ad->diseqc_param.only13v = o13v;
		LOG("Setting %s adapter %d slot %d freq %d pin %d",
			type == SWITCH_UNICABLE ? "unicable" : "jess", a_id, slot, freq, pin);
	}
}

void set_diseqc_adapters(char *o)
{
	int i, la, a_id, fast, addr, committed_no, uncommitted_no;
	char buf[1000], *arg[40], *sep1, *sep2;
	adapter *ad;
	SAFE_STRCPY(buf, o);
	la = split(arg, buf, ARRAY_SIZE(arg), ',');
	for (i = 0; i < la; i++)
	{
		if (arg[i] && arg[i][0] == '*')
		{
			ad = NULL;
			a_id = -1;
		}
		else
		{
			a_id = map_intd(arg[i], NULL, -1);
			if (a_id < 0 || a_id >= MAX_ADAPTERS)
				continue;

			if (!a[a_id])
				a[a_id] = adapter_alloc();
			ad = a[a_id];
		}

		sep1 = strchr(arg[i], ':');
		sep2 = strchr(arg[i], '-');

		if (!sep1 || !sep2)
			continue;

		fast = 0;
		addr = 0x10;
		while (sep1[1] == '*' || sep1[1] == '@' || sep1[1] == '.')
		{
			if (sep1[1] == '*')
			{
				fast = 1;
				sep1++;
			}
			else if (sep1[1] == '@')
			{
				addr = 0;
				sep1++;
			}
			else if (sep1[1] == '.')
			{
				addr = 0x11;
				sep1++;
			}
		}

		committed_no = map_intd(sep1 + 1, NULL, -1);
		uncommitted_no = map_intd(sep2 + 1, NULL, -1);
		if (committed_no < 0 || uncommitted_no < 0)
			continue;

		if (ad)
		{
			ad->diseqc_param.fast = fast;
			ad->diseqc_param.addr = addr;
			ad->diseqc_param.committed_no = committed_no;
			ad->diseqc_param.uncommitted_no = uncommitted_no;
		}
		else
		{
			opts.diseqc_fast = fast;
			opts.diseqc_addr = addr;
			opts.diseqc_committed_no = committed_no;
			opts.diseqc_uncommitted_no = uncommitted_no;
			int j;
			for (j = 0; j < MAX_ADAPTERS; j++)
				if (a[j])
				{
					a[j]->diseqc_param.fast = fast;
					a[j]->diseqc_param.addr = addr;
					a[j]->diseqc_param.committed_no = committed_no;
					a[j]->diseqc_param.uncommitted_no = uncommitted_no;
				}
		}
		LOG(
			"Setting diseqc adapter %d fast %d addr 0x%02x committed_no %d uncommitted_no %d",
			a_id, fast, addr, committed_no, uncommitted_no);
	}
}

void set_sources_adapters(char *o)
{
	int i, la, lb, st, end, j, k, adap;
	uint64_t all = 0xFFFFFFFFFFFFFFFF;
	char buf[1024], *arg[128], *sep;
	SAFE_STRCPY(buf, o);
	la = split(arg, buf, ARRAY_SIZE(arg), ':');
	if ((la == 1 && strlen(arg[0]) == 0) || la < 1 || la > MAX_SOURCES)
		goto ERR;

	LOG("Calculating adapter sources: [%s] ", o);
	for (i = 0; i < MAX_SOURCES; i++)
	{
		if (i >= la)
		{
			LOGM(" src=%d undefined, using all", i + 1);
			source_map[i] = all;
			continue;
		}

		source_map[i] = 0;
		if (arg[i] && arg[i][0] == '*')
		{
			if (strlen(arg[i]) != 1)
				goto ERR;
			source_map[i] = all;
			char buf2[128];
			buf2[0] = '\0';
			for (j = 0; j < MAX_ADAPTERS; j++)
				strcat(buf2, "X");
			LOGM(" src=%d using adapters %s (%lu)", i + 1, buf2, (unsigned long)source_map[i]);
			continue;
		}
		char buf2[128], *arg2[128];
		SAFE_STRCPY(buf2, arg[i]);
		lb = split(arg2, buf2, ARRAY_SIZE(arg2), ',');
		for (j = 0; j < lb; j++)
		{
			sep = strchr(arg2[j], '-');
			if (sep == NULL)
			{
				adap = map_int(arg2[j], NULL);
				if (adap < 0 || adap >= MAX_ADAPTERS)
					goto ERR;
				source_map[i] |= (unsigned long)1 << adap;
			}
			else
			{
				st = map_int(arg2[j], NULL);
				end = map_int(sep + 1, NULL);
				if (st < 0 || end < 0 || st >= MAX_ADAPTERS || end >= MAX_ADAPTERS || end < st)
					goto ERR;
				for (k = st; k <= end; k++)
				{
					adap = k;
					source_map[i] |= (unsigned long)1 << adap;
				}
			}
		}
		buf2[0] = '\0';
		for (j = 0; j < MAX_ADAPTERS; j++)
			if (source_map[i] & ((unsigned long)1 << j))
				strcat(buf2, "X");
			else
				strcat(buf2, ".");
		LOGM(" src=%d using adapters %s (%lu)", i + 1, buf2, (unsigned long)source_map[i]);
	}
	LOG("Adapter correct sources format parameter");
	return;

ERR:
	for (i = 0; i < MAX_SOURCES; i++)
		source_map[i] = all;
	LOG("Adapter sources format parameter %s, using defaults for all adapters!", strlen(arg[0]) > 0 ? "incorrect" : "missing");
}

void set_diseqc_multi(char *o)
{
	int i, la, a_id, position;
	char buf[1000], *arg[40], *sep1;
	adapter *ad;
	SAFE_STRCPY(buf, o);
	la = split(arg, buf, ARRAY_SIZE(arg), ',');
	for (i = 0; i < la; i++)
	{
		if (arg[i] && arg[i][0] == '*')
		{
			ad = NULL;
			a_id = -1;
		}
		else
		{
			a_id = map_intd(arg[i], NULL, -1);
			if (a_id < 0 || a_id >= MAX_ADAPTERS)
				continue;

			if (!a[a_id])
				a[a_id] = adapter_alloc();
			ad = a[a_id];
		}

		sep1 = strchr(arg[i], ':');

		if (!sep1)
			continue;
		position = map_intd(sep1 + 1, NULL, -1);
		if (position < 0)
			continue;
		if (ad)
		{
			ad->diseqc_multi = position;
		}
		else
		{
			opts.diseqc_multi = position;
			int j;
			for (j = 0; j < MAX_ADAPTERS; j++)
				if (a[j])
				{
					a[j]->diseqc_multi = position;
				}
		}
		LOG(
			"Setting diseqc multi adapter %d position %d",
			a_id, position);
	}
}

void set_lnb_adapters(char *o)
{
	int i, la, a_id, lnb_low, lnb_high, lnb_switch;
	char buf[1000], *arg[40], *sep1, *sep2, *sep3;
	adapter *ad;
	SAFE_STRCPY(buf, o);
	la = split(arg, buf, ARRAY_SIZE(arg), ',');
	for (i = 0; i < la; i++)
	{
		if (arg[i] && arg[i][0] == '*')
		{
			ad = NULL;
			a_id = -1;
		}
		else
		{
			a_id = map_intd(arg[i], NULL, -1);
			if (a_id < 0 || a_id >= MAX_ADAPTERS)
				continue;

			if (!a[a_id])
				a[a_id] = adapter_alloc();
			ad = a[a_id];
		}
		sep3 = NULL;
		sep1 = strchr(arg[i], ':');
		sep2 = strchr(arg[i], '-');
		if (sep2)
			sep3 = strchr(sep2 + 1, '-');

		if (!sep1 || !sep2 || !sep3)
		{
			LOG("LNB parameters not correctly specified: %s", arg[i]);
			continue;
		}

		lnb_low = map_intd(sep1 + 1, NULL, -1) * 1000;
		lnb_high = map_intd(sep2 + 1, NULL, -1) * 1000;
		lnb_switch = map_intd(sep3 + 1, NULL, -1) * 1000;
		if (lnb_low < 0 || lnb_high < 0 || lnb_switch < 0)
			continue;

		if (ad)
		{
			ad->diseqc_param.lnb_low = lnb_low;
			ad->diseqc_param.lnb_high = lnb_high;
			ad->diseqc_param.lnb_switch = lnb_switch;
			ad->diseqc_param.lnb_circular = 0;
		}
		else
		{
			opts.lnb_low = lnb_low;
			opts.lnb_high = lnb_high;
			opts.lnb_switch = lnb_switch;
			opts.lnb_circular = 0;
			int j;
			for (j = 0; j < MAX_ADAPTERS; j++)
				if (a[j])
				{
					a[j]->diseqc_param.lnb_low = lnb_low;
					a[j]->diseqc_param.lnb_high = lnb_high;
					a[j]->diseqc_param.lnb_switch = lnb_switch;
					a[j]->diseqc_param.lnb_circular = 0;
				}
		}
		LOG(
			"Setting diseqc adapter %d lnb_low %d lnb_high %d lnb_switch %d",
			a_id, lnb_low, lnb_high, lnb_switch);
	}
}

void set_diseqc_timing(char *o)
{
	int i, la, a_id;
	int before_cmd, after_cmd, after_repeated_cmd;
	int after_switch, after_burst, after_tone;
	char buf[2000], *arg[40];
	char *sep1, *sep2, *sep3, *sep4, *sep5, *sep6;
	adapter *ad;
	SAFE_STRCPY(buf, o);
	la = split(arg, buf, ARRAY_SIZE(arg), ',');
	for (i = 0; i < la; i++)
	{
		if (arg[i] && arg[i][0] == '*')
		{
			ad = NULL;
			a_id = -1;
		}
		else
		{
			a_id = map_intd(arg[i], NULL, -1);
			if (a_id < 0 || a_id >= MAX_ADAPTERS)
				continue;

			if (!a[a_id])
				a[a_id] = adapter_alloc();
			ad = a[a_id];
		}

		sep1 = strchr(arg[i], ':');
		sep2 = strchr(arg[i], '-');
		sep3 = sep2 ? strchr(sep2 + 1, '-') : NULL;
		sep4 = sep3 ? strchr(sep3 + 1, '-') : NULL;
		sep5 = sep4 ? strchr(sep4 + 1, '-') : NULL;
		sep6 = sep5 ? strchr(sep5 + 1, '-') : NULL;

		if (!sep1 || !sep2 || !sep3 || !sep4 || !sep5 || !sep6)
			continue;
		before_cmd = map_intd(sep1 + 1, NULL, -1);
		after_cmd = map_intd(sep2 + 1, NULL, -1);
		after_repeated_cmd = map_intd(sep3 + 1, NULL, -1);
		after_switch = map_intd(sep4 + 1, NULL, -1);
		after_burst = map_intd(sep5 + 1, NULL, -1);
		after_tone = map_intd(sep6 + 1, NULL, -1);
		if (before_cmd < 0 || after_cmd < 0 || after_repeated_cmd < 0 || after_switch < 0 || after_burst < 0 || after_tone < 0)
			continue;

		if (ad)
		{
			ad->diseqc_param.before_cmd = before_cmd;
			ad->diseqc_param.after_cmd = after_cmd;
			ad->diseqc_param.after_repeated_cmd = after_repeated_cmd;
			ad->diseqc_param.after_switch = after_switch;
			ad->diseqc_param.after_burst = after_burst;
			ad->diseqc_param.after_tone = after_tone;
		}
		else
		{
			opts.diseqc_before_cmd = before_cmd;
			opts.diseqc_after_cmd = after_cmd;
			opts.diseqc_after_repeated_cmd = after_repeated_cmd;
			opts.diseqc_after_switch = after_switch;
			opts.diseqc_after_burst = after_burst;
			opts.diseqc_after_tone = after_tone;
			int j;
			for (j = 0; j < MAX_ADAPTERS; j++)
				if (a[j])
				{
					a[j]->diseqc_param.before_cmd = before_cmd;
					a[j]->diseqc_param.after_cmd = after_cmd;
					a[j]->diseqc_param.after_repeated_cmd = after_repeated_cmd;
					a[j]->diseqc_param.after_switch = after_switch;
					a[j]->diseqc_param.after_burst = after_burst;
					a[j]->diseqc_param.after_tone = after_tone;
				}
		}
		LOG(
			"Setting diseqc timing for adapter %d before cmd %d after cmd %d "
			"after repeated cmd %d after switch %d after burst %d after tone %d",
			a_id, before_cmd, after_cmd, after_repeated_cmd, after_switch,
			after_burst, after_tone);
	}
}

void set_slave_adapters(char *o)
{
	int i, j, la, a_id, a_id2, master = 0;
	char buf[1000], *arg[40], *sep, *sep2;
	adapter *ad;
	SAFE_STRCPY(buf, o);
	la = split(arg, buf, ARRAY_SIZE(arg), ',');
	for (i = 0; i < la; i++)
	{
		a_id = map_intd(arg[i], NULL, -1);
		if (a_id < 0 || a_id >= MAX_ADAPTERS)
			continue;

		sep = strchr(arg[i], '-');
		a_id2 = a_id;
		if (sep)
			a_id2 = map_intd(sep + 1, NULL, -1);

		if (a_id2 < 0 || a_id2 >= MAX_ADAPTERS)
			continue;

		sep2 = strchr(arg[i], ':');
		if (sep2)
			master = map_intd(sep2 + 1, NULL, 0);

		for (j = a_id; j <= a_id2; j++)
		{
			if (!a[j])
				a[j] = adapter_alloc();

			ad = a[j];

			// make sure the source is not already set
			if (ad && ad->master_source != -1)
				continue;

			if (master >= 0 && master < MAX_ADAPTERS)
			{
				if (!a[master] && !is_adapter_disabled(master))
					a[master] = adapter_alloc();

				if (a[master] && a[master]->master_source < 0)
				{
					a[master]->master_source = -2; // force this adapter as master
					ad->master_source = master;
					LOG("Setting master adapter %d for adapter %d", ad->master_source, j);
				}
			}
		}
	}
}

void set_timeout_adapters(char *o)
{
	int i, j, la, a_id, a_id2;
	int timeout = opts.adapter_timeout / 1000;
	char buf[1000], *arg[40], *sep;
	adapter *ad;
	SAFE_STRCPY(buf, o);
	sep = strchr(buf, ':');
	if (sep)
		timeout = map_intd(sep + 1, NULL, timeout);
	la = split(arg, buf, ARRAY_SIZE(arg), ',');
	if (arg[0] && (arg[0][0] == '*'))
	{
		opts.adapter_timeout = timeout * 1000;
		int j;
		for (j = 0; j < MAX_ADAPTERS; j++)
			if (a[j])
			{
				a[j]->adapter_timeout = timeout * 1000;
			}
		LOG("Set default timeout to %d", opts.adapter_timeout);
		return;
	}
	for (i = 0; i < la; i++)
	{
		a_id = map_intd(arg[i], NULL, -1);
		if (a_id < 0 || a_id >= MAX_ADAPTERS)
			continue;

		sep = strchr(arg[i], '-');
		a_id2 = a_id;
		if (sep)
			a_id2 = map_intd(sep + 1, NULL, -1);

		if (a_id2 < 0 || a_id2 >= MAX_ADAPTERS)
			continue;

		for (j = a_id; j <= a_id2; j++)
		{
			if (!a[j])
				a[j] = adapter_alloc();

			ad = a[j];
			ad->adapter_timeout = timeout * 1000;

			LOG("Set timeout for adapter %d to %d", j, ad->adapter_timeout);
		}
	}
}

extern char *fe_delsys[];
void set_adapters_delsys(char *o)
{
	int i, la, a_id, ds;
	char buf[1000], *arg[40], *sep;
	adapter *ad;
	SAFE_STRCPY(buf, o);
	la = split(arg, buf, ARRAY_SIZE(arg), ',');
	for (i = 0; i < la; i++)
	{
		a_id = map_intd(arg[i], NULL, -1);
		if (a_id < 0 || a_id >= MAX_ADAPTERS)
			continue;

		sep = strchr(arg[i], ':');
		if (!sep)
		{
			LOG(
				"Delivery system is missing, the format is adapter_number:delivery_system\n example: 2:dvbs2");
			return;
		}
		ds = map_intd(sep + 1, fe_delsys, 0);

		if (!a[a_id])
			a[a_id] = adapter_alloc();

		ad = a[a_id];
		ad->sys[0] = ds;

		if (ad->sys[0] == SYS_DVBS2)
			ad->sys[1] = SYS_DVBS;
		if (ad->sys[0] == SYS_DVBT2)
			ad->sys[1] = SYS_DVBT;
		if (ad->sys[0] == SYS_DVBC2)
			ad->sys[1] = SYS_DVBC_ANNEX_A;

		LOG("Setting delivery system for adapter %d to %s and %s", a_id,
			get_delsys(ad->sys[0]), get_delsys(ad->sys[1]));
	}
}

void set_adapter_dmxsource(char *o)
{
	int i, j, la, st, end, fd;
	char buf[1000], *arg[40], *sep, *seps;
	adapter *ad;
	SAFE_STRCPY(buf, o);
	la = split(arg, buf, ARRAY_SIZE(arg), ',');
	for (i = 0; i < la; i++)
	{
		sep = strchr(arg[i], '-');
		seps = strchr(arg[i], ':');

		if (!seps)
			continue;

		fd = map_intd(seps + 1, NULL, -1);

		if (sep == NULL)
		{
			st = end = map_int(arg[i], NULL);
			set_disable(st, 0);
		}
		else
		{
			st = map_int(arg[i], NULL);
			end = map_int(sep + 1, NULL);
		}
		for (j = st; j <= end; j++)
		{
			if (!a[j])
				a[j] = adapter_alloc();

			ad = a[j];
			ad->dmx_source = fd;
			LOG("Setting frontend %d for adapter %d", fd, j);
		}
	}
}

void set_signal_multiplier(char *o)
{
	int i, la, a_id;
	float strength_multiplier, snr_multiplier;
	char buf[1000], *arg[40], *sep1, *sep2;
	adapter *ad;
	SAFE_STRCPY(buf, o);
	la = split(arg, buf, ARRAY_SIZE(arg), ',');
	for (i = 0; i < la; i++)
	{
		if (arg[i] && arg[i][0] == '*')
		{
			ad = NULL;
			a_id = -1;
		}
		else
		{
			a_id = map_intd(arg[i], NULL, -1);
			if (a_id < 0 || a_id >= MAX_ADAPTERS)
				continue;

			if (!a[a_id])
				a[a_id] = adapter_alloc();
			ad = a[a_id];
		}

		sep1 = strchr(arg[i], ':');
		sep2 = strchr(arg[i], '-');

		if (!sep1 || !sep2)
			continue;

		strength_multiplier = strtod(sep1 + 1, NULL);
		snr_multiplier = strtod(sep2 + 1, NULL);
		if (strength_multiplier < 0 || snr_multiplier < 0)
			continue;

		if (ad)
		{
			ad->strength_multiplier = strength_multiplier;
			ad->snr_multiplier = snr_multiplier;
		}
		else
		{
			opts.strength_multiplier = strength_multiplier;
			opts.snr_multiplier = snr_multiplier;
			int j;
			for (j = 0; j < MAX_ADAPTERS; j++)
				if (a[j])
				{
					a[j]->strength_multiplier = strength_multiplier;
					a[j]->snr_multiplier = snr_multiplier;
				}
		}
		LOG("Setting signal multipler for adapter %d strength_multiplier %.2f snr_multiplier %.2f",
			a_id, (double)strength_multiplier, (double)snr_multiplier);
	}
}

int delsys_match(adapter *ad, int del_sys)
{
	int i;
	if (!ad)
		LOG_AND_RETURN(0, "delsys_match: adapter is NULL, delsys %d", del_sys);

	if (del_sys == 0)
		LOG_AND_RETURN(0,
					   "delsys_match: requesting delsys is 0 for adapter handle %d",
					   ad->fe);

	for (i = 0; i < 10; i++)
		if (ad->sys[i] == (unsigned int)del_sys)
			return 1;
	return 0;
}

int signal_thread(sockets *s)
{
	int i, status;
	int64_t ts, ctime;
	adapter *ad;
	for (i = 0; i < MAX_ADAPTERS; i++)
	{
		if ((ad = get_adapter_nw(i)) == NULL || ad->get_signal == NULL)
			continue;
		if (ad->fe <= 0 || ad->tp.freq <= 0)
			continue;
		status = ad->status;
		if (ad->status_cnt++ <= 0) // make sure the kernel has updated the status
			continue;
		// do not get the signal when the adapter is being changed
		if (!opts.no_threads && ad->mutex.state != 0)
			continue;
		if (opts.no_threads && !ad->fast_status && status >= 0)
			continue;
		ts = getTick();
		ad->get_signal(ad);
		ctime = getTick();
		if (status < 0 || (opts.log & DEFAULT_LOG))
			LOG(
				"get_signal%s took %jd ms for adapter %d handle %d (status: %d, ber: %d, strength:%d, snr: %d, force scan %d)",
				(ad->new_gs == 1) ? "_new" : "", ctime - ts, ad->id, ad->fe,
				ad->status, ad->ber, ad->strength, ad->snr, opts.force_scan);
	}
	return 0;
}

void adapter_lock1(char *FILE, int line, int aid)
{
	adapter *ad;
	ad = get_adapter_nw(aid);
	if (!ad)
		return;
	mutex_lock1(FILE, line, &ad->mutex);
}

void adapter_unlock1(char *FILE, int line, int aid)
{
	adapter *ad;
	ad = get_adapter_nw(aid);
	if (!ad)
		return;
	mutex_unlock1(FILE, line, &ad->mutex);
}

#if 0
// unused
int get_enabled_pids(adapter *ad, int *pids, int lpids)
{
	int ep = 0, i;

	for (i = 0; i < MAX_PIDS; i++)
	{
		if (ad->pids[i].flags == 1 || ad->pids[i].flags == 2) // enabled or needed to be added
			pids[ep++] = ad->pids[i].pid;
		if (ep >= lpids)
			break;
	}

	return ep;
}

// unused
int get_all_pids(adapter *ad, int *pids, int lpids)
{
	int ep = 0, i;

	for (i = 0; i < MAX_PIDS; i++)
	{
		if ((ad->pids[i].flags > 0) && (ad->pids[i].flags < 4))
			pids[ep++] = ad->pids[i].pid;
		if (ep >= lpids)
			break;
	}

	return ep;
}

#endif

char *get_adapter_pids(int aid, char *dest, int max_size)
{
	int len = 0;
	int i;
	adapter *ad = get_adapter_nw(aid);
	dest[0] = 0;

	if (!ad)
		return dest;

	for (i = 0; i < MAX_PIDS; i++)
		if (ad->pids[i].flags == 1 || ad->pids[i].flags == 2)
		{
			int pid = ad->pids[i].pid;
			if (pid == 8192)
			{
				strlcatf(dest, max_size, len, "all,");
				break;
			}
			else
				strlcatf(dest, max_size, len, "%d,", pid);
		}
	if (len > 0)
		dest[len - 1] = 0;
	else
		strlcatf(dest, max_size, len, "none");

	return dest;
}

char *get_all_delsys(int aid, char *dest, int max_size)
{
	int i, len;
	adapter *ad = get_adapter_nw(aid);
	dest[0] = 0;
	len = 0;
	if (!ad)
		return dest;

	for (i = 0; i < MAX_DELSYS; i++)
		if (ad->sys[i] > 0)
			strlcatf(dest, max_size, len, "%s,",
					 get_delsys(ad->sys[i]));

	if (len > 0)
		dest[len - 1] = 0;

	return dest;
}

int get_adapter_ccerrs(int aid)
{
	int i, cc = 0;
	adapter *ad = get_adapter_nw(aid);
	if (!ad)
		return 0;

	for (i = 0; i < 2; i++)
		if (ad->pids[i].flags == 1)
			cc += ad->pids[i].cc_err;
	return cc;
}

int get_adapter_decerrs(int aid)
{
	int i, dec = 0;
	adapter *ad = get_adapter_nw(aid);
	if (!ad)
		return 0;

	for (i = 0; i < 2; i++)
		if (ad->pids[i].flags == 1)
			dec += ad->pids[i].dec_err;
	return dec;
}

_symbols adapters_sym[] =
	{
		{"ad_enabled", VAR_AARRAY_INT8, a, 1, MAX_ADAPTERS, offsetof(adapter, enabled)},
		{"ad_type", VAR_AARRAY_INT8, a, 1, MAX_ADAPTERS, offsetof(adapter, type)},
		{"ad_pos", VAR_AARRAY_INT, a, 1, MAX_ADAPTERS, offsetof(adapter, tp.diseqc)},
		{"ad_freq", VAR_AARRAY_INT, a, 1. / 1000, MAX_ADAPTERS, offsetof(adapter, tp.freq)},
		{"ad_strength", VAR_AARRAY_UINT16, a, 1, MAX_ADAPTERS, offsetof(adapter, strength)},
		{"ad_snr", VAR_AARRAY_UINT16, a, 1, MAX_ADAPTERS, offsetof(adapter, snr)},
		{"ad_ber", VAR_AARRAY_INT, a, 1, MAX_ADAPTERS, offsetof(adapter, ber)},
		{"ad_pol", VAR_AARRAY_INT, a, 1, MAX_ADAPTERS, offsetof(adapter, tp.pol)},
		{"ad_sr", VAR_AARRAY_INT, a, 1. / 1000, MAX_ADAPTERS, offsetof(adapter, tp.sr)},
		{"ad_bw", VAR_AARRAY_INT, a, 1. / 1000, MAX_ADAPTERS, offsetof(adapter, tp.bw)},
		{"ad_stream", VAR_AARRAY_INT, a, 1, MAX_ADAPTERS, offsetof(adapter, tp.plp_isi)},
		{"ad_fe", VAR_AARRAY_INT, a, 1, MAX_ADAPTERS, offsetof(adapter, fe)},
		{"ad_master", VAR_AARRAY_UINT8, a, 1, MAX_ADAPTERS, offsetof(adapter, master_sid)},
		{"ad_sidcount", VAR_AARRAY_UINT8, a, 1, MAX_ADAPTERS, offsetof(adapter, sid_cnt)},
		{"ad_phyad", VAR_AARRAY_INT, a, 1, MAX_ADAPTERS, offsetof(adapter, pa)},
		{"ad_phyfd", VAR_AARRAY_INT, a, 1, MAX_ADAPTERS, offsetof(adapter, fn)},
		{"ad_sys", VAR_AARRAY_INT8, a, 1, MAX_ADAPTERS, offsetof(adapter, tp.sys)},
		{"ad_allsys", VAR_FUNCTION_STRING, (void *)&get_all_delsys, 0, MAX_ADAPTERS, 0},
		{"ad_pids", VAR_FUNCTION_STRING, (void *)&get_adapter_pids, 0, MAX_ADAPTERS, 0},
		{"ad_ccerr", VAR_FUNCTION_INT, (void *)&get_adapter_ccerrs, 0, MAX_ADAPTERS, 0},
		{"ad_decerr", VAR_FUNCTION_INT, (void *)&get_adapter_decerrs, 0, MAX_ADAPTERS, 0},
		{"tuner_s2", VAR_INT, &tuner_s2, 1, 0, 0},
		{"tuner_t2", VAR_INT, &tuner_t2, 1, 0, 0},
		{"tuner_c2", VAR_INT, &tuner_c2, 1, 0, 0},
		{"tuner_t", VAR_INT, &tuner_t, 1, 0, 0},
		{"tuner_c", VAR_INT, &tuner_c, 1, 0, 0},
		{"tuner_ac", VAR_INT, &tuner_ac, 1, 0, 0},
		{"tuner_at", VAR_INT, &tuner_at, 1, 0, 0},
		{"ad_disabled", VAR_ARRAY_INT8, disabled, 1, MAX_ADAPTERS, 0},
		{NULL, 0, NULL, 0, 0, 0}};
