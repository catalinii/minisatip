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
#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "minisatip.h"
#include "socketworks.h"
#include "t2mi.h"
#include "utils.h"
#include "utils/alloc.h"
#include "utils/ticks.h"

#define DEFAULT_LOG LOG_SOCKETWORKS

sockets *s[MAX_SOCKS];
int max_sock;
SMutex s_mutex;

// returns the protocol (AF_INET, AF_INET6) or 0 in case of failure
int fill_sockaddr(USockAddr *serv, char *host, int port, int ipv4_only) {
    int family = 0;
    memset(serv, 0, sizeof(*serv));

    if (host) {
        char str_port[12];
        struct addrinfo hints;
        struct addrinfo *result = NULL;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC; /* Allow IPv4 or IPv6 */
        if (ipv4_only)
            hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
        hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
        hints.ai_protocol = 0;          /* Any protocol */
        hints.ai_canonname = NULL;
        hints.ai_addr = NULL;
        hints.ai_next = NULL;
        sprintf(str_port, "%d", port);

        int s = getaddrinfo(host, str_port, &hints, &result);
        if (s != 0) {
            LOG("getaddrinfo failed: host %s, port %s, %s", host, str_port,
                gai_strerror(s));
            if (result)
                freeaddrinfo(result);
            return 0;
        }
        if (result != NULL) {
            memcpy(serv, result->ai_addr, result->ai_addrlen);
            family = result->ai_family;
            freeaddrinfo(result);
        }
    }
    if (!family && ipv4_only) // use IPv4 only
    {
        family = AF_INET;
        serv->sin.sin_addr.s_addr = htonl(INADDR_ANY);
        serv->sin.sin_family = family;
        serv->sin.sin_port = htons(port);
    } else if (!family) // LISTEN on IPv6 AND IPv4
    {
        family = AF_INET6;
        serv->sin6.sin6_family = family;
        serv->sin6.sin6_flowinfo = 0;
        serv->sin6.sin6_addr = in6addr_any;
        serv->sin6.sin6_port = htons(port);
    }
    return family;
}

char localip[MAX_HOST];
char *getlocalip() {
    const char *dest = opts.disc_host;
    int port = 1900;

    USockAddr serv;

    int family = fill_sockaddr(&serv, (char *)dest, port, opts.use_ipv4_only);

    int sock = socket(family, SOCK_DGRAM, IPPROTO_IP);

    // Socket could not be created
    if (sock < 0) {
        LOG("getlocalip: Cannot create socket: %s", strerror(errno));
        return localip;
    }

    int err = connect(sock, &serv.sa, SOCKADDR_SIZE(serv));
    if (err) {
        LOG("getlocalip: Error '%s'' during connect", strerror(errno));
        memset(localip, 0, sizeof(localip));
    } else {
        get_sock_shost(sock, localip, sizeof(localip));
    }
    close(sock);
    return localip;
}

int udp_bind(char *addr, int port, int ipv4_only) {
    USockAddr serv;
    char localhost[100];
    int sock, optval = 1;
    int is_multicast = 0;
    int family = fill_sockaddr(&serv, addr, port, ipv4_only);

    if (!family)
        return -1;
    //	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    sock = socket(family, SOCK_DGRAM, IPPROTO_UDP);

    if (sock < 0) {
        LOG("udp_bind failed: socket(): %s", strerror(errno));
        return -1;
    }

    if (family == AF_INET && addr && atoi(addr) >= 239) {
        struct ip_mreq mreq;

        mreq.imr_multiaddr.s_addr = inet_addr(addr);
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        is_multicast = 1;
        LOG("setting multicast for %s", addr);
        if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq,
                       sizeof(mreq)) == -1) {
            LOG("membership error: %s", strerror(errno));
        }
    }
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) <
        0) {
        LOG("udp_bind failed: setsockopt(SO_REUSEADDR): %s", strerror(errno));
        close(sock);
        return -1;
    }
#ifdef SO_REUSEPORT
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) < 0)
        LOG("setsockopt failed to set SO_REUSEPORT %s", strerror(errno));
#endif

    if (bind(sock, &serv.sa, SOCKADDR_SIZE(serv)) < 0) {
        if (is_multicast) // only IPv4
        {
            struct sockaddr_in *s = (struct sockaddr_in *)&serv;
            s->sin_addr.s_addr = htonl(INADDR_ANY);
            if (bind(sock, &serv.sa, sizeof(serv.sa)) < 0) {
                LOG("udp_bind: failed: bind() on host ANY port %d: error %s",
                    port, strerror(errno));
                close(sock);
                return -1;
            }
        } else {
            LOG("udp_bind: failed: bind() on host %s port %d: error %s", addr,
                port, strerror(errno));
            close(sock);
            return -1;
        }
    }

    set_linux_socket_timeout(sock);

    LOG("New UDP socket %d bound to %s:%d %s%s%s", sock,
        get_sockaddr_host(serv, localhost, sizeof(localhost)),
        get_sockaddr_port(serv), is_multicast ? "(mcast:" : "",
        is_multicast ? addr : "", is_multicast ? ")" : "");
    return sock;
}

int udp_bind_connect(char *src, int sport, char *dest, int dport,
                     USockAddr *serv) {
    int sock;
    char localhost[100];
    int family = fill_sockaddr(serv, dest, dport, opts.use_ipv4_only);
    sock = udp_bind(src, sport, family == AF_INET);
    if (sock < 0)
        return sock;

    if (connect(sock, &serv->sa, SOCKADDR_SIZE(*serv)) < 0) {
        LOG("udp_bind_connect: failed: bind(): %s", strerror(errno));
        close(sock);
        return -1;
    }
    LOG("New UDP socket %d connected to %s:%d", sock,
        get_sockaddr_host(*serv, localhost, sizeof(localhost)),
        get_sockaddr_port(*serv));

    return sock;
}

