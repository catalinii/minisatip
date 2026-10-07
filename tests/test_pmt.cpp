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
#include "ca.h"
#include "dvb.h"
#include "minisatip.h"
#include "socketworks.h"
#include "tables.h"
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

// Forward declarations
descriptor_t create_descriptor(const uint8_t *data);
void cache_pmt_for_adapter(adapter *ad, SPMT *pmt);
void pmt_add_active_pmt(adapter *ad, int pmt_id);
void start_active_pmts(adapter *ad);
SPMT *get_pmt_for_sid_pid(int aid, int sid, int pid);

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

int test_descriptor_equality() {
    const uint8_t descr1_data[] = {0x09, 0x04, 0x0B, 0x00, 0x05, 0x73};
    descriptor_t descr1 = create_descriptor(descr1_data);

    // other type
    const uint8_t descr2_data[] = {0x01, 0x04, 0x0B, 0x00, 0x05, 0x73};
    descriptor_t descr2 = create_descriptor(descr2_data);

    // other length
    const uint8_t descr3_data[] = {0x09, 0x02, 0x0B, 0x00};
    descriptor_t descr3 = create_descriptor(descr3_data);

    // other data
    const uint8_t descr4_data[] = {0x09, 0x04, 0x0B, 0x00, 0x05, 0xAB};
    descriptor_t descr4 = create_descriptor(descr4_data);

    // identical
    const uint8_t descr5_data[] = {0x09, 0x04, 0x0B, 0x00, 0x05, 0x73};
    descriptor_t descr5 = create_descriptor(descr5_data);

    ASSERT(descr1 != descr2, "descr1 and descr2 should not match");
    ASSERT(descr1 != descr3, "descr1 and descr3 should not match");
    ASSERT(descr1 != descr4, "descr1 and descr4 should not match");
    ASSERT(descr1 == descr1, "descr1 should match itself");
    ASSERT(descr1 == descr5, "descr1 and descr5 should match");

    return 0;
}

int test_descriptor_caid_capid_getters() {
    const uint8_t descr1_data[] = {0x09, 0x04, 0x0B, 0x00, 0x05, 0x73};
    descriptor_t descr1 = create_descriptor(descr1_data);

    ASSERT(descr1.get_ca_descriptor_caid() == 0x0B00, "CAID mismatch");
    ASSERT(descr1.get_ca_descriptor_capid() == 0x0573, "CA PID mismatch");

    return 0;
}

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
    // Claims reference listed streams; the decrypt path skips stale ones.
    pmt_add_stream_pid(pmts[0], 0xff, 2, false, true);
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
    delete a[0];
    a[0] = NULL;
    delete pmts[0];
    pmts[0] = NULL;
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
    delete a[0];
    a[0] = NULL;
    return 0;
}

int test_assemble_packet_adaptation() {
    unsigned char packet[] = {
        0x47, 0x41, 0x33, 0x3f, 0x68, 0x0,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0x0,  0x2,  0xb0, 0x4b, 0x4,  0xdd, 0xc7, 0x0,  0x0,  0xe3, 0xef,
        0xf0, 0x0,  0x1b, 0xe3, 0xef, 0xf0, 0x18, 0x28, 0x4,  0x64, 0x0,  0x28,
        0x3f, 0x9,  0x4,  0x9,  0x6a, 0xe5, 0x6b, 0x9,  0x4,  0x9,  0x58, 0xe5,
        0xcf, 0x9,  0x4,  0x6,  0xcd, 0xe6, 0x33, 0x6,  0xe4, 0x53, 0xf0, 0x1c,
        0xa,  0x4,  0x65, 0x6e, 0x67, 0x0,  0x6a, 0x2,  0x40, 0x8,  0x9,  0x4,
        0x9,  0x6a, 0xe5, 0x6b, 0x9,  0x4,  0x9,  0x58, 0xe5, 0xcf, 0x9,  0x4,
        0x6,  0xcd, 0xe6, 0x33, 0xc9, 0x52, 0xa8, 0xed};
    SFilter f;
    f.id = 0;
    f.flags = FILTER_CRC;
    int data = assemble_packet(&f, packet);
    ASSERT_EQUAL(78, data, "asemble_packet failed when using adaptation")
    ASSERT_EQUAL(0x02, f.data[0],
                 "asemble_packet failed when using adaptation on first byte")
    return 0;
}

int test_assemble_packet() {
    unsigned char packet[] = {
        0x47, 0x46, 0x31, 0x14, 0x0,  0x80, 0x70, 0x78, 0x41, 0x0,  0x2,  0x0,
        0x55, 0x4,  0x8,  0x40, 0x6f, 0x5a, 0x1d, 0xe8, 0x21, 0x5e, 0xda, 0x28,
        0xab, 0xbe, 0xe4, 0xe2, 0x6f, 0x8e, 0xbb, 0x2f, 0x2,  0xa0, 0x91, 0xe6,
        0x51, 0x81, 0xe,  0x93, 0xcf, 0xf7, 0x71, 0x56, 0x2d, 0x56, 0xf4, 0x94,
        0xbb, 0xd0, 0x9d, 0xb3, 0x3c, 0x6f, 0xc7, 0xc3, 0x19, 0xc8, 0x38, 0xed,
        0x1f, 0x3d, 0x26, 0x33, 0x65, 0xde, 0xb2, 0xc1, 0xf5, 0x5e, 0x1a, 0x2e,
        0x9e, 0xa3, 0x30, 0x3,  0x3f, 0x50, 0xa9, 0xf,  0x15, 0x2,  0x86, 0xb2,
        0x55, 0xf1, 0xbf, 0x6e, 0x6e, 0x5,  0x1,  0x9b, 0xd4, 0xc5, 0x55, 0xe3,
        0x96, 0xeb, 0x5d, 0xd2, 0xfc, 0x23, 0xfa, 0xb1, 0xa,  0x67, 0xfe, 0x6a,
        0xde, 0x56, 0x30, 0xee, 0x51, 0xc1, 0x96, 0x31, 0xe0, 0x8b, 0x25, 0x14,
        0x1,  0xcb, 0xcb, 0x86, 0xbd, 0x10, 0xf6, 0xf9, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    SFilter f;
    f.id = 0;
    f.flags = 0;
    int data = assemble_packet(&f, packet);
    ASSERT_EQUAL(123, data, "asemble_packet failed without adaptation")
    ASSERT_EQUAL(
        0x80, f.data[0],
        "asemble_packet failed without adaptation failed on first byte")
    return 0;
}

int test_assemble_multi_packet() {
    unsigned char p1[] = {
        0x47, 0x40, 0x11, 0x12, 0x0,  0x42, 0xf1, 0x5,  0x0,  0xe8, 0xc5, 0x0,
        0x0,  0x0,  0x1,  0xff, 0x0,  0xd2, 0xfd, 0x80, 0x12, 0x48, 0x10, 0x1,
        0x4,  0x44, 0x49, 0x47, 0x49, 0x9,  0x46, 0x69, 0x6c, 0x6d, 0x20, 0x43,
        0x61, 0x66, 0x65, 0x1,  0x57, 0xfd, 0x90, 0x14, 0x48, 0x12, 0x1,  0x4,
        0x44, 0x49, 0x47, 0x49, 0xb,  0x46, 0x49, 0x4c, 0x4d, 0x20, 0x4e, 0x4f,
        0x57, 0x20, 0x48, 0x44, 0x1,  0x72, 0xfd, 0x80, 0x12, 0x48, 0x10, 0x1,
        0x4,  0x44, 0x49, 0x47, 0x49, 0x9,  0x41, 0x58, 0x4e, 0x20, 0x57, 0x68,
        0x69, 0x74, 0x65, 0x1,  0x7c, 0xfd, 0x80, 0x14, 0x48, 0x12, 0x1,  0x4,
        0x44, 0x49, 0x47, 0x49, 0xb,  0x4e, 0x69, 0x63, 0x6b, 0x65, 0x6c, 0x6f,
        0x64, 0x65, 0x6f, 0x6e, 0x1,  0xa6, 0xfd, 0x80, 0x12, 0x48, 0x10, 0x1,
        0x4,  0x44, 0x49, 0x47, 0x49, 0x9,  0x4e, 0x69, 0x63, 0x6b, 0x74, 0x6f,
        0x6f, 0x6e, 0x73, 0x1,  0xcc, 0xff, 0x80, 0xe,  0x48, 0xc,  0x1,  0x4,
        0x44, 0x49, 0x47, 0x49, 0x5,  0x4d, 0x45, 0x5a, 0x5a, 0x4f, 0x2,  0x61,
        0xfd, 0x80, 0xc,  0x48, 0xa,  0x1,  0x4,  0x44, 0x49, 0x47, 0x49, 0x3,
        0x43, 0x4e, 0x4e, 0x2,  0x83, 0xfd, 0x90, 0x14, 0x48, 0x12, 0x1,  0x4,
        0x44, 0x49, 0x47, 0x49, 0xb,  0x53, 0x75, 0x70};
    unsigned char p2[] = {
        0x47, 0x0,  0x11, 0x11, 0x65, 0x72, 0x4f, 0x4e, 0x45, 0x20, 0x48, 0x44,
        0x2,  0x8c, 0xfd, 0x90, 0xf,  0x48, 0xd,  0x1,  0x4,  0x44, 0x49, 0x47,
        0x49, 0x6,  0x48, 0x42, 0x4f, 0x20, 0x48, 0x44, 0x1f, 0x18, 0xfd, 0x80,
        0x13, 0x48, 0x11, 0x6,  0x4,  0x44, 0x49, 0x47, 0x49, 0xa,  0x53, 0x57,
        0x20, 0x44, 0x4c, 0x20, 0x53, 0x4d, 0x49, 0x54, 0x1f, 0x4a, 0xfd, 0x80,
        0x14, 0x48, 0x12, 0x6,  0x4,  0x44, 0x49, 0x47, 0x49, 0xb,  0x53, 0x57,
        0x20, 0x53, 0x6d, 0x61, 0x72, 0x74, 0x44, 0x54, 0x56, 0x31, 0x62, 0xad,
        0xf5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    SFilter f;
    f.id = 0;
    f.enabled = 1;
    f.flags = FILTER_CRC;
    int data = assemble_packet(&f, p1);
    ASSERT_EQUAL(data, 0,
                 "asemble_packet expected length 0 for the first packet")
    data = assemble_packet(&f, p2);
    printf("Got %d from assemble_packet\n", data);
    ASSERT_EQUAL(264, data, "asemble_packet failed for multi-packet")
    ASSERT_EQUAL(
        0x42, f.data[0],
        "asemble_packet failed without adaptation failed on first byte")
    return 0;
}

int test_emulate_add_all_pids() {
    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    SPMT pmt;
    pmts[0] = &pmt;
    pmt.enabled = 1;
    pmt.adapter = 0;
    opts.emulate_pids_all = 1;
    SStreamPid sp{.type = 1, .pid = 100};
    pmt.stream_pids.push_back(sp);
    SStreamPid sp1{.type = 1, .pid = 101};
    pmt.stream_pids.push_back(sp1);
    ad.active_pmts = 1;
    ad.active_pmt[0] = 0;
    mark_pid_add(0, ad.id, 8192);
    mark_pid_add(1, ad.id, 8192);
    mark_pid_add(2, ad.id, 101);
    update_pids(ad.id);
    int pids[] = {100, 0, 1, 16};
    for (auto pid : pids) {
        SPid *p = find_pid(ad.id, pid);
        ASSERT_EQUAL(p->pid, pid, "emulate_add_all_pids failed");
        ASSERT_EQUAL(p->sid.count(0), 1,
                     "emulate_add_all_pids failed to set first stream");
        ASSERT_EQUAL(p->sid.count(1), 1,
                     "emulate_add_all_pids failed to set second stream");
    }
    SPid *p = find_pid(ad.id, 101); // pid 101 should have sid 0, 1. 2

    ASSERT(p->has_stream(2) && p->has_stream(0) && p->has_stream(1),
           "Expected 3 sids to be set fo pid 101");
    opts.emulate_pids_all = 0;
    return 0;
}

// First PAT after (re)init (pat_processed == 0, every CI channel change):
// a PMT missing from it must still retire its CA registration.

extern int npmts;
extern int process_pat(int filter, unsigned char *b, int len, void *opaque);

static int fake_ca_del_calls;
static int fake_ca_last_del_pmt;

static int fake_ca_add_pmt(adapter *ad, SPMT *pmt, int update) {
    (void)ad;
    (void)pmt;
    (void)update;
    return TABLES_RESULT_OK;
}

static int fake_ca_del_pmt(adapter *ad, SPMT *pmt) {
    fake_ca_del_calls++;
    fake_ca_last_del_pmt = pmt->id;
    return TABLES_RESULT_OK;
}

static int fake_ca_init_dev(adapter *ad) { return TABLES_RESULT_OK; }
static int fake_ca_close_dev(adapter *ad) { return TABLES_RESULT_OK; }
static int fake_ca_close_ca() { return 0; }

static SCA_op fake_ca_op = {.ca_add_pid = NULL,
                            .ca_del_pid = NULL,
                            .ca_add_pmt = fake_ca_add_pmt,
                            .ca_del_pmt = fake_ca_del_pmt,
                            .ca_init_dev = fake_ca_init_dev,
                            .ca_close_dev = fake_ca_close_dev,
                            .ca_ts = NULL,
                            .ca_close_ca = fake_ca_close_ca};

// Builds a minimal PAT section as process_pat() expects to receive it.
static int build_pat(uint8_t *b, int tsid, int version, const int *sids,
                     const int *pids, int n) {
    int i, len = 8 + 4 * n + 4;
    b[0] = 0x00; // table_id
    b[1] = 0xb0 | (((len - 3) >> 8) & 0x0f);
    b[2] = (len - 3) & 0xff;
    b[3] = (tsid >> 8) & 0xff;
    b[4] = tsid & 0xff;
    b[5] = 0xc1 | ((version & 0x1f) << 1);
    b[6] = 0; // section_number
    b[7] = 0; // last_section_number
    for (i = 0; i < n; i++) {
        b[8 + i * 4] = (sids[i] >> 8) & 0xff;
        b[9 + i * 4] = sids[i] & 0xff;
        b[10 + i * 4] = 0xe0 | ((pids[i] >> 8) & 0x1f);
        b[11 + i * 4] = pids[i] & 0xff;
    }
    memset(b + 8 + 4 * n, 0, 4); // CRC, not verified on this path
    return len;
}

// Builds a minimal PMT section: one program CA descriptor plus
// audio/video streams without ES descriptors.
static int build_pmt(uint8_t *b, int sid, int version, int pcr, uint16_t caid,
                     uint16_t ecm, const int *types, const int *pids, int n) {
    int pi_len = caid ? 6 : 0;
    int len = 12 + pi_len + 5 * n + 4;
    b[0] = 0x02; // table_id
    b[1] = 0xb0 | (((len - 3) >> 8) & 0x0f);
    b[2] = (len - 3) & 0xff;
    b[3] = (sid >> 8) & 0xff;
    b[4] = sid & 0xff;
    b[5] = 0xc1 | ((version & 0x1f) << 1);
    b[6] = 0; // section_number
    b[7] = 0; // last_section_number
    b[8] = 0xe0 | ((pcr >> 8) & 0x1f);
    b[9] = pcr & 0xff;
    b[10] = 0xf0 | ((pi_len >> 8) & 0x0f);
    b[11] = pi_len & 0xff;
    if (caid) {
        b[12] = 0x09;
        b[13] = 0x04;
        b[14] = (caid >> 8) & 0xff;
        b[15] = caid & 0xff;
        b[16] = 0xe0 | ((ecm >> 8) & 0x1f);
        b[17] = ecm & 0xff;
    }
    for (int i = 0; i < n; i++) {
        int o = 12 + pi_len + 5 * i;
        b[o] = types[i];
        b[o + 1] = 0xe0 | ((pids[i] >> 8) & 0x1f);
        b[o + 2] = pids[i] & 0xff;
        b[o + 3] = 0xf0;
        b[o + 4] = 0x00; // es_len
    }
    // Real trailing CRC: it covers the version byte, so it differs per
    // version exactly as on the wire.
    copy32(b, 12 + pi_len + 5 * n, crc_32(b, 12 + pi_len + 5 * n));
    return len;
}

#define PAT_TSID 40901
#define SID_KEPT 100
#define PID_KEPT 1000
#define SID_GONE 200
#define PID_GONE 2000

static int check_pat_drop_releases_ca(int adapter_type) {
    int i;
    uint8_t priv[1] = {0};
    uint8_t pat[64];
    int sids[] = {SID_KEPT};
    int pids[] = {PID_KEPT};

    // Earlier tests leave stack SPMTs in pmts[]: detach all, never free,
    // as some entries are not ours.
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    ad.type = adapter_type;
    // ad.pat_processed stays 0: this is the first PAT after the adapter was
    // initialised, which is the case the old code did not clean up.

    int ica = add_ca(&fake_ca_op);
    ASSERT(ica >= 0, "could not register the fake CA");
    ad.ca_mask = 1 << ica;

    ad.pat_filter = add_filter(ad.id, 0, (void *)process_pat, &ad,
                               FILTER_PERMANENT | FILTER_CRC);
    ASSERT(ad.pat_filter >= 0, "could not add the PAT filter");

    // Both services are known and registered with the CA.
    int kept = pmt_add(ad.id, SID_KEPT, PID_KEPT);
    int gone = pmt_add(ad.id, SID_GONE, PID_GONE);
    ASSERT(kept >= 0 && gone >= 0, "could not create the PMTs");

    pmt_add_caid(pmts[kept], 0x0664, 0x1F06, priv, 0);
    pmt_add_caid(pmts[gone], 0x0664, 0x1F08, priv, 0);
    send_pmt_to_cas(&ad, pmts[kept]);
    send_pmt_to_cas(&ad, pmts[gone]);
    ASSERT(pmts[kept]->ca_mask != 0, "kept PMT was not registered with the CA");
    ASSERT(pmts[gone]->ca_mask != 0, "gone PMT was not registered with the CA");

    pmts[kept]->state = PMT_RUNNING;
    pmts[gone]->state = PMT_RUNNING;
    ad.active_pmts = 2;
    ad.active_pmt[0] = kept;
    ad.active_pmt[1] = gone;

    // The kept service is watched: subscribe its pids so the update_pids
    // tail election leaves it running with its CA registration intact.
    pmt_add_stream_pid(pmts[kept], 1101, 2, false, true);
    pmt_add_stream_pid(pmts[kept], 1201, 3, true, false);
    ASSERT(mark_pid_add(0, 0, PID_KEPT) == 0, "kept PMT pid should be added");
    ASSERT(mark_pid_add(0, 0, 1101) == 0, "kept video pid should be added");
    ASSERT(mark_pid_add(0, 0, 1201) == 0, "kept audio pid should be added");

    // A PAT that no longer carries SID_GONE.
    fake_ca_del_calls = 0;
    fake_ca_last_del_pmt = -1;
    int len = build_pat(pat, PAT_TSID, 1, sids, pids, 1);
    process_pat(ad.pat_filter, pat, len, &ad);

    ASSERT_EQUAL(fake_ca_del_calls, 1,
                 "the PMT missing from the PAT was not closed on the CA");
    ASSERT_EQUAL(fake_ca_last_del_pmt, gone,
                 "the wrong PMT was closed on the CA");
    ASSERT_EQUAL(pmts[gone]->ca_mask, 0,
                 "the CA registration of the dropped PMT was not released");

    // The service still present in the PAT must be left alone.
    ASSERT(pmts[kept] != NULL && pmts[kept]->enabled,
           "the PMT still present in the PAT was retired");
    ASSERT(pmts[kept]->ca_mask != 0,
           "the PMT still present in the PAT lost its CA registration");

    // A CI adapter deletes the PMT outright, any other adapter caches it.
    if (adapter_type == ADAPTER_CI) {
        ASSERT_EQUAL(pmts[gone]->enabled, 0,
                     "the dropped PMT was not deleted on a CI adapter");
    } else {
        ASSERT_EQUAL(pmts[gone]->state, PMT_CACHED,
                     "the dropped PMT was not cached");
    }

    del_ca(&fake_ca_op);
    // Only the PMTs created above are left in pmts[] at this point.
    free_all_pmts();
    return 0;
}

int test_pat_drop_releases_ca() {
    if (check_pat_drop_releases_ca(ADAPTER_DVB))
        return 1;
    return check_pat_drop_releases_ca(ADAPTER_CI);
}

// A revived PMT must get its fresh filter handed over, or start_pmt fails
// before adding the pid and the PMT pid stays out of the demux.
int test_revived_cached_pmt_gets_its_filter() {
    int i;
    uint8_t pat[64];
    int sids[] = {SID_KEPT};
    int pids[] = {PID_KEPT};

    for (i = 0; i < MAX_ADAPTERS; i++)
        a[i] = NULL;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    ad.type = ADAPTER_DVB;
    ad.pat_filter = add_filter(ad.id, 0, (void *)process_pat, &ad,
                               FILTER_PERMANENT | FILTER_CRC);

    int id = pmt_add(ad.id, SID_KEPT, PID_KEPT);
    ASSERT(id >= 0, "could not create the PMT");
    SPMT *pmt = pmts[id];
    pmt->version = 7;
    pmt->state = PMT_RUNNING;
    SStreamPid sp{.type = 2, .pid = 300, .is_audio = false, .is_video = true};
    pmt->stream_pids.push_back(sp);

    // the client keeps streaming while the PMT is gone: the PMT pid
    // and the video pid stay subscribed (a PMT only starts with both)
    ad.pids[0].pid = 300;
    ad.pids[0].flags = PID_STATE_ACTIVE;
    ad.pids[0].pmt = -1;
    ad.pids[0].sid.insert(0);
    ad.pids[0].order = 2;
    ad.pids[1].pid = PID_KEPT;
    ad.pids[1].flags = PID_STATE_ACTIVE;
    ad.pids[1].pmt = -1;
    ad.pids[1].sid.insert(0);
    ad.pids[1].order = 1;

    ad.active_pmts = 1;
    ad.active_pmt[0] = id;

    cache_pmt_for_adapter(&ad, pmt);
    ASSERT_EQUAL(pmt->state, PMT_CACHED, "PMT should be cached");
    ASSERT_EQUAL(pmt->filter, -1, "the cached PMT should have no filter");

    // the sid is back in the PAT
    int len = build_pat(pat, PAT_TSID, 1, sids, pids, 1);
    process_pat(ad.pat_filter, pat, len, &ad);

    ASSERT_EQUAL(pmt->state, PMT_RUNNING,
                 "the revived PMT should start straight from the cache");
    ASSERT(pmt->filter >= 0,
           "process_pat() should hand the new filter to the revived PMT");
    ASSERT_EQUAL(filters[pmt->filter]->pid, PID_KEPT,
                 "the filter should be the one for the PMT pid");
    ASSERT(filters[pmt->filter]->opaque == pmt,
           "the filter should belong to this PMT");

    // a second PAT must not add another filter for the same PMT
    int before = pmt->filter;
    len = build_pat(pat, PAT_TSID, 2, sids, pids, 1);
    process_pat(ad.pat_filter, pat, len, &ad);
    ASSERT_EQUAL(pmt->filter, before,
                 "a second PAT should reuse the filter of the revived PMT");

    // the loop pass that used to start it is now a no-op backstop
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmt->state, PMT_RUNNING, "the revived PMT should stay up");
    ASSERT(filters[pmt->filter]->flags != 0,
           "starting the PMT should have enabled its filter");

    del_filter(ad.pat_filter);
    free_all_pmts();
    free_filters();
    a[0] = NULL;
    return 0;
}

// Two services sharing every ES pid (#1445): only the sibling whose
// PMT pid is subscribed starts, else nothing decrypts (FTA).
int test_pmt_starts_only_with_pmt_pid() {
    int i;
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;

    int aid = pmt_add(0, 14101, 48);
    int bid = pmt_add(0, 14104, 52);
    ASSERT(aid >= 0 && bid >= 0, "could not create the PMTs");
    for (int id : {aid, bid}) {
        pmt_add_stream_pid(pmts[id], 3301, 2, false, true);
        pmt_add_stream_pid(pmts[id], 3401, 3, true, false);
    }
    ad.active_pmts = 2;
    ad.active_pmt[0] = aid;
    ad.active_pmt[1] = bid;

    // subscribe the wanted PMT pid plus the shared elementary pids
    int pids[] = {52, 3301, 3401};
    for (i = 0; i < 3; i++) {
        ad.pids[i].pid = pids[i];
        ad.pids[i].flags = PID_STATE_ACTIVE;
        ad.pids[i].pmt = -1;
        ad.pids[i].filter = -1;
        ad.pids[i].sid.insert(0);
        ad.pids[i].order = i + 1;
    }

    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[bid]->state, PMT_RUNNING, "wanted PMT should run");
    ASSERT_EQUAL(pmts[aid]->state, PMT_STOPPED, "sibling PMT should wait");
    ASSERT(find_pid(0, 3301)->pmt == bid, "video pid should be claimed by B");
    ASSERT(find_pid(0, 3401)->pmt == bid, "audio pid should be claimed by B");

    // drop the PMT pid: AV stays with no rival, so the PMT stays
    find_pid(0, 52)->sid.clear();
    find_pid(0, 52)->order = 0;
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[bid]->state, PMT_RUNNING, "PMT should stay");
    ASSERT(find_pid(0, 3301)->pmt == bid, "video claim should be kept");
    ASSERT(find_pid(0, 3401)->pmt == bid, "audio claim should be kept");

    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// Retune A->B completes in one pass with no sections: A stops and
