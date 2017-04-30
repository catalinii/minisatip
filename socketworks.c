/*
 * Copyright (C) 2014-2020 Catalin Toda
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
#include <sys/time.h>
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
#include <sys/un.h>

#include "minisatip.h"
#include "socketworks.h"
#include "utils.h"

extern struct struct_opts opts;
sockets *s[MAX_SOCKS];
int max_sock;
SMutex s_mutex;

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
	int is_multicast = 0;

	if (!fill_sockaddr(&serv, addr, port))
		return -1;
//	sock = socket(AF_INET, SOCK_DGRAM, 0);
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

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
		is_multicast = 0;
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
		if(is_multicast)
		{
			serv.sin_addr.s_addr = htonl(INADDR_ANY);
			if (bind(sock, (struct sockaddr *) &serv, sizeof(serv)) < 0)
			{
				LOGL(0, "udp_bind: failed: bind() on host ANY port %d: error %s", port, strerror(errno));
				return -1;
			}

		}
	}

	set_linux_socket_timeout(sock);

	LOGL(1, "New UDP socket %d bound to %s:%d", sock, inet_ntoa(serv.sin_addr),
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

int set_linux_socket_nonblock(int sockfd)
{
	int flags = fcntl(sockfd, F_GETFL, 0);
	return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

int set_linux_socket_timeout(int sockfd)
{
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 100000;

	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout,
																sizeof(timeout)) < 0)
		LOG("setsockopt failed for socket %d", sockfd);

	if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeout,
																sizeof(timeout)) < 0)
		LOG("setsockopt failed for socket %d", sockfd);
	return 0;
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

	set_linux_socket_timeout(sock);

	if (blocking)
		set_linux_socket_nonblock(sock);

	if (connect(sock, (struct sockaddr *) serv, sizeof(*serv)) < 0)
	{
		if (errno != EINPROGRESS)
		{
			LOGL(0, "tcp_connect: failed: connect to %s:%d failed: %s", addr,
								port, strerror(errno));
			close(sock);
			return -1;
		}
	}
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


int connect_local_socket(char *file, int blocking)
{
	struct sockaddr_un serv;
	int sock, optval = 1;


	sock = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (sock < 0)
	{
		LOGL(0, "tcp_connect failed: socket() %s", strerror(errno));
		return -1;
	}
	memset(&serv, 0, sizeof(serv));
	serv.sun_family = AF_LOCAL;
	strncpy(serv.sun_path, file, sizeof(serv.sun_path) - 1);

	set_linux_socket_timeout(sock);

	if (blocking)
		set_linux_socket_nonblock(sock);

	if (connect(sock, (struct sockaddr *) &serv, sizeof(serv)) < 0)
	{
		if (errno != EINPROGRESS)
		{
			LOGL(0, "tcp_connect: failed: connect to %s failed: %s", file, strerror(errno));
			close(sock);
			return -1;
		}
	}
	LOG("New LOCAL socket %d connected to %s", sock, file);
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
	new_sock = accept(ss->sock, (struct sockaddr *) &sa, (socklen_t *) &sas);
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
		ss->action(s[ni]);
	}
	set_linux_socket_timeout(new_sock);

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
	*rv = recvfrom(socket, buf, len, 0, (struct sockaddr *) &ss->sa,
																(socklen_t *) &slen);
	return (*rv > 0);
}

int init_sock = 0;

void sockets_lock(sockets *ss)
{
	int rv;
	mutex_lock(&ss->mutex);
	if (ss->lock)
		if ((rv = mutex_lock(ss->lock)))
		{
			LOG("%s: Changing socket %d lock %p to NULL error %d %s",
							__FUNCTION__, ss->id, ss->lock, rv, strerror(rv));
			ss->lock = NULL;
		}
}

void sockets_unlock(sockets *ss)
{
	int rv;
	if (ss->lock)
		if ((rv = mutex_unlock(ss->lock)))
		{
			LOG("%s: Changing socket %d lock %p to NULL error %d %s",
							__FUNCTION__, ss->id, ss->lock, rv, strerror(rv));
			ss->lock = NULL;
		}
	mutex_unlock(&ss->mutex);

}

void set_sock_lock(int i, SMutex *m)
{
	sockets *ss = get_sockets(i);
	if (ss)
	{
		ss->lock = m;
		LOG("%s: sock_id %d locks also mutex %p", __FUNCTION__, i, m);
	}
}

int sockets_add(int sock, struct sockaddr_in *sa, int sid, int type,
																socket_action a, socket_action c, socket_action t)
{
	int i;
	char ra[50];
	sockets *ss;

	if(sock < 0 && sock != SOCK_TIMEOUT)
		LOG_AND_RETURN(-1, "sockets_add does not add negative sockets %d", sock);

	if(sock == SOCK_TIMEOUT && t == NULL)
		LOG_AND_RETURN(-1, "sockets_add timeout without timeout function");


	i = add_new_lock((void **) s, MAX_SOCKS, sizeof(sockets), &s_mutex);
	if (i == -1)
		LOG_AND_RETURN(-1, "sockets_add failed for socks %d", sock);

	ss = s[i];
	ss->enabled = 1;
	ss->sock = sock;
	ss->nonblock = !!(type & TYPE_NONBLOCK);
	ss->tid = get_tid();
	memset(&ss->sa, 0, sizeof(ss->sa));
	if (sa)
		memcpy(&ss->sa, sa, sizeof(ss->sa));
	ss->action = ss->close = ss->timeout = NULL;
	if (a)
		ss->action = a;
	if (c)
		ss->close = c;
	if (t)
		ss->timeout = t;
	ss->sid = sid;
	ss->type = type & ~(TYPE_NONBLOCK|TYPE_CONNECT);
	ss->rtime = getTick();
	ss->wtime = 0;
	if (max_sock <= i)
		max_sock = i + 1;
	ss->buf = NULL;
	ss->lbuf = 0;
	ss->timeout_ms = 0;
	ss->id = i;
	ss->sock_err = 0;
	ss->overflow = 0;
	ss->buf_alloc = ss->buf_used = 0;
	ss->iteration = 0;
	ss->spos = ss->wpos = 0;
	ss->wmax = opts.max_sbuf;

	ss->read = (read_action) sockets_read;
	ss->lock = NULL;
	if (ss->type == TYPE_UDP || ss->type == TYPE_RTCP)
		ss->read = (read_action) sockets_recv;
	else if (ss->type == TYPE_SERVER)
		ss->read = (read_action) sockets_accept;
	ss->events = POLLIN | POLLPRI;
	if (type & TYPE_CONNECT)
		ss->events |= POLLOUT;

	LOG("sockets_add: handle %d (type %d) returning socket index %d [%s:%d] read: %p",
					ss->sock, ss->type, i, get_socket_rhost(i, ra, sizeof(ra)),
					ntohs(ss->sa.sin_port), ss->read);
	mutex_unlock(&ss->mutex);
	return i;
}

int sockets_del(int sock)
{
	int i, so;
	sockets *ss;

	if (sock < 0 || sock > MAX_SOCKS || !s[sock] || !s[sock]->enabled)
		return 0;

	ss = s[sock];
	mutex_lock(&ss->mutex);
	if (!ss->enabled)
	{
		mutex_unlock(&ss->mutex);
		return 0;

	}
	mutex_lock(&s_mutex);
	ss->enabled = 0;
	so = ss->sock;
	ss->sock = -1;                             // avoid infinite loop
	LOG("sockets_del: %d -> handle %d, sid %d, overflow %d, allocated %d, used %d", sock, so, ss->sid, ss->overflow, ss->buf_alloc, ss->buf_used);

	if (ss->close)
		ss->close(ss);

	if (so >= 0)
		close(so);
	ss->sid = -1;
	i = MAX_SOCKS;
	while (--i >= 0)
		if (s[i] && !s[i]->enabled)
			s[i]->sock = -1;
		else if (s[i] && s[i]->enabled)
			break;
	max_sock = i + 1;
	ss->events = 0;
	ss->lock = NULL;
	if((ss->flags & 1) && ss->buf)
		free1(ss->buf);
	if(ss->pack)
	{
		int i;
		for(i = 0; i < ss->wmax; i++)
			if(ss->pack[i].buf)
			{
				free1(ss->pack[i].buf);
				ss->pack[i].buf = NULL;
			}
		free1(ss->pack);
		ss->pack = NULL;
	}

	LOG("sockets_del: %d Last open socket is at index %d current_handle %d",
					sock, i, so);
	mutex_destroy(&ss->mutex);
	mutex_unlock(&s_mutex);
	return 0;
}

int run_loop = 1;
extern pthread_t main_tid;
extern int bwnotify;
extern int64_t bwtt, bw;
extern uint32_t writes, failed_writes;

__thread pthread_t tid;
__thread char *thread_name;

void *select_and_execute(void *arg)
{
	fd_set io;
	int i, rv, rlen, les, es;
	unsigned char buf[2001];
	int err;
	struct pollfd pf[MAX_SOCKS];
	int64_t lt, c_time;
	int read_ok;
	char ra[50];

	if (arg)
		thread_name = (char *) arg;
	else
		thread_name = "main";

	tid = get_tid();
	les = 1;
	es = 0;
	lt = getTick();
	memset(&pf, -1, sizeof(pf));
	LOG("Starting select_and_execute on thread ID %x, thread_name %s", tid,
					thread_name);
	while (run_loop)
	{
		c_time = getTick();
		es = 0;
		clean_mutexes();
		for (i = 0; i < max_sock; i++)
			if (s[i] && s[i]->enabled && s[i]->tid == tid)
			{
				pf[i].fd = s[i]->sock;
				pf[i].events = s[i]->events;

				if(s[i]->spos != s[i]->wpos)
					pf[i].events |= POLLOUT;

				pf[i].revents = 0;
				s[i]->last_poll = c_time;
				es++;
			}
			else
			{
				pf[i].fd = -1;
				pf[i].events = pf[i].revents = 0;
			}
		i = -1;
		if (les == 0 && es == 0 && tid != main_tid)
		{
			LOG("No enabled sockets for Thread ID %lx name %s ... exiting ",
							tid, thread_name);
			break;
		}
		les = es;
		//    LOG("start select");
		if ((rv = poll(pf, max_sock, 100)) < 0)
		{
			LOG("select_and_execute: select() error %d: %s", errno,
							strerror(errno));
			continue;
		}
		//              LOG("select returned %d",rv);
		if (rv > 0)
			while (++i < max_sock)
				if ((pf[i].fd >= 0) && pf[i].revents)
				{
					sockets *ss = s[i];
					if (!ss)
						continue;

					c_time = getTick();
					ss->iteration++;

					LOGL(6,
										"event on socket index %d handle %d type %d (poll fd:%d, revents=%d)",
										i, ss->sock, ss->type, pf[i].fd, pf[i].revents);
					sockets_lock(ss);

					if ((pf[i].revents & POLLOUT) && (ss->spos != ss->wpos))
					{
						int k=300;

						while(!flush_socket(ss))
							if(!k--)
								break;
						if(k == 0)
							LOGL(7, "Sock %d: Dequeued max packets", ss->id);

						if((pf[i].revents & (~POLLOUT)) == 0)
						{
							LOGL(7, "Sock %d: No Read event, continuing", ss->id);
							sockets_unlock(ss);
							continue;
						}

						pf[i].revents &= ~POLLOUT;
					}

					if (pf[i].revents & POLLOUT)
					{
						ss->events &= ~POLLOUT;
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
					read_ok = 0;
					if(ss->read)
						read_ok = ss->read(ss->sock, &ss->buf[ss->rlen],
																									ss->lbuf - ss->rlen, ss, &rlen);

					if (opts.log >= 1)
					{
						int64_t now = getTick();
						if (now - c_time > 100)
							LOG(
								"WARNING: read on socket id %d, handle %d, took %jd ms",
								ss->id, ss->sock, now - c_time);
					}

					err = 0;
					if (rlen < 0)
						err = errno;
					if (rlen > 0)
						ss->rtime = c_time;
					if (read_ok && rlen >= 0)
						ss->rlen += rlen;
					else
						ss->rlen = 0;
					//force 0 at the end of the string
					if (ss->lbuf >= ss->rlen)
						ss->buf[ss->rlen] = 0;
					LOGL(6,
										"Read %s %d (rlen:%d/total:%d) bytes from %d -> %p - iteration %d action %p",
										read_ok ? "OK" : "NOK", rlen, ss->rlen, ss->lbuf,
										ss->sock, ss->buf, ss->iteration, ss->action);

					if (((ss->rlen > 0) || err == EWOULDBLOCK) && ss->action
									&& (ss->type != TYPE_SERVER))
						ss->action(ss);
					sockets_unlock(ss);

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

						if (ss->type == TYPE_RTCP || ss->sock == SOCK_TIMEOUT)
						{
							LOG(
								"ignoring error on sock_id %d handle %d type %d error %d : %s",
								ss->id, ss->sock, ss->type, err, err_str);
							continue;                                                                                                                                                                                                     // do not close the RTCP socket, we might get some errors here but ignore them
						}
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
		c_time = getTick();
		if (rv == 0 || (c_time - lt >= 200))
		{
			sockets *ss;
			lt = c_time;
			i = -1;
			while (++i < max_sock)
				if ((ss = get_sockets(i)) && (ss->tid == tid)
								&& ((ss->timeout_ms > 0
													&& lt - ss->rtime > ss->timeout_ms)
												|| (ss->timeout_ms == 1)))
				{
					if (ss->timeout)
					{
						int rv;
						if (ss->sock == SOCK_TIMEOUT)
							ss->rtime = getTick();
						sockets_lock(ss);
						rv = ss->timeout(ss);
						sockets_unlock(ss);
						if (rv)
							sockets_del(i);
					}

					if (!ss->timeout)
						sockets_del(i);
				}
		}
	}

	clean_mutexes();

	if (tid == main_tid)
		LOG("The main loop ended, run_loop = %d", run_loop);
	add_join_thread(tid);

	return NULL;
}

void sockets_setread(int i, void *r)
{
	sockets *ss = get_sockets(i);
	if (ss)
		ss->read = (read_action) r;
}

void sockets_setbuf(int i, char *buf, int len)
{
	sockets *ss = get_sockets(i);
	if (ss)
	{
		ss->buf = (unsigned char *) buf;
		ss->lbuf = len;
	}
}

void sockets_timeout(int i, int t)
{
	sockets *ss = get_sockets(i);
	if (ss)
		ss->timeout_ms = t;
}

void set_sockets_rtime(int i, int r)
{
	sockets *ss = get_sockets(i);
	if (ss)
		ss->rtime = r;
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
			{                                                                                     // don't count loopback
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
	struct timeval tv;
	struct tm *t;

	if (gettimeofday(&tv, NULL))
		return "01/01 00:00:20";
	t = localtime(&tv.tv_sec);
	if (!t)
		return "01/01 00:00:20";
	snprintf(date_str, sizeof(date_str), "%02d/%02d %02d:%02d:%02d.%03d",
										t->tm_mday, t->tm_mon + 1, t->tm_hour, t->tm_min, t->tm_sec,
										(int) (tv.tv_usec / 1000));
	return date_str;
}

int sockets_del_for_sid(int sid)
{
	int i;
	sockets *ss;
	if (sid < 0)
		return 0;
	for (i = 0; i < MAX_SOCKS; i++)
		if ((ss = get_sockets(i)) && ss->sid >= 0 && ss->type == TYPE_RTSP
						&& ss->sid == sid)
		{
			ss->timeout_ms = 1;                                                                                     //trigger close of the socket after this operation ends, otherwise we might close an socket on which we run action
			ss->sid = -1;                                                                                    // make sure the stream is not closed in the future to prevent closing the stream created by another socket
		}
	return 0;
}

void set_socket_buffer(int sid, unsigned char *buf, int len)
{
	sockets *ss = get_sockets(sid);
	if (!ss)
		return;

	ss->buf = buf;
	ss->lbuf = len;
}

void free_all_streams();
void free_all_adapters();
void free_all_keys();

void free_all()
{
	int i = 0;

	for (i = MAX_SOCKS - 1; i > 0; i--)
	{
		if (s[i] && s[i]->enabled)
			sockets_del(i);
		if (s[i])
			free(s[i]);
		s[i] = NULL;
	}
	free_all_streams();
	free_all_adapters();
#ifndef DISABLE_DVBAPI
	free_all_keys();
#endif
}

void set_socket_send_buffer(int sock, int len)
{
	int sl;
	int rv;
	if (len <= 0)
		return;
	// len = 8*1024; /* have a nice testing !!!! */