int udp_connect(char *addr, int port, USockAddr *serv) {
    USockAddr sv;
    int sock, optval = 1;
    int family;
    char localhost[100];

    if (serv == NULL)
        serv = &sv;
    if (!(family = fill_sockaddr(serv, addr, port, opts.use_ipv4_only)))
        return -1;
    sock = socket(family, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        LOG("udp_connect failed: socket() %s", strerror(errno));
        return -1;
    }

    if (family == AF_INET && setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval,
                                        sizeof(optval)) < 0) {
        LOG("udp_bind: setsockopt(SO_REUSEADDR): %s", strerror(errno));
        close(sock);
        return -1;
    }

    if (connect(sock, &serv->sa, SOCKADDR_SIZE(*serv)) < 0) {
        LOG("udp_connect: failed: bind(): %s", strerror(errno));
        close(sock);
        return -1;
    }
    LOG("New UDP socket %d connected to %s:%d", sock,
        get_sockaddr_host(*serv, localhost, sizeof(localhost)),
        get_sockaddr_port(*serv));
    return sock;
}

int set_linux_socket_nonblock(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

int set_linux_connect_timeout(int sockfd) {
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                   sizeof(timeout)) < 0)
        LOG("setsockopt failed for socket %d", sockfd);

    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                   sizeof(timeout)) < 0)
        LOG("setsockopt failed for socket %d", sockfd);
    return 0;
}

int set_linux_socket_timeout(int sockfd) {
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 100000;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                   sizeof(timeout)) < 0)
        LOG("setsockopt failed for socket %d", sockfd);

    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                   sizeof(timeout)) < 0)
        LOG("setsockopt failed for socket %d", sockfd);
    return 0;
}

int tcp_connect_src(char *addr, int port, USockAddr *serv, int blocking,
                    char *src) {
    USockAddr sv;
    int sock, optval = 1;
    int family;

    if (serv == NULL)
        serv = &sv;

    if (!(family = fill_sockaddr(serv, addr, port, opts.use_ipv4_only)))
        return -1;

    sock = socket(family, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0) {
        LOG("tcp_connect failed: socket() %s", strerror(errno));
        return -1;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) <
        0) {
        LOG("tcp_connect: setsockopt(SO_REUSEADDR): %s", strerror(errno));
        close(sock);
        return -1;
    }

    set_linux_connect_timeout(sock);

    if (!blocking) {
        if (set_linux_socket_nonblock(sock) < 0)
            LOG("%s: could not set the socket %d in nonblocking mode %d err: "
                "%s",
                __FUNCTION__, sock, errno, strerror(errno));
    }

    if (src && src[0]) {
        USockAddr src_add;
        if (!fill_sockaddr(&src_add, src, 0, family == AF_INET)) {
            close(sock);
            return -1;
        }
        if (bind(sock, &src_add.sa, SOCKADDR_SIZE(src_add)) < 0) {
            LOG("%s: failed: bind() on address: %s: error %s", __FUNCTION__,
                src, strerror(errno));
            close(sock);
            return -1;
        }
    }

    if (connect(sock, &serv->sa, SOCKADDR_SIZE(*serv)) < 0) {
        if (blocking || errno != EINPROGRESS) {
            LOG("tcp_connect: failed: connect to %s:%d failed with errno %d: "
                "%s",
                addr, port, errno,
                errno == 11 ? "connect timed out" : strerror(errno));
            close(sock);
            return -1;
        }
    }
    set_linux_socket_timeout(sock);
    LOG("New TCP socket %d connected to %s:%d", sock, addr, port);
    return sock;
}

int tcp_connect(char *addr, int port, USockAddr *serv, int blocking) {
    return tcp_connect_src(addr, port, serv, blocking, NULL);
}
int tcp_listen(char *addr, int port, int ipv4_only) {
    USockAddr serv;
    int sock, optval = 1;
    int family;
    char sa[50];

    if (!(family = fill_sockaddr(&serv, addr, port, ipv4_only)))
        return -1;

    sock = socket(family, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0) {
        LOG("tcp_listen failed: socket(): %s", strerror(errno));
        return -1;
    }
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) <
        0) {
        LOG("tcp_listen failed: setsockopt(SO_REUSEADDR): %s", strerror(errno));
        close(sock);
        return -1;
    }

    if (bind(sock, &serv.sa, SOCKADDR_SIZE(serv)) < 0) {
        LOG("tcp_listen: failed: bind() on address: %s [%s], port %d : error "
            "%s",
            addr ? addr : "ANY", get_sockaddr_host(serv, sa, sizeof(sa)), port,
            strerror(errno));
        close(sock);
        return -1;
    }
    if (listen(sock, 10) < 0) {
        LOG("tcp_listen: listen() error: %s", strerror(errno));
        close(sock);
        return -1;
    }
    LOG("New TCP listening socket %d at %s:%d, family %d", sock,
        addr ? addr : "0:0:0:0", port, family);
    return sock;
}

int connect_local_socket(char *file, int blocking) {
    struct sockaddr_un serv;
    int sock;

    sock = socket(AF_LOCAL, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0) {
        LOG("tcp_connect failed: socket() %s", strerror(errno));
        return -1;
    }
    memset(&serv, 0, sizeof(serv));
    serv.sun_family = AF_LOCAL;
    safe_strncpy(serv.sun_path, file);

    set_linux_socket_timeout(sock);

    if (blocking) {
        if (set_linux_socket_nonblock(sock) < 0)
            LOG("%s: could not set the socket %d in nonblocking mode %d err: "
                "%s",
                __FUNCTION__, sock, errno, strerror(errno));
    }

    if (connect(sock, (struct sockaddr *)&serv, sizeof(serv)) < 0) {
        if (errno != EINPROGRESS) {
            LOG("tcp_connect: failed: connect to %s failed: %s", file,
                strerror(errno));
            close(sock);
            return -1;
        }
    }
    LOG("New LOCAL socket %d connected to %s", sock, file);
    return sock;
}

int no_action(int s) { return 1; }

