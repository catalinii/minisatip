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
extern SPMT *pmts[MAX_PMT];
void remove_pmt_from_device(ca_device_t *d, SPMT *pmt);
SCAPMT *add_pmt_to_capmt(ca_device_t *d, SPMT *pmt, int multiple);
int get_active_capmts(ca_device_t *d);
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
    len = create_capmt(&scampt, CLM_ONLY, capmt, sizeof(capmt), CMD_ID_OK_MMI,
                       0);
    ASSERT(len > 0, "create_capmt failed for CMD_ID_OK_MMI");
    // cmd_id is at position 11 (after stream type, pid, and es_info_length)
    ASSERT(capmt[11] == CMD_ID_OK_MMI, "cmd_id should be CMD_ID_OK_MMI");

    // Test CMD_ID_QUERY
    len = create_capmt(&scampt, CLM_ONLY, capmt, sizeof(capmt), CMD_ID_QUERY,
                       0);
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
    SCAPMT scampt = {.pmt_id = 9999,  // invalid PMT ID
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
                     .version = 15,  // max 4-bit version
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
                     .sid = 0xFFFF};  // max 16-bit SID

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
        pmt_add_stream_pid(pmt, 0x500 + i, (i % 2 == 0) ? 2 : 3,
                           (i % 2 == 1), (i % 2 == 0));
    }

    int other_pmt_id = pmt_add(0, 0x200, 0x201);
    SPMT *other = get_pmt(other_pmt_id);
    for (int i = 0; i < 10; i++) {
        pmt_add_caid(other, 0x0C00 + i, 0x670 + i, nullptr, 0);
    }
    for (int i = 0; i < 8; i++) {
        pmt_add_stream_pid(other, 0x600 + i, (i % 2 == 0) ? 2 : 3,
                           (i % 2 == 1), (i % 2 == 0));
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
        pmt_add_stream_pid(pmt, 0x500 + i, (i % 2 == 0) ? 2 : 3,
                           (i % 2 == 1), (i % 2 == 0));
    }

    int other_pmt_id = pmt_add(0, 0x200, 0x201);
    SPMT *other = get_pmt(other_pmt_id);
    // Add 10 CAIDs to other PMT
    for (int i = 0; i < 10; i++) {
        pmt_add_caid(other, 0x0C00 + i, 0x670 + i, nullptr, 0);
    }
    // Add 11 streams to other PMT
    for (int i = 0; i < 11; i++) {
        pmt_add_stream_pid(other, 0x600 + i, (i % 2 == 0) ? 2 : 3,
                           (i % 2 == 1), (i % 2 == 0));
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

    TEST_FUNC(test_get_ca_caids_string(), "testing CAID string generation");
    TEST_FUNC(test_multiple_pmt(), "testing CA multiple pmt");
    memset(d.capmt, -1, sizeof(d.capmt));
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
    TEST_FUNC(test_create_capmt_size_near_limit(),
              "testing create_capmt size near 1500 byte limit");
    fflush(stdout);
    return 0;
}
