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

#include "minisatip.h"
#include "socketworks.h"
#include "stream.h"
#include "dvb.h"
#include "adapter.h"

extern struct struct_opts opts;
streams st[MAX_STREAMS];
unsigned init_tick, theTick;
uint64_t ntime;

uint32_t
getTick ()
{								 //ms
	struct timespec ts;

	clock_gettime (CLOCK_REALTIME, &ts);
	theTick = ts.tv_nsec / 1000000;
	theTick += ts.tv_sec * 1000;
	ntime = ts.tv_sec * 1000000 + ts.tv_nsec;
	if (init_tick == 0)
		init_tick = theTick;
	return theTick - init_tick;
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
	LOG ("decode_transport ->%d %d %d %s", p.type, p.ttl, p.port, p.dest);
	if( sid->type != 0)
	{
		if(sid->type == STREAM_RTSP_UDP && sid->rsock >= 0)			
		{
			if(p.port != ntohs(sid->sa.sin_port) || strcmp(p.dest,inet_ntoa(sid->sa.sin_addr)))
				LOG("Transport for this connection already setup, leaving as it is: sid = %d, handle %d", sid->sid, sid->rsock)
			else {
				LOG("Stream has already a transport header associated with it - sid = %d type = %d, closing %d", sid->sid, sid->type, sid->rsock);
				close(sid->rsock);
				sid->rsock = -1;
				sid->type = 0;
			}
		}
	}
	
	if(sid->type == 0)
	{
		int i;
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
								 // max 7 packets
	st[i].total_len = 7 * DVB_FRAME;
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

#define copy32(a,i,v) { a[i] = ((v)>>24) & 0xFF;\
			a[i+1] = ((v)>>16) & 0xFF;\
			a[i+2] = ((v)>>8) & 0xFF;\
			a[i+3] = (v) & 0xFF; }
#define copy16(a,i,v) { a[i] = ((v)>>8) & 0xFF; a[i+1] = (v) & 0xFF; }

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
	//      LOG("write to %d (len:%d)-> %X, %d, %X, %d ",sock,liov*sizeof(iov),io[0].iov_base,io[0].iov_len,io[1].iov_base,io[1].iov_len);
	rv = writev (sid->rsock, (const struct iovec *) io, liov + 1);
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
	copy16( rtcp, 54, (la + 16) / 4);
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


extern int bw;
extern uint32_t nsecs, reads;
void
flush_streamb (streams * sid, char *buf, int rlen, int ctime)
{
	int i, rv = 0;
	
	if (sid->type == STREAM_HTTP)
		rv = send (sid->rsock, buf, rlen, MSG_NOSIGNAL);
	else
		for (i = 0; i < rlen; i += DVB_FRAME * 7)
			rv += send_rtpb (sid, &buf[i], DVB_FRAME * 7);
	
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


void
flush_streami (streams * sid, int ctime)
{
	int rv;
	if (sid->type == STREAM_HTTP)
		rv = writev (sid->rsock, sid->iov, sid->iiov);
	else
		rv = send_rtp (sid, sid->iov, sid->iiov);
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
	pid *p;
	int pid, flush_all = 0;
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
								 
	if (s->rlen < s->lbuf && s->rtime - ad->rtime < 50) // flush buffers every 50ms
		return 0;		// the buffer is not full, read more
	else if(s->rlen < s->lbuf)
		flush_all = 1; // flush everything that we read so far
	if (s->rlen > s->lbuf)
		LOG ("Buffer overrun %d %d", s->rlen, s->lbuf);

//	LOGL(2, "read_dmx called for adapter %d -> %d bytes read ",s->sid, s->rlen);
		
	int rlen = s->rlen;
	ad->rtime = s->rtime;
	s->rlen = 0;
	getTick();
	stime = ntime;
	
	if (cnt > 0 && cnt % 100 == 0)
		LOG ("Reading max size for the last %d buffers", cnt);
								 // we have just 1 stream, do not check the pids, send everything to the destination
	if (ad->sid_cnt == 1 && ad->master_sid >= 0 && opts.log < 2)
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
		//              LOG("not implemented");
		//              char used[DVB_FRAME*7*MAX_PACK]; // if the DVB packet is already sent
		//              return 0;
		for (dp = 0; dp < rlen; dp += DVB_FRAME)
		{
			unsigned char *b = &s->buf[dp];

			pid = (b[1] & 0x1f) * 256 + b[2];
			p = ad->pids;
			for (i = 0; i < MAX_PIDS; i++)
				if (p[i].flags == 1 && (p[i].pid == pid || p[i].pid == 8192))
				{
					int cc;   // calculate pid continuity
					p[i].cnt++;
					cc = b[3] & 0xF;
					if (p[i].cc == 255)
						p[i].cc = cc;
					else if (p[i].cc == 15)
						p[i].cc = 0;
					else p[i].cc ++;
						
					if(p[i].cc != cc)
					{	
//						LOG("PID Continuity error (adapter %d): pid: %03d, Expected CC: %X, Actual CC: %X", s->sid, pid, p[i].cc, cc);
						p[i].err ++;
					}
					p[i].cc = cc;

					for (j = 0; p[i].sid[j] > -1 && j < MAX_STREAMS_PER_PID; j++)
					{
						if ((sid = get_sid(p[i].sid[j])))
						{
							if (sid->iiov > 7)
							{
								LOG ("ERROR: possible writing outside of allocated space iiov > 7 for SID %d PID %d", sid->sid, pid);
								sid->iiov = 6;
							}
							sid->iov[sid->iiov].iov_base = b;
							sid->iov[sid->iiov++].iov_len = DVB_FRAME;
							if (sid->iiov >= 7)
								flush_streami (sid, s->rtime);
						}
						else
							LOG ("could not find a valid sid %d for pid:%d",
								p[i].sid[j], pid);
					}
				}
		}

		if(flush_all)   // more than 50ms passed since the last write, so we flush our buffers
		{
			for (i = 0; i < MAX_STREAMS; i++)
                        	if (st[i].enabled && st[i].adapter == s->sid && sid->iiov > 0)
					flush_streami (sid, s->rtime);

		}else{   //move all dvb packets that were not sent out of the s->buf
			min = s->buf;
			max = &s->buf[rlen];
			for (i = 0; i < MAX_STREAMS; i++)
				if (st[i].enabled && st[i].adapter == s->sid && sid->iiov > 0)
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
	getTick();
	nsecs += ntime - stime;
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
		if (st[i].enabled && st[i].do_play && st[i].type != STREAM_HTTP)
		{
			sid = get_sid(i);
			rttime = sid->rtcp_wtime,
			rtime = sid->wtime;
	
			//LOG("stream timeouts called for sid %d c:%d r:%d rt:%d",i,ctime,rtime,rttime);
			if (ctime - rtime > 1000)
			{
				LOG ("no data sent for more than 1s sid: %d for %s:%d", i,
						inet_ntoa (sid->sa.sin_addr), ntohs (sid->sa.sin_port));
				flush_streami (sid, ctime);								
			}
			if (ctime - rttime > 200)
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
