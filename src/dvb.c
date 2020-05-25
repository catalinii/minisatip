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
#include <net/if.h>
#include <fcntl.h>
#include <ctype.h>
#include "dvb.h"
#include "minisatip.h"
#include "adapter.h"
#include "ca.h"
#include "utils.h"

#define DEFAULT_LOG LOG_DVB

#define DEV_FRONTEND "/dev/dvb/adapter%d/frontend%d"
#define DEV_DEMUX "/dev/dvb/adapter%d/demux%d"
#define DEV_DVR "/dev/dvb/adapter%d/dvr%d"

#ifdef ENIGMA
#ifndef DMX_SET_SOURCE
/**
 * DMX_SET_SOURCE and dmx_source enum removed on 4.14 kernel
 * From Github.com/OpenPLI/enigma2 commit 7996dbb
**/
enum dmx_source
{
	DMX_SOURCE_FRONT0 = 0,
	DMX_SOURCE_FRONT1,
	DMX_SOURCE_FRONT2,
	DMX_SOURCE_FRONT3,
	DMX_SOURCE_DVR0 = 16,
	DMX_SOURCE_DVR1,
	DMX_SOURCE_DVR2,
	DMX_SOURCE_DVR3
};
#define DMX_SET_SOURCE _IOW('o', 49, enum dmx_source)
#endif
#endif

char *fe_pilot[] =
	{"on", "off", " ", //auto
	 NULL};

char *fe_rolloff[] =
	{"0.35", "0.20", "0.25", " ", //auto
	 "0.15", "0.10", "0.05", NULL};

char *fe_delsys[] =
	{"undefined", "dvbc", "dvbcb", "dvbt", "dss", "dvbs", "dvbs2", "dvbh", "isdbt",
	 "isdbs", "isdbc", "atsc", "atscmh", "dmbth", "cmmb", "dab", "dvbt2",
	 "turbo", "dvbcc", "dvbc2",
	 NULL};

char *fe_fec[] =
	{"none", "12", "23", "34", "45", "56", "67", "78", "89", " ", //auto
	 "35", "910", "25", "14", "13",
	 NULL};

char *fe_modulation[] =
	{"qpsk", "16qam", "32qam", "64qam", "128qam", "256qam", " ", // auto
	 "8vsb", "16vsb", "8psk", "16apsk", "32apsk", "dqpsk", "qam_4_nr", "64apsk", "128apsk", "256apsk",
	 NULL};

char *fe_tmode[] =
	{"2k", "8k", " ", //auto
	 "4k", "1k", "16k", "32k", "c1", "c3780",
	 NULL};

char *fe_gi[] =
	{"132", "116", "18", "14", " ", // auto
	 "1128", "19128", "19256", "pn420", "pn595", "pn945",
	 NULL};

char *fe_hierarchy[] =
	{"HIERARCHY_NONE", "HIERARCHY_1", "HIERARCHY_2", "HIERARCHY_4",
	 "HIERARCHY_AUTO",
	 NULL};

char *fe_specinv[] =
	{"off", "on", " ", // auto
	 NULL};

char *fe_inversion[] =
	{"0", "1", " ", // auto
	 NULL};

char *fe_pol[] =
	{"none", "v", "h", "r", "l",
	 NULL};

char *fe_pls_mode[] =
	{"root", "gold", "combo",
	 NULL};

