#ifndef DVB_H
#define DVB_H

#ifdef __APPLE__
#include <sys/types.h>
#else
#include <linux/types.h>
#include <stdint.h>
#endif

#ifdef DISABLE_LINUXDVB
// #include <linux/types.h>
#include <stdint.h>
#include <time.h>
#define DVBAPIVERSION 0x0500
#define LOGDVBAPIVERSION 0x0000
#endif

#ifndef DISABLE_LINUXDVB
#include <linux/dvb/ca.h>
#include <linux/dvb/dmx.h>
#include <linux/dvb/frontend.h>
#include <linux/dvb/version.h>
#define DVBAPIVERSION (DVB_API_VERSION << 8 | DVB_API_VERSION_MINOR)
#define LOGDVBAPIVERSION DVBAPIVERSION
#endif

#if DVBAPIVERSION < 0x0500
#error minisatip requires Linux DVB driver API version 5.0 or higher!
#endif

#ifdef DISABLE_LINUXDVB
typedef enum fe_delivery_system {
    SYS_UNDEFINED,
    SYS_DVBC_ANNEX_AC,
    SYS_DVBC_ANNEX_B,
    SYS_DVBT,
    SYS_DSS,
    SYS_DVBS,
    SYS_DVBS2,
    SYS_DVBH,
    SYS_ISDBT,
    SYS_ISDBS,
    SYS_ISDBC,
    SYS_ATSC,
    SYS_ATSCMH,
    SYS_DMBTH,
    SYS_CMMB,
    SYS_DAB,
    SYS_DVBT2,
    SYS_TURBO,
    SYS_DVBC_ANNEX_C,
    SYS_DVBC2,
} fe_delivery_system_t;
typedef enum fe_status {
    FE_HAS_SIGNAL = 0x01,
    FE_HAS_CARRIER = 0x02,
    FE_HAS_VITERBI = 0x04,
    FE_HAS_SYNC = 0x08,
    FE_HAS_LOCK = 0x10,
    FE_TIMEDOUT = 0x20,
    FE_REINIT = 0x40
} fe_status_t;
typedef enum fe_code_rate {
    FEC_NONE = 0,
    FEC_1_2,
    FEC_2_3,
    FEC_3_4,
    FEC_4_5,
    FEC_5_6,
    FEC_6_7,
    FEC_7_8,
    FEC_8_9,
    FEC_AUTO,
    FEC_3_5,
    FEC_9_10,
    FEC_2_5,
    FEC_1_4,
    FEC_1_3,
} fe_code_rate_t;
typedef enum fe_rolloff {
    ROLLOFF_35,
    ROLLOFF_20,
    ROLLOFF_25,
    ROLLOFF_AUTO,
    ROLLOFF_15,
    ROLLOFF_10,
    ROLLOFF_5,
} fe_rolloff_t;
typedef enum fe_pilot {
    PILOT_ON,
    PILOT_OFF,
    PILOT_AUTO,
} fe_pilot_t;
typedef enum fe_bandwidth {
    BANDWIDTH_8_MHZ,
    BANDWIDTH_7_MHZ,
    BANDWIDTH_6_MHZ,
    BANDWIDTH_AUTO,
    BANDWIDTH_5_MHZ,
    BANDWIDTH_10_MHZ,
    BANDWIDTH_1_712_MHZ,
} fe_bandwidth_t;
typedef enum fe_guard_interval {
    GUARD_INTERVAL_1_32,
    GUARD_INTERVAL_1_16,
    GUARD_INTERVAL_1_8,
    GUARD_INTERVAL_1_4,
    GUARD_INTERVAL_AUTO,
    GUARD_INTERVAL_1_128,
    GUARD_INTERVAL_19_128,
    GUARD_INTERVAL_19_256,
} fe_guard_interval_t;
typedef enum fe_spectral_inversion {
    INVERSION_OFF,
    INVERSION_ON,
    INVERSION_AUTO
} fe_spectral_inversion_t;
typedef enum fe_transmit_mode {
    TRANSMISSION_MODE_2K,
    TRANSMISSION_MODE_8K,
    TRANSMISSION_MODE_AUTO,
    TRANSMISSION_MODE_4K,
    TRANSMISSION_MODE_1K,
    TRANSMISSION_MODE_16K,
    TRANSMISSION_MODE_32K,
    TRANSMISSION_MODE_C1,
    TRANSMISSION_MODE_C3780,
} fe_transmit_mode_t;
typedef enum fe_type { FE_QPSK, FE_QAM, FE_OFDM, FE_ATSC } fe_type_t;
typedef enum fe_modulation {
    QPSK,
    QAM_16,
    QAM_32,
    QAM_64,
    QAM_128,
    QAM_256,
    QAM_AUTO,
    VSB_8,
    VSB_16,
    PSK_8,
    APSK_16,
    APSK_32,
    DQPSK,
    QAM_4_NR,
    APSK_64,
    APSK_128,
    APSK_256,
} fe_modulation_t;
#endif

