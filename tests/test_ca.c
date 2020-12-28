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
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <libdvben50221/en50221_app_ca.h>
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
void remove_and_sort_pmt_ids(ca_device_t *d, SPMT *pmt);
int pmt_add(int i, int adapter, int sid, int pmt_pid);
void get_2_pmt_to_process(ca_device_t *d, SPMT *pmt, SPMT **other,
                          int *listmgmt);

typedef struct ca_device {
    int enabled;
    int pmt_id[MAX_CA_PMT], enabled_pmts;

} ca_device_t;

int test_quirks() {
    int listmgmt = -1;
    SPMT *other;
    // id, adapter, sid, pmt_pid
    pmt_add(0, 0, 100, 100);
    pmt_add(1, 0, 200, 200);
    pmt_add(2, 0, 100, 300);
    pmt_add(3, 0, 400, 400);
    ca_device_t d;
    memset(&d, 0, sizeof(d));
    d.enabled = 1;

    d.pmt_id[0] = -1;
    d.pmt_id[1] = 2;
    d.pmt_id[2] = 0;
    d.pmt_id[3] = 1;
    d.enabled_pmts = 2;
    remove_and_sort_pmt_ids(&d, get_pmt(2));
    if (d.pmt_id[0] == -1 || d.pmt_id[1] == -1)
        LOG_AND_RETURN(
            1,
            "Sorting not performed correctly: expected first 2 PMT id "
            "%d %d to be >= 0",
            d.pmt_id[0], d.pmt_id[1]);

    if (d.pmt_id[2] != -1 || d.pmt_id[3] != -1)
        LOG_AND_RETURN(
            1,
            "Sorting not performed correctly: last 2 PMTs to be not set: %d %d",
            d.pmt_id[0], d.pmt_id[1]);

    d.enabled_pmts = 1;
    remove_and_sort_pmt_ids(&d, get_pmt(d.pmt_id[1]));
    d.enabled_pmts = 2;
    get_2_pmt_to_process(&d, get_pmt(0), &other, &listmgmt);
    if (listmgmt != CA_LIST_MANAGEMENT_ONLY)
        LOG_AND_RETURN(1, "List mgmt should not ONLY");

    if (other != get_pmt(1))
        LOG_AND_RETURN(1, "Expected the other PMT to be set correctly");

    d.enabled_pmts = 3;
    get_2_pmt_to_process(&d, get_pmt(2), &other, &listmgmt);
    if (listmgmt != CA_LIST_MANAGEMENT_ADD)
        LOG_AND_RETURN(1, "List mgmt should not ADD");

    if (other != NULL)
        LOG_AND_RETURN(
            1,
            "Expected the other PMT to be set correctly for the second case");

    return 0;
}

int main() {
    opts.log = 1;
    opts.debug = 255;
    strcpy(thread_name, "test");
    TEST_FUNC(test_quirks(), "testing CA quirks");
    fflush(stdout);
    return 0;
}
