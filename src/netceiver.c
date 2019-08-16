/*
 * Copyright (C) 2014-2020 Beat Zahnd <beat.zahnd@gmail.com>
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

#ifndef DISABLE_NETCVCLIENT

#define _GNU_SOURCE
#include <sys/types.h>
#include <ifaddrs.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "netceiver.h"

#define DEFAULT_LOG LOG_NETCEIVER

SNetceiver *sn[MAX_ADAPTERS];
#define SN sn[ad->id]

int handle_ts(unsigned char *buffer, size_t len, void *p);
int handle_ten(tra_t *ten, void *p);

extern char *fe_pilot[];
extern char *fe_rolloff[];
extern char *fe_delsys[];
extern char *fe_fec[];
extern char *fe_modulation[];
extern char *fe_tmode[];
extern char *fe_gi[];
extern char *fe_hierarchy[];
extern char *fe_specinv[];
extern char *fe_pol[];

int netcv_close(adapter *ad)
{
	SN->want_commit = 0;

	if (!SN->ncv_rec)
		LOG("netceiver: receiver instance is NULL (id=%d)", ad->id);

	LOG("netceiver: delete receiver instance for adapter %d", ad->id);

	/* unregister handlers */
	register_ten_handler(SN->ncv_rec, NULL, NULL);
	register_ts_handler(SN->ncv_rec, NULL, NULL);

	/* call recv_init of libmcli to clean up the NetCeiver API */
	recv_del(SN->ncv_rec);
	SN->ncv_rec = NULL;

	/* close DVR pipe */
	close(SN->pwfd);
	close(ad->dvr);

	ad->strength = 0;
	ad->status = 0;
	ad->snr = 0;
	ad->ber = 0;

	return 0;
}

int netcv_open_device(adapter *ad)
{
	SN->want_commit = 0;
	SN->ncv_rec = NULL;

	/* create DVR pipe for TS data transfer from libmcli to minisatip */
	int pipe_fd[2];
	if (pipe2(pipe_fd, O_NONBLOCK))
		LOG("netceiver: creating pipe failed (%s)", strerror(errno));
	if (-1 == fcntl(pipe_fd[0], F_SETPIPE_SZ, 5 * 188 * 1024))
		LOG("netceiver pipe buffer size change failed (%s)", strerror(errno));
	ad->dvr = pipe_fd[0];  // read end of pipe
	SN->pwfd = pipe_fd[1]; // write end of pipe
	LOG("netceiver: creating DVR pipe for adapter %d  -> dvr: %d", ad->id, ad->dvr);

	return 0;
}

int netcv_set_pid(adapter *ad, int pid)
{
	int aid = ad->id;
	LOG("netceiver: set_pid for adapter %d, pid %d, err %d", aid, pid,
		SN->err);

	if (SN->err) // error reported, return error
		return 0;

	SN->npid[SN->lp] = pid;
	SN->lp++;
	SN->want_commit = 1;

	return aid + 100;
}

int netcv_del_pid(adapter *ad, int fd, int pid)
{
	int i, hit = 0;
	fd -= 100;
	LOG("netceiver: del_pid for aid %d, pid %d, err %d", fd, pid, SN->err);
	if (SN->err) // error reported, return error
		return 0;

	for (i = 0; i < MAX_PIDS - 1; i++)
		if (SN->npid[i] == pid || hit)
		{
			SN->npid[i] = SN->npid[i + 1];
			hit = 1;
		}
	if (hit && sn[fd]->lp > 0)
		sn[fd]->lp--;
	SN->want_commit = 1;

	return 0;
}

