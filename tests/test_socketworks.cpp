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
#include "ddci.h"
#include "dvb.h"
#include "minisatip.h"
#include "socketworks.h"
#include "utils.h"
#include "utils/testing.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ucontext.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_LOG LOG_DVBCA

extern sockets *s[MAX_SOCKS];
int read_fd, id;
int test_socket_enque_highprio() {
    sockets *ss = s[id];
    struct iovec iov[2];
    iov[0].iov_base = (void *)"test1";
    iov[0].iov_len = 5;
    iov[1].iov_base = (void *)"test2";
    iov[1].iov_len = 5;
    socket_enque_highprio(ss, iov, 2);
    iov[0].iov_base = (void *)"test3";
    socket_enque_highprio(ss, iov, 1);
    if (strncmp((char *)ss->prio_data, "test1test2test3", ss->prio_data_len))
        LOG_AND_RETURN(1, "expected != the actual result: %s len %d",
                       ss->prio_data, ss->prio_data_len);
    free(ss->prio_data);
    ss->prio_data_len = 0;
    ss->prio_data = NULL;
    return 0;
}

int test_socket_writev_flush_enqued() {
    struct iovec iov[2];
    char buf[100];
    uint8_t start_rtsp_frame[] = {0x24, 0x0, 1, 2, 0x80, 0x21};
    int rv;
    memset(buf, 0, sizeof(buf));
    memset(&iov, 0, sizeof(iov));
    sockets *ss = s[id];
    create_fifo(&ss->fifo, 100);

    // flush enqued data (everything after 0x24 is not written)
    ss->fifo.write_index = 1;
    ss->fifo.read_index = 0; // trigger buffering
    char *data = (char *)ss->fifo.data;
    data[0] = '0';
    sockets_write(id, (void *)"1", 1);
    sockets_write(id, start_rtsp_frame, sizeof(start_rtsp_frame));
    sockets_write(id, buf, sizeof(buf) / 2);
    ss->flush_enqued_data = 1;
    sockets_write(id, (void *)"2", 1);
    flush_socket(ss);
    if (3 != (rv = read(read_fd, buf, sizeof(buf))))
        LOG_AND_RETURN(2, "read unexpected number of bytes %d", rv);
    if (strncmp(buf, "012", 3))
        LOG_AND_RETURN(2, "unexpected result: 012 != %s", buf);

    free_fifo(&ss->fifo);
    return 0;
}

uint8_t test_buf[1000];
int buf_pos;
int to_write;

// simulates a write to the buffer
// assumes all data to write is less than the buffer size
ssize_t writev_test(int rsock, const struct iovec *iov, int iiov) {
    int i;
    int left = to_write;
    int old_pos = buf_pos;
    for (i = 0; i < iiov; i++) {
        int w = (left < iov[i].iov_len) ? left : iov[i].iov_len;
        memcpy(test_buf + buf_pos, iov[i].iov_base, w);
        left -= w;
        buf_pos += w;
    }
    return buf_pos - old_pos;
}

int test_socket_buffering() {
    struct iovec iov[3];
    char buf[255];
    uint8_t start_rtsp_frame[] = {0x24, 0x0, 1, 2, 0x80, 0x21};
    int i;
    // account for 9 missing bytes at offset 50 (prio data + start_rtsp_frame)
    for (i = 0; i < sizeof(buf); i++)
        if (i < 50)
            buf[i] = i;
        else
            buf[i] = i + 9;

    memset(&iov, 0, sizeof(iov));
    sockets *ss = s[id];
    create_fifo(&ss->fifo, 100);
    ss->fifo.write_index = 50;
    ss->fifo.read_index = 50;
    _writev = writev_test;
    // test writing flushing multiple packets at once
    to_write = 1;
    for (i = 0; i < 10; i++) {
        if (i == 5)
            sockets_write(id, start_rtsp_frame, sizeof(start_rtsp_frame));
        sockets_write(id, buf + i * 10, 10);
    }

    ASSERT_EQUAL(ss->overflow, 10, "Expected overflow");
    // prio data
    iov[0].iov_base = (void *)"XYZ";
    iov[0].iov_len = 3;
    sockets_writev_prio(id, iov, 1, 1);
    _hexdump("buffered data: ", ss->fifo.data, 100);
    // trigger partial write
    to_write = 47;
    flush_socket(ss);
    to_write = 5;
    flush_socket(ss);
    to_write = 5;
    flush_socket(ss);
    to_write = 50;
    flush_socket(ss);
    _hexdump("read data: ", test_buf, 95);

    for (i = 0; i < 90; i++) {
        char message[100];
        sprintf(message,
                "Invalid value found in the buffer at position %d: %02X", i,
                test_buf[i]);
        if (i < 50 || i > 58)
            ASSERT(test_buf[i] == i, message);
    }
    ASSERT(memcmp(test_buf + 50, start_rtsp_frame, sizeof(start_rtsp_frame)),
           "RTSP frame not found at expected position");

    // test copy_iovec_to_fifo
    iov[0].iov_base = buf;
    iov[0].iov_len = 10;
    iov[1].iov_base = buf + 10;
    iov[1].iov_len = 10;
    iov[2].iov_base = buf + 20;
    iov[2].iov_len = 10;
    ss->fifo.read_index = ss->fifo.write_index = 0;
    copy_iovec_to_fifo(&ss->fifo, 0, iov, 3);
    ASSERT(!memcmp(buf, ss->fifo.data, fifo_used(&ss->fifo)),
           "copy_iovec_to_fifo failed with 0 offset");
    ss->fifo.read_index = ss->fifo.write_index = 0;
    copy_iovec_to_fifo(&ss->fifo, 19, iov, 3);
    ASSERT(!memcmp(buf + 19, ss->fifo.data, fifo_used(&ss->fifo)),
           "copy_iovec_to_fifo failed with custom offset offset");

    free_fifo(&ss->fifo);

    return 0;
}

