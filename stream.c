/*
 * Copyright (C) 2014-2020 Catalin Toda <catalinii@yahoo.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 *
 */

#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>
#include <poll.h>
#include <fcntl.h>

#include "minisatip.h"
#include "socketworks.h"
#include "stream.h"
#include "dvb.h"
#include "adapter.h"

#include "dvbapi.h"

extern struct struct_opts opts;
streams st[MAX_STREAMS];
unsigned init_tick, theTick;

uint32_t
getTick ()
{								 //ms
	struct timespec ts;
	clock_gettime (CLOCK_REALTIME, &ts);
	theTick = ts.tv_nsec / 1000000;
	theTick += ts.tv_sec * 1000;
	if (init_tick == 0)
		init_tick = theTick;
	return theTick - init_tick;
}

uint64_t getTickUs()
{
	uint64_t utime;
	struct timespec ts;
	clock_gettime (CLOCK_REALTIME, &ts);	
	utime = ((uint64_t )ts.tv_sec) * 1000000 + ts.tv_nsec / 1000;
	return utime;

}

int get_next_free_stream()
{
	int i;
	for(i=0;i<MAX_STREAMS;i++)
		if(!st[i].enabled)
			return i;
	return -1;
}

char *describe_streams (sockets *s, char *req, char *sbuf, int size)
{
	char *str, *stream_id;
	int i, sidf, do_play = 0, streams_enabled = 0;
	streams *sid;
	int do_all = 1;

	if (s->sid == -1 && strchr(req, '?'))
		setup_stream(req, s);

	sidf = get_session_id(s->sid);
	sid = get_sid(s->sid);
	if(sid)
		do_play = sid->do_play;
//	else LOG_AND_RETURN(NULL, "No session associated with sock_id %d", s->sock_id);
		
	snprintf(sbuf,size-1,"v=0\r\no=- %010d %010d IN IP4 %s\r\ns=SatIPServer:1 %d %d %d\r\nt=0 0\r\n", sidf, sidf, get_sock_host(s->sock), getS2Adapters(), getTAdapters(), getCAdapters() );
	if(strchr(req, '?'))
		do_all = 0;
		
	if((stream_id = strstr(req, "stream=")))
	{
		do_all = 0;
		sid = get_sid(map_int(stream_id + 7, NULL) - 1);
		if(sid == NULL)
			return NULL;		
	}	
	
	if(do_all)
	{
		for( i=0; i<MAX_STREAMS; i++)
			if(st[i].enabled)
			{
				int slen=strlen(sbuf);
				streams_enabled ++;
				snprintf(sbuf + slen, size - slen - 1, "m=video %d RTP/AVP 33\r\nc=IN IP4 0.0.0.0\r\na=control:stream=%d\r\na=fmtp:33 %s\r\na=%s\r\n", 
					ntohs (st[i].sa.sin_port), i+1, describe_adapter(i, st[i].adapter), st[i].do_play?"sendonly":"inactive");
				if( size - slen < 10)LOG_AND_RETURN(sbuf, "DESCRIBE BUFFER is full");
			}
	}else{
		int slen = strlen(sbuf);
		snprintf(sbuf + slen, size - slen - 1, "m=video 0 RTP/AVP 33\r\nc=IN IP4 0.0.0.0\r\na=control:stream=%d\r\na=fmtp:33 %s\r\nb=AS:5000\r\na=%s\r\n", 
			sid->sid + 1, describe_adapter(sid->sid, sid->adapter), do_play?"sendonly":"inactive");
	}
	return sbuf;
}



// we need to keep the pids from SETUP and PLAY into sid->tp
void
set_stream_parameters (int s_id, transponder * t)
{
	streams *sid;

	sid = get_sid(s_id);
	if (!sid || !sid->enabled)
		return;
	if (t->apids && t->apids[0] >= '0' && t->apids[0] <= '9')
	{
		strcpy (sid->apids, t->apids);
		t->apids = sid->apids;
	}
	if (t->dpids && t->dpids[0] >= '0' && t->dpids[0] <= '9')
	{
		strcpy (sid->dpids, t->dpids);
		t->dpids = sid->dpids;
	}
	if (t->pids && t->pids[0] >= '0' && t->pids[0] <= '9')
	{
		strcpy (sid->pids, t->pids);
		t->pids = sid->pids;
	}
	if (!t->apids)
		t->apids = sid->tp.apids;
	if (!t->dpids)
		t->dpids = sid->tp.dpids;
	if (!t->pids)
		t->pids = sid->tp.pids;
	copy_dvb_parameters (t, &sid->tp);
}


