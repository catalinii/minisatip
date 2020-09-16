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
#include <poll.h>
#include <fcntl.h>
#include <sys/time.h>

#include "minisatip.h"
#include "socketworks.h"
#include "stream.h"
#include "dvb.h"
#include "adapter.h"
#include "t2mi.h"

#include "pmt.h"

#define DEFAULT_LOG LOG_STREAM

streams *st[MAX_STREAMS];
SMutex st_mutex, bw_mutex;
extern int tuner_s2, tuner_t, tuner_c, tuner_t2, tuner_c2;
int pmt_process_stream(adapter *ad);

streams *get_stream(int i)
{
	return (i >= 0 && i < MAX_STREAMS && st[i] && st[i]->enabled) ? st[i] : NULL;
}

streams *get_sid1(int sid, char *file, int line)
{
	if (sid < 0 || sid >= MAX_STREAMS || !st[sid] || st[sid]->enabled == 0)
	{
		LOG("%s:%d get_sid returns NULL for s_id = %d", file, line, sid);
		return NULL;
	}
	return st[sid];
}

char *describe_streams(sockets *s, char *req, char *sbuf, int size)
{
	char *stream_id, dad[1000], localhost[100];
	int i, sidf, do_play = 0, streams_enabled = 0;
	streams *sid, *sid2;
	int do_all = 1;
	int is_ipv6 = 0;

	if (s->sid == -1 && strchr(req, '?'))
		setup_stream(req, s);

	sidf = get_session_id(s->sid);
	sid = get_sid(s->sid);
	if (sid)
		do_play = sid->do_play;

	get_sock_shost(s->sock, localhost, sizeof(localhost));
	is_ipv6 = strchr(localhost, ':') != NULL;

	snprintf(sbuf, size - 1,
			 "v=0\r\no=- %010d %010d IN %s %s\r\ns=SatIPServer:1 %d,%d,%d\r\nt=0 0\r\n",
			 sidf, sidf, is_ipv6 ? "IP6" : "IP4", localhost,
			 tuner_s2, tuner_t + tuner_t2, tuner_c + tuner_c2);
	if (strchr(req, '?'))
		do_all = 0;

	if ((stream_id = strstr(req, "stream=")))
	{
		do_all = 0;
		sid = get_sid(map_int(stream_id + 7, NULL) - 1);
		if (sid == NULL)
			return NULL;
	}

	if (do_all)
	{
		for (i = 0; i < MAX_STREAMS; i++)
			if ((sid2 = get_sid_for(i)))
			{
				int slen = strlen(sbuf);
				streams_enabled++;
				strlcatf(sbuf, size, slen,
						 "m=video %d RTP/AVP 33\r\nc=IN IP4 0.0.0.0\r\na=control:stream=%d\r\na=fmtp:33 %s\r\na=%s\r\n",
						 get_sockaddr_port(sid2->sa), i + 1,
						 describe_adapter(i, sid2->adapter, dad, sizeof(dad)),
						 sid2->do_play ? "sendonly" : "inactive");
				if (size - slen < 10)
					LOG_AND_RETURN(sbuf, "DESCRIBE BUFFER is full");
			}
	}
	if (!streams_enabled)
	{
		int slen = strlen(sbuf);
		int s_id = 1;
		char *tp;
		if (sid)
		{
			s_id = sid->sid + 1;
			do_play = sid->do_play;
			tp = describe_adapter(sid->sid, sid->adapter, dad, sizeof(dad));
		}
		else
			return NULL;

		strlcatf(sbuf, size, slen,
				 "m=video 0 RTP/AVP 33\r\nc=IN IP4 0.0.0.0\r\na=control:stream=%d\r\na=fmtp:33 %s\r\nb=AS:5000\r\na=%s\r\n",
				 s_id, tp, do_play ? "sendonly" : "inactive");
	}
	return sbuf;
}

// we need to keep the pids from SETUP and PLAY into sid->tp
void set_stream_parameters(int s_id, transponder *t)
{
	streams *sid;

	sid = get_sid(s_id);
	if (!sid || !sid->enabled)
		return;
	if (t->apids && t->apids[0] >= '0' && t->apids[0] <= '9')
	{
		strncpy(sid->apids, t->apids, LEN_PIDS);
		t->apids = sid->apids;
		sid->apids[LEN_PIDS] = 0;
	}
	if (t->dpids && t->dpids[0] >= '0' && t->dpids[0] <= '9')
	{
		strncpy(sid->dpids, t->dpids, LEN_PIDS);
		t->dpids = sid->dpids;
		sid->dpids[LEN_PIDS] = 0;
	}
	if (t->pids && t->pids[0] >= '0' && t->pids[0] <= '9')
	{
		strncpy(sid->pids, t->pids, LEN_PIDS);
		t->pids = sid->pids;
		sid->pids[LEN_PIDS] = 0;
	}

	if (t->x_pmt && t->x_pmt[0] >= '0' && t->x_pmt[0] <= '9')
	{
		strncpy(sid->x_pmt, t->x_pmt, LEN_PIDS);
		t->x_pmt = sid->x_pmt;
		sid->x_pmt[LEN_PIDS] = 0;
	}

	if (!t->apids)
		t->apids = sid->tp.apids;
	if (!t->dpids)
		t->dpids = sid->tp.dpids;
	if (!t->pids)
		t->pids = sid->tp.pids;
	if (!t->x_pmt)
		t->x_pmt = sid->tp.x_pmt;

	copy_dvb_parameters(t, &sid->tp);
}

