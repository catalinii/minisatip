#ifndef OPTS_H
#define OPTS_H

#include "utils/uuid.h"
#include <stdint.h>
#include <time.h>

typedef struct struct_opts {
    char *rrtp;
    char *name_app;
    char *command_line;
    char *http_host; // http-server host
    char *rtsp_host; // rtsp-server host
    char *bind;      // bind address (RTSP + SSDP)
    char *bind_http; // bind address (HTTP)
    char *bind_dev;  // bind device
    char *datetime_compile;
    time_t start_time;
    char *datetime_start;
    char *datetime_current;
    char *time_running;
    int run_user;
    int run_pid;
    char *disc_host; // discover host
    char uuid[UUID_STR_LEN];
    unsigned int log, debug, slog, start_rtp, http_port;
    int timeout_sec;
    int force_sadapter, force_tadapter, force_cadapter;
    int daemon;
    int device_id;
    int bootid;
    int dvr_buffer;
    int satipc_buffer;
    int adapter_buffer;
    int output_buffer;
    int udp_threshold;
    int tcp_threshold;
    int force_scan;
    int send_all_ecm;
    int file_line;
    int dvbapi_port;
    char dvbapi_host[100];
    int dvbapi_offset;
    int drop_encrypted;
    int pids_all_no_dec;
    int rtsp_port;
    uint8_t netcv_count;
    char *netcv_if;
    char *playlist;
    const char *log_file;
    int use_ipv4_only;
    int use_demux_device;
    float strength_multiplier, snr_multiplier;
    char enigma;
#ifndef DISABLE_SATIPCLIENT
    char *satip_servers;
    char *satip_xml;
    uint8_t satip_rtsp_over_tcp;
#endif
    const char *document_root;
    const char *xml_path;
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
    const char *cache_dir;
} struct_opts_t;

void parse_dvbapi_opt(char *optarg, struct_opts_t *optz);

extern struct_opts_t opts;

#endif