streams *
setup_stream (char *str, sockets * s)
{
	char *arg[20], pol;
	streams *sid;

	int i;
	transponder t;
	init_hw ();
	detect_dvb_parameters (str, &t);
	LOG ("Setup stream %d parameters, sock_id %d, handle %d", s->sid,
		s->sock_id, s->sock);
	if (s->sid < 0)				 // create the stream
	{
		int a_id = 0,
			s_id = 0;
		char pol;

		s_id = streams_add ();
		if(!(sid = get_sid(s_id)))
			LOG_AND_RETURN ( NULL, "Could not add a new stream");

		s->sid = s_id;
//		s->close_sec = 200;
//		s->timeout = (socket_action) stream_timeout;
		LOG ("Setup stream done: sid: %d (e:%d) for sock %d handle %d", s_id,
			st[s_id].enabled, s->sock_id, s->sock);
	}
	if (!( sid = get_sid(s->sid)))
		LOG_AND_RETURN (NULL, "Stream %d not enabled for sock_id %d handle %d",
			s->sid, s->sock_id, s->sock);
	
	set_stream_parameters (s->sid, &t);
	sid->do_play = 0;
	
	if(sid->adapter >= 0 && !strncasecmp( s->buf, "SETUP", 5 )) // SETUP after PLAY
	{
		int ad = sid->adapter;
		sid->adapter = -1;
		close_adapter_for_stream(sid->sid, ad);
	}
	
	return sid;
}


int
start_play (streams * sid, sockets * s)
{
	int a_id;

	if (sid->type == 0 && s->type == TYPE_HTTP)
		{
			sid->type = STREAM_HTTP;
			sid->rsock = s->sock;
		}

	if(sid->type == 0)
	{
//			LOG("Assuming RTSP over TCP for stream %d, most likely transport was not specified", sid->sid);
//			sid->type = STREAM_RTSP_TCP;
//			sid->rsock = s->sock;
			LOG_AND_RETURN(-454, "No Transport header was specified, for sid %d", sid->sid);
	}
	
	LOG ("Play for stream %d, type %d, rsock %d, adapter %d, sock_id %d handle %d", s->sid, sid->type, sid->rsock,
		sid->adapter, s->sock_id, s->sock);

	if (sid->adapter == -1)		 // associate the adapter only at play (not at setup)
	{
		a_id =
			get_free_adapter (sid->tp.freq, sid->tp.pol, sid->tp.sys,
			sid->tp.fe);
		LOG ("Got adapter %d on socket %d", a_id, s->sock_id);
		if (a_id < 0)
			return -404;
		sid->adapter = a_id;
		set_adapter_for_stream (sid->sid, a_id);

	}
	if (set_adapter_parameters (sid->adapter, s->sid, &sid->tp) < 0)
		return -404;
	sid->do_play = 1;
	sid->start_streaming = 0;
	sid->tp.apids = sid->tp.dpids = sid->tp.pids = NULL;
	return tune (sid->adapter, s->sid);
}