streams *
setup_stream(char *str, sockets *s)
{
	streams *sid;
	char tmp_str[2000];

	transponder t;
	strncpy(tmp_str, str, sizeof(tmp_str));
	tmp_str[sizeof(tmp_str) - 1] = 0;
	detect_dvb_parameters(str, &t);
	LOG("Setup stream sid %d parameters, sock_id %d, handle %d", s->sid, s->id,
		s->sock);
	if (!get_sid(s->sid)) // create the stream
	{
		int s_id = streams_add();
		if (!(sid = get_sid(s_id)))
			LOG_AND_RETURN(NULL, "Could not add a new stream");

		mutex_lock(&sid->mutex);
		set_sock_lock(s->id, &sid->mutex); // lock the mutex as the sockets_unlock will unlock it
		s->sid = s_id;
		sid->sock = s->sock;

		if (sid->st_sock == -1)
		{

			if (0 > (sid->st_sock = sockets_add(SOCK_TIMEOUT, NULL, sid->sid,
												TYPE_UDP, NULL,
												NULL, (socket_action)stream_timeout)))
				LOG_AND_RETURN(NULL,
							   "sockets_add failed for stream timeout sid %d",
							   sid->sid);
			sockets_timeout(sid->st_sock, 200);
			set_sock_lock(sid->st_sock, &sid->mutex);
		}

		LOG("Setup stream done: sid %d for sock %d handle %d", s_id, s->id,
			s->sock);
	}
	if (!(sid = get_sid(s->sid)))
		LOG_AND_RETURN(NULL, "stream sid %d not enabled for sock_id %d handle %d",
					   s->sid, s->id, s->sock);

	set_stream_parameters(s->sid, &t);
	sid->do_play = 0;

	if (sid->adapter >= 0 && !strncasecmp((const char *)s->buf, "SETUP", 5)) // SETUP after PLAY
	{
		int ad = sid->adapter;
		if (!strstr(tmp_str, "addpids") && !strstr(tmp_str, "delpids"))
		{
			close_adapter_for_stream(sid->sid, ad, 0);
		}
	}

	return sid;
}

int start_play(streams *sid, sockets *s)
{
	int a_id;
	adapter *ad;

	if (sid->type == 0 && s->type == TYPE_HTTP)
	{
		sid->type = STREAM_HTTP;
		sid->rsock = s->sock;
		sid->rsock_id = s->id;
		memcpy(&sid->sa, &s->sa, sizeof(s->sa));
	}

	if (sid->type == 0)
	{
		//			LOG("Assuming RTSP over TCP for stream %d, most likely transport was not specified", sid->sid);
		//			sid->type = STREAM_RTSP_TCP;
		//			sid->rsock = s->sock;
		LOG_AND_RETURN(-454, "No Transport header was specified, for stream sid %d",
					   sid->sid);
	}

	LOG(
		"Play for stream sid %d, type %d, rsock %d, adapter %d, sock_id %d, rsock_id %d, handle %d",
		s->sid, sid->type, sid->rsock, sid->adapter, s->id, sid->rsock_id, s->sock);
	ad = get_adapter(sid->adapter);

	if (compare_tunning_parameters(sid->adapter, &sid->tp)) // close the adapter that is required to be closed
	{
		if (ad && !compare_slave_parameters(ad, &sid->tp))
		{
			close_adapter_for_stream(sid->sid, ad->id, 0);
		}
		ad = get_adapter(sid->adapter);
	}
	// TO DO: if a slave adapter changes the band, pol or diseqc, detach it from the adapter and find a new one for it
	// the same applies to a stream from a master adapter where there are already slave adapters using it.

	// check if the adapter is not valid or if a slave SID is trying to change frequency
	if (!ad || (compare_tunning_parameters(sid->adapter, &sid->tp) && ad->master_sid != sid->sid)) // associate the adapter only at play (not at setup)
	{
		if (sid->tp.sys == 0 && sid->tp.freq == 0) // play streams with no real parameters ==> sending empty packets to the dest
		{
			sid->do_play = 1;
			LOG_AND_RETURN(0, "Tune requested with no real parameters, ignoring ...");
		}
		if (ad)
		{
			LOG("slave stream tuning to a new frequency, finding a new adapter");
			close_adapter_for_stream(sid->sid, ad->id, 0);
		}
		a_id = get_free_adapter(&sid->tp);
		LOG("Got adapter %d on sid %d socket %d", a_id, sid->sid, s->id);
		if (a_id < 0)
			return -404;
		sid->adapter = a_id;
		set_adapter_for_stream(sid->sid, a_id);
	}
	if (set_adapter_parameters(sid->adapter, s->sid, &sid->tp) < 0)
		return -404;

	if (!opts.no_threads && get_socket_thread(sid->st_sock) == get_tid())
	{
		ad = get_adapter(sid->adapter);

		// the stream timeout thread will be running in the same thread with the adapter
		if (ad)
			set_socket_thread(sid->st_sock, get_socket_thread(ad->sock));
	}
	//  flush the sockets buffer if no pid was requested
	if (!sid->tp.apids && sid->tp.pids && (!sid->tp.pids[0] || !strcmp(sid->tp.pids, "0")))
		s->flush_enqued_data = 1;
	sid->do_play = 1;
	if (s->type != TYPE_HTTP)
		sid->start_streaming = 0;
	sid->tp.apids = sid->tp.dpids = sid->tp.pids = sid->tp.x_pmt = NULL;

	ad = get_adapter(sid->adapter);
	if (ad && ad->do_tune)
		s->flush_enqued_data = 1;

	return tune(sid->adapter, s->sid);
}

int close_stream_for_socket(sockets *s)
{
	streams *sid = get_sid(s->sid);
	LOG("%s: start close_stream_for_socket for id %d %p", __FUNCTION__, s->sid, sid);
	if (sid)
		sid->timeout = 1;
	return 0;
}

