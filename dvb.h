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

#ifndef SYS_DVBC2
#define SYS_DVBC2 19 // support for DVB-C2 DD 
#endif

#ifndef SYS_DVBT2
#define SYS_DVBT2 16
#endif


#define SLOF (11700*1000UL)
#define LOF1 (9750*1000UL)
#define LOF2 (10600*1000UL)
#define LOF3 (10750*1000UL)
#define LP_CODERATE_DEFAULT (0)

#define DELSYS 0
#define FREQUENCY 1
#define MODULATION 2
#define INVERSION 3
#define SYMBOL_RATE 4
#define BANDWIDTH 4
#define FEC_INNER 5
#define DSPLPC2 5
#define FEC_LP 6
#define GUARD 7
#define PILOT 7
#define TRANSMISSION 8
#define ROLLOFF 8
#define HIERARCHY 9
#define DSPLPT2 10

#ifndef DTV_STREAM_ID
#define DTV_STREAM_ID           42
#endif

#define MIN_FRQ_DVBT  174000
#define MAX_FRQ_DVBT  858000
#define MIN_FRQ_DVBC   51000
#define MAX_FRQ_DVBC  860000
#define MIN_FRQ_DVBS  950000
#define MAX_FRQ_DVBS 2150000

typedef struct struct_transponder
{
	fe_delivery_system_t sys;
	int freq;
	int inversion;
	int mtype;
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
	int plts;
	int fec;
	int sr;
	int pol;
	int diseqc;

#define SWITCH_UNICABLE 1
#define SWITCH_JESS 2	
	int switch_type; // tuner type 
	int uslot; // unicable/jess slot
	int ufreq; // unicable/jess frequency
	int pin;
	int committed_no, uncommitted_no; //diseqc informations
	int old_pol, old_hiband, old_diseqc; // used to cache the diseqc position
	
	// DVB-C2
	int c2tft;
	int ds;
	int plp;
	
	char *apids, *pids, *dpids, *x_pmt;
} transponder;

#define MAX_PIDS 64

//int tune_it(int fd_frontend, unsigned int freq, unsigned int srate, char pol, int tone, fe_spectral_inversion_t specInv, unsigned char diseqc,fe_modulation_t modulation,fe_code_rate_t HP_CodeRate,fe_transmit_mode_t TransmissionMode,fe_guard_interval_t guardInterval, fe_bandwidth_t bandwidth);
int tune_it_s2 (int fd_frontend, transponder * tp);

fe_delivery_system_t dvb_delsys (int aid, int fd, fe_delivery_system_t *sys);
int detect_dvb_parameters (char *s, transponder * tp);
void init_dvb_parameters (transponder * tp);
void copy_dvb_parameters (transponder * s, transponder * d);
void get_signal (int fd, fe_status_t * status, uint32_t * ber,
	uint16_t * strength, uint16_t * snr);
int get_signal_new (int fd, fe_status_t * status, uint32_t * ber,
	uint16_t * strength, uint16_t * snr);

char *get_pilot(int i);
char *get_rolloff(int i);
char *get_delsys(int i);
char *get_fec(int i);
char *get_modulation(int i);
char *get_tmode(int i);
char *get_gi(int i);
char *get_specinv(int i);
char *get_pol(int i);

#endif							 /*  */
