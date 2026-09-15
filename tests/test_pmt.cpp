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
#include "utils.h"
#include "utils/testing.h"
#include "utils/ticks.h"
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

// A broadcast PMT: CA descriptors on the program level and on the audio
// stream when ca is set, a private descriptor, a video, an audio and a teletext
// stream with their descriptors.
static int build_broadcast_pmt(uint8_t *s, int sid, int version, int cni,
                               int ca) {
    static const uint8_t pca[] = {0x09, 0x04, 0x09, 0x8C, 0xE0, 0xCC};
    static const uint8_t priv[] = {0x5F, 0x04, 0x00, 0x00, 0x00, 0x2A};
    static const uint8_t aca[] = {0x09, 0x04, 0x09, 0x8D, 0xE0, 0xCD};
    static const uint8_t lang[] = {0x0A, 0x04, 'd', 'e', 'u', 0x00};
    static const uint8_t ttx[] = {0x56, 0x05, 'd',  'e', 'u',
                                  0x10, 0x00, 0x52, 0x01, 0x03};
    uint8_t *b = s, *l;
    auto put = [&](const uint8_t *d, int n) {
        memcpy(b, d, n);
        b += n;
    };
    auto stream = [&](int type, int pid) {
        *b++ = type;
        copy16(b, 0, 0xE000 | pid);
        b += 2;
        l = b;
        b += 2;
    };
    auto close = [&]() { copy16(l, 0, 0xF000 | (b - l - 2)); };

    *b++ = 0x02;
    b += 2; // section_length
    copy16(b, 0, sid);
    b += 2;
    *b++ = 0xC0 | ((version & 0x1F) << 1) | (cni ? 1 : 0);
    *b++ = 0; // section number
    *b++ = 0; // last section number
    copy16(b, 0, 0xE000 | 1279);
    b += 2;
    l = b;
    b += 2;
    if (ca)
        put(pca, sizeof(pca));
    put(priv, sizeof(priv));
    close();
    stream(27, 1279);
    close();
    stream(3, 1283);
    if (ca)
        put(aca, sizeof(aca));
    put(lang, sizeof(lang));
    close();
    stream(6, 34);
    put(ttx, sizeof(ttx));
    close();

    copy16(s, 1, 0xB000 | ((b - s) + 4 - 3));
    copy32(b, 0, crc_32(s, b - s));
    return (b - s) + 4;
}

// The rewrite is the broadcast section minus its CA descriptors: every stream
// and every other descriptor stays, byte for byte and in its order.
int test_clean_pmt() {
    SPMT pmt = {};
    uint8_t want[256], *sec;
    int slen, wlen;

    // Sized exactly, so that a read past the section is caught
    slen = build_broadcast_pmt(want, 0x0083, 5, 1, 1);
    sec = (uint8_t *)malloc(slen);

    build_broadcast_pmt(sec, 0x0083, 5, 1, 1);
    wlen = build_broadcast_pmt(want, 0x0083, 6, 1, 0);
    pmt_clean_build(&pmt, sec, slen);
    ASSERT((int)pmt.clean.size() == wlen &&
               !memcmp(pmt.clean.data(), want, wlen),
           "the rewrite is not the broadcast without its CA descriptors");
    ASSERT(!memcmp(pmt.clean_hdr, sec, sizeof(pmt.clean_hdr)),
           "the header of the broadcast section was not kept");

    // The version is five bits wide and wraps
    build_broadcast_pmt(sec, 0x0083, 31, 1, 1);
    wlen = build_broadcast_pmt(want, 0x0083, 0, 1, 0);
    pmt_clean_build(&pmt, sec, slen);
    ASSERT((int)pmt.clean.size() == wlen &&
               !memcmp(pmt.clean.data(), want, wlen),
           "the version did not wrap");

    // Nothing to strip, or a section that is not on the wire yet
    pmt_clean_build(&pmt, sec, build_broadcast_pmt(sec, 0x0083, 5, 1, 0));
    ASSERT(pmt.clean.empty(), "a free to air PMT was rewritten");
    build_broadcast_pmt(sec, 0x0083, 5, 0, 1);
    pmt_clean_build(&pmt, sec, slen);
    ASSERT(pmt.clean.empty(), "a section that is not current was rewritten");

    // A descriptor that runs past its loop leaves nothing half built
    build_broadcast_pmt(sec, 0x0083, 5, 1, 1);
    sec[13] = 0xFF;
    pmt_clean_build(&pmt, sec, slen);
    ASSERT(pmt.clean.empty(), "a broken descriptor loop was rewritten");

    free(sec);
    return 0;
}