int close_stream(int i)
{
	int ad;
	streams *sid;
	LOG("closing stream sid %d", i);
	if (i < 0 || i >= MAX_STREAMS || !st[i] || !st[i]->enabled)
		return 0;

	sid = st[i];
	mutex_lock(&sid->mutex);
	if (!sid->enabled)
	{
		adapter_unlock(sid->adapter);
		mutex_unlock(&sid->mutex);
		return 0;
	}
	mutex_lock(&st_mutex);
	sid->enabled = 0;
	sid->start_streaming = 0;
	sid->timeout = 0;
	ad = sid->adapter;
	sid->adapter = -1;
	if (sid->type == STREAM_RTSP_UDP && sid->rsock_id > 0)
	{
		LOG("Closing RTP sock %d handle %d", sid->rsock_id, sid->rsock);
		sockets_del(sid->rsock_id);
	}
	sid->rsock = -1;

	mutex_destroy(&sid->mutex);

	if (sid->rtcp_sock > 0 || sid->rtcp > 0)
	{
		LOG("Closing RTCP sock %d handle %d", sid->rtcp_sock, sid->rtcp);
		sockets_del(sid->rtcp_sock);
		sid->rtcp_sock = -1;
		sid->rtcp = -1;
	}

	if (sid->st_sock > 0)
	{
		sockets_del(sid->st_sock);
		sid->st_sock = -1;
	}

	if (ad >= 0)
		close_adapter_for_stream(i, ad, 1);

	sockets_del_for_sid(i);

	mutex_unlock(&st_mutex);
	LOG("closed stream sid %d", i);
	return 0;
}

int decode_transport(sockets *s, char *arg, char *default_rtp, int start_rtp)
{
	char *arg2[10];
	int l;
	char ra[50];
	rtp_prop p;
	streams *sid = get_sid(s->sid);
	streams *sid2;
	if (!sid)
	{
		LOG("Error: No stream to set transport to, sock_id %d, arg %s ", s->id,
			arg);
		return -1;
	}
	l = 0;
	if (arg)
	{
		if (strstr(arg, "RTP/AVP/TCP"))
		{
			LOG("Assuming RTSP over TCP for stream sid %d, arg %s", sid->sid, arg);
			sid->type = STREAM_RTSP_TCP;
			sid->rsock = s->sock;
			sid->rsock_id = s->id;
			memcpy(&sid->sa, &s->sa, sizeof(s->sa));
			if (!set_linux_socket_nonblock(s->sock))
				s->nonblock = 1;
			set_socket_send_buffer(s->sock, opts.output_buffer);
			return 0;
		}

		l = split(arg2, arg, ARRAY_SIZE(arg2), ';');
	}
	//      LOG("arg2 %s %s %s",arg2[0],arg2[1],arg2[2]);
	memset(&p, 0, sizeof(p));
	while (l > 0)
	{
		l--;
		if (strcmp("multicast", arg2[l]) == 0)
			p.type = TYPE_MCAST;
		if (strcmp("unicast", arg2[l]) == 0)
			p.type = TYPE_UNICAST;
		if (strncmp("ttl=", arg2[l], 4) == 0)
			p.ttl = atoi(arg2[l] + 4);
		if (strncmp("client_port=", arg2[l], 12) == 0)
			p.port = atoi(arg2[l] + 12);
		if (strncmp("port=", arg2[l], 5) == 0)
			p.port = atoi(arg2[l] + 5);
		if (strncmp("destination=", arg2[l], 12) == 0)
			SAFE_STRCPY(p.dest, arg2[l] + 12);
	}
	if (default_rtp)
		SAFE_STRCPY(p.dest, default_rtp);
	if (p.dest[0] == 0 && p.type == TYPE_UNICAST)
		get_sockaddr_host(s->sa, p.dest, sizeof(p.dest) - 1);
	if (p.dest[0] == 0)
		SAFE_STRCPY(p.dest, opts.disc_host);
	if (p.port == 0)
		p.port = start_rtp;
	LOG("decode_transport ->type %d, ttl %d new socket to: %s:%d", p.type,
		p.ttl, p.dest, p.port);
	if (sid->type != 0)
	{
		if (sid->type == STREAM_RTSP_UDP && sid->rsock >= 0)
		{
			int oldport = get_stream_rport(sid->sid);
			char *oldhost = get_stream_rhost(sid->sid, ra, sizeof(ra) - 1);

			if (p.port == oldport && !strcmp(p.dest, oldhost))
				LOG(
					"Transport for this connection is already setup to %s:%d, leaving as it is: sid = %d, handle %d",
					p.dest, p.port, sid->sid, sid->rsock)
			else
			{
				LOG(
					"Stream has already a transport header associated with it to %s:%d - sid = %d type = %d, closing %d",
					oldhost, oldport, sid->sid, sid->type, sid->rsock);
				sockets_del(sid->rsock_id);
				sid->rsock = -1;
				sid->rsock_id = -1;
				sid->type = 0;
			}
		}
	}

	if (sid->type == 0)
	{
		int i;
		USockAddr sa;

		sid->type = STREAM_RTSP_UDP;
		if (sid->rtcp_sock > 0 || sid->rtcp)
		{
			sockets_del(sid->rtcp_sock);
			sid->rtcp_sock = -1;
			sid->rtcp = -1;
		}

		if ((sid->rsock = udp_bind_connect(NULL,
										   opts.start_rtp + (sid->sid * 2), p.dest, p.port, &sid->sa)) < 0)
			LOG_AND_RETURN(-1,
						   "decode_transport failed: UDP connection on rtp port to %s:%d failed",
						   p.dest, p.port);

		if ((sid->rsock_id = sockets_add(sid->rsock, &sid->sa, sid->sid, TYPE_UDP, NULL, (socket_action)close_stream_for_socket, NULL)) < 0)
			LOG_AND_RETURN(-1, "RTP sockets_add failed");

		set_socket_send_buffer(sid->rsock, opts.output_buffer);
		set_socket_dscp(sid->rsock, IPTOS_DSCP_EF, 7);

		if ((sid->rtcp = udp_bind_connect(NULL,
										  opts.start_rtp + (sid->sid * 2) + 1, p.dest, p.port + 1, &sa)) < 1)
			LOG_AND_RETURN(-1,
						   "decode_transport failed: UDP connection on rtcp port to %s:%d failed",
						   p.dest, p.port + 1);

		set_linux_socket_timeout(sid->rtcp);
		set_socket_dscp(sid->rtcp, IPTOS_DSCP_EF, 6);

		if ((sid->rtcp_sock = sockets_add(sid->rtcp, &sa, sid->sid, TYPE_RTCP,
										  (socket_action)rtcp_confirm, NULL, NULL)) < 0) // read rtcp
			LOG_AND_RETURN(-1, "RTCP sockets_add failed");

		for (i = 0; i < MAX_STREAMS; i++)
			if ((sid2 = get_sid_for(i)) && i != sid->sid && get_sockaddr_port(sid2->sa) == get_sockaddr_port(sid->sa))
			{
				char h1[100], h2[100];
				get_sockaddr_host(sid->sa, h1, sizeof(h1));
				get_sockaddr_host(sid2->sa, h2, sizeof(h2));
				if (!strncmp(h1, h2, sizeof(h1)))
				{
					LOG("Detected stream with the same destination as sid %d: sid %d -> %s:%d, aid: %d",
						sid->sid, i, p.dest, p.port, sid2->adapter);
					close_stream(i);
				}
			}
	}

	return 0;
}