// releases, B claims and starts, and the CA handover is ordered.
static int fake_ca_add_calls;
static int fake_ca_last_add_pmt;
static int fake_ca_last_add_update;

static int counting_ca_add_pmt(adapter *ad, SPMT *pmt, int update) {
    fake_ca_add_calls++;
    fake_ca_last_add_pmt = pmt->id;
    fake_ca_last_add_update = update;
    return fake_ca_add_pmt(ad, pmt, update);
}

static int fail_next_ca_add;

static int failing_ca_add_pmt(adapter *ad, SPMT *pmt, int update) {
    if (fail_next_ca_add) {
        fail_next_ca_add = 0;
        return TABLES_RESULT_ERROR_NORETRY;
    }
    return counting_ca_add_pmt(ad, pmt, update);
}

int test_retune_handover_same_loop() {
    int i;
    uint8_t priv[1] = {0};
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;

    SCA_op counting_op = fake_ca_op;
    counting_op.ca_add_pmt = counting_ca_add_pmt;
    int ica = add_ca(&counting_op);
    ASSERT(ica >= 0, "could not register the fake CA");
    ad.ca_mask = 1 << ica;

    int aid = pmt_add(0, 14101, 48);
    int bid = pmt_add(0, 14104, 52);
    for (int id : {aid, bid}) {
        pmt_add_caid(pmts[id], 0x0664, 0x1F06, priv, 0);
        pmt_add_stream_pid(pmts[id], 3301, 2, false, true);
        pmt_add_stream_pid(pmts[id], 3401, 3, true, false);
    }
    ad.active_pmts = 2;
    ad.active_pmt[0] = aid;
    ad.active_pmt[1] = bid;

    int pids[] = {48, 3301, 3401};
    for (i = 0; i < 3; i++) {
        ad.pids[i].pid = pids[i];
        ad.pids[i].flags = PID_STATE_ACTIVE;
        ad.pids[i].pmt = -1;
        ad.pids[i].filter = -1;
        ad.pids[i].sid.insert(0);
        ad.pids[i].order = i + 1;
    }
    fake_ca_add_calls = fake_ca_del_calls = 0;
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[aid]->state, PMT_RUNNING, "A should run first");
    ASSERT(fake_ca_add_calls == 1 && fake_ca_last_add_pmt == aid,
           "A should be sent to the CA");

    // retune: A leaves the subscription list, B joins it
    find_pid(0, 48)->sid.clear();
    find_pid(0, 48)->order = 0;
    ad.pids[3].pid = 52;
    ad.pids[3].flags = PID_STATE_ACTIVE;
    ad.pids[3].pmt = -1;
    ad.pids[3].filter = -1;
    ad.pids[3].sid.insert(0);
    ad.pids[3].order = 4;
    fake_ca_add_calls = fake_ca_del_calls = 0;
    fake_ca_last_add_pmt = fake_ca_last_del_pmt = -1;
    start_active_pmts(&ad);

    ASSERT_EQUAL(pmts[aid]->state, PMT_STOPPED, "A should stop");
    ASSERT_EQUAL(pmts[aid]->ca_mask, 0, "A should release its CA slot");
    ASSERT_EQUAL(pmts[bid]->state, PMT_RUNNING, "B should run");
    ASSERT(pmts[bid]->ca_mask != 0, "B should hold a CA slot");
    ASSERT(fake_ca_del_calls == 1 && fake_ca_last_del_pmt == aid,
           "A should be closed on the CA");
    ASSERT(fake_ca_add_calls == 1 && fake_ca_last_add_pmt == bid,
           "B should be sent to the CA");
    ASSERT(find_pid(0, 3301)->pmt == bid, "video pid should move to B");

    del_ca(&counting_op);
    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// Single-slot CAM (D6/surfcu): one slot shared by two siblings, the add
// fails while the slot is held, so the retune must release A before B.
static int single_slot_holder = -1;
static int single_slot_seq;
static int single_slot_add_seq;
static int single_slot_del_seq;
static int single_slot_failed_adds;

static int single_slot_ca_add_pmt(adapter *ad, SPMT *pmt, int update) {
    (void)ad;
    (void)update;
    if (single_slot_holder != -1 && single_slot_holder != pmt->id) {
        single_slot_failed_adds++;
        return TABLES_RESULT_ERROR_RETRY;
    }
    single_slot_holder = pmt->id;
    single_slot_add_seq = ++single_slot_seq;
    return TABLES_RESULT_OK;
}

