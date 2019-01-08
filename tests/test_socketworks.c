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

#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <net/if.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include <sys/ucontext.h>
#include <unistd.h>
#include <syslog.h>
#include <stdarg.h>
#include <sys/mman.h>
#include "dvb.h"
#include "utils.h"
#include "minisatip.h"
#include "socketworks.h"
#include "ddci.h"

#define DEFAULT_LOG LOG_DVBCA

extern sockets *s[MAX_SOCKS];
int read_fd, id;
int test_socket_enque_highprio()
{
	sockets *ss = s[id];
	struct iovec iov[2];
	iov[0].iov_base = "test1";
	iov[0].iov_len = 5;
	iov[1].iov_base = "test2";
	iov[1].iov_len = 5;
	socket_enque_highprio(ss, iov, 2);
	iov[0].iov_base = "test3";
	socket_enque_highprio(ss, iov, 1);
	if (strncmp((char *)ss->prio_pack.buf, "test1test2test3", ss->prio_pack.len))
		LOG_AND_RETURN(1, "expected != the actual result: %s len %d", ss->prio_pack.buf, ss->prio_pack.len);
	free_pack(&ss->prio_pack);
	iov[0].iov_base = "test";
	iov[0].iov_len = 4;
	socket_enque_highprio(ss, iov, 1);
	if (strncmp((char *)ss->prio_pack.buf, "test", ss->prio_pack.len))
		LOG_AND_RETURN(1, "expected (%s) != the actual result: %s len %d", iov[0].iov_base, ss->prio_pack.buf, ss->prio_pack.len);
	free_pack(&ss->prio_pack);
	return 0;
}

int test_socket_writev_prio()
{
	struct iovec iov[2];
	char buf[100];
	int rv;
	memset(buf, 0, sizeof(buf));
	memset(&iov, 0, sizeof(iov));
	sockets *ss = s[id];
	ss->wpos = 0;
	ss->spos = 5; // trigger buffering
	sockets_write(id, "0", 1);
	ss->spos = 0;
	sockets_write(id, "1", 1);
	iov[0].iov_base = "p";
	iov[0].iov_len = 1;
	sockets_writev_prio(id, iov, 1, 1);
	int k = 300;
	while (!flush_socket(ss))
		if (!k--)
			break;
	if (k <= 0)
		LOG_AND_RETURN(1, "flush_socket caused infinite loop k = %d", k);
	if (3 != (rv = read(read_fd, buf, sizeof(buf))))
		LOG_AND_RETURN(2, "read unexpected number of bytes %d", rv);
	if (strcmp(buf, "0p1"))
		LOG_AND_RETURN(2, "unexpected result: 0p1 != %s", buf);
	return 0;
}

int test_socket_writev_prio_flush()
{
	struct iovec iov[2];
	char buf[100];
	int rv;
	memset(buf, 0, sizeof(buf));
	memset(&iov, 0, sizeof(iov));
	sockets *ss = s[id];
	ss->wpos = 0;
	ss->spos = 5; // trigger buffering
	sockets_write(id, "0", 1);
	ss->spos = 0;
	sockets_write(id, "1", 1);
	iov[0].iov_base = "p";
	iov[0].iov_len = 1;
	ss->flush_enqued_data = 1;
	sockets_writev_prio(id, iov, 1, 1);
	sockets_write(id, "2", 1);
	int k = 300;
	while (!flush_socket(ss))
		if (!k--)
			break;
	if (k <= 0)
		LOG_AND_RETURN(1, "flush_socket caused infinite loop k = %d", k);
	if (3 != (rv = read(read_fd, buf, sizeof(buf))))
		LOG_AND_RETURN(2, "read unexpected number of bytes %d", rv);
	if (strcmp(buf, "0p2"))
		LOG_AND_RETURN(2, "unexpected result: 0p1 != %s", buf);
	return 0;
}

int main()
{
	opts.log = 1;//LOG_UTILS | LOG_SOCKET;
	strcpy(thread_name, "test");;
	_writev = writev;
	int fd[2];
	if (pipe(fd) == -1)
	{
		LOG("pipe failed errno %d: %s", errno, strerror(errno));
		return 1;
	}
	read_fd = fd[0];
	id = sockets_add(fd[1], NULL, -1, TYPE_UDP, NULL, NULL, NULL);
	s[id]->wmax = 10;
	TEST_FUNC(test_socket_enque_highprio(), "testing test_socket_enque_highprio");
	TEST_FUNC(test_socket_writev_prio(), "testing socket_writev_prio");
	TEST_FUNC(test_socket_writev_prio_flush(), "testing socket_writev_prio with flushing the queue");
	fflush(stdout);
	free_all();
	return 0;
}
