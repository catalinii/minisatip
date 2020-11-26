#ifndef MINISATIP_H
#define MINISATIP_H

#define _GNU_SOURCE

#include "stream.h"
#include "socketworks.h"
#include "utils.h"

#ifndef VERSION
#define VERSION "1.0-unknown"
#endif

#define EMU_PIDS_ALL_ENFORCED_PIDS_LIST 1,16,17,18,20,21

void set_options(int argc, char *argv[]);

extern char pid_file[];
extern char app_name[], version[];

#ifndef offsetof
#define offsetof(st, m) __builtin_offsetof(st, m)
#endif

#define copy32(a, i, v)                \
	{                                  \
		a[i] = ((v) >> 24) & 0xFF;     \
		a[i + 1] = ((v) >> 16) & 0xFF; \
		a[i + 2] = ((v) >> 8) & 0xFF;  \
		a[i + 3] = (v)&0xFF;           \
	}
#define copy16(a, i, v)           \
	{                             \
		a[i] = ((v) >> 8) & 0xFF; \
		a[i + 1] = (v)&0xFF;      \
	}

#define copy16r(v, a, i)                     \
	{                                        \
		v = ((a[i] & 0xFF) << 8) | a[i + 1]; \
	}
#define copy16rr(v, a, i)                    \
	{                                        \
		v = ((a[i + 1] & 0xFF) << 8) | a[i]; \
	}

#define copy32r(v, a, i)                                                                                      \
	{                                                                                                         \
		v = ((a[i] & 0xFF) << 24) | ((a[i + 1] & 0xFF) << 16) | ((a[i + 2] & 0xFF) << 8) | (a[i + 3] & 0xFF); \
	}
#define copy32rr(v, a, i)                                                                                     \
	{                                                                                                         \
		v = ((a[i + 3] & 0xFF) << 24) | ((a[i + 2] & 0xFF) << 16) | ((a[i + 1] & 0xFF) << 8) | (a[i] & 0xFF); \
	}

struct struct_opts
{
	char *rrtp;
	char *http_host; //http-server host
	char *rtsp_host; //rtsp-server host
	char *datetime_compile;
	time_t start_time;
	char *datetime_start;
	char *datetime_current;
	char *time_running;
	int run_user;
	int run_pid;
	char *disc_host; //discover host
	char mac[13];
	unsigned int log, debug, slog, start_rtp, http_port;
	int timeout_sec;
	int force_sadapter, force_tadapter, force_cadapter;
	int daemon;
	int device_id;
	int bootid;
	int dvr_buffer;
	int adapter_buffer;
	int output_buffer;
	int udp_threshold;
	int tcp_threshold;
	int force_scan;
	int clean_psi;
	int file_line;
	char *last_log;
	int dvbapi_port;
	char dvbapi_host[100];
	int dvbapi_offset;
	int drop_encrypted;
	int rtsp_port;
	uint8_t netcv_count;
	char *netcv_if;
	char *playlist;
	char *log_file;
	int use_ipv4_only;
	int use_demux_device;
	float strength_multiplier, snr_multiplier;
#ifndef DISABLE_SATIPCLIENT
	char *satip_servers;
	char *satip_xml;
	uint8_t satip_addpids, satip_setup_pids, satip_rtsp_over_tcp;
#endif
	char *document_root;
	char *xml_path;
	char no_threads;
	int th_priority;
	int diseqc_fast;
	int diseqc_addr;
	int diseqc_committed_no;
	int diseqc_uncommitted_no;
	int diseqc_before_cmd;
	int diseqc_after_cmd;
	int diseqc_after_repeated_cmd;
	int diseqc_after_switch;
	int diseqc_after_burst;
	int diseqc_after_tone;
	int diseqc_multi;
	int lnb_low, lnb_high, lnb_switch, lnb_circular;
	int adapter_timeout;
	int max_pids;
	int max_sbuf;
	char disable_dvb;
	char disable_ssdp;
	char pmt_scan;
	char emulate_pids_all;
#ifdef AXE
	int quattro;
	int quattro_hiband;
	int axe_unicinp[4];
	int axe_power;
#endif
};
extern struct struct_opts opts;

int ssdp_discovery(sockets *s);
int becomeDaemon();
int readBootID();
void http_response(sockets *s, int rc, char *ah, char *desc, int cseq, int lr);

#endif
