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
#include "utils/ticks.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
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
extern SPMT *pmts[MAX_PMT];
void remove_pmt_from_device(ca_device_t *d, SPMT *pmt);
SCAPMT *add_pmt_to_capmt(ca_device_t *d, SPMT *pmt, int multiple);
int dvbca_del_pmt(adapter *ad, SPMT *spmt);
extern ca_device_t *ca_devices[MAX_ADAPTERS];
extern int dvbca_id;
int get_active_capmts(ca_device_t *d);
int dvbca_init_dev(adapter *ad);
int ca_timeout(sockets *s);
int ca_read_tpdu(int socket, void *buf, int buf_len, sockets *ss, int *rb);
int ca_read_enigma(int socket, void *buf, int len, sockets *ss, int *rb);
int get_enabled_pmts_for_ca(ca_device_t *d);

ca_device_t d;

int test_multiple_pmt() {
    // adapter, sid, pmt_pid
    pmt_add(0, 100, 100);
    pmt_add(0, 200, 200);
    pmt_add(0, 300, 300);
    pmt_add(0, 400, 400);

    add_pmt_to_capmt(&d, get_pmt(0), 1);
    add_pmt_to_capmt(&d, get_pmt(1), 1);
    ASSERT(get_active_capmts(&d) == 1, "expected number of capmt ahould be 1");
    remove_pmt_from_device(&d, get_pmt(0));
    ASSERT(get_enabled_pmts_for_ca(&d) == 1, "expected capmt should be 1");
    ASSERT(PMT_ID_IS_VALID(d.capmt[0].pmt_id) &&
               !PMT_ID_IS_VALID(d.capmt[0].other_id),
           "only first PMT should be active");

    ASSERT(add_pmt_to_capmt(&d, get_pmt(2), 0) == NULL, "failed adding PMT");

    return 0;
}

// Releasing a service is a CA_PMT for the same program with the command id
// set to not_selected, not a missing message.
int test_create_capmt_not_selected() {
    int pmt_id = pmt_add(0, 0x100, 0x101);
    SPMT *pmt = get_pmt(pmt_id);
    pmt_add_caid(pmt, 0x0B00, 0x573, nullptr, 0);
    pmt_add_stream_pid(pmt, 0x501, 2, false, true);
    pmt_add_stream_pid(pmt, 0x502, 3, true, false);

    SCAPMT scapmt = {.pmt_id = pmt->id,
                     .other_id = PMT_INVALID,
                     .version = 2,
                     .sid = 0x1234};

    uint8_t capmt[1500];
    int len = create_capmt(&scapmt, CLM_UPDATE, capmt, sizeof(capmt),
                           CA_PMT_CMD_ID_NOT_SELECTED, 0);

    ASSERT(len > 0, "create_capmt failed");
    hexdump("CAPMT: ", capmt, len);

    ASSERT(capmt[0] == CLM_UPDATE, "ca_pmt_list_management incorrect");
    uint16_t sid = (capmt[1] << 8) | capmt[2];
    ASSERT(sid == 0x1234, "program_number incorrect");
    ASSERT(capmt[11] == CA_PMT_CMD_ID_NOT_SELECTED,
           "stream PID 1 should be released");
    ASSERT(capmt[23] == CA_PMT_CMD_ID_NOT_SELECTED,
           "stream PID 2 should be released");

    return 0;
}

// The CAM is told about the release before the CAPMT is handed back.
int test_capmt_release_on_last_pmt() {
    ca_device_t dev;
    memset(&dev, 0, sizeof(dev));
    memset(dev.capmt, -1, sizeof(dev.capmt));
    dev.enabled = 1;
    dev.state = CA_STATE_INITIALIZED;
    dev.id = 0;
    dev.max_ca_pmt = 4;

    adapter ad = {};
    ad.id = 0;
    ca_devices[0] = &dev;

    int first = pmt_add(0, 0x300, 0x301);
    int second = pmt_add(0, 0x400, 0x401);
    SCAPMT *c1 = add_pmt_to_capmt(&dev, get_pmt(first), 1);
    SCAPMT *c2 = add_pmt_to_capmt(&dev, get_pmt(second), 1);
    ASSERT(c1 != c2, "the two PMTs should be in different CAPMTs");

    int version = c2->version;
    dvbca_del_pmt(&ad, get_pmt(second));
    ASSERT(!PMT_ID_IS_VALID(c2->pmt_id), "the CAPMT should have been released");
    ASSERT(c2->version == ((version + 1) & 0xF),
           "the release should be sent as a new version of the program");
    ASSERT(c1->pmt_id == first, "the other CAPMT should be left alone");

    ca_devices[0] = NULL;
    return 0;
}

// A new PMT takes an empty CAPMT first: packing restarts the running
// channel carried by the rewritten CAPMT.
int test_capmt_uses_empty_slots_first() {
    ca_device_t dev;
    memset(&dev, 0, sizeof(dev));
    memset(dev.capmt, -1, sizeof(dev.capmt));
    dev.enabled = 1;
    dev.multiple_pmt = 1;
    dev.max_ca_pmt = 4;

    int first = pmt_add(0, 500, 500);
    int second = pmt_add(0, 600, 600);

    SCAPMT *c1 = add_pmt_to_capmt(&dev, get_pmt(first), dev.multiple_pmt);
    ASSERT(c1 == dev.capmt, "the first PMT should use the first CAPMT");

    SCAPMT *c2 = add_pmt_to_capmt(&dev, get_pmt(second), dev.multiple_pmt);
    ASSERT(c2 == dev.capmt + 1,
           "the second PMT should use an empty CAPMT, not the one in use");
    ASSERT(dev.capmt[0].pmt_id == first &&
               !PMT_ID_IS_VALID(dev.capmt[0].other_id),
           "the first CAPMT should be left alone");
    ASSERT(dev.capmt[1].pmt_id == second, "the second CAPMT should be used");

    // with every CAPMT taken, packing two PMTs together is the only option
    int third = pmt_add(0, 700, 700);
    int fourth = pmt_add(0, 800, 800);
    int fifth = pmt_add(0, 900, 900);
    add_pmt_to_capmt(&dev, get_pmt(third), dev.multiple_pmt);
    add_pmt_to_capmt(&dev, get_pmt(fourth), dev.multiple_pmt);
    SCAPMT *c5 = add_pmt_to_capmt(&dev, get_pmt(fifth), dev.multiple_pmt);
    ASSERT(c5 == dev.capmt, "the fifth PMT should be packed in the first "
                            "CAPMT once all of them are used");
    ASSERT(dev.capmt[0].other_id == fifth, "expected the fifth PMT packed");

    // an update of a PMT keeps using the CAPMT it is already in
    ASSERT(add_pmt_to_capmt(&dev, get_pmt(second), dev.multiple_pmt) ==
               dev.capmt + 1,
           "an update should reuse the same CAPMT");

    return 0;
}

