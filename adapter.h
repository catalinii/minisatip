#ifndef ADAPTER_H
#define ADAPTER_H
#include "minisatip.h"
#include <linux/dvb/frontend.h>
#include "dvb.h"

#define MAX_ADAPTERS 16
#define DVR_BUFFER 30*1024*188
#define MAX_STREAMS_PER_PID 8
#define ADAPTER_BUFFER (128 + 5)*DVB_FRAME

#define TYPE_PMT 1
#define TYPE_ECM 2
#define PMT_COMPLETE 4
typedef struct struct_pid
{
	int pid;					 // pid for this demux - not used
	int fd;						 // fd for this demux
	int err;
								// stream id - one more to set it -1
	signed char sid[MAX_STREAMS_PER_PID];
	char flags;					 // 0 - disabled , 1 enabled, 2 - will be enabled next tune when tune is called, 3 disable when tune is called
	int type;
	int csid;  // channel sid if type & TYPE_PMT
	int cnt;
	unsigned char key, filter, ecm_parity; // custom data kept in the SPid structure
	unsigned char cc; // continuity
} SPid;

typedef struct struct_adapter
{
	int enabled;
	int force_disable;
	int fe,	dvr;
	int pa, fn;		
		// physical adapter, physical frontend number
	fe_delivery_system_t sys[10]; 
	transponder tp;
	SPid pids[MAX_PIDS];
	int master_sid;				 // first SID, the one that controls the tuning
	int sid_cnt;				 //number of streams
	int sock;
	int do_tune;
	char *buf;					 // 7 rtp packets = MAX_PACK, 7 frames / packet
	int rlen,rtime;
	int last_sort;
	int status_cnt;
	int new_gs;
	fe_status_t status;
	uint32_t ber;
	uint16_t strength, snr, max_strength, max_snr;
	int switch_type;
	int uslot; // unicable/jess slot
	int ufreq; // unicable/jess frequency	
	int pin;
	int id;
	int pat_processed;
} adapter;

int init_hw ();
int getS2Adapters ();
int getTAdapters ();
int getCAdapters ();
void close_adapter (int na);
int get_free_adapter (int freq, int pol, int msys, int src);
int set_adapter_for_stream (int i, int a);
int close_adapter_for_stream (int sid, int aid);
int set_adapter_parameters (int aid, int sid, transponder * tp);
void mark_pids_deleted (int aid, int sid, char *pids);
int mark_pids_add (int sid, int aid, char *pids);
int mark_pid_add(int sid, int aid, int _pid);
void mark_pid_deleted(int aid, int sid, int _pid, SPid *p);
int update_pids (int aid);
SPid *find_pid(int aid, int p);
adapter * get_adapter1 (int aid, char *file, int line);
char *describe_adapter (int sid, int aid);
void dump_pids (int aid);
void sort_pids (int aid);
void enable_adapters(char *o);
void set_unicable_adapters(char *o, int type);
void reset_pids_type(int aid);
void reset_pids_type_for_key(int aid, int key);
int delsys_match(adapter *ad, int del_sys);

#define get_adapter(a) get_adapter1(a, __FILE__, __LINE__)
#endif							 /*  */
