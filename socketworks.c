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

#include "socketworks.h"
#include "minisatip.h"

extern struct struct_opts opts;
sockets s[MAX_SOCKS];
int max_sock;
struct pollfd pf[MAX_SOCKS];

int fill_sockaddr(struct sockaddr_in *serv, char *host, int port)
{
	struct hostent *h;

	if (host)
	{
		h = gethostbyname(host);
		if (h == NULL)
		{
			LOG("fill_sockaddr: gethostbyname(%s): %s", host, strerror(errno));
			return 0;
		}
	}
	memset(serv, 0, sizeof(struct sockaddr_in));
	serv->sin_family = AF_INET;
	if (host)
		memcpy(&serv->sin_addr.s_addr, h->h_addr, h->h_length);
		else
		serv->sin_addr.s_addr = htonl(INADDR_ANY);
	serv->sin_port = htons(port);
	return 1;
}

char localip[MAX_HOST];
char *
getlocalip()
{
	//      if(localip[0]!=0)return localip;

	const char *dest = opts.disc_host, *h;
	int port = 1900;

	struct sockaddr_in serv;

	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	//Socket could not be created
	if (sock < 0)
	{
		LOG("getlocalip: Cannot create socket: %s", strerror(errno));
		return localip;
	}

	fill_sockaddr(&serv, (char *) dest, port);
	int err = connect(sock, (const struct sockaddr *) &serv, sizeof(serv));
	if (err)
	{
		LOG("getlocalip: Error '%s'' during connect", strerror(errno));
		memset(localip, 0, sizeof(localip));
	}
	else
	{
		h = get_sock_shost(sock);
		if (h)
			strcpy(localip, h);
	}
	close(sock);
	return localip;

}

int udp_bind(char *addr, int port)
{
	struct sockaddr_in serv;
	int sock, optval = 1;

	if (!fill_sockaddr(&serv, addr, port))
		return -1;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
	{
		LOGL(0, "udp_bind failed: socket(): %s", strerror(errno));
		return -1;
	}

	if (addr && atoi(addr) >= 239)
	{
		struct ip_mreq mreq;

		mreq.imr_multiaddr.s_addr = inet_addr(addr);
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		LOG("setting multicast for %s", addr);
		if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq))
				== -1)
		{
			LOGL(0, "membership error: %s", strerror(errno));
		}
	}
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
	{
		LOGL(0, "udp_bind failed: setsockopt(SO_REUSEADDR): %s",
				strerror(errno));
		return -1;
	}

	if (bind(sock, (struct sockaddr *) &serv, sizeof(serv)) < 0)
	{
		LOGL(0, "udp_bind: failed: bind() on host %s port %d: error %s", addr,
				port, strerror(errno));
		return -1;
	}
	LOGL(0, "New UDP socket %d bound to %s:%d", sock, inet_ntoa(serv.sin_addr),
			ntohs(serv.sin_port));
	return sock;
}

int udp_bind_connect(char *src, int sport, char *dest, int dport,
		struct sockaddr_in *serv)
{
	int sock;
	sock = udp_bind(src, sport);
	if (sock < 0)
		return sock;
	fill_sockaddr(serv, dest, dport);
	if (connect(sock, (struct sockaddr *) serv, sizeof(*serv)) < 0)
	{
		LOGL(0, "udp_bind_connect: failed: bind(): %s", strerror(errno));
		return -1;
	}
	LOG("New UDP socket %d connected to %s:%d", sock, inet_ntoa(serv->sin_addr),
			ntohs(serv->sin_port));

	return sock;
}

