#ifndef DVB_H
#define DVB_H
#include <linux/dvb/frontend.h>
#include <linux/dvb/version.h>

#define DVBAPIVERSION (DVB_API_VERSION << 8 | DVB_API_VERSION_MINOR)
#if DVBAPIVERSION < 0x0500
#error minisatip requires Linux DVB driver API version 5.0 or higher!
#endif

#if DVBAPIVERSION < 0x0505
#define DTV_ENUM_DELSYS 44
#define SYS_DVBC_ANNEX_A       SYS_DVBC_ANNEX_AC
#endif


#define SLOF (11700*1000UL)
#define LOF1 (9750*1000UL)
#define LOF2 (10600*1000UL)
#define LP_CODERATE_DEFAULT (0)

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

#ifndef DTV_STREAM_ID
#define DTV_STREAM_ID           42
#endif


typedef struct struct_transponder
{
	fe_delivery_system_t sys;
	int freq;
	int inversion;
	int mod;
	int fe;

	// DVB-T
	int hprate;
	int tmode;
	int gi;
	int bw;
	int sm;
	int t2id;

	// DVB-S2
	int ro;
	int mtype;
	int plts;
	int fec;
	int sr;
	int pol;
	int diseqc;
	char *apids,
		*pids,
		*dpids;
} transponder;

#define MAX_PIDS 64

//int tune_it(int fd_frontend, unsigned int freq, unsigned int srate, char pol, int tone, fe_spectral_inversion_t specInv, unsigned char diseqc,fe_modulation_t modulation,fe_code_rate_t HP_CodeRate,fe_transmit_mode_t TransmissionMode,fe_guard_interval_t guardInterval, fe_bandwidth_t bandwidth);
int tune_it_s2 (int fd_frontend, transponder * tp);
int set_pid (int hw, int ad, uint16_t i_pid);
int del_filters (int fd, int pid);
fe_delivery_system_t dvb_delsys (int aid, int fd, fe_delivery_system_t *sys);
int detect_dvb_parameters (char *s, transponder * tp);
void init_dvb_parameters (transponder * tp);
void copy_dvb_parameters (transponder * s, transponder * d);
void get_signal (int fd, fe_status_t * status, uint32_t * ber,
uint16_t * strength, uint16_t * snr);
char *modulation_string(int mtype);
char *delsys_string(int delsys);
#endif							 /*  */
