#ifndef SATIPCLIENT_H
#define SATIPCLIENT_H

#include "adapter.h"

#ifndef DISABLE_SRT
#include <srt/srt.h>
#endif

#define SATIP_STR_LEN 5000
#define SATIP_MAX_STRENGTH 255
#define SATIP_MAX_QUALITY 15

#define SATIP_STATE_DISCONNECTED 0
#define SATIP_STATE_SETUP 1
#define SATIP_STATE_PLAY 2
#define SATIP_STATE_TEARDOWN 3
#define SATIP_STATE_INACTIVE 0

enum satipc_transport_type {
    SIP_TRANSPORT_UDP = 0,
    SIP_TRANSPORT_TCP = 1,
    SIP_TRANSPORT_SRT = 2
};

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
    int listen_udp;
    int rtcp, rtcp_sock, cseq;
    bool ignore_packets; // ignore packets coming from satip server while tuning
    int satip_fe;
    char last_cmd;
    satipc_transport_type transport_type;
    char no_pids_all;
    char state;
    int64_t last_setup, last_connect, last_close, last_response_sent;
    uint8_t addpids, setup_pids;
    unsigned char *tcp_data;
    int tcp_size, tcp_pos, tcp_len;
    bool option_no_setup;
    uint32_t rcvp, repno, rtp_miss, rtp_ooo; // rtp statstics
    uint16_t rtp_seq;
    char static_config;
    int num_describe;
    int timeout_ms;
    // Bit Fields
    bool rtsp_socket_closed;    // is set when the adapter was closed
                                // unexpected and needs to be re-enabled
    bool keep_adapter_open;     // if set, the adapter will not be closed
                                // when the rtsp socket is being closed
    bool can_keep_adapter_open; // if set, the adapter is valid and
                                // can be restarted
    bool restart_when_tune;
    bool restart_needed;
    bool expect_reply;

    bool want_commit;
    bool want_tune;
    bool force_pids;
    bool sent_transport;
#ifndef DISABLE_SRT
    SRTSOCKET srt_sock = SRT_INVALID_SOCK;
    int udp_sock = -1; // UDP socket for SRT
    std::string
        srt_streamid; // random SRT stream ID for caller/listener correlation
#endif
} satipc;

extern satipc *satip[];

void find_satip_adapter(adapter **a);
int satip_getxml(void *);
char *init_satip_pointer(int len);
int satipc_timeout(sockets *s);
#endif