int sockets_accept(int socket, void *buf, int len, sockets *ss) {
    int new_sock, sas, ni;
    USockAddr sa;
    sas = sizeof(sa);
    new_sock = accept(ss->sock, &sa.sa, (socklen_t *)&sas);
    if (new_sock < 0) {
        LOG("sockets_accept: failed %d: %s", errno, strerror(errno));
        return 1;
    }
    ni = sockets_add(new_sock, &sa, -1, TYPE_TCP, NULL, NULL, NULL);
    if (ni < 0) {
        LOG("ERROR on accept: could not add new socket %d to the list",
            new_sock);
        close(new_sock);
        return 1;
    }
    if (ss->action != NULL) {
        ss->action(s[ni]);
    }
    set_linux_socket_timeout(new_sock);

    return 1;
}

int sockets_read(int socket, void *buf, int len, sockets *ss, int *rv) {
    *rv = read(socket, buf, len);
    if (*rv > 0 && ss->type == TYPE_DVR && (opts.debug & LOG_DMX))
        _dump_packets("read ->", buf, *rv, ss->rlen);

    return (*rv > 0);
}

int sockets_recv(int socket, void *buf, int len, sockets *ss, int *rv) {
    int slen = sizeof(ss->sa);
    *rv = recvfrom(socket, buf, len, 0, &ss->sa.sa, (socklen_t *)&slen);
    // 0 is totally acceptable for UDP
    return (*rv >= 0);
}

int init_sock = 0;

void sockets_lock(sockets *ss) {
    int rv;
    sockets *s = NULL;
    mutex_lock(&ss->mutex);
    if (ss->lock)
        if ((rv = mutex_lock(ss->lock))) {
            LOG("%s: Changing socket %d lock %p to NULL error %d %s",
                __FUNCTION__, ss->id, ss->lock, rv,
                strerror((rv > 0) ? rv : 0));
            ss->lock = NULL;
        }
    if ((ss->master >= 0) && (s = get_sockets(ss->master))) {
        if (ss->tid != s->tid) {
            LOG("Master socket %d has different thread id than socket %d: %lx "
                "!= "
                "%lx, closing slave socket",
                s->id, ss->id, s->tid, ss->tid);
            ss->force_close = 1;
        } else
            sockets_lock(s);
    }
}

void sockets_unlock(sockets *ss) {
    int rv;
    sockets *s;
    if ((ss->master >= 0) && (s = get_sockets(ss->master))) {
        sockets_unlock(s);
    }
    if (ss->lock)
        if ((rv = mutex_unlock(ss->lock))) {
            LOG("%s: Changing socket %d lock %p to NULL error %d %s",
                __FUNCTION__, ss->id, ss->lock, rv,
                strerror((rv > 0) ? rv : 0));
            ss->lock = NULL;
        }
    mutex_unlock(&ss->mutex);
}

void set_sock_lock(int i, SMutex *m) {
    sockets *ss = get_sockets(i);
    if (ss) {
        ss->lock = m;
        LOG("%s: sock_id %d locks also mutex %p", __FUNCTION__, i, m);
    }
}

int sockets_add(int sock, USockAddr *sa, int sid, int type, socket_action a,
                socket_action c, socket_action t) {
    int i;
    char ra[50];
    sockets *ss;

    if (sock < 0 && sock != SOCK_TIMEOUT)
        LOG_AND_RETURN(-1, "sockets_add does not add negative sockets %d",
                       sock);

    if (sock == SOCK_TIMEOUT && t == NULL)
        LOG_AND_RETURN(-1, "sockets_add timeout without timeout function");

    i = add_new_lock((void **)s, MAX_SOCKS, sizeof(sockets), &s_mutex);
    if (i == -1)
        LOG_AND_RETURN(-1, "sockets_add failed for socks %d", sock);

    ss = s[i];
    ss->enabled = 1;
    ss->is_enabled = 1;
    ss->force_close = 0;
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
    ss->type = type & ~(TYPE_NONBLOCK | TYPE_CONNECT);
    ss->rtime = getTick();
    ss->wtime = 0;
    if (max_sock <= i)
        max_sock = i + 1;
    ss->opaque = ss->opaque2 = ss->opaque3 = NULL;
    ss->buf = NULL;
    ss->lbuf = 0;
    ss->timeout_ms = 0;
    ss->id = i;
    ss->sock_err = 0;
    ss->overflow = 0;
    ss->iteration = 0;
    ss->master = -1;
    ss->flush_enqued_data = 0;
    ss->prio_data_len = 0;

    ss->read = (read_action)sockets_read;
    ss->lock = NULL;
    if (ss->type == TYPE_UDP || ss->type == TYPE_RTCP)
        ss->read = (read_action)sockets_recv;
    else if (ss->type == TYPE_SERVER)
        ss->read = (read_action)sockets_accept;
    ss->events = POLLIN | POLLPRI;
    if (type & TYPE_CONNECT)
        ss->events |= POLLOUT;

    LOG("sockets_add: handle %d (type %d) returning socket sock %d [%s:%d] "
        "read: "
        "%p",
        ss->sock, ss->type, i, get_sockaddr_host(ss->sa, ra, sizeof(ra)),
        get_sockaddr_port(ss->sa), ss->read);
    mutex_unlock(&ss->mutex);
    return i;
}

int sockets_del(int sock) {
    int i, so;
    sockets *ss;

    if (sock < 0 || sock >= MAX_SOCKS || !s[sock] || !s[sock]->enabled ||
        !s[sock]->is_enabled)
        return 0;

    ss = s[sock];
    mutex_lock(&ss->mutex);
    if (!ss->enabled) {
        mutex_unlock(&ss->mutex);
        return 0;
    }
    if (ss->close)
        ss->close(ss);

    ss->is_enabled = 0;
    so = ss->sock;
    ss->sock = -1; // avoid infinite loop
    LOG("sockets_del: sock %d -> handle %d, sid %d, overflow bytes %d, "
        "total buffered bytes %jd",
        sock, so, ss->sid, ss->overflow, ss->buffered_bytes);

    if (so >= 0) {
        LOGM("sockets_del: closing socket handle %d", so);
        close(so);
    }
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
    ss->master = -1;
    if ((ss->flags & 1) && ss->buf)
        _free(ss->buf);
    if (ss->prio_data) {
        _free(ss->prio_data);
        ss->prio_data = NULL;
        ss->prio_data_len = 0;
    }
    free_fifo(&ss->fifo);

    LOG("sockets_del: sock %d Last open socket is at index %d current_handle "
        "%d",
        sock, i, so);
    mutex_destroy(&ss->mutex);
    mutex_lock(&s_mutex);
    ss->enabled = 0;
    mutex_unlock(&s_mutex);

    for (i = 0; i < MAX_SOCKS; i++)
        if (s[i] && s[i]->enabled && s[i]->master == sock) {
            LOG("Closing slave socket index %d (master sock %d)", i, sock);
            sockets_del(i);
        }
    return 0;
}
#undef DEFAULT_LOG
#define DEFAULT_LOG LOG_SOCKET