int streams_add()
{
	int i;
	streams *ss;
	i = add_new_lock((void **)st, MAX_STREAMS, sizeof(streams), &st_mutex);
	if (i == -1)
		LOG_AND_RETURN(-1, "streams_add failed");

	ss = st[i];
	ss->enabled = 1;
	ss->adapter = -1;
	ss->sid = i;
	ss->rsock = -1;
	ss->rsock_id = -1;
	ss->type = 0;
	ss->do_play = 0;
	ss->sp = ss->sb = 0;
	init_dvb_parameters(&ss->tp);
	ss->useragent[0] = 0;
	ss->len = 0;
	ss->st_sock = -1;
	//	ss->seq = 0; // set the sequence to 0 for testing purposes - it should be random
	ss->ssrc = random();
	ss->timeout = opts.timeout_sec;
	ss->wtime = ss->rtcp_wtime = getTick();

	mutex_unlock(&ss->mutex);
	return i;
}

int
// close all streams for adapter, excepting except
close_streams_for_adapter(int ad, int except)
{
	int i;
	streams *sid;
	if (ad < 0)
		return 0;
	for (i = 0; i < MAX_STREAMS; i++)
		if ((sid = get_sid_for(i)) && sid->adapter == ad)
			if (except < 0 || except != i)
				close_stream(i);
	return 0;
}

int64_t tbw, bw, bwtt;
uint32_t reads, writes, failed_writes;
int64_t nsecs;

int64_t c_tbw, c_bw;
uint32_t c_reads, c_writes, c_failed_writes;
int64_t c_ns_read, c_tt;

uint64_t last_sd;

int enqueue_rtp_header(streams *sid, struct iovec *iov, int liov, int iiov_rtp_header, char *rtp_buf)
{
	int i, total_len = 0, len = 0;
	struct timespec ts;
	uint32_t timestamp;

	for (i = iiov_rtp_header + 1; i < liov; i++)
		total_len += iov[i].iov_len;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	timestamp = (uint32_t)(90000 * ((ts.tv_sec * 1000000ll + ts.tv_nsec / 1000) / 1000000ll)) + (9 * ((ts.tv_sec * 1000000ll + ts.tv_nsec / 1000) % 1000000ll)) / 100; // 90 kHz Clock

	if (sid->type == STREAM_RTSP_TCP)
	{
		rtp_buf[0] = 0x24;
		rtp_buf[1] = 0;
		copy16(rtp_buf, 2, total_len + 12);
		len = 4;
	}

	copy16(rtp_buf, len + 0, 0x8021);
	copy16(rtp_buf, len + 2, sid->seq);
	copy32(rtp_buf, len + 4, timestamp);
	copy32(rtp_buf, len + 8, sid->ssrc);
	len += 12;
	iov[iiov_rtp_header].iov_base = rtp_buf;
	iov[iiov_rtp_header].iov_len = len;
	sid->seq = (sid->seq + 1) & 0xFFFF; // rollover

	return len;
}

int send_rtcp(int s_id, int64_t ctime)
{
	int len, rv = 0;
	int total_len = 0;
	char dad[1000];
	char ra[50];
	unsigned char rtcp_buf[1600];
	unsigned char *rtcp = rtcp_buf + 4;
	streams *sid = get_sid(s_id);
	unsigned long long ntp;
	struct timeval tv;

	if (!sid)
		LOG_AND_RETURN(0, "Stream sid is null for s_id %d", s_id);

	gettimeofday(&tv, NULL);
	ntp = (((unsigned long long)tv.tv_sec + 2208988800ULL) << 32) |
		  (((unsigned long long)tv.tv_usec << 32) / 1000000ULL);

	char *a = describe_adapter(s_id, sid->adapter, dad, sizeof(dad));
	unsigned int la = strlen(a);
	if (la > sizeof(rtcp_buf) - 78)
		la = sizeof(rtcp_buf) - 80;
	len = la + 16;
	if (len % 4 > 0)
		len = len - (len % 4) + 4;
	//      LOG("send_rtcp (sid: %d)-> %s",s_id,a);
	// rtp header
	rtcp[0] = 0x80; // Begin Sender Report
	rtcp[1] = 0xC8;
	rtcp[2] = 0;
	rtcp[3] = 6;
	copy32(rtcp, 4, sid->ssrc);
	copy32(rtcp, 8, (int)(ntp >> 32));
	copy32(rtcp, 12, ctime);
	copy32(rtcp, 16, sid->wtime);
	copy32(rtcp, 20, sid->sp);
	copy32(rtcp, 24, sid->sb);
	rtcp[28] = 0x81; // Begin Source Description
	rtcp[29] = 0xCA;
	rtcp[30] = 0;
	rtcp[31] = 5;
	copy32(rtcp, 32, sid->ssrc);
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
	copy32(rtcp, 47, 0);
	rtcp[51] = 0;
	rtcp[52] = 0x80;
	rtcp[53] = 0xCC;
	copy16(rtcp, 54, ((la + 16 + 3) / 4) - 1);
	copy32(rtcp, 56, sid->ssrc);
	rtcp[60] = 'S';
	rtcp[61] = 'E';
	rtcp[62] = 'S';
	rtcp[63] = '1';
	rtcp[64] = 0;
	rtcp[65] = 0;
	copy16(rtcp, 66, la);
	memcpy(rtcp + 68, a, la + 4);
	total_len = len + 52;
	if (sid->type == STREAM_RTSP_UDP)
		rv = sockets_write(sid->rtcp_sock, rtcp, total_len);
	else
	{
		rtcp_buf[0] = 0x24;
		rtcp_buf[1] = 1;
		copy16(rtcp_buf, 2, total_len);
		total_len += 4;
		rv = sockets_write(sid->rsock_id, rtcp_buf, total_len);
	}

	sid->rtcp_wtime = ctime;
	DEBUGM("%s: sent %d bytes for sid %d, handle %d seq %d => %s:%d",
		   __FUNCTION__, total_len, sid->sid, sid->rsock, sid->seq - 1,
		   get_stream_rhost(sid->sid, ra, sizeof(ra)), get_stream_rport(sid->sid));

	//	sid->sp = 0;
	//	sid->sb = 0;
	return rv;
}