int decode_transport (sockets * s, char *arg, char *default_rtp, int start_rtp)
{
	char *arg2[10];
	int l;
	rtp_prop p;
	streams *sid = get_sid(s->sid);
	if (! sid)
	{
		LOG("Error: No stream to set transport to, sock_id %d, arg %s ",s->sock_id, arg);
		return -1;
	}
	l = 0;
	sid->rsock_err = 0;
	if ( arg )
	{
		if (strstr(arg, "RTP/AVP/TCP"))
		{
			LOG("Assuming RTSP over TCP for stream %d, arg %s", sid->sid, arg);
			sid->type = STREAM_RTSP_TCP;
			sid->rsock = s->sock;
			return 0;
		}
		
		l = split (arg2, arg, 10, ';');
	
	}
	//      LOG("arg2 %s %s %s",arg2[0],arg2[1],arg2[2]);
	memset (&p, 0, sizeof (p));
	while (l > 0)
	{
		l--;
		if (strcmp ("multicast", arg2[l]) == 0)
			p.type = TYPE_MCAST;
		if (strcmp ("unicast", arg2[l]) == 0)
			p.type = TYPE_UNICAST;
		if (strncmp ("ttl=", arg2[l], 4) == 0)
			p.ttl = atoi (arg2[l] + 4);
		if (strncmp ("client_port=", arg2[l], 12) == 0)
			p.port = atoi (arg2[l] + 12);
		if (strncmp ("port=", arg2[l], 5) == 0)
			p.port = atoi (arg2[l] + 5);
		if (strncmp ("destination=", arg2[l], 12) == 0)
			strncpy (p.dest, arg2[l] + 12, sizeof (p.dest));
	}
	if (default_rtp)
		strncpy (p.dest, default_rtp, sizeof (p.dest));
	if (p.dest[0] == 0 && p.type == TYPE_UNICAST)
		strncpy (p.dest, inet_ntoa (s->sa.sin_addr), sizeof (p.dest));
	if (p.dest[0] == 0)
		strcpy (p.dest, opts.disc_host);
	if (p.port == 0)
		p.port = start_rtp;
	LOG ("decode_transport ->type %d, ttl %d new socket to: %s:%d", p.type, p.ttl, p.dest, p.port);
	if( sid->type != 0)
	{
		if(sid->type == STREAM_RTSP_UDP && sid->rsock >= 0)			
		{
			int oldport = ntohs(sid->sa.sin_port);
			char *oldhost = inet_ntoa(sid->sa.sin_addr);
			
			if(p.port == oldport && !strcmp(p.dest,oldhost))
				LOG("Transport for this connection is already setup to %s:%d, leaving as it is: sid = %d, handle %d", 
					p.dest, p.port, sid->sid, sid->rsock)
			else {
				LOG("Stream has already a transport header associated with it to %s:%d - sid = %d type = %d, closing %d", 
					oldhost, oldport, sid->sid, sid->type, sid->rsock);
				close(sid->rsock);
				sid->rsock = -1;
				sid->type = 0;
			}
		}
	}
	
	if(sid->type == 0)
	{
		int i;
		socklen_t sl;
		struct sockaddr_in sa;
		
		sid->type = STREAM_RTSP_UDP;
		if(sid->rtcp_sock > 0 || sid->rtcp)
		{
			sockets_del(sid->rtcp_sock);
			sid->rtcp_sock = -1;
			sid->rtcp = -1;
		}			
	
		if((sid->rsock = udp_bind_connect (NULL, opts.start_rtp + (sid->sid * 2), p.dest, p.port, &sid->sa)) < 0)
			LOG_AND_RETURN (-1, "decode_transport failed: UDP connection on rtp port to %s:%d failed", p.dest, p.port);

		i = 512*1024;
		if(setsockopt(sid->rsock, SOL_SOCKET, SO_SNDBUF, &i, sizeof(i)))
			LOG("unable to set output UDP buffer size to %d", i);
                sl = sizeof(int);
		if(!getsockopt(sid->rsock, SOL_SOCKET, SO_SNDBUF, &i, &sl))
			LOG("output UDP buffer size is %d bytes", i);
			
		if ((sid->rtcp = udp_bind_connect (NULL, opts.start_rtp + (sid->sid * 2) +1, p.dest, p.port + 1, &sa)) < 1)
			LOG_AND_RETURN (-1, "decode_transport failed: UDP connection on rtcp port to %s:%d failed", p.dest, p.port+1);
			
		if ((sid->rtcp_sock = sockets_add(sid->rtcp, NULL, sid->sid, TYPE_RTCP, (socket_action )rtcp_confirm, NULL, NULL)) < 0) // read rtcp
			LOG_AND_RETURN(-1, "RTCP socket_add failed");
		
		for(i=0;i<MAX_STREAMS;i++)
			if(st[i].enabled && i!=sid->sid && st[i].sa.sin_port  == sid->sa.sin_port && st[i].sa.sin_addr.s_addr == sid->sa.sin_addr.s_addr)
			{
				LOG("Detected stream with the same destination as sid %d: sid %d -> %s:%d, aid: %d", sid->sid, i, p.dest, p.port, st[i].adapter);
				close_stream(i);
			}           
	}
	
	return 0;
}


