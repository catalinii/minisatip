#ifndef ADAPTER_H
#define ADAPTER_H
#include "minisatip.h"
#include "dvb.h"

typedef struct ca_device ca_device_t;

#define MAX_ADAPTERS 16
#define DVR_BUFFER 30*1024*188
#ifdef NO_BACKTRACE
#define MAX_STREAMS_PER_PID 8
#else
#define MAX_STREAMS_PER_PID 16
#endif
#define ADAPTER_BUFFER (128 + 5)*DVB_FRAME
#define ADAPTER_TIMEOUT 60000

#define TYPE_PMT 1
#define TYPE_ECM 2
#define PMT_COMPLETE 4
#define CLEAN_PMT 16

#define RTSP_SETUP 1
#define RTSP_PLAY 2
#define RTSP_OPTIONS 3
#define RTSP_TEARDOWN 4
#define RTSP_DESCRIBE 5

typedef struct struct_pid
{
	int16_t pid;					 // pid for this demux - not used
	int fd;						 // fd for this demux
	int err;					// counter errors
								// stream id - one more to set it -1
	signed char sid[MAX_STREAMS_PER_PID];
	char flags;	// 0 - disabled , 1 enabled, 2 - will be enabled next tune when tune is called, 3 disable when tune is called
	char type;
	int cnt;
	int dec_err;			// decrypt errors
	unsigned char key, filter, ecm_parity; // custom data kept in the SPid structure
	unsigned char cc, version; // continuity
	unsigned char enabled_channels; // ca information
	uint16_t csid; // channel sid
} SPid;

typedef int (*Set_pid)(void *ad, uint16_t i_pid);
typedef int (*Del_filters)(int fd, int pid);
typedef int (*Adapter_commit)(void *ad);
typedef int (*Open_device)(void *ad);
typedef int (*Device_signal)(void *ad);
typedef int (*Tune)(int aid, transponder * tp);
typedef fe_delivery_system_t (*Dvb_delsys)(int aid, int fd,
		fe_delivery_system_t *sys);

#define ADAPTER_DVB 1
#define ADAPTER_SATIP 2
#define ADAPTER_NETCV 3

#define MAX_DELSYS 10

typedef struct struct_adapter
{
	char enabled;
	SMutex mutex;
	char type, slow_dev; // available on the system
	int fe, dvr, dmx;
	int pa, fn;
	// physical adapter, physical frontend number
	fe_delivery_system_t sys[MAX_DELSYS];
	transponder tp;
	SPid pids[MAX_PIDS];
	int ca_mask;	
	int master_sid;				 // first SID, the one that controls the tuning
	int sid_cnt;				 //number of streams
	int sock, fe_sock;
	int do_tune;
	int force_close;
	unsigned char *buf;			// 7 rtp packets = MAX_PACK, 7 frames / packet
	int rlen;
	int64_t rtime;
	int64_t last_sort;
	int new_gs;
	int status, status_cnt;
	int dmx_source;
	uint32_t ber;
	uint16_t strength, snr, max_strength, max_snr;
	uint32_t pid_err, dec_err; // detect pids received but not part of any stream, decrypt errors
	diseqc diseqc_param;
	int old_diseqc;
	int old_hiband;
	int old_pol;
	int id;
	int pat_processed, transponder_id, pat_ver;
	char name[5];

	Set_pid set_pid;
	Del_filters del_filters;
	Adapter_commit commit;
	Open_device open;
	Tune tune;
	Dvb_delsys delsys;
	Device_signal get_signal;
	Adapter_commit post_init, close;
} adapter;

extern adapter *a[MAX_ADAPTERS];
extern int a_count;

int init_hw(int dev);
int init_all_hw();
int getAdaptersCount();
adapter *adapter_alloc();
int close_adapter(int na);
int get_free_adapter(transponder *tp);
int set_adapter_for_stream(int i, int a);
void close_adapter_for_stream(int sid, int aid);
int set_adapter_parameters(int aid, int sid, transponder * tp);
void mark_pids_deleted(int aid, int sid, char *pids);
int mark_pids_add(int sid, int aid, char *pids);
int mark_pid_add(int sid, int aid, int _pid);
void mark_pid_deleted(int aid, int sid, int _pid, SPid *p);
int update_pids(int aid);
int tune(int aid, int sid);
SPid *find_pid(int aid, int p);
adapter * get_adapter1(int aid, char *file, int line);
char *describe_adapter(int sid, int aid, char *dad, int ld);
void dump_pids(int aid);
void sort_pids(int aid);
void enable_adapters(char *o);
void set_unicable_adapters(char *o, int type);
void set_diseqc_adapters(char *o);
void set_diseqc_timing(char *o);
void set_slave_adapters(char *o);
void set_adapter_dmxsource(char *o);
void reset_pids_type(int aid, int clear_pat);
void reset_ecm_type_for_key(int aid, int key);
int delsys_match(adapter *ad, int del_sys);
int get_enabled_pids(adapter *ad, int *pids, int lpids);
int get_all_pids(adapter *ad, int *pids, int lpids);
char *get_adapter_pids(int aid, char *dest, int max_size);
void adapter_lock1(char *FILE, int line, int aid);
void adapter_unlock1(char *FILE, int line, int aid);
char is_adapter_disabled(int i);
void set_adapters_delsys(char *o);
int signal_thread(sockets *s);
int compare_tunning_parameters(int aid, transponder * tp);

#define get_adapter(a) get_adapter1(a, __FILE__, __LINE__)
#define get_adapter_nw(aid) ((aid >= 0 && aid < MAX_ADAPTERS && a[aid] && a[aid]->enabled)?a[aid]:NULL)

#define adapter_lock(a) adapter_lock1(__FILE__,__LINE__,a)
#define adapter_unlock(a) adapter_unlock1(__FILE__,__LINE__,a)
#endif							 /*  */