int run_loop = 1;
extern pthread_t main_tid;
extern int bwnotify;
extern int64_t bwtt, bw, buffered_bytes, dropped_bytes;
extern uint32_t writes, failed_writes;

// remove __thread if your platform does not support threads.
// also make sure to run with option -t (no threads)
// or set EMBEDDED=1 in Makefile

__thread int select_timeout;
SMutex thread_mutex;

int get_thread_index() {
    int i;
    mutex_init(&thread_mutex);
    mutex_lock(&thread_mutex);
    for (i = 0; i < MAX_THREAD_INFO; i++)
        if (thread_info[i].enabled == 0) {
            thread_info[i].enabled = 1;
            break;
        }
    mutex_unlock(&thread_mutex);
    if (i == MAX_THREAD_INFO)
        return -1;
    return i;
}

void *select_and_execute(void *arg) {
    int i, rv, rlen, les, es, pos_len;
    unsigned char buf[10240], *pos;
    int err;
    struct pollfd pf[MAX_SOCKS];
    int64_t lt, c_time;
    pthread_t tid;
    int read_ok;
    char ra[50];

    if (arg != NULL) // main thread is called with arg == NULL
        thread_index = get_thread_index();
    if (thread_index == -1) {
        _log(__FILE__, __LINE__,
             "Too many threads, increase MAX_THREAD_INFO from %d",
             MAX_THREAD_INFO);
        return NULL;
    }

    memset(thread_info[thread_index].thread_name, 0,
           sizeof(thread_info[thread_index].thread_name));
    if (arg) {
        safe_strncpy(thread_info[thread_index].thread_name, (char *)arg);
    } else
        strcpy(thread_info[thread_index].thread_name, "main");

    tid = get_tid();
    pthread_setname_np(tid, thread_info[thread_index].thread_name);

    select_timeout = SELECT_TIMEOUT;
    thread_info[thread_index].tid = tid;
    les = 1;
    es = 0;
    lt = getTick();
    memset(&pf, -1, sizeof(pf));
    LOG("Starting select_and_execute on thread ID %lx, thread_name %s", tid,
        thread_info[thread_index].thread_name);
    while (run_loop) {
        c_time = getTick();
        es = 0;
        clean_mutexes();
        for (i = 0; i < max_sock; i++)
            if (s[i] && s[i]->enabled && s[i]->tid == tid) {
                pf[i].fd = s[i]->sock;
                pf[i].events = s[i]->events;

                if (fifo_used(&s[i]->fifo))
                    pf[i].events |= POLLOUT;

                pf[i].revents = 0;
                s[i]->last_poll = c_time;
                es++;
            } else {
                pf[i].fd = -1;
                pf[i].events = pf[i].revents = 0;
            }
        i = -1;
        if (les == 0 && es == 0 && tid != main_tid) {
            LOG("No enabled sockets for Thread ID %lx name %s ... exiting ",
                tid, thread_info[thread_index].thread_name);
            break;
        }
        les = es;
        //    LOG("start select");
        if ((rv = poll(pf, max_sock, select_timeout)) < 0) {
            LOG("select_and_execute: select() error %d: %s", errno,
                strerror(errno));
            continue;
        }
        //              LOG("select returned %d",rv);
        if (rv > 0)
            while (++i < max_sock)
                if ((pf[i].fd >= 0) && pf[i].revents) {
                    sockets *ss = s[i];
                    if (!ss)
                        continue;
                    if (!ss->enabled)
                        continue;

                    c_time = getTick();
                    ss->iteration++;
                    ss->revents = pf[i].revents;
                    int buffered = fifo_used(&ss->fifo);

                    DEBUGM("event on socket index %d handle %d type %d "
                           "buffered %d (poll fd: %d, events: %d, revents: %d)",
                           i, ss->sock, ss->type, buffered, pf[i].fd,
                           pf[i].events, pf[i].revents);
                    sockets_lock(ss);

                    if ((pf[i].revents & POLLOUT) && buffered) {
                        LOGM("start flush sock id %d, buffered %d", ss->id,
                             buffered);
                        flush_socket(ss);

                        if ((pf[i].revents & (~POLLOUT)) == 0) {
                            DEBUGM("Sock %d: No Read event, continuing",
                                   ss->id);
                            sockets_unlock(ss);
                            continue;
                        }

                        pf[i].revents &= ~POLLOUT;
                    }

                    if (pf[i].revents & POLLOUT) {
                        ss->events &= ~POLLOUT;
                    }
                    // use the buffer of the master socket, but the FD and the
                    // read function of the slave socket
                    sockets *master = ss;
                    if (ss->master >= 0)
                        master = get_sockets(ss->master);
                    if (!master)
                        master = ss;

                    if (!master->buf || master->buf == buf) {
                        master->buf = buf;
                        master->lbuf = sizeof(buf) - 1;
                        master->rlen = 0;
                    }
                    if (master->rlen >= master->lbuf) {
                        DEBUGM("Socket buffer full, handle %d, sock_id %d (m: "
                               "%d), type "
                               "%d, lbuf %d, rlen %d, ss->buf = %p, buf %p",
                               master->sock, ss->id, master->id, master->type,
                               master->lbuf, master->rlen, master->buf, buf);
                        master->rlen = 0;
                    }
                    rlen = 0;
                    read_ok = 0;

                    pos = master->buf + master->rlen;
                    pos_len = master->lbuf - master->rlen;

                    if (ss->read)
                        read_ok = ss->read(ss->sock, pos, pos_len, ss, &rlen);

                    err = 0;
                    if (!read_ok)
                        err = errno;

                    if (opts.log & LOG_SOCKET) {
                        int64_t now = getTick();
                        if (now - c_time > 100)
                            LOGM("WARNING: read on socket id %d, handle %d, "
                                 "took %jd ms",
                                 ss->id, ss->sock, now - c_time);
                    }

                    if (rlen > 0)
                        master->rtime = c_time;

                    if (read_ok && rlen >= 0)
                        master->rlen += rlen;
                    else {
                        if (master->rlen > 0) {
                            LOG("socket %d, handle %d, master %d, errno %d, "
                                "read_ok %d, "
                                "clearing buffer with len %d",
                                ss->id, ss->sock, master->id, err, read_ok,
                                master->rlen)
                        }
                        master->rlen = 0;
                    }

                    // force 0 at the end of the string
                    if (master->lbuf >= master->rlen)
                        master->buf[master->rlen] = 0;

                    DEBUGM("Read %s %d (rlen:%d/total:%d) bytes from %d [s: %d "
                           "m: %d] -> "
                           "%p (buf: %p) - iteration %jd action %p",
                           read_ok ? "OK" : "NOK", rlen, master->rlen,
                           master->lbuf, ss->sock, ss->id, master->id, pos,
                           master->buf, ss->iteration, master->action);

                    if (((master->rlen > 0) || err == EWOULDBLOCK) &&
                        master->action && (master->type != TYPE_SERVER))
                        master->action(master);

                    sockets_unlock(ss);

                    if (!read_ok && ss->type != TYPE_SERVER) {
                        char *err_str;
                        char *types[] = {"udp",  "tcp", "server", "http",
                                         "rtsp", "dvr", NULL,     NULL};
                        if (rlen == 0) {
                            err = 0;
                            err_str = "Close";
                        } else if (err == EOVERFLOW)
                            err_str = "EOVERFLOW";
                        else if (err == EWOULDBLOCK)
                            err_str = "Connected";
                        else
                            err_str = strerror(err);

                        if (ss->sock == SOCK_TIMEOUT) {
                            LOG("ignoring error on sock_id %d handle %d type "
                                "%d error %d : "
                                "%s",
                                ss->id, ss->sock, ss->type, err, err_str);
                            continue; // do not close the RTCP socket, we might
                                      // get some errors here but ignore them
                        }
                        LOG("select_and_execute[%d]: %s on socket %d (sid:%d) "
                            "from %s:%d - "
                            "type %s (%d) errno %d",
                            i, err_str, ss->sock, ss->sid,
                            get_sockaddr_host(ss->sa, ra, sizeof(ra)),
                            get_sockaddr_port(ss->sa), types[ss->type & 0x7],
                            ss->type, err);
                        if (err == EOVERFLOW || err == EWOULDBLOCK ||
                            err == EINTR)
                            continue;
                        if (err == EAGAIN) {
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
        // checking every 60seconds for idle connections - or if select times
        // out
        c_time = getTick();
        if (rv == 0 || (c_time - lt >= 200)) {
            sockets *ss;
            lt = c_time;
            i = -1;
            while (++i < max_sock) {
                if ((ss = get_sockets(i)) == NULL || (ss->tid != tid))
                    continue;
                if (((ss->timeout_ms > 0) &&
                     (lt - ss->rtime > ss->timeout_ms) &&
                     (fifo_used(&ss->fifo) == 0)) ||
                    (ss->force_close)) {
                    if (ss->timeout && !ss->force_close) {
                        int rv;
                        if (ss->sock == SOCK_TIMEOUT)
                            ss->rtime = getTick();
                        sockets_lock(ss);
                        rv = ss->timeout(ss);
                        sockets_unlock(ss);
                        if (rv)
                            sockets_del(i);
                    } else
                        sockets_del(i);
                }
            }
        }
    }

    clean_mutexes();

    if (tid == main_tid)
        LOG("The main loop ended, run_loop = %d", run_loop)
    else
        add_join_thread(tid);
    thread_info[thread_index].enabled = 0;

    return NULL;
}

#undef DEFAULT_LOG
#define DEFAULT_LOG LOG_SOCKETWORKS

void sockets_setread(int i, void *r) {
    sockets *ss = get_sockets(i);
    if (ss)
        ss->read = (read_action)r;
}

void sockets_setclose(int i, void *r) {
    sockets *ss = get_sockets(i);
    if (ss)
        ss->close = (socket_action)r;
}

void sockets_setbuf(int i, char *buf, int len) {
    sockets *ss = get_sockets(i);
    if (ss) {
        ss->buf = (unsigned char *)buf;
        ss->lbuf = len;
    }
}

void sockets_timeout(int i, int t) {
    sockets *ss = get_sockets(i);
    if (ss) {
        ss->timeout_ms = t;
    }
}

void set_sockets_rtime(int i, int r) {
    sockets *ss = get_sockets(i);
    if (ss)
        ss->rtime = r;
}

int sockets_del_for_sid(int sid) {
    int i;
    sockets *ss;
    if (sid < 0)
        return 0;
    for (i = 0; i < MAX_SOCKS; i++)
        if ((ss = get_sockets(i)) && ss->sid >= 0 && ss->type == TYPE_RTSP &&
            ss->sid == sid) {
            ss->sid =
                -1; // make sure the stream is not closed in the future to
                    // prevent closing the stream created by another socket
        }
    return 0;
}

void set_socket_buffer(int sid, unsigned char *buf, int len) {
    sockets *ss = get_sockets(sid);
    if (!ss)
        return;

    ss->buf = buf;
    ss->lbuf = len;
}

void free_all_streams();
void free_all_adapters();
void free_all_keys();

void free_all() {
    int i = 0;

    for (i = MAX_SOCKS - 1; i > 0; i--) {
        if (!s[i])
            continue;
        if (s[i]->enabled)
            sockets_del(i);
        free_fifo(&s[i]->fifo);
        _free(s[i]->prio_data);
        if (s[i])
            _free(s[i]);
        s[i] = NULL;
    }
    free_all_streams();
    free_all_adapters();
#ifndef DISABLE_DVBAPI
    free_all_keys();
#endif
#ifndef DISABLE_T2MI
    free_t2mi();
#endif
    free_alloc();
}

void set_socket_send_buffer(int sock, int len) {
    int sl;
    int rv = 0;
    if (len <= 0)
        return;
// len = 8*1024; /* have a nice testing !!!! */
#ifdef SO_SNDBUFFORCE
    if ((rv = setsockopt(sock, SOL_SOCKET, SO_SNDBUFFORCE, &len, sizeof(len))))
        LOG("unable to set output socket buffer (force) size to %d", len);
#endif
    if (rv && setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &len, sizeof(len)))
        LOG("unable to set output socket buffer size to %d", len);
    sl = sizeof(int);
    if (!getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &len, (socklen_t *)&sl))
        LOG("output socket buffer size for socket %d is %d bytes", sock, len);
}

void set_socket_receive_buffer(int sock, int len) {
    socklen_t sl;
    int rv = 0;
    if (len <= 0)
        return;
#ifdef SO_RCVBUFFORCE
    if ((rv = setsockopt(sock, SOL_SOCKET, SO_RCVBUFFORCE, &len, sizeof(len))))
        LOG("unable to set receive socket buffer (force) size to %d", len);
#endif
    if (rv && setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &len, sizeof(len)))
        LOG("unable to set receive socket buffer size to %d", len);
    sl = sizeof(int);
    if (!getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &len, &sl))
        LOG("receive socket buffer size is %d bytes", len);
}