int test_create_capmt_single_clear() {
    int pmt_id = pmt_add(0, 0x100, 0x101);
    SPMT *pmt = get_pmt(pmt_id);
    pmt_add_stream_pid(pmt, 0x501, 2, false, true);
    pmt_add_stream_pid(pmt, 0x502, 3, true, false);

    SCAPMT scampt = {.pmt_id = pmt->id,
                     .other_id = PMT_INVALID,
                     .version = 1,
                     .sid = 0x1234};

    uint8_t capmt[1500];
    int len = create_capmt(&scampt, CLM_ONLY, capmt, sizeof(capmt),
                           CMD_ID_OK_DESCRAMBLING, 0);

    ASSERT(len > 0, "create_capmt failed");
    hexdump("CAPMT: ", capmt, len);

    ASSERT(capmt[0] == CLM_ONLY, "ca_pmt_list_management incorrect");
    uint16_t sid = (capmt[1] << 8) | capmt[2];
    ASSERT(sid == 0x1234, "program_number incorrect");
    uint8_t version = (capmt[3] & 0x3E) >> 1;
    ASSERT(version == 1, "version incorrect");
    bool current_next_indicator = (capmt[3] & 1) == 1;
    ASSERT(current_next_indicator == true, "current next indicator wrong");
    uint16_t pi_len = (capmt[4] << 8) | capmt[5];
    ASSERT(pi_len == 0, "program info length incorrect");
    ASSERT(capmt[6] == 2, "stream PID 1 service type incorrect");
    ASSERT(capmt[11] == 3, "stream PID 2 service type incorrect");

    return 0;
}

int test_create_capmt_single_pmt_scrambled() {
    int pmt_id = pmt_add(0, 0x100, 0x101);
    SPMT *pmt = get_pmt(pmt_id);
    pmt_add_caid(pmt, 0x0B00, 0x573, nullptr, 0);
    pmt_add_stream_pid(pmt, 0x501, 2, false, true);
    pmt_add_stream_pid(pmt, 0x502, 3, true, false);

    SCAPMT scampt = {.pmt_id = pmt->id,
                     .other_id = PMT_INVALID,
                     .version = 1,
                     .sid = 0x1234};

    uint8_t capmt[1500];
    int len = create_capmt(&scampt, CLM_ONLY, capmt, sizeof(capmt),
                           CMD_ID_OK_DESCRAMBLING, 0);

    ASSERT(len > 0, "create_capmt failed");
    hexdump("CAPMT: ", capmt, len);

    // Descriptors should have been added on the stream-level, not program level
    uint16_t pi_len = (capmt[4] << 8) | capmt[5];
    ASSERT(pi_len == 0, "program info length incorrect");
    ASSERT(capmt[6] == 2, "stream PID 1 service type incorrect");
    uint16_t stream1_es_len = (capmt[9] << 8) | capmt[10];
    ASSERT(stream1_es_len == 7, "stream PID 1 ES info length incorrect");
    ASSERT(capmt[18] == 3, "stream PID 2 service type incorrect");
    uint16_t stream2_es_len = (capmt[21] << 8) | capmt[22];
    ASSERT(stream2_es_len == 7, "stream PID 1 ES info length incorrect");

    return 0;
}

int test_create_capmt_multiple_pmt_scrambled() {
    int pmt_id = pmt_add(0, 0x100, 0x101);
    SPMT *pmt = get_pmt(pmt_id);
    pmt_add_caid(pmt, 0x0B00, 0x573, nullptr, 0);
    pmt_add_stream_pid(pmt, 0x501, 2, false, true);
    pmt_add_stream_pid(pmt, 0x502, 3, true, false);

    pmt_id = pmt_add(0, 0x200, 0x201);
    SPMT *other = get_pmt(pmt_id);
    pmt_add_caid(other, 0x0B01, 0xABC, nullptr, 0);
    pmt_add_stream_pid(other, 0x601, 2, false, true);
    pmt_add_stream_pid(other, 0x602, 3, true, false);

    SCAPMT scampt = {
        .pmt_id = pmt->id, .other_id = other->id, .version = 1, .sid = 0x1234};

    uint8_t capmt[1500];
    int len = create_capmt(&scampt, CLM_ONLY, capmt, sizeof(capmt),
                           CMD_ID_OK_DESCRAMBLING, 0);

    ASSERT(len > 0, "create_capmt failed");
    hexdump("CAPMT: ", capmt, len);

    // Descriptors should have been added on the stream-level, not program level
    uint16_t pi_len = (capmt[4] << 8) | capmt[5];
    ASSERT(pi_len == 0, "program info length incorrect");
    ASSERT(capmt[6] == 2, "stream PID 1 service type incorrect");
    uint16_t stream1_es_len = (capmt[9] << 8) | capmt[10];
    ASSERT(stream1_es_len == 7, "stream PID 1 ES info length incorrect");
    ASSERT(capmt[18] == 3, "stream PID 2 service type incorrect");
    uint16_t stream2_es_len = (capmt[21] << 8) | capmt[22];
    ASSERT(stream2_es_len == 7, "stream PID 1 ES info length incorrect");

    // Stream PIDs from other PMT should be present too
    ASSERT(capmt[30] == 2, "stream PID 3 service type incorrect");
    uint16_t stream3_es_len = (capmt[33] << 8) | capmt[34];
    ASSERT(stream3_es_len == 7, "stream PID 3 ES info length incorrect");
    ASSERT(capmt[42] == 3, "stream PID 3 service type incorrect");
    uint16_t stream4_es_len = (capmt[45] << 8) | capmt[46];
    ASSERT(stream4_es_len == 7, "stream PID 4 ES info length incorrect");
    return 0;
}

int test_create_capmt_different_listmgmt() {
    int pmt_id = pmt_add(0, 0x100, 0x101);
    SPMT *pmt = get_pmt(pmt_id);
    pmt_add_stream_pid(pmt, 0x501, 2, false, true);

    SCAPMT scampt = {.pmt_id = pmt->id,
                     .other_id = PMT_INVALID,
                     .version = 1,
                     .sid = 0x1234};

    uint8_t capmt[1500];
    int len;

    // Test CLM_FIRST
    len = create_capmt(&scampt, CLM_FIRST, capmt, sizeof(capmt),
                       CMD_ID_OK_DESCRAMBLING, 0);
    ASSERT(len > 0, "create_capmt failed for CLM_FIRST");
    ASSERT(capmt[0] == CLM_FIRST, "listmgmt should be CLM_FIRST");

    // Test CLM_LAST
    len = create_capmt(&scampt, CLM_LAST, capmt, sizeof(capmt),
                       CMD_ID_OK_DESCRAMBLING, 0);
    ASSERT(len > 0, "create_capmt failed for CLM_LAST");
    ASSERT(capmt[0] == CLM_LAST, "listmgmt should be CLM_LAST");

    // Test CLM_ADD
    len = create_capmt(&scampt, CLM_ADD, capmt, sizeof(capmt),
                       CMD_ID_OK_DESCRAMBLING, 0);
    ASSERT(len > 0, "create_capmt failed for CLM_ADD");
    ASSERT(capmt[0] == CLM_ADD, "listmgmt should be CLM_ADD");

    // Test CLM_UPDATE
    len = create_capmt(&scampt, CLM_UPDATE, capmt, sizeof(capmt),
                       CMD_ID_OK_DESCRAMBLING, 0);
    ASSERT(len > 0, "create_capmt failed for CLM_UPDATE");
    ASSERT(capmt[0] == CLM_UPDATE, "listmgmt should be CLM_UPDATE");

    // Test CLM_MORE
    len = create_capmt(&scampt, CLM_MORE, capmt, sizeof(capmt),
                       CMD_ID_OK_DESCRAMBLING, 0);
    ASSERT(len > 0, "create_capmt failed for CLM_MORE");
    ASSERT(capmt[0] == CLM_MORE, "listmgmt should be CLM_MORE");

    return 0;
}