// Wrap a section into one TS packet. pusi 0 makes it a continuation packet,
// adaptation shrinks the payload to that many bytes.
static void build_ts(uint8_t *p, int pid, int pusi, const uint8_t *payload,
                     int len, int payload_room) {
    int pay = 4;
    memset(p, 0xFF, DVB_FRAME);
    p[0] = 0x47;
    p[1] = (pusi ? 0x40 : 0) | ((pid >> 8) & 0x1F);
    p[2] = pid & 0xFF;
    p[3] = 0x10;
    if (payload_room > 0 && payload_room < DVB_FRAME - 4) {
        int af = DVB_FRAME - 4 - payload_room - 1;
        p[3] |= 0x20;
        p[4] = af;
        if (af > 0) {
            p[5] = 0;
            memset(p + 6, 0xFF, af - 1);
        }
        pay = 5 + af;
    }
    if (pusi)
        p[pay++] = 0; // pointer field
    if (len > DVB_FRAME - pay)
        len = DVB_FRAME - pay;
    if (len > 0)
        memcpy(p + pay, payload, len);
}

// The real reset, plus the proof of descrambling that puts the PMT into
// CLEAN_WRITE. Anything the rewriter then leaves alone, it leaves alone on
// purpose.
static void arm_clean(SPMT *pmt) {
    pmt_clean_reset(pmt);
    pmt->ever_decrypted = 1;
}

// Set up an adapter whose buffer holds exactly npkt packets, so that a read or
// a write past the buffer is caught.
static adapter *clean_adapter(int npkt) {
    adapter *ad = adapter_alloc();
    free(ad->buf);
    ad->buf = (uint8_t *)malloc(npkt * DVB_FRAME);
    ad->rlen = npkt * DVB_FRAME;
    a[0] = ad;
    ad->id = 0;
    ad->enabled = 1;
    return ad;
}

// The service under test, descrambled, with CA descriptors on the program
// level and on the audio stream.
static void clean_pmt_setup(SPMT *pmt, adapter *ad) {
    uint8_t sec[256];
    pmts[0] = pmt;
    npmts = 1; // get_pmt() refuses anything at or above this
    pmt->id = 0;
    pmt->enabled = 1;
    pmt->adapter = 0;
    pmt->master_pmt = 0;
    pmt->sid = 0x0083;
    pmt->pid = 100;
    pmt->pcr_pid = 1279;
    pmt->version = 5;
    pmt->state = PMT_RUNNING;
    pmt->ever_decrypted = 1;
    pmt_clean_build(pmt, sec, build_broadcast_pmt(sec, 0x0083, 5, 1, 1));
    ad->active_pmts = 1;
    ad->active_pmt[0] = 0;
    mark_pid_add(0, ad->id, 100);
    update_pids(ad->id);
}

static void clean_teardown(adapter *ad) {
    ad->active_pmts = 0;
    pmts[0] = NULL;
    pmts[1] = NULL;
    npmts = 0;
    free(ad->buf);
    delete ad;
    a[0] = NULL;
}

// What a client is handed for packet idx of the buffer. pos is the client's
// cursor, so a caller that walks the whole buffer keeps it across the calls.
static uint8_t *clean_get(adapter *ad, int idx, int in_grace, int *pos) {
    return pmt_clean_packet(ad, idx, ad->buf + idx * DVB_FRAME, in_grace, pos);
}

// The single packet in the buffer, as a client with an open window sees it.
static uint8_t *clean_one(adapter *ad) {
    int pos = 0;
    pmt_clean_prepare(ad, 1);
    return clean_get(ad, 0, 1, &pos);
}