static int single_slot_ca_del_pmt(adapter *ad, SPMT *pmt) {
    (void)ad;
    if (single_slot_holder == pmt->id) {
        single_slot_holder = -1;
        single_slot_del_seq = ++single_slot_seq;
    }
    return TABLES_RESULT_OK;
}

int test_single_slot_retune_releases_first() {
    int i;
    uint8_t priv[1] = {0};
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;

    SCA_op single_slot_op = fake_ca_op;
    single_slot_op.ca_add_pmt = single_slot_ca_add_pmt;
    single_slot_op.ca_del_pmt = single_slot_ca_del_pmt;
    int ica = add_ca(&single_slot_op);
    ASSERT(ica >= 0, "could not register the fake CA");
    ad.ca_mask = 1 << ica;

    int aid = pmt_add(0, 14101, 48);
    int bid = pmt_add(0, 14104, 52);
    for (int id : {aid, bid}) {
        pmt_add_caid(pmts[id], 0x0664, 0x1F06, priv, 0);
        pmt_add_stream_pid(pmts[id], 3301, 2, false, true);
        pmt_add_stream_pid(pmts[id], 3401, 3, true, false);
    }
    ad.active_pmts = 2;
    ad.active_pmt[0] = aid;
    ad.active_pmt[1] = bid;

    int pids[] = {48, 3301, 3401};
    for (i = 0; i < 3; i++) {
        ad.pids[i].pid = pids[i];
        ad.pids[i].flags = PID_STATE_ACTIVE;
        ad.pids[i].pmt = -1;
        ad.pids[i].filter = -1;
        ad.pids[i].sid.insert(0);
        ad.pids[i].order = i + 1;
    }
    single_slot_holder = -1;
    single_slot_seq = 0;
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[aid]->state, PMT_RUNNING, "A should run first");
    ASSERT_EQUAL(single_slot_holder, aid, "A should hold the slot");

    // retune: A leaves, B joins, the shared streams stay subscribed
    find_pid(0, 48)->sid.clear();
    find_pid(0, 48)->order = 0;
    ad.pids[3].pid = 52;
    ad.pids[3].flags = PID_STATE_ACTIVE;
    ad.pids[3].pmt = -1;
    ad.pids[3].filter = -1;
    ad.pids[3].sid.insert(0);
    ad.pids[3].order = 4;
    single_slot_seq = 0;
    single_slot_add_seq = single_slot_del_seq = 0;
    single_slot_failed_adds = 0;
    start_active_pmts(&ad);

    ASSERT_EQUAL(pmts[aid]->state, PMT_STOPPED, "A should stop");
    ASSERT_EQUAL(pmts[bid]->state, PMT_RUNNING, "B should run");
    ASSERT(pmts[bid]->ca_mask != 0, "B should hold a CA slot");
    ASSERT_EQUAL(single_slot_holder, bid, "B should hold the slot");
    ASSERT_EQUAL(single_slot_failed_adds, 0, "B must not see a full slot");
    ASSERT(single_slot_del_seq > 0 && single_slot_add_seq > 0 &&
               single_slot_del_seq < single_slot_add_seq,
           "the release must come before the acquire");

    del_ca(&single_slot_op);
    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// Tvheadend mux scan (surfcu): PMT pids subscribed without any ES pid
// must not start anything; adding the streams elects the earlier one.
int test_scan_pmt_only_starts_nothing() {
    int i;
    uint8_t priv[1] = {0};
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;

    SCA_op counting_op = fake_ca_op;
    counting_op.ca_add_pmt = counting_ca_add_pmt;
    int ica = add_ca(&counting_op);
    ASSERT(ica >= 0, "could not register the fake CA");
    ad.ca_mask = 1 << ica;

    int aid = pmt_add(0, 14101, 48);
    int bid = pmt_add(0, 14104, 52);
    ASSERT(aid >= 0 && bid >= 0, "could not create the PMTs");
    for (int id : {aid, bid}) {
        pmt_add_caid(pmts[id], 0x0664, 0x1F06, priv, 0);
        pmt_add_stream_pid(pmts[id], 3301, 2, false, true);
        pmt_add_stream_pid(pmts[id], 3401, 3, true, false);
    }
    ad.active_pmts = 2;
    ad.active_pmt[0] = aid;
    ad.active_pmt[1] = bid;

    int pids[] = {48, 52};
    for (i = 0; i < 2; i++) {
        ad.pids[i].pid = pids[i];
        ad.pids[i].flags = PID_STATE_ACTIVE;
        ad.pids[i].pmt = -1;
        ad.pids[i].filter = -1;
        ad.pids[i].sid.insert(0);
        ad.pids[i].order = i + 1;
    }
    fake_ca_add_calls = 0;
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[aid]->state, PMT_STOPPED, "A must not run on scan");
    ASSERT_EQUAL(pmts[bid]->state, PMT_STOPPED, "B must not run on scan");
    ASSERT_EQUAL(fake_ca_add_calls, 0, "nothing must be sent to the CA");

    // the client tunes for real: both PMT pids plus the shared streams
    int es[] = {3301, 3401};
    for (i = 0; i < 2; i++) {
        ad.pids[2 + i].pid = es[i];
        ad.pids[2 + i].flags = PID_STATE_ACTIVE;
        ad.pids[2 + i].pmt = -1;
        ad.pids[2 + i].filter = -1;
        ad.pids[2 + i].sid.insert(0);
        ad.pids[2 + i].order = 3 + i;
    }
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[aid]->state, PMT_RUNNING, "earlier PMT should run");
    ASSERT_EQUAL(pmts[bid]->state, PMT_STOPPED, "later PMT should wait");

    del_ca(&counting_op);
    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// Claims are sticky in the loop: a later-subscribed newcomer waits on
// pids held by the earlier holder. Preemption happens at parse instead.
// The active list is capped: a full active_pmts must not overflow when a new
// PMT starts.
int test_active_pmt_list_capped_at_max() {
    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    opts.log = 1;

    int ids[MAX_PMT_FOR_ADAPTER + 1];
    for (int i = 0; i <= MAX_PMT_FOR_ADAPTER; i++)
        ids[i] = pmt_add(0, 1000 + i, 4000 + i);
    for (int i = 0; i < MAX_PMT_FOR_ADAPTER; i++)
        pmt_add_active_pmt(&ad, ids[i]);
    ASSERT(ad.active_pmts == MAX_PMT_FOR_ADAPTER, "active list must fill up");

    pmt_add_active_pmt(&ad, ids[MAX_PMT_FOR_ADAPTER]);
    ASSERT(ad.active_pmts == MAX_PMT_FOR_ADAPTER,
           "full active list must refuse");

    a[0] = NULL;
    free_all_pmts();
    return 0;
}

// 19.2E 11493H (live S1+S2): A/B share AV 5121/5122 and differ only in a
// data pid. Replace-zaps follow the PMT pid; add/del hands claims over.
int test_19e_11493h_zap_flows() {
    int i;
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    opts.emulate_pids_all = 0;

    int aid = pmt_add(0, 10303, 5120);
    int bid = pmt_add(0, 10304, 5130);
    ASSERT(aid >= 0 && bid >= 0, "could not create the PMTs");
    for (int id : {aid, bid}) {
        pmt_add_stream_pid(pmts[id], 5121, 27, false, true);
        pmt_add_stream_pid(pmts[id], 5122, 3, true, false);
    }
    pmt_add_stream_pid(pmts[aid], 5124, 6, false, false);
    pmt_add_stream_pid(pmts[bid], 5134, 6, false, false);
    ad.active_pmts = 2;
    ad.active_pmt[0] = aid;
    ad.active_pmt[1] = bid;

    // S1: replace-zap follows the PMT pid both ways
    ASSERT(mark_pid_add(0, 0, 5120) == 0, "pid 5120 should be added");
    ASSERT(mark_pid_add(0, 0, 5121) == 0, "pid 5121 should be added");
    ASSERT(mark_pid_add(0, 0, 5122) == 0, "pid 5122 should be added");
    update_pids(0);
    ASSERT_EQUAL(pmts[aid]->state, PMT_RUNNING, "A should run first");
    ASSERT(find_pid(0, 5121)->pmt == aid, "A should own video");
    mark_pid_deleted(0, 0, 5120, NULL);
    ASSERT(mark_pid_add(0, 0, 5130) == 0, "pid 5130 should be added");
    update_pids(0);
    ASSERT_EQUAL(pmts[bid]->state, PMT_RUNNING, "zap should run B");
    ASSERT_EQUAL(pmts[aid]->state, PMT_STOPPED, "zap should stop A");
    ASSERT(find_pid(0, 5121)->pmt == bid, "video should move to B");
    mark_pid_deleted(0, 0, 5130, NULL);
    ASSERT(mark_pid_add(0, 0, 5120) == 0, "pid 5120 should be re-added");
    update_pids(0);
    ASSERT_EQUAL(pmts[aid]->state, PMT_RUNNING, "zap back should run A");
    ASSERT(find_pid(0, 5121)->pmt == aid, "video should move back to A");

    // S2: AV-only runs nothing, PMT-after-AV elects, del hands over
    mark_pid_deleted(0, 0, 5120, NULL);
    mark_pid_deleted(0, 0, 5121, NULL);
    mark_pid_deleted(0, 0, 5122, NULL);
    ASSERT(mark_pid_add(0, 0, 5124) == 0, "pid 5124 should be added");
    update_pids(0);
    ASSERT_EQUAL(pmts[aid]->state, PMT_STOPPED, "data-only runs nothing");
    ASSERT(mark_pid_add(0, 0, 5121) == 0, "pid 5121 should be added");
    ASSERT(mark_pid_add(0, 0, 5122) == 0, "pid 5122 should be added");
    update_pids(0);
    ASSERT_EQUAL(pmts[aid]->state, PMT_STOPPED, "AV-only runs nothing");
    ASSERT_EQUAL(pmts[bid]->state, PMT_STOPPED, "AV-only runs nothing");
    ASSERT(mark_pid_add(0, 0, 5120) == 0, "pid 5120 should be added");
    update_pids(0);
    ASSERT_EQUAL(pmts[aid]->state, PMT_RUNNING, "PMT-after-AV runs A");
    ASSERT(find_pid(0, 5124)->pmt == aid, "data is marked with its owner");
    ASSERT(mark_pid_add(0, 0, 5130) == 0, "pid 5130 should be added");
    update_pids(0);
    ASSERT_EQUAL(pmts[aid]->state, PMT_RUNNING, "first keeps running");
    ASSERT_EQUAL(pmts[bid]->state, PMT_STOPPED, "second waits");
    ASSERT(pmts[bid]->best, "waiter stays in the set");
    ASSERT(find_pid(0, 5121)->pmt == aid, "video stays with A");
    mark_pid_deleted(0, 0, 5120, NULL);
    update_pids(0);
    ASSERT_EQUAL(pmts[bid]->state, PMT_RUNNING, "del hands over to B");
    ASSERT_EQUAL(pmts[aid]->state, PMT_STOPPED, "deleted PMT stops");
    ASSERT(find_pid(0, 5121)->pmt == bid, "video moves to B");
    ASSERT(find_pid(0, 5124)->pmt == -1, "released data has no owner");

    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// 19.2E 11582H (live S4+S5): C/D are byte-identical on distinct PMT pids,
// E/F/G/H share AV and differ in one data pid each, I is disjoint.
int test_19e_11582h_group_and_disjoint() {
    int i;
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    opts.emulate_pids_all = 0;

    int cid = pmt_add(0, 10325, 5200);
    int did = pmt_add(0, 10326, 5210);
    ASSERT(cid >= 0 && did >= 0, "could not create the PMTs");
    for (int id : {cid, did}) {
        pmt_add_stream_pid(pmts[id], 5201, 27, false, true);
        pmt_add_stream_pid(pmts[id], 5202, 3, true, false);
        pmt_add_stream_pid(pmts[id], 5204, 6, false, false);
    }
    int efgh[4];
    int sids[4] = {10327, 10328, 10329, 10330};
    int ppids[4] = {5220, 5230, 5240, 5250};
    int dpids[4] = {5225, 5235, 5245, 5255};
    for (i = 0; i < 4; i++) {
        efgh[i] = pmt_add(0, sids[i], ppids[i]);
        ASSERT(efgh[i] >= 0, "could not create the group PMT");
        pmt_add_stream_pid(pmts[efgh[i]], 5221, 27, false, true);
        pmt_add_stream_pid(pmts[efgh[i]], 5222, 3, true, false);
        pmt_add_stream_pid(pmts[efgh[i]], dpids[i], 6, false, false);
    }
    int iid = pmt_add(0, 10331, 5260);
    ASSERT(iid >= 0, "could not create the disjoint PMT");
    pmt_add_stream_pid(pmts[iid], 5261, 27, false, true);
    pmt_add_stream_pid(pmts[iid], 5262, 3, true, false);
    ad.active_pmts = 7;
    ad.active_pmt[0] = cid;
    ad.active_pmt[1] = did;
    for (i = 0; i < 4; i++)
        ad.active_pmt[2 + i] = efgh[i];
    ad.active_pmt[6] = iid;

    // S4: identical pair dedupes on distinct PMT pids, del hands over
    ASSERT(mark_pid_add(0, 0, 5201) == 0, "pid 5201 should be added");
    ASSERT(mark_pid_add(0, 0, 5202) == 0, "pid 5202 should be added");
    ASSERT(mark_pid_add(0, 0, 5200) == 0, "pid 5200 should be added");
    update_pids(0);
    ASSERT_EQUAL(pmts[cid]->state, PMT_RUNNING, "C should run first");
    ASSERT(mark_pid_add(0, 0, 5210) == 0, "pid 5210 should be added");
    update_pids(0);
    ASSERT_EQUAL(pmts[cid]->state, PMT_RUNNING, "C keeps running");
    ASSERT_EQUAL(pmts[did]->state, PMT_STOPPED, "dup never starts");
    ASSERT(!pmts[did]->best, "dup drops out of the set");
    mark_pid_deleted(0, 0, 5200, NULL);
    update_pids(0);
    ASSERT_EQUAL(pmts[did]->state, PMT_RUNNING, "del hands over to D");
    ASSERT(find_pid(0, 5201)->pmt == did, "video moves to D");
    mark_pid_deleted(0, 0, 5201, NULL);
    mark_pid_deleted(0, 0, 5202, NULL);
    mark_pid_deleted(0, 0, 5210, NULL);

    // S5: group winner and disjoint coexist, AV-drop stops only the group
    ASSERT(mark_pid_add(0, 0, 5221) == 0, "pid 5221 should be added");
    ASSERT(mark_pid_add(0, 0, 5222) == 0, "pid 5222 should be added");
    for (i = 0; i < 4; i++) {
        ASSERT(mark_pid_add(0, 0, ppids[i]) == 0, "group pid added");
        update_pids(0);
    }
    ASSERT_EQUAL(pmts[efgh[0]]->state, PMT_RUNNING, "E should run");
    for (i = 1; i < 4; i++) {
        ASSERT_EQUAL(pmts[efgh[i]]->state, PMT_STOPPED, "group waits");
        ASSERT(pmts[efgh[i]]->best, "waiter stays in the set");
    }
    ASSERT(mark_pid_add(0, 0, 5260) == 0, "pid 5260 should be added");
    ASSERT(mark_pid_add(0, 0, 5261) == 0, "pid 5261 should be added");
    ASSERT(mark_pid_add(0, 0, 5262) == 0, "pid 5262 should be added");
    update_pids(0);
    ASSERT_EQUAL(pmts[iid]->state, PMT_RUNNING, "disjoint runs too");
    ASSERT_EQUAL(pmts[efgh[0]]->state, PMT_RUNNING, "E keeps running");
    ASSERT(find_pid(0, 5261)->pmt == iid, "I owns its video");
    mark_pid_deleted(0, 0, 5221, NULL);
    mark_pid_deleted(0, 0, 5222, NULL);
    update_pids(0);
    ASSERT_EQUAL(pmts[efgh[0]]->state, PMT_STOPPED, "AV-drop stops E");
    ASSERT_EQUAL(pmts[iid]->state, PMT_RUNNING, "disjoint survives");
    ASSERT(find_pid(0, 5261)->pmt == iid, "I keeps its video");

    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// 19.2E 11914H Sky DE (live S7): an encrypted zap sends the new PMT to the
// CA, closes the old one, and teardown closes the last one.
int test_19e_11914h_ca_send_close() {
    int i;
    uint8_t priv[1] = {0};
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    opts.emulate_pids_all = 0;

    SCA_op counting_op = fake_ca_op;
    counting_op.ca_add_pmt = counting_ca_add_pmt;
    int ica = add_ca(&counting_op);
    ASSERT(ica >= 0, "could not register the fake CA");
    ad.ca_mask = 1 << ica;
    fake_ca_add_calls = 0;
    fake_ca_last_add_pmt = -1;
    fake_ca_del_calls = 0;
    fake_ca_last_del_pmt = -1;

    int jid = pmt_add(0, 13, 101);
    int kid = pmt_add(0, 17, 102);
    ASSERT(jid >= 0 && kid >= 0, "could not create the PMTs");
    pmt_add_caid(pmts[jid], 0x098C, 6786, priv, 0);
    pmt_add_caid(pmts[jid], 0x09F0, 8066, priv, 0);
    pmt_add_stream_pid(pmts[jid], 1535, 27, false, true);
    pmt_add_caid(pmts[kid], 0x098C, 6843, priv, 0);
    pmt_add_caid(pmts[kid], 0x09F0, 8123, priv, 0);
    pmt_add_stream_pid(pmts[kid], 1791, 27, false, true);
    ad.active_pmts = 2;
    ad.active_pmt[0] = jid;
    ad.active_pmt[1] = kid;

    ASSERT(mark_pid_add(0, 0, 101) == 0, "pid 101 should be added");
    ASSERT(mark_pid_add(0, 0, 1535) == 0, "pid 1535 should be added");
    update_pids(0);
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[jid]->state, PMT_RUNNING, "J should run");
    ASSERT_EQUAL(fake_ca_add_calls, 1, "J should be sent once");
    ASSERT_EQUAL(fake_ca_last_add_pmt, jid, "J should be the sent PMT");

    mark_pid_deleted(0, 0, 101, NULL);
    mark_pid_deleted(0, 0, 1535, NULL);
    ASSERT(mark_pid_add(0, 0, 102) == 0, "pid 102 should be added");
    ASSERT(mark_pid_add(0, 0, 1791) == 0, "pid 1791 should be added");
    update_pids(0);
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[kid]->state, PMT_RUNNING, "zap should run K");
    ASSERT_EQUAL(pmts[jid]->state, PMT_STOPPED, "zap should stop J");
    ASSERT_EQUAL(fake_ca_last_add_pmt, kid, "K should be sent on zap");
    ASSERT_EQUAL(fake_ca_del_calls, 1, "J should be closed on zap");
    ASSERT_EQUAL(fake_ca_last_del_pmt, jid, "closed PMT should be J");

    mark_pid_deleted(0, 0, 102, NULL);
    mark_pid_deleted(0, 0, 1791, NULL);
    update_pids(0);
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[kid]->state, PMT_STOPPED, "teardown stops K");
    ASSERT_EQUAL(fake_ca_del_calls, 2, "teardown closes K");
    ASSERT_EQUAL(fake_ca_last_del_pmt, kid, "closed PMT should be K");

    del_ca(&counting_op);
    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// DDCI marks the CI adapter's pids with DDCI_SID and no client stream exists
