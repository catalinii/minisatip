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
#include <linux/ioctl.h>

#include "socketworks.h"
#include "dvb.h"
#include "adapter.h"
#include "dvbapi.h"

#ifndef DISABLE_SATIPCLIENT
#include "satipc.h"
#endif

adapter a[MAX_ADAPTERS];
extern struct struct_opts opts;
int tuner_s2, tuner_t, tuner_c, tuner_t2, tuner_c2;
extern void find_dvb_adapter(adapter *a);

void find_adapters()
{
	static int init_find_adapter;
	if (init_find_adapter == 1)
		return;
	init_find_adapter = 1;
	find_dvb_adapter(a);
#ifndef DISABLE_SATIPCLIENT
	find_satip_adapter(a);
#endif 
}

// avoid adapter close unless all the adapters can be closed
int adapter_timeout(sockets *s)
{
	int do_close = 1, i, max_close = 0;
	int rtime = getTick();
	adapter *ad = get_adapter(s->sid);
	if(!ad)
		return 1;
	
	if(ad->force_close)
		return 1;
	
	if(get_streams_for_adapter(ad->id) > 0)
	{
		ad->rtime = getTick();
		s->rtime = ad->rtime;
		LOG("Keeping the adapter %d open as there are active streams", ad->id);
		return 0;
	}
	for (i = 0; i < MAX_ADAPTERS; i++)
	{
		if (a[i].enabled && rtime - a[i].rtime < s->close_sec)
			do_close = 0;
		if (max_close < a[i].rtime)
			max_close = a[i].rtime;

	}
	LOG("Requested adapter %d close due to timeout, result %d max_rtime %d",
			s->sid, do_close, max_close);
	if (!do_close)
		s->rtime = max_close;

	return do_close;
}

void request_adapter_close(adapter *ad)
{
	ad->force_close = 1;
	sockets_timeout(ad->sock, 1);
}

int close_adapter_for_socket(sockets * s)
{
	int aid = s->sid;

	LOG("closing DVR socket %d pos %d aid %d", s->sock, s->id, aid);
	if (aid >= 0)
		close_adapter(aid);
	return 0;
}

int init_complete = 0;
int num_adapters = 0;
int init_hw()
{
	int i;

	LOG("starting init_hw %d", init_complete);
	if (init_complete)
		return num_adapters;
	num_adapters = 0;
	init_complete = 1;
	for (i = 0; i < MAX_ADAPTERS; i++)
		if ((!a[i].enabled || a[i].fe <= 0)
				&& ((a[i].pa >= 0 && a[i].fn >= 0) || a[i].sip))
		{
			if (a[i].force_disable)
				continue;
			a[i].sock = -1;
			a[i].id = i;
			a[i].fe_sock = -1;
			a[i].sock = -1;
			if (a[i].pa <= 0 && a[i].fn <= 0)
				find_adapters();
			if (a[i].open(&a[i]))
			{
				init_complete = 0;
				continue;
			}
			a[i].enabled = 1;
			if (!a[i].buf)
				a[i].buf = malloc1(opts.adapter_buffer + 10);
			if (!a[i].buf)
			{
				LOG(
						"memory allocation failed for %d bytes failed, adapter %d, trying %d bytes",
						opts.adapter_buffer, i, ADAPTER_BUFFER);
				opts.adapter_buffer = ADAPTER_BUFFER;
				a[i].buf = malloc1(opts.adapter_buffer + 10);
				if (!a[i].buf)
				{
					LOG(
							"memory allocation failed for %d bytes failed, adapter %d",
							opts.adapter_buffer, i);
					close_adapter(i);
				}
				continue;
			}
			memset(a[i].buf, 0, opts.adapter_buffer + 1);

			num_adapters++;
			init_dvb_parameters(&a[i].tp);
			mark_pids_deleted(i, -1, NULL);
			update_pids(i);
			a[i].delsys(i, a[i].fe, a[i].sys);
			a[i].master_sid = -1;
			a[i].sid_cnt = 0;
			a[i].pid_err = a[i].dec_err = 0;
			a[i].new_gs = 0;
			a[i].force_close = 0;
			a[i].rtime =getTick();
			a[i].sock = sockets_add(a[i].dvr, NULL, i, TYPE_DVR,
					(socket_action) read_dmx,
					(socket_action) close_adapter_for_socket,
					(socket_action) adapter_timeout);
			memset(a[i].buf, 0, opts.adapter_buffer + 1);
			set_socket_buffer(a[i].sock, (unsigned char*) a[i].buf,
					opts.adapter_buffer);
			sockets_timeout(a[i].sock, ADAPTER_TIMEOUT);
			if (a[i].post_init)
				a[i].post_init(&a[i]);
			LOG("done opening adapter %i fe_sys %d %d %d %d", i, a[i].sys[0],
					a[i].sys[1], a[i].sys[2], a[i].sys[3]);

		}
		else if (a[i].enabled)
			num_adapters++;
	if (num_adapters == 0)
		init_complete = 0;
	LOG("done init_hw %d", init_complete);
	if (init_complete)
		getAdaptersCount();
	return num_adapters;
}