int test_create_capmt_different_cmd_ids() {
    int pmt_id = pmt_add(0, 0x100, 0x101);
    SPMT *pmt = get_pmt(pmt_id);
    pmt_add_caid(pmt, 0x0B00, 0x573, nullptr, 0);
    pmt_add_stream_pid(pmt, 0x501, 2, false, true);

    SCAPMT scampt = {.pmt_id = pmt->id,
                     .other_id = PMT_INVALID,
                     .version = 1,
                     .sid = 0x1234};

    uint8_t capmt[1500];
    int len;

    // Test CMD_ID_OK_MMI
    len =
        create_capmt(&scampt, CLM_ONLY, capmt, sizeof(capmt), CMD_ID_OK_MMI, 0);
    ASSERT(len > 0, "create_capmt failed for CMD_ID_OK_MMI");
    // cmd_id is at position 11 (after stream type, pid, and es_info_length)
    ASSERT(capmt[11] == CMD_ID_OK_MMI, "cmd_id should be CMD_ID_OK_MMI");

    // Test CMD_ID_QUERY
    len =
        create_capmt(&scampt, CLM_ONLY, capmt, sizeof(capmt), CMD_ID_QUERY, 0);
    ASSERT(len > 0, "create_capmt failed for CMD_ID_QUERY");
    ASSERT(capmt[11] == CMD_ID_QUERY, "cmd_id should be CMD_ID_QUERY");

    // Test CMD_ID_NOT_SELECTED
    len = create_capmt(&scampt, CLM_ONLY, capmt, sizeof(capmt),
                       CMD_ID_NOT_SELECTED, 0);
    ASSERT(len > 0, "create_capmt failed for CMD_ID_NOT_SELECTED");
    ASSERT(capmt[11] == CMD_ID_NOT_SELECTED,
           "cmd_id should be CMD_ID_NOT_SELECTED");

    return 0;
}

int test_create_capmt_invalid_pmt() {
    SCAPMT scampt = {.pmt_id = 9999, // invalid PMT ID
                     .other_id = PMT_INVALID,
                     .version = 1,
                     .sid = 0x1234};

    uint8_t capmt[1500];
    int len = create_capmt(&scampt, CLM_ONLY, capmt, sizeof(capmt),
                           CMD_ID_OK_DESCRAMBLING, 0);

    ASSERT(len == 1, "create_capmt should return 1 (error) for invalid PMT");

    return 0;
}

int test_create_capmt_max_version() {
    int pmt_id = pmt_add(0, 0x100, 0x101);
    SPMT *pmt = get_pmt(pmt_id);
    pmt_add_stream_pid(pmt, 0x501, 2, false, true);

    SCAPMT scampt = {.pmt_id = pmt->id,
                     .other_id = PMT_INVALID,
                     .version = 15, // max 4-bit version
                     .sid = 0x1234};

    uint8_t capmt[1500];
    int len = create_capmt(&scampt, CLM_ONLY, capmt, sizeof(capmt),
                           CMD_ID_OK_DESCRAMBLING, 0);

    ASSERT(len > 0, "create_capmt failed");
    uint8_t version = (capmt[3] & 0x3E) >> 1;
    ASSERT(version == 15, "version should be 15");

    return 0;
}

int test_create_capmt_large_sid() {
    int pmt_id = pmt_add(0, 0x100, 0x101);
    SPMT *pmt = get_pmt(pmt_id);
    pmt_add_stream_pid(pmt, 0x501, 2, false, true);

    SCAPMT scampt = {.pmt_id = pmt->id,
                     .other_id = PMT_INVALID,
                     .version = 1,
                     .sid = 0xFFFF}; // max 16-bit SID

    uint8_t capmt[1500];
    int len = create_capmt(&scampt, CLM_ONLY, capmt, sizeof(capmt),
                           CMD_ID_OK_DESCRAMBLING, 0);

    ASSERT(len > 0, "create_capmt failed");
    uint16_t sid = (capmt[1] << 8) | capmt[2];
    ASSERT(sid == 0xFFFF, "SID should be 0xFFFF");

    return 0;
}

int test_create_capmt_both_pmt_and_other_with_caids() {
    // Both PMTs have CAIDs - basic test case
    int pmt_id = pmt_add(0, 0x100, 0x101);
    SPMT *pmt = get_pmt(pmt_id);
    for (int i = 0; i < 8; i++) {
        pmt_add_caid(pmt, 0x0B00 + i, 0x570 + i, nullptr, 0);
    }
    pmt_add_stream_pid(pmt, 0x501, 2, false, true);
    pmt_add_stream_pid(pmt, 0x502, 3, true, false);

    int other_pmt_id = pmt_add(0, 0x200, 0x201);
    SPMT *other = get_pmt(other_pmt_id);
    for (int i = 0; i < 8; i++) {
        pmt_add_caid(other, 0x0C00 + i, 0x670 + i, nullptr, 0);
    }
    pmt_add_stream_pid(other, 0x601, 2, false, true);
    pmt_add_stream_pid(other, 0x602, 3, true, false);

    SCAPMT scampt = {
        .pmt_id = pmt->id, .other_id = other->id, .version = 1, .sid = 0x1234};

    uint8_t capmt[1500];
    int len = create_capmt(&scampt, CLM_ONLY, capmt, sizeof(capmt),
                           CMD_ID_OK_DESCRAMBLING, 0);

    ASSERT(len > 0, "create_capmt failed");
    ASSERT(len < 1500, "CAPMT exceeds 1500 bytes");
    hexdump("CAPMT both PMT and other with 8 CAIDs each: ", capmt, len);

    LOG("CAPMT size with 4 streams and 16 CAIDs total: %d bytes", len);

    // Header checks
    ASSERT(capmt[0] == CLM_ONLY, "ca_pmt_list_management incorrect");
    uint16_t sid = (capmt[1] << 8) | capmt[2];
    ASSERT(sid == 0x1234, "program_number incorrect");

    return 0;
}