void netcv_commit(adapter *ad)
{
	int i;

	int m_pos = 0;
	fe_type_t type = 0;
	recv_sec_t m_sec;
	struct dvb_frontend_parameters m_fep;
	dvb_pid_t m_pids[MAX_PIDS];

	/* check if we have to create a receiver instance first */
	SN->err = 0;
	if (!SN->ncv_rec)
	{
		LOG("netceiver: add a new receiver instance for adapter %d", ad->id);

		/* call recv_add of libmcli to add a new receiver instance */
		SN->ncv_rec = recv_add();

		/* register handlers */
		register_ten_handler(SN->ncv_rec, &handle_ten, ad);
		register_ts_handler(SN->ncv_rec, &handle_ts, SN);

		if (!SN->ncv_rec)
			SN->err = 1;

		SN->want_tune = 0; // wait until netcv_tune triggers the tuning
	}

	/* tune receiver to a new frequency / tranponder */
	if (SN->want_tune)
	{
		transponder *tp = &ad->tp;

		int map_pos[] =
			{0, 192, 130, 282, -50}; // Default sat positions: 19.2E, 13E, 28.2E, 5W
		int map_pol[] =
			{0, SEC_VOLTAGE_13, SEC_VOLTAGE_18, SEC_VOLTAGE_OFF};

		switch (tp->sys)
		{
		case SYS_DVBS:
		case SYS_DVBS2:
			m_pos = 1800 + map_pos[tp->diseqc];

			memset(&m_sec, 0, sizeof(recv_sec_t));
			m_sec.voltage = map_pol[tp->pol];

			memset(&m_fep, 0, sizeof(struct dvb_frontend_parameters));
			m_fep.frequency = tp->freq;
			m_fep.inversion = INVERSION_AUTO;
			m_fep.u.qpsk.symbol_rate = tp->sr;

			if (tp->sys == SYS_DVBS)
			{
				m_fep.u.qpsk.fec_inner = tp->fec;
				type = FE_QPSK;
			}
			else
			{
				switch (tp->fec) // Handle FEC numbering exceptions
				{
				case FEC_3_5:
					m_fep.u.qpsk.fec_inner = 13;
					break;

				case FEC_9_10:
					m_fep.u.qpsk.fec_inner = 14;
					break;

				default:
					m_fep.u.qpsk.fec_inner = tp->fec;
				}

				// Für DVB-S2 PSK8 oder QPSK, siehe vdr-mcli-plugin/device.c
				if (tp->mtype)
					m_fep.u.qpsk.fec_inner |= (PSK8 << 16);
				else
					m_fep.u.qpsk.fec_inner |= (QPSK_S2 << 16);
				type = FE_DVBS2;
			}

			char *map_posc[] =
				{"", " @ 19.2E", " @ 13E", " @ 28.2E", " @ 5W"};
			LOG("netceiver: adapter %d tuning to %d%s pol:%s sr:%d fec:%s delsys:%s mod:%s",
				ad->id, tp->freq / 1000, map_posc[tp->diseqc], get_pol(tp->pol), tp->sr / 1000,
				fe_fec[tp->fec], fe_delsys[tp->sys], fe_modulation[tp->mtype]);

			break;

			/* set roll-off */
			// TODO: check if needed for DVB-S2 transponders
			// unreachable code
			//			m_fep.u.qpsk.fec_inner |= (tp->ro << 24);
			//			break;

		case SYS_DVBC_ANNEX_A:
			m_pos = 0xfff; /* not sure, to be tested */
			memset(&m_sec, 0, sizeof(recv_sec_t));

			m_fep.frequency = tp->freq * 1000;
			m_fep.inversion = INVERSION_AUTO;
			m_fep.u.qam.fec_inner = FEC_NONE;
			/* we enforce qam256 if qpsk is used falsely, e.g. by Elgato SAT>IP app */
			m_fep.u.qam.modulation = tp->mtype == QPSK ? QAM_256 : tp->mtype;
			m_fep.u.qam.symbol_rate = tp->sr;

			type = FE_QAM;

			LOG("netceiver: adapter %d tuning to %d sr:%d delsys:%s mod:%s",
				ad->id, tp->freq / 1000, tp->sr / 1000,
				fe_delsys[tp->sys], fe_modulation[tp->mtype]);

			break;

		case SYS_DVBT:
			m_fep.frequency = tp->freq * 1000;
			m_fep.inversion = INVERSION_AUTO;

			switch (tp->bw)
			{
			case 8000000:
				m_fep.u.ofdm.bandwidth = BANDWIDTH_8_MHZ;
				break;
			case 7000000:
				m_fep.u.ofdm.bandwidth = BANDWIDTH_7_MHZ;
				break;
			case 6000000:
				m_fep.u.ofdm.bandwidth = BANDWIDTH_6_MHZ;
				break;
			case 0:
				m_fep.u.ofdm.bandwidth = BANDWIDTH_AUTO;
				break;
			case 5000000:
				m_fep.u.ofdm.bandwidth = BANDWIDTH_5_MHZ;
				break;
			case 10000000:
				m_fep.u.ofdm.bandwidth = BANDWIDTH_10_MHZ;
				break;
			case 1712000:
				m_fep.u.ofdm.bandwidth = BANDWIDTH_1_712_MHZ;
				break;
			default:
				m_fep.u.ofdm.bandwidth = BANDWIDTH_AUTO;
				LOG("netceiver: DVB-T: unknown bandwith: %d", tp->bw);
			}

			m_fep.u.ofdm.code_rate_HP = FEC_AUTO; // TBC
			m_fep.u.ofdm.code_rate_LP = FEC_AUTO; // TBC

			switch (tp->mtype) // not properly handled by vdr-satip-plugin ?
			{
			case 0:
				m_fep.u.ofdm.constellation = QAM_AUTO;
				break;
			case 6:
				m_fep.u.ofdm.constellation = QAM_32;
				break;
			default:
				m_fep.u.ofdm.constellation = tp->mtype;
			}

			m_fep.u.ofdm.transmission_mode = tp->tmode;
			m_fep.u.ofdm.guard_interval = tp->gi;
			m_fep.u.ofdm.hierarchy_information = HIERARCHY_NONE;

			type = FE_OFDM;

			LOG("netceiver: adapter %d tuning to %d inv: %d mtype: %d "
				"hprate: %d tmode: %d gi: %d bw:%d sm %d t2id %d",
				ad->id, tp->freq / 1000, tp->inversion, tp->mtype,
				tp->hprate, tp->tmode, tp->gi, tp->bw, tp->sm, tp->t2id);

			break;
		}

		memset(m_pids, 0, sizeof(m_pids));
		m_pids[0].pid = -1;

		/* call recv_tune of libmcli */
		if (recv_tune(SN->ncv_rec, type, m_pos, &m_sec, &m_fep, m_pids) != 0)
			LOG("netceiver: Tuning receiver failed");

		ad->strength = 0;
		ad->status = 0;
		ad->snr = 0;
		ad->ber = 0;

		SN->want_tune = 0;
	}

	/* activate or deactivate PIDs */
	if (SN->want_commit)
	{
		if (SN->lp)
		{
			memset(m_pids, 0, sizeof(m_pids));
			for (i = 0; i < SN->lp; i++)
			{
				m_pids[i].pid = SN->npid[i];
				m_pids[i].id = 0; // here we maybe have to set the SID if this PID is encrypted
			}

			m_pids[i].pid = -1;
			/* call recv_pids of libmcli to set the active PIDs */
			usleep(10000);
			if (recv_pids(SN->ncv_rec, m_pids))
				LOG("netceiver: seting PIDs failed");
		}
		else
		{
			/* call recv_stop of libmcli to deactivate all PIDs */
			if (recv_stop(SN->ncv_rec))
				LOG("netceiver: removing all PIDs failed");
		}

		SN->want_commit = 0;
	}

	return;
}