int flush_stream(streams *sid, struct iovec *iov, int iiov, int64_t ctime)
{
	int rv = 0;
	char ra[50];
	rv = sockets_writev(sid->rsock_id, iov, iiov);
	if (rv > 0 && sid->start_streaming == 0)
	{
		sid->start_streaming = 1;
		LOG("Start streaming for stream sid %d, len %d to handle %d => %s:%d",
			sid->sid, rv, sid->rsock,
			get_stream_rhost(sid->sid, ra, sizeof(ra)),
			get_stream_rport(sid->sid));
	}

	DEBUGM("%s: sent %d bytes for sid %d, handle %d, sock_id %d, seq %d => %s:%d",
		   __FUNCTION__, rv, sid->sid, sid->rsock, sid->rsock_id, sid->seq,
		   get_stream_rhost(sid->sid, ra, sizeof(ra)), get_stream_rport(sid->sid));

#ifdef DEBUG
	static int fd, freq, pid;
	unsigned char *b, fn[50];
	sprintf(fn, "freq=%d.ts", sid->tp.freq / 1000);
	SPid *p;
	fd = open(fn, O_WRONLY);
	if (fd < 0)
		fd = open(fn, O_CREAT | O_WRONLY, 0666);
	if (fd)
	{
		lseek(fd, 0, 2);
		writev(fd, iov, iiov);
		close(fd);
	}
#endif
	sid->wtime = ctime;
	sid->len = 0;

	if (rv > 0)
	{
		sid->sp++;
		sid->sb += rv;
	}
	return rv;
}

#undef DEFAULT_LOG
#define DEFAULT_LOG LOG_DMX

int check_new_transponder(adapter *ad, int rlen)
{
	unsigned char *b;
	int tid = 0;
	int i;

	for (i = 0; i < rlen; i += 188)
	{
		b = ad->buf + i;
		if (b[0] == 0x47 && b[1] == 0x40 && b[2] == 0) // pid 0, calculate transponder_id
		{
			tid = b[8] * 256 + b[9];
			if (tid != ad->wait_transponder_id)
			{
				ad->wait_transponder_id = tid;
				LOG("Got the new transponder %04X %d, position %d, %jd ms after tune", tid, tid, i, getTick() - ad->tune_time);
				memmove(ad->buf, ad->buf + i, rlen - i);
				return rlen - i;
			}
			else
				LOGM("Got old transponder id %04X %d, position %d, %jd ms after tune", tid, tid, i, getTick() - ad->tune_time);
		}
	}
	return 0;
}

int check_cc(adapter *ad)
{
	int i;
	char cc, cc_before;
	SPid *p;
	int pid;
	int packet_no_sid = 0;
	unsigned char *b;

	if ((p = find_pid(ad->id, 8192)))
		return 0;

	for (i = 0; i < ad->rlen; i += DVB_FRAME)
	{
		b = ad->buf + i;
		if (b[1] & 0x80)
			continue;

		pid = PID_FROM_TS(b);
		if (pid == 8191)
			continue;

		if ((opts.debug & LOG_DMX) == LOG_DMX)
			_dump_packets("check_cc -> ", b, 188, i);

		p = find_pid(ad->id, pid);

		if ((!p))
		{
			LOGM("%s: pid %03d not found", __FUNCTION__, pid);
			ad->pid_err++;
			packet_no_sid++;
			continue;
		}

		p->packets++;

		if (b[3] & 0x10)
		{
			cc_before = p->cc;
			cc = b[3] & 0xF;
			if (p->cc < 0 || p->cc > 15)
				p->cc = cc;
			else
				p->cc = (p->cc + 1) % 16;

			//	if(b[1] ==0x40 && b[2]==0) LOG("PAT TID = %d", b[8] * 256 + b[9]);
			if (p->cc != cc)
			{
				LOG("PID Continuity error (adapter %d, pos %d): pid: %03d, Expected CC: %X, Actual CC: %X, CC Before %X",
					ad->id, i / DVB_FRAME, pid, p->cc, cc, cc_before);
				p->cc_err++;
			}
			p->cc = cc;
		}
		if (!VALID_SID(p->sid[0]))
			packet_no_sid++;
#ifdef CRC_TS
		if (p)
		{
			char buf[1024];
			int len = 0, i;
			uint32_t crc = crc_32(b, 188);
			p->crc ^= crc;
			if (p->count + 1 >= CRC_TS)
			{
				hexdump("packet", b, 188);
				LOG("pid %d cnt %d count %d cc %d err %d crc %08X ", p->pid, p->packets, p->count, p->cc, p->cc_err, p->crc);
				//			LOG("count %d", p->cnt);
				p->count = 0;
				p->cc_crc = 0;
			}

			if ((p->count < 15) && (p->cc == 0))
				p->crc = 0;
			if (p->cc == 0 && ((CRC_TS % 16) == 0))
			{
				LOG("Reseting count from %d, CRC_TS %d", p->count, CRC_TS);
				p->count = 0;
			}
			p->count++;
		}
#endif
	}
	return packet_no_sid;
}