int test_create_capmt_both_pmt_and_other_many_streams() {
    // Both PMTs with many streams and CAIDs
    int pmt_id = pmt_add(0, 0x100, 0x101);
    SPMT *pmt = get_pmt(pmt_id);
    for (int i = 0; i < 10; i++) {
        pmt_add_caid(pmt, 0x0B00 + i, 0x570 + i, nullptr, 0);
    }
    for (int i = 0; i < 8; i++) {
        pmt_add_stream_pid(pmt, 0x500 + i, (i % 2 == 0) ? 2 : 3, (i % 2 == 1),
                           (i % 2 == 0));
    }

    int other_pmt_id = pmt_add(0, 0x200, 0x201);
    SPMT *other = get_pmt(other_pmt_id);
    for (int i = 0; i < 10; i++) {
        pmt_add_caid(other, 0x0C00 + i, 0x670 + i, nullptr, 0);
    }
    for (int i = 0; i < 8; i++) {
        pmt_add_stream_pid(other, 0x600 + i, (i % 2 == 0) ? 2 : 3, (i % 2 == 1),
                           (i % 2 == 0));
    }

    SCAPMT scampt = {
        .pmt_id = pmt->id, .other_id = other->id, .version = 5, .sid = 0xABCD};

    uint8_t capmt[1500];
    int len = create_capmt(&scampt, CLM_ONLY, capmt, sizeof(capmt),
                           CMD_ID_OK_DESCRAMBLING, 0);

    ASSERT(len > 0, "create_capmt failed");
    ASSERT(len < 1500, "CAPMT must be smaller than 1500 bytes");
    hexdump("CAPMT with 16 streams and 20 CAIDs: ", capmt, len);

    LOG("CAPMT size with 16 streams and 20 CAIDs total: %d bytes", len);

    return 0;
}

int test_create_capmt_size_near_limit() {
    // Stress test: maximize size while staying under 1500 bytes
    // Each stream gets all CAIDs, so we need to balance streams vs CAIDs
    int pmt_id = pmt_add(0, 0x100, 0x101);
    SPMT *pmt = get_pmt(pmt_id);
    // Add 10 CAIDs to primary PMT
    for (int i = 0; i < 10; i++) {
        pmt_add_caid(pmt, 0x0B00 + i, 0x570 + i, nullptr, 0);
    }
    // Add 11 streams to primary PMT
    for (int i = 0; i < 11; i++) {
        pmt_add_stream_pid(pmt, 0x500 + i, (i % 2 == 0) ? 2 : 3, (i % 2 == 1),
                           (i % 2 == 0));
    }

    int other_pmt_id = pmt_add(0, 0x200, 0x201);
    SPMT *other = get_pmt(other_pmt_id);
    // Add 10 CAIDs to other PMT
    for (int i = 0; i < 10; i++) {
        pmt_add_caid(other, 0x0C00 + i, 0x670 + i, nullptr, 0);
    }
    // Add 11 streams to other PMT
    for (int i = 0; i < 11; i++) {
        pmt_add_stream_pid(other, 0x600 + i, (i % 2 == 0) ? 2 : 3, (i % 2 == 1),
                           (i % 2 == 0));
    }

    SCAPMT scampt = {
        .pmt_id = pmt->id, .other_id = other->id, .version = 1, .sid = 0x9999};

    uint8_t capmt[1500];
    int len = create_capmt(&scampt, CLM_ONLY, capmt, sizeof(capmt),
                           CMD_ID_OK_DESCRAMBLING, 0);

    ASSERT(len > 0, "create_capmt failed");
    ASSERT(len < 1500, "CAPMT must be smaller than 1500 bytes");
    ASSERT(len > 1200, "CAPMT should be close to 1500 bytes limit");

    LOG("CAPMT size with 22 streams and 20 CAIDs total: %d bytes", len);

    return 0;
}

// A CAPMT that does not fit must truncate, never overrun its buffer.
int test_create_capmt_respects_length() {
    int pmt_id = pmt_add(0, 0x100, 0x101);
    SPMT *pmt = get_pmt(pmt_id);
    uint8_t priv[64];
    memset(priv, 0xAB, sizeof(priv));
    for (int i = 0; i < MAX_CAID; i++)
        pmt_add_caid(pmt, 0x0B00 + i, 0x570 + i, priv, sizeof(priv));
    for (int i = 0; i < 20; i++)
        pmt_add_stream_pid(pmt, 0x500 + i, 2, false, true);

    SCAPMT scampt = {.pmt_id = pmt->id,
                     .other_id = PMT_INVALID,
                     .version = 1,
                     .sid = 0x9999};
    uint8_t capmt[128];
    memset(capmt, 0xAA, sizeof(capmt));
    int len =
        create_capmt(&scampt, CLM_ONLY, capmt, 40, CMD_ID_OK_DESCRAMBLING, 0);
    ASSERT(len > 0 && len <= 40, "CAPMT must respect its length");
    return 0;
}

int test_get_authdata_filename() {
    const char *expected_file_name = "/tmp/ci_auth_Conax_CSP_CIPLUS_CAM_4.bin";
    char actual_filename[FILENAME_MAX];
    get_authdata_filename(actual_filename, sizeof(actual_filename), 4,
                          "Conax CSP CIPLUS CAM");
    LOG("Expected file name: %s", expected_file_name)
    LOG("File name: %s", actual_filename);
    ASSERT(strcmp(actual_filename, expected_file_name) == 0,
           "Auth data filename mismatch");

    return 0;
}

int test_get_ca_caids_string() {
    char caid_string[64];
    ca_devices[0] = alloc_ca_device();

    // No CAIDs
    ca_devices[0]->caids = 0;
    get_ca_caids_string(0, caid_string, 64);
    LOG("CAID string: %s", caid_string);
    ASSERT(strcmp(caid_string, "") == 0, "invalid empty CAID string");

    // One CAID
    ca_devices[0]->caids = 1;
    ca_devices[0]->caid[0] = 0x0B00;
    get_ca_caids_string(0, caid_string, 64);
    LOG("CAID string: %s", caid_string);
    ASSERT(strcmp(caid_string, "0B00") == 0, "invalid single CAID string");

    // Multiple CAIDs
    ca_devices[0]->caids = 3;
    ca_devices[0]->caid[1] = 0x0B01;
    ca_devices[0]->caid[2] = 0x0B02;
    get_ca_caids_string(0, caid_string, 64);
    LOG("CAID string: %s", caid_string);
    ASSERT(strcmp(caid_string, "0B00, 0B01, 0B02") == 0,
           "invalid single CAID string");

    free(ca_devices[0]);
    ca_devices[0] = nullptr;

    return 0;
}

