#ifndef MINISATIP_H
#define MINISATIP_H

#define _GNU_SOURCE 

#include "stream.h"
#include "socketworks.h"


#define VERSION_BUILD "9"
#define CC(a,b,c) #a b #c
#define VERSION CC(0.3.,VERSION_BUILD,)

void set_options (int argc, char *argv[]);

#define RRTP_OPT 'r'
#define DEVICEID_OPT 'd'
#define HTTPSERVER_OPT 'w'
#define HTTPPORT_OPT 'x'
#define LOG_OPT 'l'
#define HELP_OPT 'h'
#define SCAN_OPT 's'
#define PLAYLIST_OPT 'p'
#define DVBS2_ADAPTERS_OPT 'a'
#define DVBT2_ADAPTERS_OPT 't'
#define MAC_OPT 'm'
#define FOREGROUND_OPT 'f'
#define BW_OPT 'c'
#define DVRBUFFER_OPT 'b'
#define ENABLE_ADAPTERS_OPT 'e'
#define UNICABLE_OPT 'u'
#define JESS_OPT 'j'
#define DVBAPI_OPT 'o'
#define SYSLOG_OPT 'g'

#define PID_FILE "/var/run/minisatip.pid"

#define copy32(a,i,v) { a[i] = ((v)>>24) & 0xFF;\
			a[i+1] = ((v)>>16) & 0xFF;\
			a[i+2] = ((v)>>8) & 0xFF;\
			a[i+3] = (v) & 0xFF; }
#define copy16(a,i,v) { a[i] = ((v)>>8) & 0xFF; a[i+1] = (v) & 0xFF; }

#define copy16r(v, a, i) { v = ((a[i] & 0xFF) << 8) | a[i+1]; }
#define copy32r(v, a, i) { v = ((a[i] & 0xFF) << 24) | ((a[i+1] & 0xFF) << 16) | ((a[i+2] & 0xFF) << 8)| (a[i+3] & 0xFF);   }

struct struct_opts
{
	char *rrtp;
	char *http_host;			 //http-server host
	char *disc_host;			 //discover host
	char mac[13];
	unsigned int log, slog, start_rtp, http_port;
	int timeout_sec;
	int force_sadapter, force_tadapter, force_cadapter;
	int daemon;
	int device_id;
	int bootid;
	int bw;	
	int dvr_buffer;
	int adapter_buffer;
	int force_scan;
	int file_line;
	char *last_log;	
	int dvbapi_port;
	char *dvbapi_host;
	char playlist[200];
	int drop_encrypted;
};

void _log(int level, char * file, int line, const char *fmt, ...);

//#define proxy_log(level, fmt, ...) _proxy_log(level, fmt"\n", ##__VA_ARGS__)

//#define LOG(a,...) {opts.last_log=a;if(opts.log){int x=getTick();printf(CC([%d.%03d]: ,a,\n),x/1000,x%1000,##__VA_ARGS__);fflush(stdout);};}
//#define LOG(a,...) {opts.last_log=a;if(opts.log){printf(CC([%s]:\x20,a,\n),get_current_timestamp_log(),##__VA_ARGS__);fflush(stdout);};}
#define LOG(a,...) { _log(1,__FILE__,__LINE__,a, ##__VA_ARGS__);}
#define LOGL(level,a,...) { if(level<=opts.log)_log(level,__FILE__,__LINE__,a, ##__VA_ARGS__);}


#define FAIL(a,...) {LOGL(0,a,##__VA_ARGS__);unlink(PID_FILE);exit(1);}
#define LOG_AND_RETURN(rc,a,...) {LOG(a,##__VA_ARGS__);return rc;}
int ssdp_discovery (sockets * s);
int split (char **rv, char *s, int lrv, char sep);
int map_int (char *s, char ** v);
int map_intd (char *s, char ** v, int dv);
int map_float (char *s, int mul);
void *mymalloc (int a, char *f, int l);
void myfree (void *x, char *f, int l);
int becomeDaemon ();
int readBootID();
void hexDump (char *desc, void *addr, int len);

#define malloc1(a) mymalloc(a,__FILE__,__LINE__)
#define free1(a) myfree(a,__FILE__,__LINE__)
#endif
