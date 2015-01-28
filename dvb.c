/*

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
   Or, point your browser to http://www.gnu.org/copyleft/gpl.html

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
#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>
#include <fcntl.h>
#include <ctype.h>
#include "dvb.h"
#include "minisatip.h"

extern struct struct_opts opts;

struct diseqc_cmd
{
	struct dvb_diseqc_master_cmd cmd;
	uint32_t wait;
};

static const char *fe_pilot_tab[] =
{
	"PILOT_ON",
	"PILOT_OFF",
	"PILOT_AUTO",
};

static const char *fe_rolloff_tab[] =
{
	"ROLLOFF_35",
	"ROLLOFF_20",
	"ROLLOFF_25",
	"ROLLOFF_AUTO"
};

static char *fe_delivery_system_tab[] =
{
	"undefined",
	"dvbc",
	"SYS_DVBC_ANNEX_B",
	"dvbt",
	"dss",
	"dvbs",
	"dvbs2",
	"dvbh",
	"isdbt",
	"isdbs",
	"isdbc",
	"atsc",
	"atscmh",
	"dmbth",
	"cmmb",
	"dab",
	"dvbt2",
	"turbo",
	"dvbc_annex_c"
};

static const char *fe_spectral_inversion_tab[] =
{
	"INVERSION_OFF",
	"INVERSION_ON",
	"INVERSION_AUTO"
};

static const char *fe_code_rate_tab[] =
{
	"FEC_NONE",
	"FEC_1_2",
	"FEC_2_3",
	"FEC_3_4",
	"FEC_4_5",
	"FEC_5_6",
	"FEC_6_7",
	"FEC_7_8",
	"FEC_8_9",
	"FEC_AUTO",
	"FEC_3_5",
	"FEC_9_10",
};

static char *fe_modulation_tab[] =
{
	"qpsk",
	"16qam",
	"32qam",
	"64qam",
	"128qam",
	"256qam",
	"auto_qam",
	"8vsb",
	"16vsb",
	"8psk",
	"16apsk",
	"32apsk",
	"dqpsk"
};

static const char *fe_transmit_mode_tab[] =
{
	"TRANSMISSION_MODE_2K",
	"TRANSMISSION_MODE_8K",
	"TRANSMISSION_MODE_AUTO",
	"TRANSMISSION_MODE_4K",
	"TRANSMISSION_MODE_1K",
	"TRANSMISSION_MODE_16K",
	"TRANSMISSION_MODE_32K"
};

static const char *fe_bandwidth_tab[] =
{
	"BANDWIDTH_8_MHZ",
	"BANDWIDTH_7_MHZ",
	"BANDWIDTH_6_MHZ",
	"BANDWIDTH_AUTO",
	"BANDWIDTH_5_MHZ",
	"BANDWIDTH_10_MHZ",
	"BANDWIDTH_1_712_MHZ",
};

static const char *fe_guard_interval_tab[] =
{
	"GUARD_INTERVAL_1_32",
	"GUARD_INTERVAL_1_16",
	"GUARD_INTERVAL_1_8",
	"GUARD_INTERVAL_1_4",
	"GUARD_INTERVAL_AUTO",
	"GUARD_INTERVAL_1_128",
	"GUARD_INTERVAL_19_128",
	"GUARD_INTERVAL_19_256"
};

static const char *fe_hierarchy_tab[] =
{
	"HIERARCHY_NONE",
	"HIERARCHY_1",
	"HIERARCHY_2",
	"HIERARCHY_4",
	"HIERARCHY_AUTO"
};

struct diseqc_cmd committed_switch_cmds[] = {
	{ { { 0xe0, 0x10, 0x38, 0xf0, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x38, 0xf2, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x38, 0xf1, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x38, 0xf3, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x38, 0xf4, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x38, 0xf6, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x38, 0xf5, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x38, 0xf7, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x38, 0xf8, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x38, 0xfa, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x38, 0xf9, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x38, 0xfb, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x38, 0xfc, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x38, 0xfe, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x38, 0xfd, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x38, 0xff, 0x00, 0x00 }, 4 }, 20 }
};

struct diseqc_cmd uncommitted_switch_cmds[] = {
	{ { { 0xe0, 0x10, 0x39, 0xf0, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x39, 0xf1, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x39, 0xf2, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x39, 0xf3, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x39, 0xf4, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x39, 0xf5, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x39, 0xf6, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x39, 0xf7, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x39, 0xf8, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x39, 0xf9, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x39, 0xfa, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x39, 0xfb, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x39, 0xfc, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x39, 0xfd, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x39, 0xfe, 0x00, 0x00 }, 4 }, 20 },
	{ { { 0xe0, 0x10, 0x39, 0xff, 0x00, 0x00 }, 4 }, 20 }
};
/*--------------------------------------------------------------------------*/

