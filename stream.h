#ifndef STREAM_H
#define STREAM_H

#include <sys/socket.h>
#include "socketworks.h"
#include "dvb.h"

#define MAX_STREAMS 100
#define DVB_FRAME 188
#define STREAMS_BUFFER 7*DVB_FRAME

#define STREAM_HTTP 1
#define STREAM_RTSP_UDP 2
#define STREAM_RTSP_TCP 3
#define MAX_PACK 7				 // maximum rtp packets to buffer

typedef struct struct_streams
{
	char enabled;
	int sid;					 // socket - <0 for invalid/not used, 0 for end of the list
	int adapter;
	struct sockaddr_in sa;		 //remote address - set on accept or recvfrom on udp sockets
	int rsock;				 // return socket handle, for rtsp over tcp, rtsp over udp or http
	int rsock_err; 
	int rtcp, rtcp_sock; 
	int type; 
	unsigned char *buf;
	int len, total_len;
	uint16_t seq;  //rtp seq id
	int ssrc; // rtp seq id			 
	int wtime;
	int rtime;  // stream timeout
	int rtcp_wtime;
	int do_play;
	int start_streaming;
	transponder tp;
	char *apids, *dpids, *pids, *x_pmt;
	struct iovec iov[MAX_PACK + 2];
	int iiov;
	uint32_t sp,sb;
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

uint32_t getTick ();
uint64_t getTickUs();
char *describe_streams (sockets *s, char *req, char *sbuf,int size);
streams *setup_stream (char *str, sockets * s);
int start_play (streams * sid, sockets * s);
int decode_transport (sockets * s, char *arg, char *default_rtp, int start_rtp);
int streams_add ();
int read_dmx (sockets * s);
int stream_timeouts ();
int close_streams_for_adapter (int ad, int except);
void dump_streams ();
streams *get_sid1 (int sid, char *file, int line, int warning);
int get_session_id( int i);
void set_session_id(int i, int id);
int fix_master_sid(int adapter);
int rtcp_confirm(sockets *s);
char *get_stream_rhost(int s_id, char *dest, int ld);
int get_stream_rport(int s_id);
int get_streams_for_adapter(int aid);

#define get_sid(a) get_sid1(a, __FILE__, __LINE__,1)
#define get_sid_nw(a) get_sid1(a, __FILE__, __LINE__,0)
#endif