int udp_connect(char *addr, int port, struct sockaddr_in *serv)
{
	struct sockaddr_in sv;
	int sock, optval = 1;

	if (serv == NULL)
		serv = &sv;
	if (!fill_sockaddr(serv, addr, port))
		return -1;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
	{
		LOGL(0, "udp_connect failed: socket() %s", strerror(errno));
		return -1;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
	{
		LOGL(0, "udp_bind: setsockopt(SO_REUSEADDR): %s", strerror(errno));
		return -1;
	}

	if (connect(sock, (struct sockaddr *) serv, sizeof(*serv)) < 0)
	{
		LOGL(0, "udp_connect: failed: bind(): %s", strerror(errno));
		return -1;
	}
	LOG("New UDP socket %d connected to %s:%d", sock, inet_ntoa(serv->sin_addr),
			ntohs(serv->sin_port));
	return sock;
}

int set_tcp_socket_timeout(int sockfd)
{
	struct timeval timeout;      
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
        LOG("setsockopt failed for socket %d", sockfd);

    if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
        LOG("setsockopt failed for socket %d", sockfd);

}


int tcp_connect(char *addr, int port, struct sockaddr_in *serv, int blocking)
{
	struct sockaddr_in sv;
	int sock, optval = 1;

	if (serv == NULL)
		serv = &sv;
	if (!fill_sockaddr(serv, addr, port))
		return -1;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		LOGL(0, "tcp_connect failed: socket() %s", strerror(errno));
		return -1;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
	{
		LOGL(0, "tcp_connect: setsockopt(SO_REUSEADDR): %s", strerror(errno));
		close(sock);
		return -1;
	}
	if (blocking)
	{
		int flags = fcntl(sock, F_GETFL, 0);
		fcntl(sock, F_SETFL, flags | O_NONBLOCK);
	}

	if (connect(sock, (struct sockaddr *) serv, sizeof(*serv)) < 0)
	{
		LOGL(0, "tcp_connect: failed: connect to %s:%d failed: %s", addr, port,
				strerror(errno));
		if (errno != EINPROGRESS)
		{
			close(sock);
			return -1;
		}
	}
	set_tcp_socket_timeout(sock);
	LOG("New TCP socket %d connected to %s:%d", sock, addr, port);
	return sock;
}

int tcp_listen(char *addr, int port)
{
	struct sockaddr_in serv;
	int sock, optval = 1;

	if (!fill_sockaddr(&serv, addr, port))
		return -1;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		LOGL(0, "tcp_listen failed: socket(): %s", strerror(errno));
		return -1;
	}
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
	{
		LOGL(0, "tcp_listen failed: setsockopt(SO_REUSEADDR): %s",
				strerror(errno));
		return -1;
	}

	if (bind(sock, (struct sockaddr *) &serv, sizeof(serv)) < 0)
	{
		LOGL(0, "tcp_listen: failed: bind() on address: %s, port %d : error %s",
				addr ? addr : "ANY", port, strerror(errno));
		return -1;
	}
	if (listen(sock, 10) < 0)
	{
		LOGL(0, "tcp_listen: listen(): %s", strerror(errno));
		return -1;
	}
	return sock;
}

char *get_sock_shost(int fd)
{
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	getsockname(fd, (struct sockaddr *) &sin, &len);
	return inet_ntoa(sin.sin_addr);
}

int get_sock_sport(int fd)
{
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	getsockname(fd, (struct sockaddr *) &sin, &len);
	return ntohs(sin.sin_port);
}

int no_action(int s)
{
	return 1;
}

int sockets_accept(int socket, void *buf, int len, sockets *ss)
{
	int new_sock, sas, ni;
	struct sockaddr_in sa;
	sas = sizeof(sa);
	new_sock = accept(ss->sock, (struct sockaddr *) &sa, &sas);
	if (new_sock < 0)
	{
		if (errno != EINTR)
		{
			LOG("sockets_accept: accept()");
			return 1;
		}
	}
	ni = sockets_add(new_sock, &sa, -1, TYPE_TCP, NULL, NULL, NULL);
	if (ss->action != NULL)
	{
		ss->action(&s[ni]);
	}
	set_tcp_socket_timeout(new_sock);
	
	return 1;
}

int sockets_read(int socket, void *buf, int len, sockets *ss, int *rv)
{
	*rv = read(socket, buf, len);
	return (*rv > 0);
}

int sockets_recv(int socket, void *buf, int len, sockets *ss, int *rv)
{
	int slen = sizeof(ss->sa);
	*rv = recvfrom(socket, buf, len, 0, (struct sockaddr *) &ss->sa, &slen);
	return (*rv > 0);
}

int init_sock = 0;

