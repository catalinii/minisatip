#ifndef ADAPTER_H
#define ADAPTER_H
#include "minisatip.h"
#include "dvb.h"

typedef struct ca_device ca_device_t;

#define MAX_ADAPTERS 32
#define MAX_SOURCES 64
#define DVR_BUFFER 30 * 1024 * 188

#define ADAPTER_BUFFER 384 * DVB_FRAME // 128 * 3 > 65535
#define RTSP_SETUP 1
#define RTSP_PLAY 2
#define RTSP_OPTIONS 3
#define RTSP_TEARDOWN 4
#define RTSP_DESCRIBE 5

typedef int (*Set_pid)(void *ad, int i_pid);
typedef int (*Del_filters)(void *ad, int fd, int pid);
typedef int (*Adapter_commit)(void *ad);
typedef int (*Open_device)(void *ad);
typedef int (*Device_signal)(void *ad);
typedef int (*Device_wakeup)(void *ad, int fd, int voltage);
typedef int (*Device_standby)(void *ad);
typedef int (*Tune)(int aid, transponder *tp);
typedef uint8_t (*Dvb_delsys)(int aid, int fd,
							  uint8_t *sys);

#define ADAPTER_DVB 1
#define ADAPTER_SATIP 2
#define ADAPTER_NETCV 3

#define MAX_DELSYS 10

typedef struct struct_adapter
{
	char enabled;
	SMutex mutex;
	char type; // available on the system
	int fe, dmx, dvr;
	int pa, fn;
	// flags

	char slow_dev;
	char err; // adapter in an error state (initialized but not working correctly)
	int adapter_timeout;
	char flush, updating_pids;
	// physical adapter, physical frontend number
	uint8_t sys[MAX_DELSYS];
	transponder tp;
	SPid pids[MAX_PIDS];
	int ca_mask;
	int master_sid; // first SID, the one that controls the tuning
	int sid_cnt;	//number of streams
	int sock, fe_sock;
	int do_tune;
	int force_close;
	unsigned char *buf; // 7 rtp packets = MAX_PACK, 7 frames / packet
	int64_t rtime;
	int64_t last_sort;
	int new_gs;
	int status, status_cnt, fast_status;
	int dmx_source;
	int master_source;
	int is_fbc;
	uint64_t used;
	uint16_t strength; // strength have values between 0 and 255
	uint32_t ber;
	uint16_t snr;							   // strength have values between 0 and 255
	float strength_multiplier, snr_multiplier; // final value: strength * strength_multipler, same for snr
	uint32_t pid_err, dec_err;				   // detect pids received but not part of any stream, decrypt errors
	diseqc diseqc_param;
	int diseqc_multi;
	int sources_pos[MAX_SOURCES + 1]; // includes SRC=0 used internally only
	char debug_pos[MAX_SOURCES + 1];
	uint64_t debug_src;
	int old_diseqc;
	int old_hiband;
	int old_pol;
	int id;
	int rlen, lbuf; // how many bytes are received in the TS buffer, length of the buffer
	int pat_processed;
	int wait_new_stream, wait_transponder_id;
	int threshold;
	int active_pids, max_active_pids, max_pids;
	int active_demux_pids;
	int is_t2mi;
	uint64_t tune_time;
	pthread_t thread;
	char name[5];
	char null_packets;
	char drop_encrypted;
	char failed_adapter; // failed adapters will not be closed due to timeout nor chosen in get_free_adapters
#ifndef DISABLE_PMT
	int transponder_id,
		pat_ver, pat_filter, sdt_filter;
#endif
#ifdef AXE
	int fe2;
	int64_t axe_vdevice_last_sync;
	int64_t axe_pktc;
	int64_t axe_ccerr;
	int axe_used;
#endif

	Set_pid set_pid;
	Del_filters del_filters;
	Adapter_commit commit;
	Open_device open;
	Tune tune;
	Dvb_delsys delsys;
	Device_signal get_signal;
	Device_wakeup wakeup;
	Device_standby standby;
	Adapter_commit post_init, close;
} adapter;

extern adapter *a[MAX_ADAPTERS];
extern uint64_t source_map[MAX_SOURCES];
extern int a_count;
extern char do_dump_pids;

int init_hw(int i);
int init_all_hw();
int getAdaptersCount();
adapter *adapter_alloc();
int close_adapter(int na);
int get_free_adapter(transponder *tp);
int set_adapter_for_stream(int sid, int aid);
void close_adapter_for_stream(int sid, int aid, int close_stream);
int set_adapter_parameters(int aid, int sid, transponder *tp);
void mark_pids_deleted(int aid, int sid, char *pids);
int mark_pids_add(int sid, int aid, char *pids);
int mark_pid_add(int sid, int aid, int _pid);
void mark_pid_deleted(int aid, int sid, int _pid, SPid *p);
int update_pids(int aid);
int tune(int aid, int sid);
void post_tune(adapter *ad);
SPid *find_pid(int aid, int p);
adapter *get_adapter1(int aid, char *file, int line);
adapter *get_configured_adapter1(int aid, char *file, int line);
char *describe_adapter(int sid, int aid, char *dad, int ld);
void dump_pids(int aid);
void sort_pids(int aid);
void enable_adapters(char *o);
void set_unicable_adapters(char *o, int type);
void set_sources_adapters(char *o);
void set_diseqc_adapters(char *o);
void set_diseqc_timing(char *o);
void set_diseqc_multi(char *o);
void set_slave_adapters(char *o);
void set_timeout_adapters(char *o);
void set_adapter_dmxsource(char *o);
void reset_pids_type(int aid, int clear_pat);
void reset_ecm_type_for_pmt(int aid, int pmt);
int delsys_match(adapter *ad, int del_sys);
int get_enabled_pids(adapter *ad, int *pids, int lpids);
int get_all_pids(adapter *ad, int *pids, int lpids);
char *get_adapter_pids(int aid, char *dest, int max_size);
void adapter_lock1(char *FILE, int line, int aid);
void adapter_unlock1(char *FILE, int line, int aid);
int adapter_timeout(sockets *s);
void adapter_set_dvr(adapter *ad);
char is_adapter_disabled(int i);
void set_adapters_delsys(char *o);
void set_lnb_adapters(char *o);
void set_signal_multiplier(char *o);
int signal_thread(sockets *s);
int close_adapter_for_socket(sockets *s);
int compare_tunning_parameters(int aid, transponder *tp);
void request_adapter_close(adapter *ad);
int compare_slave_parameters(adapter *ad, transponder *tp);
#define get_adapter(a) get_adapter1(a, __FILE__, __LINE__)
#define get_configured_adapter(a) get_configured_adapter1(a, __FILE__, __LINE__)
#define get_adapter_nw(aid) ((aid >= 0 && aid < MAX_ADAPTERS && a[aid] && a[aid]->enabled) ? a[aid] : NULL)

#define adapter_lock(a) adapter_lock1(__FILE__, __LINE__, a)
#define adapter_unlock(a) adapter_unlock1(__FILE__, __LINE__, a)
#endif
