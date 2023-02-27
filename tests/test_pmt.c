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

#include "ca.h"
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

#define DEFAULT_LOG LOG_PMT

extern adapter *a[MAX_ADAPTERS];
extern SFilter *filters[MAX_FILTERS];
extern SPMT *pmts[MAX_PMT];

uint8_t packet[188] = {
    0x47, 0x40, 0xff, 0x99, 0x14, 0x4c, 0x83, 0x7f, 0x46, 0xba, 0xb8, 0x12,
    0xfb, 0x83, 0xf7, 0x50, 0x9c, 0x73, 0x55, 0xe1, 0x8a, 0x1a, 0x54, 0x66,
    0x87, 0xb1, 0xd6, 0x04, 0x10, 0xc4, 0xa9, 0xb8, 0x53, 0x4e, 0x75, 0x11,
    0xcd, 0xaf, 0xd7, 0x05, 0x9c, 0xea, 0x08, 0x65, 0x3b, 0x36, 0x62, 0xac,
    0xb2, 0x2c, 0xd3, 0x42, 0xb8, 0xfd, 0x67, 0x4d, 0xbf, 0xa3, 0x04, 0x4d,
    0x0c, 0x0b, 0xb6, 0x70, 0x3f, 0xaf, 0xcc, 0x26, 0x8c, 0xf2, 0x92, 0x7d,
    0x64, 0x37, 0x18, 0x48, 0x0b, 0xd5, 0xd6, 0x50, 0x2c, 0x79, 0xc5, 0xd9,
    0x30, 0xb9, 0xb5, 0x9f, 0xca, 0x12, 0x0a, 0x10, 0xf2, 0x36, 0xa2, 0x23,
    0x3c, 0xc9, 0xb7, 0x70, 0x08, 0xfb, 0x94, 0x1d, 0x36, 0x79, 0x04, 0x5e,
    0xe6, 0x70, 0xfa, 0xaf, 0xe4, 0x12, 0x51, 0xad, 0x53, 0xb1, 0x48, 0xb7,
    0x25, 0x67, 0x3c, 0xf5, 0x6f, 0x47, 0xe2, 0x97, 0xe4, 0x93, 0xcb, 0x87,
    0x4f, 0x77, 0x49, 0x7a, 0x7b, 0x7e, 0x26, 0xe0, 0xc9, 0xb4, 0x6e, 0x6a,
    0x52, 0xb8, 0xab, 0x25, 0xbf, 0x33, 0xb9, 0x4b, 0x25, 0x39, 0x26, 0x24,
    0xaa, 0xa6, 0x19, 0xe1, 0x3f, 0xbd, 0x33, 0x7f, 0xd9, 0xa5, 0xb4, 0x25,
    0x44, 0xb1, 0x45, 0xee, 0xee, 0x25, 0x04, 0x47, 0xcd, 0x63, 0x81, 0x03,
    0x15, 0x59, 0x58, 0x1d, 0x00, 0x00, 0x00, 0x00};
uint8_t cw0[] = {0x64, 0xBB, 0x0E, 0x2D, 0x98, 0xAD, 0x8C, 0xD1};
uint8_t cw1[] = {0x77, 0xC1, 0x1F, 0x57, 0x96, 0xFB, 0xC3, 0x54};
uint8_t cw_invalid[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};

extern adapter *a[MAX_ADAPTERS];
extern SCW *cws[MAX_CW];

int test_decrypt() {
    int i, max_len = 1000;
    opts.adapter_buffer = 188 * 1000;
    a[0] = adapter_alloc();
    a[0]->id = 0;
    a[0]->pids[0].pid = 0xff;
    a[0]->pids[0].flags = 1;
    a[0]->pids[0].pmt = 0;
    a[0]->enabled = 1;
    pmt_add(0, 0, 100);
    for (i = 0; i < max_len; i++) {
        memcpy(a[0]->buf + i * sizeof(packet), packet, sizeof(packet));
    }
    a[0]->rlen = max_len * sizeof(packet);
    init_algo();
    uint8_t ecm = 0;
    send_cw(0, CA_ALGO_DVBCSA, 0, cw_invalid, NULL, 25, &ecm);
    send_cw(0, CA_ALGO_DVBCSA, 0, cw0, NULL, 25, &ecm);
    send_cw(0, CA_ALGO_DVBCSA, 1, cw1, NULL, 25, &ecm);
    send_cw(0, CA_ALGO_DVBCSA, 0, cw_invalid, NULL, 25, &ecm);

    SPMT_batch batch[1] = {{.data = packet, .len = sizeof(packet)}};
    ASSERT(0 != test_decrypt_packet(cws[0], batch, 1),
           "test_decrypt_packet expected to fail");
    ASSERT(0 == test_decrypt_packet(cws[1], batch, 1),
           "test_decrypt_packet expected to work");

    pmt_decrypt_stream(a[0]);
    uint8_t *b = a[0]->buf + (max_len - 1) * sizeof(packet);
    ASSERT(b[4] + b[5] + b[6] == 1, "MPEG header expected");
    hexdump("adapter buffer ", a[0]->buf, 188);
    free(a[0]->buf);
    free(a[0]);
    a[0] = NULL;
    return 0;
}

int test_wait_pusi() {
    int i, max_len = 3 * 188;
    opts.adapter_buffer = 188 * 1000;
    a[0] = adapter_alloc();
    a[0]->id = 0;
    a[0]->pids[0].pid = 0xff;
    a[0]->pids[0].flags = 1;
    a[0]->pids[0].pmt = 0;
    a[0]->enabled = 1;
    memset(a[0]->buf, 0, a[0]->lbuf);
    for (i = 0; i < max_len; i += 188) {
        uint8_t *b = a[0]->buf + i;
        b[0] = 0x47;
        b[1] = 0x00; // no packet start
        b[2] = 0xFF; // pid
        b[3] = 0xC0; // encrypted + parity 1
    }
    // second packet changes parity
    a[0]->buf[1 * 188 + 3] = 0x80;

    // keep the same parity
    a[0]->buf[2 * 188 + 3] = 0x80;
    a[0]->buf[2 * 188 + 1] |= 0x40;

    ASSERT(wait_pusi(a[0], 1 * 188) == 0, "wait_pusi failed");
    ASSERT(wait_pusi(a[0], 2 * 188) == 1, "getItem should not fail");
    ASSERT(wait_pusi(a[0], 3 * 188) == 0, "getItem should not fail");
    free(a[0]->buf);
    free(a[0]);
    a[0] = NULL;
    return 0;
}

int main() {
    opts.log = 255;
    opts.debug = 255;
    strcpy(thread_info[thread_index].thread_name, "test_pmt");
    TEST_FUNC(test_wait_pusi(), "testing decrypt");
    TEST_FUNC(test_decrypt(), "testing decrypt");
    fflush(stdout);
    return 0;
}