int process_packets_for_stream(streams *sid, adapter *ad)
{
	int i, j, st_id = sid->sid;
	SPid *p;
	uint8_t *b;
	int max_iov = TCP_MAX_IOV;
	char pids[8193];
	struct iovec iov[max_iov + 1];
	char rtp_buf[16 * (max_iov + 1)];
	int rtp_pos = 0;
	int iiov = 1;
	int num_enabled_pids = 0;
	int last_rtp_header = 0;
	int64_t rtime = ad->rtime;
	int total_len = 0;
	int max_pack = TCP_MAX_PACK;

	memset(iov, 0, sizeof(iov));
	memset(pids, 0, sizeof(pids));

	// cache the pids for this stream
	for (i = 0; i < MAX_PIDS; i++)
		if (ad->pids[i].flags == 1)
		{
			p = &ad->pids[i];
			for (j = 0; j < MAX_STREAMS_PER_PID && p->sid[j] > -1; j++)
				if (p->sid[j] == st_id)
				{
					pids[p->pid] = 1;
					num_enabled_pids++;
				}
		}

	if (sid->type == STREAM_RTSP_UDP)
	{
		max_pack = UDP_MAX_PACK;
		//		max_iov = max_pack;
	}

	if (sid->type == STREAM_HTTP)
	{
		iiov = 0;
		max_pack = 0;
	}

	for (i = 0; i < ad->rlen; i += DVB_FRAME)
	{
		int rtp_added = 0;
		b = ad->buf + i;
		if (b[0] != 0x47)
		{
			LOG("Non TS packet found %02X", b[0]);
			continue;
		}
		int _pid = PID_FROM_TS(b);
		if (!pids[_pid] && !pids[8192])
			continue;

		if (total_len && max_pack && (total_len / DVB_FRAME % max_pack == 0))
		{
			rtp_pos += enqueue_rtp_header(sid, iov, iiov, last_rtp_header, rtp_buf + rtp_pos);
			last_rtp_header = iiov;
			iiov++;
			rtp_added = 1;
		}

		// unlikely: if the rtp header was just enqueued try to flush if there is not enough iiov left
		if ((rtp_added || !max_pack) && (iiov >= max_iov))
		{
			LOG("stream sid %d, flushing intermediary stream iiov %d max_iiov %d, total_len %d", st_id, iiov - 1, max_iov, total_len);
			// iiov was incremented previously
			flush_stream(sid, iov, max_pack ? iiov - 1 : iiov, rtime);
			iiov = max_pack ? 1 : 0;
			last_rtp_header = 0;
			rtp_pos = 0;
			total_len = 0;
		}

		total_len += DVB_FRAME;
		// try to increase iov_len if the previous packet ends before the currnet one
		if (iiov - 1 >= 0 && iov[iiov - 1].iov_base + iov[iiov - 1].iov_len == b)
			iov[iiov - 1].iov_len += DVB_FRAME;
		else
		{
			iov[iiov].iov_base = b;
			iov[iiov++].iov_len = DVB_FRAME;
		}
	}

	if ((sid->type == STREAM_RTSP_UDP || sid->type == STREAM_RTSP_TCP))
		enqueue_rtp_header(sid, iov, iiov, last_rtp_header, rtp_buf + rtp_pos);

	LOGM("Processing done sid %d at pos %d, rtp header %d, rtp_pos %d, total_len %d, rlen %d", st_id, iiov, last_rtp_header, rtp_pos, total_len, ad->rlen);
	flush_stream(sid, iov, iiov, rtime);
	return 0;
}

int process_dmx(sockets *s)
{
	int i;
	adapter *ad;
	int64_t stime;
	int rlen = s->rlen;
	s->rlen = 0;

	ad = get_adapter(s->sid);
	if (!ad)
		return 0;

	int64_t ms_ago = s->rtime - ad->rtime;
	ad->rtime = s->rtime;
	ad->rlen = rlen;
	stime = getTickUs();

	LOGM("process_dmx start called for adapter %d -> %d out of %d bytes read, %jd ms ago",
		 ad->id, rlen, s->lbuf, ms_ago);

#ifndef DISABLE_T2MI
	if (ad->is_t2mi >= 0)
		t2mi_process_ts(ad);
#endif

#ifndef DISABLE_TABLES
	pmt_process_stream(ad);
#endif

	rlen = ad->rlen;
#ifndef AXE
	check_cc(ad);
#endif

	for (i = 0; i < MAX_STREAMS; i++)
		if (st[i] && st[i]->enabled && st[i]->adapter == ad->id)
			process_packets_for_stream(st[i], ad);

	if (s->rtime - ad->last_sort > 2000)
	{
		ad->last_sort = s->rtime + 60000;
		sort_pids(s->sid);
	}

	nsecs += getTickUs() - stime;
	reads++;
	//      if(!found)LOG("pid not found = %d -> 1:%d 2:%d 1&1f=%d",pid,s->buf[1],s->buf[2],s->buf[1]&0x1f);
	//      LOG("done send stream");
	return 0;
}

// lock order: socket -> stream -> adapter
// after stream or adapter, avoid locking socket