void close_adapter(int na)
{
	init_complete = 0;

	if (na < 0 || na >= MAX_ADAPTERS || !a[na].enabled)
		return;
	LOG("closing adapter %d  -> fe:%d dvr:%d", na, a[na].fe, a[na].dvr);
	a[na].enabled = 0;
	if (a[na].close)
		a[na].close(&a[na]);
	//close all streams attached to this adapter
//	close_streams_for_adapter (na, -1);
	mark_pids_deleted(na, -1, NULL);
	update_pids(na);
	//      if(a[na].dmx>0)close(a[na].dmx);
	if (a[na].fe > 0)
		close(a[na].fe);
	if (a[na].sock > 0)
		sockets_del(a[na].sock);
	if (a[na].ca > 0)
		close(a[na].ca);
	if (a[na].fe_sock > 0)
		sockets_del(a[na].fe_sock);
	a[na].ca = 0;
	a[na].fe = 0;
	a[na].dvr = 0;
	a[na].strength = 0;
	a[na].snr = 0;

	//      if(a[na].buf)free1(a[na].buf);a[na].buf=NULL;
	LOG("done closing adapter %d", na);
}

int getAdaptersCount()
{
	int i;
	tuner_s2 = tuner_c2 = tuner_t2 = tuner_c = tuner_t = 0;
	if (opts.force_sadapter)
		tuner_s2 = opts.force_sadapter;
	if (opts.force_tadapter)
		tuner_t = opts.force_tadapter;
	if (opts.force_cadapter)
		tuner_c = opts.force_cadapter;
	for (i = 0; i < MAX_ADAPTERS; i++)
		if (a[i].enabled)
		{
			if (!opts.force_sadapter
					&& (delsys_match(&a[i], SYS_DVBS)
							|| delsys_match(&a[i], SYS_DVBS2)))
				tuner_s2++;

			if (!opts.force_tadapter && delsys_match(&a[i], SYS_DVBT)
					&& !delsys_match(&a[i], SYS_DVBT2))
				tuner_t++;

			if (!opts.force_cadapter && delsys_match(&a[i], SYS_DVBC_ANNEX_A)
					&& !delsys_match(&a[i], SYS_DVBC2))
				tuner_c++;

			if (delsys_match(&a[i], SYS_DVBT2))
				tuner_t2++;

			if (delsys_match(&a[i], SYS_DVBC2))
				tuner_c2++;
		}

	return tuner_s2 + tuner_c2 + tuner_t2 + tuner_c + tuner_t;
}

void dump_adapters()
{
	int i;

	if (!opts.log)
		return;
	LOG("Dumping adapters:");
	for (i = 0; i < MAX_ADAPTERS; i++)
		if (a[i].enabled)
			LOG("%d|f: %d sid_cnt:%d master_sid:%d del_sys: %s %s %s", i,
					a[i].tp.freq, a[i].sid_cnt, a[i].master_sid,
					get_delsys(a[i].sys[0]), get_delsys(a[i].sys[1]),
					get_delsys(a[i].sys[2]));
	dump_streams();

}

