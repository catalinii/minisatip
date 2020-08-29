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
#include "ca.h"

#define DEFAULT_LOG LOG_PMT

extern adapter *a[MAX_ADAPTERS];
extern SFilter *filters[MAX_FILTERS];
extern SPMT *pmts[MAX_PMT];

int test_clean_psi_buffer_only()
{
	unsigned char pmt_sample[] = "\x02\xb0\x3b\x02\x06\xc9\x00\x00\xeb\xff\xf0\x18\x09\x04\x09\xc4"
								 "\xfb\x62\x09\x04\x09\x8c\xfa\x62\x09\x04\x09\xaf\xff\x62\x09\x04"
								 "\x09\x8d\xfc\x62\x1b\xeb\xff\xf0\x03\x52\x01\x02\x03\xec\x00\xf0"
								 "\x09\x0a\x04\x64\x65\x75\x01\x52\x01\x03\x7c\xae\xca\xf2";
	unsigned char clean[4096];
	memset(clean, 0, sizeof(clean));
	hexdump("Original PMT:", pmt_sample, sizeof(pmt_sample));
	int clean_size = clean_psi_buffer(pmt_sample, clean, sizeof(clean));
	hexdump("CLEAN ONLY:", clean, clean_size);
	if (clean_size > sizeof(pmt_sample))
		return 1;
	return 0;
}

int test_clean_psi_buffer()
{

	unsigned char pmt_sample[] = "\x02\xb1\x4e\x78\x57\xc1\x00\x00\xe0\xa1\xf0\x00\x1b\xe0\xa1\xf0"
								 "\x09\x52\x01\xa1\x09\x04\x18\x10\xe8\xb2\x06\xe0\x56\xf0\x13\x52"
								 "\x01\x56\x0a\x04\x73\x70\x61\x00\x09\x04\x18\x10\xe8\xb2\x6a\x02"
								 "\x80\x44\x06\xe0\x57\xf0\x13\x52\x01\x57\x0a\x04\x71\x61\x61\x00"
								 "\x09\x04\x18\x10\xe8\xb2\x6a\x02\x80\x44\x06\xe0\x23\xf0\x0d\x52"
								 "\x01\x23\x59\x08\x73\x70\x61\x10\x00\x04\x00\x05\x06\xe0\x24\xf0"
								 "\x0d\x52\x01\x24\x59\x08\x65\x73\x6c\x20\x00\x04\x00\x05\x06\xe0"
								 "\x25\xf0\x0a\x59\x08\x65\x6e\x67\x10\x00\x04\x00\x05\x06\xe0\x26"
								 "\xf0\x0a\x59\x08\x63\x61\x74\x10\x00\x04\x00\x05\x06\xe0\x27\xf0"
								 "\x0d\x52\x01\x27\x59\x08\x65\x75\x73\x10\x00\x04\x00\x05\xc0\xe0"
								 "\xd0\xf0\x41\xc6\x05\x01\x00\x05\x06\xff\xc2\x38\x43\x53\x45\x5f"
								 "\x41\x55\x44\x49\x43\x53\x45\x5f\x46\x55\x54\x31\x43\x53\x45\x5f"
								 "\x46\x55\x54\x32\x43\x53\x45\x5f\x47\x43\x4e\x31\x43\x53\x45\x5f"
								 "\x47\x43\x4e\x32\x43\x53\x45\x5f\x4d\x53\x44\x31\x43\x53\x45\x5f"
								 "\x4d\x53\x44\x32\xc0\xe1\x35\xf0\x0a\xc2\x08\x4c\x41\x4e\x5a\x00"
								 "\x00\x00\x00\xc1\xe0\xfd\xf0\x0a\xc2\x08\x44\x43\x4f\x4d\x55\x4e"
								 "\x00\x00\xc1\xe1\x33\xf0\x0a\xc2\x08\x4c\x41\x4e\x5a\x00\x00\x00"
								 "\x00\xc1\xe1\x64\xf0\x0a\xc2\x08\x4c\x4f\x52\x44\x00\x00\x00\x00"
								 "\xc1\xe1\x88\xf0\x0a\xc2\x08\x50\x52\x4f\x46\x49\x4c\x45\x00\xc1"
								 "\xe2\x6a\xf0\x0a\xc2\x08\x47\x43\x5f\x44\x41\x54\x41\x00\xc1\xe3"
								 "\x78\xf0\x0a\xc2\x08\x41\x53\x54\x52\x41\x00\x00\x00\x13\xaa\xe6\xbc";

	SPMT pmt;
	SFilter f;
	adapter ad;
	uint8_t clean[4096], data[4096];
	memset(&f, 0, sizeof(f));
	memset(&ad, 0, sizeof(a));
	memset(&pmt, 0, sizeof(pmt));
	memset(&clean, 0, sizeof(clean));
	filters[0] = &f;
	pmts[0] = &pmt;
	a[0] = &ad;
	f.pid = 99;
	pmt.adapter = 0;
	pmt.enabled = 1;
	ad.enabled = 1;
	ad.id = 0;
	ad.pids[0].pid = f.pid;
	ad.pids[0].flags = 1;
	f.adapter = 0;
	f.enabled = 1;
	f.next_filter = -1;
	clean[0] = 0;
	hexdump("START: ", pmt_sample, sizeof(pmt_sample));
	int clean_size = clean_psi_buffer(pmt_sample, clean + 1, sizeof(clean) - 1);
	if (clean_size < 1)
		LOG_AND_RETURN(1, "clean_psi_buffer failed");
	hexdump("CLEAN: ", clean, clean_size);
	char cc;
	int data_len = buffer_to_ts(data, sizeof(data), clean, clean_size, &cc, f.pid);
	hexdump("TS CLEAN: ", data, data_len);

	int i, len = 0;
	for (i = 0; i < data_len; i += 188)
		len = assemble_packet(&f, data + i);

	if (!len)
		LOG_AND_RETURN(1, "assemble_packet failed");
	LOG("Assemble packet returned %d", len);

	if (process_pmt(0, f.data, len, &pmt))
		LOG_AND_RETURN(1, "process_pmt failed");
#if defined(__x86_64__) || defined(__i386__)
	if (!createCAPMT(pmt.pmt, pmt.pmt_len, 1, clean, sizeof(clean), 0))
		LOG_AND_RETURN(1, "createCAPMT failed");
#endif
	return 0;
}

int main()
{
	opts.log = 1;
	opts.debug = 0;
	strcpy(thread_name, "test");
	TEST_FUNC(test_clean_psi_buffer_only(), "testing test_clean_psi_buffer_only");
	TEST_FUNC(test_clean_psi_buffer(), "testing test_clean_psi_buffer");
	fflush(stdout);
	return 0;
}
