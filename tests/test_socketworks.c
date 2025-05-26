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
#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64

#include "ddci.h"
#include "dvb.h"
#include "minisatip.h"
#include "socketworks.h"
#include "utils.h"
#include "utils/alloc.h"
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
    _free(ss->prio_data);
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

int main() {
    opts.log = 1; // LOG_UTILS | LOG_SOCKET;
    strcpy(thread_info[thread_index].thread_name, "test_socketworks");
    init_alloc();
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
    fflush(stdout);
    free_all();
    return 0;
}