// there, yet its PMT must start so the CA_PMT reaches the CAM.
int test_ci_adapter_pmt_starts() {
    int i;
    free_all_pmts();

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    ad.type = ADAPTER_CI;

    int id = pmt_add(0, 16542, 67);
    ASSERT(id >= 0, "could not create the PMT");
    pmt_add_stream_pid(pmts[id], 4942, 2, false, true);
    pmt_add_stream_pid(pmts[id], 5042, 3, true, false);
    ad.active_pmts = 1;
    ad.active_pmt[0] = id;

    int pids[] = {67, 4942, 5042};
    for (i = 0; i < 3; i++) {
        ad.pids[i].pid = pids[i];
        ad.pids[i].flags = PID_STATE_ACTIVE;
        ad.pids[i].pmt = -1;
        ad.pids[i].filter = -1;
        ad.pids[i].sid.insert(DDCI_SID);
    }

    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING,
                 "the CI adapter's PMT should start");

    free_all_pmts();
    a[0] = NULL;
    return 0;
}

int test_sticky_claims_without_parse() {
    int i;
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;

    int aid = pmt_add(0, 14101, 48);
    int bid = pmt_add(0, 14104, 52);
    for (int id : {aid, bid}) {
        pmt_add_stream_pid(pmts[id], 3301, 2, false, true);
        pmt_add_stream_pid(pmts[id], 3401, 3, true, false);
    }
    // B subscribed earlier and runs; A is the later newcomer
    int pids[] = {48, 52, 3301, 3401};
    int orders[] = {5, 3, 6, 7};
    for (i = 0; i < 4; i++) {
        ad.pids[i].pid = pids[i];
        ad.pids[i].flags = PID_STATE_ACTIVE;
        ad.pids[i].pmt = -1;
        ad.pids[i].filter = -1;
        ad.pids[i].sid.insert(i < 2 ? 1 - i : 0);
        if (i >= 2)
            ad.pids[i].sid.insert(1);
        ad.pids[i].order = orders[i];
    }
    ad.active_pmts = 1;
    ad.active_pmt[0] = bid;
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[bid]->state, PMT_RUNNING, "B should run alone");

    // A arrives parsed and subscribed, but the loop must not steal for it
    ad.active_pmts = 2;
    ad.active_pmt[1] = aid;
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[aid]->state, PMT_STOPPED, "newcomer should wait");
    ASSERT_EQUAL(pmts[bid]->state, PMT_RUNNING, "holder should keep running");
    ASSERT(find_pid(0, 3301)->pmt == bid, "video pid should stay with B");

    // steady state: no flapping between the two passes
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[aid]->state, PMT_STOPPED, "newcomer should keep waiting");
    ASSERT_EQUAL(pmts[bid]->state, PMT_RUNNING, "holder should keep running");

    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// Mirror image: an earlier-subscribed newcomer steals by order.
int test_earlier_newcomer_steals_by_order() {
    int i;
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;

    int aid = pmt_add(0, 14101, 48);
    int bid = pmt_add(0, 14104, 52);
    pmt_add_stream_pid(pmts[aid], 3301, 2, false, true);
    pmt_add_stream_pid(pmts[bid], 3301, 2, false, true);
    pmt_add_stream_pid(pmts[bid], 3401, 3, true, false);
    // B subscribed earlier but joins late; A runs holding video only.
    // Sets differ so no dedupe: the takeover must steal mid-loop.
    int pids[] = {48, 52, 3301, 3401};
    int orders[] = {5, 3, 6, 7};
    for (i = 0; i < 4; i++) {
        ad.pids[i].pid = pids[i];
        ad.pids[i].flags = PID_STATE_ACTIVE;
        ad.pids[i].pmt = -1;
        ad.pids[i].filter = -1;
        ad.pids[i].sid.insert(i < 2 ? 1 - i : 0);
        if (i >= 2)
            ad.pids[i].sid.insert(1);
        ad.pids[i].order = orders[i];
    }
    ad.active_pmts = 1;
    ad.active_pmt[0] = aid;
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[aid]->state, PMT_RUNNING, "A should run alone");
    ASSERT(find_pid(0, 3301)->pmt == aid, "A should own the video pid");

    // B arrives fully subscribed with the earlier order and takes over
    ad.active_pmts = 2;
    ad.active_pmt[1] = bid;
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[bid]->state, PMT_RUNNING, "earlier should take over");
    ASSERT_EQUAL(pmts[aid]->state, PMT_STOPPED, "holder should step aside");
    ASSERT(find_pid(0, 3301)->pmt == bid, "video pid should move to B");

    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// Partial overlap: the thief takes the shared pid, the holder keeps its
// own audio and both stay running.
int test_steal_splits_partial_overlap() {
    int i;
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;

    int aid = pmt_add(0, 14101, 48);
    int bid = pmt_add(0, 14104, 52);
    pmt_add_stream_pid(pmts[aid], 3301, 2, false, true);
    pmt_add_stream_pid(pmts[aid], 3401, 3, true, false);
    pmt_add_stream_pid(pmts[bid], 3301, 2, false, true);
    pmt_add_stream_pid(pmts[bid], 3402, 3, true, false);
    int pids[] = {48, 52, 3301, 3401, 3402};
    int orders[] = {5, 3, 6, 7, 8};
    int sids[] = {1, 0, 0, 1, 0};
    for (i = 0; i < 5; i++) {
        ad.pids[i].pid = pids[i];
        ad.pids[i].flags = PID_STATE_ACTIVE;
        ad.pids[i].pmt = -1;
        ad.pids[i].filter = -1;
        ad.pids[i].sid.insert(sids[i]);
        if (pids[i] == 3301)
            ad.pids[i].sid.insert(1);
        ad.pids[i].order = orders[i];
    }
    ad.active_pmts = 1;
    ad.active_pmt[0] = aid;
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[aid]->state, PMT_RUNNING, "A should run alone");

    ad.active_pmts = 2;
    ad.active_pmt[1] = bid;
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[bid]->state, PMT_RUNNING, "earlier should run");
    ASSERT_EQUAL(pmts[aid]->state, PMT_RUNNING, "holder keeps own audio");
    ASSERT(find_pid(0, 3301)->pmt == bid, "shared video moves to B");
    ASSERT(find_pid(0, 3401)->pmt == aid, "own audio stays with A");
    ASSERT(find_pid(0, 3402)->pmt == bid, "own audio goes to B");

    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// SPid.order is assigned on subscription and reset on removal.
int test_pid_order_reset_on_remove() {
    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    opts.emulate_pids_all = 0;

    ASSERT(mark_pid_add(0, 0, 100) == 0, "pid 100 should be added");
    ASSERT(mark_pid_add(0, 0, 101) == 0, "pid 101 should be added");
    update_pids(0);
    SPid *p100 = find_pid(0, 100);
    SPid *p101 = find_pid(0, 101);
    ASSERT(p100 && p101, "both pids should be present");
    ASSERT(p100->order > 0 && p101->order > p100->order,
           "later add should have larger order");
    uint32_t first_order = p100->order;

    mark_pid_deleted(0, 0, 100, NULL);
    update_pids(0);
    ASSERT(find_pid(0, 100) == NULL, "pid 100 should be gone");
    ASSERT(mark_pid_add(0, 0, 100) == 0, "pid 100 should be re-added");
    update_pids(0);
    p100 = find_pid(0, 100);
    ASSERT(p100 && p100->order > 0 && p100->order != first_order,
           "re-added pid should get a fresh order");

    // scanner marks carry no client sid and no order until subscribed
    ASSERT(mark_pid_add(PID_STREAM_ID_UNDEFINED, 0, 200) == 0,
           "scanner pid should be added");
    SPid *p200 = find_pid(0, 200);
    ASSERT(p200 && p200->order == 0, "scanner pid should have no order");
    ASSERT(mark_pid_add(0, 0, 200) == 0, "pid 200 should be subscribed");
    ASSERT(p200->order > 0, "subscribed pid should gain an order");

    mark_pids_deleted(0, PID_STREAM_ID_UNDEFINED, NULL);
    update_pids(0);
    a[0] = NULL;
    return 0;
}

// D8: several SIDs on one PMT pid. Identical stream sets run the lowest
// sid; different sets coexist and split the streams by subscription order.
int test_d8_shared_pid() {
    int i;
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;

    int m1 = pmt_add(0, 100, 60);
    int m2 = pmt_add(0, 200, 60);
    pmt_add_stream_pid(pmts[m1], 701, 2, false, true);
    pmt_add_stream_pid(pmts[m1], 711, 3, true, false);
    pmt_add_stream_pid(pmts[m2], 701, 2, false, true);
    pmt_add_stream_pid(pmts[m2], 712, 3, true, false);
    int m3 = pmt_add(0, 300, 61);
    int m4 = pmt_add(0, 400, 61);
    for (int id : {m3, m4}) {
        pmt_add_stream_pid(pmts[id], 801, 2, false, true);
        pmt_add_stream_pid(pmts[id], 811, 3, true, false);
    }
    ad.active_pmts = 4;
    ad.active_pmt[0] = m1;
    ad.active_pmt[1] = m2;
    ad.active_pmt[2] = m3;
    ad.active_pmt[3] = m4;

    int pids[] = {60, 701, 712, 61, 801, 811};
    for (i = 0; i < 6; i++) {
        ad.pids[i].pid = pids[i];
        ad.pids[i].flags = PID_STATE_ACTIVE;
        ad.pids[i].pmt = -1;
        ad.pids[i].filter = -1;
        ad.pids[i].sid.insert(0);
        ad.pids[i].order = i + 1;
    }
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[m1]->state, PMT_RUNNING, "first set should run");
    ASSERT_EQUAL(pmts[m2]->state, PMT_RUNNING, "second set should run");
    ASSERT_EQUAL(pmts[m3]->state, PMT_RUNNING, "lowest sid should run");
    ASSERT_EQUAL(pmts[m4]->state, PMT_STOPPED, "duplicate should wait");
    ASSERT(find_pid(0, 701)->pmt == m1, "shared video goes to the first");
    ASSERT(find_pid(0, 712)->pmt == m2, "own audio goes to the second");
    ASSERT(pmts[m1]->best && pmts[m2]->best, "different sets stay in");
    ASSERT(pmts[m3]->best && !pmts[m4]->best, "only the lowest sid is best");

    // drop 712: m2 has nothing left to serve and waits, m1 keeps all
    find_pid(0, 712)->sid.clear();
    find_pid(0, 712)->order = 0;
    ad.pids[6].pid = 711;
    ad.pids[6].flags = PID_STATE_ACTIVE;
    ad.pids[6].pmt = -1;
    ad.pids[6].filter = -1;
    ad.pids[6].sid.insert(0);
    ad.pids[6].order = 7;
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[m1]->state, PMT_RUNNING, "m1 should keep running");
    ASSERT_EQUAL(pmts[m2]->state, PMT_STOPPED, "m2 should wait claimless");
    ASSERT(pmts[m1]->best && pmts[m2]->best, "waiter stays in the set");
    ASSERT(find_pid(0, 701)->pmt == m1, "shared video should not move");

    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// Same PMT pid, different stream sets: both stay in the set and sticky