void dump_pids(int aid)
{
	int i, dp = 1;

	if (!opts.log)
		return;
	adapter *p = get_adapter(aid);
	if (!p)
		return;
	for (i = 0; i < MAX_PIDS; i++)
		if (p->pids[i].flags > 0)
		{
			if (dp)
				LOGL(2, "Dumping pids table for adapter %d, pid errors %d", aid,
						p->pid_err - p->dec_err);
			dp = 0;
			LOGL(2,
					"pid %d, fd %d, type %d packets %d, d/c errs %d/%d, flags %d, key %d, sids: %d %d %d %d %d %d %d %d",
					p->pids[i].pid, p->pids[i].fd, p->pids[i].type,
					p->pids[i].cnt, p->pids[i].dec_err, p->pids[i].err,
					p->pids[i].flags, p->pids[i].key, p->pids[i].sid[0],
					p->pids[i].sid[1], p->pids[i].sid[2], p->pids[i].sid[3],
					p->pids[i].sid[4], p->pids[i].sid[5], p->pids[i].sid[6],
					p->pids[i].sid[7]);
		}
}

int get_free_adapter(int freq, int pol, int msys, int src)
{
	int i;
	init_hw();

	i = (src > 0) ? src - 1 : 0;
	LOG("get free adapter %d - a[%d] => e:%d m:%d sid_cnt:%d f:%d pol=%d",
			src - 1, i, a[i].enabled, a[i].master_sid, a[i].sid_cnt,
			a[i].tp.freq, a[i].tp.pol);
	if (src > 0)
	{
		if (a[i].enabled)
		{
			if (a[i].sid_cnt == 0 && delsys_match(&a[i], msys))
				return i;
			if (a[i].tp.freq == freq && delsys_match(&a[i], msys))
				return i;
		}
	}
	for (i = 0; i < MAX_ADAPTERS; i++)
		//first free adapter that has the same msys
		if (a[i].enabled && a[i].sid_cnt == 0 && delsys_match(&a[i], msys))
			return i;

	for (i = 0; i < MAX_ADAPTERS; i++)
		if (a[i].enabled && a[i].tp.freq == freq && delsys_match(&a[i], msys))
		{
			if ((msys == SYS_DVBS2 || msys == SYS_DVBS) && a[i].tp.pol == pol)
				return i;
			else
				return i;
		}
	LOG("no adapter found for f:%d pol:%d msys:%d", freq, pol, msys);
	dump_adapters();
	return -1;
}

int set_adapter_for_stream(int sid, int aid)
{
	if (!get_adapter(aid))
		return -1;
	if (a[aid].master_sid == -1)
		a[aid].master_sid = sid;
	a[aid].sid_cnt++;
	LOG("set adapter %d for stream %d m:%d s:%d", aid, sid, a[aid].master_sid,
			a[aid].sid_cnt);
	return 0;
}

void close_adapter_for_stream(int sid, int aid)
{

	if (!get_adapter(aid))
		return;
	if (a[aid].master_sid == sid)
	{
		a[aid].master_sid = -1;
		fix_master_sid(aid);
	}
	if (a[aid].sid_cnt > 0)
		a[aid].sid_cnt--;
	else
		mark_pids_deleted(aid, -1, NULL);
	LOG("closed adapter %d for stream %d m:%d s:%d", aid, sid,
			a[aid].master_sid, a[aid].sid_cnt);
	// delete the attached PIDs as well
	mark_pids_deleted(aid, sid, NULL);
	update_pids(aid);
//	if (a[aid].sid_cnt == 0) 
//		close_adapter (aid);
}

int update_pids(int aid)
{
	int i, dp = 1;
	adapter *ad;
	if (aid < 0 || aid >= MAX_ADAPTERS)
		return 0;
	ad = &a[aid];

#ifndef DISABLE_DVBCSA	
	for (i = 0; i < MAX_PIDS; i++)
		if ((ad->pids[i].flags == 3))
			dvbapi_pid_del(ad, ad->pids[i].pid, &ad->pids[i]);
#endif

	for (i = 0; i < MAX_PIDS; i++)
		if ((ad->pids[i].flags == 3))
		{
			if (dp)
				dump_pids(aid);
			dp = 0;
			ad->pids[i].flags = 0;
			if (ad->pids[i].fd > 0)
				ad->del_filters(ad->pids[i].fd, ad->pids[i].pid);
			ad->pids[i].fd = 0;
			ad->pids[i].type = 0;
			ad->pids[i].filter = ad->pids[i].key = 255;
		}

	for (i = 0; i < MAX_PIDS; i++)
		if (ad->pids[i].flags == 2)
		{
			if (dp)
				dump_pids(aid);
			dp = 0;
			if (ad->pids[i].fd <= 0)
				if ((ad->pids[i].fd = ad->set_pid(ad, ad->pids[i].pid)) < 0)
				{
					request_adapter_close(ad);
					return 1; // error
				}
			ad->pids[i].flags = 1;
			if (ad->pids[i].pid == 0)
				ad->pat_processed = 0;
			ad->pids[i].cnt = 0;
			ad->pids[i].cc = 255;
			ad->pids[i].err = 0;
			ad->pids[i].dec_err = 0;
		}

	ad->commit(ad);

	return 0;
}