int test_set_ca_channels_parsing() {
    for (int i = 0; i < MAX_ADAPTERS; ++i) {
        ca_devices[i] = nullptr;
    }

    char config[] = "0:*2-0B00-0500";
    set_ca_channels(config);

    ASSERT(ca_devices[0] != nullptr, "Expected ca_devices[0] to be allocated");
    ASSERT(ca_devices[0]->multiple_pmt == 1, "Expected multiple_pmt to be 1");
    ASSERT(ca_devices[0]->max_ca_pmt == 2, "Expected max_ca_pmt to be 2");
    ASSERT(ca_devices[0]->caids == 2, "Expected 2 CAIDs to be parsed");
    ASSERT(ca_devices[0]->caid[0] == 0x0B00,
           "Expected first CAID to be 0x0B00");
    ASSERT(ca_devices[0]->has_forced_caids == 1,
           "Expected has_forced_caids to be 1");

    free(ca_devices[0]);
    ca_devices[0] = nullptr;

    // Test MAX_CAID bounds protection
    char config_overflow[] =
        "0:*2-0100-0200-0300-0400-0500-0600-0700-0800-0900-0A00-0B00-0C00-0D00-"
        "0E00-0F00-1000-1100-1200-1300-1400-1500-1600";
    set_ca_channels(config_overflow);
    ASSERT(ca_devices[0] != nullptr,
           "Expected ca_devices[0] to be allocated on overflow test");
    ASSERT(ca_devices[0]->caids == MAX_CAID,
           "Expected caids to be clamped to MAX_CAID");
    ASSERT(ca_devices[0]->caid[MAX_CAID - 1] == 0x1400,
           "Expected 20th CAID to be 0x1400");

    free(ca_devices[0]);
    ca_devices[0] = nullptr;

    return 0;
}

// A CA that only counts how often a PMT is released on it
static int fake_ca_del_calls;
static int fake_ca_add_pmt(adapter *ad, SPMT *pmt) { return TABLES_RESULT_OK; }
static int fake_ca_del_pmt(adapter *ad, SPMT *pmt) {
    fake_ca_del_calls++;
    return TABLES_RESULT_OK;
}
static SCA_op fake_ca_op = {.ca_add_pid = NULL,
                            .ca_del_pid = NULL,
                            .ca_add_pmt = fake_ca_add_pmt,
                            .ca_del_pmt = fake_ca_del_pmt,
                            .ca_init_dev = NULL,
                            .ca_close_dev = NULL,
                            .ca_ts = NULL,
                            .ca_close_ca = NULL};

// pmt_add_caid() clears ca_mask to force a re-send. A PMT stopped before that
// re-send happens must still be released on the CA that holds it (#1442)
extern int pmt_del(int id);

int test_close_pmt_after_caid_update() {
    uint8_t priv[1] = {0};
    adapter ad = {};
    ad.enabled = 1;
    ad.id = 5;
    ad.type = ADAPTER_DVB;
    a[5] = &ad;

    int ica = add_ca(&fake_ca_op);
    ASSERT(ica >= 0, "could not register the fake CA");
    ad.ca_mask = 1 << ica;

    int id = pmt_add(ad.id, 0x2000, 0x2001);
    ASSERT(id >= 0, "could not create the PMT");
    SPMT *pmt = get_pmt(id);
    pmt_add_caid(pmt, 0x0664, 0x1F06, priv, 0);
    send_pmt_to_cas(&ad, pmt);
    ASSERT(pmt->ca_mask & (1 << ica), "PMT was not sent to the CA");

    // a new CA descriptor turns up, the PMT is due for a re-send ...
    pmt_add_caid(pmt, 0x06EE, 0x1F07, priv, 0);
    ASSERT_EQUAL(pmt->ca_mask, 0, "pmt_add_caid() expected to clear ca_mask");

    // ... but it is stopped before the re-send
    fake_ca_del_calls = 0;
    close_pmt_for_cas(&ad, pmt);
    ASSERT_EQUAL(fake_ca_del_calls, 1,
                 "the CA holding the PMT was not told that it stopped");
    ASSERT_EQUAL(pmt->ca_registered_mask, 0,
                 "the CA registration was not cleared");

    // nothing left to release
    close_pmt_for_cas(&ad, pmt);
    ASSERT_EQUAL(fake_ca_del_calls, 1, "the PMT was released twice");

    pmt_del(id);
    del_ca(&fake_ca_op);
    a[5] = NULL;
    return 0;
}

// Fake CAM slot for testing the reset-and-wait policy.
static int fake_resets;
static int fake_queries_until_ready;
static int fake_present;
static int fake_fail_reset;
static int fake_fail_info;

static int fake_ca_reset(int fd) {
    (void)fd;
    fake_resets++;
    return fake_fail_reset ? -1 : 0;
}

static int fake_ca_slot_info(int fd, struct ca_slot_info *info) {
    (void)fd;
    if (fake_fail_info)
        return -1;
    info->type = CA_CI_LINK;
    info->flags = 0;
    if (fake_present)
        info->flags |= CA_CI_MODULE_PRESENT;
    if (fake_queries_until_ready <= 0)
        info->flags |= CA_CI_MODULE_READY;
    else
        fake_queries_until_ready--;
    return 0;
}

static void fake_ca_sleep(int ms) { (void)ms; }

static void reset_fake_cam(int present, int queries_until_ready) {
    fake_resets = 0;
    fake_present = present;
    fake_queries_until_ready = queries_until_ready;
    fake_fail_reset = 0;
    fake_fail_info = 0;
}

int test_ca_reset_ready_immediately() {
    struct ca_slot_info info;
    reset_fake_cam(1, 0);
    int rv = ca_reset_and_wait_ready(-1, &info, 4, 3, 10, fake_ca_reset,
                                     fake_ca_slot_info, fake_ca_sleep);
    ASSERT_EQUAL(rv, 0, "expected ready");
    ASSERT_EQUAL(fake_resets, 1, "expected a single reset");
    return 0;
}

// A CAM that misses the first reset becomes ready after a retry.
int test_ca_reset_retries_present_module() {
    struct ca_slot_info info;
    reset_fake_cam(1, 25);
    int rv = ca_reset_and_wait_ready(-1, &info, 4, 3, 10, fake_ca_reset,
                                     fake_ca_slot_info, fake_ca_sleep);
    ASSERT_EQUAL(rv, 0, "expected ready after retry");
    ASSERT_EQUAL(fake_resets, 3, "expected resets until ready");
    return 0;
}

// No module in the slot: fail fast without further resets.
int test_ca_reset_no_module_no_retry() {
    struct ca_slot_info info;
    reset_fake_cam(0, 1000000);
    int rv = ca_reset_and_wait_ready(-1, &info, 4, 3, 10, fake_ca_reset,
                                     fake_ca_slot_info, fake_ca_sleep);
    ASSERT_EQUAL(rv, 1, "expected timeout");
    ASSERT_EQUAL(fake_resets, 1, "expected no retry without a module");
    return 0;
}

// A stuck module exhausts all attempts and then gives up.
int test_ca_reset_gives_up_after_attempts() {
    struct ca_slot_info info;
    reset_fake_cam(1, 1000000);
    int rv = ca_reset_and_wait_ready(-1, &info, 4, 3, 10, fake_ca_reset,
                                     fake_ca_slot_info, fake_ca_sleep);
    ASSERT_EQUAL(rv, 1, "expected timeout");
    ASSERT_EQUAL(fake_resets, 3, "expected all attempts used");
    return 0;
}

int test_ca_reset_io_error() {
    struct ca_slot_info info;
    reset_fake_cam(1, 0);
    fake_fail_info = 1;
    ASSERT_EQUAL(ca_reset_and_wait_ready(-1, &info, 4, 3, 10, fake_ca_reset,
                                         fake_ca_slot_info, fake_ca_sleep),
                 -1, "expected io error from slot info");
    reset_fake_cam(1, 0);
    fake_fail_reset = 1;
    ASSERT_EQUAL(ca_reset_and_wait_ready(-1, &info, 4, 3, 10, fake_ca_reset,
                                         fake_ca_slot_info, fake_ca_sleep),
                 -1, "expected io error from reset");
    return 0;
}