#ifdef SO_SNDBUFFORCE
	if ((rv = setsockopt(sock, SOL_SOCKET, SO_SNDBUFFORCE, &len, sizeof(len))))
		LOGL(3, "unable to set output socket buffer (force) size to %d", len);
#endif
	if (rv && setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &len, sizeof(len)))
		LOGL(3, "unable to set output socket buffer size to %d", len);
	sl = sizeof(int);
	if (!getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &len, (socklen_t *) &sl))
		LOG("output socket buffer size for socket %d is %d bytes", sock, len);

}

void set_socket_receive_buffer(int sock, int len)
{
	socklen_t sl;
	int rv;
	if (len <= 0)
		return;
#ifdef SO_RCVBUFFORCE
	if ((rv = setsockopt(sock, SOL_SOCKET, SO_RCVBUFFORCE, &len, sizeof(len))))
		LOGL(3, "unable to set receive socket buffer (force) size to %d", len);
#endif
	if (rv && setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &len, sizeof(len)))
		LOGL(3, "unable to set receive socket buffer size to %d", len);
	sl = sizeof(int);
	if (!getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &len, &sl))
		LOG("receive socket buffer size is %d bytes", len);

}

sockets *get_sockets(int i)
{
	if (i < 0 || i >= MAX_SOCKS || !s[i] || !s[i]->enabled)
		return NULL;
	return s[i];
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
	if (!ss)
		return dest;
	inet_ntop(AF_INET, &(ss->sa.sin_addr), dest, ld);
	return dest;
}