typedef enum fe_pls_mode {
    PLS_MODE_ROOT,
    PLS_MODE_GOLD,
    PLS_MODE_COMBO,
} fe_pls_mode_t;

// dB scale * 10 for Integer conversion
typedef enum dvb_snr_table {
    DVB_C__QAM_64___FEC_NONE = 300,
    DVB_C__QAM_256__FEC_NONE = 340,
    DVB_C__OTHER = 360,

    DVB_S__QPSK__FEC_1_2 = 70,
    DVB_S__QPSK__FEC_2_3 = 90,
    DVB_S__QPSK__FEC_3_4 = 100,
    DVB_S__QPSK__FEC_5_6 = 110,
    DVB_S__QPSK__FEC_7_8 = 120,
    DVB_S__OTHER = 130, /* Calculated */

    DVB_S2_QPSK__FEC_1_2 = 90,
    DVB_S2_QPSK__FEC_2_3 = 110,
    DVB_S2_QPSK__FEC_3_4 = 120,
    DVB_S2_QPSK__FEC_4_5 = 130, /* Calculated */
    DVB_S2_QPSK__FEC_5_6 = 120,
    DVB_S2_QPSK__FEC_8_9 = 130,
    DVB_S2_QPSK__FEC_9_10 = 135,
    DVB_S2_OTHER = 130, /* Calculated */

    DVB_S2_PSK_8_FEC_1_2 = 128, /* Calculated */
    DVB_S2_PSK_8_FEC_2_3 = 145,
    DVB_S2_PSK_8_FEC_3_4 = 160,
    DVB_S2_PSK_8_FEC_4_5 = 180, /* Deduced */
    DVB_S2_PSK_8_FEC_5_6 = 175,
    DVB_S2_PSK_8_FEC_8_9 = 190,
    DVB_S2_PSK_8_FEC_9_10 = 200, /* Deduced */
    DVB_S2_PSK_8_OTHER = 180,    /* Deduced */

    DVB_T__QPSK__FEC_1_2 = 41,
    DVB_T__QPSK__FEC_2_3 = 61,
    DVB_T__QPSK__FEC_3_4 = 72,
    DVB_T__QPSK__FEC_5_6 = 85,
    DVB_T__QPSK__FEC_7_8 = 92,
    DVB_T__QPSK_OTHER = 110,

    DVB_T__QAM16_FEC_1_2 = 98,
    DVB_T__QAM16_FEC_2_3 = 121,
    DVB_T__QAM16_FEC_3_4 = 134,
    DVB_T__QAM16_FEC_5_6 = 148,
    DVB_T__QAM16_FEC_7_8 = 157,
    DVB_T__QAM16_OTHER = 180,

    DVB_T__QAM64_FEC_1_2 = 140,
    DVB_T__QAM64_FEC_2_3 = 199,
    DVB_T__QAM64_FEC_3_4 = 249,
    DVB_T__QAM64_FEC_5_6 = 213,
    DVB_T__QAM64_FEC_7_8 = 220,
    DVB_T__QAM64_OTHER = 250,

    /* Table 2.12 - Memoryless Rayleigh channel K=0 - EBU Tech 3348 */
    DVB_T2_QPSK__FEC_1_2 = 28,
    DVB_T2_QPSK__FEC_3_5 = 47,
    DVB_T2_QPSK__FEC_2_3 = 59,
    DVB_T2_QPSK__FEC_3_4 = 76,
    DVB_T2_QPSK__FEC_4_5 = 89,
    DVB_T2_QPSK__FEC_5_6 = 98,
    DVB_T2_QPSK_OTHER = 110,

    DVB_T2_QAM16_FEC_1_2 = 81,
    DVB_T2_QAM16_FEC_3_5 = 101,
    DVB_T2_QAM16_FEC_2_3 = 115,
    DVB_T2_QAM16_FEC_3_4 = 133,
    DVB_T2_QAM16_FEC_4_5 = 147,
    DVB_T2_QAM16_FEC_5_6 = 156,
    DVB_T2_QAM16_OTHER = 180,

    DVB_T2_QAM64_FEC_1_2 = 126,
    DVB_T2_QAM64_FEC_3_5 = 147,
    DVB_T2_QAM64_FEC_2_3 = 162,
    DVB_T2_QAM64_FEC_3_4 = 182,
    DVB_T2_QAM64_FEC_4_5 = 197,
    DVB_T2_QAM64_FEC_5_6 = 207,
    DVB_T2_QAM64_OTHER = 230,

    DVB_T2_QAM256_FEC_1_2 = 165,
    DVB_T2_QAM256_FEC_3_5 = 190,
    DVB_T2_QAM256_FEC_2_3 = 206,
    DVB_T2_QAM256_FEC_3_4 = 229,
    DVB_T2_QAM256_FEC_4_5 = 246,
    DVB_T2_QAM256_FEC_5_6 = 256,
    DVB_T2_QAM256_OTHER = 280,
} dvb_snr_table_t;