#define make_func(a)                                               \
	char *get_##a(int i)                                           \
	{                                                              \
		if (i >= 0 && i < (int)sizeof(fe_##a) / sizeof(fe_##a[0])) \
		{                                                          \
			if (fe_##a[i][0] == 32)                                \
				return "";                                         \
			else                                                   \
				return fe_##a[i];                                  \
		}                                                          \
		return "NONE";                                             \
	}
make_func(pilot);
make_func(rolloff);
make_func(delsys);
make_func(fec);
make_func(modulation);
make_func(tmode);
make_func(gi);
make_func(specinv);
make_func(inversion);
make_func(pol);
make_func(pls_mode);

#define INVALID_URL(a) \
	{                  \
		LOG(a);        \
		return 0;      \
	}
char def_pids[100];

int detect_dvb_parameters(char *s, transponder *tp)
{
	char *arg[30];
	int la, i;

	tp->sys = -1;
	tp->freq = -1;
	tp->inversion = -1;
	tp->hprate = -1;
	tp->tmode = -1;
	tp->gi = -1;
	tp->bw = -1;
	tp->sm = -1;
	tp->t2id = -1;
	tp->fe = -1;
	tp->ro = -1;
	tp->mtype = -1;
	tp->plts = -1;
	tp->fec = -1;
	tp->sr = -1;
	tp->pol = -1;
	tp->diseqc = -1;
	tp->c2tft = -1;
	tp->ds = -1;
	tp->plp_isi = TP_VALUE_UNSET;
	tp->pls_mode = -1;
	tp->pls_code = -1;

	tp->pids = tp->apids = tp->dpids = tp->x_pmt = NULL;

	while (*s > 0 && *s != '?')
		s++;

	if (*s == 0)
		LOG_AND_RETURN(0, "no ? found in URL");

	s++;
	if (strstr(s, "freq="))
		init_dvb_parameters(tp);

	LOG("detect_dvb_parameters (S)-> %s", s);
	la = split(arg, s, ARRAY_SIZE(arg), '&');

	for (i = 0; i < la; i++)
	{
		if (strncmp("msys=", arg[i], 5) == 0)
			tp->sys = map_int(arg[i] + 5, fe_delsys);
		if (strncmp("freq=", arg[i], 5) == 0)
			tp->freq = map_float(arg[i] + 5, 1000);
		if (strncmp("pol=", arg[i], 4) == 0)
			tp->pol = map_int(arg[i] + 4, fe_pol);
		if (strncmp("sr=", arg[i], 3) == 0)
			tp->sr = map_int(arg[i] + 3, NULL) * 1000;
		if (strncmp("fe=", arg[i], 3) == 0)
			tp->fe = map_int(arg[i] + 3, NULL);
		if (strncmp("src=", arg[i], 4) == 0)
			tp->diseqc = map_int(arg[i] + 4, NULL);
		if (strncmp("ro=", arg[i], 3) == 0)
			tp->ro = map_int(arg[i] + 3, fe_rolloff);
		if (strncmp("mtype=", arg[i], 6) == 0)
			tp->mtype = map_int(arg[i] + 6, fe_modulation);
		if (strncmp("fec=", arg[i], 4) == 0)
			tp->fec = map_int(arg[i] + 4, fe_fec);
		if (strncmp("plts=", arg[i], 5) == 0)
			tp->plts = map_int(arg[i] + 5, fe_pilot);
		if (strncmp("gi=", arg[i], 3) == 0)
			tp->gi = map_int(arg[i] + 3, fe_gi);
		if (strncmp("tmode=", arg[i], 6) == 0)
			tp->tmode = map_int(arg[i] + 6, fe_tmode);
		if (strncmp("bw=", arg[i], 3) == 0)
			tp->bw = map_float(arg[i] + 3, 1000000);
		if (strncmp("specinv=", arg[i], 8) == 0)
			tp->inversion = map_int(arg[i] + 8, NULL);
		if (strncmp("c2tft=", arg[i], 6) == 0)
			tp->c2tft = map_int(arg[i] + 6, NULL);
		if (strncmp("ds=", arg[i], 3) == 0)
			tp->ds = map_int(arg[i] + 3, NULL);
		if (strncmp("plp=", arg[i], 4) == 0 ||
			strncmp("isi=", arg[i], 4) == 0)
			tp->plp_isi = map_int(arg[i] + 4, NULL);
		if (strncmp("plsm=", arg[i], 5) == 0)
			tp->pls_mode = map_int(arg[i] + 5, fe_pls_mode);
		if (strncmp("plsc=", arg[i], 5) == 0)
			tp->pls_code = map_int(arg[i] + 5, NULL);

		if (strncmp("x_pmt=", arg[i], 6) == 0)
			tp->x_pmt = arg[i] + 6;
		if (strncmp("pids=", arg[i], 5) == 0)
			tp->pids = arg[i] + 5;
		if (strncmp("addpids=", arg[i], 8) == 0)
			tp->apids = arg[i] + 8;
		if (strncmp("delpids=", arg[i], 8) == 0)
			tp->dpids = arg[i] + 8;
	}

	if (tp->pids && strstr(tp->pids, "all"))
	{
		strcpy(def_pids, "8192");
		// map pids=all to essential pids
		tp->pids = (char *)def_pids;
	}

	if (tp->pids && strncmp(tp->pids, "none", 4) == 0)
		tp->pids = "";

	//      if(!msys)INVALID_URL("no msys= found in URL");
	//      if(freq<10)INVALID_URL("no freq= found in URL or frequency invalid");
	//      if((msys==SYS_DVBS || msys==SYS_DVBS2) && (pol!='H' && pol!='V'))INVALID_URL("no pol= found in URL or pol is not H or V");
	LOG(
		"detect_dvb_parameters (E) -> src=%d, fe=%d, freq=%d, fec=%d, sr=%d, pol=%d, ro=%d, msys=%d, mtype=%d, plts=%d, bw=%d, inv=%d, pids=%s - apids=%s - dpids=%s x_pmt=%s",
		tp->diseqc, tp->fe, tp->freq, tp->fec, tp->sr, tp->pol, tp->ro,
		tp->sys, tp->mtype, tp->plts, tp->bw, tp->inversion,
		tp->pids ? tp->pids : "NULL", tp->apids ? tp->apids : "NULL",
		tp->dpids ? tp->dpids : "NULL", tp->x_pmt ? tp->x_pmt : "NULL");
	return 0;
}

void init_dvb_parameters(transponder *tp)
{
	memset(tp, 0, sizeof(transponder));
	tp->inversion = INVERSION_AUTO;
	tp->hprate = FEC_AUTO;
	tp->tmode = TRANSMISSION_MODE_AUTO;
	tp->gi = GUARD_INTERVAL_AUTO;
	tp->bw = 8000000;
	tp->ro = ROLLOFF_AUTO;
	tp->mtype = QAM_AUTO;
	tp->plts = PILOT_AUTO;
	tp->fec = FEC_AUTO;
	tp->ds = TP_VALUE_UNSET;
	tp->plp_isi = TP_VALUE_UNSET;
	tp->pls_mode = TP_VALUE_UNSET;
	tp->pls_code = TP_VALUE_UNSET;
}

void copy_dvb_parameters(transponder *s, transponder *d)
{
	LOG(
		"copy_dvb_param start -> src=%d, fe=%d, freq=%d, fec=%d, sr=%d, pol=%d, ro=%d, msys=%d, mtype=%d, plts=%d, bw=%d, inv=%d, pids=%s, apids=%s, dpids=%s x_pmt=%s",
		d->diseqc, d->fe, d->freq, d->fec, d->sr, d->pol, d->ro, d->sys,
		d->mtype, d->plts, d->bw, d->inversion, d->pids ? d->pids : "NULL",
		d->apids ? d->apids : "NULL", d->dpids ? d->dpids : "NULL",
		d->x_pmt ? d->x_pmt : "NULL");
	if (s->sys != -1)
		d->sys = s->sys;
	if (s->freq != -1)
		d->freq = s->freq;
	if (s->inversion != -1)
		d->inversion = s->inversion;
	if (s->hprate != -1)
		d->hprate = s->hprate;
	if (s->tmode != -1)
		d->tmode = s->tmode;
	if (s->gi != -1)
		d->gi = s->gi;
	if (s->bw != -1)
		d->bw = s->bw;
	if (s->sm != -1)
		d->sm = s->sm;
	if (s->t2id != -1)
		d->t2id = s->t2id;
	if (s->fe != -1)
		d->fe = s->fe;
	if (s->ro != -1)
		d->ro = s->ro;
	if (s->mtype != -1)
		d->mtype = s->mtype;
	if (s->plts != -1)
		d->plts = s->plts;
	if (s->fec != -1)
		d->fec = s->fec;
	if (s->sr != -1)
		d->sr = s->sr;
	if (s->pol != -1)
		d->pol = s->pol;
	if (s->diseqc != -1)
		d->diseqc = s->diseqc;
	if (s->c2tft != -1)
		d->c2tft = s->c2tft;
	if (s->ds != TP_VALUE_UNSET)
		d->ds = s->ds;
	if (s->plp_isi != TP_VALUE_UNSET)
		d->plp_isi = s->plp_isi;
	if (s->pls_mode != TP_VALUE_UNSET)
		d->pls_mode = s->pls_mode;
	if (s->pls_code != -1)
		d->pls_code = s->pls_code;

	d->x_pmt = s->x_pmt;
	d->apids = s->apids;
	d->pids = s->pids;
	d->dpids = s->dpids;

	if (d->diseqc < 1) // force position 1 on the diseqc switch
		d->diseqc = 1;

	if ((d->sys == SYS_DVBS2) && (d->mtype == -1))
		d->mtype = PSK_8;

	if ((d->sys == SYS_DVBS) && (d->mtype == -1))
		d->mtype = QPSK;

	LOG(
		"copy_dvb_parameters -> src=%d, fe=%d, freq=%d, fec=%d sr=%d, pol=%d, ro=%d, msys=%d, mtype=%d, plts=%d, bw=%d, inv=%d, pids=%s, apids=%s, dpids=%s x_pmt=%s",
		d->diseqc, d->fe, d->freq, d->fec, d->sr, d->pol, d->ro, d->sys,
		d->mtype, d->plts, d->bw, d->inversion, d->pids ? d->pids : "NULL",
		d->apids ? d->apids : "NULL", d->dpids ? d->dpids : "NULL",
		d->x_pmt ? d->x_pmt : "NULL");
}

#ifndef DISABLE_LINUXDVB

struct diseqc_cmd
{
	struct dvb_diseqc_master_cmd cmd;
	uint32_t wait;
};

// based on enigma2 fbc.cpp, setProcData
void set_proc_data(int adapter, char *name, int val)
{
	char filename[100];
	sprintf(filename, "/proc/stb/frontend/%d/%s", adapter, name);
	LOG("setProcData %s -> %d", filename, val);
	FILE *fp = fopen(filename, "w");
	if (fp)
	{
		fprintf(fp, "%d", val);
		fclose(fp);
	}
	else
	{
		LOG("setProcData open failed, %s: %m", filename);
	}
}

void dvb_set_demux_source(adapter *ad)
{
#ifdef DMX_SET_SOURCE
	if (ad->dmx_source >= 0)
	{
		char buf[255];
		sprintf(buf, DEV_DEMUX, ad->pa, ad->fn);
		int dmx = open(buf, O_RDWR | O_NONBLOCK);
		if (dmx <= 0)
			LOG("DMX_SET_SOURCE: Failed opening %s", buf)
		else if (ioctl(dmx, DMX_SET_SOURCE, &ad->dmx_source))
		{
			LOG("DMX_SET_SOURCE failed for adapter %d - %d: %s", ad->id, errno,
				strerror(errno));
		}
		else
			LOG("Set DMX_SET_SOURCE for adapter %d to %d", ad->id,
				ad->dmx_source);
		if (dmx >= 0)
			close(dmx);
	}
#endif
}

int dvb_open_device(adapter *ad)
{
	char buf[100];
	int use_demux = opts.use_demux_device == USE_DEMUX || opts.use_demux_device == USE_PES_FILTERS_AND_DEMUX;
	LOG("trying to open [%d] adapter %d and frontend %d", ad->id, ad->pa,
		ad->fn);
	sprintf(buf, DEV_FRONTEND, ad->pa, ad->fn);
	ad->fe = open(buf, O_RDWR | O_NONBLOCK);
	if (use_demux)
		sprintf(buf, DEV_DEMUX, ad->pa, ad->fn);
	else
		sprintf(buf, DEV_DVR, ad->pa, ad->fn);

	ad->dvr = open(buf, use_demux ? O_RDWR | O_NONBLOCK : O_RDONLY | O_NONBLOCK);
	if (ad->fe < 0 || ad->dvr < 0)
	{
		sprintf(buf, DEV_FRONTEND, ad->pa, ad->fn);
		LOG("Could not open %s in RW mode (fe: %d, dvr: %d) error %d: %s", buf, ad->fe,
			ad->dvr, errno, strerror(errno));
		if (ad->fe >= 0)
			close(ad->fe);
		if (ad->dvr >= 0)
			close(ad->dvr);
		ad->fe = ad->dvr = -1;
		return 1;
	}
	ad->type = ADAPTER_DVB;
	ad->dmx = -1;
	LOG("opened DVB adapter %d fe:%d dvr:%d", ad->id, ad->fe, ad->dvr);
	if (ioctl(ad->dvr, DMX_SET_BUFFER_SIZE, opts.dvr_buffer) < 0)
		LOG("couldn't set DVR buffer size error %d: %s", errno, strerror(errno))
	else
		LOG("DVR buffer set to %d bytes", opts.dvr_buffer);

	if (ad->is_fbc)
	{
		if (ad->master_source >= 0)
		{
			set_proc_data(ad->fn, "fbc_link", 1);
			set_proc_data(ad->fn, "fbc_connect", ad->master_source);
		}
		else
		{
			if (ad->fn > 1)
				LOG("FBC master adapter is recommended to be adapter 0 or 1, depending on the number of physical inputs");
			set_proc_data(ad->fn, "fbc_link", 0);
			set_proc_data(ad->fn, "fbc_connect", ad->fn);
		}
	}
	dvb_set_demux_source(ad);

	return 0;
}

void msleep(long ms)
{
	if (ms > 0)
		usleep(ms * 1000);
}

void diseqc_cmd(int fd, int times, char *str, struct dvb_diseqc_master_cmd *cmd,
				diseqc *d)
{
	int i;
	msleep(d->before_cmd);
	for (i = 0; i < times; i++)
	{
		if (ioctl(fd, FE_DISEQC_SEND_MASTER_CMD, cmd) == -1)
			LOG(
				"send_diseqc: FE_DISEQC_SEND_MASTER_CMD %s failed for fd %d: %s",
				str, fd, strerror(errno));
		msleep(i > 0 ? d->after_repeated_cmd : d->after_cmd);
	}
}

int send_diseqc(adapter *ad, int fd, int pos, int pos_change, int pol, int hiband, diseqc *d)
{
	int committed_no = d->committed_no;
	int uncommitted_no = d->uncommitted_no;
	int uncommitted_first = 0;
	int posu, posc;
	/* DiSEqC 1.0 */
	struct dvb_diseqc_master_cmd cmd =
		{
			{0xe0, 0x10, 0x38, 0xf0, 0x00, 0x00}, 4};
	/* DiSEqC 1.1 */
	struct dvb_diseqc_master_cmd uncmd =
		{
			{0xe0, 0x10, 0x39, 0xf0, 0x00, 0x00}, 4};

	if (pos_change && ad->diseqc_multi >= 0 && pos != ad->diseqc_multi)
	{
		send_diseqc(ad, fd, ad->diseqc_multi, 1, pol, hiband, d);
		pos_change = 1;
	}

	if (uncommitted_no > committed_no)
		uncommitted_first = 1;

	if (committed_no == 0 && uncommitted_no == 0)
		committed_no = 1;
	posu = posc = pos;

	if (uncommitted_no > 0 && committed_no > 0) // Diseqc 1.0 and 1.1 enabled.
	{
		posc = pos % 4;
		posu = pos / 4;
	}

	cmd.msg[1] = d->addr;
	cmd.msg[3] = 0xf0 | (((posc << 2) & 0x0c) | (hiband ? 1 : 0) | (pol ? 2 : 0));
	uncmd.msg[1] = d->addr;
	uncmd.msg[3] = 0xf0 | (posu & 0x0f);

	LOGM("send_diseqc fd %d, pos = %d (c %d u %d), pol = %d, hiband = %d",
		 fd, pos, posc, posu, pol, hiband);
	if (ad->wakeup)
		ad->wakeup(ad, fd, pol ? SEC_VOLTAGE_18 : SEC_VOLTAGE_13);

	if (ioctl(fd, FE_SET_TONE, SEC_TONE_OFF) == -1)
		LOG("send_diseqc: FE_SET_TONE failed for fd %d: %s", fd,
			strerror(errno));
	if (ioctl(fd, FE_SET_VOLTAGE, pol ? SEC_VOLTAGE_18 : SEC_VOLTAGE_13) == -1)
		LOG("send_diseqc: FE_SET_VOLTAGE failed for fd %d: %s", fd,
			strerror(errno));

	if (!d->fast || pos_change)
	{

		if (uncommitted_first)
			diseqc_cmd(fd, uncommitted_no, "uncommitted", &uncmd, d);

		diseqc_cmd(fd, committed_no, "committed", &cmd, d);

		if (!uncommitted_first)
			diseqc_cmd(fd, uncommitted_no, "uncommitted", &uncmd, d);

		msleep(d->after_switch);

		if (ioctl(fd, FE_DISEQC_SEND_BURST, (pos & 1) ? SEC_MINI_B : SEC_MINI_A) == -1)
			LOG("send_diseqc: FE_DISEQC_SEND_BURST failed for fd %d: %s", fd,
				strerror(errno));
	}

	msleep(d->after_burst);

	if (ioctl(fd, FE_SET_TONE, hiband ? SEC_TONE_ON : SEC_TONE_OFF) == -1)
		LOG("send_diseqc: FE_SET_TONE failed for fd %d: %s", fd,
			strerror(errno));

	msleep(d->after_tone);

	return 0;
}

int send_unicable(adapter *ad, int fd, int freq, int pos, int pol, int hiband, diseqc *d)
{
	struct dvb_diseqc_master_cmd cmd =
		{
			{0xe0, 0x10, 0x5a, 0x00, 0x00}, 5};
	int t;
	int committed_no = d->committed_no;

	t = (freq + d->ufreq + 2) / 4 - 350;

	cmd.msg[1] = d->addr;
	cmd.msg[3] = ((t & 0x0300) >> 8) | (d->uslot << 5) | (pos ? 0x10 : 0) | (hiband ? 4 : 0) | (pol ? 8 : 0);
	cmd.msg[4] = t & 0xff;

	if (d->pin > 0 && d->pin < 256)
	{
		cmd.msg_len = 6;
		cmd.msg[2] = 0x5C;
		cmd.msg[5] = d->pin;
	}

	LOGM(
		"send_unicable fd %d, freq %d, ufreq %d, pos = %d, pol = %d, hiband = %d, slot %d, pin %d, diseqc => %02x %02x %02x %02x %02x",
		fd, freq, d->ufreq, pos, pol, hiband, d->uslot, d->pin, cmd.msg[0],
		cmd.msg[1], cmd.msg[2], cmd.msg[3], cmd.msg[4]);
	if (ad->wakeup)
		ad->wakeup(ad, fd, SEC_VOLTAGE_13);
	if (!ad->tune_time)
	{
		if (ioctl(fd, FE_SET_VOLTAGE, SEC_VOLTAGE_13) == -1)
			LOG("send_unicable: pre voltage  SEC_VOLTAGE_13 failed for fd %d: %s",
				fd, strerror(errno));
		if (ioctl(fd, FE_SET_TONE, SEC_TONE_OFF) == -1)
			LOG("send_unicable: FE_SET_TONE failed for fd %d: %s", fd,
				strerror(errno));
		msleep(d->after_tone);
	}
	if (!d->only13v && ioctl(fd, FE_SET_VOLTAGE, SEC_VOLTAGE_18) == -1)
		LOG("send_unicable: FE_SET_VOLTAGE failed for fd %d: %s", fd,
			strerror(errno));
	diseqc_cmd(fd, committed_no, "unicable", &cmd, d);
	if (ioctl(fd, FE_SET_VOLTAGE, SEC_VOLTAGE_13) == -1)
		LOG("send_unicable: FE_SET_VOLTAGE failed for fd %d: %s", fd,
			strerror(errno));

	return d->ufreq * 1000;
}

int send_jess(adapter *ad, int fd, int freq, int pos, int pol, int hiband, diseqc *d)
{
	struct dvb_diseqc_master_cmd cmd =
		{
			{0x70, 0x00, 0x00, 0x00, 0x00}, 4};

	int committed_no = d->committed_no;

	//	int t = (freq / 1000) - 100;
	int t = freq - 100;

	cmd.msg[1] = d->uslot << 3;
	cmd.msg[1] |= ((t >> 8) & 0x07);
	cmd.msg[2] = (t & 0xff);
	cmd.msg[3] = ((pos & 0x3f) << 2) | (pol ? 2 : 0) | (hiband ? 1 : 0);
	if (d->pin > 0 && d->pin < 256)
	{
		cmd.msg_len = 5;
		cmd.msg[0] = 0x71;
		cmd.msg[4] = d->pin;
	}

	LOGM("send_jess fd %d, freq %d, ufreq %d, pos = %d, pol = %d, hiband = %d, slot %d, pin %d, diseqc => %02x %02x %02x %02x %02x",
		 fd, freq, d->ufreq, pos, pol, hiband, d->uslot, d->pin, cmd.msg[0],
		 cmd.msg[1], cmd.msg[2], cmd.msg[3], cmd.msg[4]);

	if (ad->wakeup)
		ad->wakeup(ad, fd, SEC_VOLTAGE_13);

	if (!ad->tune_time)
	{
		if (ioctl(fd, FE_SET_VOLTAGE, SEC_VOLTAGE_13) == -1)
			LOG("send_jess: pre voltage  SEC_VOLTAGE_13 failed for fd %d: %s", fd,
				strerror(errno));
		if (ioctl(fd, FE_SET_TONE, SEC_TONE_OFF) == -1)
			LOG("send_jess: FE_SET_TONE failed for fd %d: %s", fd, strerror(errno));
		msleep(d->after_tone);
	}

	if (!d->only13v && ioctl(fd, FE_SET_VOLTAGE, SEC_VOLTAGE_18) == -1)
		LOG("send_jess: FE_SET_VOLTAGE failed for fd %d: %s", fd,
			strerror(errno));
	diseqc_cmd(fd, committed_no, "jess", &cmd, d);

	if (ioctl(fd, FE_SET_VOLTAGE, SEC_VOLTAGE_13) == -1)
		LOG("send_jess: FE_SET_VOLTAGE failed for fd %d: %s", fd,
			strerror(errno));

	return d->ufreq * 1000;
}

int setup_switch(adapter *ad)
{
	int frontend_fd = -1;
	transponder *tp = &ad->tp;
	int hiband = 0;
	int diseqc = (tp->diseqc > 0) ? tp->diseqc - 1 : 0;
	int freq = tp->freq;
	int pol = (tp->pol - 1) & 1;
	adapter *master = ad;

	if (ad->master_source >= 0)
	{
		master = get_adapter(ad->master_source);
		if (!master)
		{
			init_hw(ad->master_source);
			master = get_adapter(ad->master_source);
		}
		if (!master)
		{
			LOG("Adapter %d master source %d for specified but adapter not enabled", ad->id, ad->master_source);
			master = ad;
		}
	}
	// master should be set to the master adapter or to the current adapter if there is no master

	frontend_fd = master->fe;

	if (tp->pol > 2 && tp->diseqc_param.lnb_circular > 0)
	{
		freq = (freq - tp->diseqc_param.lnb_circular);
		hiband = 0;
	}
	else if (freq < tp->diseqc_param.lnb_switch)
	{
		freq = (freq - tp->diseqc_param.lnb_low);
		hiband = 0;
	}
	else
	{
		freq = (freq - tp->diseqc_param.lnb_high);
		hiband = 1;
	}

	if (tp->diseqc_param.switch_type == SWITCH_UNICABLE)
	{
		freq = send_unicable(ad, frontend_fd, freq / 1000, diseqc,
							 pol, hiband, &tp->diseqc_param);
		hiband = pol = 0; // do not care about polarity and hiband on Unicable
	}
	else if (tp->diseqc_param.switch_type == SWITCH_JESS)
	{
		freq = send_jess(ad, frontend_fd, freq / 1000, diseqc,
						 pol, hiband, &tp->diseqc_param);
		hiband = pol = 0; // do not care about polarity and hiband on Unicable
	}
	else
	{
		int do_setup_switch = 0;
		int change_par = 0;
		if (ad->old_pol != pol || ad->old_hiband != hiband || ad->old_diseqc != diseqc)
			change_par = 1;

		if (ad == master && change_par)
			do_setup_switch = 1;

		if (ad != master) // slave adapter
		{
			change_par = 0;

			if (master->old_pol != pol || master->old_hiband != hiband || master->old_diseqc != diseqc)
				change_par = 1;

			if (!master->sid_cnt && !(master->used & ~(1 << ad->id))) // the master is not used by another adapters
				do_setup_switch = 1;

			if (change_par && !do_setup_switch)
			{
				LOG("Slave adapter %d wants to change the paramters of the master adapter %d: pol %d %d, hiband %d %d, diseqc %d %d ", ad->id, master->id, pol, ad->old_pol, hiband, ad->old_hiband, diseqc, ad->old_diseqc);
				return -1;
			}
		}
		if (do_setup_switch)
			send_diseqc(ad, frontend_fd, diseqc, ad->old_diseqc != diseqc, pol,
						hiband, &tp->diseqc_param);
		else if (ad == master)
			LOGM("Skip sending diseqc commands since the switch position doesn't need to be changed: pol %d, hiband %d, switch position %d",
				 pol, hiband, diseqc)
		else
			LOGM("Slave adapter %d not sending switch updates using master %d", ad->id, master->id);
	}
	// update master if exists, otherwise the current adapter is updated
	if (ad != master)
	{
		master->old_pol = pol;
		master->old_hiband = hiband;
		master->old_diseqc = diseqc;
	}
	ad->old_pol = pol;
	ad->old_hiband = hiband;
	ad->old_diseqc = diseqc;

	return freq;
}

//#define USE_DVBAPI3

#if DVBAPIVERSION < 0x0500
#define USE_DVBAPI3
#endif

#define ADD_PROP(c, d)             \
	{                              \
		p_cmd[iProp].cmd = (c);    \
		p_cmd[iProp].u.data = (d); \
		iProp++;                   \
	}

uint32_t pls_scrambling_index(transponder *tp)
{
	if (tp->pls_mode == PLS_MODE_ROOT)
	{
		/* convert ROOT code to GOLD code */
		uint32_t x, g;
		for (g = 0, x = 1; g < 0x3ffff; g++)
		{
			if (tp->pls_code == x)
				return g;
			x = (((x ^ (x >> 7)) & 1) << 17) | (x >> 1);
		}
		return 0x3ffff;
	}
	return tp->pls_code; /* GOLD code 0 (default) */
}

int dvb_tune(int aid, transponder *tp)
{
	int64_t bclear, bpol;
	int iProp = 0;
	adapter *ad = get_adapter(aid);
	int fd_frontend;

	int freq = tp->freq;
	struct dtv_property p_cmd[20];
	struct dtv_properties p =
		{.num = 0, .props = p_cmd};
	struct dvb_frontend_event ev;

	struct dtv_property p_clear[] =
		{
			{.cmd = DTV_CLEAR},
		};

	struct dtv_properties cmdseq_clear =
		{.num = 1, .props = p_clear};

#ifdef USE_DVBAPI3
	struct dvb_frontend_parameters fep;
	memset(&fep, 0, sizeof(fep));
#endif

	if (!ad)
		return -404;

	fd_frontend = ad->fe;
	memset(p_cmd, 0, sizeof(p_cmd));
	bclear = getTick();

	if ((ioctl(fd_frontend, FE_SET_PROPERTY, &cmdseq_clear)) == -1)
	{
		LOG("FE_SET_PROPERTY DTV_CLEAR failed for fd %d: %s", fd_frontend,
			strerror(errno));
		//        return -1;
	}

	switch (tp->sys)
	{
	case SYS_DVBS:
	case SYS_DVBS2:

		bpol = getTick();
		freq = setup_switch(ad);
		if (freq < MIN_FRQ_DVBS || freq > MAX_FRQ_DVBS)
			LOG_AND_RETURN(-404, "Frequency %d is not within range ", freq)

		ADD_PROP(DTV_SYMBOL_RATE, tp->sr)
		ADD_PROP(DTV_INNER_FEC, tp->fec)
		ADD_PROP(DTV_PILOT, tp->plts)
		ADD_PROP(DTV_ROLLOFF, tp->ro)
#if DVBAPIVERSION >= 0x0502
		if (tp->plp_isi >= 0)
#if DVBAPIVERSION >= 0x050b /* 5.11 */
			ADD_PROP(DTV_STREAM_ID, tp->plp_isi)
#else
			//In DVBAPI < 5.11 DTV_STREAM_ID takes also the PLS Code and Mode
			ADD_PROP(DTV_STREAM_ID, tp->plp_isi | (tp->pls_code << 8) | (tp->pls_mode << 26))
#endif
#endif
#if DVBAPIVERSION >= 0x050b /* 5.11 */
		if (tp->pls_mode != TP_VALUE_UNSET)
			ADD_PROP(DTV_SCRAMBLING_SEQUENCE_INDEX, pls_scrambling_index(tp))
#endif

#ifdef USE_DVBAPI3
		fep.inversion = tp->inversion;
		fep.u.qpsk.symbol_rate = tp->sr;
		fep.u.qpsk.fec_inner = tp->fec;
		fep.frequency = freq;
#endif

		LOG("tuning to %d(%d) pol: %s (%d) sr:%d fec:%s delsys:%s mod:%s rolloff:%s pilot:%s, ts clear=%jd, ts pol=%jd",
			tp->freq, freq, get_pol(tp->pol), tp->pol, tp->sr,
			get_fec(tp->fec), get_delsys(tp->sys), get_modulation(tp->mtype),
			get_rolloff(tp->ro), get_pilot(tp->plts),
			bclear, bpol)
		break;

	case SYS_DVBT:
	case SYS_DVBT2:

		if (tp->freq < MIN_FRQ_DVBT || tp->freq > MAX_FRQ_DVBT)
			LOG_AND_RETURN(-404, "Frequency %d is not within range ", tp->freq)

		freq = freq * 1000;
		ADD_PROP(DTV_BANDWIDTH_HZ, tp->bw)
		ADD_PROP(DTV_CODE_RATE_HP, tp->fec)
		ADD_PROP(DTV_CODE_RATE_LP, tp->fec)
		ADD_PROP(DTV_GUARD_INTERVAL, tp->gi)
		ADD_PROP(DTV_TRANSMISSION_MODE, tp->tmode)
		ADD_PROP(DTV_HIERARCHY, HIERARCHY_AUTO)
#if DVBAPIVERSION >= 0x0502
		if (tp->plp_isi >= 0)
			ADD_PROP(DTV_STREAM_ID, tp->plp_isi & 0xFF)
#endif

// old DVBAPI version 3
#ifdef USE_DVBAPI3
		fep.frequency = freq;
		fep.u.ofdm.bandwidth = BANDWIDTH_8_MHZ;
		if (tp->bw == 6000000)
			fep.u.ofdm.bandwidth = BANDWIDTH_6_MHZ;
		else if (tp->bw == 7000000)
			fep.u.ofdm.bandwidth = BANDWIDTH_7_MHZ;

		fep.u.ofdm.code_rate_HP = tp->fec;
		fep.u.ofdm.code_rate_LP = tp->fec;
		fep.u.ofdm.constellation = tp->mtype;
		fep.u.ofdm.transmission_mode = tp->tmode;
		fep.u.ofdm.guard_interval = tp->gi;
		fep.u.ofdm.hierarchy_information = HIERARCHY_AUTO;
		fep.inversion = tp->inversion;

#endif

		LOG(
			"tuning to %d delsys: %s bw:%d inversion:%s mod:%s fec:%s guard:%s transmission: %s, ts clear = %jd",
			freq, get_delsys(tp->sys), tp->bw, get_specinv(tp->inversion),
			get_modulation(tp->mtype), get_fec(tp->fec), get_gi(tp->gi),
			get_tmode(tp->tmode), bclear)
		break;

	case SYS_DVBC2:
	case SYS_DVBC_ANNEX_A:

		if (tp->freq < MIN_FRQ_DVBC || tp->freq > MAX_FRQ_DVBC)
			LOG_AND_RETURN(-404, "Frequency %d is not within range ", tp->freq)

		freq = freq * 1000;
		ADD_PROP(DTV_SYMBOL_RATE, tp->sr)
#if DVBAPIVERSION >= 0x0502
		if (tp->plp_isi >= 0)
		{
			int v = tp->plp_isi & 0xFF;
			if (tp->ds >= 0)
				v |= (tp->ds & 0xFF) << 8;
			ADD_PROP(DTV_STREAM_ID, v);
		}
#endif
		// valid for DD DVB-C2 devices

#ifdef USE_DVBAPI3
		fep.frequency = tp->freq;
		fep.inversion = tp->inversion;
		fep.u.qam.symbol_rate = tp->sr;
		fep.u.qam.fec_inner = FEC_AUTO;
		fep.u.qam.modulation = tp->mtype;
#endif

		LOG("tuning to %d sr:%d specinv:%s delsys:%s mod:%s ts clear = %jd",
			freq, tp->sr, get_specinv(tp->inversion), get_delsys(tp->sys),
			get_modulation(tp->mtype), bclear)
		break;

	case SYS_ATSC:
	case SYS_DVBC_ANNEX_B:

		if (tp->freq < MIN_FRQ_DVBC || tp->freq > MAX_FRQ_DVBC)
			LOG_AND_RETURN(-404, "Frequency %d is not within range ", tp->freq)

		freq = freq * 1000;

		LOG("tuning to %d delsys:%s mod:%s specinv:%s ts clear = %jd", freq,
			get_delsys(tp->sys), get_modulation(tp->mtype),
			get_specinv(tp->inversion), bclear)

		break;

	case SYS_ISDBT:

		if (tp->freq < MIN_FRQ_DVBT || tp->freq > MAX_FRQ_DVBT)
			LOG_AND_RETURN(-404, "Frequency %d is not within range ", tp->freq)

		freq = freq * 1000;
		ADD_PROP(DTV_BANDWIDTH_HZ, tp->bw)
#if DVBAPIVERSION >= 0x0501
		ADD_PROP(DTV_ISDBT_PARTIAL_RECEPTION, 0)
//		ADD_PROP(DTV_ISDBT_LAYERA_SEGMENT_COUNT,   1);
//		ADD_PROP(DTV_ISDBT_LAYER_ENABLED,   1);
#endif

		LOG("tuning to %d delsys: %s bw:%d inversion:%s , ts clear = %jd", freq,
			get_delsys(tp->sys), tp->bw, get_specinv(tp->inversion), bclear);

		break;
	default:
		LOG("tuning to unknown delsys: %s freq %d ts clear = %jd",
			get_delsys(tp->sys), freq, bclear)
		break;
	}

	ADD_PROP(DTV_FREQUENCY, freq)
	ADD_PROP(DTV_INVERSION, tp->inversion)
	ADD_PROP(DTV_MODULATION, tp->mtype);
	ADD_PROP(DTV_DELIVERY_SYSTEM, tp->sys);
	ADD_PROP(DTV_TUNE, 0)

	p.num = iProp;
	/* discard stale QPSK events */
	while (1)
	{
		if (ioctl(fd_frontend, FE_GET_EVENT, &ev) == -1)
			break;
	}

#ifndef USE_DVBAPI3
	if ((ioctl(fd_frontend, FE_SET_PROPERTY, &p)) == -1)
		if (ioctl(fd_frontend, FE_SET_PROPERTY, &p) == -1)
		{
			LOG("dvb_tune: set property failed %d %s", errno, strerror(errno));
			return -404;
		}
#else
	LOG("dvb_tune: trying dvbapi version 3");
	if (ioctl(fd_frontend, FE_SET_FRONTEND, &fep) == -1)
	{
		LOG("dvbapi v3 ioctl failed, fd %d, errno %d (%s)", fd_frontend, errno, strerror(errno));
		return -404;
	}
#endif

	return 0;
}

int dvb_set_pid(adapter *a, int i_pid)
{
	char buf[100];
	int fd;
	int hw, ad;

	hw = a->pa;
	ad = a->fn;
	if (i_pid > 8192)
		LOG_AND_RETURN(-1, "pid %d > 8192 for " DEV_DEMUX,
					   i_pid, hw, ad);

	sprintf(buf, DEV_DEMUX, hw, ad);
	if ((fd = open(buf, O_RDWR | O_NONBLOCK)) < 0)
	{
		LOG("Could not open demux device " DEV_DEMUX ": %s ", hw,
			ad, strerror(errno));
		return -1;
	}

	struct dmx_pes_filter_params s_filter_params;

	memset(&s_filter_params, 0, sizeof(s_filter_params));
	s_filter_params.pid = i_pid;
	s_filter_params.input = DMX_IN_FRONTEND;
	s_filter_params.output = DMX_OUT_TS_TAP;
	s_filter_params.flags = DMX_IMMEDIATE_START;
	s_filter_params.pes_type = DMX_PES_OTHER;

	if (ioctl(fd, DMX_SET_PES_FILTER, &s_filter_params) < 0)
	{
		int ep = a->active_pids;
		LOG0("failed setting filter on %s pid %d, errno %d (%s), enabled pids %d", buf, i_pid,
			 errno, strerror(errno), ep);
		close(fd);
		return -1;
	}

	LOG("AD %d [demux %d %d], setting filter on PID %d for fd %d", a->id, hw, ad, i_pid, fd);

	SPid *p = find_pid(a->id, i_pid);
	if (p)
		p->sock = -1;

	return fd;
}

int dvb_del_filters(adapter *ad, int fd, int pid)
{
	if (fd < 0)
		LOG_AND_RETURN(0, "DMX_STOP on an invalid handle %d, pid %d", fd, pid);
	if (ioctl(fd, DMX_STOP, NULL) < 0)
		LOG0("DMX_STOP failed on PID %d FD %d: error %d %s", pid, fd, errno, strerror(errno))
	else
		LOG("clearing filter on PID %d FD %d", pid, fd);

	SPid *p = find_pid(ad->id, pid);
	if (!p)
		LOG("%s: Could not find pid %d on adapter %d", __FUNCTION__, pid, ad->id);
	if (p && (p->sock >= 0))
	{
		sockets_force_close(p->sock);
		p->sock = -1;
	}
	else
		close(fd);
	return 0;
}

// useful on devices where DVR is not used

int dvb_demux_set_pid(adapter *a, int i_pid)
{
	int fd = a->dvr;

	if (i_pid > 8192)
		LOG_AND_RETURN(-1, "pid %d > 8192 for adapter %d", i_pid, a->id);

	if (a->active_demux_pids++ == 0)
	{
		struct dmx_pes_filter_params s_filter_params;

		memset(&s_filter_params, 0, sizeof(s_filter_params));
		s_filter_params.pid = i_pid;
		s_filter_params.input = DMX_IN_FRONTEND;
		s_filter_params.output = DMX_OUT_TSDEMUX_TAP;
		s_filter_params.flags = DMX_IMMEDIATE_START;
		s_filter_params.pes_type = DMX_PES_OTHER;

		if (ioctl(fd, DMX_SET_PES_FILTER, &s_filter_params) < 0)
		{
			int ep = a->active_pids;
			LOG0("failed setting filter on fd %d, adapter %d, pid %d, errno %d (%s), enabled pids %d", fd, a->id, i_pid, errno, strerror(errno), ep);
			return -1;
		}
		LOG("AD %d started setting filters for fd %d, active pids %d", a->id, fd, a->active_pids);
		return fd;
	}

	uint16_t p = i_pid;
	if (ioctl(fd, DMX_ADD_PID, &p) < 0)
	{
		LOG0("failed to add pid %d to fd %d maximum pids %d: errno %d, %s", p, fd, a->active_pids, errno, strerror(errno));
		return -1;
	}
	LOG("AD %d setting demux filter on PID %d for fd %d", a->id, i_pid, fd);

	return fd;
}

int dvb_demux_del_filters(adapter *ad, int fd, int pid)
{
	if (fd < 0)
		LOG_AND_RETURN(0, "DMX_STOP on an invalid handle %d, pid %d", fd, pid);

	if (pid > 8192)
		LOG_AND_RETURN(-1, "pid %d > 8192 for adapter %d", pid, ad->id);

	uint16_t p = pid;
	if (ioctl(fd, DMX_REMOVE_PID, &p) < 0)
	{
		LOG0("failed to remove pid %d to fd %d: errno %d, %s", p, fd, errno, strerror(errno));
	}

	if (!--ad->active_demux_pids)
	{
		if (ioctl(fd, DMX_STOP, NULL) < 0)
			LOG("DMX_STOP failed on PID %d FD %d: error %d %s", pid, fd, errno, strerror(errno));
		LOG("stopped filters on fd %d", fd);
	}

	LOG("clearing demux filter on PID %d FD %d, active_pids %d", pid, fd, ad->active_pids);
	return 0;
}

// construct TS header from PSI data

int dvb_psi_read(int socket, void *buf, int len, sockets *ss, int *rb)
{
	unsigned char section[4096]; // max section size
	int r;
	*rb = 0;
	//	if(len < sizeof(section) * 1.1)
	//		return 1;
	memset(section, 0, sizeof(section));
	r = read(socket, section + 1, sizeof(section) - 1); // section[0] = 0
	if (r <= 0)
	{
		//		LOG("%s: read %d, errno %d", __FUNCTION__, r, errno);
		if (errno == EOVERFLOW)
			LOG_AND_RETURN(1, "socket %d error: EOVERFLOW", socket);
		*rb = r;

		return 0;
	}
	r++;

	adapter *ad = get_adapter(ss->sid);
	if (!ad)
		return 0;

	int i, pid = -1;
	// obtain the pid
	for (i = 0; i < MAX_PIDS; i++)
		if ((ad->pids[i].flags == 1) && (ad->pids[i].fd == socket))
		{
			pid = ad->pids[i].pid;
			break;
		}

	if (pid == -1)
	{
		LOGM("Pid was not found for adapter %d, socket %d, section bytes: %02X, %02X, %02X, %02X", ad->id, socket, section[0], section[1], section[2], section[3]);
		return 1;
	}

	SPid *p = &ad->pids[i];
	//	uint32_t crc = 0;
	//	if (pid < 32 || section[1] == 2)
	//	{
	//		crc = crc_32(section + 1, r - 1);
	//		copy32(section, r, crc);
	//		r += 4;
	//	}
	DEBUGM("%s: socket %d, master %d, buf %p len %d, read %d", __FUNCTION__, socket, ss->master, buf, len, r);
	*rb = buffer_to_ts(buf, len, section, r, &p->cc1, pid);

	return 1;
}
// Use DMX_SET_FILTER to add PSI filter

int dvb_set_psi_filter(adapter *a, int i_pid)
{
	char buf[100];
	int fd;
	int hw, ad;

	hw = a->pa;
	ad = a->fn;
	if (i_pid > 8192)
		LOG_AND_RETURN(-1, "pid %d > 8192 for " DEV_DEMUX,
					   i_pid, hw, ad);

	sprintf(buf, DEV_DEMUX, hw, ad);
	if ((fd = open(buf, O_RDWR | O_NONBLOCK)) < 0)
	{
		LOG("Could not open demux device " DEV_DEMUX ": %s ", hw,
			ad, strerror(errno));
		return -1;
	}

	struct dmx_sct_filter_params sct;
	memset(&sct, 0, sizeof(sct));
	sct.pid = i_pid;
	sct.timeout = 0;
	sct.flags = DMX_IMMEDIATE_START;

	if (ioctl(fd, DMX_SET_FILTER, &sct) < 0)
	{
		int ep = a->active_pids;
		LOG0("%s: failed setting filter on %s pid %d, errno %d (%s), enabled pids %d", __FUNCTION__,
			 buf, i_pid, errno, strerror(errno), ep);
		close(fd);
		return -1;
	}

	LOG("AD %d [demux %d %d], setting PSI filter on PID %d for fd %d", a->id, hw, ad, i_pid, fd);

	int sock = sockets_add(fd, NULL, a->id, TYPE_DVR, NULL, NULL, NULL);
	if (sock < 0)
	{
		LOG("Failed to add socket for filter pid %d, adapter %d", i_pid, a->id);
		close(fd);
		return -1;
	}
	sockets_setread(sock, dvb_psi_read);
	sockets_set_master(sock, a->sock);
	SPid *p = find_pid(a->id, i_pid);
	if (p)
		p->sock = sock;
	else
		LOG0("%s: could not find pid %d", __FUNCTION__, i_pid)
	return fd;
}

int dvb_is_psi(adapter *ad, int pid)
{
	if (pid < 32)
		return 1;
#ifndef DISABLE_PMT
	if (get_pid_filter(ad->id, pid) >= 0)
		return 1;
#endif
	return 0;
}

int dvb_set_psi_pes_filter(adapter *a, int i_pid)
{
	if (dvb_is_psi(a, i_pid))
		return dvb_set_psi_filter(a, i_pid);
	else if (opts.use_demux_device == USE_PES_FILTERS_AND_DVR)
		return dvb_set_pid(a, i_pid);
	else
		return dvb_demux_set_pid(a, i_pid);
}

int dvb_del_psi_filters(adapter *ad, int fd, int pid)
{
	int rv = 0, is_psi = 0;
	SPid *p = find_pid(ad->id, pid);
	if (p)
		is_psi = (p->sock >= 0);

	if (is_psi || opts.use_demux_device == USE_PES_FILTERS_AND_DVR)
		rv = dvb_del_filters(ad, fd, pid);
	else
		rv = dvb_demux_del_filters(ad, fd, pid);
	return rv;
}

fe_delivery_system_t dvb_delsys(int aid, int fd, fe_delivery_system_t *sys)
{
	int i, res, rv = 0;
	struct dvb_frontend_info fe_info;

	static struct dtv_property enum_cmdargs[] =
		{
			{.cmd = DTV_ENUM_DELSYS, .u.data = 0},
		};
	static struct dtv_properties enum_cmdseq =
		{.num = sizeof(enum_cmdargs) / sizeof(struct dtv_property), .props = enum_cmdargs};

	for (i = 0; i < 10; i++)
		sys[i] = 0;
	if (ioctl(fd, FE_GET_PROPERTY, &enum_cmdseq) < 0)
	{
		LOG("unable to query frontend, perhaps DVB-API < 5.5 ?");
		//	return 0;
	}

	if ((res = ioctl(fd, FE_GET_INFO, &fe_info) < 0))
	{
		LOG("FE_GET_INFO failed for adapter %d, fd %d: %s ", aid, fd,
			strerror(errno));
		//	return -1;
	}

	LOG("Detected adapter %d handle %d DVB Card Name: %s", aid, fd,
		fe_info.name);

	int nsys = enum_cmdargs[0].u.buffer.len;

	if (nsys < 1)
	{

		int idx = 0;
		switch (fe_info.type)
		{
		case FE_OFDM:
#if DVBAPIVERSION >= 0x0501
			if (fe_info.caps & FE_CAN_2G_MODULATION)
				sys[idx++] = SYS_DVBT2;
#endif

			sys[idx++] = SYS_DVBT;

			break;
		case FE_QAM:
			sys[idx++] = SYS_DVBC_ANNEX_AC;
			break;
		case FE_QPSK:
#if DVBAPIVERSION >= 0x0501
			if (fe_info.caps & FE_CAN_2G_MODULATION)
				sys[idx++] = SYS_DVBS2;
#endif

			sys[idx++] = SYS_DVBS;

			break;
		case FE_ATSC:
			if (fe_info.caps & (FE_CAN_8VSB | FE_CAN_16VSB))
				sys[idx++] = SYS_ATSC;
			else if (fe_info.caps & (FE_CAN_QAM_64 | FE_CAN_QAM_256 | FE_CAN_QAM_AUTO))
				sys[idx++] = SYS_DVBC_ANNEX_B;
			else
				return 0;

			break;
		default:
			LOG("no available delivery system for adapter %d", aid);
			return 0;
		}
		nsys = idx;
		rv = sys[0];
	}
	else
	{
		for (i = 0; i < nsys; i++)
		{
			sys[i] = enum_cmdargs[0].u.buffer.data[i];
		}
		rv = enum_cmdargs[0].u.buffer.data[0];
	}
	for (i = 0; i < nsys; i++)
		LOG("Detected delivery system for adapter %d: %s [%d]", aid,
			get_delsys(sys[i]), sys[i]);

	return (fe_delivery_system_t)rv;
}

// returns the strength and SNR between 0 .. 65535

void get_signal(adapter *ad, int *status, uint32_t *ber, uint16_t *strength, uint16_t *snr)
{
	*status = 0;
	*ber = *snr = *strength = 0;

	if (ad->fe > 0 && ioctl(ad->fe, FE_READ_STATUS, status) < 0)
	{
		LOG("ad %d ioctl fd %d FE_READ_STATUS failed, error %d (%s)", ad->id, ad->fe, errno, strerror(errno));
		*status = 0;
		return;
	}
	//	*status = (*status & FE_HAS_LOCK) ? 1 : 0;
	if (*status)
	{
		if (ad->fe > 0 && ioctl(ad->fe, FE_READ_BER, ber) < 0)
			LOG("ad %d ioctl fd %d, FE_READ_BER failed, error %d (%s)", ad->id, ad->fe, errno, strerror(errno));

		if (ad->fe > 0 && ioctl(ad->fe, FE_READ_SIGNAL_STRENGTH, strength) < 0)
		{
			LOG("ad %d ioctl fd %d FE_READ_SIGNAL_STRENGTH failed, error %d (%s)", ad->id, ad->fe, errno, strerror(errno));
		}

		if (ad->fe > 0 && ioctl(ad->fe, FE_READ_SNR, snr) < 0)
		{
			LOG("ad %d ioctl fd %d FE_READ_SNR failed, error %d (%s)", ad->id, ad->fe, errno, strerror(errno));
		}
	}
	LOGM("get_signal adapter %d: status %d, strength %d, snr %d, BER: %d", ad->id, *status, *strength, *snr, *ber);
}

// returns the strength and SNR between 0 .. 65535

int get_signal_new(adapter *ad, int *status, uint32_t *ber, uint16_t *strength, uint16_t *snr)
{
#if DVBAPIVERSION >= 0x050A
	*status = *snr = *ber = *strength = 0;
	int64_t strengthd = 0, snrd = 0, init_strength = 0, init_snr = 0;
	char *strength_s = "(none)", *snr_s = "(none)";
	int err = 0;
	static struct dtv_property enum_cmdargs[] =
		{
			{.cmd = DTV_STAT_SIGNAL_STRENGTH, .u.data = 0},
			{.cmd = DTV_STAT_CNR, .u.data = 0},
			{.cmd = DTV_STAT_ERROR_BLOCK_COUNT, .u.data = 0},
		};
	static struct dtv_properties enum_cmdseq =
		{.num = sizeof(enum_cmdargs) / sizeof(struct dtv_property), .props = enum_cmdargs};

	if (ad->fe > 0 && ioctl(ad->fe, FE_GET_PROPERTY, &enum_cmdseq) < 0)
	{
		LOG("get_signal_new: unable to query frontend %d: %s", ad->fe,
			strerror(errno));
		err = 100;
	}

	if (enum_cmdargs[0].u.st.stat[0].scale == FE_SCALE_RELATIVE)
	{
		strength_s = "%";
		strengthd = enum_cmdargs[0].u.st.stat[0].uvalue; // The frontend provides a 0% to 100% measurement for Signal/Noise (actually, 0 to 65535)
	}
	else if (enum_cmdargs[0].u.st.stat[0].scale == FE_SCALE_DECIBEL)
	{
		strength_s = "dBm";
		init_strength = enum_cmdargs[0].u.st.stat[0].svalue;			   // dBm / 1000
		strengthd = (init_strength + 100 * 1000) * 65535.0 / (100 * 1000); // dBm value + 100 ==> %  ( -97 dBm => 3%)
	}
	else if (enum_cmdargs[0].u.st.stat[0].scale == 0)
		err |= 1;

	if (enum_cmdargs[1].u.st.stat[0].scale == FE_SCALE_RELATIVE)
	{
		snr_s = "%";
		snrd = enum_cmdargs[1].u.st.stat[0].uvalue;
	}
	else if (enum_cmdargs[1].u.st.stat[0].scale == FE_SCALE_DECIBEL)
	{
		snr_s = "dB";
		init_snr = enum_cmdargs[1].u.st.stat[0].svalue; // dB / 1000
		snrd = init_snr * 65535.0 / (100 * 1000);		// dB value ==> %  ( 3 dB => 3%)
	}
	//	else if (enum_cmdargs[1].u.st.stat[0].scale == 0)
	//		err |= 2;

	*ber = enum_cmdargs[2].u.st.stat[0].uvalue & 0x7FFF;
	if (ad->fe > 0 && ioctl(ad->fe, FE_READ_STATUS, status) < 0)
	{
		LOG("ioctl fd %d FE_READ_STATUS failed, error %d (%s)", ad->fe, errno, strerror(errno));
		*status = 0;
	}

	LOGM("get_signal_new adapter %d: status %d, strength %jd %s -> %jd, snr %jd %s -> %jd, BER: %d, err %d",
		 ad->id, *status, init_strength, strength_s, strengthd, init_snr, snr_s, snrd, *ber, err);

	if (err)
		return err;

	*strength = (int)strengthd;
	*snr = (int)snrd;

	return 0;

#else
	return -1;
#endif
}

#define NEW_SIGNAL 1
#define OLD_SIGNAL 2

// converts the strength and SNR between 0 .. 255 after multiplying with *_multiplier

void dvb_get_signal(adapter *ad)
{
	int start = 0;
	uint16_t strength = 0, snr = 0;
	int status = 0;
	uint32_t ber = 0;

	if (ad->strength_multiplier || ad->snr_multiplier)
	{
		if (ad->new_gs == 0)
		{
			int new_gs = get_signal_new(ad, &status, &ber, &strength, &snr);
			if (!new_gs)
				ad->new_gs = NEW_SIGNAL;
			else
				ad->new_gs = OLD_SIGNAL;
			start = 1;
		}

		if (!start && ad->new_gs == NEW_SIGNAL)
			get_signal_new(ad, &status, &ber, &strength, &snr);

		if (ad->new_gs == OLD_SIGNAL)
			get_signal(ad, &status, &ber, &strength, &snr);
	}
	else
	{
		status = 1;
		LOGM("Signal is not retrieved from the adapter as both signal multipliers are 0");
	}
	if (ad->strength_multiplier)
		strength = strength * ad->strength_multiplier;
	else
		strength = 65535;
	if (ad->snr_multiplier)
		snr = snr * ad->snr_multiplier;
	else
		snr = 65535;

	strength = strength >> 8;
	snr = snr >> 8;

	if (strength > 255)
		strength = 255;

	if (snr > 255)
		snr = 255;

	// keep the assignment at the end for the signal thread to get the right values as no locking is done on the adapter
	ad->snr = snr;
	ad->strength = strength;
	ad->status = status;
	ad->ber = ber;

	if (ad->status == 0 && ((ad->tp.diseqc_param.switch_type == SWITCH_JESS) || (ad->tp.diseqc_param.switch_type == SWITCH_UNICABLE)))
	{
		adapter_lock(ad->id);
		setup_switch(ad);
		adapter_unlock(ad->id);
	}
}

void dvb_commit(adapter *a)
{
	return;
}

void dvb_close(adapter *a)
{
	if (a->dmx >= 0)
		close(a->dmx);
	a->dmx = -1;
	return;
}

void find_dvb_adapter(adapter **a)
{
	int na = 0;
	char buf[100];
	int cnt;
	int i = 0, j = 0;
	adapter *ad;
	if (opts.disable_dvb)
	{
		LOG("DVB device detection disabled");
		return;
	}
	for (i = 0; i < MAX_ADAPTERS; i++)
		for (j = 0; j < MAX_ADAPTERS; j++)
		{
			cnt = 0;
			sprintf(buf, DEV_FRONTEND, i, j);
			if (!access(buf, R_OK))
				cnt++;

			sprintf(buf, DEV_DEMUX, i, j);
			if (!access(buf, R_OK))
				cnt++;

			sprintf(buf, DEV_DVR, i, j);
			if (!access(buf, R_OK))
				cnt++;

			if (cnt == 3)
			{
				//				if (is_adapter_disabled(na))
				//				{
				//					na++;
				//					continue;
				//				}
				LOGM("%s: adding %d %d to the list of devices", __FUNCTION__, i, j);
				if (!a[na])
					a[na] = adapter_alloc();

				ad = a[na];
				ad->pa = i;
				ad->fn = j;

				ad->open = (Open_device)dvb_open_device;
				if (opts.use_demux_device == USE_DVR)
				{
					ad->set_pid = (Set_pid)dvb_set_pid;
					ad->del_filters = (Del_filters)dvb_del_filters;
				}
				else if (opts.use_demux_device == USE_DEMUX)
				{
					ad->set_pid = (Set_pid)dvb_demux_set_pid;
					ad->del_filters = (Del_filters)dvb_demux_del_filters;
				}
				else if (opts.use_demux_device >= USE_PES_FILTERS_AND_DVR)
				{
					ad->set_pid = (Set_pid)dvb_set_psi_pes_filter;
					ad->del_filters = (Del_filters)dvb_del_psi_filters;
				}

				ad->commit = (Adapter_commit)dvb_commit;
				ad->tune = (Tune)dvb_tune;
				ad->delsys = (Dvb_delsys)dvb_delsys;
				ad->post_init = NULL;
				ad->close = (Adapter_commit)dvb_close;
				ad->get_signal = (Device_signal)dvb_get_signal;
				ad->type = ADAPTER_DVB;

				if (ad->pa == 0)
				{
					sprintf(buf, "/proc/stb/frontend/%d/fbc_connect", ad->fn);
					if (!access(buf, W_OK))
						ad->is_fbc = 1;
				}

				na++;
				a_count = na; // update adapter counter
				if (na == MAX_ADAPTERS)
					return;
			}
		}
	for (; na < MAX_ADAPTERS; na++)
		if (a[na])
			a[na]->pa = a[na]->fn = -1;
}

#endif // #ifndef DISABLE_LINUXDVB