// A reconnect is due only for an enabled, down device
// whose fixed retry interval has elapsed.
int test_ca_reconnect_due() {
    ca_device_t dev;
    memset(&dev, 0, sizeof(dev));
    ASSERT(!ca_reconnect_due(NULL, 1000), "null device not due");
    dev.fd = -1;
    ASSERT(!ca_reconnect_due(&dev, 1000), "disabled device not due");
    dev.enabled = 1;
    dev.fd = 5;
    ASSERT(!ca_reconnect_due(&dev, 1000), "live device not due");
    dev.fd = -1;
    dev.reconnect_next_try = 2000;
    ASSERT(!ca_reconnect_due(&dev, 1000), "interval not elapsed");
    ASSERT(ca_reconnect_due(&dev, 2000), "due at the deadline");
    ASSERT(ca_reconnect_due(&dev, 9000), "due after the deadline");
    return 0;
}

// Teardown drops volatile CAM state but keeps the device enabled
// so the reconnect poller picks it up again.
int test_ca_teardown_releases_pmts() {
    ca_device_t dev;
    memset(&dev, 0, sizeof(dev));
    memset(dev.capmt, -1, sizeof(dev.capmt));
    dev.enabled = 1;
    // Shrunken table: a stale entry past max_ca_pmt must still release.
    dev.max_ca_pmt = 1;
    dev.state = CA_STATE_INITIALIZED;
    dev.fd = 42;
    dev.sock = 43;
    dev.id = 2;
    dev.caids = 3;
    dev.poll_fails = 2;
    dev.datetime_next_send = 1234;
    dev.sessions[0].handler.resource = 0x1234;
    dev.sessions[0].session_number = 7;
    memset(dev.key, 0xAB, sizeof(dev.key));
    memset(dev.iv, 0xCD, sizeof(dev.iv));

    int first = pmt_add(0, 0x500, 0x501);
    int second = pmt_add(0, 0x600, 0x601);
    int third = pmt_add(0, 0x700, 0x701);
    dev.capmt[0].pmt_id = first;
    dev.capmt[0].other_id = second;
    dev.capmt[3].pmt_id = third;
    uint64_t mask = 1ULL << dvbca_id;
    for (int id : {first, second, third}) {
        SPMT *pmt = get_pmt(id);
        pmt->ca_mask = mask | 0x40;
        pmt->ca_registered_mask = mask | 0x40;
        pmt->disabled_ca_mask = mask | 0x40;
        pmt->update_cw = 0;
    }

    ca_teardown(&dev);

    ASSERT_EQUAL(dev.enabled, 1, "enabled stays sticky");
    ASSERT_EQUAL(dev.state, CA_STATE_INACTIVE, "state reset");
    ASSERT_EQUAL(dev.fd, -1, "fd reset");
    ASSERT_EQUAL(dev.sock, -1, "sock reset");
    ASSERT_EQUAL(dev.caids, 0u, "caids cleared");
    ASSERT_EQUAL(dev.poll_fails, 0, "poll fails cleared");
    ASSERT_EQUAL(dev.datetime_next_send, 0, "datetime resend due");
    for (int i = 0; i < MAX_SESSIONS; i++)
        ASSERT(dev.sessions[i].handler.resource == 0 &&
                   dev.sessions[i].session_number == 0,
               "sessions cleared");
    static const uint8_t zero_key[2][16] = {};
    ASSERT(memcmp(dev.key, zero_key, sizeof(dev.key)) == 0 &&
               memcmp(dev.iv, zero_key, sizeof(dev.iv)) == 0,
           "keys cleared");
    for (int i = 0; i < MAX_CA_PMT; i++)
        ASSERT(!PMT_ID_IS_VALID(dev.capmt[i].pmt_id) &&
                   !PMT_ID_IS_VALID(dev.capmt[i].other_id),
               "capmt table wiped");
    for (int id : {first, second, third}) {
        SPMT *pmt = get_pmt(id);
        ASSERT_EQUAL(pmt->ca_mask, 0x40, "pmt released for resend");
        ASSERT_EQUAL(pmt->ca_registered_mask, 0x40, "registration cleared");
        ASSERT_EQUAL(pmt->disabled_ca_mask, 0x40, "disabled bit cleared");
        ASSERT_EQUAL(pmt->update_cw, 1, "stale cw disabled");
    }
    // Idempotent and NULL-safe: a second teardown changes nothing.
    ca_teardown(&dev);
    ASSERT_EQUAL(dev.state, CA_STATE_INACTIVE, "second teardown stable");
    ca_teardown(NULL);
    ca_release_pmts(NULL);
    return 0;
}

int test_ca_close_null_safe() {
    ASSERT_EQUAL(ca_close(NULL), 0, "null socket");
    sockets s{};
    s.sid = -1;
    ASSERT_EQUAL(ca_close(&s), 0, "negative sid");
    s.sid = MAX_ADAPTERS;
    ASSERT_EQUAL(ca_close(&s), 0, "sid out of range");
    s.sid = 9;
    ca_device_t *saved = ca_devices[9];
    ca_devices[9] = NULL;
    ASSERT_EQUAL(ca_close(&s), 0, "no device");

    // Happy path: the current socket tears the device down.
    ca_device_t dev;
    memset(&dev, 0, sizeof(dev));
    memset(dev.capmt, -1, sizeof(dev.capmt));
    dev.enabled = 1;
    dev.state = CA_STATE_INITIALIZED;
    dev.fd = 42;
    dev.sock = 41;
    dev.sessions[0].handler.resource = 0x1234;
    memset(dev.key, 0xAB, sizeof(dev.key));
    ca_devices[9] = &dev;
    s.id = 41;
    ASSERT_EQUAL(ca_close(&s), 0, "close runs teardown");
    ASSERT_EQUAL(dev.state, CA_STATE_INACTIVE, "state reset");
    ASSERT_EQUAL(dev.fd, -1, "fd reset");
    ASSERT_EQUAL(dev.enabled, 1, "enabled stays sticky");
    ASSERT_EQUAL(dev.sessions[0].handler.resource, 0, "sessions cleared");

    // Stale generation: an old socket must not wipe the new state.
    dev.state = CA_STATE_INITIALIZED;
    dev.sock = 43;
    ASSERT_EQUAL(ca_close(&s), 0, "stale close skipped");
    ASSERT_EQUAL(dev.state, CA_STATE_INITIALIZED, "new state kept");
    ca_devices[9] = saved;
    return 0;
}

