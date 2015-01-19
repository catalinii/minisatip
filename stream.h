#ifndef STREAM_H
#define STREAM_H

#include <sys/socket.h>
#include "socketworks.h"
#include "dvb.h"
#define MAX_STREAMS 100
#define DVB_FRAME 188
#define STREAMS_BUFFER 7*DVB_FRAME
typedef struct struct_streams
{
	int enabled;
	int sid;					 // socket - <0 for invalid/not used, 0 for end of the list
	int adapter;
	struct sockaddr_in sa;		 //remote address - set on accept or recvfrom on udp sockets
	int https;					 // http socket -where to write data

	unsigned char *buf;
	int len,
		total_len;
	uint16_t fid;				 //frame id
	int wtime;
	int rtcp_wtime;
	int do_play;
	int start_streaming;
	transponder tp;
	int rtp;
	char *apids,
		*dpids,
		*pids;
	struct iovec iov[7];
	int iiov;

} streams;

#define TYPE_MCAST 1
#define TYPE_UNICAST 2
#define MAX_PACK 7				 // maximum rtp packets to buffer
typedef struct struct_rtp_prop
{
	int type;
	int port;
	char dest[50];
	int ttl;
} rtp_prop;

uint32_t getTick ();
char *describe_streams (int sid, char *sbuf,int size);
streams *setup_stream (char **str, sockets * s, rtp_prop * p);
int start_play (streams * sid, sockets * s);
void decode_transport (sockets * s, rtp_prop * p, char *arg,
char *default_rtp, int start_rtp);
int streams_add (int a_id, rtp_prop * p, int https);
int read_dmx (sockets * s);
int stream_timeout (sockets * s);
int close_streams_for_adapter (int ad, int except);
void dump_streams ();
streams *get_sid (int sid);
#endif
