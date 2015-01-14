#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>
#include <poll.h>
#include <linux/ioctl.h>

#include "socketworks.h"
#include "dvb.h"
#include "adapter.h"

adapter a[MAX_ADAPTERS];
extern struct struct_opts opts;

void
find_adapters ()
{
	int na = 0;
	char buf[100];
	int fd;
	int i = 0,
		j = 0;

	for (i = 0; i < 8; i++)
		for (j = 0; j < 8; j++)
	{
		sprintf (buf, "/dev/dvb/adapter%d/frontend%d", i, j);
		fd = open (buf, O_RDONLY | O_NONBLOCK);
		//LOG("testing device %s -> fd: %d",buf,fd);
		if (fd >= 0)
		{
			a[na].pa = i;
			a[na].fn = j;
			close (fd);
			na++;
			if (na == MAX_ADAPTERS)
				return;
		}
	}
	for (na; na < MAX_ADAPTERS; na++)
		a[na].pa = a[na].fn = -1;
}


int
close_adapter_for_socket (sockets * s)
{
	int aid = s->sid;

	LOG ("closing DVR socket %d pos %d aid %d", s->sock, s->sock_id, aid);
	if (aid >= 0)
		close_adapter (aid);
}


int init_complete = 0;
int num_adapters = 0;
int
init_hw ()
{
	int i,
		na;
	char buf[100];

	na = 0;
	LOG ("starting init_hw %d", init_complete);
	if (init_complete)
		return num_adapters;
	num_adapters = 0;
	init_complete = 1;
	for (i = 0; i < MAX_ADAPTERS; i++)
		if ((!a[i].enabled || a[i].fe <= 0) && (a[i].pa >= 0 && a[i].fn >= 0))
	{
		int k;

		a[i].sock = -1;
		if (a[i].pa <= 0 && a[i].fn <= 0)
			find_adapters ();
		LOG ("trying to open [%d] adapter %d and frontend %d", i, a[i].pa,
			a[i].fn);
		sprintf (buf, "/dev/dvb/adapter%d/frontend%d", a[i].pa, a[i].fn);
		a[i].fe = open (buf, O_RDWR | O_NONBLOCK);
		sprintf (buf, "/dev/dvb/adapter%d/dvr%d", a[i].pa, a[i].fn);
		a[i].dvr = open (buf, O_RDONLY | O_NONBLOCK);
		if (a[i].fe < 0 || a[i].dvr < 0)
		{
			printf ("Could not open %s in RW mode\n", buf);
			if (a[i].fe >= 0)
				close (a[i].fe);
			if (a[i].dvr >= 0)
				close (a[i].dvr);
			a[i].fe = a[i].dvr = -1;
			continue;
		}

		a[i].enabled = 1;
		if (!a[i].buf)
			a[i].buf = malloc1 (ADAPTER_BUFFER + 10);
		if (!a[i].buf)
		{
			LOG ("memory allocation failed for %d bytes failed, adapter %d",
				ADAPTER_BUFFER, i);
			close_adapter (i);
			continue;
		}
		memset (a[i].buf, 0, ADAPTER_BUFFER + 1);

		num_adapters++;
		LOG ("opened DVB adapter %d fe:%d dvr:%d", i, a[i].fe, a[i].dvr);
		if (ioctl (a[i].dvr, DMX_SET_BUFFER_SIZE, opts.dvr) < 0)
			perror ("couldn't set DVR buffer size");
		else
			LOG ("Done setting DVR buffer to %d bytes", DVR_BUFFER);
		init_dvb_parameters (&a[i].tp);
		mark_pids_deleted (i, -1, NULL);
		update_pids (i);
		a[i].tp.sys = dvb_delsys (a[i].fe);
		a[i].master_sid = -1;
		a[i].sid_cnt = 0;
		a[i].sock =
			sockets_add (a[i].dvr, NULL, i, TYPE_DVR, (socket_action) read_dmx,
			(socket_action) close_adapter_for_socket, NULL);
		memset (a[i].buf, 0, ADAPTER_BUFFER + 1);
		set_socket_buffer (a[i].sock, a[i].buf, ADAPTER_BUFFER);
		sockets_timeout (a[i].sock, 60000);
		LOG ("done opening adapter %i fe_sys %d", i, a[i].tp.sys);

	}
	else if (a[i].enabled)
		num_adapters++;
	if (num_adapters == 0)
		init_complete = 0;
	LOG ("done init_hw %d", init_complete);
	return num_adapters;
}