int tune(int aid, int sid)
{
	adapter *ad = get_adapter(aid);
	int rv = 0;

	if (!ad)
		return -400;
	ad->last_sort = getTick();
	if (sid == ad->master_sid && ad->do_tune)
	{
		ad->tp.switch_type = ad->switch_type;
		ad->tp.uslot = ad->uslot;
		ad->tp.ufreq = ad->ufreq;
		ad->tp.pin = ad->pin;

		ad->tp.committed_no = ad->committed_no;
		ad->tp.uncommitted_no = ad->uncommitted_no;

		rv = ad->tune(ad->id, &ad->tp);
		ad->status = 0;
		ad->status_cnt = 0;
		set_socket_pos(ad->sock, 0);	// flush the existing buffer	
		if (ad->sid_cnt > 1)	 // the master changed the frequency
		{
			close_streams_for_adapter(aid, sid);
			if (update_pids(aid))
				return -503;
		}
#ifdef TABLES_H
		p = find_pid(aid, 0);
		if(!p || p->flags == 3) // add pid 0 if not explicitly added
		{
			LOG("Adding pid 0 to the list of pids as not explicitly added for adapter %d", aid);
			mark_pid_add(-1, aid, 0);
		}
#endif
	}
	else
		LOG("not tuning for SID %d (do_tune=%d, master_sid=%d)", sid,
				a[aid].do_tune, a[aid].master_sid);
	if (rv < 0)
		mark_pids_deleted(aid, sid, NULL);
	if (update_pids(aid))
		return -503;
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
			if (sid == -1)  // delete all pids if sid = -1
				p->sid[j] = -1;
		return;
	}
	// sid != -1
	for (j = 0; j < MAX_STREAMS_PER_PID; j++)
		if (p->sid[j] == sid)  // delete all pids where .sid == sid
		{
			p->sid[j] = -1;
			if ((j + 1 < MAX_PIDS) && (p->sid[j + 1] >= 0))
				sort = 1;
		}

	for (j = 0; j < MAX_STREAMS_PER_PID; j++)
		if (p->sid[j] >= 0)
			cnt++;
//	if ((cnt == 0) && (p->flags != 0) && (p->type == 0 || didit))
	if ((cnt == 0) && (p->flags != 0) && (p->type == 0))
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

	if (aid < 0 || aid >= MAX_ADAPTERS)
		return;
	ad = &a[aid];

	LOG("deleting pids on adapter %d, sid %d, pids=%s", aid, sid,
			pids ? pids : "NULL");
	if (pids)
	{
		la = split(arg, pids, MAX_PIDS, ',');
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
		return 1;
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
#ifndef DISABLE_DVBCSA	
		dvbapi_pid_add(ad, _pid, p, 1);
#endif
		return 0;
	}
	// add the new pid in a new position
	for (i = 0; i < MAX_PIDS; i++)
		if (ad->pids[i].flags <= 0)
		{
			ad->pids[i].flags = 2;
			ad->pids[i].pid = _pid;
			ad->pids[i].sid[0] = sid;
			ad->pids[i].filter = ad->pids[i].key = ad->pids[i].ecm_parity = 255;
#ifndef DISABLE_DVBCSA	
			dvbapi_pid_add(ad, _pid, &ad->pids[i], 0);
#endif
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

	la = split(arg, pids, MAX_PIDS, ',');
	for (i = 0; i < la; i++)
	{
		pid = map_intd(arg[i], NULL, -1);
		if (pid == -1)
			continue;
		if (mark_pid_add(sid, aid, pid) < 0)
			return -1;
	}
	dump_pids(aid);
	return 0;
}