// claims split the streams, each serving the pids it carries.
int test_shared_pid_split() {
    int i;
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;

    int m1 = pmt_add(0, 100, 60);
    int m2 = pmt_add(0, 200, 60);
    pmt_add_stream_pid(pmts[m1], 701, 2, false, true);
    pmt_add_stream_pid(pmts[m1], 711, 3, true, false);
    pmt_add_stream_pid(pmts[m2], 701, 2, false, true);
    pmt_add_stream_pid(pmts[m2], 712, 3, true, false);
    ad.active_pmts = 2;
    ad.active_pmt[0] = m1;
    ad.active_pmt[1] = m2;

    int pids[] = {60, 701, 711, 712};
    for (i = 0; i < 4; i++) {
        ad.pids[i].pid = pids[i];
        ad.pids[i].flags = PID_STATE_ACTIVE;
        ad.pids[i].pmt = -1;
        ad.pids[i].filter = -1;
        ad.pids[i].sid.insert(0);
        ad.pids[i].order = i + 1;
    }
    start_active_pmts(&ad);
    ASSERT(pmts[m1]->best && pmts[m2]->best, "both should stay in the set");
    ASSERT_EQUAL(pmts[m1]->state, PMT_RUNNING, "first contender should run");
    ASSERT_EQUAL(pmts[m2]->state, PMT_RUNNING, "second contender should run");
    ASSERT(find_pid(0, 701)->pmt == m1, "shared video goes to the first");
    ASSERT(find_pid(0, 712)->pmt == m2, "own audio goes to the second");

    // sticky claims: a second pass changes nothing
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[m1]->state, PMT_RUNNING, "first should stay up");
    ASSERT_EQUAL(pmts[m2]->state, PMT_RUNNING, "second should stay up");
    ASSERT(find_pid(0, 701)->pmt == m1, "shared video should not move");

    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// 30W 12437H: pid 817 carries dozens of PMTs (Meo/Nos). SIDs 1338/6043
// share all A/V pids with different ECMs; 807 is disjoint. Lowest sid serves.
int test_30w_shared_pmt_pid() {
    int i;
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;
    for (i = 0; i < MAX_FILTERS; i++)
        filters[i] = NULL;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    opts.emulate_pids_all = 0;
    ad.pids[0].pid = 817;
    ad.pids[0].flags = PID_STATE_ACTIVE;
    ad.pids[0].pmt = -1;
    ad.pids[0].filter = -1;
    ad.pids[0].sid.insert(0);
    ad.pids[0].order = 1;

    int ica = add_ca(&fake_ca_op);
    ASSERT(ica >= 0, "could not register the fake CA");
    ad.ca_mask = 1 << ica;

    // verbatim sections from the pmt817.ts sample
    uint8_t sec6043[] = {0x02, 0xB0, 0x2A, 0x17, 0x9B, 0xE9, 0x00, 0x00, 0xFD,
                         0x30, 0xF0, 0x09, 0x09, 0x07, 0x18, 0x14, 0xE6, 0x43,
                         0x02, 0x52, 0x11, 0x1B, 0xFD, 0x30, 0xF0, 0x00, 0x0F,
                         0xFD, 0x31, 0xF0, 0x0A, 0x0A, 0x04, 0x70, 0x6F, 0x72,
                         0x00, 0x7C, 0x02, 0x00, 0x00, 0x43, 0x9A, 0x7E, 0xD6};
    uint8_t sec807[] = {
        0x02, 0xB0, 0x2D, 0x03, 0x27, 0xE9, 0x00, 0x00, 0xFF, 0x70, 0xF0, 0x00,
        0x0F, 0xFF, 0x71, 0xF0, 0x10, 0x09, 0x04, 0x18, 0x02, 0xE4, 0x06, 0x0A,
        0x04, 0x70, 0x6F, 0x72, 0x00, 0x7C, 0x02, 0x00, 0x00, 0x1B, 0xFF, 0x70,
        0xF0, 0x06, 0x09, 0x04, 0x18, 0x02, 0xE4, 0x06, 0xA0, 0x11, 0x76, 0x82};
    uint8_t sec1338[] = {
        0x02, 0xB0, 0x2D, 0x05, 0x3A, 0xE9, 0x00, 0x00, 0xFD, 0x30, 0xF0, 0x00,
        0x0F, 0xFD, 0x31, 0xF0, 0x10, 0x09, 0x04, 0x18, 0x02, 0xE4, 0xCF, 0x0A,
        0x04, 0x70, 0x6F, 0x72, 0x00, 0x7C, 0x02, 0x00, 0x00, 0x1B, 0xFD, 0x30,
        0xF0, 0x06, 0x09, 0x04, 0x18, 0x02, 0xE4, 0xCF, 0xDD, 0x41, 0xAD, 0x2C};

    int f1 = add_filter(0, 817, (void *)process_pmt, NULL, 0);
    int f2 = add_filter(0, 817, (void *)process_pmt, NULL, 0);
    int f3 = add_filter(0, 817, (void *)process_pmt, NULL, 0);
    ASSERT(f1 >= 0 && f2 >= 0 && f3 >= 0, "need three filters on pid 817");

    // higher sid parses first: the winner must not depend on arrival
    ASSERT(process_pmt(f1, sec6043, sizeof(sec6043), NULL) == 0,
           "6043 to parse");
    ASSERT(process_pmt(f2, sec1338, sizeof(sec1338), NULL) == 0,
           "1338 to parse");
    ASSERT(process_pmt(f3, sec807, sizeof(sec807), NULL) == 0, "807 to parse");

    SPMT *p1338 = get_pmt_for_sid_pid(0, 1338, 817);
    SPMT *p6043 = get_pmt_for_sid_pid(0, 6043, 817);
    SPMT *p807 = get_pmt_for_sid_pid(0, 807, 817);
    ASSERT(p1338 && p6043 && p807, "each sid should own a PMT object");
    ASSERT(p1338->stream_pids.size() == 2, "1338 should hold two streams");
    ASSERT(p6043->stream_pids.size() == 2, "6043 should hold two streams");
    ASSERT(p807->stream_pids.size() == 2, "807 should hold two streams");
    ad.active_pmts = 3;
    ad.active_pmt[0] = p1338->id;
    ad.active_pmt[1] = p6043->id;
    ad.active_pmt[2] = p807->id;

    auto carries_ecm = [](SPMT *p, int ecm) {
        for (auto &d : p->descriptors)
            if (d.type == 0x09 && d.get_ca_descriptor_capid() == ecm)
                return true;
        for (auto &sp : p->stream_pids)
            for (auto &d : sp.descriptors)
                if (d.type == 0x09 && d.get_ca_descriptor_capid() == ecm)
                    return true;
        return false;
    };
    ASSERT(carries_ecm(p1338, 1231), "1338 should carry ECM 1231");
    ASSERT(carries_ecm(p6043, 1603), "6043 should carry ECM 1603");
    ASSERT(carries_ecm(p807, 1030), "807 should carry ECM 1030");

    ASSERT(mark_pid_add(0, 0, 7472) == 0, "pid 7472 should be added");
    ASSERT(mark_pid_add(0, 0, 7473) == 0, "pid 7473 should be added");
    ASSERT(mark_pid_add(0, 0, 8048) == 0, "pid 8048 should be added");
    ASSERT(mark_pid_add(0, 0, 8049) == 0, "pid 8049 should be added");
    update_pids(0);
    ASSERT_EQUAL(p1338->state, PMT_RUNNING, "lowest sid should run");
    ASSERT_EQUAL(p6043->state, PMT_STOPPED, "higher sid should wait");
    ASSERT_EQUAL(p807->state, PMT_RUNNING, "disjoint service should run");
    ASSERT(p6043->best, "waiter stays in the set");
    ASSERT(find_pid(0, 7472)->pmt == p1338->id, "1338 should own video");
    ASSERT(find_pid(0, 7473)->pmt == p1338->id, "1338 should own audio");
    ASSERT(find_pid(0, 8048)->pmt == p807->id, "807 should own video");
    start_active_pmts(&ad);
    ASSERT(p1338->ca_mask != 0, "loop should send the winner");
    ASSERT(p807->ca_mask != 0, "loop should send 807");
    ASSERT_EQUAL(p6043->ca_mask, 0, "waiter should never be sent");

    // zap away from the pair: only 807 keeps running
    mark_pid_deleted(0, 0, 7472, NULL);
    mark_pid_deleted(0, 0, 7473, NULL);
    update_pids(0);
    ASSERT_EQUAL(p1338->state, PMT_STOPPED, "1338 should stop");
    ASSERT_EQUAL(p807->state, PMT_RUNNING, "807 should keep running");
    ASSERT(find_pid(0, 7472) == NULL, "video pid should leave the demux");
    start_active_pmts(&ad);
    ASSERT_EQUAL(p807->state, PMT_RUNNING, "807 should stay up");
    ASSERT_EQUAL(p1338->state, PMT_STOPPED, "1338 should stay down");

    del_ca(&fake_ca_op);
    free_all_pmts();
    free_filters();
    a[0] = NULL;
    return 0;
}