// pmt_clean_prepare() decides per packet; these are the shapes it must not
// touch. Whatever it decides, the adapter buffer stays as it was read: it is
// shared with every other client and with the CA layer.
int test_clean_psi_packets() {
    adapter *ad = clean_adapter(1);
    SPMT pmt = {}, other = {};
    uint8_t sec[512], before[DVB_FRAME], *out;
    int slen, saved_clean = opts.clean_psi;
    int saved_grace = opts.clean_psi_grace;

    clean_pmt_setup(&pmt, ad);
    opts.clean_psi = 1;
    opts.clean_psi_grace = CLEAN_PSI_GRACE;

    slen = build_broadcast_pmt(sec, pmt.sid, pmt.version, 1, 1);

    // 1. A continuation packet arriving first must be passed on: clean_off
    //    starts at -1, so nothing may be written into the middle of a section
    arm_clean(&pmt);
    build_ts(ad->buf, 100, 0, sec, slen, 0);
    ASSERT(clean_one(ad) == ad->buf, "a continuation packet was rewritten");

    // 2. The real thing is rewritten, and the CA descriptors are gone
    arm_clean(&pmt);
    build_ts(ad->buf, 100, 1, sec, slen, 0);
    memcpy(before, ad->buf, DVB_FRAME);
    out = clean_one(ad);
    ASSERT(out && out != ad->buf, "the PMT of a descrambled service was kept");
    ASSERT(!memcmp(before, ad->buf, DVB_FRAME),
           "the rewrite changed the adapter buffer");
    ASSERT(!memcmp(before, out, 4), "the rewrite changed the TS header");
    // payload at 4, pointer field 0, so the section starts at 5 and its
    // version byte is the sixth byte of the section
    ASSERT(out[4] == 0 && out[5] == 0x02,
           "the rewritten packet lost its pointer field or table id");
    ASSERT(((out[8] << 8) | out[9]) == pmt.sid,
           "the rewritten section changed the service id");
    ASSERT(((out[10] & 0x3E) >> 1) == ((pmt.version + 1) & 0x1F),
           "the rewritten section kept the broadcast version");
    ASSERT(!memcmp(out + 5, pmt.clean.data(), pmt.clean.size()),
           "the packet does not carry the section without CA descriptors");
    for (int i = 5 + pmt.clean.size(); i < DVB_FRAME; i++)
        ASSERT(out[i] == 0xFF, "the freed bytes were not stuffed");

    // 3. Another service on the pid must not be replaced with ours
    arm_clean(&pmt);
    slen = build_broadcast_pmt(sec, 0x0084, pmt.version, 1, 1);
    build_ts(ad->buf, 100, 1, sec, slen, 0);
    ASSERT(clean_one(ad) == ad->buf, "a foreign service id was rewritten");

    // 4. A version that was never parsed, and a not-yet-current section
    arm_clean(&pmt);
    slen = build_broadcast_pmt(sec, pmt.sid, pmt.version + 1, 1, 1);
    build_ts(ad->buf, 100, 1, sec, slen, 0);
    ASSERT(clean_one(ad) == ad->buf, "an unparsed version was rewritten");

    arm_clean(&pmt);
    slen = build_broadcast_pmt(sec, pmt.sid, pmt.version, 0, 1);
    build_ts(ad->buf, 100, 1, sec, slen, 0);
    ASSERT(clean_one(ad) == ad->buf,
           "a current_next_indicator of 0 was made current");

    // 5. A second section packed behind ours would be stuffed over
    arm_clean(&pmt);
    slen = build_broadcast_pmt(sec, pmt.sid, pmt.version, 1, 1);
    sec[slen] = 0x02; // a further table starts right here
    sec[slen + 1] = 0xB0;
    build_ts(ad->buf, 100, 1, sec, slen + 2, 0);
    ASSERT(clean_one(ad) == ad->buf,
           "a section packed behind the PMT was overwritten");

    // 6. A payload too short to hold a PMT header: the diagnostics must not
    //    read past the end of the packet
    arm_clean(&pmt);
    build_ts(ad->buf, 100, 1, sec, 4, 6);
    ASSERT(clean_one(ad) == ad->buf, "a truncated PSI header was rewritten");

    // 7. Two services on one PMT pid: neither is ours to replace
    arm_clean(&pmt);
    slen = build_broadcast_pmt(sec, pmt.sid, pmt.version, 1, 1);
    build_ts(ad->buf, 100, 1, sec, slen, 0);
    other = pmt;
    other.id = 1;
    other.sid = 0x0084;
    pmts[1] = &other;
    npmts = 2;
    ad->active_pmts = 2;
    ad->active_pmt[1] = 1;
    ASSERT(clean_one(ad) == ad->buf,
           "a pid carrying two services was rewritten");
    ad->active_pmts = 1;
    pmts[1] = NULL;
    npmts = 1;

    // 8. With the option off there is nothing to hand a client
    arm_clean(&pmt);
    opts.clean_psi = 0;
    build_ts(ad->buf, 100, 1, sec, slen, 0);
    ASSERT(clean_one(ad) == ad->buf && ad->clean_psi_packets == 0,
           "--clean-psi is off but a packet was replaced");

    opts.clean_psi = saved_clean;
    opts.clean_psi_grace = saved_grace;
    clean_teardown(ad);
    return 0;
}