int
streams_add ()
{
	int i;
	char *ra;

	i = get_next_free_stream();
	if (i == -1)
	{
		LOG ("The stream could not be added - most likely no free stream");
		return -1;
	}
	
	st[i].enabled = 1;
	st[i].adapter = -1;
	st[i].sid = i;
	st[i].rsock = -1;
	st[i].rsock_err = 0;
	st[i].type = 0;
	st[i].do_play = 0;
	st[i].iiov = 0;
	st[i].sp = st[i].sb = 0;
	memset (&st[i].iov, 0 , sizeof(st[i].iiov));
	init_dvb_parameters (&st[i].tp);
	st[i].len = 0;
//	st[i].seq = 0; // set the sequence to 0 for testing purposes - it should be random 
	st[i].ssrc = random();
	st[i].timeout = opts.timeout_sec;
	st[i].wtime = st[i].rtcp_wtime = getTick ();	
	
	st[i].total_len = 7 * DVB_FRAME; // max 7 packets
	if (!st[i].pids)
		st[i].pids = malloc1 (MAX_PIDS * 5 + 1);
	if (!st[i].apids)
		st[i].apids = malloc1 (MAX_PIDS * 5 + 1);
	if (!st[i].dpids)
		st[i].dpids = malloc1 (MAX_PIDS * 5 + 1);
	if (!st[i].buf)
		st[i].buf = malloc1 (STREAMS_BUFFER + 10);
	if (!st[i].pids || !st[i].apids || !st[i].dpids || !st[i].buf)
	{
		LOG ("memory allocation failed for stream %d\n", i);
		if (st[i].pids)
			free1 (st[i].pids);
		if (st[i].apids)
			free1 (st[i].apids);
		if (st[i].dpids)
			free1 (st[i].dpids);
		if (st[i].buf)
			free1 (st[i].buf);
		st[i].pids = st[i].apids = st[i].dpids = st[i].buf = NULL;
		return -1;
	}
	return i;
}


int
close_stream (int i)
{
	int ad;
	streams *sid;
	if (i < 0 || i >= MAX_STREAMS || st[i].enabled == 0)
		return;		
	LOG ("closing stream %d", i);
	sid = &st[i];
	sid->enabled = 0;
	ad = sid->adapter;
	sid->adapter = -1;
	if ( sid->type == STREAM_RTSP_UDP && sid->rsock > 0) close (sid->rsock);
	sid->rsock = -1;
	if(sid->rtcp_sock > 0 || sid->rtcp > 0 )
	{
		sockets_del(sid->rtcp_sock);
		sid->rtcp_sock = -1;
		sid->rtcp = -1;
	}

	if (ad >= 0)
		close_adapter_for_stream (i, ad);
	sockets_del_for_sid (i);
	/*  if(sid->pids)free(sid->pids);
	  if(sid->apids)free(sid->apids);
	  if(sid->dpids)free(sid->dpids);
	  if(sid->buf)free(sid->buf);
	  sid->pids = sid->apids = sid->dpids = sid->buf = NULL;
	*/
	LOG ("closed stream %d", i);
}


int
								 // close all streams for adapter, excepting except
close_streams_for_adapter (int ad, int except)
{
	int i;

	if (ad < 0)
		return 0;
	for (i = 0; i < MAX_STREAMS; i++)
		if (st[i].enabled && st[i].adapter == ad)
			if (except < 0 || except != i)
				close_stream (i);
}


unsigned char rtp_buf[16];

int
send_rtpb (streams * sid, unsigned char *b, int len)
{
	struct iovec iov[2];

	iov[0].iov_base = b;
	iov[0].iov_len = len;
	//      LOG("called send_rtpb %X %d",b,len);
	return send_rtp (sid, (const struct iovec *) iov, 1);
}

extern int bw, sleeping, sleeping_cnt;
extern uint64_t nsecs;
extern uint32_t reads;

int slow_down;
uint64_t last_sd;