void set_socket_pos(int sock, int pos) {
    sockets *ss = get_sockets(sock);
    if (!ss)
        return;
    ss->rlen = pos;
}

char *get_sock_shost(int fd, char *dest, int ld) {
    USockAddr sin;
    socklen_t len = sizeof(sin);
    if (getsockname(fd, &sin.sa, &len))
        return "none";
    return get_sockaddr_host(sin, dest, ld);
}

int get_sock_sport(int fd) {
    USockAddr sin;
    socklen_t len = sizeof(sin);
    if (getsockname(fd, &sin.sa, &len))
        return -1;
    return get_sockaddr_port(sin);
}

int get_sockaddr_port(USockAddr s) {
    if (s.sa.sa_family == AF_INET)
        return ntohs(s.sin.sin_port);
    else if (s.sa.sa_family == AF_INET6)
        return ntohs(s.sin6.sin6_port);
    return 0;
}

char *get_sockaddr_host(USockAddr s, char *dest, int ld) {
    void *p = (void *)&s.sin.sin_addr;
    dest[0] = 0;
    memset(dest, 0, ld);
    if (s.sa.sa_family == AF_INET6)
        p = (void *)&s.sin6.sin6_addr;
    else if (s.sa.sa_family != AF_INET)
        return dest;
    inet_ntop(s.sa.sa_family, p, dest, ld);
    if (s.sa.sa_family == AF_INET6) // convert IPv4 in IPv6 to IPv4 address
    {
        if (!strncmp(dest, "::ffff:", 7)) {
            memmove(dest, dest + 7, ld - 7);
        }
    }
    return dest;
}