int sockets_add(int sock, struct sockaddr_in *sa, int sid, int type,
		socket_action a, socket_action c, socket_action t)
{
	int i;
	char ra[50];
	if (init_sock == 0)
	{
		init_sock = 1;
		for (i = 0; i < MAX_SOCKS; i++)
			s[i].sock = -1;
	}
	for (i = 0; i < MAX_SOCKS; i++)
		if (s[i].sock < 0)
			break;
	if (i == MAX_SOCKS)
		return -1;
	s[i].sock = sock;
	memset(&s[i].sa, 0, sizeof(s[i].sa));
	if (sa)
		memcpy(&s[i].sa, sa, sizeof(s[i].sa));
	s[i].action = s[i].close = s[i].timeout = NULL;
	if (a)
		s[i].action = a;
	if (c)
		s[i].close = c;
	if (t)
		s[i].timeout = t;
	s[i].sid = sid;
	s[i].type = type & ~TYPE_CONNECT;
	s[i].rtime = getTick();
	s[i].wtime = 0;
	if (max_sock <= i)
		max_sock = i + 1;
	s[i].buf = NULL;
	s[i].lbuf = 0;
	s[i].close_sec = 0;
	s[i].id = i;
	s[i].read = (read_action) sockets_read;
	if (s[i].type == TYPE_UDP || s[i].type == TYPE_RTCP)
		s[i].read = (read_action) sockets_recv;
	else if (s[i].type == TYPE_SERVER)
		s[i].read = (read_action) sockets_accept;
	pf[i].fd = sock;
	pf[i].events = POLLIN | POLLPRI;
	if (type & TYPE_CONNECT)
		pf[i].events |= POLLOUT;
	pf[i].revents = 0;

	LOG(
			"sockets_add: handle %d (type %d) returning socket index %d [%s:%d] read: %p",
			s[i].sock, s[i].type, i, get_socket_rhost(i, ra, sizeof(ra)),
			ntohs(s[i].sa.sin_port), s[i].read);
	return i;
}

int sockets_del(int sock)
{
	int i, ss;

	if (sock < 0 || sock > MAX_SOCKS)
		return 0;
	ss = s[sock].sock;
	s[sock].sock = -1;			 // avoid infinite loop
	LOG("sockets_del: %d -> handle %d, sid %d", sock, ss, s[sock].sid);
	if (ss == -1)
		return 0;
	if (s[sock].close)
		s[sock].close(&s[sock]);
	close(ss);
	s[sock].sid = -1;
	i = MAX_SOCKS;
	while (--i >= 0 && s[i].sock <= 0)
		s[i].sock = -1;
	max_sock = i + 1;
	pf[sock].fd = -1;
	LOG("sockets_del: %d Last open socket is at index %d current_handle %d",
			sock, i, ss);
	return 0;
}

int run_loop, it = 0;
int64_t bw, bwtt;
int bwnotify, sleeping, sleeping_cnt;
unsigned long int tbw;
uint32_t reads;
uint64_t nsecs;