int
send_rtp (streams * sid, struct iovec *iov, int liov)
{
	struct iovec io[MAX_PACK + 3];
	int i, total_len = 0, rv;
	unsigned char *rtp_h;

	if(sid->rsock_err > 5)
		return 0;
	rtp_h = rtp_buf + 4;
	
	for(i = 0;i < liov; i ++)
		total_len += iov[i].iov_len;

	memset (&io, 0, sizeof(io));
	rtp_buf[0] = 0x24;
	rtp_buf[1] = 0;
	copy16 (rtp_buf, 2, total_len + 12); 
	copy16 (rtp_h, 0, 0x8021 );
	copy16 ( rtp_h, 2, sid->seq);
	copy32 ( rtp_h, 4, sid->wtime);
	copy32 ( rtp_h, 8, sid->ssrc);
	sid->seq ++;
 
	if ( sid->type == STREAM_RTSP_UDP)
	{
		io[0].iov_base = rtp_h;
		io[0].iov_len = 12;
	}else{  // RTSP over TCP
		io[0].iov_base = rtp_buf;
		io[0].iov_len = 16;		

	}
	memcpy (&io[1], iov, liov * sizeof (struct iovec));
	rv = my_writev (sid->rsock, (const struct iovec *) io, liov + 1, sid);
	if(opts.bw && (slow_down++ > 20))
	{
		uint64_t tn = getTickUs();
//		int interval = (1328 * 20000 / (opts.bw / 1024)) ;   // For 1328kb/s we have 1000 packets/s, each 20 packets => 20 000 us
		int interval = opts.bw * 10 / 1024; 
		int result = tn - last_sd;
		if((result > 0) && (result < interval))
		{		
				usleep(result);		
				sleeping += result;
				sleeping_cnt ++;
		}
		if(slow_down > 20)
		{
			last_sd = tn;
			slow_down = 0;
		}
	}
	if(rv<0)
	{
		sid->rsock_err ++;
		LOG("write to handle %d failed: %d, %s, socket err %d %s", sid->rsock, rv, strerror(errno), sid->rsock_err, sid->rsock_err>5?"socket blacklisted":"");		
	} else sid->rsock_err = 0;
	
	if(total_len > 0 && sid->start_streaming == 0)
	{
		sid->start_streaming = 1;
		LOG("Start streaming for stream %d, len %d to handle %d => %s:%d", sid->sid, total_len, sid->rsock, inet_ntoa(sid->sa.sin_addr), ntohs(sid->sa.sin_port));
	}
	
	LOGL(5, "sent %d bytes for stream %d, handle %d seq %d => %s:%d",  total_len, sid->sid, sid->rsock, sid->seq - 1, inet_ntoa(sid->sa.sin_addr), ntohs(sid->sa.sin_port));
	
	return rv;
}

unsigned char rtcp_buf[1600];


int
send_rtcp (int s_id, int ctime)
{
	int tmp_port;
	int len, rv = 0;
	char *rtcp = rtcp_buf + 4;
	streams *sid = get_sid(s_id);
	
	if(!sid)
		LOG_AND_RETURN (0, "Sid is null for s_id %d", s_id);

	if(sid->rsock_err > 5)
		return 0;		
		
	char *a = describe_adapter (s_id, st[s_id].adapter);
	int la = strlen (a);
	if ( la > sizeof(rtcp_buf) - 68) 
		la = sizeof(rtcp_buf)-70;
	len = la + 16;
	if (len % 4 > 0)len = len - (len % 4) +4;
	//      LOG("send_rtcp (sid: %d)-> %s",s_id,a);
								 // rtp header
	rtcp[0] = 0x80; // Begin Sender Report
	rtcp[1] = 0xC8;
	rtcp[2] = 0;
	rtcp[3] = 6;
	copy32( rtcp, 4, sid->ssrc);
	copy32( rtcp, 8,  0);
	copy32( rtcp, 12,  (theTick - init_tick)/1000);
	copy32( rtcp, 16, sid->wtime);
	copy32( rtcp, 20, sid->sp);
	copy32( rtcp, 24, sid->sb);  
	rtcp[28] = 0x81; // Begin Source Description
	rtcp[29] = 0xCA;
	rtcp[30] = 0;
	rtcp[31] = 5;
	copy32( rtcp, 32, sid->ssrc);  
	rtcp[36] = 1;
	rtcp[37] = 10;
	rtcp[38] = 'm';
	rtcp[39] = 'i';
	rtcp[40] = 'n';
	rtcp[41] = 'i';
	rtcp[42] = 's';
	rtcp[43] = 'a';
	rtcp[44] = 't';
	rtcp[45] = 'i';
	rtcp[46] = 'p';
	copy32( rtcp, 47, 0);  
	rtcp[51] = 0;
	rtcp[52] = 0x80;
	rtcp[53] = 0xCC;
	copy16( rtcp, 54, ((la + 16 + 3) / 4) - 1);
	copy32( rtcp, 56, sid->ssrc);  
	rtcp[60] = 'S';
	rtcp[61] = 'E';
	rtcp[62] = 'S';
	rtcp[63] = '1';
	rtcp[64] = 0;
	rtcp[65] = 0;
	copy16( rtcp, 66, la);
	memcpy (rtcp+68, a, la+4);
	if ( sid->type == STREAM_RTSP_UDP)
		rv = send (sid->rtcp, rtcp, len + 52, MSG_NOSIGNAL);			
	else 
	{
		rtcp_buf[0] = 0x24;
		rtcp_buf[1] = 1; 
		copy16(rtcp_buf, 2, len + 52);
		rv = send (sid->rsock, rtcp_buf, len + 52 + 4, MSG_NOSIGNAL);
	}
//	if(rv>0)
//		sid->rsock_err = 0;
//	else	
//		sid->rsock_err ++;
	sid->rtcp_wtime = ctime;
//	sid->sp = 0;
//	sid->sb = 0;
	return rv;
}