int set_adapter_parameters(int aid, int sid, transponder * tp)
{
	adapter *ad = get_adapter(aid);

	if (!ad)
		return -1;

	LOG("setting DVB parameters for adapter %d - master_sid %d sid %d old f:%d",
			aid, ad->master_sid, sid, ad->tp.freq);
	if (ad->master_sid == -1)
		ad->master_sid = sid; // master sid was closed

	ad->do_tune = 0;
	if (tp->freq != ad->tp.freq || tp->plp != ad->tp.plp
			|| tp->diseqc != ad->tp.diseqc
			|| (tp->pol > 0 && tp->pol != ad->tp.pol)
			|| (tp->sr > 1000 && tp->sr != ad->tp.sr)
			|| (tp->mtype > 0 && tp->mtype != ad->tp.mtype))
	{
		if (sid != ad->master_sid) // slave sid requesting to tune to a different frequency
			LOG_AND_RETURN(-1,
					"secondary stream requested tune, not gonna happen\n ad: f:%d sr:%d pol:%d plp:%d src:%d mod %d\n \
			new: f:%d sr:%d pol:%d plp:%d src:%d mod %d",
					ad->tp.freq, ad->tp.sr, ad->tp.pol, ad->tp.plp,
					ad->tp.diseqc, ad->tp.mtype, tp->freq, tp->sr, tp->pol,
					tp->plp, tp->diseqc, tp->mtype);
					
		mark_pids_deleted(aid, -1, NULL);
		if (update_pids(aid))
			return -1;
		ad->do_tune = 1;
	}

	copy_dvb_parameters(tp, &ad->tp);

	if (ad->tp.pids) // pids can be specified in SETUP and then followed by a delpids in PLAY, make sure the behaviour is right
	{
		mark_pids_deleted(aid, sid, NULL);  // delete all the pids for this 
		if (mark_pids_add(sid, aid, ad->tp.pids) < 0)
			return -1;
	}

	if (ad->tp.dpids)
	{
		mark_pids_deleted(aid, sid, ad->tp.dpids);
	}
	if (ad->tp.apids)
	{
		if (mark_pids_add(sid, aid, ad->tp.apids ? ad->tp.apids : ad->tp.pids)
				< 0)
			return -1;
	}

	if (ad->tp.x_pmt)
	{
		char *arg[64];
		int i, la;
		la = split(arg, ad->tp.x_pmt, 64, ',');
		for (i = 0; i < la; i++)
		{
			int pmt = map_int(arg[i], NULL);
			if (pmt <= 0)
				continue;
			SPid *cp = find_pid(ad->id, pmt);
			if (!cp)
				mark_pid_add(-1, aid, pmt);
			cp = find_pid(ad->id, pmt);
			if (!cp)
				continue;
			cp->type |= TYPE_PMT;
		}
	}

	if (0 && (ad->tp.apids || ad->tp.pids || ad->tp.dpids))
		dump_pids(aid);
	return 0;
}

adapter *
get_adapter1(int aid, char *file, int line, int warning)
{
	if (aid < 0 || aid >= MAX_ADAPTERS || !a[aid].enabled)
	{
		if (warning)
			LOG("%s:%d: get_adapter returns NULL for adapter_id %d", file, line,
					aid);
		return NULL;
	}
	return &a[aid];
}

