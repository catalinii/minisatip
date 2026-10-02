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

// ---------------------------------------------------------------------------
// Regression test for the CA channel-slot leak fixed in this commit.
//
// process_pat() resets ad->active_pmts and rebuilds it from the PAT it has
// just parsed, so a PMT that was active before and is absent from the new PAT
// is already out of ad->active_pmt[] when the function returns.
// start_active_pmts() only walks that array, so it never stops such a PMT and
// close_pmt_for_cas() is never reached: the PMT keeps its CA registration for
// ever. As a CAM supports one channel by default, one leaked registration is
// enough to stop the next service from being descrambled.
//
// The retirement used to run only when the PAT version changed on an already
// processed PAT. This test covers the other case: the first PAT after the
// adapter was (re)initialised, when ad->pat_processed is still 0, which is
// what a CI adapter does on every channel change.
// ---------------------------------------------------------------------------

extern int npmts;
extern int process_pat(int filter, unsigned char *b, int len, void *opaque);

static int fake_ca_del_calls;
static int fake_ca_last_del_pmt;

static int fake_ca_add_pmt(adapter *ad, SPMT *pmt) { return TABLES_RESULT_OK; }

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

    // Earlier tests in this binary leave SPMT objects with automatic storage
    // duration in the global pmts[] array, so start from a known state. The
    // entries are only detached, never freed, as some of them are not ours.
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

// A PMT that disappeared from the PAT is cached: it keeps its stream pids,
// its CA descriptors and its version, but its filter is deleted. When the sid
// shows up in the PAT again, process_pat() creates a fresh filter for the PMT
// pid and used to throw the id away, so the PMT stayed on filter -1 until a
// section happened to arrive. start_active_pmts() started it anyway, and
// start_pmt() then called set_filter_flags(-1), which fails before it adds the
// pid: the PMT was RUNNING, the CAs had a CA_PMT, and the PMT pid was not in
// the demux. A second PAT would also add a second filter for the same PMT.
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

    ASSERT_EQUAL(pmt->state, PMT_STOPPED, "the revived PMT should be stopped");
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

    // with a real filter the PMT starts from cache on this pass, and
    // set_filter_flags() can put the PMT pid back in the demux
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmt->state, PMT_RUNNING,
                 "the revived PMT should start straight from the cache");
    ASSERT(filters[pmt->filter]->flags != 0,
           "starting the PMT should have enabled its filter");

    del_filter(ad.pat_filter);
    free_all_pmts();
    free_filters();
    a[0] = NULL;
    return 0;
}

// The active PMT is the one in the pid list (#1445): with two services
// sharing every elementary pid, only the sibling whose PMT pid is
// subscribed starts; with no PMT pid subscribed nothing starts (FTA).
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

    // drop the PMT pid: ES-only playlist must not decrypt anything
    find_pid(0, 52)->sid.clear();
    find_pid(0, 52)->order = 0;
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[bid]->state, PMT_STOPPED, "PMT should stop");
    ASSERT(find_pid(0, 3301)->pmt == -1, "video claim should be released");
    ASSERT(find_pid(0, 3401)->pmt == -1, "audio claim should be released");

    free_all_pmts();
    a[0] = NULL;
    return 0;
}

// Retune A->B completes in one pass with no sections: A stops and
// releases, B claims and starts, and the CA handover is ordered.
static int fake_ca_add_calls;
static int fake_ca_last_add_pmt;