void flush_streamb (streams * sid, char *buf, int rlen, int ctime)
{
	int i, rv = 0;
	
	if (sid->type == STREAM_HTTP)
		rv = send (sid->rsock, buf, rlen, MSG_NOSIGNAL);
	else
		for (i = 0; i < rlen; i += DVB_FRAME * 7)
			rv += send_rtpb (sid, &buf[i], ((rlen - i) > DVB_FRAME * 7)?DVB_FRAME*7:(rlen - i));
	
	sid->iiov = 0;
	sid->wtime = ctime;
	sid->len = 0;

        if(rv > 0)
        {
                bw += rv;
                sid->sp ++;
                sid->sb += rv;
        }
	
}

int my_writev(int sock, struct iovec *iov, int iiov, streams *sid)
{
	int rv;
	LOGL(6,"start writev handle %d, iiov %d", sock, iiov);
	rv = writev(sock,iov,iiov);
	if(errno < 0 && errno == ECONNREFUSED) // close the stream int the next second
		sid->timeout = 1000;
	LOGL(6,"writev returned %d handle %d, iiov %d", rv, sock, iiov);
	return rv;
}

void
flush_streami (streams * sid, int ctime)
{
	int rv, i, len;
	
	if (sid->type == STREAM_HTTP)
		rv = my_writev (sid->rsock, sid->iov, sid->iiov, sid);
	else
		rv = send_rtp (sid, sid->iov, sid->iiov);		

#ifdef DEBUG

	static int fd;
	if(!fd)
	{
		unlink("output.ts");
		fd = open("output.ts", O_CREAT | O_WRONLY, 0666);
	}
	if(fd)writev(fd, sid->iov, sid->iiov);
#endif		
	sid->iiov = 0;
	sid->wtime = ctime;
	sid->len = 0;

        if(rv > 0)
        {
                bw += rv;
                sid->sp ++;
                sid->sb += rv;
        }

}


int process_packet(unsigned char *b, adapter *ad)
{
	int i, j, cc;
	SPid *p;
	int _pid = (b[1] & 0x1f) * 256 + b[2];
	int de = have_dvbapi();
	streams *sid;
	int rtime = ad->rtime;
	if( _pid == 8191)
	{
//		LOGL(2, "process_packet: pid 8191");
		return 0;	
	}
	p = find_pid(ad->id, _pid);
	if((!p) || ((p->sid[0]==-1) && (p->type==0)))
	{
//		LOGL(2, "process_packet: pid %d not found", _pid);
		return 0;
	}
	p->cnt++;
	cc = b[3] & 0xF;
	if (p->cc == 255)
		p->cc = cc;
	else if (p->cc == 15)
		p->cc = 0;
	else p->cc ++;
				
	if(p->cc != cc)
	{	
//		LOG("PID Continuity error (adapter %d): pid: %03d, Expected CC: %X, Actual CC: %X", s->sid, pid, p->cc, cc);
		p->err ++;
	}
	p->cc = cc;

	for (j = 0; p->sid[j] > -1 && j < MAX_STREAMS_PER_PID; j++)
	{
		if ((sid = get_sid(p->sid[j])))
		{
			if (sid->iiov > 7)
			{
				LOG ("ERROR: possible writing outside of allocated space iiov > 7 for SID %d PID %d", sid->sid, _pid);
				sid->iiov = 6;
			}
			sid->iov[sid->iiov].iov_base = b;
			sid->iov[sid->iiov++].iov_len = DVB_FRAME;
			if (sid->iiov >= 7)
				flush_streami (sid, rtime);
		}
	}
	
}


