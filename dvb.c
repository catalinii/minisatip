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

char *fe_pilot[] =
{ "on", "off", " ", //auto
		NULL };

char *fe_rolloff[] =
{ "0.35", "0.20", "0.25", " ", //auto
		NULL };

char *fe_delsys[] =
{ "undefined", "dvbc", "dvbcb", "dvbt", "dss", "dvbs", "dvbs2", "dvbh", "isdbt",
		"isdbs", "isdbc", "atsc", "atscmh", "dmbth", "cmmb", "dab", "dvbt2",
		"turbo", "dvbcc", "dvbc2",
		NULL };

char *fe_fec[] =
{ "none", "12", "23", "34", "45", "56", "67", "78", "89", " ", //auto
		"35", "910", "25",
		NULL };

char *fe_modulation[] =
{ "qpsk", "16qam", "32qam", "64qam", "128qam", "256qam", " ", // auto
		"8vsb", "16vsb", "8psk", "16apsk", "32apsk", "dqpsk",
		NULL };

char *fe_tmode[] =
{ "2k", "8k", " ", //auto
		"4k", "1k", "16k", "32k", "c1", "c3780",
		NULL };

char *fe_gi[] =
{ "132", "116", "18", "14", " ", // auto
		"1128", "19128", "19256", "pn420", "pn595", "pn945",
		NULL };

char *fe_hierarchy[] =
{ "HIERARCHY_NONE", "HIERARCHY_1", "HIERARCHY_2", "HIERARCHY_4",
		"HIERARCHY_AUTO",
		NULL };

char *fe_specinv[] =
{ "off", "on", " ", // auto
		NULL };

char *fe_pol[] =
{ "none", "v", "h", "r", "l",
NULL };

