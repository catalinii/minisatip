#ifndef MINISATIP_H
#define MINISATIP_H
#include "stream.h"
#include "socketworks.h"


#define VERSION_BUILD "8"
#define CC(a,b,c) #a b #c
#define VERSION CC(0.1.,VERSION_BUILD,)

void set_options (int argc, char *argv[]);

#define RRTP_OPT 'r'
#define DISCOVERYIP_OPT 'd'
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
	int bw;
	int dvr;
	int force_scan;
	char *last_log;	
	char playlist[200];
};

typedef struct struct_pchar_int
{
	char *s;
	int v;
} pchar_int;

//#define LOG(a,...) {opts.last_log=a;if(opts.log){int x=getTick();printf(CC([%d.%03d]: ,a,\n),x/1000,x%1000,##__VA_ARGS__);fflush(stdout);};}
#define LOG(a,...) {opts.last_log=a;if(opts.log){printf(CC([%s]:\x20,a,\n),get_current_timestamp(),##__VA_ARGS__);fflush(stdout);};}
#define FAIL(a,...) {printf(a,##__VA_ARGS__);printf("\n");fflush(stdout);exit(1);}
#define LOG_AND_RETURN(rc,a,...) {LOG(a,##__VA_ARGS__);return rc;}
int ssdp_discovery (sockets * s);
int split (char **rv, char *s, int lrv, char sep);
int map_int (char *s, pchar_int * v);

void *mymalloc (int a, char *f, int l);
void myfree (void *x, char *f, int l);
int becomeDaemon ();
int readBootID();

#define malloc1(a) mymalloc(a,__FILE__,__LINE__)
#define free1(a) myfree(a,__FILE__,__LINE__)
#endif