int
read_dmx (sockets * s)
{
	void *min,
		*max;
	int i,  j, dp;
	static int cnt;
	streams *sid;
	adapter *ad;
	unsigned char sids;
	SPid *p;
	int pid, send = 0, flush_all = 0;
	uint64_t stime;

	if (s->rlen % DVB_FRAME != 0)
		s->rlen = ((int) s->rlen / DVB_FRAME) * DVB_FRAME;
	if (s->rlen == s->lbuf)
		cnt++;
	else
		cnt = 0;
	ad = get_adapter (s->sid);
	if (!ad)
	{
		s->rlen = 0;
		return 0;
	}
								 
	if (s->rtime - ad->rtime > 50) // flush buffers every 50ms
	{
		flush_all = 1; // flush everything that we've read so far
		send = 1;
	}
	
	if(flush_all && (s->rlen > 20000))  // if the bw/s > 300kB - waiting for the buffer to fill - most likely watching a TV channel and not scanning
		send = 0;
	
	if(s->rlen == s->lbuf)
		send = 1;
	
	if(s->rtime - ad->rtime > 1000)
		send = 1;
	
	LOGL(6, "read_dmx send=%d, flush_all=%d called for adapter %d -> %d out of %d bytes read, %d ms ago", send, flush_all, s->sid, s->rlen, s->lbuf, s->rtime - ad->rtime);

	if(!send) 
		return;
		
	int rlen = s->rlen;
	int ms_ago = s->rtime - ad->rtime;
	ad->rtime = s->rtime;
	s->rlen = 0;
	stime = getTickUs();

	LOGL(5, "read_dmx start flush_all=%d called for adapter %d -> %d out of %d bytes read, %d ms ago", flush_all, s->sid, rlen, s->lbuf, ms_ago );   	
	if (cnt > 0 && cnt % 100 == 0)
		LOG ("Reading max size for the last %d buffers", cnt);								 
								 
	ca_grab_pmt(ad, rlen);
	
	decrypt_stream(ad, rlen);	
								 
	if (ad->sid_cnt == 1 && ad->master_sid >= 0 && opts.log < 2) // we have just 1 stream, do not check the pids, send everything to the destination
	{
		sid = get_sid(ad->master_sid);
		if (sid->enabled != 1)
		{
			LOG ("Master SID %d not enabled ", ad->master_sid);
			return -1;
		}
		if (sid->len > 0)
			flush_streamb (sid, sid->buf, sid->len, s->rtime);
		flush_streamb (sid, s->buf, rlen, s->rtime);

	}
	else
	{	
		for (dp = 0; dp < rlen; dp += DVB_FRAME)		
			process_packet(&s->buf[dp], ad);

		if(flush_all)   // more than 50ms passed since the last write, so we flush our buffers
		{
			for (i = 0; i < MAX_STREAMS; i++)
				if (st[i].enabled && st[i].adapter == s->sid && st[i].iiov > 0)
					flush_streami (&st[i], s->rtime);

		}
		else{   //move all dvb packets that were not sent out of the s->buf
			min = s->buf;
			max = &s->buf[rlen];
			for (i = 0; i < MAX_STREAMS; i++)
				if (st[i].enabled && st[i].adapter == s->sid && st[i].iiov > 0)
				{
					sid = get_sid(i);
					if(!sid)
						continue;
					for (j = 0; j < sid->iiov; j++)
						if (sid->iov[j].iov_base >= min && sid->iov[j].iov_base <= max)
						{
							if (sid->len + DVB_FRAME >= STREAMS_BUFFER)
							{
								LOG ("ERROR: requested to write outside of stream's buffer for sid %d len %d iiov %d - flushing stream's buffer", i, sid->len, sid->iiov);
								flush_streamb (sid, sid->buf, sid->len, s->rtime);
							}

							memcpy (&sid->buf[sid->len], sid->iov[j].iov_base, DVB_FRAME);
							sid->iov[j].iov_base = &sid->buf[sid->len];
							sid->len += DVB_FRAME;
					}
			}
		}

		if (s->rtime - ad->last_sort > 2000)
		{
			ad->last_sort = s->rtime + 60000;
			sort_pids (s->sid);
		}
	}
	
	nsecs += getTickUs() - stime;
	reads ++;
	//      if(!found)LOG("pid not found = %d -> 1:%d 2:%d 1&1f=%d",pid,s->buf[1],s->buf[2],s->buf[1]&0x1f);
	//      LOG("done send stream");
	return 0;
}