static inline void
msleep (uint32_t msec)
{
	struct timespec req = { msec / 1000, 1000000 * (msec % 1000) };

	while (nanosleep (&req, &req))
		;
}


#if 0
#define DISEQC_X 2
int
rotor_command (int frontend_fd, int cmd, int n1, int n2, int n3)
{
	int err;

        struct dvb_diseqc_master_cmd cmds[] = {
                { { 0xe0, 0x31, 0x60, 0x00, 0x00, 0x00 }, 3 },  //0 Stop Positioner movement
                { { 0xe0, 0x31, 0x63, 0x00, 0x00, 0x00 }, 3 },  //1 Disable Limits
                { { 0xe0, 0x31, 0x66, 0x00, 0x00, 0x00 }, 3 },  //2 Set East Limit
                { { 0xe0, 0x31, 0x67, 0x00, 0x00, 0x00 }, 3 },  //3 Set West Limit
                { { 0xe0, 0x31, 0x68, 0x00, 0x00, 0x00 }, 4 },  //4 Drive Motor East continously
                { { 0xe0, 0x31, 0x68,256-n1,0x00, 0x00 }, 4 },  //5 Drive Motor East nn steps
                { { 0xe0, 0x31, 0x69,256-n1,0x00, 0x00 }, 4 },  //6 Drive Motor West nn steps
                { { 0xe0, 0x31, 0x69, 0x00, 0x00, 0x00 }, 4 },  //7 Drive Motor West continously
                { { 0xe0, 0x31, 0x6a, n1, 0x00, 0x00 }, 4 },  //8 Store nn
                { { 0xe0, 0x31, 0x6b, n1, 0x00, 0x00 }, 4 },   //9 Goto nn
                { { 0xe0, 0x31, 0x6f, n1, n2, n3 }, 4}, //10 Recalculate Position
                { { 0xe0, 0x31, 0x6a, 0x00, 0x00, 0x00 }, 4 },  //11 Enable Limits
                { { 0xe0, 0x31, 0x6e, n1, n2, 0x00 }, 5 },   //12 Gotoxx
                { { 0xe0, 0x10, 0x38, 0xF4, 0x00, 0x00 }, 4 }    //13 User
        };
	int i;

	for (i = 0; i < DISEQC_X; ++i)
	{
		usleep (15 * 1000);
		if (err = ioctl (frontend_fd, FE_DISEQC_SEND_MASTER_CMD, &cmds[cmd]))
			error ("rotor_command: FE_DISEQC_SEND_MASTER_CMD failed, err=%i\n",
				err);
	}
	return err;
}


int
rotate_rotor (int frontend_fd, int from_rotor_pos, int to_rotor_pos,
int voltage_18, int hiband)
{
	/* Rotate a DiSEqC 1.2 rotor from position from_rotor_pos to position to_rotor_pos */
	/* Uses Goto nn (command 9) */
	float rotor_wait_time;		 //seconds
	int err = 0;
	float speed_13V = 1.5;		 //degrees per second
	float speed_18V = 2.4;		 //degrees per second
	float degreesmoved,
		a1,
		a2;

	if (to_rotor_pos != 0)
	{
		if (from_rotor_pos != to_rotor_pos)
		{
			info ("Moving rotor from position %i to position %i\n",
				from_rotor_pos, to_rotor_pos);
			if (from_rotor_pos == 0)
			{
								 // starting from unknown position
				rotor_wait_time = 15;
			}
			else
			{
				a1 = rotor_angle (to_rotor_pos);
				a2 = rotor_angle (from_rotor_pos);
				degreesmoved = abs (a1 - a2);
				if (degreesmoved > 180)
					degreesmoved = 360 - degreesmoved;
				rotor_wait_time = degreesmoved / speed_18V;
			}
			//switch tone off
			if (err = ioctl (frontend_fd, FE_SET_TONE, SEC_TONE_OFF))
				return err;
			msleep (15);
			// high voltage for high speed rotation
			if (err = ioctl (frontend_fd, FE_SET_VOLTAGE, SEC_VOLTAGE_18))
				return err;
			msleep (15);
			err = rotor_command (frontend_fd, 9, to_rotor_pos, 0, 0);
			if (err)
			{
				info ("Rotor move error!\n");
			}
			else
			{
				int i;

				info ("Rotating");
				for (i = 0; i < 10; i++)
				{
					usleep (rotor_wait_time * 100000);
					info (".");
				}
				info ("completed.\n");
			}
		}
		else
		{
			info ("Rotor already at position %i\n", from_rotor_pos);
		}
		// correct tone and voltage
		if (err =
			ioctl (frontend_fd, FE_SET_TONE,
			hiband ? SEC_TONE_ON : SEC_TONE_OFF))
			return err;
		msleep (15);
		if (err = ioctl (frontend_fd, FE_SET_VOLTAGE, voltage_18))
			return err;
		msleep (15);
	}
	return err;
}
#endif