void set_socket_iteration(int s_id, uint64_t it) {
    sockets *ss = get_sockets(s_id);
    if (!ss)
        return;
    ss->iteration = it;
}

void set_socket_thread(int s_id, pthread_t tid) {
    sockets *ss = get_sockets(s_id);
    if (!ss)
        return;
    LOG("set_socket_thread: thread %lx for sockets %i", tid, s_id);
    ss->tid = tid;
}

pthread_t get_socket_thread(int s_id) {
    sockets *ss = get_sockets(s_id);
    if (!ss)
        LOG_AND_RETURN(0, "get_socket_thread: socket is NULL for s_id %d",
                       s_id);
    return ss->tid;
}
#undef DEFAULT_LOG
#define DEFAULT_LOG LOG_SOCKET

#ifdef __APPLE__
struct mmsghdr {
    struct msghdr msg_hdr; /* Message header */
    unsigned int msg_len;  /* Number of bytes transmitted */
};
#endif

#if defined(__APPLE__) || defined(__SH4__) || defined(NEEDS_SENDMMSG_SHIM)
int sendmmsg0(int rsock, struct mmsghdr *msg, int len, int t) {
    int i;
    for (i = 0; i < len; i++)
        writev(rsock, msg[i].msg_hdr.msg_iov, msg[i].msg_hdr.msg_iovlen);
    return len;
}
#define sendmmsg(a, b, c, d) sendmmsg0(a, b, c, d)
#endif

int writev_udp(int rsock, struct iovec *iov, int iiov) {
    struct mmsghdr msg[1024];
    int i, j = 0, last_i = 0, retval, total_iov = 0;
    int total_len = 0;
    memset(msg, 0, sizeof(msg));

    for (i = 0; i < iiov; i++) {
        if (iov[i].iov_len < 188) {
            if (j > 0)
                msg[j - 1].msg_hdr.msg_iovlen = i - last_i;

            msg[j].msg_hdr.msg_iov = iov + i;
            last_i = i;
            j++;
        }
    }
    if (j > 0)
        msg[j - 1].msg_hdr.msg_iovlen = i - last_i;
    retval = sendmmsg(rsock, msg, j, 0);
    if (retval == -1)
        LOG("sendmmsg(): errno %d: %s", errno, strerror(errno))
    else if (retval != j)
        LOG("%s: %d of out %d UDP messages sent", __FUNCTION__, retval, j);

    for (i = 0; i < retval; i++)
        total_iov += msg[i].msg_hdr.msg_iovlen;
    for (i = 0; i < total_iov; i++)
        total_len += iov[i].iov_len;
    return total_len;
}