int
stream_timeouts ()
{
	int i;
	int ctime, rttime, rtime;
	streams *sid;
	char buf[2000];
	
	ctime = getTick ();
	
	for (i=0; i< MAX_STREAMS; i++)
		if (st[i].enabled && st[i].type != STREAM_HTTP)
		{
			sid = get_sid(i);
			rttime = sid->rtcp_wtime,
			rtime = sid->wtime;
	
			//LOG("stream timeouts called for sid %d c:%d r:%d rt:%d",i,ctime,rtime,rttime);
			if (sid->do_play && ctime - rtime > 1000)
			{
				LOG ("no data sent for more than 1s sid: %d for %s:%d", i,
						inet_ntoa (sid->sa.sin_addr), ntohs (sid->sa.sin_port));
				flush_streami (sid, ctime);								
			}
			if (sid->do_play && ctime - rttime > 200)
				send_rtcp (i, ctime);
						// check stream timeout, and allow 10s more to respond
			if (sid->timeout > 0 && (ctime - sid->rtime > sid->timeout + 10000)) 
			{
				LOG("Stream timeout %d, closing (ctime %d , sid->rtime %d, sid->timeout %d)", i, ctime, sid->rtime, sid->timeout);
				close_stream(i);
			}			
		
	}
	return 0;
}


void
dump_streams ()
{
	int i;

	if (!opts.log)
		return;
	LOG ("Dumping streams:");
	for (i = 0; i < MAX_STREAMS; i++)
		if (st[i].enabled)
			LOG ("%d|  a:%d rsock:%d type:%d play:%d remote:%s:%d", i, st[i].adapter,
				st[i].rsock, st[i].type, st[i].do_play, inet_ntoa (st[i].sa.sin_addr),
				ntohs (st[i].sa.sin_port));
}


void
free_all_streams ()
{
	int i;

	for (i = 0; i < MAX_STREAMS; i++)
	{
		if (st[i].pids)
			free1 (st[i].pids);
		if (st[i].apids)
			free1 (st[i].apids);
		if (st[i].dpids)
			free1 (st[i].dpids);
		if (st[i].buf)
			free1 (st[i].buf);
	}
}


streams *
get_sid1 (int sid,char *file, int line)
{
	if (sid < 0 || sid > MAX_STREAMS || st[sid].enabled == 0)
		LOG_AND_RETURN(NULL, "%s:%d get_sid returns NULL for s_id = %d", file, line, sid);
	return &st[sid];
}


int get_session_id( int i)
{
	if (i<0 || i>MAX_STREAMS || st[i].enabled==0)
		return 0;
	return st[i].ssrc;
}

void set_session_id(int i, int id)
{
	if (i<0 || i>MAX_STREAMS || st[i].enabled==0)
		return;
	if(st[i].ssrc != id)
	{
		LOG("Forcing session id %d on stream %d", id, i);
		st[i].ssrc = id;
	}	
}


int fix_master_sid(int a_id)
{
	int i;
	adapter *ad;
	ad = get_adapter(a_id);
	
	if(!ad || ad->master_sid!=-1)
		return 0;
	if(ad->sid_cnt<1)
		return 0;
	for(i=0;i<MAX_STREAMS;i++)
		if(st[i].enabled && st[i].adapter == a_id)
		{
				LOG("fix master_sid to %d for adapter %d", st[i].sid, a_id);
				ad->master_sid = i;
		}
	return 0;
}

int find_session_id(int id)
{
	int i;
	for(i=0;i<MAX_STREAMS;i++)
		if(st[i].enabled && st[i].ssrc == id)
		{
			st[i].rtime = getTick();
			LOG("recovered session id from a closed connection, sid %d , id: %d", i, id);
			return i;
		}
	return -1;
}

int rtcp_confirm(sockets *s)
{
	streams *sid;
//	LOG("rtcp_confirm called for from %s:%d", inet_ntoa(s->sa.sin_addr), ntohs(s->sa.sin_port));
		// checking just the ports and the destination
	sid = get_sid(s->sid);
	if(sid)
	{
		LOG("Acknowledging stream %d via rtcp packet", s->sid);
		sid->rtime = s->rtime;
	}
	return 0;
}