char* get_stream_pids(int s_id, char *dest, int max_size);
char *
describe_adapter(int sid, int aid, char *dad, int ld)
{
	int i = 0, ts, j, use_ad, len;
	transponder *t;
	adapter *ad;
	streams *ss;
	int status = 1, strength = 255, snr = 15;

	ss = get_sid(sid);

	use_ad = 1;
	if (!(ad = get_adapter(aid)) || (ss && !ss->do_play))
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
	memset(dad, 0, sizeof(dad));
	// do just max 3 signal check 1s after tune
	if (use_ad && (!ad->sip)
			&& ((ad->status <= 0 && ad->status_cnt < 8 && ad->status_cnt++ > 4)
					|| opts.force_scan))
	{
		int new_gs = 1;
		ts = getTick();
		if (ad->new_gs == 0
				&& (new_gs = get_signal_new(ad->fe, &ad->status, &ad->ber,
						&ad->strength, &ad->snr)))
			get_signal(ad->fe, &ad->status, &ad->ber, &ad->strength, &ad->snr);
		else if (new_gs)
			get_signal(ad->fe, &ad->status, &ad->ber, &ad->strength, &ad->snr);

		if (ad->status > 0 && new_gs != 0) // we have signal but no new stats, don't try to get them from now on until adapter close
			ad->new_gs = 1;

		if (ad->max_strength <= ad->strength)
			ad->max_strength = (ad->strength > 0) ? ad->strength : 1;
		if (ad->max_snr <= ad->snr)
			ad->max_snr = (ad->snr > 0) ? ad->snr : 1;
		LOG(
				"get_signal%s took %d ms for adapter %d handle %d (status: %d, ber: %d, strength:%d, snr: %d, max_strength: %d, max_snr: %d %d)",
				new_gs ? "" : "_new", getTick() - ts, aid, ad->fe, ad->status,
				ad->ber, ad->strength, ad->snr, ad->max_strength, ad->max_snr,
				opts.force_scan);
		if (ad->snr > 4096)
			new_gs = 0;
		if (new_gs)
		{
			ad->strength = ad->strength * 255.0 / ad->max_strength;
			ad->snr = ad->snr * 255.0 / ad->max_snr;
		}
		else
		{
			ad->strength = ad->strength >> 8;
			ad->snr = ad->snr >> 8;
		}
	}
	if (use_ad)
	{
		strength = ad->strength;
		snr = ad->snr >> 4;
		status = (ad->status & FE_HAS_LOCK) > 0;
	}
	if (t->sys == 0)
		len = snprintf(dad, ld, "ver=1.0;src=1;tuner=%d,0,0,0,0,,,,,,,;pids=",
				aid + 1);
	else if (t->sys == SYS_DVBS || t->sys == SYS_DVBS2)
		len =
				snprintf(dad, ld,
						"ver=1.0;src=%d;tuner=%d,%d,%d,%d,%d,%s,%s,%s,%s,%s,%d,%s;pids=",
						t->diseqc, aid + 1, strength, status, snr,
						t->freq / 1000, get_pol(t->pol),
						get_modulation(t->mtype), get_pilot(t->plts),
						get_rolloff(t->ro), get_delsys(t->sys), t->sr / 1000,
						get_fec(t->fec));
	else if (t->sys == SYS_DVBT || t->sys == SYS_DVBT2)
		len =
				snprintf(dad, ld,
						"ver=1.1;src=%d;tuner=%d,%d,%d,%d,%.2f,%d,%s,%s,%s,%s,%s,%d,%d,%d;pids=",
						t->diseqc, aid + 1, strength, status, snr,
						(double) t->freq / 1000, t->bw, get_delsys(t->sys),
						get_tmode(t->tmode), get_modulation(t->mtype),
						get_gi(t->gi), get_fec(t->fec), t->plp, t->t2id, t->sm);
	else
		len =
				snprintf(dad, ld,
						"ver=1.2;src=%d;tuner=%d,%d,%d,%d,%.2f,8,%s,%s,%d,%d,%d,%d,%d;pids=",
						t->diseqc, aid + 1, strength, status, snr,
						(double) t->freq / 1000, get_delsys(t->sys),
						get_modulation(t->mtype), t->sr, t->c2tft, t->ds,
						t->plp, t->inversion);

	if (use_ad)
		len += strlen(get_stream_pids(sid, dad + len, ld - len));

	if (!use_ad && (t->apids || t->pids))
		len += snprintf(dad + len, ld - len, "%s,",
				t->pids ? t->pids : t->apids);

	LOGL(5, "describe_adapter: sid %d, aid %d => %s", sid, aid, dad);

	return dad;
}