int read_dmx(sockets *s)
{
	static int cnt;
	adapter *ad;
	int send = 0, flush_all = 0, ls, lse;
	int threshold = opts.udp_threshold;
	int64_t rtime = getTick();

	if (s->rlen % DVB_FRAME != 0)
		//		s->rlen = ((int) s->rlen / DVB_FRAME) * DVB_FRAME;
		return 0;

	if (s->rlen == s->lbuf)
		cnt++;
	else
		cnt = 0;
	ad = get_adapter(s->sid);
	if (!ad)
	{
		s->rlen = 0;
		return 0;
	}

	threshold = ad->threshold;

	// flush buffers every 50ms or the first 1000 packets (for PAT and PMT processing)
	if (rtime - ad->rtime > threshold)
	{
		flush_all = 1; // flush everything that we've read so far
		send = 1;
	}

	if (s->lbuf - s->rlen <= 7 * DVB_FRAME)
		send = 1;

	if (ad->wait_new_stream && !ad->tune_time)
		ad->tune_time = rtime;

	if (ad->wait_new_stream && (rtime - ad->tune_time < 200)) // check new transponder
	{
		int new_rlen = check_new_transponder(ad, s->rlen);
		if (!new_rlen)
		{
			LOGM("Flushing adapter %d buffer of %d bytes after the tune %jd ms ago", ad->id, s->rlen, rtime - ad->tune_time);
			s->rlen = 0;
			return 0;
		}
		s->rlen = new_rlen;
	}
	ad->tune_time = 0;
	ad->wait_new_stream = 0;

	if (ad->flush)
		send = 1;

	LOGM("read_dmx send=%d, flush_all=%d, cnt %d called for adapter %d -> %d out of %d bytes read, %jd ms ago (%jd %jd)",
		 send, flush_all, cnt, ad->id, s->rlen, s->lbuf, rtime - ad->rtime, rtime, ad->rtime);

	if (!send)
		return 0;

	ad->flush = 0;
	ls = lock_streams_for_adapter(ad->id);
	adapter_lock(ad->id);
	process_dmx(s);
	adapter_unlock(ad->id);
	lse = unlock_streams_for_adapter(ad->id);
	if (ls != lse)
		LOG("leak detected %d %d!!! ", ls, lse);
	return 0;
}
#undef DEFAULT_LOG
#define DEFAULT_LOG LOG_STREAM

int calculate_bw(sockets *s)
{
	int64_t c_time = getTick();
	s->rtime = c_time;

	if (bwtt > c_time)
		bwtt = c_time;
	if (c_time - bwtt > 1000)
	{
		bwtt = c_time;
		tbw += bw;
		if (!reads)
			reads = 1;
		if (bw > 2000)
		{
			mutex_init(&bw_mutex);
			mutex_lock(&bw_mutex);
			c_bw = bw / 1024;
			c_tbw = tbw / 1024576;
			c_ns_read = nsecs / reads;
			c_reads = reads;
			c_writes = writes;
			c_failed_writes = failed_writes;
			c_tt = nsecs / 1000;
			LOG(
				"BW %jdKB/s, Total BW: %jd MB, ns/read %jd, r: %d, w: %d fw: %d, tt: %jd ms",
				c_bw, c_tbw, c_ns_read, c_reads, c_writes, c_failed_writes, c_tt);
			mutex_unlock(&bw_mutex);
		}
		bw = 0;
		failed_writes = 0;
		nsecs = 0;
		reads = 0;
		writes = 0;
	}
	join_thread();
	return 0;
}

int stream_timeout(sockets *s)
{
	char ra[50];
	struct iovec iov[10];
	char rtp_buf[20];
	int64_t ctime, rttime, rtime;
	streams *sid;

	ctime = getTick();
	s->rtime = ctime;

	if ((sid = get_sid_for(s->sid)) && sid->type != STREAM_HTTP)
	{
		mutex_lock(&sid->mutex);
		rttime = sid->rtcp_wtime, rtime = sid->wtime;

		if (sid->do_play && ctime - rtime > 1000)
		{
			LOG("no data sent for more than 1s sid: %d for %s:%d", sid->sid,
				get_stream_rhost(sid->sid, ra, sizeof(ra)),
				get_stream_rport(sid->sid));
			enqueue_rtp_header(sid, iov, 1, 0, rtp_buf);
			flush_stream(sid, iov, 1, ctime);
		}
		if (sid->do_play && ctime - rttime >= 200)
			send_rtcp(sid->sid, ctime);
		mutex_unlock(&sid->mutex);
		// check stream timeout, and allow 10s more to respond
		if ((sid->timeout > 0 && (ctime - sid->rtime > sid->timeout + 10000)) || (sid->timeout == 1))
		{
			LOG(
				"Stream timeout sid %d, closing (ctime %jd , sid->rtime %jd, sid->timeout %d)",
				sid->sid, ctime, sid->rtime, sid->timeout);
			close_stream(sid->sid); // do not lock before this
		}
	}

	return 0;
}

void dump_streams()
{
	int i;
	char ra[50];
	streams *sid;
	if (!opts.log)
		return;
	LOG("Dumping streams:");
	for (i = 0; i < MAX_STREAMS; i++)
		if ((sid = get_sid_for(i)))
			LOG("%d|  a:%d rsock:%d type:%d play:%d remote:%s:%d", i,
				sid->adapter, sid->rsock, sid->type, sid->do_play,
				get_stream_rhost(sid->sid, ra, sizeof(ra)),
				get_stream_rport(sid->sid));
}

int lock_streams_for_adapter(int aid)
{
	streams *sid;
	int i = 0, ls = 0;
	for (i = 0; i < MAX_STREAMS; i++)
		if ((sid = get_sid_for(i)) && sid->adapter == aid)
		{
			mutex_lock(&sid->mutex);
			if ((sid = get_sid_for(i)) && (sid->adapter != aid))
				mutex_unlock(&sid->mutex);
			else
				ls++;
		}
	return ls;
}