int
diseqc_send_msg (int fd, fe_sec_voltage_t v, struct diseqc_cmd **cmd,
fe_sec_tone_mode_t t, fe_sec_mini_cmd_t b)
{
	int err;

	if ((err = ioctl (fd, FE_SET_TONE, SEC_TONE_OFF)))
		return err;

	if ((err = ioctl (fd, FE_SET_VOLTAGE, v)))
		return err;

	msleep (15);

	while (*cmd)
	{
		//            fprintf(stderr,"DiSEqC: %02x %02x %02x %02x %02x %02x\n",
		//                    (*cmd)->cmd.msg[0], (*cmd)->cmd.msg[1],
		//                    (*cmd)->cmd.msg[2], (*cmd)->cmd.msg[3],
		//                    (*cmd)->cmd.msg[4], (*cmd)->cmd.msg[5]);

		if ((err = ioctl (fd, FE_DISEQC_SEND_MASTER_CMD, &(*cmd)->cmd)))
			return err;

		//              msleep((*cmd)->wait);
		cmd++;
	}

	//fprintf(stderr," %s ", v == SEC_VOLTAGE_13 ? "SEC_VOLTAGE_13" :
	//    v == SEC_VOLTAGE_18 ? "SEC_VOLTAGE_18" : "???");

	//fprintf(stderr," %s ", b == SEC_MINI_A ? "SEC_MINI_A" :
	//    b == SEC_MINI_B ? "SEC_MINI_B" : "???");

	//fprintf(stderr," %s\n", t == SEC_TONE_ON ? "SEC_TONE_ON" :
	//    t == SEC_TONE_OFF ? "SEC_TONE_OFF" : "???");

	msleep (15);

	if ((err = ioctl (fd, FE_DISEQC_SEND_BURST, b)))
		return err;

	msleep (15);

	err = ioctl (fd, FE_SET_TONE, t);

	msleep (15);

	return err;
}


int
setup_switch (int frontend_fd, int switch_pos, int voltage_18, int hiband,
int uncommitted_switch_pos)
{
	int i;
	int err;
	struct diseqc_cmd *cmd[2] = { NULL, NULL };

	i = uncommitted_switch_pos;

	//      fprintf(stderr,"DiSEqC: uncommitted switch pos %i\n", uncommitted_switch_pos);
	if (i < 0
		|| i >=
		(int) (sizeof (uncommitted_switch_cmds) / sizeof (struct diseqc_cmd)))
		return -EINVAL;

	cmd[0] = &uncommitted_switch_cmds[i];

	diseqc_send_msg (frontend_fd,
		voltage_18 ? SEC_VOLTAGE_18 : SEC_VOLTAGE_13,
		cmd,
		hiband ? SEC_TONE_ON : SEC_TONE_OFF,
		switch_pos % 2 ? SEC_MINI_B : SEC_MINI_A);

	i = 4 * switch_pos + 2 * hiband + (voltage_18 ? 1 : 0);

	//      fprintf(stderr,"DiSEqC: switch pos %i, %sV, %sband (index %d)\n",
	//              switch_pos, voltage_18 ? "18" : "13", hiband ? "hi" : "lo", i);

	if (i < 0
		|| i >=
		(int) (sizeof (committed_switch_cmds) / sizeof (struct diseqc_cmd)))
		return -EINVAL;

	cmd[0] = &committed_switch_cmds[i];

	err = diseqc_send_msg (frontend_fd,
		voltage_18 ? SEC_VOLTAGE_18 : SEC_VOLTAGE_13,
		cmd,
		hiband ? SEC_TONE_ON : SEC_TONE_OFF,
		switch_pos % 2 ? SEC_MINI_B : SEC_MINI_A);

	return err;
}