// A PMT that does not end on a packet boundary: the packet it ends in carries
// a pointer field and the start of the next section behind it.
int test_clean_psi_spanning_section() {
    adapter *ad = clean_adapter(3);
    SPMT pmt = {};
    uint8_t sec[512], before[3 * DVB_FRAME], asmb[512], *first, *last;
    int i, slen, head, tail, olen, pos = 0;
    int saved_clean = opts.clean_psi, saved_grace = opts.clean_psi_grace;
    uint32_t crc;

    clean_pmt_setup(&pmt, ad);
    opts.clean_psi = 1;
    opts.clean_psi_grace = CLEAN_PSI_GRACE;
    arm_clean(&pmt);

    // 20 payload bytes in the first packet: the pointer field and 19 of the
    // section, so the rest of it ends inside the second one
    slen = build_broadcast_pmt(sec, pmt.sid, pmt.version, 1, 1);
    head = 19;
    tail = slen - head;
    ASSERT(tail > 0 && tail < DVB_FRAME - 6,
           "the test section does not end inside the second packet");
    build_ts(ad->buf, 100, 1, sec, slen, head + 1);
    last = ad->buf + DVB_FRAME;
    build_ts(last, 100, 1, sec + head, tail, 0);
    last[4] = (uint8_t)tail; // pointer field: the rest of the PMT comes first
    last[5 + tail] = 0x42;   // and a table of whoever sent it behind that
    last[6 + tail] = 0xF0;
    // that table continues in the packet after it, which is none of ours
    build_ts(ad->buf + 2 * DVB_FRAME, 100, 0, sec, slen, 0);
    memcpy(before, ad->buf, sizeof(before));

    pmt_clean_prepare(ad, 0);
    first = clean_get(ad, 0, 1, &pos);
    ASSERT(first && first != ad->buf, "the start of the PMT was not rewritten");
    last = clean_get(ad, 1, 1, &pos);
    ASSERT(last && last != ad->buf + DVB_FRAME,
           "the end of the PMT reached the client as broadcast");
    ASSERT(clean_get(ad, 2, 1, &pos) == ad->buf + 2 * DVB_FRAME,
           "the section behind the PMT was written into");

    // The TS header, the pointer field and the table behind it are the
    // sender's; only the bytes the PMT occupied are ours
    ASSERT(!memcmp(last, before + DVB_FRAME, 5),
           "the TS header or the pointer field of the last packet changed");
    ASSERT(!memcmp(last + 5 + tail, before + DVB_FRAME + 5 + tail,
                   DVB_FRAME - 5 - tail),
           "the section behind the pointer field was overwritten");

    // What the client reassembles has to be one PMT, not half of two
    ASSERT(first[DVB_FRAME - head - 1] == 0,
           "the rewrite did not start at a zero pointer field");
    memcpy(asmb, first + DVB_FRAME - head, head);
    memcpy(asmb + head, last + 5, tail);
    olen = 3 + (((asmb[1] & 0x0F) << 8) | asmb[2]);
    ASSERT(asmb[0] == 0x02 && olen >= 4 && olen <= head + tail,
           "the reassembled section is not a PMT");
    crc = ((uint32_t)asmb[olen - 4] << 24) | (asmb[olen - 3] << 16) |
          (asmb[olen - 2] << 8) | asmb[olen - 1];
    ASSERT(crc_32(asmb, olen - 4) == crc,
           "the reassembled section fails its CRC");
    for (i = 12; i < olen - 4; i++)
        ASSERT(!(asmb[i] == 0x09 && asmb[i + 1] == 0x04),
               "a CA descriptor survived the rewrite");
    ASSERT(!memcmp(before, ad->buf, sizeof(before)),
           "the adapter buffer was modified");

    // A pointer field that runs past the payload is not one to write into
    arm_clean(&pmt);
    ad->buf[DVB_FRAME + 4] = DVB_FRAME;
    pmt_clean_prepare(ad, 0);
    pos = 0;
    ASSERT(clean_get(ad, 1, 1, &pos) == ad->buf + DVB_FRAME,
           "a pointer field past the payload was written into");
    ad->buf[DVB_FRAME + 4] = (uint8_t)tail;

    // And neither is one that arrives with no section of ours in progress
    arm_clean(&pmt);
    ad->buf[1] = 0; // the first packet is now a pid nobody asked about
    ad->buf[2] = 200;
    pmt_clean_prepare(ad, 0);
    pos = 0;
    ASSERT(clean_get(ad, 1, 1, &pos) == ad->buf + DVB_FRAME,
           "a pointer field was followed with no section to finish");

    opts.clean_psi = saved_clean;
    opts.clean_psi_grace = saved_grace;
    clean_teardown(ad);
    return 0;
}