// #1129 beIN: PMT 48 (sid 14101) and PMT 52 (sid 14104) share vpid and
// apid. The subscribed PMT pid decides, never the parsed-first sibling.
int test_1129_bein_shared_es() {
    int i;
    uint8_t priv[1] = {0};
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    opts.emulate_pids_all = 0;

    int ica = add_ca(&fake_ca_op);
    ASSERT(ica >= 0, "could not register the fake CA");
    ad.ca_mask = 1 << ica;

    // wrong sibling parsed first: the old master linkage latched onto it
    int ticari = pmt_add(0, 14104, 52);
    int bein = pmt_add(0, 14101, 48);
    ASSERT(ticari >= 0 && bein >= 0, "could not create the PMTs");
    for (int id : {ticari, bein}) {
        pmt_add_stream_pid(pmts[id], 3301, 2, false, true);
        pmt_add_stream_pid(pmts[id], 3401, 3, true, false);
        pmt_add_caid(pmts[id], 0x0664, 0x1F00 + id, priv, 0);
    }
    ad.active_pmts = 2;
    ad.active_pmt[0] = ticari;
    ad.active_pmt[1] = bein;

    ASSERT(mark_pid_add(0, 0, 48) == 0, "pid 48 should be added");
    ASSERT(mark_pid_add(0, 0, 3301) == 0, "pid 3301 should be added");
    ASSERT(mark_pid_add(0, 0, 3401) == 0, "pid 3401 should be added");
    update_pids(0);
    ASSERT_EQUAL(pmts[bein]->state, PMT_RUNNING, "subscribed PMT should run");
    ASSERT_EQUAL(pmts[ticari]->state, PMT_STOPPED, "parsed-first should wait");
    ASSERT(find_pid(0, 3301)->pmt == bein, "subscribed PMT should own video");
    start_active_pmts(&ad);
    ASSERT(pmts[bein]->ca_mask != 0, "loop should send the winner");
    ASSERT_EQUAL(pmts[ticari]->ca_mask, 0, "loser should never be sent");

    // zap to the other sibling on the same shared streams
    mark_pid_deleted(0, 0, 48, NULL);
    ASSERT(mark_pid_add(0, 0, 52) == 0, "pid 52 should be added");
    update_pids(0);
    ASSERT_EQUAL(pmts[ticari]->state, PMT_RUNNING, "new zap should run");
    ASSERT_EQUAL(pmts[bein]->state, PMT_STOPPED, "old zap should stop");
    ASSERT(find_pid(0, 3301)->pmt == ticari, "video should move with the zap");

    del_ca(&fake_ca_op);
    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// #1129 Stingray: music channels share VPID 108 with different APIDs.
// NATURE ESCAPE (PMT 1908, APID 208) vs '80'ler (PMT 1922, APID 222).
int test_1129_stingray_shared_vpid() {
    int i;
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    opts.emulate_pids_all = 0;

    int eighties = pmt_add(0, 200, 1922);
    int nature = pmt_add(0, 100, 1908);
    ASSERT(eighties >= 0 && nature >= 0, "could not create the PMTs");
    pmt_add_stream_pid(pmts[nature], 108, 2, false, true);
    pmt_add_stream_pid(pmts[nature], 208, 3, true, false);
    pmt_add_stream_pid(pmts[eighties], 108, 2, false, true);
    pmt_add_stream_pid(pmts[eighties], 222, 3, true, false);
    ad.active_pmts = 2;
    ad.active_pmt[0] = eighties;
    ad.active_pmt[1] = nature;

    ASSERT(mark_pid_add(0, 0, 1908) == 0, "pid 1908 should be added");
    ASSERT(mark_pid_add(0, 0, 108) == 0, "pid 108 should be added");
    ASSERT(mark_pid_add(0, 0, 208) == 0, "pid 208 should be added");
    update_pids(0);
    ASSERT_EQUAL(pmts[nature]->state, PMT_RUNNING, "subscribed PMT should run");
    ASSERT_EQUAL(pmts[eighties]->state, PMT_STOPPED, "sibling should wait");
    ASSERT(find_pid(0, 108)->pmt == nature, "shared VPID follows the sub");
    ASSERT(find_pid(0, 208)->pmt == nature, "own APID follows the sub");
    ASSERT(find_pid(0, 222) == NULL, "other APID stays out of the demux");

    // zap to '80'ler: the shared VPID moves with the subscription
    mark_pid_deleted(0, 0, 1908, NULL);
    mark_pid_deleted(0, 0, 208, NULL);
    ASSERT(mark_pid_add(0, 0, 1922) == 0, "pid 1922 should be added");
    ASSERT(mark_pid_add(0, 0, 222) == 0, "pid 222 should be added");
    update_pids(0);
    ASSERT_EQUAL(pmts[eighties]->state, PMT_RUNNING, "new zap should run");
    ASSERT_EQUAL(pmts[nature]->state, PMT_STOPPED, "old zap should stop");
    ASSERT(find_pid(0, 108)->pmt == eighties, "VPID should move with zap");
    ASSERT(find_pid(0, 222)->pmt == eighties, "new APID should be owned");

    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// A RUNNING PMT whose pids are held without any client sid (DDCI marks)
// is out of the set and must stop, releasing its CA registration.
int test_held_pids_without_client_stop_pmt() {
    int i;
    uint8_t priv[1] = {0};
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    opts.emulate_pids_all = 0;

    int ica = add_ca(&fake_ca_op);
    ASSERT(ica >= 0, "could not register the fake CA");
    ad.ca_mask = 1 << ica;

    int id = pmt_add(0, 100, 48);
    ASSERT(id >= 0, "could not create the PMT");
    pmt_add_stream_pid(pmts[id], 3301, 2, false, true);
    pmt_add_stream_pid(pmts[id], 3401, 3, true, false);
    pmt_add_caid(pmts[id], 0x0664, 0x1F06, priv, 0);
    send_pmt_to_cas(&ad, pmts[id]);
    ASSERT(pmts[id]->ca_mask != 0, "PMT was not registered with the CA");
    pmts[id]->state = PMT_RUNNING;
    ad.active_pmts = 1;
    ad.active_pmt[0] = id;

    int pids[] = {48, 3301, 3401};
    for (i = 0; i < 3; i++) {
        ad.pids[i].pid = pids[i];
        ad.pids[i].flags = PID_STATE_ACTIVE;
        ad.pids[i].pmt = id;
        ad.pids[i].filter = -1;
        ad.pids[i].sid.insert(DDCI_SID);
    }
    fake_ca_del_calls = 0;
    update_pids(0);
    ASSERT_EQUAL(pmts[id]->state, PMT_STOPPED, "held PMT should stop");
    ASSERT_EQUAL(fake_ca_del_calls, 1, "held PMT should release the CA");
    ASSERT_EQUAL(pmts[id]->ca_mask, 0, "CA registration should be gone");

    del_ca(&fake_ca_op);
    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// CWs are stored under the PMT they arrived for, with no master hop.
int test_cw_keyed_by_pmt() {
    int i;
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;
    for (i = 0; i < MAX_CW; i++)
        if (cws[i])
            cws[i]->enabled = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    init_algo();

    int id = pmt_add(0, 100, 48);
    ASSERT(id >= 0, "could not create the PMT");
    uint8_t cw[8] = {0x10, 0x22, 0x34, 0x66, 0x88, 0x9A, 0xAC, 0xC6};
    ASSERT(send_cw(id, CA_ALGO_DVBCSA, 0, cw, NULL, 25, NULL) == 0,
           "send_cw should store the CW");
    int found = -1;
    for (i = 0; i < MAX_CW; i++)
        if (cws[i] && cws[i]->enabled && cws[i]->pmt == id)
            found = i;
    ASSERT(found >= 0, "CW should be keyed by the PMT id");
    ASSERT(send_cw(id, CA_ALGO_DVBCSA, 0, cw, NULL, 25, NULL) != 0,
           "duplicate CW should be rejected");

    for (i = 0; i < MAX_CW; i++)
        if (cws[i])
            cws[i]->enabled = 0;
    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// pids=all expands every known PMT pid with the requestor sid, so the
// subscribed-PMT rule still decrypts as before under claims arbitration.
int test_pids_all_expands_pmt_pids() {
    int i;
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    opts.emulate_pids_all = 1;

    int id = pmt_add(0, 100, 1000);
    ASSERT(id >= 0, "could not create the PMT");
    pmt_add_stream_pid(pmts[id], 100, 2, false, true);
    ad.active_pmts = 1;
    ad.active_pmt[0] = id;

    ASSERT(mark_pid_add(0, 0, 8192) == 0, "pids=all should be added");
    update_pids(0);
    SPid *pp = find_pid(0, 1000);
    ASSERT(pp && pp->has_stream(0), "PMT pid should carry the client sid");
    SPid *ps = find_pid(0, 100);
    ASSERT(ps && ps->has_stream(0), "stream pid should carry the client sid");

    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should run for pids=all");

    opts.emulate_pids_all = 0;
    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// Multi-service PMT pid: two SIDs on pid 60 parse into two PMT objects
// with their own ES lists, which is what D8 selection arbitrates.
int test_multi_service_pid_parses_per_sid() {
    int i;
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;
    for (i = 0; i < MAX_FILTERS; i++)
        filters[i] = NULL;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    ad.pids[0].pid = 60;
    ad.pids[0].flags = PID_STATE_ACTIVE;
    ad.pids[0].pmt = -1;
    ad.pids[0].filter = -1;
    ad.pids[0].sid.insert(0);
    ad.pids[0].order = 1;

    int f1 = add_filter(0, 60, (void *)process_pmt, NULL, 0);
    int f2 = add_filter(0, 60, (void *)process_pmt, NULL, 0);
    ASSERT(f1 >= 0 && f2 >= 0 && f1 != f2, "need two filters on pid 60");

    uint8_t sec[21] = {0x02, 0xB0, 0x12, 0x00, 0x64, 0xC1, 0x00,
                       0x00, 0xE2, 0xBD, 0xF0, 0x00, 0x02, 0xE2,
                       0xBD, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00};
    ASSERT(process_pmt(f1, sec, sizeof(sec), NULL) == 0, "sid 100 to parse");
    sec[4] = 0xC8; // sid 200, video pid 702
    sec[8] = 0xE2;
    sec[9] = 0xBE;
    sec[13] = 0xE2;
    sec[14] = 0xBE;
    ASSERT(process_pmt(f2, sec, sizeof(sec), NULL) == 0, "sid 200 to parse");

    SPMT *p1 = get_pmt_for_sid_pid(0, 100, 60);
    SPMT *p2 = get_pmt_for_sid_pid(0, 200, 60);
    ASSERT(p1 && p2 && p1 != p2, "each sid should own a PMT object");
    ASSERT(p1->stream_pids.size() == 1 && p1->stream_pids[0].pid == 701,
           "sid 100 should carry pid 701");
    ASSERT(p2->stream_pids.size() == 1 && p2->stream_pids[0].pid == 702,
           "sid 200 should carry pid 702");

    free_all_pmts();
    free_filters();
    a[0] = NULL;
    return 0;
}

// Late parse handover: Q runs holding pids subscribed after P, then P
// parses. process_pmt stops Q at once; P starts on the next pass.
int test_late_parse_handover() {
    int i;
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;

    int qid = pmt_add(0, 14104, 52);
    ASSERT(qid >= 0, "could not create Q");
    pmt_add_stream_pid(pmts[qid], 3301, 2, false, true);
    pmt_add_stream_pid(pmts[qid], 3401, 3, true, false);
    ad.active_pmts = 1;
    ad.active_pmt[0] = qid;

    int pids[] = {52, 3301, 3401};
    int orders[] = {5, 6, 7};
    for (i = 0; i < 3; i++) {
        ad.pids[i].pid = pids[i];
        ad.pids[i].flags = PID_STATE_ACTIVE;
        ad.pids[i].pmt = -1;
        ad.pids[i].filter = -1;
        ad.pids[i].sid.insert(1);
        if (i > 0)
            ad.pids[i].sid.insert(0);
        ad.pids[i].order = orders[i];
    }
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[qid]->state, PMT_RUNNING, "Q should run alone");
    ASSERT(find_pid(0, 3301)->pmt == qid, "Q should own the video pid");

    // P subscribed earlier but parses only now
    ad.pids[3].pid = 48;
    ad.pids[3].flags = PID_STATE_ACTIVE;
    ad.pids[3].pmt = -1;
    ad.pids[3].filter = -1;
    ad.pids[3].sid.insert(0);
    ad.pids[3].order = 3;
    int fid = add_filter(0, 48, (void *)process_pmt, NULL, 0);
    ASSERT(fid >= 0, "could not add the PMT filter");

    uint8_t sec[26] = {0x02, 0xB0, 0x17, 0x37, 0x15, 0xC1, 0x00, 0x00, 0xEC,
                       0xE5, 0xF0, 0x00, 0x02, 0xEC, 0xE5, 0xF0, 0x00, 0x03,
                       0xED, 0x49, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00};
    ASSERT(process_pmt(fid, sec, sizeof(sec), NULL) == 0, "P to parse");
    SPMT *p = get_pmt_for_sid_pid(0, 14101, 48);
    ASSERT(p && p->stream_pids.size() == 2, "P should hold both streams");
    ASSERT_EQUAL(pmts[qid]->state, PMT_STOPPED, "Q should stop at parse");
    ASSERT(find_pid(0, 3301)->pmt == -1, "video claim should be released");

    start_active_pmts(&ad);
    ASSERT_EQUAL(p->state, PMT_RUNNING, "P should start on next pass");
    ASSERT(find_pid(0, 3301)->pmt == p->id, "P should own the video pid");

    del_filter(fid);
    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// Last unsubscribe deletes the pid, but the PMT stays on AV with no rival.
int test_running_pmt_pid_deleted_on_unsubscribe() {
    int i;
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    opts.emulate_pids_all = 0;

    int id = pmt_add(0, 100, 48);
    ASSERT(id >= 0, "could not create the PMT");
    pmt_add_stream_pid(pmts[id], 3301, 2, false, true);
    pmt_add_stream_pid(pmts[id], 3401, 3, true, false);
    int fid = add_filter(0, 48, (void *)process_pmt, pmts[id], 0);
    ASSERT(fid >= 0, "could not add the PMT filter");
    pmts[id]->filter = fid;
    ad.active_pmts = 1;
    ad.active_pmt[0] = id;

    int pids[] = {48, 3301, 3401};
    for (i = 0; i < 3; i++) {
        ad.pids[i].pid = pids[i];
        ad.pids[i].flags = PID_STATE_ACTIVE;
        ad.pids[i].pmt = -1;
        ad.pids[i].filter = i == 0 ? fid : -1;
        ad.pids[i].sid.insert(0);
        ad.pids[i].order = i + 1;
    }
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should run");
    ASSERT(filters[fid]->flags == FILTER_CRC, "running filter keeps CRC only");

    mark_pid_deleted(0, 0, 48, NULL);
    update_pids(0);
    ASSERT(find_pid(0, 48) == NULL, "PMT pid should leave the demux");
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should stay on pid delete");
    ASSERT(find_pid(0, 3301)->pmt == id, "video claim should be kept");

    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should stay running");

    del_filter(fid);
    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// Pid leaves and returns, AV stays with no rival: PMT runs, CA sees no churn.
int test_pmt_pid_remove_readd_no_churn() {
    int i;
    uint8_t priv[1] = {0};
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    opts.emulate_pids_all = 0;

    SCA_op counting_op = fake_ca_op;
    counting_op.ca_add_pmt = counting_ca_add_pmt;
    int ica = add_ca(&counting_op);
    ASSERT(ica >= 0, "could not register the fake CA");
    ad.ca_mask = 1 << ica;

    int id = pmt_add(0, 222, 222);
    ASSERT(id >= 0, "could not create the PMT");
    pmt_add_stream_pid(pmts[id], 2221, 27, false, true);
    pmt_add_stream_pid(pmts[id], 2222, 6, true, false);
    pmt_add_caid(pmts[id], 0x4AFC, 2220, priv, 0);
    int fid = add_filter(0, 222, (void *)process_pmt, pmts[id], 0);
    ASSERT(fid >= 0, "could not add the PMT filter");
    pmts[id]->filter = fid;
    ad.active_pmts = 1;
    ad.active_pmt[0] = id;

    ASSERT(mark_pid_add(0, 0, 222) == 0, "pid 222 should be added");
    ASSERT(mark_pid_add(0, 0, 2221) == 0, "pid 2221 should be added");
    ASSERT(mark_pid_add(0, 0, 2222) == 0, "pid 2222 should be added");
    update_pids(0);
    fake_ca_add_calls = fake_ca_del_calls = 0;
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should run");
    ASSERT_EQUAL(fake_ca_add_calls, 1, "PMT should reach the CA once");

    // delpids=222 while the streams stay subscribed
    fake_ca_add_calls = fake_ca_del_calls = 0;
    mark_pid_deleted(0, 0, 222, NULL);
    update_pids(0);
    ASSERT(find_pid(0, 222) == NULL, "pid 222 should leave the demux");
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should stay");
    ASSERT_EQUAL(fake_ca_del_calls, 0, "PMT should not leave the CA");
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should stay on pass");
    ASSERT_EQUAL(fake_ca_add_calls, 0, "PMT should not be re-sent");

    // addpids=222 back: still no churn
    ASSERT(mark_pid_add(0, 0, 222) == 0, "pid 222 should be added back");
    update_pids(0);
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should stay");
    ASSERT_EQUAL(fake_ca_del_calls, 0, "PMT should not leave the CA");
    ASSERT_EQUAL(fake_ca_add_calls, 0, "PMT should not be re-sent");
    ASSERT(find_pid(0, 2221)->pmt == id, "video claim should be kept");

    del_ca(&counting_op);
    del_filter(fid);
    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// Scenario 1: deleting the PMT pid stops its PMT while the streams stay
// subscribed; the sibling takes over once its own pid is subscribed.
int test_pmt_pid_delete_hands_over() {
    int i;
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    opts.emulate_pids_all = 0;

    int aid = pmt_add(0, 14101, 48);
    int bid = pmt_add(0, 14104, 52);
    ASSERT(aid >= 0 && bid >= 0, "could not create the PMTs");
    for (int id : {aid, bid}) {
        pmt_add_stream_pid(pmts[id], 3301, 2, false, true);
        pmt_add_stream_pid(pmts[id], 3401, 3, true, false);
    }
    int fa = add_filter(0, 48, (void *)process_pmt, pmts[aid], 0);
    int fb = add_filter(0, 52, (void *)process_pmt, pmts[bid], 0);
    ASSERT(fa >= 0 && fb >= 0, "could not add the PMT filters");
    pmts[aid]->filter = fa;
    pmts[bid]->filter = fb;
    ad.active_pmts = 2;
    ad.active_pmt[0] = aid;
    ad.active_pmt[1] = bid;

    ASSERT(mark_pid_add(0, 0, 48) == 0, "pid 48 should be added");
    ASSERT(mark_pid_add(0, 0, 3301) == 0, "pid 3301 should be added");
    ASSERT(mark_pid_add(0, 0, 3401) == 0, "pid 3401 should be added");
    update_pids(0);
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[aid]->state, PMT_RUNNING, "A should run");
    ASSERT(find_pid(0, 3301)->pmt == aid, "A should own the video pid");

    // zap: the PMT pid leaves while the streams stay subscribed
    mark_pid_deleted(0, 0, 48, NULL);
    ASSERT(mark_pid_add(0, 0, 52) == 0, "pid 52 should be added");
    update_pids(0);
    ASSERT_EQUAL(pmts[aid]->state, PMT_STOPPED, "A should stop on pid delete");
    ASSERT_EQUAL(pmts[bid]->state, PMT_RUNNING, "B should take over at once");
    ASSERT(find_pid(0, 3301)->pmt == bid, "B should own the video pid");
    ASSERT(find_pid(0, 48) == NULL, "pid 48 should leave the demux");

    // the loop pass that used to elect B is now a no-op backstop
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[bid]->state, PMT_RUNNING, "B should stay up");

    del_filter(fa);
    del_filter(fb);
    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// The update_pids tail elects synchronously but never sends: states and
// claims settle with no demux pass, CAPMTs go out on the loop instead.
int test_update_pids_tail_elects() {
    int i;
    uint8_t priv[1] = {0};
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    opts.emulate_pids_all = 0;

    int ica = add_ca(&fake_ca_op);
    ASSERT(ica >= 0, "could not register the fake CA");
    ad.ca_mask = 1 << ica;

    int aid = pmt_add(0, 14101, 48);
    int bid = pmt_add(0, 14104, 52);
    ASSERT(aid >= 0 && bid >= 0, "could not create the PMTs");
    for (int id : {aid, bid}) {
        pmt_add_stream_pid(pmts[id], 3301, 2, false, true);
        pmt_add_stream_pid(pmts[id], 3401, 3, true, false);
        pmt_add_caid(pmts[id], 0x0664, 0x1F00 + id, priv, 0);
    }
    int fa = add_filter(0, 48, (void *)process_pmt, pmts[aid], 0);
    int fb = add_filter(0, 52, (void *)process_pmt, pmts[bid], 0);
    ASSERT(fa >= 0 && fb >= 0, "could not add the PMT filters");
    pmts[aid]->filter = fa;
    pmts[bid]->filter = fb;
    ad.active_pmts = 2;
    ad.active_pmt[0] = aid;
    ad.active_pmt[1] = bid;

    ASSERT(mark_pid_add(0, 0, 48) == 0, "pid 48 should be added");
    ASSERT(mark_pid_add(0, 0, 3301) == 0, "pid 3301 should be added");
    ASSERT(mark_pid_add(0, 0, 3401) == 0, "pid 3401 should be added");
    update_pids(0);
    ASSERT_EQUAL(pmts[aid]->state, PMT_RUNNING, "A should run with no pump");
    ASSERT(find_pid(0, 3301)->pmt == aid, "A should own the video pid");
    ASSERT_EQUAL(pmts[aid]->ca_mask, 0, "A should be elected but unsent");
    start_active_pmts(&ad);
    ASSERT(pmts[aid]->ca_mask != 0, "the loop should send A to the CA");

    // zap: the PMT pid leaves while the streams stay subscribed
    mark_pid_deleted(0, 0, 48, NULL);
    ASSERT(mark_pid_add(0, 0, 52) == 0, "pid 52 should be added");
    fake_ca_del_calls = 0;
    fake_ca_last_del_pmt = -1;
    update_pids(0);
    ASSERT_EQUAL(pmts[aid]->state, PMT_STOPPED, "A should stop on pid delete");
    ASSERT_EQUAL(fake_ca_del_calls, 1, "A should release the CA at the tail");
    ASSERT_EQUAL(fake_ca_last_del_pmt, aid, "the wrong PMT left the CA");
    ASSERT_EQUAL(pmts[bid]->state, PMT_RUNNING,
                 "B should take over with no pump");
    ASSERT(find_pid(0, 3301)->pmt == bid, "B should own the video pid");
    ASSERT_EQUAL(pmts[bid]->ca_mask, 0, "B should be elected but unsent");
    start_active_pmts(&ad);
    ASSERT(pmts[bid]->ca_mask != 0, "the loop should send B to the CA");

    del_ca(&fake_ca_op);
    del_filter(fa);
    del_filter(fb);
    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// Scenario 2: deleting the streams stops the PMT even though its own pid
// stays subscribed.
int test_stream_pid_delete_stops_pmt() {
    int i;
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    opts.emulate_pids_all = 0;

    int id = pmt_add(0, 100, 48);
    ASSERT(id >= 0, "could not create the PMT");
    pmt_add_stream_pid(pmts[id], 3301, 2, false, true);
    pmt_add_stream_pid(pmts[id], 3401, 3, true, false);
    int fid = add_filter(0, 48, (void *)process_pmt, pmts[id], 0);
    ASSERT(fid >= 0, "could not add the PMT filter");
    pmts[id]->filter = fid;
    ad.active_pmts = 1;
    ad.active_pmt[0] = id;

    ASSERT(mark_pid_add(0, 0, 48) == 0, "pid 48 should be added");
    ASSERT(mark_pid_add(0, 0, 3301) == 0, "pid 3301 should be added");
    ASSERT(mark_pid_add(0, 0, 3401) == 0, "pid 3401 should be added");
    update_pids(0);
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should run");

    mark_pid_deleted(0, 0, 3301, NULL);
    mark_pid_deleted(0, 0, 3401, NULL);
    update_pids(0);
    ASSERT_EQUAL(pmts[id]->state, PMT_STOPPED, "PMT should stop");
    ASSERT(find_pid(0, 3301) == NULL, "video pid should be gone");
    SPid *pp = find_pid(0, 48);
    ASSERT(pp && pp->flags == PID_STATE_ACTIVE, "PMT pid should remain");

    del_filter(fid);
    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// A PMT version bump with identical content (#1346) keeps the PMT
// running: no CA delete, claims kept, the CA gets an update re-send.
int test_pmt_cosmetic_update_keeps_running() {
    int i;
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    opts.emulate_pids_all = 0;

    SCA_op counting_op = fake_ca_op;
    counting_op.ca_add_pmt = counting_ca_add_pmt;
    int ica = add_ca(&counting_op);
    ASSERT(ica >= 0, "could not register the fake CA");
    ad.ca_mask = 1 << ica;

    int id = pmt_add(0, 100, 48);
    ASSERT(id >= 0, "could not create the PMT");
    int fid = add_filter(0, 48, (void *)process_pmt, pmts[id], 0);
    ASSERT(fid >= 0, "could not add the PMT filter");
    pmts[id]->filter = fid;

    ASSERT(mark_pid_add(0, 0, 48) == 0, "pid 48 should be added");
    ASSERT(mark_pid_add(0, 0, 3301) == 0, "pid 3301 should be added");
    ASSERT(mark_pid_add(0, 0, 3401) == 0, "pid 3401 should be added");
    update_pids(0);

    int types[] = {2, 3};
    int spids[] = {3301, 3401};
    uint8_t sec[64];
    int len = build_pmt(sec, 100, 1, 3301, 0x0B00, 0x0C00, types, spids, 2);
    ASSERT(process_pmt(fid, sec, len, pmts[id]) == 0, "v1 to parse");
    fake_ca_add_calls = fake_ca_del_calls = 0;
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should run");
    ASSERT(fake_ca_add_calls == 1, "PMT should be sent to the CA");

    len = build_pmt(sec, 100, 2, 3301, 0x0B00, 0x0C00, types, spids, 2);
    fake_ca_add_calls = fake_ca_del_calls = 0;
    ASSERT(process_pmt(fid, sec, len, pmts[id]) == 0, "v2 to parse");
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should keep running");
    ASSERT_EQUAL(pmts[id]->version, 2, "version should advance");
    ASSERT(find_pid(0, 3301)->pmt == id, "video claim should be kept");
    ASSERT(find_pid(0, 3401)->pmt == id, "audio claim should be kept");
    ASSERT_EQUAL(fake_ca_del_calls, 0, "CA should see no delete");

    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should still run");
    ASSERT_EQUAL(fake_ca_add_calls, 1, "CA should see the update");
    ASSERT_EQUAL(fake_ca_last_add_update, 1, "re-send is an update");

    // v3 reorders the same streams: the CA gets a re-send, but
    // nothing is dropped or deleted.
    int types3[] = {3, 2};
    int spids3[] = {3401, 3301};
    len = build_pmt(sec, 100, 3, 3301, 0x0B00, 0x0C00, types3, spids3, 2);
    fake_ca_add_calls = fake_ca_del_calls = 0;
    ASSERT(process_pmt(fid, sec, len, pmts[id]) == 0, "v3 to parse");
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should keep running");
    ASSERT_EQUAL(pmts[id]->version, 3, "version should advance");
    ASSERT_EQUAL((int)pmts[id]->stream_pids.size(), 2,
                 "streams should be intact");
    ASSERT_EQUAL(pmts[id]->pcr_pid, 3301, "PCR should be intact");
    ASSERT(find_pid(0, 3301)->pmt == id, "video claim should be kept");
    ASSERT(find_pid(0, 3401)->pmt == id, "audio claim should be kept");
    ASSERT_EQUAL(fake_ca_del_calls, 0, "CA should see no delete");
    ASSERT_EQUAL(pmts[id]->ca_mask, 0, "CA re-send should be pending");

    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should still run");
    ASSERT(fake_ca_add_calls == 1, "CA should see one benign re-send");

    del_ca(&counting_op);
    del_filter(fid);
    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// A PMT version update with changed streams keeps the PMT running;
// removed pids keep stale claims the election ignores, and the CA
// gets a re-send, no delete.
int test_pmt_content_update_delta() {
    int i;
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    opts.emulate_pids_all = 0;

    SCA_op counting_op = fake_ca_op;
    counting_op.ca_add_pmt = counting_ca_add_pmt;
    int ica = add_ca(&counting_op);
    ASSERT(ica >= 0, "could not register the fake CA");
    ad.ca_mask = 1 << ica;

    int id = pmt_add(0, 100, 48);
    ASSERT(id >= 0, "could not create the PMT");
    int fid = add_filter(0, 48, (void *)process_pmt, pmts[id], 0);
    ASSERT(fid >= 0, "could not add the PMT filter");
    pmts[id]->filter = fid;

    ASSERT(mark_pid_add(0, 0, 48) == 0, "pid 48 should be added");
    ASSERT(mark_pid_add(0, 0, 3301) == 0, "pid 3301 should be added");
    ASSERT(mark_pid_add(0, 0, 3401) == 0, "pid 3401 should be added");
    update_pids(0);

    int types[] = {2, 3};
    int spids[] = {3301, 3401};
    uint8_t sec[64];
    int len = build_pmt(sec, 100, 1, 3301, 0x0B00, 0x0C00, types, spids, 2);
    ASSERT(process_pmt(fid, sec, len, pmts[id]) == 0, "v1 to parse");
    fake_ca_add_calls = fake_ca_del_calls = 0;
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should run");
    ASSERT(fake_ca_add_calls == 1, "PMT should be sent to the CA");

    int types2[] = {2};
    int spids2[] = {3301};
    len = build_pmt(sec, 100, 2, 3301, 0x0B00, 0x0C00, types2, spids2, 1);
    fake_ca_add_calls = fake_ca_del_calls = 0;
    ASSERT(process_pmt(fid, sec, len, pmts[id]) == 0, "v2 to parse");
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should keep running");
    ASSERT_EQUAL(pmts[id]->version, 2, "version should advance");
    ASSERT(find_pid(0, 3301)->pmt == id, "video claim should be kept");
    // Removed pids keep a stale claim; the election ignores holders
    // that no longer list the pid.
    ASSERT(find_pid(0, 3401)->pmt == id, "stale audio claim should linger");
    ASSERT_EQUAL(fake_ca_del_calls, 0, "CA should see no delete");
    ASSERT_EQUAL(pmts[id]->ca_mask, 0, "CA re-send should be pending");

    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should still run");
    ASSERT(fake_ca_add_calls == 1, "CA should see one re-send");
    ASSERT(pmts[id]->ca_mask != 0, "PMT should hold a CA slot again");
    ASSERT(find_pid(0, 3401)->pmt == id, "stale claim should survive pass");

    // v3 changes only the ECM pid: still a delta re-send, no delete.
    len = build_pmt(sec, 100, 3, 3301, 0x0B00, 0x0C01, types2, spids2, 1);
    fake_ca_add_calls = fake_ca_del_calls = 0;
    ASSERT(process_pmt(fid, sec, len, pmts[id]) == 0, "v3 to parse");
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should keep running");
    ASSERT_EQUAL(pmts[id]->version, 3, "version should advance");
    ASSERT(find_pid(0, 3301)->pmt == id, "video claim should be kept");
    ASSERT_EQUAL(fake_ca_del_calls, 0, "CA should see no delete");
    ASSERT_EQUAL(pmts[id]->ca_mask, 0, "CA re-send should be pending");

    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should still run");
    ASSERT(fake_ca_add_calls == 1, "CA should see one re-send");
    ASSERT(pmts[id]->ca_mask != 0, "PMT should hold a CA slot again");

    // v4 changes only the stream type: same pids, so the delta path
    // drops nothing but still re-sends while encrypted.
    int types4[] = {27};
    len = build_pmt(sec, 100, 4, 3301, 0x0B00, 0x0C01, types4, spids2, 1);
    fake_ca_add_calls = fake_ca_del_calls = 0;
    ASSERT(process_pmt(fid, sec, len, pmts[id]) == 0, "v4 to parse");
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should keep running");
    ASSERT_EQUAL(pmts[id]->version, 4, "version should advance");
    ASSERT(find_pid(0, 3301)->pmt == id, "video claim should be kept");
    ASSERT_EQUAL(fake_ca_del_calls, 0, "CA should see no delete");
    ASSERT_EQUAL(pmts[id]->ca_mask, 0, "CA re-send should be pending");

    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should still run");
    ASSERT(fake_ca_add_calls == 1, "CA should see one re-send");
    ASSERT(pmts[id]->ca_mask != 0, "PMT should hold a CA slot again");

    // v5 turns the service FTA: the CA registration is released, and
    // nothing is re-sent afterwards.
    len = build_pmt(sec, 100, 5, 3301, 0, 0, types4, spids2, 1);
    fake_ca_add_calls = fake_ca_del_calls = 0;
    ASSERT(process_pmt(fid, sec, len, pmts[id]) == 0, "v5 to parse");
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should keep running");
    ASSERT_EQUAL(pmts[id]->version, 5, "version should advance");
    ASSERT(find_pid(0, 3301)->pmt == id, "video claim should be kept");
    ASSERT_EQUAL(fake_ca_del_calls, 1, "CA registration should be released");
    ASSERT_EQUAL(pmts[id]->ca_registered_mask, 0, "registration is gone");

    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should still run");
    ASSERT_EQUAL(fake_ca_add_calls, 0, "FTA PMT should see no re-send");

    // A later PMT listing the dropped pid steals it: the stale holder
    // no longer lists 3401, so its earlier order does not block.
    int bid = pmt_add(0, 200, 52);
    ASSERT(bid >= 0, "could not create the second PMT");
    uint8_t priv[1] = {0};
    pmt_add_caid(pmts[bid], 0x0B00, 0x0C00, priv, 0);
    pmt_add_stream_pid(pmts[bid], 3401, 3, true, false);
    ASSERT(mark_pid_add(0, 0, 52) == 0, "pid 52 should be added");
    update_pids(0);
    pmt_add_active_pmt(&ad, bid);

    // Preconditions for a stale steal: 3401 still held by A, and B
    // subscribed strictly later so order alone would block the steal.
    ASSERT(find_pid(0, 3401)->pmt == id, "3401 should still be stale-held");
    ASSERT(find_pid(0, 48)->order < find_pid(0, 52)->order,
           "B should subscribe later than A");
    fake_ca_add_calls = fake_ca_del_calls = 0;
    start_active_pmts(&ad);
    ASSERT(find_pid(0, 3401)->pmt == bid, "stale claim should be stolen");
    ASSERT_EQUAL(pmts[bid]->state, PMT_RUNNING, "second PMT should run");
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "first PMT should still run");
    ASSERT(fake_ca_add_calls == 1, "second PMT should be sent to the CA");
    ASSERT_EQUAL(fake_ca_last_add_pmt, bid, "B should be the one sent");

    del_ca(&counting_op);
    del_filter(fid);
    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// The CA add call carries update=0 on first registration and update=1
// on a PMT-update re-send; every version bump re-sends.
int test_ca_update_flag_distinguishes_resend() {
    int i;
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    opts.emulate_pids_all = 0;

    SCA_op counting_op = fake_ca_op;
    counting_op.ca_add_pmt = counting_ca_add_pmt;
    int ica = add_ca(&counting_op);
    ASSERT(ica >= 0, "could not register the fake CA");
    ad.ca_mask = 1 << ica;

    int id = pmt_add(0, 100, 48);
    ASSERT(id >= 0, "could not create the PMT");
    int fid = add_filter(0, 48, (void *)process_pmt, pmts[id], 0);
    ASSERT(fid >= 0, "could not add the PMT filter");
    pmts[id]->filter = fid;

    ASSERT(mark_pid_add(0, 0, 48) == 0, "pid 48 should be added");
    ASSERT(mark_pid_add(0, 0, 3301) == 0, "pid 3301 should be added");
    update_pids(0);

    int types[] = {2};
    int spids[] = {3301};
    uint8_t sec[64];
    int len = build_pmt(sec, 100, 1, 3301, 0x0B00, 0x0C00, types, spids, 1);
    ASSERT(process_pmt(fid, sec, len, pmts[id]) == 0, "v1 to parse");
    fake_ca_add_calls = 0;
    start_active_pmts(&ad);
    ASSERT_EQUAL(fake_ca_add_calls, 1, "PMT should be sent to the CA");
    ASSERT_EQUAL(fake_ca_last_add_update, 0, "first send is an add");

    len = build_pmt(sec, 100, 2, 3301, 0x0B00, 0x0C00, types, spids, 1);
    fake_ca_add_calls = 0;
    ASSERT(process_pmt(fid, sec, len, pmts[id]) == 0, "v2 to parse");
    start_active_pmts(&ad);
    ASSERT_EQUAL(fake_ca_add_calls, 1, "cosmetic bump still informs the CA");
    ASSERT_EQUAL(fake_ca_last_add_update, 1, "re-send is an update");

    ASSERT(mark_pid_add(0, 0, 3401) == 0, "pid 3401 should be added");
    update_pids(0);
    int types3[] = {2, 3};
    int spids3[] = {3301, 3401};
    len = build_pmt(sec, 100, 3, 3301, 0x0B00, 0x0C00, types3, spids3, 2);
    fake_ca_add_calls = 0;
    ASSERT(process_pmt(fid, sec, len, pmts[id]) == 0, "v3 to parse");
    start_active_pmts(&ad);
    ASSERT_EQUAL(fake_ca_add_calls, 1, "content change re-sends");
    ASSERT_EQUAL(fake_ca_last_add_update, 1, "re-send is an update");

    del_ca(&counting_op);
    del_filter(fid);
    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// A late parse whose streams are stale-held by a later PMT must not
// stop it: the holder dropped the pid, so handover skips it.
int test_handover_ignores_stale_holder() {
    int i;
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    opts.emulate_pids_all = 0;

    // B's PMT pid first: B subscribes strictly earlier than A.
    ASSERT(mark_pid_add(0, 0, 60) == 0, "pid 60 should be added");
    ASSERT(mark_pid_add(0, 0, 48) == 0, "pid 48 should be added");
    ASSERT(mark_pid_add(0, 0, 3301) == 0, "pid 3301 should be added");
    ASSERT(mark_pid_add(0, 0, 3302) == 0, "pid 3302 should be added");
    update_pids(0);
    ASSERT(find_pid(0, 60)->order < find_pid(0, 48)->order,
           "B should subscribe earlier than A");

    int aid = pmt_add(0, 100, 48);
    ASSERT(aid >= 0, "could not create PMT A");
    pmt_add_stream_pid(pmts[aid], 3301, 2, false, true);
    pmt_add_stream_pid(pmts[aid], 3302, 2, false, true);
    pmt_add_active_pmt(&ad, aid);
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[aid]->state, PMT_RUNNING, "A should run");
    ASSERT(find_pid(0, 3301)->pmt == aid, "A should claim 3301");

    // A drops 3301 without unclaiming (post-update state).
    for (auto it = pmts[aid]->stream_pids.begin();
         it != pmts[aid]->stream_pids.end(); ++it)
        if (it->pid == 3301) {
            pmts[aid]->stream_pids.erase(it);
            break;
        }

    int bid = pmt_add(0, 200, 60);
    ASSERT(bid >= 0, "could not create PMT B");
    int bfid = add_filter(0, 60, (void *)process_pmt, pmts[bid], 0);
    ASSERT(bfid >= 0, "could not add the PMT filter");
    pmts[bid]->filter = bfid;
    int types[] = {2};
    int spids[] = {3301};
    uint8_t sec[64];
    int len = build_pmt(sec, 200, 1, 3301, 0, 0, types, spids, 1);
    ASSERT(process_pmt(bfid, sec, len, pmts[bid]) == 0, "B to parse");
    ASSERT_EQUAL(pmts[aid]->state, PMT_RUNNING, "A must survive handover");

    start_active_pmts(&ad);
    ASSERT(find_pid(0, 3301)->pmt == bid, "B should take 3301");
    ASSERT_EQUAL(pmts[bid]->state, PMT_RUNNING, "B should run");
    ASSERT_EQUAL(pmts[aid]->state, PMT_RUNNING, "A should run on 3302");

    del_filter(bfid);
    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// A re-send the CA rejects with NORETRY (match lost on update)
// releases the stale registration instead of leaking it.
int test_resend_rejected_releases_ca() {
    int i;
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    opts.emulate_pids_all = 0;

    SCA_op failing_op = fake_ca_op;
    failing_op.ca_add_pmt = failing_ca_add_pmt;
    int ica = add_ca(&failing_op);
    ASSERT(ica >= 0, "could not register the fake CA");
    ad.ca_mask = 1 << ica;

    int id = pmt_add(0, 100, 48);
    ASSERT(id >= 0, "could not create the PMT");
    uint8_t priv[1] = {0};
    pmt_add_caid(pmts[id], 0x0B00, 0x0C00, priv, 0);
    pmt_add_stream_pid(pmts[id], 3301, 2, false, true);
    ASSERT(mark_pid_add(0, 0, 48) == 0, "pid 48 should be added");
    ASSERT(mark_pid_add(0, 0, 3301) == 0, "pid 3301 should be added");
    update_pids(0);
    pmt_add_active_pmt(&ad, id);

    fail_next_ca_add = 0;
    fake_ca_add_calls = fake_ca_del_calls = 0;
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should run");
    ASSERT(pmts[id]->ca_registered_mask != 0, "PMT should be registered");

    // update clears the send mask; the re-send is rejected this time
    pmts[id]->ca_mask = 0;
    fail_next_ca_add = 1;
    fake_ca_add_calls = fake_ca_del_calls = 0;
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should keep running");
    ASSERT_EQUAL(fake_ca_del_calls, 1, "stale registration should close");
    ASSERT_EQUAL(pmts[id]->ca_registered_mask, 0, "registration is gone");
    ASSERT(pmts[id]->disabled_ca_mask != 0, "CA should be disabled");

    del_ca(&failing_op);
    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// A PMT version update with identical content keeps the PMT running
// with its claims (#1346); stopping here was the glitch.
int test_version_update_keeps_claims() {
    int i;
    for (i = 0; i < MAX_PMT; i++)
        pmts[i] = NULL;
    npmts = 0;

    adapter ad = {};
    a[0] = &ad;
    ad.enabled = 1;
    ad.id = 0;
    opts.emulate_pids_all = 0;

    int id = pmt_add(0, 100, 48);
    ASSERT(id >= 0, "could not create the PMT");
    pmt_add_stream_pid(pmts[id], 3301, 2, false, true);
    pmt_add_stream_pid(pmts[id], 3401, 3, true, false);
    pmts[id]->pcr_pid = 3301;
    int fid = add_filter(0, 48, (void *)process_pmt, pmts[id], 0);
    ASSERT(fid >= 0, "could not add the PMT filter");
    pmts[id]->filter = fid;
    ad.active_pmts = 1;
    ad.active_pmt[0] = id;

    ASSERT(mark_pid_add(0, 0, 48) == 0, "pid 48 should be added");
    ASSERT(mark_pid_add(0, 0, 3301) == 0, "pid 3301 should be added");
    ASSERT(mark_pid_add(0, 0, 3401) == 0, "pid 3401 should be added");
    update_pids(0);
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should run");
    ASSERT(find_pid(0, 3301)->pmt == id, "PMT should own the video pid");

    uint8_t sec[26] = {0x02, 0xB0, 0x17, 0x00, 0x64, 0xC3, 0x00, 0x00, 0xEC,
                       0xE5, 0xF0, 0x00, 0x02, 0xEC, 0xE5, 0xF0, 0x00, 0x03,
                       0xED, 0x49, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00};
    copy32(sec, 22, crc_32(sec, 22));
    // First parse on a running PMT re-sends with update=1;
    // nothing is released.
    ASSERT(process_pmt(fid, sec, sizeof(sec), pmts[id]) == 0,
           "update to parse");
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should keep running");
    ASSERT_EQUAL(pmts[id]->version, 1, "version should advance");
    ASSERT(find_pid(0, 3301)->pmt == id, "video claim should be kept");
    ASSERT(find_pid(0, 3401)->pmt == id, "audio claim should be kept");

    // Same content, next version: still running after the re-send.
    uint8_t sec2[sizeof(sec)];
    memcpy(sec2, sec, sizeof(sec));
    sec2[5] = 0xC1 | (2 << 1);
    copy32(sec2, 22, crc_32(sec2, 22));
    ASSERT(process_pmt(fid, sec2, sizeof(sec2), pmts[id]) == 0,
           "same content to parse");
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should keep running");
    ASSERT_EQUAL(pmts[id]->version, 2, "version should advance");
    ASSERT(find_pid(0, 3301)->pmt == id, "video claim should be kept");

    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[id]->state, PMT_RUNNING, "PMT should still run");
    ASSERT(find_pid(0, 3301)->pmt == id, "video claim should stay kept");

    del_filter(fid);
    free_all_pmts();
    a[0] = NULL;
    return 0;
}

int main() {
    opts.log = 255;
    opts.debug = 255;
    strcpy(thread_info[thread_index].thread_name, "test_pmt");
    TEST_FUNC(test_descriptor_equality(),
              "testing descriptor equality operator");
    TEST_FUNC(test_descriptor_caid_capid_getters(),
              "testing descriptor getters");
    TEST_FUNC(test_wait_pusi(), "testing decrypt");
    TEST_FUNC(test_decrypt(), "testing decrypt");
    TEST_FUNC(test_assemble_packet(),
              "testing assemble_packet without adaptation");
    TEST_FUNC(test_assemble_packet_adaptation(),
              "testing assemble_packet with adaptation");
    TEST_FUNC(test_assemble_multi_packet(),
              "testing assemble_packet with multiple packets");
    TEST_FUNC(test_emulate_add_all_pids(),
              "testing test_emulate_add_all_pids failed")
    TEST_FUNC(test_pat_drop_releases_ca(),
              "testing that PMTs missing from the PAT release their CA slot")
    TEST_FUNC(test_revived_cached_pmt_gets_its_filter(),
              "testing that a revived cached PMT is given its new filter")
    TEST_FUNC(test_pmt_starts_only_with_pmt_pid(),
              "testing that the subscribed PMT pid selects the PMT")
    TEST_FUNC(test_retune_handover_same_loop(),
              "testing the single-pass retune handover with CA ordering")
    TEST_FUNC(test_single_slot_retune_releases_first(),
              "testing single-slot release before acquire on retune")
    TEST_FUNC(test_scan_pmt_only_starts_nothing(),
              "testing PMT-only scan subscriptions start nothing")
    TEST_FUNC(test_active_pmt_list_capped_at_max(),
              "testing the active PMT list refuses to overflow")
    TEST_FUNC(test_sticky_claims_without_parse(),
              "testing later order never steals from earlier holder")
    TEST_FUNC(test_earlier_newcomer_steals_by_order(),
              "testing earlier order steals from later holder")
    TEST_FUNC(test_steal_splits_partial_overlap(),
              "testing steal splits partial overlap, both run")
    TEST_FUNC(test_pid_order_reset_on_remove(),
              "testing SPid order assignment and reset")
    TEST_FUNC(test_d8_shared_pid(), "testing D8 shared pid dedupe and split")
    TEST_FUNC(test_shared_pid_split(),
              "testing same-pid different sets split streams")
    TEST_FUNC(test_30w_shared_pmt_pid(),
              "testing 30W pid 817 multi-service election")
    TEST_FUNC(test_1129_bein_shared_es(),
              "testing #1129 beIN shared streams elect subscribed pid")
    TEST_FUNC(test_19e_11493h_zap_flows(),
              "testing 19E 11493H replace-zap and add/del flows")
    TEST_FUNC(test_19e_11582h_group_and_disjoint(),
              "testing 19E 11582H identical pair, group and disjoint")
    TEST_FUNC(test_19e_11914h_ca_send_close(),
              "testing 19E 11914H CA send on zap and close on teardown")
    TEST_FUNC(test_ci_adapter_pmt_starts(),
              "testing that the CI adapter PMT starts")
    TEST_FUNC(test_1129_stingray_shared_vpid(),
              "testing #1129 Stingray shared VPID follows subscription")
    TEST_FUNC(test_held_pids_without_client_stop_pmt(),
              "testing stop when pids are held without a client")
    TEST_FUNC(test_cw_keyed_by_pmt(), "testing direct CW to PMT mapping")
    TEST_FUNC(test_pids_all_expands_pmt_pids(),
              "testing pids=all PMT pid expansion")
    TEST_FUNC(test_multi_service_pid_parses_per_sid(),
              "testing per-sid PMT objects on a shared PMT pid")
    TEST_FUNC(test_late_parse_handover(),
              "testing handover when the earlier PMT parses late")
    TEST_FUNC(test_running_pmt_pid_deleted_on_unsubscribe(),
              "testing demux release on last PMT pid unsubscribe")
    TEST_FUNC(test_pmt_pid_remove_readd_no_churn(),
              "testing no churn on PMT pid remove and re-add")
    TEST_FUNC(test_pmt_pid_delete_hands_over(),
              "testing handover when the PMT pid is deleted")
    TEST_FUNC(test_update_pids_tail_elects(),
              "testing tail election with the CA send on the loop")
    TEST_FUNC(test_stream_pid_delete_stops_pmt(),
              "testing stop when the streams are deleted")
    TEST_FUNC(test_version_update_keeps_claims(),
              "testing claims survive a PMT version update")
    TEST_FUNC(test_pmt_cosmetic_update_keeps_running(),
              "testing cosmetic PMT update keeps running with a re-send")
    TEST_FUNC(test_pmt_content_update_delta(),
              "testing PMT content update keeps running with a re-send")
    TEST_FUNC(test_ca_update_flag_distinguishes_resend(),
              "testing the CA add call flags updates vs first adds")
    TEST_FUNC(test_handover_ignores_stale_holder(),
              "testing handover skips holders that dropped the pid")
    TEST_FUNC(test_resend_rejected_releases_ca(),
              "testing a rejected re-send releases the CA slot")
    fflush(stdout);
    return 0;
}