int
tune_it_s2 (int fd_frontend, transponder * tp)
{
	uint32_t if_freq = 0;
	int hiband = 0;
	int res;

	static int uncommitted_switch_pos = 0;
	struct dvb_frontend_event event;
	
	int freq = tp->freq;
	int pol = tp->pol;
	int diseqc = (tp->diseqc == -1) ? 1 : tp->diseqc;
	struct dtv_properties *p;
	struct dvb_frontend_event ev;

	struct dtv_property p_clear[] =
	{
		{.cmd = DTV_CLEAR},
	};

	struct dtv_properties cmdseq_clear =
	{
		.num = 1,
		.props = p_clear
	};
	#ifndef DTV_STREAM_ID
	#define DTV_STREAM_ID           42
	#endif
	static struct dtv_property dvbs2_cmdargs[] =
	{
		{.cmd = DTV_DELIVERY_SYSTEM,.u.data = 0},
		{.cmd = DTV_FREQUENCY,.u.data = 0},
		{.cmd = DTV_MODULATION,.u.data = 0},
		{.cmd = DTV_INVERSION,.u.data = 0},
		{.cmd = DTV_SYMBOL_RATE,.u.data = 0},
		{.cmd = DTV_INNER_FEC,.u.data = 0},
		{.cmd = DTV_PILOT,.u.data = 0},
		{.cmd = DTV_ROLLOFF,.u.data = 0},
		{.cmd = DTV_STREAM_ID,.u.data = 0},
		{.cmd = DTV_TUNE},
	};
	static struct dtv_properties dvbs2_cmdseq =
	{
		.num = sizeof (dvbs2_cmdargs) / sizeof (struct dtv_property),
		.props = dvbs2_cmdargs
	};

	static struct dtv_property dvbt_cmdargs[] =
	{
		{.cmd = DTV_DELIVERY_SYSTEM,.u.data = 0},
		{.cmd = DTV_FREQUENCY,.u.data = 0},
		{.cmd = DTV_MODULATION,.u.data = 0},
		{.cmd = DTV_INVERSION,.u.data = 0},
		{.cmd = DTV_BANDWIDTH_HZ,.u.data = 0},
		{.cmd = DTV_CODE_RATE_HP,.u.data = 0},
		{.cmd = DTV_CODE_RATE_LP,.u.data = 0},
		{.cmd = DTV_GUARD_INTERVAL,.u.data = 0},
		{.cmd = DTV_TRANSMISSION_MODE,.u.data = 0},
		{.cmd = DTV_HIERARCHY,.u.data = HIERARCHY_AUTO},
		{.cmd = DTV_TUNE},
	};
	static struct dtv_properties dvbt_cmdseq =
	{
		.num = sizeof (dvbt_cmdargs) / sizeof (struct dtv_property),
		.props = dvbt_cmdargs
	};
	
	static struct dtv_property dvbc_cmdargs[] = {
    { .cmd = DTV_DELIVERY_SYSTEM, .u.data = SYS_DVBC_ANNEX_A },
    { .cmd = DTV_FREQUENCY,       .u.data = 0 },
    { .cmd = DTV_MODULATION,      .u.data = QAM_AUTO },
    { .cmd = DTV_INVERSION,       .u.data = INVERSION_AUTO },
    { .cmd = DTV_SYMBOL_RATE,     .u.data = 0 },
    { .cmd = DTV_TUNE },
	};
	
	static struct dtv_properties dvbc_cmdseq = {
    .num = sizeof(dvbc_cmdargs)/sizeof(struct dtv_property),
    .props = dvbc_cmdargs
	};
	//    while(1)  {
	//      if (ioctl(fd_frontend, FE_GET_EVENT, &event) < 0)       //EMPTY THE EVENT QUEUE
	//        break;
	//    }

	if ((ioctl (fd_frontend, FE_SET_PROPERTY, &cmdseq_clear)) == -1)
	{
		LOG ("FE_SET_PROPERTY DTV_CLEAR failed for fd %d: %s", fd_frontend, strerror(errno));
		//        return -1;
	}

	#define DELSYS 0
	#define FREQUENCY 1
	#define MODULATION 2
	#define INVERSION 3
	#define SYMBOL_RATE 4
	#define BANDWIDTH 4
	#define FEC_INNER 5
	#define FEC_LP 6
	#define GUARD 7
	#define PILOT 7
	#define TRANSMISSION 8
	#define ROLLOFF 8
	#define MIS 9
	#define HIERARCHY 9

	switch (tp->sys)
	{
		case SYS_DVBS:
		case SYS_DVBS2:
			/* Voltage-controlled switch */
			hiband = 0;

			if (freq < SLOF)
			{
				if_freq = (freq - LOF1);
				hiband = 0;
			}
			else
			{
				if_freq = (freq - LOF2);
				hiband = 1;
			}
			if (tp->sys == SYS_DVBS2 && tp->mtype == 0)
				tp->mtype = PSK_8;
			if (tp->sys == SYS_DVBS && tp->mtype == 0)
				tp->mtype = QPSK;
			//              LOG("Polarity=%c, diseqc=%d,hiband=%d",pol,diseqc,hiband);
			setup_switch (fd_frontend,
				diseqc - 1,
				(toupper (pol) == 'V' ? 0 : 1),
				hiband, uncommitted_switch_pos);
			p = &dvbs2_cmdseq;
			p->props[DELSYS].u.data = tp->sys;
			p->props[MODULATION].u.data = tp->mtype;
			//              p->props[PILOT].u.data=PILOT_AUTO;
			p->props[PILOT].u.data = tp->plts;
			p->props[ROLLOFF].u.data = ROLLOFF_AUTO;
			p->props[ROLLOFF].u.data = tp->ro;
			//        p->props[MIS].u.data = 0;
			p->props[INVERSION].u.data = tp->inversion;
			p->props[SYMBOL_RATE].u.data = tp->sr;
			p->props[FEC_INNER].u.data = tp->fec;
			p->props[FREQUENCY].u.data = if_freq;
			usleep (50000);
			break;

		case SYS_DVBT:
		case SYS_DVBT2:
			if (tp->sys == SYS_DVBT && tp->mtype == 0)
				tp->mtype = QAM_AUTO;
			if (tp->sys == SYS_DVBT2 && tp->mtype == 0)
				tp->mtype = QAM_AUTO;
			p = &dvbt_cmdseq;
			p->props[DELSYS].u.data = tp->sys;
			p->props[FREQUENCY].u.data = freq * 1000;
			p->props[INVERSION].u.data = tp->inversion;
			p->props[MODULATION].u.data = tp->mtype;
			p->props[BANDWIDTH].u.data = tp->bw;
			p->props[FEC_INNER].u.data = tp->fec;
			p->props[FEC_LP].u.data = tp->fec;
			p->props[GUARD].u.data = tp->gi;
			p->props[TRANSMISSION].u.data = tp->tmode;
			p->props[HIERARCHY].u.data = HIERARCHY_AUTO;
			break;
		case SYS_DVBC_ANNEX_A:
			p = &dvbc_cmdseq;
			if(tp->mtype == 0)
				tp->mtype = QAM_AUTO;
			p->props[DELSYS].u.data = tp->sys;
			p->props[FREQUENCY].u.data = freq * 1000;
			p->props[INVERSION].u.data = tp->inversion;
			p->props[SYMBOL_RATE].u.data = tp->sr;
			p->props[MODULATION].u.data = tp->mtype;
			
			break;

	}

	/* discard stale QPSK events */
	while (1)
	{
		if (ioctl (fd_frontend, FE_GET_EVENT, &ev) == -1)
			break;
	}
	if (tp->sys == SYS_DVBS || tp->sys == SYS_DVBS2)
		LOG
			("tunning to %d(%d) sr:%d fec:%s delsys:%s mod:%s rolloff:%s pilot:%s",
			tp->freq, p->props[FREQUENCY].u.data, p->props[SYMBOL_RATE].u.data, fe_code_rate_tab[p->props[FEC_INNER].u.data],
			fe_delivery_system_tab[p->props[DELSYS].u.data], fe_modulation_tab[p->props[MODULATION].u.data],
			fe_rolloff_tab[p->props[ROLLOFF].u.data], fe_pilot_tab[p->props[PILOT].u.data])
	else if (tp->sys == SYS_DVBT || tp->sys == SYS_DVBT2)
			LOG ("tunning to %d delsys: %s bw:%d inversion:%s mod:%s fec:%s fec_lp:%s guard:%s transmission: %s",
					p->props[FREQUENCY].u.data,
					fe_delivery_system_tab[p->props[DELSYS].u.data],
					p->props[BANDWIDTH].u.data,
					fe_spectral_inversion_tab[p->props[INVERSION].u.data],
					fe_modulation_tab[p->props[MODULATION].u.data],
					fe_code_rate_tab[p->props[FEC_INNER].u.data],
					fe_code_rate_tab[p->props[FEC_LP].u.data],
					fe_guard_interval_tab[p->props[GUARD].u.data],
					fe_transmit_mode_tab[p->props[TRANSMISSION].u.data])
	else if (tp->sys == SYS_DVBC_ANNEX_A) 
			LOG("tunning to %d sr:%d specinv:%s delsys:%s mod:%s", p->props[FREQUENCY].u.data, tp->sr, fe_spectral_inversion_tab[p->props[INVERSION].u.data], fe_delivery_system_tab[p->props[DELSYS].u.data], fe_modulation_tab[p->props[MODULATION].u.data]);

	if ((ioctl (fd_frontend, FE_SET_PROPERTY, p)) == -1)
		if (ioctl (fd_frontend, FE_SET_PROPERTY, p) == -1)
		{
			perror ("FE_SET_PROPERTY TUNE failed");
			LOG ("set property failed");
			return -404;
		}

	return 0;
}