// The verdict is the adapter's, the waiting is the client's: two clients read
// the same buffer and only the one still inside its window holds the pid back.
int test_clean_psi_streams() {
    adapter *ad = clean_adapter(3);
    SPMT pmt = {};
    uint8_t sec[512], before[3 * DVB_FRAME], *out;
    int slen, pos, saved_clean = opts.clean_psi;
    int saved_grace = opts.clean_psi_grace;

    clean_pmt_setup(&pmt, ad);
    opts.clean_psi = 1;
    opts.clean_psi_grace = CLEAN_PSI_GRACE;
    slen = build_broadcast_pmt(sec, pmt.sid, pmt.version, 1, 1);

    // The PMT of the descrambled service, a video packet, and the PMT of a
    // service nothing is known about yet
    build_ts(ad->buf, 100, 1, sec, slen, 0);
    build_ts(ad->buf + DVB_FRAME, 1279, 0, sec, 0, 0);
    build_ts(ad->buf + 2 * DVB_FRAME, 200, 1, sec, slen, 0);
    memcpy(before, ad->buf, sizeof(before));

    // 1. A client inside its window: the rewrite, the video untouched, and
    //    nothing at all for the service that has no verdict
    arm_clean(&pmt);
    pmt_clean_prepare(ad, 1);
    pos = 0;
    out = clean_get(ad, 0, 1, &pos);
    ASSERT(out && out != ad->buf, "the descrambled PMT was not rewritten");
    ASSERT(clean_get(ad, 1, 1, &pos) == ad->buf + DVB_FRAME,
           "a video packet was replaced");
    ASSERT(clean_get(ad, 2, 1, &pos) == NULL,
           "an unclassified PMT was handed to a waiting client");

    // 2. A client past its window sees the same rewrite, but is no longer
    //    willing to wait for the service that has no verdict
    pos = 0;
    ASSERT(clean_get(ad, 0, 0, &pos) == out,
           "the rewrite depends on the client");
    ASSERT(clean_get(ad, 2, 0, &pos) == ad->buf + 2 * DVB_FRAME,
           "an unclassified PMT was withheld after the window closed");

    // 3. Without a client waiting, an unknown PMT pid is not looked at
    arm_clean(&pmt);
    pmt_clean_prepare(ad, 0);
    pos = 0;
    ASSERT(clean_get(ad, 2, 1, &pos) == ad->buf + 2 * DVB_FRAME,
           "an unknown PMT pid was held with no client waiting");

    // 4. An encrypted service that nothing has decrypted is held, never
    //    announced as free to air
    pmt_clean_reset(&pmt); // no ever_decrypted this time
    pmt_clean_prepare(ad, 0);
    pos = 0;
    ASSERT(clean_get(ad, 0, 1, &pos) == NULL,
           "an undecrypted service was passed on to a waiting client");
    pos = 0;
    ASSERT(clean_get(ad, 0, 0, &pos) == ad->buf,
           "an undecrypted service was still held after the window closed");

    ASSERT(!memcmp(before, ad->buf, sizeof(before)),
           "the adapter buffer was modified");

    opts.clean_psi = saved_clean;
    opts.clean_psi_grace = saved_grace;
    clean_teardown(ad);
    return 0;
}