// A disconnected but enabled CA counts as initializing so PMTs
// keep retrying instead of being disabled while it reconnects.
int test_is_ca_initializing_reconnect_pending() {
    ca_device_t dev;
    memset(&dev, 0, sizeof(dev));
    ca_device_t *saved = ca_devices[7];
    ca_devices[7] = &dev;
    dev.enabled = 1;
    dev.state = CA_STATE_INACTIVE;
    ASSERT_EQUAL(is_ca_initializing(7), 1, "reconnect pending retries");
    dev.state = CA_STATE_ACTIVE;
    ASSERT_EQUAL(is_ca_initializing(7), 1, "activating retries");
    dev.state = CA_STATE_INITIALIZED;
    ASSERT_EQUAL(is_ca_initializing(7), 0, "initialized runs");
    dev.enabled = 0;
    dev.state = CA_STATE_INACTIVE;
    ASSERT_EQUAL(is_ca_initializing(7), 0, "disabled skips");
    ca_devices[7] = NULL;
    ASSERT_EQUAL(is_ca_initializing(7), 0, "missing device skips");
    ca_devices[7] = saved;
    return 0;
}

// Slot flags decide presence: present or ready proceeds, empty defers.
int test_ca_slot_has_module() {
    struct ca_slot_info info;
    memset(&info, 0, sizeof(info));
    ASSERT(!ca_slot_has_module(NULL), "null has no module");
    ASSERT(!ca_slot_has_module(&info), "empty slot defers");
    info.flags = CA_CI_MODULE_PRESENT;
    ASSERT(ca_slot_has_module(&info), "present proceeds");
    info.flags = CA_CI_MODULE_READY;
    ASSERT(ca_slot_has_module(&info), "ready proceeds");
    return 0;
}

// Failed init drops the handle but stays enabled for the poller.
int test_ca_init_failed() {
    ASSERT_EQUAL(ca_init_failed(NULL), TABLES_RESULT_ERROR_RETRY,
                 "null retries");
    ca_device_t dev;
    memset(&dev, 0, sizeof(dev));
    dev.fd = -1;
    dev.sock = 5;
    dev.state = CA_STATE_ACTIVE;
    ASSERT_EQUAL(ca_init_failed(&dev), TABLES_RESULT_ERROR_RETRY, "retry");
    ASSERT_EQUAL(dev.enabled, 1, "stays enabled");
    ASSERT_EQUAL(dev.fd, -1, "fd dropped");
    ASSERT_EQUAL(dev.sock, -1, "sock dropped");
    ASSERT_EQUAL(dev.state, CA_STATE_INACTIVE, "state reset");
    return 0;
}

// The poller skips future/disabled devices and re-arms failures.
int test_ca_reconnect_poller() {
    adapter ad = {};
    ad.id = 60;
    ad.pa = 99;
    ad.type = ADAPTER_DVB;
    ad.enabled = 1;
    ca_device_t dev;
    memset(&dev, 0, sizeof(dev));
    dev.enabled = 1;
    dev.fd = -1;
    adapter *saved_a = a[60];
    ca_device_t *saved_c = ca_devices[60];
    a[60] = &ad;
    ca_devices[60] = &dev;

    int64_t next = getTick() + 60000;
    dev.reconnect_next_try = next;
    ca_reconnect(NULL);
    ASSERT_EQUAL(dev.reconnect_next_try, next, "future interval skipped");

    ad.enabled = 0;
    dev.reconnect_next_try = 0;
    ca_reconnect(NULL);
    ASSERT_EQUAL(dev.reconnect_next_try, 0, "disabled adapter skipped");
    ad.enabled = 1;

    int64_t before = getTick();
    ca_reconnect(NULL); // open fails: no /dev/dvb/adapter99 node
    ASSERT(dev.reconnect_next_try >= before + CA_RECONNECT_INTERVAL_MS,
           "failure re-arms the timer");
    ASSERT(dev.reconnect_next_try <= getTick() + CA_RECONNECT_INTERVAL_MS,
           "re-arm is one interval out");

    a[60] = saved_a;
    ca_devices[60] = saved_c;
    return 0;
}

// Keepalive failures count to a close; any success resets the count.
int test_ca_keepalive_counts_failures() {
    ca_device_t dev;
    memset(&dev, 0, sizeof(dev));
    ca_device_t *saved = ca_devices[61];
    ca_devices[61] = &dev;
    dev.enabled = 1;
    dev.state = CA_STATE_ACTIVE;
    dev.fd = -1; // every write fails without an EIO close (EBADF)
    dev.sock = -1;
    sockets ss{};
    ss.sid = 61;
    ca_timeout(&ss);
    ASSERT_EQUAL(dev.poll_fails, 1, "first failure counts");
    ca_timeout(&ss);
    ASSERT_EQUAL(dev.poll_fails, 2, "second failure counts");
    ca_timeout(&ss);
    ASSERT_EQUAL(dev.poll_fails, 0, "third failure closes and resets");

    int fds[2];
    ASSERT(pipe(fds) == 0, "pipe for successful write");
    dev.fd = fds[1];
    dev.poll_fails = 2;
    ca_timeout(&ss);
    ASSERT_EQUAL(dev.poll_fails, 0, "success resets the count");
    close(fds[0]);
    close(fds[1]);
    ca_devices[61] = saved;
    return 0;
}

// TPDU reads: guards drop, EAGAIN stays up, EOF tears down.
int test_ca_read_tpdu_outcomes() {
    int rb = -1;
    uint8_t buf[64];
    ASSERT_EQUAL(ca_read_tpdu(0, buf, sizeof(buf), NULL, &rb), 0,
                 "null sockets");
    ASSERT_EQUAL(rb, 0, "nothing read");
    sockets ss{};
    ss.sid = -1;
    ASSERT_EQUAL(ca_read_tpdu(0, buf, sizeof(buf), &ss, &rb), 0, "bad sid");
    ca_device_t *saved = ca_devices[62];
    ca_devices[62] = NULL;
    ss.sid = 62;
    ASSERT_EQUAL(ca_read_tpdu(0, buf, sizeof(buf), &ss, &rb), 0, "no device");

    ca_device_t dev;
    memset(&dev, 0, sizeof(dev));
    dev.fd = -1;
    ca_devices[62] = &dev;
    ASSERT_EQUAL(ca_read_tpdu(0, buf, sizeof(buf), &ss, &rb), 0, "no fd");

    int fds[2];
    ASSERT(pipe(fds) == 0, "pipe for read outcomes");
    int flags = fcntl(fds[0], F_GETFL);
    fcntl(fds[0], F_SETFL, flags | O_NONBLOCK);
    dev.fd = fds[0];
    ASSERT_EQUAL(ca_read_tpdu(0, buf, sizeof(buf), &ss, &rb), 1,
                 "EAGAIN stays up");
    ASSERT_EQUAL(rb, 0, "nothing buffered");
    uint8_t one = 0x00;
    ASSERT(write(fds[1], &one, 1) == 1, "one byte queued");
    ASSERT_EQUAL(ca_read_tpdu(0, buf, sizeof(buf), &ss, &rb), 1,
                 "short read parses");
    close(fds[1]); // read end now hits EOF
    ASSERT_EQUAL(ca_read_tpdu(0, buf, sizeof(buf), &ss, &rb), 0,
                 "EOF tears down");
    ASSERT_EQUAL(errno, EIO, "errno set for the socket layer");
    close(fds[0]);
    ca_devices[62] = saved;
    return 0;
}