int netcv_tune(int aid, transponder *tp)
{
	adapter *ad = get_adapter(aid);
	if (!ad)
		return 1;

	SN->want_tune = 1; // we do not tune right now, just set the flag for netcv_commit
	SN->want_commit = 0;
	SN->lp = 0; // make sure number of active pids is 0 after tuning
	return 0;
}

fe_delivery_system_t netcv_delsys(int aid, int fd, fe_delivery_system_t *sys)
{
	return 0;
}

void find_netcv_adapter(adapter **a)
{
	int i, k, n, na;
	netceiver_info_list_t *nc_list;
	adapter *ad;
	char dbuf[2048];

	/* check if network interface is available */
	struct ifaddrs *nif, *nif1;

	if (!opts.netcv_if)
		return;

	getifaddrs(&nif);

	nif1 = nif;
	while (strcmp(nif1->ifa_name, opts.netcv_if) || nif1->ifa_addr->sa_family != AF_INET6)
	{
		if (nif1->ifa_next == NULL)
		{
			nif1 = NULL;
			break;
		}
		nif1 = nif1->ifa_next;
	}

#define IFF_NETCV (IFF_UP | IFF_RUNNING | IFF_MULTICAST)
	if (nif1 == NULL || (nif1->ifa_flags & IFF_NETCV) != IFF_NETCV)
	{
		LOG("network interface %s not available", opts.netcv_if);
		exit(0);
	}
	freeifaddrs(nif);

	/* call recv_init of libmcli to initialize the NetCeiver API */
	if (recv_init(opts.netcv_if, 23000))
		LOG("Netceiver init failed");

	/* Call api_sock_init to initialize the socket needed for netceiver tools */
	if (api_sock_init(API_SOCK_NAMESPACE))
		LOG("Netceiver API socket init failed");

	sprintf(dbuf, "REEL: Search for %d Netceiver%s on %s... ",
			opts.netcv_count, opts.netcv_count == 1 ? "" : "s", opts.netcv_if);
	n = 0;
	do
	{
		usleep(250000);
		sprintf(dbuf + strlen(dbuf), "##");
		nc_list = nc_get_list();
	} while (nc_list->nci_num < opts.netcv_count && n++ < 19);
	nc_lock_list();
	sprintf(dbuf + strlen(dbuf), "\n");

	/* count available tuner types */
	int nc_sys_c[] =
		{0, 0, 0, 0, 0};
	int map_type[] =
		{FE_DVBS2, FE_QPSK, FE_QAM, FE_OFDM, FE_ATSC};
	int nc_sys_t = 0;
	for (n = 0; n < nc_list->nci_num; n++)
	{
		netceiver_info_t *nci = nc_list->nci + n;
		sprintf(dbuf + strlen(dbuf), "Found NetCeiver: %s \n", nci->uuid);
		for (i = 0; i < nci->tuner_num; i++)
		{
			sprintf(dbuf + strlen(dbuf), "	Tuner: %s, Type %d\n", nci->tuner[i].fe_info.name,
					nci->tuner[i].fe_info.type);
			nc_sys_c[nci->tuner[i].fe_info.type]++;
			nc_sys_t++;
		}
	}
	LOG("%s", dbuf);

	// add netceiver tuners to the list of adapters
	i = FE_QPSK;
	na = a_count;
	sprintf(dbuf, "netceiver: adding ");
	for (n = 0; n < nc_sys_t; n++)
	{
		while (i < FE_DVBS2 && nc_sys_c[map_type[i]] == 0)
			i++;
		nc_sys_c[map_type[i]]--;

		if (is_adapter_disabled(na))
		{
			na++;
			continue;
		}

		if (na >= MAX_ADAPTERS)
			break;
		if (!a[na])
			a[na] = adapter_alloc();
		if (!sn[na])
			sn[na] = malloc1(sizeof(SNetceiver));

		ad = a[na];
		ad->pa = 0;
		ad->fn = 0;
		sn[na]->want_tune = 0;
		sn[na]->want_commit = 0;
		sn[na]->ncv_rec = NULL;

		/* initialize signal status info */
		ad->strength = 0;
		ad->status = 0;
		ad->snr = 0;
		ad->ber = 0;

		/* register callback functions in adapter structure */
		ad->open = (Open_device)netcv_open_device;
		ad->set_pid = (Set_pid)netcv_set_pid;
		ad->del_filters = (Del_filters)netcv_del_pid;
		ad->commit = (Adapter_commit)netcv_commit;
		ad->tune = (Tune)netcv_tune;
		ad->delsys = (Dvb_delsys)netcv_delsys;
		ad->post_init = (Adapter_commit)NULL;
		ad->close = (Adapter_commit)netcv_close;
		ad->type = ADAPTER_NETCV;

		/* register delivery system type */
		for (k = 0; k < 10; k++)
			ad->sys[k] = 0;
		switch (map_type[i])
		{
		case FE_DVBS2:
			ad->sys[0] = SYS_DVBS2;
			ad->sys[1] = SYS_DVBS;
			sprintf(dbuf + strlen(dbuf), "[AD%d DVB-S2] ", na);
			break;

		case FE_QPSK:
			ad->sys[0] = SYS_DVBS;
			sprintf(dbuf + strlen(dbuf), "[AD%d DVB-S] ", na);
			break;

		case FE_QAM:
			ad->sys[0] = SYS_DVBC_ANNEX_A;
			sprintf(dbuf + strlen(dbuf), "[AD%d DVB-C] ", na);
			break;

		case FE_OFDM:
			sprintf(dbuf + strlen(dbuf), "[AD%d DVB-T] ", na);
			ad->sys[0] = SYS_DVBT;
			break;
		}

		na++; // increase number of tuner count
		a_count = na;
	}
	LOG("%s", dbuf);
	nc_unlock_list(); // netceivers appearing after this will be recognized by libmcli but will not made available to minisatip

	for (; na < MAX_ADAPTERS; na++)
		if (a[na])
			a[na]->pa = a[na]->fn = -1;
}

/*
 * Handle TS data
 * This function is called by libmcli each time a IP packet with TS packets arrives.
 * We write the data to the write end of a pipe
 *
 */

int handle_ts(unsigned char *buffer, size_t len, void *p)
{
	SNetceiver *nc = p;
	size_t lw;

	if (nc->lp == 0)
		return len;

	/* simple data format check */
	if (buffer[0] != 0x47 || len % 188 != 0)
	{
		LOG("netceiver: TS data mallformed: buf[0]=0x%02x len=%d",
			buffer[0], len);
		return len;
	}

	/* write TS data to DVR pipe */
	lw = write(nc->pwfd, buffer, len);
	if (lw != len)
		LOG("netceiver: not all data forwarded (%s)", strerror(errno));

	return len;
}

/* Handle signal status information */
int handle_ten(tra_t *ten, void *p)
{
	adapter *ad = p;
	recv_festatus_t *festat;

	if (ten)
	{
		festat = &ten->s;
		ad->strength = (festat->strength & 0xffff) >> 8;
		ad->status = festat->st == 0x1f ? FE_HAS_LOCK : 0;
		ad->snr = (festat->snr & 0xffff) >> 8;
		ad->ber = festat->ber;

		return 0;
	}

	return 0;
}
#endif