void
close_adapter (int na)
{
	init_complete = 0;

	if (na < 0 || na >= MAX_ADAPTERS || !a[na].enabled)
		return;
	LOG ("closing adapter %d  -> fe:%d dvr:%d", na, a[na].fe, a[na].dvr);
	a[na].enabled = 0;
								 //close all streams attached to this adapter
	close_streams_for_adapter (na, -1);
	mark_pids_deleted (na, -1, NULL);
	update_pids (na);
	//      if(a[na].dmx>0)close(a[na].dmx);
	if (a[na].fe > 0)
		close (a[na].fe);
	if (a[na].sock >= 0)
		sockets_del (a[na].sock);
	a[na].fe = 0;
	//      if(a[na].buf)free1(a[na].buf);a[na].buf=NULL;
	LOG ("done closing adapter %d", na);
}


int
getS2Adapters ()
{
	int i,
		s2 = 0;

	if (opts.force_sadapter)
		LOG_AND_RETURN (opts.force_sadapter,
			"Returning %d DVB-S adapters as requested",
			opts.force_sadapter);
	init_hw ();
	for (i = 0; i < MAX_ADAPTERS; i++)
		if (a[i].enabled && (a[i].tp.sys == SYS_DVBS || a[i].tp.sys == SYS_DVBS2))
			s2++;
	//      return 1;
	return s2;
}


int
getTAdapters ()
{
	int i,
		t = 0;

	if (opts.force_tadapter)
		return opts.force_tadapter;
	init_hw ();
	for (i = 0; i < MAX_ADAPTERS; i++)
		if (a[i].enabled && (a[i].tp.sys == SYS_DVBT || a[i].tp.sys == SYS_DVBT2))
			t++;
	return t;
}


void
dump_adapters ()
{
	int i;

	if (!opts.log)
		return;
	LOG ("Dumping adapters:");
	for (i = 0; i < MAX_ADAPTERS; i++)
		if (a[i].enabled)
			LOG ("%d|f: %d sid_cnt:%d master_sid:%d", i, a[i].tp.freq, a[i].sid_cnt,
				a[i].master_sid);
	dump_streams ();

}


void
dump_pids (int aid)
{
	int i,dp = 1;

	if (!opts.log)
		return;
	adapter *p = &a[aid];

	for (i = 0; i < MAX_PIDS; i++)
		if (p->pids[i].flags > 0)
	{
		if(dp)
			LOG ("Dumping pids table for adapter %d : ", aid);
		dp = 0;
		LOG
			("pid = %d, packets = %d, fd = %d, flags = %d, sids: %d %d %d %d %d %d %d %d",
			p->pids[i].pid, p->pids[i].cnt, p->pids[i].fd, p->pids[i].flags,
			p->pids[i].sid[0], p->pids[i].sid[1], p->pids[i].sid[2],
			p->pids[i].sid[3], p->pids[i].sid[4], p->pids[i].sid[5],
			p->pids[i].sid[6], p->pids[i].sid[7]);
	}
}


int
get_free_adapter (int freq, int pol, int msys, int src)
{
	int i;
	int omsys = msys;

	i = (src >= 0) ? src : 0;
	LOG ("get free adapter %d - a[%d] => e:%d m:%d sid_cnt:%d f:%d\n", src, i,
		a[i].enabled, a[i].master_sid, a[i].sid_cnt, a[i].tp.freq);
	if (src >= 0)
	{
		if (!a[src].enabled)
		{
			if (a[src].sid_cnt == 0)
				return src;
			if (a[src].tp.freq == freq)
				return src;
		}
	}
	for (i = 0; i < MAX_ADAPTERS; i++)
								 //first free1 adapter that has the same msys
		if (a[i].enabled && a[i].sid_cnt == 0 && a[i].tp.sys == msys)
			return i;
	if (msys == SYS_DVBS)
		msys = SYS_DVBS2;
	if (msys == SYS_DVBT)
		msys = SYS_DVBT2;
	for (i = 0; i < MAX_ADAPTERS; i++)
								 //first free1 adapter that supports also msys (for example: DVBS2 will match DVBS streams)
		if (a[i].enabled && a[i].sid_cnt == 0 && a[i].tp.sys == msys)
			return i;
	for (i = 0; i < MAX_ADAPTERS; i++)
		if (a[i].enabled && a[i].tp.freq == freq
		&& (a[i].tp.sys == msys || a[i].tp.sys == omsys))
			if (msys == SYS_DVBS2 && a[i].tp.pol == pol)
				return i;
	else
		return i;
	LOG ("no adapter found for %d %c %d", freq, pol, msys);
	dump_adapters ();
	return -1;
}