// sorting the pid list in order to get faster the pids that are frequestly used
void sort_pids(int aid)
{
	int b, i;
	SPid pp;
	SPid *p;

	if (!get_adapter(aid))
		return;
	p = a[aid].pids;
	b = 1;
	while (b)
	{
		b = 0;
		for (i = 0; i < MAX_PIDS - 1; i++)
			if (p[i].cnt < p[i + 1].cnt)
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

	for (i = 0; i < MAX_ADAPTERS; i++)
		if (a[i].buf)
			free1(a[i].buf);

}

void set_disable(int ad, int v)
{
	if (ad >= 0 && ad < MAX_ADAPTERS)
		a[ad].force_disable = v;
}

void enable_adapters(char *o)
{
	int i, la, st, end, j;
	char buf[100], *arg[20], *sep;
	for (i = 0; i < MAX_ADAPTERS; i++)
		set_disable(i, 1);
	strncpy(buf, o, sizeof(buf));

	la = split(arg, buf, sizeof(arg), ',');
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
	int i, la, a_id, slot, freq, pin;
	char buf[100], *arg[20], *sep1, *sep2, *sep3;

	strncpy(buf, o, sizeof(buf));
	la = split(arg, buf, sizeof(arg), ',');
	for (i = 0; i < la; i++)
	{
		a_id = map_intd(arg[i], NULL, -1);
		if (a_id < 0 || a_id >= MAX_ADAPTERS)
			continue;
		sep1 = strchr(arg[i], ':');
		sep2 = strchr(arg[i], '-');

		if (!sep1 || !sep2)
			continue;
		slot = map_intd(sep1 + 1, NULL, -1);
		freq = map_intd(sep2 + 1, NULL, -1);
		if (slot < 0 || freq < 0)
			continue;
		sep3 = strchr(sep2 + 1, '-');
		pin = map_intd(sep3, NULL, 0);
		a[a_id].uslot = slot;
		a[a_id].ufreq = freq;
		a[a_id].switch_type = type;
		a[a_id].pin = pin;
		LOGL(0, "Setting %s adapter %d slot %d freq %d",
				type == SWITCH_UNICABLE ? "unicable" : "jess", a_id, slot, freq);
	}
}

void set_diseqc_adapters(char *o)
{
	int i, la, a_id, committed_no, uncommitted_no;
	char buf[100], *arg[20], *sep1, *sep2;

	strncpy(buf, o, sizeof(buf));
	la = split(arg, buf, sizeof(arg), ',');
	for (i = 0; i < la; i++)
	{
		a_id = map_intd(arg[i], NULL, -1);
		if (a_id < 0 || a_id >= MAX_ADAPTERS)
			continue;
		sep1 = strchr(arg[i], ':');
		sep2 = strchr(arg[i], '-');

		if (!sep1 || !sep2)
			continue;
		committed_no = map_intd(sep1 + 1, NULL, -1);
		uncommitted_no = map_intd(sep2 + 1, NULL, -1);
		if (committed_no < 0 || uncommitted_no < 0)
			continue;

		a[a_id].committed_no = committed_no;
		a[a_id].uncommitted_no = uncommitted_no;
		LOGL(0, "Setting diseqc adapter %d committed_no %d uncommitted_no %d",
				a_id, committed_no, uncommitted_no);
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
		if (ad->sys[i] == (unsigned int) del_sys)
			return 1;
	return 0;

}

void reset_pids_type(int aid)
{
	int i;
	adapter *ad = get_adapter(aid);
	if (!ad)
		return;
	LOG("clearing type and pat processed for adapter %d", aid);
	for (i = 0; i < MAX_PIDS; i++)
		if (ad->pids[i].flags > 0)
		{
			ad->pids[i].type = 0;
			ad->pids[i].key = 255;
			ad->pids[i].filter = 255;
			if (ad->pids[i].sid[0] == -1)
				ad->pids[i].flags = 3;
		}
	ad->pat_processed = 0;
	ad->transponder_id = -1;
	ad->pat_ver = -1;
}

void reset_pids_type_for_key(int aid, int key)
{
	int i;
	adapter *ad = get_adapter(aid);
	if (!ad)
		return;
	LOG("clearing type for key %d for adapter %d", key, aid);
	for (i = 0; i < MAX_PIDS; i++)
		if ((ad->pids[i].flags > 0) && (ad->pids[i].key == key))
		{
			ad->pids[i].type = 0;
			ad->pids[i].key = 255;
			ad->pids[i].filter = 255;
			if (ad->pids[i].sid[0] == -1)
				ad->pids[i].flags = 3;
		}
}

int get_enabled_pids(adapter *ad, int *pids, int lpids)
{
	int ep = 0, i;

	for (i = 0; i < MAX_PIDS; i++)
	{
		if (ad->pids[i].flags == 1)
			pids[ep++] = ad->pids[i].pid;
		if (ep >= lpids)
			break;
	}

	return ep;
}

char* get_adapter_pids(int aid, char *dest, int max_size)
{
	int len = 0;
	int pids[MAX_PIDS];
	int lp, i;
	adapter *ad = get_adapter_nw(aid);
	dest[0] = 0;

	if (!ad)
		return dest;

	lp = get_enabled_pids(ad, pids, MAX_PIDS);
	for (i = 0; i < lp; i++)
	{
		if (pids[i] == 8192)
			len += snprintf(dest + len, max_size - len, "8192");
		else
			len += snprintf(dest + len, max_size - len, "%d,", pids[i]);
	}
	if (len > 0)
		dest[len - 1] = 0;
	else 
		snprintf(dest + len, max_size - len, "none");

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
		if (a[aid].sys[i] > 0)
			len += snprintf(dest + len, max_size - len, "%s,",
					get_delsys(a[aid].sys[i]));

	if (len > 0)
		dest[len - 1] = 0;

	return dest;
}

_symbols adapters_sym[] =
		{
		{ "ad_enabled", VAR_ARRAY_INT8, &a[0].enabled, 1, MAX_ADAPTERS,
				sizeof(a[0]) },
				{ "ad_type", VAR_ARRAY_INT8, &a[0].type, 1, MAX_ADAPTERS,
						sizeof(a[0]) },
				{ "ad_freq", VAR_ARRAY_INT, &a[0].tp.freq, 1. / 1000,
				MAX_ADAPTERS, sizeof(a[0]) },
				{ "ad_strength", VAR_ARRAY_UINT16, &a[0].strength, 1,
						MAX_ADAPTERS, sizeof(a[0]) },
				{ "ad_snr", VAR_ARRAY_UINT16, &a[0].snr, 1, MAX_ADAPTERS,
						sizeof(a[0]) },
				{ "ad_ber", VAR_ARRAY_UINT16, &a[0].ber, 1, MAX_ADAPTERS,
						sizeof(a[0]) },
				{ "ad_pol", VAR_ARRAY_INT8, &a[0].tp.pol, 1, MAX_ADAPTERS,
						sizeof(a[0]) },
				{ "ad_sr", VAR_ARRAY_INT, &a[0].tp.sr, 1. / 1000, MAX_ADAPTERS,
						sizeof(a[0]) },
				{ "ad_bw", VAR_ARRAY_INT, &a[0].tp.bw, 1. / 1000, MAX_ADAPTERS,
						sizeof(a[0]) },
				{ "ad_diseqc", VAR_ARRAY_INT, &a[0].tp.diseqc, 1, MAX_ADAPTERS,
						sizeof(a[0]) },
				{ "ad_fe", VAR_ARRAY_INT, &a[0].tp.fe, 1, MAX_ADAPTERS,
						sizeof(a[0]) },
				{ "ad_satip", VAR_ARRAY_PSTRING, &a[0].sip, 1, MAX_ADAPTERS,
						sizeof(a[0]) },
				{ "ad_master", VAR_ARRAY_UINT8, &a[0].master_sid, 1,
						MAX_ADAPTERS, sizeof(a[0]) },
				{ "ad_sidcount", VAR_ARRAY_UINT8, &a[0].sid_cnt, 1,
						MAX_ADAPTERS, sizeof(a[0]) },
				{ "ad_phyad", VAR_ARRAY_INT, &a[0].pa, 1, MAX_ADAPTERS,
						sizeof(a[0]) },
				{ "ad_phyfd", VAR_ARRAY_INT, &a[0].fn, 1, MAX_ADAPTERS,
						sizeof(a[0]) },
				{ "ad_sys", VAR_ARRAY_INT, &a[0].tp.sys, 1, MAX_ADAPTERS,
						sizeof(a[0]) },
				{ "ad_allsys", VAR_FUNCTION_STRING, (void *) &get_all_delsys, 0,
						0, 0 },
				{ "ad_pids", VAR_FUNCTION_STRING, (void *) &get_adapter_pids, 0,
						0, 0 },
				{ "tuner_s2", VAR_INT, &tuner_s2, 1, 0, 0 },
				{ "tuner_t2", VAR_INT, &tuner_t2, 1, 0, 0 },
				{ "tuner_c2", VAR_INT, &tuner_c2, 1, 0, 0 },
				{ "tuner_t", VAR_INT, &tuner_t, 1, 0, 0 },
				{ "tuner_c", VAR_INT, &tuner_c, 1, 0, 0 },
				{ NULL, 0, NULL, 0, 0 } };