// returns -1 or -EWOULDBLOCK
int my_writev(sockets *s, struct iovec *iov, int iiov) {
    int rv = 0, len = 0, i;
    char ra[50];
    int64_t stime = 0;
    int _errno;

    len = 0;
    for (i = 0; i < iiov; i++)
        len += iov[i].iov_len;

    DEBUGM("start writev handle %d, iiov %d, len %d", s->sock, iiov, len);
    if (opts.log & DEFAULT_LOG)
        stime = getTick();

    if (s->sock > 0) {
        if (s->type == TYPE_UDP && len > 1450)
            rv = writev_udp(s->sock, iov, iiov);
        else
            rv = writev(s->sock, iov, iiov);
    }
    _errno = errno;

    if (opts.log & DEFAULT_LOG)
        stime = getTick() - stime;

    if (rv > 0) {
        bw += rv;
        writes++;
    }

    if (rv != len) {
        if (rv <= 0)
            failed_writes++;
        DEBUGM("writev handle %d, iiov %d, len %d, rv %d, errno %d", s->sock,
               iiov, len, rv, _errno);
    }
    errno = _errno;
    if ((rv < 0) && (_errno == EWOULDBLOCK)) // blocking
        return -EWOULDBLOCK;

    if (rv < 0 && (errno == ECONNREFUSED ||
                   errno == EPIPE)) // close the stream int the next second
    {
        LOG("Connection REFUSED on handle %d, socket %d (sid %d), closing "
            "the "
            "socket, remote %s:%d",
            s->sock, s->id, s->sid, get_sockaddr_host(s->sa, ra, sizeof(ra)),
            get_sockaddr_port(s->sa));
        if (s->type != TYPE_RTCP)
            sockets_force_close(s->id);
    }
    if (rv < 0) {
        s->sock_err++;
        LOG("writev returned %d handle %d, iiov %d errno %d error %s", rv,
            s->sock, iiov, errno, strerror(errno));
    } else
        s->sock_err = 0;

    DEBUGM("writev returned %d handle %d, iiov %d, len %d (took %jd ms)", rv,
           s->sock, iiov, len, stime);
    errno = _errno;
    return rv;
}

int socket_enque_highprio(sockets *s, struct iovec *iov, int iovcnt) {
    int len = 0, i, pos = 0;
    for (i = 0; i < iovcnt; i++)
        len += iov[i].iov_len;

    pos = s->prio_data_len;

    if (ensure_allocated((void **)&s->prio_data, 0, 1, pos + len, 1024))
        return 0;

    for (i = 0; i < iovcnt; i++) {
        memcpy(s->prio_data + pos, iov[i].iov_base, iov[i].iov_len);
        pos += iov[i].iov_len;
    }
    s->prio_data_len = pos;
    return 0;
}

// finds the next RTSP/TCP Header which starts with:
//   -  0x24 0x00 [len len] 0x80 0x21 for TS data
//   -  0x24 0x01 [len len] 0x80 0xC8
int find_next_rtsp_header(SFIFO *fifo) {
    uint64_t offset = fifo->read_index;
    while (offset < fifo->write_index - 8) {
        if ((((fifo_peek_32(fifo, offset) & 0xFFFF0000) == 0x24000000) &&
             (fifo_peek_32(fifo, offset + 4) & 0xFFFF0000) == 0x80210000))
            return offset - fifo->read_index;
        if ((((fifo_peek_32(fifo, offset) & 0xFFFF0000) == 0x24010000) &&
             (fifo_peek_32(fifo, offset + 4) & 0xFFFF0000) == 0x80C80000))
            return offset - fifo->read_index;
        offset++;
    }
    return -1;
}

// copies the data from a iov structure to the fifo, starting with offset
// if offset is 0 the entire data is copied.
int copy_iovec_to_fifo(SFIFO *fifo, int offset, struct iovec *iov, int iovcnt) {
    int len = 0, i;
    uint64_t available;
    int size = opts.max_sbuf * 1048576;

    if (create_fifo(fifo, size))
        LOG_AND_RETURN(0, "Unable to allocate the FIFO with size %d", size);
    available = fifo_available(fifo);

    for (i = 0; i < iovcnt; i++)
        len += iov[i].iov_len;
    if (len - offset > available)
        return 0;

    for (i = 0; i < iovcnt; i++) {
        if (offset < iov[i].iov_len) {
            fifo_push(fifo, iov[i].iov_base + offset, iov[i].iov_len - offset);
        }
        offset = (offset > iov[i].iov_len) ? offset - iov[i].iov_len : 0;
    }
    return available - fifo_available(fifo);
}

// if we need the user tuned to a new freq, drop all the data that is in
// the buffer if both high_prio and flush_enqued_data are both set, no
// need to add to pio_pack
void flush_enqued_data_if_neededf_needed(sockets *s) {
    if (!s->flush_enqued_data)
        return;

    int off = find_next_rtsp_header(&s->fifo);
    if (off >= 0) {
        LOG("sock %d dropping %jd enqueued bytes (offset %d)", s->id,
            s->fifo.write_index - (s->fifo.read_index + off), off);
        s->fifo.write_index = s->fifo.read_index + off;
    }
    s->flush_enqued_data = 0;
}

void sockets_set_flush_enqued_data(int i) {
    sockets *s = get_sockets(i);
    if (s)
        s->flush_enqued_data = 1;
}