int
set_pid (int hw, int ad, uint16_t i_pid)
{
	char buf[100];
	int fd;

	if ( i_pid > 8191 )
		LOG_AND_RETURN(-1, "pid %d > 8191 for /dev/dvb/adapter%d/demux%d", i_pid, hw, ad);
		
	sprintf (buf, "/dev/dvb/adapter%d/demux%d", hw, ad);
	if ((fd = open (buf, O_RDWR | O_NONBLOCK)) < 0)
	{
		LOG("Could not open demux device /dev/dvb/adapter%d/demux%d: %s ",hw, ad, strerror (errno));
		return -1;
	}

	struct dmx_pes_filter_params s_filter_params;

	s_filter_params.pid = i_pid;
	s_filter_params.input = DMX_IN_FRONTEND;
	s_filter_params.output = DMX_OUT_TS_TAP;
	s_filter_params.flags = DMX_IMMEDIATE_START;
	s_filter_params.pes_type = DMX_PES_OTHER;

	if (ioctl (fd, DMX_SET_PES_FILTER, &s_filter_params) < 0)
	{
		LOG ("failed setting filter on %d (%s)", i_pid, strerror (errno));
		return -1;
	}

	LOG ("setting filter on PID %d for fd %d", i_pid, fd);

	return fd;
}


