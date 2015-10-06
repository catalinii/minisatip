#ifndef ADAPTER_H
#define ADAPTER_H
#include "minisatip.h"
#include <linux/dvb/frontend.h>
#include "dvb.h"

typedef struct ca_device ca_device_t;

#define MAX_ADAPTERS 16
#define DVR_BUFFER 30*1024*188
#define MAX_STREAMS_PER_PID 8
#define ADAPTER_BUFFER (128 + 5)*DVB_FRAME
#define ADAPTER_TIMEOUT 10000

#define TYPE_PMT 1
#define TYPE_ECM 2
#define PMT_COMPLETE 4
#define PMTCA_COMPLETE 8
#define CLEAN_PMT 16

#define RTSP_SETUP 1
#define RTSP_PLAY 2
#define RTSP_OPTIONS 3
#define RTSP_TEARDOWN 4
#define RTSP_DESCRIBE 5


typedef struct struct_pid
{
	int pid;					 // pid for this demux - not used
	int fd;						 // fd for this demux
	int err;					// counter errors
								// stream id - one more to set it -1
	signed char sid[MAX_STREAMS_PER_PID];
	char flags;					 // 0 - disabled , 1 enabled, 2 - will be enabled next tune when tune is called, 3 disable when tune is called
	int type;
	int csid;  // channel sid if type & TYPE_PMT
	int cnt;
	int dec_err;			// decrypt errors
	unsigned char key, filter, ecm_parity; // custom data kept in the SPid structure
	unsigned char cc; // continuity
} SPid;

typedef int (* Set_pid) (void *ad, uint16_t i_pid);
typedef int (* Del_filters) (int fd, int pid);
typedef int (* Adapter_commit)(void *ad);
typedef int (* Open_device)(void *ad);
typedef int (* Tune) (int aid, transponder * tp);
typedef fe_delivery_system_t (* Dvb_delsys) (int aid, int fd, fe_delivery_system_t *sys);

#define ADAPTER_DVB 1
#define ADAPTER_SATIP 2
#define MAX_DELSYS 10

typedef struct struct_adapter
{
	char enabled;
	char type; // available on the system 
	int force_disable;
	int fe,	dvr, ca;
	int pa, fn;		
		// physical adapter, physical frontend number
	fe_delivery_system_t sys[MAX_DELSYS]; 
	transponder tp;
	SPid pids[MAX_PIDS];
	int master_sid;				 // first SID, the one that controls the tuning
	int sid_cnt;				 //number of streams
	int sock, fe_sock;
	int do_tune;
	int force_close;
	unsigned char *buf;					 // 7 rtp packets = MAX_PACK, 7 frames / packet
	int rlen,rtime;
	int last_sort;
	int status_cnt;
	int new_gs;
	fe_status_t status;
	uint32_t ber;
	uint16_t strength, snr, max_strength, max_snr;
	uint32_t pid_err, dec_err; // detect pids received but not part of any stream, decrypt errors
	int switch_type;
	int uslot; // unicable/jess slot
	int ufreq; // unicable/jess frequency	
	int pin;
	int committed_no, uncommitted_no; // diseqc info
	int id;
	int pat_processed, transponder_id, pat_ver;
	ca_device_t * ca_device;
// satipc
	char *sip;
	int sport;
	char session[18];
	int stream_id;
	int listen_rtp;
	int rtcp, rtcp_sock, cseq;
	int err, last_connect;
	int wp, qp; // written packet, queued packet
	char ignore_packets; // ignore packets coming from satip server while tuning
	char satip_fe, last_cmd;
	char expect_reply, force_commit, want_commit, want_tune, sent_transport;
	int satip_last_setup;
	uint32_t rcvp, repno, rtp_miss, rtp_ooo;   // rtp statstics
	uint16_t rtp_seq;
	Set_pid set_pid;
	Del_filters  del_filters;
	Adapter_commit commit;
	Open_device open;
	Tune tune;
	Dvb_delsys delsys;
	Adapter_commit post_init, close;
} adapter;


int init_hw ();
int getAdaptersCount();
void close_adapter (int na);
int get_free_adapter (int freq, int pol, int msys, int src);
int set_adapter_for_stream (int i, int a);
void close_adapter_for_stream(int sid, int aid);
int set_adapter_parameters (int aid, int sid, transponder * tp);
void mark_pids_deleted (int aid, int sid, char *pids);
int mark_pids_add (int sid, int aid, char *pids);
int mark_pid_add(int sid, int aid, int _pid);
void mark_pid_deleted(int aid, int sid, int _pid, SPid *p);
int update_pids (int aid);
SPid *find_pid(int aid, int p);
adapter * get_adapter1 (int aid, char *file, int line, int warning);
char *describe_adapter (int sid, int aid, char *dad, int ld);
void dump_pids (int aid);
void sort_pids (int aid);
void enable_adapters(char *o);
void set_unicable_adapters(char *o, int type);
void set_diseqc_adapters(char *o);
void reset_pids_type(int aid);
void reset_pids_type_for_key(int aid, int key);
int delsys_match(adapter *ad, int del_sys);
int get_enabled_pids(adapter *ad, int *pids, int lpids);
char *get_adapter_pids(int aid, char *dest, int max_size);
#define get_adapter(a) get_adapter1(a, __FILE__, __LINE__,1)
#define get_adapter_nw(a) get_adapter1(a, __FILE__, __LINE__,0)
#endif							 /*  */