static int counting_ca_add_pmt(adapter *ad, SPMT *pmt) {
    fake_ca_add_calls++;
    fake_ca_last_add_pmt = pmt->id;
    return fake_ca_add_pmt(ad, pmt);
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

// Contention follows subscription order, not parse order: a PMT whose
// PMT pid was subscribed earlier preempts a running larger-order holder,
// while a later-subscribed newcomer waits without flapping.
int test_smaller_order_preempts() {
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
    // both PMT pids subscribed, A earlier; B parsed (and started) first
    int pids[] = {48, 52, 3301, 3401};
    int orders[] = {3, 5, 6, 7};
    for (i = 0; i < 4; i++) {
        ad.pids[i].pid = pids[i];
        ad.pids[i].flags = PID_STATE_ACTIVE;
        ad.pids[i].pmt = -1;
        ad.pids[i].filter = -1;
        ad.pids[i].sid.insert(i < 2 ? i : 0);
        if (i >= 2)
            ad.pids[i].sid.insert(1);
        ad.pids[i].order = orders[i];
    }
    ad.active_pmts = 1;
    ad.active_pmt[0] = bid;
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[bid]->state, PMT_RUNNING, "B should run alone");

    // A parses late but was subscribed first: it takes over in one pass
    ad.active_pmts = 2;
    ad.active_pmt[1] = aid;
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[aid]->state, PMT_RUNNING, "A should preempt");
    ASSERT_EQUAL(pmts[bid]->state, PMT_STOPPED, "B should be demoted");
    ASSERT(find_pid(0, 3301)->pmt == aid, "video pid should move to A");

    // steady state: no flapping between the two passes
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[aid]->state, PMT_RUNNING, "A should keep running");
    ASSERT_EQUAL(pmts[bid]->state, PMT_STOPPED, "B should keep waiting");

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

// D8: several SIDs on one PMT pid. The candidate whose ES set covers
// the requested pids runs; identical ES sets run the lowest sid.
int test_d8_subset_match() {
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
    ASSERT_EQUAL(pmts[m2]->state, PMT_RUNNING, "covering PMT should run");
    ASSERT_EQUAL(pmts[m1]->state, PMT_STOPPED, "subset PMT should wait");
    ASSERT_EQUAL(pmts[m3]->state, PMT_RUNNING, "lowest sid should run");
    ASSERT_EQUAL(pmts[m4]->state, PMT_STOPPED, "higher sid should wait");

    // request the other audio: the choice hands over to the sibling
    find_pid(0, 712)->sid.clear();
    find_pid(0, 712)->order = 0;
    ad.pids[6].pid = 711;
    ad.pids[6].flags = PID_STATE_ACTIVE;
    ad.pids[6].pmt = -1;
    ad.pids[6].filter = -1;
    ad.pids[6].sid.insert(0);
    ad.pids[6].order = 7;
    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[m1]->state, PMT_RUNNING, "new choice should run");
    ASSERT_EQUAL(pmts[m2]->state, PMT_STOPPED, "old choice should stop");

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

// Without ADD_REMOVE on the running filter, the last unsubscribe deletes
// the PMT pid from the demux at once; the PMT stops on the next pass.
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
    ASSERT_EQUAL(pmts[id]->state, PMT_STOPPED, "PMT should stop on pid delete");

    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[id]->state, PMT_STOPPED, "PMT should stay stopped");

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
    ASSERT(find_pid(0, 3301)->pmt == -1, "video claim should be released");
    ASSERT(find_pid(0, 48) == NULL, "pid 48 should leave the demux");

    start_active_pmts(&ad);
    ASSERT_EQUAL(pmts[bid]->state, PMT_RUNNING, "B should take over");
    ASSERT(find_pid(0, 3301)->pmt == bid, "B should own the video pid");

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
    TEST_FUNC(test_smaller_order_preempts(),
              "testing subscription-order preemption without flapping")
    TEST_FUNC(test_pid_order_reset_on_remove(),
              "testing SPid order assignment and reset")
    TEST_FUNC(test_d8_subset_match(),
              "testing multi-service PMT pid subset selection")
    TEST_FUNC(test_cw_keyed_by_pmt(), "testing direct CW to PMT mapping")
    TEST_FUNC(test_pids_all_expands_pmt_pids(),
              "testing pids=all PMT pid expansion")
    TEST_FUNC(test_multi_service_pid_parses_per_sid(),
              "testing per-sid PMT objects on a shared PMT pid")
    TEST_FUNC(test_late_parse_handover(),
              "testing handover when the earlier PMT parses late")
    TEST_FUNC(test_running_pmt_pid_deleted_on_unsubscribe(),
              "testing demux release on last PMT pid unsubscribe")
    TEST_FUNC(test_pmt_pid_delete_hands_over(),
              "testing handover when the PMT pid is deleted")
    TEST_FUNC(test_stream_pid_delete_stops_pmt(),
              "testing stop when the streams are deleted")
    fflush(stdout);
    return 0;
}