int
set_adapter_for_stream (int sid, int aid)
{
	if (a[aid].master_sid == -1)
		a[aid].master_sid = sid;
	a[aid].sid_cnt++;
	LOG ("set adapter %d for stream %d m:%d s:%d", aid, sid, a[aid].master_sid,
		a[aid].sid_cnt);
}


int
close_adapter_for_stream (int sid, int aid)
{

	if (aid < 0 || aid >= MAX_ADAPTERS || !a[aid].enabled)
		return;
	if (a[aid].master_sid == sid)
		a[aid].master_sid = -1;
	if (a[aid].sid_cnt > 0)
		a[aid].sid_cnt--;
	LOG ("closed adapter %d for stream %d m:%d s:%d", aid, sid,
		a[aid].master_sid, a[aid].sid_cnt);
								 // delete the attached PIDs as well
	mark_pids_deleted (aid, sid, NULL);
	update_pids (aid);
	if (a[aid].sid_cnt == 0)
		close_adapter (aid);
}


int
update_pids (int aid)
{
	int i, j, dp=1;
	adapter *ad = &a[aid];
	for (i = 0; i < MAX_PIDS; i++)
		if (a[aid].pids[i].flags == 3)
	{
		if(dp && opts.log==2)dump_pids (aid);
		dp = 0;
		a[aid].pids[i].flags = 0;
		if (a[aid].pids[i].fd > 0)
			del_filters (a[aid].pids[i].fd, a[aid].pids[i].pid);
		a[aid].pids[i].fd = 0;
		a[aid].pids[i].cnt = 0;

	}

	for (i = 0; i < MAX_PIDS; i++)
		if (a[aid].pids[i].flags == 2)
	{
		if(dp && opts.log==2)dump_pids (aid);
		dp = 0;
		a[aid].pids[i].flags = 1;
		if (a[aid].pids[i].fd <= 0)
			a[aid].pids[i].fd = set_pid (a[aid].pa, a[aid].fn, a[aid].pids[i].pid);
		a[aid].pids[i].cnt = 0;
	}
	return 0;
}


int
tune (int aid, int sid)
{
	adapter *ad = &a[aid];
	int i,
		rv = 0;

	ad->last_sort = getTick ();
	if (sid == ad->master_sid && ad->do_tune)
	{
		rv = tune_it_s2 (ad->fe, &ad->tp);
		a[aid].status = 0;
		a[aid].status_cnt = 0;
		if (a[aid].sid_cnt > 1)	 // the master changed the frequency
		{
			close_streams_for_adapter (aid, sid);
			update_pids (aid);
		}
	}
	else
		LOG ("not tunning for SID %d (do_tune=%d, master_sid=%d)", sid,
			a[aid].do_tune, a[aid].master_sid);
	if (rv == -1)
		mark_pids_deleted (aid, sid, NULL);
	update_pids (aid);
	return rv;
}


void
								 //pids==NULL -> delete all pids