int select_and_execute()
{
	fd_set io;
	int i, rv, rlen;
	unsigned char buf[2001];
	int err;
	run_loop = 1;
	int lt, read_ok, c_time;
	char ra[50];
	lt = getTick();
	while (run_loop)
	{
		FD_ZERO(&io);
		i = -1;
		//    LOG("start select");
		if ((rv = poll(pf, max_sock, 100)) < 0)
		{
			perror("select_and_execute: select() error");
			continue;
		}
		c_time = getTick();
		//              LOG("select returned %d",rv);
		if (rv > 0)
			while (++i < max_sock)
				if (pf[i].revents)
				{
					sockets *ss = &s[i];
					c_time = getTick();

					LOGL(6,
							"event on socket index %d handle %d type %d (poll fd:%d, revents=%d)",
							i, ss->sock, ss->type, pf[i].fd, pf[i].revents);
					if (pf[i].revents & POLLOUT)
					{
						pf[i].events &= ~POLLOUT;
					}
					if (!ss->buf || ss->buf == buf)
					{
						ss->buf = buf;
						ss->lbuf = sizeof(buf) - 1;
						ss->rlen = 0;
					}
					if (ss->rlen >= ss->lbuf)
					{
						LOG(
								"Socket buffer full, handle %d, sock_id %d, type %d, lbuf %d, rlen %d, ss->buf = %p, buf %p",
								ss->sock, i, ss->type, ss->lbuf, ss->rlen,
								ss->buf, buf);
						ss->rlen = 0;
					}
					rlen = 0;
					if (c_time - bwtt > 1000)
					{
						bwtt = c_time;
						tbw += bw;
						if (bw > 2000)
							LOG(
									"BW %dKB/s, Total BW: %ld MB, ns/read %lld, r: %d, tt: %lld ms, n: %d (s: %d ms, s_cnt %d)",
									(int ) bw / 1024, tbw / 1024576,
									nsecs / reads, reads, nsecs / 1000,
									bwnotify, sleeping / 1000, sleeping_cnt);
						bw = 0;
						bwnotify = 0;
						nsecs = 0;
						reads = 0;
						sleeping = sleeping_cnt = 0;
					}
					if (opts.bw > 0 && bw > opts.bw && ss->type == TYPE_DVR)
					{
						int ms = 1000 - c_time + bwtt;
						if (bwnotify++ == 0)
							LOG(
									"capping %d sock %d for the next %d ms, sleeping for the next %d ms",
									i, ss->sock, ms, ms / 50);
						if (ms > 50)
							usleep(ms * 20);
						continue;

					}

					read_ok = ss->read(ss->sock, &ss->buf[ss->rlen],
							ss->lbuf - ss->rlen, ss, &rlen);

					if (opts.log >= 1)
					{
						int now = getTick();
						if (now - c_time > 100)
							LOG(
									"WARNING: read on socket id %d, handle %d, took %d ms",
									ss->id, ss->sock, now - c_time);
					}

					err = 0;
					if (rlen < 0)
						err = errno;
					if (rlen > 0)
						ss->rtime = c_time;
					if (read_ok && rlen > 0)
						ss->rlen += rlen;
					else
						ss->rlen = 0;
					//force 0 at the end of the string
					if (ss->lbuf >= ss->rlen)
						ss->buf[ss->rlen] = 0;
					LOGL(6,
							"Read %s %d (rlen:%d/total:%d) bytes from %d -> %p - iteration %d action %p",
							read_ok ? "OK" : "NOK", rlen, ss->rlen, ss->lbuf,
							ss->sock, ss->buf, it++, ss->action);

					if (((ss->rlen > 0) || err == EWOULDBLOCK) && ss->action
							&& (ss->type != TYPE_SERVER))
						ss->action(ss);
					//              if(s[i].type==TYPE_DVR && (c_time/1000 % 10 == 0))sockets_del(i); // we do this in stream.c in flush_stream*
					if (!read_ok && ss->type != TYPE_SERVER)
					{
						char *err_str;
						char *types[] =
						{ "udp", "tcp", "server", "http", "rtsp", "dvr" };
						if (rlen == 0)
						{
							err = 0;
							err_str = "Close";
						}
						else if (err == EOVERFLOW)
							err_str = "EOVERFLOW";
						else if (err == EWOULDBLOCK)
							err_str = "Connected";
						else
							err_str = strerror(err);

						if (ss->type == TYPE_RTCP)
							continue; // do not close the RTCP socket, we might get some errors here but ignore them

						LOG(
								"select_and_execute[%d]: %s on socket %d (sid:%d) from %s:%d - type %s errno %d",
								i, err_str, ss->sock, ss->sid,
								get_socket_rhost(ss->id, ra, sizeof(ra)),
								ntohs(ss->sa.sin_port), types[ss->type], err);
						if (err == EOVERFLOW || err == EWOULDBLOCK)
							continue;
						if (err == EAGAIN)
						{
							ss->err++;
							if (ss->err < 10)
								continue;
						}
						sockets_del(i);
						LOG("Delete socket %d done: sid %d", i, ss->sid);
						continue;
					}

//					ss->err = 0;					
				}
		// checking every 60seconds for idle connections - or if select times out
		if (rv == 0 || c_time - lt >= 200)
		{
			lt = c_time;
			i = -1;
			while (++i < max_sock)
				if (( s[i].sock > 0 ) && ((s[i].close_sec > 0 && lt - s[i].rtime > s[i].close_sec)
						|| (s[i].close_sec == 1)))
				{
					if (s[i].timeout && s[i].timeout(&s[i]))
						sockets_del(i);
					if (!s[i].timeout)
						sockets_del(i);
				}
			stream_timeouts();
		}
	}
	LOG("The main loop ended, run_loop = %d", run_loop);
	return 0;
}

void sockets_setread(int i, void *r)
{
	if (i < 0 || i >= MAX_SOCKS)
		return;

	s[i].read = (read_action) r;
}

void sockets_setbuf(int i, char *buf, int len)
{
	if (i < 0 || i >= MAX_SOCKS || s[i].sock < 1)
		return;
	s[i].buf = buf;
	s[i].lbuf = len;
}

void sockets_timeout(int i, int t)
{
	if (i < 0 || i >= MAX_SOCKS || s[i].sock < 1)
		return;
	s[i].close_sec = t;
}

int get_mac(char *mac)
{
	struct ifreq ifr;
	struct ifconf ifc;
	char buf[1024];
	int success = 0;

	if (opts.mac[0])
	{
		// simulate mac address
		strncpy(mac, opts.mac, 13);
		return 0;
	}
	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

	if (sock == -1)
		return 0;

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl(sock, SIOCGIFCONF, &ifc) == -1)
	{
	};

	struct ifreq *it = ifc.ifc_req;
	const struct ifreq * const end = it + (ifc.ifc_len / sizeof(struct ifreq));

	for (; it != end; ++it)
	{
		strcpy(ifr.ifr_name, it->ifr_name);
		if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0)
		{
			if (!(ifr.ifr_flags & IFF_LOOPBACK))
			{		// don't count loopback
				if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0)
				{
					success = 1;
					break;
				}
			}
		}
		else
		{
		};
	}
	unsigned char m[6];

	memcpy(m, ifr.ifr_hwaddr.sa_data, 6);
	sprintf(mac, "%02x%02x%02x%02x%02x%02x", m[0], m[1], m[2], m[3], m[4],
			m[5]);
	return 1;
}