int get_socket_rport(int s_id)
{
	sockets *ss = get_sockets(s_id);
	if (!ss)
		return 0;
	return ntohs(ss->sa.sin_port);
}

void get_socket_iteration(int s_id, int it)
{
	sockets *ss = get_sockets(s_id);
	if (!ss)
		return;
	ss->iteration = it;
}


void set_socket_thread(int s_id, pthread_t tid)
{
	sockets *ss = get_sockets(s_id);
	if (!ss)
		return;
	LOG("set_socket_thread: thread %x for sockets %i", tid, s_id);
	ss->tid = tid;
}

pthread_t get_socket_thread(int s_id)
{
	sockets *ss = get_sockets(s_id);
	if (!ss)
		LOG_AND_RETURN(0, "get_socket_thread: socket is NULL for s_id %d", s_id);
	return ss->tid;
}

int my_writev(sockets *s, const struct iovec *iov, int iiov)
{
	int rv, len = 0, i;
	char ra[50];
	int log_level = 7;
	int64_t stime = 0;
	if (s->timeout_ms == 1)
		return -1;

	len = 0;
	for (i=0; i<iiov; i++)
		len += iov[i].iov_len;

	LOGL(log_level + 1, "start writev handle %d, iiov %d, len %d", s->sock, iiov, len);
	if (opts.log == log_level)
		stime = getTick();
	rv = writev(s->sock, iov, iiov);
	if (opts.log == log_level)
		stime = getTick() - stime;

	if(rv > 0)
	{
		bw += rv;
		writes++;
	}

	if(rv != len)
	{
		failed_writes++;
		LOGL(log_level, "writev handle %d, iiov %d, len %d, rv %d, errno %d", s->sock, iiov, len, rv, errno);
	}

	if ((rv < 0) && (errno == EWOULDBLOCK))                             // blocking
		return -EWOULDBLOCK;

	if (rv < 0 && (errno == ECONNREFUSED || errno == EPIPE))                             // close the stream int the next second
	{
		LOGL(1,
							"Connection REFUSED on socket %d (sid %d), closing the socket, remote %s:%d",
							s->id, s->sid, get_socket_rhost(s->id, ra, sizeof(ra)),
							get_socket_rport(s->id));
		if(s->type != TYPE_RTCP)
			s->timeout_ms = 1;
	}
	if (rv < 0)
	{
		s->sock_err++;
		LOG("writev returned %d handle %d, iiov %d errno %d error %s", rv, s->sock,
						iiov, errno, strerror(errno));
	}
	else
		s->sock_err = 0;

	LOGL(log_level, "writev returned %d handle %d, iiov %d (took %jd ms)", rv,
						s->sock, iiov, stime);
	return rv;
}