#if DVBAPIVERSION < 0x0505
#define DTV_ENUM_DELSYS 44
#define SYS_DVBC_ANNEX_A SYS_DVBC_ANNEX_AC
#endif

#ifndef SYS_DVBC2
#define SYS_DVBC2 19 // support for DVB-C2 DD
#endif

#ifndef SYS_DVBT2
#define SYS_DVBT2 16
#endif

#define LP_CODERATE_DEFAULT (0)

#ifndef DTV_STREAM_ID
#define DTV_STREAM_ID 42
#endif

#define MAX_PIDS 128
#define MAX_DVBAPI_SYSTEMS 22
#define MAX_STREAMS_PER_PID 16

#define USE_DVR 0   // Always use DVR device for the stream
#define USE_DEMUX 1 // Always use the DEMUX device for the stream
#define USE_PES_FILTERS_AND_DVR                                                \
    2 // Use PES filters for PSI pids (0-32 + pmts) and DVR DEVICE for
      // video/audio data
#define USE_PES_FILTERS_AND_DEMUX                                              \
    3 // Use PES filters for PSI pids (0-32 + pmts) and DEMUX DEVICE for
      // video/audio data - this is how enigma2 get TS data

#define MIN_FRQ_DVBT 174000
#define MAX_FRQ_DVBT 858000
#define MIN_FRQ_DVBC 51000
#define MAX_FRQ_DVBC 860000
#define MIN_FRQ_DVBS 950000
#define MAX_FRQ_DVBS 2150000

#define TP_VALUE_UNSET (-1)

typedef struct diseqc {
#define SWITCH_UNICABLE 1
#define SWITCH_JESS 2
#define SWITCH_SLAVE 3
    int switch_type;
    /* parameters */
    int uslot;          // unicable/jess slot
    int ufreq;          // unicable/jess frequency
    int pin;            // unicable pin code
    int only13v;        // unicable - use 13V voltage only
    int fast;           // don't send diseqc without position change
    int addr;           // diseqc address (second byte in the sequence)
    int committed_no;   // committed switch number
    int uncommitted_no; // uncommitted switch number
    /* timing */
    int before_cmd;
    int after_cmd;
    int after_repeated_cmd;
    int after_switch;
    int after_burst;
    int after_tone;

    int lnb_low, lnb_high, lnb_circular, lnb_switch;
} diseqc;

typedef struct struct_transponder {
    int sys;
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

    struct diseqc diseqc_param;

    int c2tft;    // DVB-C2
    int ds;       // DVB-C2 (data slice)
    int plp_isi;  // DVB-T2/DVB-S2
    int pls_mode; // DVB-S2
    int pls_code; // DVB-S2

    char *apids, *pids, *dpids, *x_pmt;
} transponder;

typedef struct struct_pid {
    int16_t pid;         // pid for this demux - not used
    int fd;              // fd for this demux
    int cc_err, cc_err2; // counter errors
    // stream id - one more to set it -1
    int16_t sid[MAX_STREAMS_PER_PID];
    char flags; // 0 - disabled , 1 enabled, 2 - will be enabled next tune when
                // tune is called, 3 disable when tune is called
    int packets, packets2; // how many packets for this pid arrived, used to
                           // sort the pids
    int dec_err;           // decrypt errors, continuity counters
    uint8_t is_decrypted;  // Set when first decrypted
    int16_t pmt, filter;
    int16_t cc, cc1, cc2;
    int sock; // sock_id
#ifdef CRC_TS
    uint32_t crc;
    int count;
#endif
} SPid;

#ifndef DISABLE_LINUXDVB
// int tune_it(int fd_frontend, unsigned int freq, unsigned int srate, char pol,
// int tone, fe_spectral_inversion_t specInv, unsigned char
// diseqc,fe_modulation_t modulation,fe_code_rate_t
// HP_CodeRate,fe_transmit_mode_t TransmissionMode,fe_guard_interval_t
// guardInterval, fe_bandwidth_t bandwidth);
int tune_it_s2(int fd_frontend, transponder *tp);

fe_delivery_system_t dvb_delsys(int aid, int fd, fe_delivery_system_t *sys);
#endif
int detect_dvb_parameters(char *s, transponder *tp);
void init_dvb_parameters(transponder *tp);
void copy_dvb_parameters(transponder *s, transponder *d);

float get_db_snr_map(transponder *tp);
uint32_t pls_scrambling_index(transponder *tp);

const char *get_pilot(int i);
const char *get_rolloff(int i);
const char *get_delsys(int i);
const char *get_fec(int i);
const char *get_modulation(int i);
const char *get_tmode(int i);
const char *get_gi(int i);
const char *get_pol(int i);
const char *get_inversion(int i);
const char *get_pls_mode(int i);

extern const char *fe_delsys[];
extern const char *fe_fec[];
extern const char *fe_tmode[];
extern const char *fe_modulation[];
extern const char *fe_gi[];

#endif /*  */