int sockets_writev_prio(int sock_id, struct iovec *iov, int iovcnt,
                        int high_prio) {
    int i, len = 0, offset = 0, rv = 0;
    sockets *s = get_sockets(sock_id);
    if (!s)
        return -1;
    for (i = 0; i < iovcnt; i++)
        len += iov[i].iov_len;
    if (len == 0)
        return 0;

    if (fifo_used(&s->fifo) == 0) {
        rv = my_writev(s, iov, iovcnt);
        if (rv == len || rv == 0 || rv == -1) {
            s->flush_enqued_data = 0;
            return rv;
        }
        if (rv == -EWOULDBLOCK)
            rv = 0;

        if (rv >= 0) {
            offset = rv;
        }
    }

    // high priority packets will be queued in high_prio and flushed after
    // the current packet example http_response, to avoid waiting all other
    // rtsp data to be dequeued first
    if (high_prio && !s->flush_enqued_data && (fifo_used(&s->fifo) > 0)) {
        return socket_enque_highprio(s, iov, iovcnt);
    }

    flush_enqued_data_if_neededf_needed(s);

    // buffer unwritten data to FIFO
    int copied = copy_iovec_to_fifo(&s->fifo, offset, iov, iovcnt);
    if (!copied) {
        if ((s->overflow == 0) || (opts.log & DEFAULT_LOG))
            LOG("Insufficient bandwidth: sock %d: overflow %d bytes, total "
                "%.1f MB, len %d, offset %d, rv %d, available %d",
                s->id, len - offset, s->overflow / 1048576.0, len, offset, rv,
                fifo_available(&s->fifo));
        s->overflow += len - offset;
        dropped_bytes += len - offset;

        return 0;
    }
    if (copied + offset != len)
        LOG("Unhandled sock %d data: copied %d offset %d len %d", s->id, copied,
            offset, len);
    LOGM("got %d bytes, offset %d, rv %d, buffered %d bytes - used %d", len,
         offset, rv, copied, fifo_used(&s->fifo));
    s->buffered_bytes += copied;
    buffered_bytes += copied;
    return offset;
}

int sockets_write(int sock_id, void *buf, int len) {
    struct iovec iov[1];
    iov[0].iov_base = buf;
    iov[0].iov_len = len;
    return sockets_writev(sock_id, iov, 1);
}

int flush_socket_all(sockets *s, int used) {
    struct iovec iov[2];
    int i = 0, rv = 0;
    uint64_t ctime = getTick();

    if (used == 0)
        return 0;

    iov[0].iov_len = fifo_peek(&s->fifo, &iov[0].iov_base, used, 0);
    i = 1;
    // the entire FIFO can be read using 2 pointers
    if (iov[0].iov_len < used) {
        iov[1].iov_len = fifo_peek(&s->fifo, &iov[1].iov_base,
                                   used - iov[0].iov_len, iov[0].iov_len);
        i++;
    }
    rv = my_writev(s, iov, i);

    if (rv <= 0)
        return rv;
    fifo_skip_bytes(&s->fifo, rv);

    LOGM("SOCK %d: flushed %d (out of %d) bytes in %jd "
         "ms",
         s->id, rv, used, getTick() - ctime);

    return rv;
}

int flush_socket_prio(sockets *s) {
    struct iovec iov[2];
    uint64_t ctime = getTick();
    int rv = 0;
    int left = find_next_rtsp_header(&s->fifo);
    memset(iov, 0, sizeof(iov));
    if (left > 0) {
        if ((rv = flush_socket_all(s, left)) >= 0)
            left -= rv;
    }

    if (left > 0)
        LOG_AND_RETURN(left,
                       "sock %d flushed incomplete packet before priority "
                       "response: left %d, rv %d in %jd ms",
                       s->id, left, rv, getTick() - ctime);

    iov[0].iov_base = s->prio_data;
    iov[0].iov_len = s->prio_data_len;
    rv = my_writev(s, iov, 1);
    LOG("sock %d, flushed %d bytes of priority data from %d", s->id, rv,
        s->prio_data_len);
    if (rv <= 0)
        return 1;
    if ((rv > 0) && (rv != s->prio_data_len)) {
        LOG("sock %d incomplete priority data write %d left %d in %jd ms",
            s->id, rv, s->prio_data_len - rv, getTick() - ctime);
        memmove(s->prio_data, s->prio_data + rv, s->prio_data_len - rv);
        s->prio_data_len -= rv;
        return 1;
    }
    s->prio_data_len = 0;

    if (s->force_close && (fifo_used(&s->fifo) == 0)) {
        LOGM("SOCK %d: set timeout_ms to 1 from %d", s->id, s->timeout_ms);
        s->timeout_ms = 1;
    }
    return rv;
}

int flush_socket(sockets *s) {
    flush_enqued_data_if_neededf_needed(s);
    if (s->force_close || s->prio_data_len) {
        return flush_socket_prio(s);
    }
    return flush_socket_all(s, fifo_used(&s->fifo));
}

void set_sockets_sid(int id, int sid) {
    sockets *s = get_sockets(id);
    if (s)
        s->sid = sid;
    else
        LOGM("sid for socket id %d could not be set", id);
}

void set_socket_dscp(int id, int dscp, int prio) {
    int d;

    d = dscp & IPTOS_DSCP_MASK_VALUE;
    if (setsockopt(id, IPPROTO_IP, IP_TOS, &d, sizeof(d)))
        LOG("%s: setsockopt IP_TOS failed", __FUNCTION__);

#if defined(SO_PRIORITY)
    d = prio;
    if (setsockopt(id, SOL_SOCKET, SO_PRIORITY, &d, sizeof(d)))
        LOG("%s: setsockopt SO_PRIORITY failed", __FUNCTION__);
#else
    LOG("%s: setsockopt SO_PRIORITY not implemented", __FUNCTION__);
#endif
}

void sockets_set_opaque(int id, void *opaque, void *opaque2, void *opaque3) {
    sockets *s = get_sockets(id);
    if (s) {
        s->opaque = opaque;
        s->opaque2 = opaque2;
        s->opaque3 = opaque3;
    }
}

// does not flush all the buffers before close, but ensures the socket will be
// closed regardless of the result of the timeout function

void sockets_force_close(int id) {
    sockets *s = get_sockets(id);
    if (s) {
        s->force_close = 1;
    }
}

void sockets_set_master(int slave, int master) {
    sockets *s = get_sockets(slave);
    sockets *m = get_sockets(master);

    if (!s || !m) {
        LOG("Unable to set master for sockets %d to %d as one of them is NULL",
            slave, master);
        return;
    }
    s->tid = m->tid;
    s->master = master;
    LOG("sock %d is master for sock %d", s->master, s->id);
}
