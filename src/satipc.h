#ifndef SATIPCLIENT_H
#define SATIPCLIENT_H

#include "adapter.h"

#define SATIP_STR_LEN 5000
#define SATIP_MAX_STRENGTH 255
#define SATIP_MAX_QUALITY 15

#define SATIP_STATE_DISCONNECTED 0
#define SATIP_STATE_SETUP 1
#define SATIP_STATE_PLAY 2
#define SATIP_STATE_TEARDOWN 3
#define SATIP_STATE_INACTIVE 0

typedef struct struct_satipc {
    char enabled;
    SMutex mutex;
    int id;
    int lap, ldp;            // number of pids to add, number of pids to delete
    uint16_t apid[MAX_PIDS]; // pids to add
    uint16_t dpid[MAX_PIDS]; // pids to delete
    // satipc
    char sip[40];
    char source_ip[20]; // source ip address
    int sport;
    char session[18];
    int stream_id;
    int listen_rtp;
    int rtcp, rtcp_sock, cseq;
    char ignore_packets; // ignore packets coming from satip server while tuning
    int satip_fe;
    char last_cmd;
    char use_tcp, init_use_tcp;
    char no_pids_all;
    char state;
    int64_t last_setup, last_connect, last_close, last_response_sent;
    uint8_t addpids, setup_pids;
    unsigned char *tcp_data;
    int tcp_size, tcp_pos, tcp_len;
    char option_no_setup;
    uint32_t rcvp, repno, rtp_miss, rtp_ooo; // rtp statstics
    uint16_t rtp_seq;
    char static_config;
    int num_describe;
    int timeout_ms;
    // Bit Fields
    unsigned int
        rtsp_socket_closed : 1; // is set when the adapter was closed
                                // unexpected and needs to be re-enabled
    unsigned int
        keep_adapter_open : 1; // if set, the adapter will not be closed
                               // when the rtsp socket is being closed
    unsigned int can_keep_adapter_open : 1; // if set, the adapter is valid and
                                            // can be restarted
    unsigned int restart_when_tune : 1;
    unsigned int restart_needed : 1;
    unsigned int expect_reply : 1;

    unsigned int want_commit : 1;
    unsigned int want_tune : 1;
    unsigned int force_pids : 1;
    unsigned int sent_transport : 1;
} satipc;

extern satipc *satip[];

void find_satip_adapter(adapter **a);
int satip_getxml(void *);
char *init_satip_pointer(int len);
void get_s2_url(adapter *ad, char *url, int url_len);
void get_c2_url(adapter *ad, char *url, int url_len);
void get_t2_url(adapter *ad, char *url, int url_len);
#endif