char *
get_current_timestamp(void)
{
	static char date_str[200];
	time_t date;
	struct tm *t;
	char *day[] =
	{ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	char *month[] =
	{ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct",
			"Nov", "Dec" };
	time(&date);
	t = gmtime(&date);
	if (!t)
		return "Fri, Sat Jan 1 00:00:20 2000 GMT";
	snprintf(date_str, sizeof(date_str), "%s, %s %d %02d:%02d:%02d %d GMT",
			day[t->tm_wday], month[t->tm_mon], t->tm_mday, t->tm_hour,
			t->tm_min, t->tm_sec, t->tm_year + 1900);
	return date_str;
}

char *
get_current_timestamp_log(void)
{
	static char date_str[200];
	time_t date;
	struct tm *t;
	time(&date);
	t = localtime(&date);
	if (!t)
		return "01/01 00:00:20";
	snprintf(date_str, sizeof(date_str), "%02d/%02d %02d:%02d:%02d.%03d",
			t->tm_mday, t->tm_mon + 1, t->tm_hour, t->tm_min, t->tm_sec,
			getTick());
	return date_str;
}

int sockets_del_for_sid(int ad)
{
	int i;

	if (ad < 0)
		return 0;
	for (i = 0; i < MAX_SOCKS; i++)
		if (s[i].sock >= 0 && s[i].sid >= 0 && s[i].type != TYPE_DVR
				&& s[i].sid == ad)
		{
			s[i].close_sec = 1;	//trigger close of the socket after this operation ends, otherwise we might close an socket on which we run action
			s[i].sid = -1;// make sure the stream is not closed in the future to prevent closing the stream created by another socket
		}
	return 0;
}

void set_socket_buffer(int sid, unsigned char *buf, int len)
{
	s[sid].buf = buf;
	s[sid].lbuf = len;
//	s[sid].rlen = 0;
}

void free_all()
{
	int i = 0;

	for (i = MAX_SOCKS - 1; i > 0; i--)
		if (s[i].sock >= 0)
			sockets_del(i);
	free_all_streams();
	free_all_adapters();
}

void set_socket_send_buffer(int sock, int len)
{
	int sl;
	if (len <= 0)
		return;
	if (setsockopt(sock, SOL_SOCKET, SO_SNDBUFFORCE, &len, sizeof(len)))
		LOGL(3, "unable to set output UDP buffer (force) size to %d", len);

	if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &len, sizeof(len)))
		LOGL(3, "unable to set output UDP buffer size to %d", len);
	sl = sizeof(int);
	if (!getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &len, &sl))
		LOG("output UDP buffer size for socket %d is %d bytes", sock, len);

}

void set_socket_receive_buffer(int sock, int len)
{
	socklen_t sl;
	if (len <= 0)
		return;

	if (setsockopt(sock, SOL_SOCKET, SO_RCVBUFFORCE, &len, sizeof(len)))
		LOGL(3, "unable to set output UDP buffer (force) size to %d", len);

	if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &len, sizeof(len)))
		LOGL(3, "unable to set output UDP buffer size to %d", len);
	sl = sizeof(int);
	if (!getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &len, &sl))
		LOG("output UDP buffer size is %d bytes", len);

}

sockets *get_sockets(int i)
{
	if (i < 0 || i >= MAX_SOCKS || s[i].sock <= 0)
		return NULL;
	return &s[i];
}

void set_socket_pos(int sock, int pos)
{
	sockets *ss = get_sockets(sock);
	if (!ss)
		return;
	ss->rlen = pos;
}

char *get_socket_rhost(int s_id, char *dest, int ld)
{
	sockets *ss = get_sockets(s_id);
	dest[0] = 0;
	if(!ss)
		return dest;
	inet_ntop(AF_INET, &(ss->sa.sin_addr), dest, ld);
	return dest;
}

int get_socket_rport(int s_id)
{
	sockets *ss = get_sockets(s_id);
	if(!ss)
		return 0;
	return ntohs (ss->sa.sin_port);
}