int test_bind_dev() {
    char ip[MAX_HOST], baseline[MAX_HOST];
    char *saved_bind = opts.bind;
    char *saved_http = opts.bind_http;
    char *saved_dev = opts.bind_dev;
    char *saved_disc = opts.disc_host;
    int saved_ipv4 = opts.use_ipv4_only;
    opts.bind = opts.bind_http = opts.bind_dev = NULL;
    opts.disc_host = (char *)"239.255.255.250";
    opts.use_ipv4_only = 1;

    ASSERT(get_dev_ip(NULL, ip, sizeof(ip)), "NULL dev must fail");
    ASSERT(get_dev_ip((char *)"", ip, sizeof(ip)), "empty dev must fail");
    ASSERT(get_dev_ip((char *)"lo", NULL, 10), "NULL buf must fail");
    ASSERT(get_dev_ip((char *)"lo", ip, 4), "short buf must fail");
    ASSERT(!validate_bind_dev((char *)"eth0"), "eth0 must validate");
    ASSERT(validate_bind_dev(NULL), "NULL must not validate");
    ASSERT(validate_bind_dev((char *)""), "empty must not validate");
    ASSERT(!validate_bind_dev((char *)"123456789012345"), "15 chars must pass");
    ASSERT(validate_bind_dev((char *)"1234567890123456"), "16 chars must fail");
    ASSERT(validate_bind_dev((char *)"a/b"), "slash must not validate");
    ASSERT(validate_bind_dev((char *)"a\nb"), "newline must not validate");

    char *dev = NULL;
    if (!get_dev_ip((char *)"lo", ip, sizeof(ip)))
        dev = (char *)"lo";
    else if (!get_dev_ip((char *)"lo0", ip, sizeof(ip)))
        dev = (char *)"lo0";
    if (!dev) {
        opts.disc_host = saved_disc;
        opts.use_ipv4_only = saved_ipv4;
        LOG("no loopback device, skipping bind-dev test");
        return 0;
    }

    safe_strncpy(baseline, getlocalip());

    opts.bind = (char *)"127.0.0.2";
    opts.bind_dev = dev;
    ASSERT(resolve_bind_opts(), "bind + bind-dev must conflict");
    opts.bind = NULL;
    opts.bind_http = (char *)"127.0.0.2";
    ASSERT(resolve_bind_opts(), "bind-http + bind-dev must conflict");
    opts.bind_http = NULL;
    opts.bind_dev = (char *)"no-such-dev-xyz";
    ASSERT(resolve_bind_opts(), "unknown dev must fail");

    opts.bind_dev = dev;
    ASSERT(!resolve_bind_opts(), "loopback dev must resolve");
    ASSERT(opts.bind && !strcmp(opts.bind, ip), "bind must hold dev IP");
    ASSERT(opts.bind_http && !strcmp(opts.bind_http, ip),
           "bind-http must hold dev IP");
    ASSERT(bind_dev_ip() && !strcmp(bind_dev_ip(), ip),
           "source IP must be dev IP");
    ASSERT(!strcmp(getlocalip(), baseline),
           "bind-dev must not affect getlocalip");

    int fd = tcp_listen(opts.bind, 0, 1);
    char tcp_host[100] = {0};
    const char *tcp_got =
        fd >= 0 ? get_sock_shost(fd, tcp_host, sizeof(tcp_host)) : NULL;
    int dev_ok = -1;
#ifdef SO_BINDTODEVICE
    int soft = -2;
    if (fd >= 0) {
        int t = socket(AF_INET, SOCK_STREAM, 0);
        if (t >= 0) {
            soft = set_socket_bind_dev(t);
            close(t);
        }
        if (soft == 0)
            dev_ok = socket_bind_dev_ok(fd) ? 1 : 0;
    }
#endif
    int afd = tcp_listen(NULL, 0, 1);
    char any_host[100] = {0};
    const char *any_got =
        afd >= 0 ? get_sock_shost(afd, any_host, sizeof(any_host)) : NULL;
    int ufd = udp_bind(opts.bind, 0, 1);
    char udp_host[100] = {0};
    const char *udp_got =
        ufd >= 0 ? get_sock_shost(ufd, udp_host, sizeof(udp_host)) : NULL;
    int cfd = -1;
    char conn_host[100] = {0};
    if (fd >= 0) {
        int port = get_sock_sport(fd);
        if (port > 0)
            cfd = tcp_connect_src(ip, port, NULL, 1, bind_dev_ip());
        if (cfd >= 0)
            get_sock_shost(cfd, conn_host, sizeof(conn_host));
    }
    if (fd >= 0)
        close(fd);
    if (afd >= 0)
        close(afd);
    if (ufd >= 0)
        close(ufd);
    if (cfd >= 0)
        close(cfd);
    ASSERT(tcp_got && !strcmp(tcp_host, ip), "tcp_listen must bind dev IP");
    ASSERT(any_got && !strcmp(any_host, "0.0.0.0"),
           "NULL must bind ANY without substitution");
    ASSERT(udp_got && !strcmp(udp_host, ip), "udp_bind must bind dev IP");
    ASSERT(cfd >= 0, "connect with explicit source must succeed");
    ASSERT(!strcmp(conn_host, ip), "connect must source dev IP");
#ifdef SO_BINDTODEVICE
    ASSERT(soft == 0 || soft == 1, "valid dev must apply or degrade");
#endif
    ASSERT(dev_ok != 0, "device restriction not applied");

    opts.bind = (char *)"127.0.0.2";
    opts.bind_http = NULL;
    opts.bind_dev = NULL;
    ASSERT(!resolve_bind_opts(), "bind alone must resolve");
    ASSERT(opts.bind_http && !strcmp(opts.bind_http, "127.0.0.2"),
           "bind-http must default to bind");
    ASSERT(!bind_dev_ip(), "no source IP without bind-dev");
    int efd = tcp_listen(opts.bind, 0, 1);
    char exp_host[100] = {0};
    const char *exp_got =
        efd >= 0 ? get_sock_shost(efd, exp_host, sizeof(exp_host)) : NULL;
    if (efd >= 0)
        close(efd);
    ASSERT(exp_got && !strcmp(exp_host, "127.0.0.2"),
           "bind must be used as-is");

    opts.bind = opts.bind_http = opts.bind_dev = NULL;
    ASSERT(!resolve_bind_opts(), "empty opts must resolve");
    ASSERT(!opts.bind && !opts.bind_http && !bind_dev_ip(),
           "empty opts must stay NULL");

    opts.bind = saved_bind;
    opts.bind_http = saved_http;
    opts.bind_dev = saved_dev;
    opts.disc_host = saved_disc;
    opts.use_ipv4_only = saved_ipv4;
    return 0;
}

int main() {
    opts.log = 1; // LOG_UTILS | LOG_SOCKET;
    strcpy(thread_info[thread_index].thread_name, "test_socketworks");
    _writev = writev;
    int fd[2];
    if (pipe(fd) == -1) {
        LOG("pipe failed errno %d: %s", errno, strerror(errno));
        return 1;
    }
    read_fd = fd[0];
    id = sockets_add(fd[1], NULL, -1, TYPE_UDP, NULL, NULL, NULL);
    TEST_FUNC(test_socket_enque_highprio(),
              "testing test_socket_enque_highprio");
    TEST_FUNC(test_socket_writev_flush_enqued(),
              "testing socket_writev with flushing the queue");
    TEST_FUNC(test_socket_buffering(), "testing socket buffering and flushing");
    TEST_FUNC(test_bind_dev(), "testing bind-dev handling");
    fflush(stdout);
    free_all();
    return 0;
}
