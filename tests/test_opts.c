/*
 * Copyright (C) 2014-2023 Catalin Toda <catalinii@yahoo.com> et al
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

#include "adapter.h"
#include "opts.h"
#include "utils/testing.h"
#include <stdio.h>
#include <string.h>

extern adapter *a[MAX_ADAPTERS];

void _reset_dvbapi_opts() {
    opts.pids_all_no_dec = 0;
    opts.dvbapi_offset = 0;
    opts.dvbapi_port = 0;
    memset(opts.dvbapi_host, 0, sizeof(opts.dvbapi_host));
}

void _parse_dvbapi_opt(char *optarg) {
    LOG("Parsing \"%s\"", optarg);
    _reset_dvbapi_opts();
    parse_dvbapi_opt(optarg, &opts);
}

int test_parse_dvbapi_opt() {
    // Socket
    _parse_dvbapi_opt("/tmp/camd.socket");
    ASSERT(opts.pids_all_no_dec == 0, "opts.pids_all_no_dec != 0");
    ASSERT(opts.dvbapi_offset == 0, "opts.dvbapi_offset != 0");
    ASSERT(opts.dvbapi_port == 9000, "opts.dvbapi_port != 9000");
    ASSERT(strcmp(opts.dvbapi_host, "/tmp/camd.socket") == 0,
           "opts.dvbapi_host != \"/tmp/camd.socket\"");

    // Socket with offset or port doesn't make sense, so skip checking those

    // Host
    _parse_dvbapi_opt("192.168.1.100");
    ASSERT(opts.pids_all_no_dec == 0, "opts.pids_all_no_dec != 0");
    ASSERT(opts.dvbapi_offset == 0, "opts.dvbapi_offset != 0");
    ASSERT(opts.dvbapi_port == 9000, "opts.dvbapi_port != 9000");
    ASSERT(strcmp(opts.dvbapi_host, "192.168.1.100") == 0,
           "opts.dvbapi_host != \"192.168.1.100\"");

    // Host with pids_all_no_dec
    _parse_dvbapi_opt("~192.168.1.100");
    ASSERT(opts.pids_all_no_dec == 1, "opts.pids_all_no_dec != 1");
    ASSERT(opts.dvbapi_offset == 0, "opts.dvbapi_offset != 0");
    ASSERT(opts.dvbapi_port == 9000, "opts.dvbapi_port != 9000");
    ASSERT(strcmp(opts.dvbapi_host, "192.168.1.100") == 0,
           "opts.dvbapi_host != \"192.168.1.100\"");

    // Host with port
    _parse_dvbapi_opt("192.168.1.100:9001");
    ASSERT(opts.pids_all_no_dec == 0, "opts.pids_all_no_dec != 0");
    ASSERT(opts.dvbapi_offset == 0, "opts.dvbapi_offset != 0");
    ASSERT(opts.dvbapi_port == 9001, "opts.dvbapi_port != 9001");
    ASSERT(strcmp(opts.dvbapi_host, "192.168.1.100") == 0,
           "opts.dvbapi_host != \"192.168.1.100\"");

    // Host with port and offset
    _parse_dvbapi_opt("192.168.1.100:9001,1");
    ASSERT(opts.pids_all_no_dec == 0, "opts.pids_all_no_dec != 0");
    ASSERT(opts.dvbapi_offset == 1, "opts.dvbapi_offset != 1");
    ASSERT(opts.dvbapi_port == 9001, "opts.dvbapi_port != 9001");
    ASSERT(strcmp(opts.dvbapi_host, "192.168.1.100") == 0,
           "opts.dvbapi_host != \"192.168.1.100\"");

    // Host with port and offset and pids_all_no_dec
    _parse_dvbapi_opt("~192.168.1.100:9001,1");
    ASSERT(opts.pids_all_no_dec == 1, "opts.pids_all_no_dec != 1");
    ASSERT(opts.dvbapi_offset == 1, "opts.dvbapi_offset != 1");
    ASSERT(opts.dvbapi_port == 9001, "opts.dvbapi_port != 9001");
    ASSERT(strcmp(opts.dvbapi_host, "192.168.1.100") == 0,
           "opts.dvbapi_host != \"192.168.1.100\"");

    return 0;
}

int test_set_slave_adapters() {
    int i;
    set_slave_adapters("0-1:0,2-3:2");

    for (i = 0; i < 4; i++) {
        LOG("adapter %d master %d", a[i]->id, a[i]->master_source);
    }

    ASSERT(a[0]->master_source == 0, "adapter 0 master != 0");
    ASSERT(a[1]->master_source == 0, "adapter 1 master != 0");
    ASSERT(a[2]->master_source == 2, "adapter 2 master != 2");
    ASSERT(a[3]->master_source == 2, "adapter 3 master != 2");
       
    return 0;
}

int main() {
    opts.log = 255;
    strcpy(thread_info[thread_index].thread_name, "test_opts");
    TEST_FUNC(test_parse_dvbapi_opt(), "parse_dvbapi_offset() failed");
    TEST_FUNC(test_set_slave_adapters(), "test_set_slave_adapters() failed");
    return 0;
}
