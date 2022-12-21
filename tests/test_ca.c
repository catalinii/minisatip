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
extern SPMT *pmts[MAX_PMT];
void remove_pmt_from_device(ca_device_t *d, SPMT *pmt);
SCAPMT *add_pmt_to_capmt(ca_device_t *d, SPMT *pmt, int multiple);
int get_active_capmts(ca_device_t *d);
int get_enabled_pmts_for_ca(ca_device_t *d);

ca_device_t d;

int test_multiple_pmt() {
    // id, adapter, sid, pmt_pid
    pmt_add(0, 0, 100, 100);
    pmt_add(1, 0, 200, 200);
    pmt_add(2, 0, 300, 300);
    pmt_add(3, 0, 400, 400);

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

int test_create_capmt() {
    uint8_t clean[1500];
    pmt_add(5, 0, 500, 500);
    pmt_add(6, 0, 600, 600);
    add_pmt_to_capmt(&d, get_pmt(5), 1);
    add_pmt_to_capmt(&d, get_pmt(6), 1);
    SPMT *pmt = get_pmt(5);
    pmt->stream_pids = 2;
    pmt->stream_pid[0].pid = 501;
    pmt->stream_pid[0].type = 2;
    pmt->stream_pid[0].pid = 502;
    pmt->stream_pid[0].type = 3;

    pmt = get_pmt(6);
    pmt->stream_pids = 2;
    pmt->stream_pid[0].pid = 601;
    pmt->stream_pid[0].type = 2;
    pmt->stream_pid[0].pid = 602;
    pmt->stream_pid[0].type = 3;

    int len = create_capmt(d.capmt, 1, clean, sizeof(clean), 0, 0);
    if (len <= 0)
        LOG_AND_RETURN(1, "createCAPMT failed");
    hexdump("CAPMT: ", clean, len);
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

    strcpy(thread_name, "test_ca");
    memset(&d, 0, sizeof(d));
    memset(d.capmt, -1, sizeof(d.capmt));
    d.enabled = 1;
    d.multiple_pmt = 1;
    d.max_ca_pmt = 1;

    TEST_FUNC(test_get_ca_caids_string(), "testing CAID string generation");
    TEST_FUNC(test_multiple_pmt(), "testing CA multiple pmt");
    memset(d.capmt, -1, sizeof(d.capmt));
    TEST_FUNC(test_create_capmt(), "testing CA creating multiple pmt");
    TEST_FUNC(test_get_authdata_filename(), "testing filename helper function");
    fflush(stdout);
    return 0;
}
