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

extern ddci_device_t *ddci_devices[MAX_ADAPTERS];
extern adapter *a[MAX_ADAPTERS];
int test_push_ts_to_ddci()
{
	ddci_device_t d;
	uint8_t buf[188 * 10];
	d.id = 0;
	d.enabled = 1;
	d.out = malloc1(DDCI_BUFFER + 10);
	d.wo = DDCI_BUFFER - 188;
	d.ro = 188;
	memset(ddci_devices, 0, sizeof(ddci_devices));
	ddci_devices[0] = &d;
	if (push_ts_to_ddci(&d, buf, 376) != 188)
		LOG_AND_RETURN(1, "push 376 bytes when 188 are left in the buffer");
	if (push_ts_to_ddci(&d, buf, 188) != 188)
		LOG_AND_RETURN(1, "push 188 bytes when 0 bytes are left in the buffer");
	d.ro = d.wo = 0;
	if (push_ts_to_ddci(&d, buf, 376) != 0)
		LOG_AND_RETURN(1, "push 376 bytes");
	return 0;
}

int test_copy_ts_from_ddci()
{
	ddci_device_t d;
	adapter ad;
	uint8_t buf[188 * 10], buf2[188 * 10];
	memset(&d, 0, sizeof(d));
	memset(&ad, 0, sizeof(ad));
	memset(buf, 0, sizeof(buf));
	memset(buf2, 0, sizeof(buf2));
	d.id = 0;
	d.enabled = 1;
	d.out = malloc1(DDCI_BUFFER + 10);
	memset(d.pid_mapping, -1, sizeof(d.pid_mapping));
	memset(ddci_devices, 0, sizeof(ddci_devices));
	ddci_devices[0] = &d;

	ad.id = 1;
	ad.enabled = 1;
	ad.buf = buf2;
	ad.lbuf = sizeof(buf2);
	a[1] = &ad;
	__attribute__((unused)) int ad_pos = 0;
	buf[0] = buf2[0] = 0x47;
	d.pid_mapping[1000] = 22; // forcing mapping to a different pid
	int new_pid = add_pid_mapping_table(1, 1000, 0, 0);
	ad.rlen = 188;
	set_pid_ts(buf, new_pid);
	set_pid_ts(buf2, 0x1FFF);

	if (copy_ts_from_ddci(&ad, &d, buf, &ad_pos))
		LOG_AND_RETURN(1, "could not copy the packet to the adapter");
	if (PID_FROM_TS(buf2) != 1000)
		LOG_AND_RETURN(1, "found pid %d expected %d", PID_FROM_TS(buf2), 1000);
	if (PID_FROM_TS(buf) != 0x1FFF)
		LOG_AND_RETURN(1, "PID from the DDCI buffer not marked correctly %d", PID_FROM_TS(buf));

	set_pid_ts(buf, new_pid);
	if (copy_ts_from_ddci(&ad, &d, buf, &ad_pos))
		LOG_AND_RETURN(1, "could not copy the packet to the adapter");
	if (ad.rlen != 376)
		LOG_AND_RETURN(1, "rlen not marked correctly %d", ad.rlen);
	ad.rlen = ad.lbuf;
	set_pid_ts(buf, new_pid);
	if (1 != copy_ts_from_ddci(&ad, &d, buf, &ad_pos))
		LOG_AND_RETURN(1, "buffer full not returned correctly");

	set_pid_ts(buf, 1200);
	if (0 != copy_ts_from_ddci(&ad, &d, buf, &ad_pos))
		LOG_AND_RETURN(1, "invalid pid not returned correctly");
	if (PID_FROM_TS(buf) != 1200)
		LOG_AND_RETURN(1, "invalid pid buffer pid not set correctly", PID_FROM_TS(buf));

	return 0;
}