int del_filters (int fd, int pid)
{
	if (fd < 0)
		LOG_AND_RETURN(0, "DMX_STOP on an invalid handle %d, pid %d", fd, pid);
	if (ioctl (fd, DMX_STOP) < 0)
		LOG ("DMX_STOP failed on PID %d FD %d: %s", pid, fd, strerror (errno))
			else
			LOG ("clearing filters on PID %d FD %d", pid, fd);
	close (fd);
	return 0;
}


fe_delivery_system_t
dvb_delsys (int aid, int fd, fe_delivery_system_t *sys)
{
	int i, res;
	struct dvb_frontend_info fe_info;

	static struct dtv_property enum_cmdargs[] =
	{
		{.cmd = DTV_ENUM_DELSYS,.u.data = 0},
	};
	static struct dtv_properties enum_cmdseq =
	{
		.num = sizeof (enum_cmdargs) / sizeof (struct dtv_property),
		.props = enum_cmdargs
	};
	
	for(i = 0 ; i < 10 ; i ++)
		sys[i] = 0;
	
	if (ioctl (fd, FE_GET_PROPERTY, &enum_cmdseq) < 0)
	{
		LOG ("unable to query frontend");
		return 0;
	}

	if ((res = ioctl (fd, FE_GET_INFO, &fe_info) < 0))
	{
		LOG ("FE_GET_INFO failed for adapter %d, fd %d: %s ", aid, fd, strerror(errno));
		//       return -1;
	}
	
	LOG("Detected Adapter %d handle %d DVB Card Name: %s", aid, fd, fe_info.name); 

	int nsys = enum_cmdargs[0].u.buffer.len;

	if (nsys < 1)
	{
		LOG ("no available delivery system");
		return 0;
	}
	
	for (i = 0; i < nsys; i++)
	{
		sys[i] = enum_cmdargs[0].u.buffer.data[i];
		LOG("Detected del_sys[%d] for adapter %d: %s", i, aid, fe_delivery_system_tab[sys[i]]);
	}
	int rv = enum_cmdargs[0].u.buffer.data[0];

	LOG ("returning default from dvb_delsys => %s (count %d)", fe_delivery_system_tab[rv] , nsys);
	return (fe_delivery_system_t) rv;

}


#define INVALID_URL(a) {LOG(a);return 0;}
char def_pids[100];

#define default_pids "0,1,2,3"