#define make_func(a) \
char * get_##a(int i) \
{ \
	if(i>=0 && i< (int)sizeof(fe_##a)) { \
		if(fe_##a[i][0] == 32)return ""; \
			else return fe_##a[i]; }  \
	return "NONE"; \
} 
make_func(pilot);
make_func(rolloff);
make_func(delsys);
make_func(fec);
make_func(modulation);
make_func(tmode);
make_func(gi);
make_func(specinv);
make_func(pol);

#define INVALID_URL(a) {LOG(a);return 0;}
char def_pids[100];

//#define default_pids "0,1,2,3"
#define default_pids "8192"

int detect_dvb_parameters(char *s, transponder * tp)
{
	char *arg[20];
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
	tp->plp = -1;

	tp->pids = tp->apids = tp->dpids = tp->x_pmt = NULL;

	while (*s > 0 && *s != '?')
		s++;

	if (*s == 0)
		LOG_AND_RETURN(0, "no ? found in URL");

	s++;
	if (strstr(s, "freq="))
		init_dvb_parameters(tp);

	LOG("detect_dvb_parameters (S)-> %s", s);
	la = split(arg, s, 20, '&');

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
		if (strncmp("plp=", arg[i], 4) == 0)
			tp->plp = map_int(arg[i] + 4, NULL);

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
		strcpy(def_pids, default_pids);
// map pids=all to essential pids
		tp->pids = (char *) def_pids;
	}

	if (tp->pids && strncmp(tp->pids, "none", 3) == 0)
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

void init_dvb_parameters(transponder * tp)
{
	memset(tp, 0, sizeof(transponder));
	tp->inversion = INVERSION_AUTO;
	tp->hprate = FEC_AUTO;
	tp->tmode = TRANSMISSION_MODE_AUTO;
	tp->gi = GUARD_INTERVAL_AUTO;
	tp->bw = 8000000;
	tp->ro = ROLLOFF_AUTO;
	tp->mtype = -1;
	tp->plts = PILOT_AUTO;
	tp->fec = FEC_AUTO;
}

void copy_dvb_parameters(transponder * s, transponder * d)
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
	if (s->ds != -1)
		d->ds = s->ds;
	if (s->plp != -1)
		d->plp = s->plp;

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

	if ((d->sys == SYS_ATSC || d->sys == SYS_DVBC_ANNEX_B) && d->mtype == -1)
		d->mtype = QAM_AUTO;

	if ((d->sys == SYS_DVBT || d->sys == SYS_DVBT2) && d->mtype == -1)
		d->mtype = QAM_AUTO;

	LOG(
			"copy_dvb_parameters -> src=%d, fe=%d, freq=%d, fec=%d sr=%d, pol=%d, ro=%d, msys=%d, mtype=%d, plts=%d, bw=%d, inv=%d, pids=%s, apids=%s, dpids=%s x_pmt=%s",
			d->diseqc, d->fe, d->freq, d->fec, d->sr, d->pol, d->ro, d->sys,
			d->mtype, d->plts, d->bw, d->inversion, d->pids ? d->pids : "NULL",
			d->apids ? d->apids : "NULL", d->dpids ? d->dpids : "NULL",
			d->x_pmt ? d->x_pmt : "NULL");
}

#ifndef DISABLE_LINUXDVB

extern struct struct_opts opts;

struct diseqc_cmd
{
	struct dvb_diseqc_master_cmd cmd;
	uint32_t wait;
};

int dvb_open_device(adapter *ad)
{
	char buf[100];
	LOG("trying to open [%d] adapter %d and frontend %d", ad->id, ad->pa,
			ad->fn);
	sprintf(buf, "/dev/dvb/adapter%d/frontend%d", ad->pa, ad->fn);
	ad->fe = open(buf, O_RDWR | O_NONBLOCK);
	sprintf(buf, "/dev/dvb/adapter%d/dvr%d", ad->pa, ad->fn);
	ad->dvr = open(buf, O_RDONLY | O_NONBLOCK);
	if (ad->fe < 0 || ad->dvr < 0)
	{
		sprintf(buf, "/dev/dvb/adapter%d/frontend%d", ad->pa, ad->fn);
		LOGL(0, "Could not open %s in RW mode (fe: %d, dvr: %d)", buf, ad->fe,
				ad->dvr);
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

	if (ad->dmx_source >= 0)
	{
		sprintf(buf, "/dev/dvb/adapter%d/demux%d", ad->pa, ad->fn);
		ad->dmx = open(buf, O_RDWR | O_NONBLOCK);
		if (ad->dmx <= 0)
			LOG("DMX_SET_SOURCE: Failed opening %s", buf)
		else if (ioctl(ad->dmx, DMX_SET_SOURCE, &ad->dmx_source))
		{
			LOG("DMX_SET_SOURCE failed for adapter %d - %d: %s", ad->id, errno,
					strerror(errno));
		}
		else
			LOG("Set DMX_SET_SOURCE for adapter %d to %d", ad->id,
					ad->dmx_source);

	}

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

int send_diseqc(int fd, int pos, int pos_change, int pol, int hiband, diseqc *d)
{
	int committed_no = d->committed_no;
	int uncommitted_no = d->uncommitted_no;
	int uncommitted_first = 0;
	int posu, posc;
	/* DiSEqC 1.0 */
	struct dvb_diseqc_master_cmd cmd =
	{
	{ 0xe0, 0x10, 0x38, 0xf0, 0x00, 0x00 }, 4 };
	/* DiSEqC 1.1 */
	struct dvb_diseqc_master_cmd uncmd =
	{
	{ 0xe0, 0x10, 0x39, 0xf0, 0x00, 0x00 }, 4 };

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

	cmd.msg[3] = 0xf0
			| (((posc << 2) & 0x0c) | (hiband ? 1 : 0) | (pol ? 2 : 0));
	uncmd.msg[3] = 0xf0 | (posu & 0x0f);

	LOGL(3, "send_diseqc fd %d, pos = %d (c %d u %d), pol = %d, hiband = %d",
			fd, pos, posc, posu, pol, hiband);

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

		if (ioctl(fd, FE_DISEQC_SEND_BURST, (pos & 1) ? SEC_MINI_B : SEC_MINI_A)
				== -1)
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

int send_unicable(int fd, int freq, int pos, int pol, int hiband, diseqc *d)
{
	struct dvb_diseqc_master_cmd cmd =
	{
	{ 0xe0, 0x11, 0x5a, 0x00, 0x00 }, 5 };
	int t;

	t = (freq + d->ufreq + 2) / 4 - 350;

	cmd.msg[3] = ((t & 0x0300) >> 8) | (d->uslot << 5) | (pos ? 0x10 : 0)
			| (hiband ? 4 : 0) | (pol ? 8 : 0);
	cmd.msg[4] = t & 0xff;

	if (d->pin)
	{
		cmd.msg_len = 6;
		cmd.msg[2] = 0x5C;
		cmd.msg[5] = d->pin;
	}

	LOGL(3,
			"send_unicable fd %d, freq %d, ufreq %d, pos = %d, pol = %d, hiband = %d, slot %d, diseqc => %02x %02x %02x %02x %02x",
			fd, freq, d->ufreq, pos, pol, hiband, d->uslot, cmd.msg[0],
			cmd.msg[1], cmd.msg[2], cmd.msg[3], cmd.msg[4]);
	if (ioctl(fd, FE_SET_VOLTAGE, SEC_VOLTAGE_13) == -1)
		LOG("send_unicable: pre voltage  SEC_VOLTAGE_13 failed for fd %d: %s",
				fd, strerror(errno));
	msleep(d->before_cmd);
	if (ioctl(fd, FE_SET_TONE, SEC_TONE_OFF) == -1)
		LOG("send_unicable: FE_SET_TONE failed for fd %d: %s", fd,
				strerror(errno));
	if (!d->only13v && ioctl(fd, FE_SET_VOLTAGE, SEC_VOLTAGE_18) == -1)
		LOG("send_unicable: FE_SET_VOLTAGE failed for fd %d: %s", fd,
				strerror(errno));
	msleep(d->after_burst);
	if (ioctl(fd, FE_DISEQC_SEND_MASTER_CMD, &cmd) == -1)
		LOG("send_unicable: FE_DISEQC_SEND_MASTER_CMD failed for fd %d: %s", fd,
				strerror(errno));
	msleep(d->after_repeated_cmd);
	if (ioctl(fd, FE_SET_VOLTAGE, SEC_VOLTAGE_13) == -1)
		LOG("send_unicable: FE_SET_VOLTAGE failed for fd %d: %s", fd,
				strerror(errno));
	msleep(d->after_tone);

	return d->ufreq * 1000;
}

int send_jess(int fd, int freq, int pos, int pol, int hiband, diseqc *d)
{
	struct dvb_diseqc_master_cmd cmd =
	{
	{ 0x70, 0x00, 0x00, 0x00, 0x00 }, 4 };
//	int t = (freq / 1000) - 100;
	int t = freq - 100;

	cmd.msg[1] = d->uslot << 3;
	cmd.msg[1] |= ((t >> 8) & 0x07);
	cmd.msg[2] = (t & 0xff);
	cmd.msg[3] = ((pos & 0x3f) << 2) | (pol ? 2 : 0) | (hiband ? 1 : 0);
	if (d->pin < 256)
	{
		cmd.msg_len = 5;
		cmd.msg[0] = 0x71;
		cmd.msg[4] = d->pin;
	}

	LOGL(3,
			"send_jess fd %d, freq %d, ufreq %d, pos = %d, pol = %d, hiband = %d, slot %d, diseqc => %02x %02x %02x %02x %02x",
			fd, freq, d->ufreq, pos, pol, hiband, d->uslot, cmd.msg[0],
			cmd.msg[1], cmd.msg[2], cmd.msg[3], cmd.msg[4]);

	if (ioctl(fd, FE_SET_VOLTAGE, SEC_VOLTAGE_13) == -1)
		LOG("send_jess: pre voltage  SEC_VOLTAGE_13 failed for fd %d: %s", fd,
				strerror(errno));
	msleep(d->before_cmd);
	if (ioctl(fd, FE_SET_TONE, SEC_TONE_OFF) == -1)
		LOG("send_jess: FE_SET_TONE failed for fd %d: %s", fd, strerror(errno));
	if (!d->only13v && ioctl(fd, FE_SET_VOLTAGE, SEC_VOLTAGE_18) == -1)
		LOG("send_jess: FE_SET_VOLTAGE failed for fd %d: %s", fd,
				strerror(errno));
	msleep(d->after_burst);
	if (ioctl(fd, FE_DISEQC_SEND_MASTER_CMD, &cmd) == -1)
		LOG("send_jess: FE_DISEQC_SEND_MASTER_CMD failed for fd %d: %s", fd,
				strerror(errno));
	msleep(d->after_repeated_cmd);
	if (ioctl(fd, FE_SET_VOLTAGE, SEC_VOLTAGE_13) == -1)
		LOG("send_jess: FE_SET_VOLTAGE failed for fd %d: %s", fd,
				strerror(errno));
	msleep(d->after_tone);

	return d->ufreq * 1000;
}

int setup_switch(int frontend_fd, adapter *ad, transponder *tp)
{
	int hiband = 0;
	int diseqc = (tp->diseqc > 0) ? tp->diseqc - 1 : 0;
	int freq = tp->freq;
	int pol = (tp->pol - 1) & 1;

	if (tp->pol > 2)
	{
		freq = (freq - LOF3);
		hiband = 0;
	}
	else if (freq < SLOF)
	{
		freq = (freq - LOF1);
		hiband = 0;
	}
	else
	{
		freq = (freq - LOF2);
		hiband = 1;
	}

	if (tp->diseqc_param.switch_type == SWITCH_UNICABLE)
	{
		freq = send_unicable(frontend_fd, freq / 1000, diseqc, pol, hiband,
				&tp->diseqc_param);
	}
	else if (tp->diseqc_param.switch_type == SWITCH_JESS)
	{
		freq = send_jess(frontend_fd, freq / 1000, diseqc, pol, hiband,
				&tp->diseqc_param);
	}
	else if (tp->diseqc_param.switch_type == SWITCH_SLAVE)
	{
		LOGL(2, "FD %d is a slave adapter", frontend_fd);
	}
	else
	{
		if (ad->old_pol != pol || ad->old_hiband != hiband
				|| ad->old_diseqc != diseqc)
			send_diseqc(frontend_fd, diseqc, ad->old_diseqc != diseqc, pol,
					hiband, &tp->diseqc_param);
		else
			LOGL(3, "Skip sending diseqc commands since "
					"the switch position doesn't need to be changed: "
					"pol %d, hiband %d, switch position %d", pol, hiband,
					diseqc);
	}

	ad->old_pol = pol;
	ad->old_hiband = hiband;
	ad->old_diseqc = diseqc;

	return freq;
}

#define ADD_PROP(c, d) {\
	p_cmd[iProp].cmd = (c);\
	p_cmd[iProp].u.data = (d);\
	iProp++;\
	}

int dvb_tune(int aid, transponder * tp)
{
	int64_t bclear, bpol;
	int iProp = 0;
	adapter *ad = get_adapter(aid);
	int fd_frontend = ad->fe;

	int freq = tp->freq;
	struct dtv_property p_cmd[20];
	struct dtv_properties p =
	{ .num = 0, .props = p_cmd };
	struct dvb_frontend_event ev;

	struct dtv_property p_clear[] =
	{
	{ .cmd = DTV_CLEAR }, };

	struct dtv_properties cmdseq_clear =
	{ .num = 1, .props = p_clear };

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
		freq = setup_switch(fd_frontend, ad, tp);
		if (freq < MIN_FRQ_DVBS || freq > MAX_FRQ_DVBS)
			LOG_AND_RETURN(-404, "Frequency %d is not within range ", freq)

		ADD_PROP(DTV_SYMBOL_RATE, tp->sr)
		ADD_PROP(DTV_INNER_FEC, tp->fec)
		ADD_PROP(DTV_PILOT, tp->plts)
		ADD_PROP(DTV_ROLLOFF, tp->ro)
#if DVBAPIVERSION >= 0x0502
		ADD_PROP(DTV_STREAM_ID, tp->plp)
#endif

		LOG(
				"tuning to %d(%d) pol: %s (%d) sr:%d fec:%s delsys:%s mod:%s rolloff:%s pilot:%s, ts clear=%jd, ts pol=%jd",
				tp->freq, freq, get_pol(tp->pol), tp->pol, tp->sr,
				fe_fec[tp->fec], fe_delsys[tp->sys], fe_modulation[tp->mtype],
				fe_rolloff[tp->ro], fe_pilot[tp->plts], bclear, bpol)
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
		ADD_PROP(DTV_STREAM_ID, tp->plp & 0xFF)
#endif

		LOG(
				"tuning to %d delsys: %s bw:%d inversion:%s mod:%s fec:%s guard:%s transmission: %s, ts clear = %jd",
				freq, fe_delsys[tp->sys], tp->bw, fe_specinv[tp->inversion],
				fe_modulation[tp->mtype], fe_fec[tp->fec], fe_gi[tp->gi],
				fe_tmode[tp->tmode], bclear)
		break;

	case SYS_DVBC2:
	case SYS_DVBC_ANNEX_A:

		if (tp->freq < MIN_FRQ_DVBC || tp->freq > MAX_FRQ_DVBC)
			LOG_AND_RETURN(-404, "Frequency %d is not within range ", tp->freq)

		freq = freq * 1000;
		ADD_PROP(DTV_SYMBOL_RATE, tp->sr)
#if DVBAPIVERSION >= 0x0502
		ADD_PROP(DTV_STREAM_ID, ((tp->ds & 0xFF) << 8) | (tp->plp & 0xFF))
#endif
		// valid for DD DVB-C2 devices

		LOG("tuning to %d sr:%d specinv:%s delsys:%s mod:%s ts clear = %jd",
				freq, tp->sr, fe_specinv[tp->inversion], fe_delsys[tp->sys],
				fe_modulation[tp->mtype], bclear)
		break;

	case SYS_ATSC:
	case SYS_DVBC_ANNEX_B:

		if (tp->freq < MIN_FRQ_DVBC || tp->freq > MAX_FRQ_DVBC)
			LOG_AND_RETURN(-404, "Frequency %d is not within range ", tp->freq)

		freq = freq * 1000;

		LOG("tuning to %d delsys:%s mod:%s specinv:%s ts clear = %jd", freq,
				fe_delsys[tp->sys], fe_modulation[tp->mtype],
				fe_specinv[tp->inversion], bclear)

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
				fe_delsys[tp->sys], tp->bw, fe_specinv[tp->inversion], bclear)
		;

		break;
	default:
		LOG("tuninng to unknown delsys: %s freq %s ts clear = %jd", freq,
				fe_delsys[tp->sys], bclear)
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

	if ((ioctl(fd_frontend, FE_SET_PROPERTY, &p)) == -1)
		if (ioctl(fd_frontend, FE_SET_PROPERTY, &p) == -1)
		{
			LOG("dvb_tune: set property failed %d %s", errno, strerror(errno));
			return -404;
		}

	return 0;
}

int dvb_set_pid(adapter *a, uint16_t i_pid)
{
	char buf[100];
	int fd;
	int hw, ad;

	hw = a->pa;
	ad = a->fn;
	if (i_pid > 8192)
		LOG_AND_RETURN(-1, "pid %d > 8192 for /dev/dvb/adapter%d/demux%d",
				i_pid, hw, ad);

	sprintf(buf, "/dev/dvb/adapter%d/demux%d", hw, ad);
	if ((fd = open(buf, O_RDWR | O_NONBLOCK)) < 0)
	{
		LOG("Could not open demux device /dev/dvb/adapter%d/demux%d: %s ", hw,
				ad, strerror (errno));
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
		int pids[MAX_PIDS];
		int ep;
		ep = get_enabled_pids(a, (int *) pids, MAX_PIDS);
		LOG("failed setting filter on %d (%s), enabled pids %d", i_pid,
				strerror (errno), ep);
		return -1;
	}

	LOG("setting filter on PID %d for fd %d", i_pid, fd);

	return fd;
}

int dvb_del_filters(int fd, int pid)
{
	if (fd < 0)
		LOG_AND_RETURN(0, "DMX_STOP on an invalid handle %d, pid %d", fd, pid);
	if (ioctl(fd, DMX_STOP, NULL) < 0)
		LOG("DMX_STOP failed on PID %d FD %d: %s", pid, fd, strerror (errno))
	else
		LOG("clearing filter on PID %d FD %d", pid, fd);
	close(fd);
	return 0;
}

fe_delivery_system_t dvb_delsys(int aid, int fd, fe_delivery_system_t *sys)
{
	int i, res, rv = 0;
	struct dvb_frontend_info fe_info;

	static struct dtv_property enum_cmdargs[] =
	{
	{ .cmd = DTV_ENUM_DELSYS, .u.data = 0 }, };
	static struct dtv_properties enum_cmdseq =
	{ .num = sizeof(enum_cmdargs) / sizeof(struct dtv_property), .props =
			enum_cmdargs };

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
			else if (fe_info.caps
					& (FE_CAN_QAM_64 | FE_CAN_QAM_256 | FE_CAN_QAM_AUTO))
				sys[idx++] = SYS_DVBC_ANNEX_B;
			else
				return 0;

			break;
		default:
			LOG("no available delivery system for adapter %d", aid)
			;
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
				fe_delsys[sys[i]], sys[i]);

	return (fe_delivery_system_t) rv;

}

void get_signal(int fd, int * status, uint32_t * ber, uint16_t * strength,
		uint16_t * snr)
{
	*status = *ber = *snr = *strength = 0;

	if (ioctl(fd, FE_READ_STATUS, status) < 0)
	{
		LOG("ioctl FE_READ_STATUS failed (%s)", strerror (errno));
		*status = 0;
		return;
	}
//	*status = (*status & FE_HAS_LOCK) ? 1 : 0;
	if (*status)
	{
		if (ioctl(fd, FE_READ_BER, ber) < 0)
			LOG("ioctl FE_READ_BER failed (%s)", strerror (errno));

		if (ioctl(fd, FE_READ_SIGNAL_STRENGTH, strength) < 0)
			LOG("ioctl FE_READ_SIGNAL_STRENGTH failed (%s)", strerror (errno));

		if (ioctl(fd, FE_READ_SNR, snr) < 0)
			LOG("ioctl FE_READ_SNR failed (%s)", strerror (errno));
	}
}

int get_signal_new(int fd, int * status, uint32_t * ber, uint16_t * strength,
		uint16_t * snr)
{

	*status = *snr = *ber = *strength = 0;

#if DVBAPIVERSION >= 0x050A
	int err = 0;
	static struct dtv_property enum_cmdargs[] =
	{
	{ .cmd = DTV_STAT_SIGNAL_STRENGTH, .u.data = 0 },
	{ .cmd = DTV_STAT_CNR, .u.data = 0 },
	{ .cmd = DTV_STAT_ERROR_BLOCK_COUNT, .u.data = 0 }, };
	static struct dtv_properties enum_cmdseq =
	{ .num = sizeof(enum_cmdargs) / sizeof(struct dtv_property), .props =
			enum_cmdargs };

	if (ioctl(fd, FE_GET_PROPERTY, &enum_cmdseq) < 0)
	{
		LOG("get_signal_new: unable to query frontend %d: %s", fd,
				strerror (errno));
		err = 100;
	}

	if (enum_cmdargs[0].u.st.stat[0].scale == FE_SCALE_RELATIVE)
		*strength = enum_cmdargs[0].u.st.stat[0].uvalue;
	else if (enum_cmdargs[0].u.st.stat[0].scale == FE_SCALE_DECIBEL)
		*strength = enum_cmdargs[0].u.st.stat[0].uvalue;
	else
		err++;

	if (enum_cmdargs[1].u.st.stat[0].scale == FE_SCALE_RELATIVE)
		*snr = enum_cmdargs[1].u.st.stat[0].uvalue;
	else if (enum_cmdargs[1].u.st.stat[0].scale == FE_SCALE_DECIBEL)
		*snr = enum_cmdargs[1].u.st.stat[0].uvalue;
	else
		err++;

	*ber = enum_cmdargs[2].u.st.stat[0].uvalue & 0xFFFF;

	if (err)
		LOG(
				"get_signal_new returned: Signal (%d): %llu, SNR(%d): %llu, BER: %llu, err %d",
				enum_cmdargs[0].u.st.stat[0].scale,
				enum_cmdargs[0].u.st.stat[0].uvalue,
				enum_cmdargs[1].u.st.stat[0].scale,
				enum_cmdargs[1].u.st.stat[0].uvalue,
				enum_cmdargs[2].u.st.stat[0].uvalue, err);
	if (err)
		return err;

	if (ioctl(fd, FE_READ_STATUS, status) < 0)
	{
		LOG("ioctl FE_READ_STATUS failed (%s)", strerror (errno));
		*status = 0;
		return -1;
	}

	if (*status && !*strength)
		return -1;
	return 0;
#else
	return -1;
#endif
}

void dvb_get_signal(adapter *ad)
{
	int new_gs = 1;
	uint16_t strength = 0, snr = 0;
	uint32_t status = 0, ber = 0;
	if (ad->new_gs == 0
			&& (new_gs = get_signal_new(ad->fe, &status, &ber, &strength, &snr)))
		get_signal(ad->fe, &status, &ber, &strength, &snr);
	else if (new_gs)
		get_signal(ad->fe, &status, &ber, &strength, &snr);

	if (status > 0 && new_gs != 0) // we have signal but no new stats, don't try to get them from now on until adapter close
		ad->new_gs = 1;

	if (ad->max_strength <= strength)
		ad->max_strength = (strength > 0) ? strength : 1;
	if (ad->max_snr <= snr)
		ad->max_snr = (snr > 0) ? snr : 1;
	if (snr > 4096)
		new_gs = 0;
	if (new_gs)
	{
		strength = strength * 255.0 / ad->max_strength;
		snr = snr * 255.0 / ad->max_snr;
	}
	else
	{
		strength = strength >> 8;
		snr = snr >> 8;
	}
	// keep the assignment at the end for the signal thread to get the right values as no locking is done on the adapter
	ad->snr = snr;
	ad->strength = strength;
	ad->status = status;
	ad->ber = ber;
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
	int fd;
	int i = 0, j = 0;
	adapter *ad;

	for (i = 0; i < MAX_ADAPTERS; i++)
		for (j = 0; j < MAX_ADAPTERS; j++)
		{
			sprintf(buf, "/dev/dvb/adapter%d/frontend%d", i, j);
			fd = open(buf, O_RDONLY | O_NONBLOCK);
			if (fd < 0)
			{
				sprintf(buf, "/dev/dvb/adapter%d/ca%d", i, j);
				fd = open(buf, O_RDONLY | O_NONBLOCK);
			}
			//LOG("testing device %s -> fd: %d",buf,fd);
			if (fd >= 0)
			{
//				if (is_adapter_disabled(na))
//				{
//					na++;
//					continue;
//				}
				if (!a[na])
					a[na] = adapter_alloc();

				ad = a[na];
				ad->pa = i;
				ad->fn = j;

				ad->open = (Open_device) dvb_open_device;
				ad->set_pid = (Set_pid) dvb_set_pid;
				ad->del_filters = (Del_filters) dvb_del_filters;
				ad->commit = (Adapter_commit) dvb_commit;
				ad->tune = (Tune) dvb_tune;
				ad->delsys = (Dvb_delsys) dvb_delsys;
				ad->post_init = NULL;
				ad->close = (Adapter_commit) dvb_close;
				ad->get_signal = (Device_signal) dvb_get_signal;
				ad->type = ADAPTER_DVB;
				close(fd);
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

#endif  // #ifndef DISABLE_LINUXDVB