int unlock_streams_for_adapter(int aid)
{
	streams *sid;
	int i = 0, ls = 0;
	for (i = MAX_STREAMS - 1; i >= 0; i--)
		if ((sid = get_sid_for(i)) && sid->adapter == aid)
		{
			mutex_unlock(&sid->mutex);
			ls++;
		}
	return ls;
}

void free_all_streams()
{
	int i;

	for (i = 0; i < MAX_STREAMS; i++)
	{
		if (st[i])
			free1(st[i]);
		st[i] = NULL;
	}
}

int get_session_id(int i)
{
	streams *sid = get_sid(i);
	if (!sid)
		return 0;
	return sid->ssrc;
}

void set_session_id(int i, int id)
{
	streams *sid = get_sid(i);
	if (!sid)
		return;
	if (sid->ssrc != id)
	{
		LOG("Forcing session id %d on stream sid %d", id, i);
		sid->ssrc = id;
	}
}

int fix_master_sid(int a_id)
{
	int i;
	adapter *ad;
	streams *sid;
	ad = get_adapter(a_id);

	if (!ad || ad->master_sid != -1)
		return 0;
	if (ad->sid_cnt < 1)
		return 0;
	for (i = 0; i < MAX_STREAMS; i++)
		if ((sid = get_sid_for(i)) && sid->adapter == a_id)
		{
			LOG("fix master_sid to %d for adapter %d", sid->sid, a_id);
			ad->master_sid = i;
		}
	return 0;
}

int find_session_id(int id)
{
	int i;
	streams *sid;
	for (i = 0; i < MAX_STREAMS; i++)
		if ((sid = get_sid_for(i)) && sid->ssrc == id)
		{
			sid->rtime = getTick();
			LOG(
				"recovered session id from a closed connection, sid %d , id: %d",
				i, id);
			return i;
		}
	return -1;
}

int rtcp_confirm(sockets *s)
{
	streams *sid;
	char ra[50];

	sid = get_sid(s->sid);
	if (!sid)
		return 0;
	LOG("%s: called for stream sid %d from %s:%d",
		__FUNCTION__, s->sid, get_stream_rhost(sid->sid, ra, sizeof(ra) - 1), get_stream_rport(sid->sid) + 1);

	LOGM("Acknowledging stream sid %d via rtcp packet", s->sid);
	sid->rtime = s->rtime;
	return 0;
}

int get_streams_for_adapter(int aid)
{
	int i, sa = 0;
	streams *sid;
	for (i = 0; i < MAX_STREAMS; i++)
		if ((sid = get_sid_for(i)) && sid->adapter == aid)
			sa++;
	return sa;
}

char *get_stream_rhost(int s_id, char *dest, int ld)
{
	streams *sid = get_sid_nw(s_id);
	dest[0] = 0;
	if (!sid)
		return dest;
	return get_sockaddr_host(sid->sa, dest, ld);
}

int get_stream_rport(int s_id)
{
	streams *sid = get_sid_nw(s_id);
	if (!sid)
		return 0;
	return get_sockaddr_port(sid->sa);
}

char *get_stream_pids(int s_id, char *dest, int max_size)
{
	int len = 0;
	int i, j;
	streams *s = get_sid_nw(s_id);
	adapter *ad;
	dest[0] = 0;

	if (!s)
		return dest;

	ad = get_adapter(s->adapter);

	if (!ad)
		return dest;

	for (i = 0; i < MAX_PIDS; i++)
		if (ad->pids[i].flags == 1)
			for (j = 0; j < MAX_STREAMS_PER_PID; j++)
				if (ad->pids[i].sid[j] == s_id)
				{
					if (ad->pids[i].pid == 8192)
						strlcatf(dest, max_size, len, "all,");
					else
						strlcatf(dest, max_size, len, "%d,", ad->pids[i].pid);
				}
	if (len > 0)
	{
		len--;
		dest[len] = 0;
	}
	return dest;
}

int get_stream_overflow(int s_id)
{
	streams *sid = get_sid_nw(s_id);
	if (!sid)
		return 0;
	sockets *s = get_sockets(sid->rsock_id);
	return s ? s->overflow : -1;
}

int get_stream_buffered_size(int s_id)
{
	int i, bytes = 0;
	streams *sid = get_sid_nw(s_id);
	if (!sid)
		return 0;
	sockets *s = get_sockets(sid->rsock_id);
	if (!s || s->spos == s->wpos || !s->pack)
		return 0;
	for (i = s->spos; i != s->wpos; i = (i + 1) % s->wmax)
		bytes += s->pack[i].len;
	return bytes;
}
_symbols stream_sym[] =
	{
		{"st_enabled", VAR_AARRAY_INT8, st, 1, MAX_STREAMS, offsetof(streams, enabled)},
		{"st_play", VAR_AARRAY_INT, st, 1, MAX_STREAMS, offsetof(streams, do_play)},
		{"st_adapter", VAR_AARRAY_INT, st, 1, MAX_STREAMS, offsetof(streams, adapter)},
		{"st_useragent", VAR_AARRAY_STRING, st, 1, MAX_STREAMS, offsetof(streams, useragent)},
		{"st_rhost", VAR_FUNCTION_STRING, (void *)&get_stream_rhost, 0, MAX_STREAMS, 0},
		{"st_rport", VAR_FUNCTION_INT, (void *)&get_stream_rport, 0, MAX_STREAMS, 0},
		{"st_pids", VAR_FUNCTION_STRING, (void *)&get_stream_pids, 0, MAX_STREAMS, 0},
		{"st_overflow", VAR_FUNCTION_INT, (void *)&get_stream_overflow, 0, MAX_STREAMS, 0},
		{"st_buffered", VAR_FUNCTION_INT, (void *)&get_stream_buffered_size, 0, MAX_STREAMS, 0},
		{NULL, 0, NULL, 0, 0, 0}};