int
detect_dvb_parameters (char *s, transponder * tp)
{
	char *arg[20];
	int la, i;

	tp->sys = -1;
	tp->freq = -1;
	tp->inversion = -1;
	tp->mod = -1;
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
	tp->pids = tp->apids = tp->dpids = NULL;

	while (*s > 0 && *s != '?')
		s++;

	if (*s == 0)
		LOG_AND_RETURN (0, "no ? found in URL");

	*s++;
	if (strstr(s, "freq="))
			init_dvb_parameters(tp);

	LOG ("detect_dvb_parameters (S)-> %s", s);
	la = split (arg, s, 20, '&');

	pchar_int mod[] =
	{
		{"dvbs2", SYS_DVBS2}
		, {"dvbs", SYS_DVBS}
		, {"dvbt2", SYS_DVBT2}
		, {"dvbt", SYS_DVBT}
		, {"dvbc", SYS_DVBC_ANNEX_A}
		, {NULL, 0}
	};
	pchar_int ro[] =
	{
		{"0.35", ROLLOFF_35}
		, {"0.25", ROLLOFF_25}
		, {"0.20", ROLLOFF_20}
		, {NULL, 0}
	};
	pchar_int mtype[] =
	{
		{"qpsk", QPSK}
		, {"8psk", PSK_8}
		, {"16qam", QAM_16}
		, {"32qam", QAM_32}
		, {"64qam", QAM_64}
		, {"128qam", QAM_128}
		, {"256qam", QAM_256}
		, {NULL, 0}
	};
	pchar_int fec[] =
	{
		{"12", FEC_1_2}
		, {"23", FEC_2_3}
		, {"34", FEC_3_4}
		, {"45", FEC_4_5}
		, {"56", FEC_5_6}
		, {"67", FEC_6_7}
		, {"78", FEC_7_8}
		, {"89", FEC_8_9}
		, {"35", FEC_3_5}
		, {"910", FEC_9_10}
		, {NULL, 0}
	};
	pchar_int plts[] =
	{
		{"on", PILOT_ON}
		, {"off", PILOT_OFF}
		, {NULL, 0}
	};
	pchar_int gi[] =
	{
		{"14", GUARD_INTERVAL_1_4}
		, {"18", GUARD_INTERVAL_1_8}
		, {"116", GUARD_INTERVAL_1_16}
		, {"132", GUARD_INTERVAL_1_32}
		, {"1128", GUARD_INTERVAL_1_128}
		, {"19128", GUARD_INTERVAL_19_128}
		, {"19256", GUARD_INTERVAL_19_256}
		, {NULL, 0}
	};
	pchar_int tmode[] =
	{
		{"2k", TRANSMISSION_MODE_2K}
		, {"4k", TRANSMISSION_MODE_4K}
		, {"8k", TRANSMISSION_MODE_8K}
		, {"1k", TRANSMISSION_MODE_1K}
		, {"16k", TRANSMISSION_MODE_16K}
		, {"32k", TRANSMISSION_MODE_32K}
		, {NULL, 0}
	};
	//      pchar_int bw[]={{"5",BANDWIDTH_5_MHZ},{"6",BANDWIDTH_6_MHZ},{"7",BANDWIDTH_7_MHZ},{"8",BANDWIDTH_8_MHZ},{"10",BANDWIDTH_10_MHZ},{"1.712",BANDWIDTH_1_712_MHZ},{NULL,0}};
	for (i = 0; i < la; i++)
	{
		if (strncmp ("msys=", arg[i], 5) == 0)
			tp->sys = map_int (arg[i] + 5, mod);
		if (strncmp ("freq=", arg[i], 5) == 0)
			tp->freq = map_float (arg[i] + 5, 1000);
		if (strncmp ("pol=", arg[i], 4) == 0)
			tp->pol = toupper (arg[i][4]);
		if (strncmp ("sr=", arg[i], 3) == 0)
			tp->sr = map_int (arg[i] + 3, NULL) * 1000;
		if (strncmp ("fe=", arg[i], 3) == 0)
			tp->fe = map_int (arg[i] + 3, NULL);
		if (strncmp ("src=", arg[i], 4) == 0)
			tp->diseqc = map_int (arg[i] + 4, NULL);
		if (strncmp ("ro=", arg[i], 3) == 0)
			tp->ro = map_int (arg[i] + 3, ro);
		if (strncmp ("mtype=", arg[i], 6) == 0)
			tp->mtype = map_int (arg[i] + 6, mtype);
		if (strncmp ("fec=", arg[i], 4) == 0)
			tp->fec = map_int (arg[i] + 4, fec);
		if (strncmp ("plts=", arg[i], 5) == 0)
			tp->plts = map_int (arg[i] + 5, plts);
		if (strncmp ("gi=", arg[i], 3) == 0)
			tp->gi = map_int (arg[i] + 3, gi);
		if (strncmp ("tmode=", arg[i], 6) == 0)
			tp->tmode = map_int (arg[i] + 6, tmode);
		if (strncmp ("bw=", arg[i], 3) == 0)
			tp->bw = map_float (arg[i] + 3, 1000000);
		if (strncmp ("specinv=", arg[i], 8) == 0)
			tp->inversion = map_int (arg[i] + 8, NULL);
		if (strncmp ("pids=", arg[i], 5) == 0)
			tp->pids = arg[i] + 5;
		if (strncmp ("addpids=", arg[i], 6) == 0)
			tp->apids = arg[i] + 8;
		if (strncmp ("delpids=", arg[i], 6) == 0)
			tp->dpids = arg[i] + 8;

	}
		
	if (tp->pids && strncmp (tp->pids, "all", 3) == 0)
	{
		strcpy (def_pids, default_pids);
								 // map pids=all to essential pids
		tp->pids = (char *) def_pids;
	}
	
	if (tp->pids && strncmp (tp->pids, "none", 3) == 0)
		tp->pids = NULL;

	//      if(!msys)INVALID_URL("no msys= found in URL");
	//      if(freq<10)INVALID_URL("no freq= found in URL or frequency invalid");
	//      if((msys==SYS_DVBS || msys==SYS_DVBS2) && (pol!='H' && pol!='V'))INVALID_URL("no pol= found in URL or pol is not H or V");
	LOG
		("detect_dvb_parameters (E) -> src=%d, fe=%d, freq=%d, sr=%d, pol=%d, ro=%d, msys=%d, mtype=%d, pltf=%d, bw=%d, inv=%d, pids=%s - apids=%s - dpids=%s",
		tp->diseqc, tp->fe, tp->freq, tp->sr, tp->pol, tp->ro, tp->sys,
		tp->mtype, tp->plts, tp->bw, tp->inversion, tp->pids ? tp->pids : "NULL",
		tp->apids ? tp->apids : "NULL", tp->dpids ? tp->dpids : "NULL");
	return 0;
}