int alloc_snpacket(SNPacket *p, int len)
{
	if(!p->buf || (p->size < len))
	{
		if(p->buf)
		{
			free1(p->buf);
			p->buf = NULL;
		}
		int newlen = (len < 1500) ? 1500 : len;
		p->buf = malloc1(newlen);
		if(p->buf) {
			p->size = newlen;
			return 1;
		}
		else
		{
			LOGL(1, "%s: could not allocate %d bytes for socket buffer", __FUNCTION__, newlen);
			return -1;
		}
	}
	return 0;
}

int sockets_writev(int sock_id, struct iovec *iov, int iovcnt)
{
	int rv = 0, i, pos = 0, len;
	unsigned char tmpbuf[(STREAMS_BUFFER*3)/2];
	struct iovec tmpiov[MAX_PACK + 3];
	sockets *s = get_sockets(sock_id);
	if(!s)
		return -1;
	if(s->spos == s->wpos)
	{
		int len = 0;
		for(i=0; i<iovcnt; i++)
			len += iov[i].iov_len;
		memcpy(tmpiov, iov, iovcnt * sizeof(tmpiov[0]));

		rv = my_writev(s, iov, iovcnt);
		if(rv == len)
			return rv;

		if(rv > 0 && (len < sizeof(tmpbuf)))
		{
			for(i=0; i<iovcnt; i++)
			{
				if (rv < pos + tmpiov[i].iov_len)
					memcpy(tmpbuf + pos, tmpiov[i].iov_base, tmpiov[i].iov_len);
				pos += tmpiov[i].iov_len;
			}
			LOGL(3, "incomplete write it %d, setting the buffer at offset %d and length %d from %d", s->iteration, rv, len - rv, len);
			tmpiov[0].iov_base = tmpbuf + rv;
			tmpiov[0].iov_len = len - rv;
			iov = tmpiov;
			iovcnt = 1;

		}else if(len > sizeof(tmpbuf))
			LOG("tmpbuf size is too small: %d required ", len);

	}

	if(!s->pack)
	{
		s->pack = malloc1(s->wmax * sizeof(SNPacket));
		if(!s->pack)
		{
			s->overflow++;
			LOG_AND_RETURN(0, "%s: Could not allocate memory for s->pack", __FUNCTION__, s->wmax);
		}
	}

	len = 0;
	for(i=0; i<iovcnt; i++)
		len += iov[i].iov_len;
	// queue the packet otherwise
	SNPacket *p = s->pack + s->wpos;

	i = alloc_snpacket(p, len);
	if (i > 0)
	{
		s->buf_alloc++;
	}
	else if (i < 0)
	{
		LOGL(4, "overflow p is %p, buf %p", p, p ? p->buf : NULL);
		s->overflow++;
		return 0;
	}
	s->buf_used++;
	LOGL(4, "SOCK %d it %d: queueing %d bytes at %d (out of %d) send pos %d [A:%d, U:%d]", s->id, s->iteration, len, s->wpos, s->wmax, s->spos, s->buf_alloc, s->buf_used);

	if(s->spos == ((s->wpos + 1) % s->wmax))                             // the queue is full, start overwriting
	{
		s->overflow++;
		if((s->overflow % 100) == 0)
			LOGL(3, "sock %d: overflow %d it %d", s->id, s->overflow, s->iteration);
		return 0;
	}

	pos = 0;
	for(i=0; i<iovcnt; i++)
	{
		memcpy(p->buf + pos, iov[i].iov_base, iov[i].iov_len);
		pos += iov[i].iov_len;
	}
	p->len = pos;
	s->wpos = (s->wpos + 1) % s->wmax;

	return 0;

}

int sockets_write(int sock_id, void *buf, int len)
{
	struct iovec iov[1];
	iov[0].iov_base = buf;
	iov[0].iov_len = len;
	return sockets_writev(sock_id, iov, 1);
}



int flush_socket(sockets *s)
{
	struct iovec iov[1];
	int rv;
	SNPacket *p = NULL;

	if(s->spos == s->wpos)
		return 1;
	if(s->pack)
		p = s->pack + s->spos;
	if(p && p->buf)
	{

		iov[0].iov_len = p->len;
		iov[0].iov_base = p->buf;
		rv = my_writev(s, iov, 1);
		if((rv > 0) && (rv != p->len))
		{
			LOG("incomplete write %d out of %d", rv, p->len);
			memmove(p->buf, p->buf + rv, p->len - rv);
			p->len -= rv;
			return 1;
		}
		else
		{
			if(rv != p->len)
				return 1;
		}

	}
	LOGL(4, "SOCK %d: flushed %d out of %d (%d bytes)", s->id, s->spos, s->wpos, p ? p->len : -1);
	s->spos = (s->spos + 1) % s->wmax;
	return 0;
}
