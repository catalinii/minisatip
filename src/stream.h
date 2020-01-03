#ifndef STREAM_H
#define STREAM_H

#include <sys/socket.h>
#include "socketworks.h"
#include "dvb.h"

// MAX_STREAMS, DDCI_SID should fit in adapter->sid
#define MAX_STREAMS 100
// STREAMS reserved by another modules
#define DDCI_SID (MAX_STREAMS + 1)

#define VALID_SID(i) (i >= 0 && i < MAX_STREAMS)

#define DVB_FRAME 188

#define STREAM_HTTP 1
#define STREAM_RTSP_UDP 2
#define STREAM_RTSP_TCP 3

#define UDP_MAX_PACK 7	// maximum udp rtp packets to buffer
#define TCP_MAX_PACK 42 // maximum tcp packets for a RTP header
#define TCP_MAX_IOV 1008 // TCP_MAX_PACK * X < 1024

#define LEN_PIDS (MAX_PIDS * 5 + 1)

typedef struct struct_streams
{
	char enabled;
	SMutex mutex;
	int sid; // socket - <0 for invalid/not used, 0 for end of the list
	int adapter;
	USockAddr sa;    //remote address - set on accept or recvfrom on udp sockets
	int sock;			   // remote tcp socket (http, rtsp) 
	int rsock;			   // return socket handle, for rtsp over tcp, rtsp over udp or http
	int rsock_id;
	int rtcp, rtcp_sock, st_sock;
	int type;
	int len;
	uint16_t seq; //rtp seq id
	int ssrc;	 // rtp seq id
	int64_t wtime;
	int64_t rtime; // stream timeout
	int64_t rtcp_wtime;
	int64_t last_init_hw;
	int do_play;
	int start_streaming;
	transponder tp;
	char apids[LEN_PIDS + 1], dpids[LEN_PIDS + 1], pids[LEN_PIDS + 1],
		x_pmt[LEN_PIDS + 1];
	uint32_t sp, sb;
	int timeout;
	char useragent[40];
} streams;

#define TYPE_MCAST 1
#define TYPE_UNICAST 2

typedef struct struct_rtp_prop
{
	int type;
	int port;
	char dest[50];
	int ttl;
} rtp_prop;

streams *get_stream(int i);

char *describe_streams(sockets *s, char *req, char *sbuf, int size);
streams *setup_stream(char *str, sockets *s);
int start_play(streams *sid, sockets *s);
int decode_transport(sockets *s, char *arg, char *default_rtp, int start_rtp);
int streams_add();
int read_dmx(sockets *s);
int stream_timeout(sockets *s);
int close_streams_for_adapter(int ad, int except);
int close_stream(int i);
void dump_streams();
streams *get_sid1(int sid, char *file, int line);
int get_session_id(int i);
void set_session_id(int i, int id);
int fix_master_sid(int a_id);
int rtcp_confirm(sockets *s);
char *get_stream_rhost(int s_id, char *dest, int ld);
int get_stream_rport(int s_id);
int get_streams_for_adapter(int aid);
int find_session_id(int id);
int calculate_bw(sockets *s);
int lock_streams_for_adapter(int aid);
int unlock_streams_for_adapter(int aid);

#define get_sid(a) get_sid1(a, __FILE__, __LINE__)
#define get_sid_for(i) ((st[i] && st[i]->enabled) ? st[i] : NULL)
#define get_sid_nw(i) ((i >= 0 && i < MAX_STREAMS && st[i] && st[i]->enabled) ? st[i] : NULL)

extern int64_t c_tbw, c_bw;
extern uint32_t c_reads, c_writes, c_failed_writes;
extern int64_t c_ns_read, c_tt;

#endif