void
init_dvb_parameters (transponder * tp)
{
	tp->sys = 0;
	tp->freq = 0;
	tp->inversion = INVERSION_AUTO;
	tp->mod = 0;
	tp->hprate = FEC_AUTO;
	tp->tmode = TRANSMISSION_MODE_AUTO;
	tp->gi = GUARD_INTERVAL_AUTO;
	tp->bw = 8000000;
	tp->sm = 0;
	tp->t2id = 0;
	tp->fe = 0;
	tp->ro = ROLLOFF_AUTO;
	tp->mtype = QPSK;
	tp->plts = PILOT_AUTO;
	tp->fec = FEC_AUTO;
	tp->sr = 0;
	tp->pol = 0;
	tp->diseqc = 0;

	tp->pids = tp->apids = tp->dpids = NULL;
}


void
copy_dvb_parameters (transponder * s, transponder * d)
{
	LOG
		("copy_dvb_param start -> src=%d, fe=%d, freq=%d, sr=%d, pol=%c, ro=%d, msys=%d, mtype=%d, pltf=%d, bw=%d, inv=%d, pids=%s, apids=%s, dpids=%s",
		d->diseqc, d->fe, d->freq, d->sr, d->pol, d->ro, d->sys, d->mtype,
		d->plts, d->bw, d->inversion, d->pids ? d->pids : "NULL",
		d->apids ? d->apids : "NULL", d->dpids ? d->dpids : "NULL");
	if (s->sys != -1)
		d->sys = s->sys;
	if (s->freq != -1)
		d->freq = s->freq;
	if (s->inversion != -1)
		d->inversion = s->inversion;
	if (s->mod != -1)
		d->mod = s->mod;
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

	//      if(s->apids)
	d->apids = s->apids;
	//      if(s->pids)
	d->pids = s->pids;
	//      if(s->dpids)
	d->dpids = s->dpids;

	LOG
		("copy_dvb_parameters -> src=%d, fe=%d, freq=%d, sr=%d, pol=%c, ro=%d, msys=%d, mtype=%d, pltf=%d, bw=%d, inv=%d, pids=%s, apids=%s, dpids=%s",
		d->diseqc, d->fe, d->freq, d->sr, d->pol, d->ro, d->sys, d->mtype,
		d->plts, d->bw, d->inversion, d->pids ? d->pids : "NULL",
		d->apids ? d->apids : "NULL", d->dpids ? d->dpids : "NULL");
}


void
get_signal (int fd, fe_status_t * status, uint32_t * ber, uint16_t * strength,
uint16_t * snr)
{
	*status = *ber = *snr = *strength = 0;

	if (ioctl (fd, FE_READ_STATUS, status) < 0)
	{
		LOG ("ioctl FE_READ_STATUS failed (%s)", strerror (errno));
		*status = -1;
		return;
	}
	*status = (*status & FE_HAS_LOCK) ? 1 : 0;
	if (*status)
	{
		if (ioctl (fd, FE_READ_BER, ber) < 0)
			LOG ("ioctl FE_READ_BER failed (%s)", strerror (errno));

		if (ioctl (fd, FE_READ_SIGNAL_STRENGTH, strength) < 0)
			LOG ("ioctl FE_READ_SIGNAL_STRENGTH failed (%s)", strerror (errno));

		if (ioctl (fd, FE_READ_SNR, snr) < 0)
			LOG ("ioctl FE_READ_SNR failed (%s)", strerror (errno));
	}
}

char *modulation_string(int mtype)
{
	if(mtype>=0 && mtype < sizeof(fe_modulation_tab) )
		return fe_modulation_tab[mtype];
	return "none";
}


char *delsys_string(int delsys)
{
	if(delsys>=0 && delsys < sizeof(fe_delivery_system_tab) )
		return fe_delivery_system_tab[delsys];
	return "none";
}
