#ifndef ADAPTER_H
#define ADAPTER_H
#include "minisatip.h"
#include <linux/dvb/frontend.h>
#include "dvb.h"

#define MAX_ADAPTERS 8
#define DVR_BUFFER 30*1024*188
#define MAX_STREAMS_PER_PID 8
#define ADAPTER_BUFFER 7*7*DVB_FRAME
  typedef struct struct_pid
{
  int pid;			// pid for this demux - not used
  int fd;			// fd for this demux    
  signed char sid[MAX_STREAMS_PER_PID];	// stream id - one more to set it -1 
  char flags;			// 0 - disabled , 1 enabled, 2 - will be enabled next tune when tune is called, 3 disable when tune is called
  int cnt;
 } pid;
   typedef struct struct_adapter
{
  int enabled;
   int fe,
    dvr;
   int pa,
    fn;				// physical adapter, physical frontend number
  transponder tp;
   pid pids[MAX_PIDS];
   int master_sid;		// first SID, the one that controls the tunning
  int sid_cnt;			//number of streams       
  int sock;
   int do_tune;
   char *buf;			// 7 rtp packets = MAX_PACK, 7 frames / packet 
  int last_sort;
   fe_status_t status;
   uint32_t ber;
   uint16_t strength, snr;
 } adapter;
 int init_hw ();
int getS2Adapters ();
int getTAdapters ();
void close_adapter (int na);
int get_free_adapter (int freq, int pol, int msys, int src);
int set_adapter_for_stream (int i, int a);
int close_adapter_for_stream (int sid, int aid);
int set_adapter_parameters (int aid, int sid, transponder * tp);
void mark_pids_deleted (int aid, int sid, char *pids);
int mark_pids_add (int sid, int aid, char *pids);
void update_pids (int aid);
adapter * get_adapter (int aid);
char *describe_adapter (int aid);
void dump_pids (int aid);
void sort_pids (int aid);


#endif	/*  */
