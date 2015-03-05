#ifndef MINISATIP_H
#define MINISATIP_H

#define _GNU_SOURCE 

#include "stream.h"
#include "socketworks.h"


#define VERSION_BUILD "84"
#define CC(a,b,c) #a b #c
#define VERSION CC(0.1.,VERSION_BUILD,)

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

#define PID_FILE "/var/run/minisatip.pid"
struct struct_opts
{
	char *rrtp;
	char *http_host;			 //http-server host
	char *disc_host;			 //discover host
	char mac[13];
	unsigned int log,
		start_rtp,
		http_port;
	int timeout_sec;
	int force_sadapter, force_tadapter, force_cadapter;
	int daemon;
	int device_id;
	int bootid;
	int bw;	
	int dvr;
	int force_scan;
	int file_line;
	char *last_log;	
	char playlist[200];
};

void _log(int level, char * file, int line, const char *fmt, ...);

//#define proxy_log(level, fmt, ...) _proxy_log(level, fmt"\n", ##__VA_ARGS__)

//#define LOG(a,...) {opts.last_log=a;if(opts.log){int x=getTick();printf(CC([%d.%03d]: ,a,\n),x/1000,x%1000,##__VA_ARGS__);fflush(stdout);};}
//#define LOG(a,...) {opts.last_log=a;if(opts.log){printf(CC([%s]:\x20,a,\n),get_current_timestamp_log(),##__VA_ARGS__);fflush(stdout);};}
#define LOG(a,...) { _log(1,__FILE__,__LINE__,a, ##__VA_ARGS__);}
#define LOGL(level,a,...) { _log(level,__FILE__,__LINE__,a, ##__VA_ARGS__);}


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

#define malloc1(a) mymalloc(a,__FILE__,__LINE__)
#define free1(a) myfree(a,__FILE__,__LINE__)
#endif