// Enigma removal (POLLPRI, alone or ORed) requests a full close.
int test_ca_read_enigma_removal() {
    int fds[2];
    ASSERT(pipe(fds) == 0, "pipe for enigma reads");
    int flags = fcntl(fds[0], F_GETFL);
    fcntl(fds[0], F_SETFL, flags | O_NONBLOCK);
    int id = sockets_add(fds[0], NULL, 63, TYPE_TCP, NULL, NULL, NULL);
    ASSERT(id >= 0, "socket added");
    sockets *ss = get_sockets(id);
    ca_device_t dev;
    memset(&dev, 0, sizeof(dev));
    dev.enabled = 1;
    dev.state = CA_STATE_ACTIVE;
    dev.sock = id;
    ca_device_t *saved = ca_devices[63];
    ca_devices[63] = &dev;
    uint8_t buf[64];
    int rb = -1;

    ss->revents = POLLIN | POLLPRI; // empty read takes the rl <= 0 path
    ASSERT_EQUAL(ca_read_enigma(fds[0], buf, sizeof(buf), ss, &rb), 1,
                 "removal read ok");
    ASSERT_EQUAL(ss->force_close, 1, "removal requests close");

    ss->force_close = 0;
    ss->revents = POLLIN;
    ASSERT_EQUAL(ca_read_enigma(fds[0], buf, sizeof(buf), ss, &rb), 1,
                 "plain poll quiet");
    ASSERT_EQUAL(ss->force_close, 0, "no close without PRI");

    ss->sid = -1;
    ss->revents = POLLPRI;
    ASSERT_EQUAL(ca_read_enigma(fds[0], buf, sizeof(buf), ss, &rb), 1,
                 "bad sid guarded");

    ca_devices[63] = saved;
    sockets_del(id); // closes fds[0]
    close(fds[1]);
    return 0;
}

// Re-init of a live device keeps the tables bit; bad input is rejected.
int test_dvbca_init_dev_guards() {
    adapter ad = {};
    ad.id = 64;
    ad.type = ADAPTER_DVB;
    ca_device_t dev;
    memset(&dev, 0, sizeof(dev));
    dev.state = CA_STATE_INITIALIZED;
    ca_device_t *saved = ca_devices[64];
    ca_devices[64] = &dev;
    ASSERT_EQUAL(dvbca_init_dev(&ad), TABLES_RESULT_OK, "live device ok");
    ASSERT_EQUAL(ad.ca_mask, 1 << dvbca_id, "tables bit kept");
    ASSERT_EQUAL(dvbca_init_dev(NULL), TABLES_RESULT_ERROR_NORETRY,
                 "null adapter");
    ad.type = ADAPTER_SATIP;
    ad.id = 65;
    ASSERT_EQUAL(dvbca_init_dev(&ad), TABLES_RESULT_ERROR_NORETRY,
                 "wrong adapter type");
    ca_devices[64] = saved;
    return 0;
}

int main() {
    opts.log = 1;
    opts.debug = 255;
    opts.cache_dir = "/tmp";

    strcpy(thread_info[thread_index].thread_name, "test_ca");
    memset(&d, 0, sizeof(d));
    memset(d.capmt, -1, sizeof(d.capmt));
    d.enabled = 1;
    d.multiple_pmt = 1;
    d.max_ca_pmt = 1;

    TEST_FUNC(test_set_ca_channels_parsing(),
              "testing set_ca_channels parsing");
    TEST_FUNC(test_get_ca_caids_string(), "testing CAID string generation");
    TEST_FUNC(test_multiple_pmt(), "testing CA multiple pmt");
    memset(d.capmt, -1, sizeof(d.capmt));
    TEST_FUNC(test_capmt_uses_empty_slots_first(),
              "testing that a new PMT uses an empty CAPMT when there is one");
    TEST_FUNC(test_capmt_release_on_last_pmt(),
              "testing that the last PMT of a CAPMT is released on the CAM");
    TEST_FUNC(test_get_authdata_filename(), "testing filename helper function");
    TEST_FUNC(test_create_capmt_single_clear(),
              "testing create_capmt with single PMT without CA descriptors");
    TEST_FUNC(test_create_capmt_single_pmt_scrambled(),
              "testing create_capmt with single PMT with program-level CA "
              "descriptors");
    TEST_FUNC(test_create_capmt_multiple_pmt_scrambled(),
              "testing create_capmt with multiple PMTs with program-level CA "
              "descriptors");
    TEST_FUNC(test_create_capmt_both_pmt_and_other_with_caids(),
              "testing create_capmt with both PMT and other with CAIDs");
    TEST_FUNC(test_create_capmt_both_pmt_and_other_many_streams(),
              "testing create_capmt with both PMT and other with many streams");
    TEST_FUNC(test_create_capmt_not_selected(),
              "testing create_capmt with the not_selected command id");
    TEST_FUNC(test_create_capmt_size_near_limit(),
              "testing create_capmt size near 1500 byte limit");
    TEST_FUNC(test_create_capmt_respects_length(),
              "testing create_capmt truncates to its length");
    TEST_FUNC(
        test_close_pmt_after_caid_update(),
        "testing CA release of a PMT stopped after a CA descriptor update");
    TEST_FUNC(test_ca_reset_ready_immediately(),
              "testing CAM ready after a single reset");
    TEST_FUNC(test_ca_reset_retries_present_module(),
              "testing CAM reset retry when the module misses a reset");
    TEST_FUNC(test_ca_reset_no_module_no_retry(),
              "testing no CAM reset retry without a module");
    TEST_FUNC(test_ca_reset_gives_up_after_attempts(),
              "testing CAM reset gives up after all attempts");
    TEST_FUNC(test_ca_reset_io_error(), "testing CAM reset io errors");
    TEST_FUNC(test_ca_reconnect_due(), "testing reconnect due gating");
    TEST_FUNC(test_ca_teardown_releases_pmts(),
              "testing teardown releases PMTs for resend");
    TEST_FUNC(test_ca_close_null_safe(), "testing ca_close guards");
    TEST_FUNC(test_is_ca_initializing_reconnect_pending(),
              "testing reconnect pending counts as initializing");
    TEST_FUNC(test_ca_slot_has_module(), "testing slot presence flags");
    TEST_FUNC(test_ca_init_failed(), "testing failed init stays enabled");
    TEST_FUNC(test_ca_reconnect_poller(), "testing reconnect poller gating");
    TEST_FUNC(test_ca_keepalive_counts_failures(),
              "testing keepalive failure counting");
    TEST_FUNC(test_ca_read_tpdu_outcomes(), "testing TPDU read outcomes");
    TEST_FUNC(test_ca_read_enigma_removal(), "testing enigma removal close");
    TEST_FUNC(test_dvbca_init_dev_guards(), "testing init device guards");
    free_all(); // releases sockets opened by the enigma test
    free_all_pmts();
    fflush(stdout);
    return 0;
}
