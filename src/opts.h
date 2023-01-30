#ifndef OPTS_H
#define OPTS_H

#define _GNU_SOURCE

#include <stdint.h>
#include <time.h>

struct struct_opts {
    char *rrtp;
    char *name_app;
    char *command_line;
    char *http_host; // http-server host
    char *rtsp_host; // rtsp-server host
    char *bind;      // bind address
    char *bind_dev;  // bind device
    char *datetime_compile;
    time_t start_time;
    char *datetime_start;
    char *datetime_current;
    char *time_running;
    int run_user;
    int run_pid;
    char *disc_host; // discover host
    char mac[13];
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
    int clean_psi;
    int send_all_ecm;
    int file_line;
    char *last_log;
    int dvbapi_port;
    char dvbapi_host[100];
    int dvbapi_offset;
    int drop_encrypted;
    int pids_all_no_dec;
    int rtsp_port;
    uint8_t netcv_count;
    char *netcv_if;
    char *playlist;
    char *log_file;
    int use_ipv4_only;
    int use_demux_device;
    float strength_multiplier, snr_multiplier;
    char enigma;
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
    char *cache_dir;
#ifdef AXE
    int quattro;
    int quattro_hiband;
    int axe_unicinp[4];
    int axe_power;
#endif
};
extern struct struct_opts opts;

#endif