mark_pids_deleted (int aid, int sid, char *pids)
{
	int i,
		j,
		la,
		k,
		cnt;
	adapter *ad;
	char *arg[MAX_PIDS];
	int p[MAX_PIDS];

	LOG ("deleting pids on adapter %d, sid %d, pids=%s", aid, sid,
		pids ? pids : "NULL");
	if (pids)
	{
		la = split (arg, pids, MAX_PIDS, ',');
		for (i = 0; i < la; i++)
			p[i] = map_int (arg[i], NULL);
	}

	ad = &a[aid];

	for (i = 0; i < MAX_PIDS; i++)
	{
		if (pids == NULL)
		{
			if (sid == -1 && ad->pids[i].flags != 0) ad->pids[i].flags = 3;
			for (j = 0; j < MAX_STREAMS_PER_PID; j++)
				if (sid == -1)
								 // delete all pids if sid = -1
					ad->pids[i].sid[j] = -1;
			else if (ad->pids[i].sid[j] == sid)
								 // delete all pids where .sid == sid
				ad->pids[i].sid[j] = -1;
			if (sid != -1)
			{
				int cnt = 0;

				for (j = 0; j < MAX_STREAMS_PER_PID; j++)
					if (ad->pids[i].sid[j] >= 0)
						cnt++;
				if (cnt == 0 && ad->pids[i].flags != 0)
					ad->pids[i].flags = 3;
			}
		}
		else
		{
			for (j = 0; j < la; j++)
				if (p[j] == ad->pids[i].pid && ad->pids[i].flags > 0)
			{
				cnt = 0;
				for (k = 0; k < MAX_STREAMS_PER_PID; k++)
				{
					if (ad->pids[i].sid[k] == sid)
						ad->pids[i].sid[k] = -1;
					if (ad->pids[i].sid[k] != -1)
						cnt++;
				}
				if (cnt == 0)
					ad->pids[i].flags = 3;
			}
								 //make sure that -1 will be after all the pids
			for (j = 0; j < MAX_STREAMS_PER_PID - 1; j++)
				if (ad->pids[i].sid[j + 1] > ad->pids[i].sid[j])
			{
				unsigned char t = ad->pids[i].sid[j];

				ad->pids[i].sid[j] = ad->pids[i].sid[j + 1];
				ad->pids[i].sid[j + 1] = t;
			}

		}
	}

}


int
mark_pids_add (int sid, int aid, char *pids)
{
	int i,
		j,
		la,
		k,
		found;
	adapter *ad;
	char *arg[MAX_PIDS];
	int p[MAX_PIDS];

	if (!pids)
		return;
	if (pids)
	{
		la = split (arg, pids, MAX_PIDS, ',');
		for (i = 0; i < la; i++)
			p[i] = map_int (arg[i], NULL);
	}
	ad = &a[aid];
	for (j = 0; j < la; j++)
	{
		found = 0;
		//              LOG("processing pid %d",p[j]);
								 // check if the pid already exists, if yes add the sid
		for (i = 0; i < MAX_PIDS; i++)
			if (ad->pids[i].flags > 0 && ad->pids[i].pid == p[j])
		{
			LOG ("found already existing pid %d on pos %i flags %d", p[j], i,
				ad->pids[i].flags);
			for (k = 0; k < MAX_STREAMS_PER_PID; k++)
				if (ad->pids[i].sid[k] == -1 || ad->pids[i].sid[k] == sid)
			{
				if (ad->pids[i].flags == 3)
					ad->pids[i].flags = 2;
				ad->pids[i].sid[k] = sid;
				found = 1;
				break;
			}
			if (!found)
			{
				LOG ("too many streams for PID %d adapter %d", p[j], aid);
				return -1;
			}
		}
		if (!found)
			for (i = 0; i < MAX_PIDS; i++)
								 // if no mark the pid for add
				if (ad->pids[i].flags <= 0)
				{
					ad->pids[i].flags = 2;
					ad->pids[i].pid = p[j];
					ad->pids[i].sid[0] = sid;
					found = 1;
					break;
				}
		if (!found)
		{
			LOG ("MAX_PIDS (%d) reached for adapter %d in adding PID: %d",
				MAX_PIDS, aid, p[j]);
			dump_pids (aid);
			dump_adapters ();
			return -1;
		}
	}
	//      LOG("Mark_pids_add failed - too ");
	return 0;
}