// A service can reach us already descrambled, from an upstream SAT>IP server
// or a CAM, and still carry the CA descriptors of the broadcast. No control
// word is validated here then, so the rewrite is gated on the stream itself
// arriving unscrambled.
int test_clean_psi_arrives_clear() {
    adapter *ad = clean_adapter(2);
    SPMT pmt = {};
    uint8_t sec[512], *v;
    int slen, pos, i, saved_clean = opts.clean_psi;
    int saved_grace = opts.clean_psi_grace;
    SPid *p;

    clean_pmt_setup(&pmt, ad);
    opts.clean_psi = 1;
    opts.clean_psi_grace = CLEAN_PSI_GRACE;
    slen = build_broadcast_pmt(sec, pmt.sid, pmt.version, 1, 1);
    build_ts(ad->buf, 100, 1, sec, slen, 0);
    v = ad->buf + DVB_FRAME;
    build_ts(v, 1279, 0, sec, 0, 0);

    mark_pid_add(0, ad->id, 1279);
    update_pids(ad->id);
    p = find_pid(ad->id, 1279);
    ASSERT(p != NULL, "no pid entry for the video stream");
    p->pmt = pmt.id;

    // 1. Nothing has decrypted this service and nothing has been seen of it:
    //    the pid is withheld, not announced free to air
    pmt_clean_reset(&pmt);
    pmt_clean_prepare(ad, 0);
    pos = 0;
    ASSERT(clean_get(ad, 0, 1, &pos) == NULL,
           "a service with no verdict was handed to a waiting client");

    // 2. A packet without payload is clear in a scrambled service too
    v[3] = 0x20;
    for (i = 0; i < 2 * CLEAN_PSI_CLEAR_PACKETS; i++)
        pmt_clean_count_clear(ad, v, p);
    ASSERT(!pmt.arrives_clear,
           "packets without payload were counted as a clear stream");

    // 3. Nor does a stream that is not the one carrying the PCR: a service
    //    may broadcast its teletext in the clear and scramble the rest
    mark_pid_add(0, ad->id, 1283);
    update_pids(ad->id);
    SPid *tt = find_pid(ad->id, 1283);
    ASSERT(tt != NULL, "no pid entry for the teletext stream");
    tt->pmt = pmt.id;
    v[3] = 0x10;
    for (i = 0; i < 2 * CLEAN_PSI_CLEAR_PACKETS; i++)
        pmt_clean_count_clear(ad, v, tt);
    ASSERT(!pmt.arrives_clear,
           "a clear stream that is not the PCR one decided the service");
    mark_pid_deleted(ad->id, 0, 1283, NULL);
    update_pids(ad->id);

    // 4. One scrambled payload packet ends the run
    v[3] = 0x10;
    for (i = 0; i < CLEAN_PSI_CLEAR_PACKETS - 1; i++)
        pmt_clean_count_clear(ad, v, p);
    v[3] = 0x90;
    pmt_clean_count_clear(ad, v, p);
    ASSERT(pmt.clear_run == 0, "a scrambled packet did not end the run");
    v[3] = 0x10;
    for (i = 0; i < CLEAN_PSI_CLEAR_PACKETS - 1; i++)
        pmt_clean_count_clear(ad, v, p);
    ASSERT(!pmt.arrives_clear, "the run was not counted from the start again");

    // 5. The packet that completes the run decides it, and the PMT is rebuilt
    //    without a control word ever having been validated
    pmt_clean_count_clear(ad, v, p);
    ASSERT(pmt.arrives_clear, "a stream that arrives in the clear was missed");
    ASSERT(!pmt.ever_decrypted, "nothing decrypted this service here");
    pmt_clean_prepare(ad, 0);
    pos = 0;
    ASSERT(clean_get(ad, 0, 1, &pos) != NULL &&
               clean_get(ad, 0, 1, &pos) != ad->buf,
           "the PMT of a service that arrives in the clear was not rewritten");

    opts.clean_psi = saved_clean;
    opts.clean_psi_grace = saved_grace;
    mark_pid_deleted(ad->id, 0, 1279, NULL);
    update_pids(ad->id);
    clean_teardown(ad);
    return 0;
}

int main() {
    opts.log = 255;
    opts.debug = 255;
    strcpy(thread_info[thread_index].thread_name, "test_pmt");
    TEST_FUNC(test_clean_psi_packets(),
              "testing which packets pmt_clean_prepare rewrites");
    TEST_FUNC(test_clean_psi_streams(),
              "testing the PMT each client is handed");
    TEST_FUNC(test_clean_psi_arrives_clear(),
              "testing a service that arrives already descrambled");
    TEST_FUNC(test_clean_psi_spanning_section(),
              "testing a PMT that ends inside a packet with a pointer field");
    TEST_FUNC(test_clean_pmt(), "testing PMT rewrite without CA descriptors");
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
    fflush(stdout);
    return 0;
}