int is_err = 0;
int expected_pid = 0;
int did_write = 0;
int xwritev(int fd, const struct iovec *io, int len)
{
	unsigned char *b = io[0].iov_base;
	LOGM("called writev with len %d, first pid %d", len, PID_FROM_TS(b));
	did_write = 1;
	if (len != 1)
	{
		is_err = 1;
		LOG_AND_RETURN(-1, "writev did not receive proper arguments, expected 1, got %d", len);
	}

	if (PID_FROM_TS(b) != expected_pid)
	{
		is_err = 1;
		LOG_AND_RETURN(-1, "writev did not receive proper TS, expected %d, got %d", expected_pid, PID_FROM_TS(b));
	}
	return len * 188;
}

int test_ddci_process_ts()
{
	ddci_device_t d;
	adapter ad;
	uint8_t buf[188 * 10];
	int i;
	memset(&d, 0, sizeof(d));
	memset(&ad, 0, sizeof(ad));
	memset(buf, 0, sizeof(buf));
	d.id = 0;
	d.enabled = 1;
	d.out = malloc1(DDCI_BUFFER + 10);
	memset(d.pid_mapping, -1, sizeof(d.pid_mapping));
	memset(ddci_devices, 0, sizeof(ddci_devices));
	ddci_devices[0] = &d;
	mutex_init(&d.mutex);
	ad.id = 1;
	ad.enabled = 1;
	ad.buf = buf;
	ad.lbuf = sizeof(buf);
	for (i = 0; i < ad.lbuf; i += 188)
		set_pid_ts(buf + i, 2121); // unmapped pid
	a[0]->enabled = 1;
	a[1] = &ad;
	buf[0] = 0x47;
	d.pid_mapping[1000] = 22; // forcing mapping to a different pid
	d.pid_mapping[2000] = 22; // forcing mapping to a different pid
	int new_pid = add_pid_mapping_table(1, 1000, 0, 0);
	int new_pid2 = add_pid_mapping_table(1, 2000, 0, 0);
	ad.rlen = ad.lbuf - 188; // allow just 1 packet + 1 cleared that it will be written to the socket
	set_pid_ts(ad.buf + 188, 1000);

	d.ro = DDCI_BUFFER - 188;		   // 1 packet before end of buffer
	d.wo = 188 * 2;					   // 2 after end of the buffer
	set_pid_ts(d.out + d.ro, new_pid); // first packet, expected 1000
	set_pid_ts(d.out, new_pid2);
	set_pid_ts(d.out + 188, new_pid2);
	expected_pid = new_pid;
	_writev = (mywritev)&xwritev;
	ddci_process_ts(&ad, &d);
	if (is_err)
		return 1;
	if (!did_write)
		LOG_AND_RETURN(1, "no writev called");
	if (PID_FROM_TS(ad.buf + 188) != 1000)
		LOG_AND_RETURN(1, "expected pid 1000 in the adapter buffer, got %d", PID_FROM_TS(ad.buf + 188));
	if (PID_FROM_TS(ad.buf + ad.lbuf - 188) != 2000)
		LOG_AND_RETURN(1, "expected pid 2000 in the adapter buffer, got %d", PID_FROM_TS(ad.buf + ad.lbuf - 188));
	if (ad.rlen != ad.lbuf)
		LOG_AND_RETURN(1, "adapter buffer length mismatch %d != %d", ad.rlen, ad.lbuf);
	if (d.ro != 188 && d.wo != 188 * 2)
		LOG_AND_RETURN(1, "indexes in DDCI devices set wrong ro %d wo %d", d.ro, d.wo);

	return 0;
}

int main()
{
	opts.log = 1;
	thread_name = "test";
	//	opts.log = LOG_DVBCA + 1;
	find_ddci_adapter(a);
	TEST_FUNC(test_push_ts_to_ddci(), "testing test_push_ts_to_ddci");
	TEST_FUNC(test_copy_ts_from_ddci(), "testing test_copy_ts_from_ddci");
	TEST_FUNC(test_ddci_process_ts(), "testing ddci_process_ts");
	fflush(stdout);
	return 0;
}