int
set_adapter_parameters (int aid, int sid, transponder * tp)
{
	int i;

	LOG ("setting DVB parameters for adapter %d - master_sid %d sid %d old f:%d", aid,
		a[aid].master_sid, sid, a[aid].tp.freq);
	if (a[aid].master_sid == -1)
		a[aid].master_sid = sid; // master sid was closed
	if ((sid != a[aid].master_sid) && (tp->freq != a[aid].tp.freq))
		return -1;				 // slave sid requesting to tune to a different frequency
	a[aid].do_tune = 0;
	if (tp->freq != a[aid].tp.freq
		|| (tp->pol != -1 && tp->pol != a[aid].tp.pol))
	{
		mark_pids_deleted (aid, -1, NULL);
		update_pids (aid);
		a[aid].do_tune = 1;
	}
								 // just 1 stream per adapter and pids= specified
	if (a[aid].sid_cnt == 1 && tp->pids)
	{
		mark_pids_deleted (aid, -1, NULL);
	}
	copy_dvb_parameters (tp, &a[aid].tp);

	if (a[aid].tp.pids)			 // pids can be specified in SETUP and then followed by a delpids in PLAY, make sure the behaviour is right
	{
		if (mark_pids_add
			(sid, aid, a[aid].tp.apids ? a[aid].tp.apids : a[aid].tp.pids) < 0)
			return -1;
	}

	if (a[aid].tp.dpids)
	{
		char *arg[MAX_PIDS];

		mark_pids_deleted (aid, sid, a[aid].tp.dpids);

	}
	if (a[aid].tp.apids)
	{
		if (mark_pids_add
			(sid, aid, a[aid].tp.apids ? a[aid].tp.apids : a[aid].tp.pids) < 0)
			return -1;
	}
	if (0 && (a[aid].tp.apids || a[aid].tp.pids || a[aid].tp.dpids))
		dump_pids (aid);
	return 0;
}


adapter *
get_adapter (int aid)
{
	if (aid < 0 || aid >= MAX_ADAPTERS || !a[aid].enabled)
	{
		LOG ("Invalid adapter_id %d or not enabled", aid);
		return NULL;
	}
	return &a[aid];
}


char dad[1000];
char *
describe_adapter (int aid)
{
	int i = 0,
		x,
		ts;
	transponder *t;
	adapter *ad;

	if (aid < 0 || aid > MAX_ADAPTERS)
		return NULL;
	t = &a[aid].tp;
	ad = &a[aid];
	memset (dad, 0, sizeof (dad));
	x = 0;
	if (ad->status == 0 && ad->status_cnt++<8 && ad->status_cnt++>4)  // do just max 3 signal check 1s after tune
	{
		ts = getTick ();
		get_signal (ad->fe, &ad->status, &ad->ber, &ad->strength, &ad->snr);
		LOG
			("get_signal took %d ms for adapter %d handle %d (status: %d, ber: %d, strength:%d, snr: %d)",
			getTick () - ts, aid, ad->fe, ad->status, ad->ber, ad->strength,
			ad->snr);
	}
	if (t->sys == SYS_DVBS || t->sys == SYS_DVBS2)
		sprintf (dad, "ver=1.1;src=%d;tuner=%d,%d,%d,%d,%d,%c,dvbs,,,,%d,;pids=",
			t->diseqc + 1, aid, ad->strength, ad->status, ad->snr,
			t->freq / 1000, t->pol, t->sr / 1000);
	else
		sprintf (dad, "ver=1.1;src=%d;tuner=%d,%d,%d,%d,%d,,dvbt,,,,,;pids=",
			t->diseqc + 1, aid, ad->strength, ad->status, ad->snr, t->freq);

	for (i = 0; i < MAX_PIDS; i++)
		if (ad->pids[i].flags == 1)
	{
		x = 1;
		sprintf (dad + strlen (dad), "%d,", ad->pids[i].pid);
	}
	if (x)
								 // cut the last comma
			dad[strlen (dad) - 1] = 0;
	else
		dad[strlen (dad)] = '0';
	return dad;
}


// sorting the pid list in order to get faster the pids that are frequestly used
void
sort_pids (int aid)
{
	int b, i, j, t;
	pid pp;
	pid *p;

	p = a[aid].pids;
	b = 1;
	while (b)
	{
		b = 0;
		for (i = 0; i < MAX_PIDS - 1; i++)
			if (p[i].cnt < p[i + 1].cnt)
		{
			b = 1;
			memcpy (&pp, &p[i], sizeof (pp));
			memcpy (&p[i], &p[i + 1], sizeof (pp));
			memcpy (&p[i + 1], &pp, sizeof (pp));
		}
	}
}


void
free_all_adapters ()
{
	int i;

	for (i = 0; i < MAX_ADAPTERS; i++)
		if (a[i].buf)
			free1 (a[i].buf);

}
