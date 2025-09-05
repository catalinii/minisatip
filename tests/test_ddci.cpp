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
#include "ddci.h"
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

#define DEFAULT_LOG LOG_DVBCA

#undef FOREACH_ITEM
#define FOREACH_ITEM(h, a)                                                     \
    for (i = 0; i < (h)->size; i++)                                            \
        if (HASH_ITEM_ENABLED((h)->items[i]) &&                                \
            (a = (ddci_mapping_table_t *)(h)->items[i].data))

extern ddci_device_t *ddci_devices[MAX_ADAPTERS];
extern adapter *a[MAX_ADAPTERS];
extern SPMT *pmts[MAX_PMT];
extern int npmts;
extern SCA_op dvbca;
extern ca_device_t *ca_devices[MAX_ADAPTERS];
extern SHashTable channels;
extern SFilter *filters[MAX_FILTERS];

SPMT *create_pmt(int ad, int sid, int pid1, int pid2, int caid1, int caid2) {
    int pmt_id = pmt_add(ad, sid, 1000);
    SPMT *pmt = get_pmt(pmt_id);
    pmt->pid = pmt_id * 1000;
    pmt_add_stream_pid(pmt, pid1, 2, 0, 1, 0);
    pmt_add_stream_pid(pmt, pid2, 6, 1, 0, 0);
    pmt_add_caid(pmt, caid1, caid1, NULL, 0);
    pmt_add_caid(pmt, caid2, caid2, NULL, 0);
    return pmt;
}

void create_adapter(adapter *ad, int id) {
    memset(ad, 0, sizeof(adapter));
    ad->enabled = 1;
    ad->id = id;
    a[id] = ad;
}

int test_channels() {
    SHashTable h;
    int i;
    Sddci_channel c1, c2, c3, *t;
    create_hash_table(&h, 10);

    // Add one channel (SID 200) that can be decoded by DDCI 1
    memset(&c1, 0, sizeof(c1));
    c1.sid = 200;
    c1.ddci[0].ddci = 1;
    c1.ddcis = 1;
    setItem(&h, c1.sid, &c1, sizeof(c1));

    // Add a second channel (SID 300) that can be decoded by both DDCI 1 and 2
    memset(&c2, 0, sizeof(c2));
    c2.sid = 300;
    c2.ddci[0].ddci = 1;
    c2.ddci[1].ddci = 2;
    c2.ddcis = 2;
    setItem(&h, c2.sid, &c2, sizeof(c2));

    // Add a third channel (SID 400) that cannot be decoded by any adapter
    memset(&c3, 0, sizeof(c3));
    c3.sid = 400;
    c3.ddcis = 0;
    setItem(&h, c3.sid, &c3, sizeof(c3));

    // Save and reload the table
    save_channels(&h);
    free_hash(&h);
    h.init = 0;
    create_hash_table(&h, 10);
    load_channels(&h);

    // Check the contents
    ASSERT(getItem(&h, 200) != NULL, "Saved SID 200 not found in table");
    ASSERT(getItem(&h, 300) != NULL, "Saved SID 300 not found in table");
    ASSERT(getItem(&h, 400) != NULL, "Saved SID 400 not found in table");
    int ch = 0;

    for (i = 0; i < h.size; i++)
        if (HASH_ITEM_ENABLED(h.items[i]))
            ch++;

    ASSERT(ch == 3, "Expected two channels after loading");
    free_hash(&h);
    return 0;
}

int test_add_del_pmt() {
    int i;
    SPMT *pmt0, *pmt1, *pmt2, *pmt3, *pmt4;
    ddci_device_t d0, d1;
    ca_device_t ca0, ca1;
    adapter ad, a0, a1;

    create_adapter(&ad, 8);
    create_adapter(&a0, 0);
    create_adapter(&a1, 1);

    pmt0 = create_pmt(8, 100, 101, 102, 0x100, 0x1800);
    pmt1 = create_pmt(8, 200, 201, 202, 0x100, 0x500);
    pmt2 = create_pmt(8, 300, 301, 302, 0x500, 0x100);
    pmt3 = create_pmt(8, 400, 401, 402, 0x600, 0x601);
    pmt4 = create_pmt(8, 500, 501, 502, 0x500, 0x500);
    memset(&d0, 0, sizeof(d0));
    memset(&d1, 0, sizeof(d1));
    memset(&d0.pmt, -1, sizeof(d0.pmt));
    memset(&d1.pmt, -1, sizeof(d1.pmt));
    d0.id = 0;
    d1.id = 1;
    d0.enabled = d1.enabled = 1;
    ddci_devices[0] = &d0;
    ddci_devices[1] = &d1;
    create_hash_table(&d0.mapping, 30);
    create_hash_table(&d1.mapping, 30);
    create_hash_table(&channels, 30);

    // Save a channel for pmt4, we'll check that it is not assigned to
    // any DDCI adapter even though its CAID matches etc.
    Sddci_channel c4;
    memset(&c4, 0, sizeof(c4));
    c4.sid = pmt4->sid;
    c4.ddcis = 0;
    setItem(&channels, c4.sid, &c4, sizeof(c4));

    int dvbca_id = add_ca(&dvbca);
    // DD 0 - 0x100, DD 1 - 0x500
    add_caid_mask(dvbca_id, 0, 0x100, 0xFFFF);
    add_caid_mask(dvbca_id, 1, 0x500, 0xFFFF);
    memset(&ca0, 0, sizeof(ca0));
    memset(&ca1, 0, sizeof(ca1));
    ca0.id = 0;
    ca1.id = 1;
    ca0.enabled = ca1.enabled = 1;
    ca0.state = CA_STATE_ACTIVE;
    ca1.state = CA_STATE_INITIALIZED;
    d0.max_channels = d1.max_channels = 1;
    ca_devices[0] = &ca0;
    ca_devices[1] = &ca1;
    // No matching DDCI
    ASSERT(ddci_process_pmt(&ad, pmt3) == TABLES_RESULT_ERROR_RETRY,
           "DDCI not ready, expected retry");

    ca0.state = CA_STATE_INITIALIZED;
    ASSERT(ddci_process_pmt(&ad, pmt3) == TABLES_RESULT_ERROR_NORETRY,
           "DDCI ready, expected no retry");

    ASSERT(ddci_process_pmt(&ad, pmt4) == TABLES_RESULT_ERROR_NORETRY,
           "Channel not assigned to any DDCI adapter, expected no retry");

    // One matching channel
    ASSERT(ddci_process_pmt(&ad, pmt0) == TABLES_RESULT_OK,
           "DDCI matching DD 0");
    ASSERT(d0.pmt[0].id == 0, "PMT 0 using DDCI 0");

    ASSERT(ddci_process_pmt(&ad, pmt1) == TABLES_RESULT_OK,
           "DDCI matching DD 1");
    ASSERT(d1.pmt[0].id == 1, "PMT 1 using DDCI 1");
    d0.max_channels = d1.max_channels = 2;

    // Multiple PMTs
    Sddci_channel c;
    memset(&c, 0, sizeof(c));
    c.sid = pmt2->sid;
    c.locked = 1;
    c.ddci[c.ddcis++].ddci = 1;
    setItem(&channels, c.sid, &c, sizeof(c));

    pmt_add_stream_pid(pmt2, 0xFF, 2, 0, 1, 0);
    pmt_add_caid(pmt2, 0x502, 0xFE, NULL, 0);

    ASSERT(ddci_process_pmt(&ad, pmt2) == TABLES_RESULT_OK,
           "DDCI matching DD 0 for second PMT");
    ASSERT(d1.pmt[1].id == 2, "PMT 2 using DDCI 1");

    // make sure we still have pids enabled from the first PMT
    int ec = 0, j, k;
    ddci_mapping_table_t *m;
    int pmt_pids[MAX_ADAPTERS];
    memset(pmt_pids, 0, sizeof(pmt_pids));
    for (k = 0; k < 2; k++) {
        FOREACH_ITEM(&ddci_devices[k]->mapping, m) {
            ec++;
            for (j = 0; j < m->npmt; j++)
                if (m->pmt[j] >= 0)
                    pmt_pids[m->pmt[j]]++;
        }
    }
    ASSERT(ec > 3, "Deleted Pids from the previously added PMT");
    ASSERT(pmt_pids[0] > 0, "PMT 0 expected to have pids");
    ASSERT(pmt_pids[1] > 0, "PMT 1 expected to have pids");
    ASSERT(pmt_pids[2] > 0, "PMT 2 expected to have pids");

    m = get_pid_mapping_allddci(ad.id, 0xFF);
    ASSERT(m != NULL, "Newly added pid not found in mapping table");
    m = get_pid_mapping_allddci(ad.id, 0xFE);
    ASSERT(m != NULL, "Newly added capid not found in mapping table");

    ddci_del_pmt(&ad, pmt1);
    ddci_del_pmt(&ad, pmt0);
    ddci_del_pmt(&ad, pmt2);
    ec = 0;
    FOREACH_ITEM(&d0.mapping, m) { ec++; }
    FOREACH_ITEM(&d1.mapping, m) { ec++; }
    ASSERT(ec == 0, "No pid should be enabled");
    free_hash(&d0.mapping);
    free_hash(&d1.mapping);
    free_hash(&channels);
    free_filters();
    return 0;
}

int test_copy_ts_from_ddci() {
    ddci_device_t d;
    ddci_mapping_table_t *m;
    adapter ad, ad0;
    uint8_t buf[188 * 10], buf2[188 * 10];
    memset(&d, 0, sizeof(d));
    memset(buf, 0, sizeof(buf));
    memset(buf2, 0, sizeof(buf2));
    d.id = 0;
    d.enabled = 1;
    create_hash_table(&d.mapping, 30);
    create_fifo(&d.fifo, DDCI_BUFFER);
    memset(ddci_devices, 0, sizeof(ddci_devices));
    ddci_devices[0] = &d;

    create_adapter(&ad, 1);
    create_adapter(&ad0, 0);
    ad.buf = buf2;
    ad.lbuf = sizeof(buf2);
    int pid = 1000;
    __attribute__((unused)) int ad_pos = 0;
    buf[0] = buf2[0] = 0x47;
    add_pid_mapping_table(2, pid, 9, &d,
                          0); // forcing mapping to a different pid
    add_pid_mapping_table(1, pid, 0, &d, 0);
    m = get_pid_mapping_allddci(1, pid);
    ASSERT(m != NULL, "Pid not found in mapping table");
    int new_pid = m->ddci_pid;
    ASSERT(new_pid == 1001, "Unexpected pid found after conflict");
    ad.rlen = 188;
    set_pid_ts(buf, new_pid);
    set_pid_ts(buf2, 0x1FFF);
    fifo_push(&d.fifo, buf, 188);
    uint16_t mapping[8192];
    memset(mapping, 0, sizeof(mapping));
    mapping[new_pid] = pid;

    if (push_ts_to_adapter(&d, &ad, mapping))
        LOG_AND_RETURN(1, "could not copy the packet to the adapter");
    if (PID_FROM_TS(buf2) != pid)
        LOG_AND_RETURN(1, "found pid %d expected %d", PID_FROM_TS(buf2), pid);

    free_hash(&d.mapping);
    free_fifo(&d.fifo);
    return 0;
}

int is_err = 0;
int expected_pid = 0;
int did_write = 0;
int xwritev(int fd, const struct iovec *io, int len) {
    unsigned char *b = (unsigned char *)io[0].iov_base;
    LOGM("called writev with len %d, first pid %d", len, PID_FROM_TS(b));
    did_write = 1;
    if (len < 2) {
        is_err = 1;
        LOG_AND_RETURN(
            -1, "writev did not receive proper arguments, expected 2, got %d",
            len);
    }

    if (PID_FROM_TS(b) != expected_pid) {
        is_err = 1;
        LOG_AND_RETURN(-1,
                       "writev did not receive proper TS, expected %d, got %d",
                       expected_pid, PID_FROM_TS(b));
    }
    return len * 188;
}

int test_ddci_process_ts() {
    ddci_device_t d;
    uint8_t buf[188 * 10];
    uint8_t fifo[188 * 3];
    int i;
    adapter ad, ad2, ad0;
    memset(buf, 0, sizeof(buf));
    memset(fifo, 0, sizeof(fifo));
    d.id = 2;
    d.enabled = 1;
    create_fifo(&d.fifo, DDCI_BUFFER);
    create_hash_table(&d.mapping, 30);
    memset(ddci_devices, 0, sizeof(ddci_devices));
    ddci_devices[0] = &d;
    create_adapter(&ad0, 0);
    create_adapter(&ad, 1);
    create_adapter(&ad2, 2);
    ad.buf = buf;
    ad.lbuf = sizeof(buf);
    for (i = 0; i < ad.lbuf; i += 188) {
        buf[i] = 0x47;
        set_pid_ts(buf + i, 2121); // unmapped pid
    }

    SPMT *save = pmts[0];
    pmts[0] = NULL;
    add_pid_mapping_table(5, 1000, 0, &d, 0);
    add_pid_mapping_table(5, 2000, 0, &d, 0);
    int new_pid = add_pid_mapping_table(1, 1000, 0, &d, 0);
    int new_pid2 = add_pid_mapping_table(1, 2000, 0, &d, 0);
    ad.rlen = ad.lbuf - 188; // allow just 2 packets
    set_pid_ts(buf, 1000);
    set_pid_ts(buf + 2 * 188, 2000);
    // Prevents resetting read_index to write_index
    d.read_index[1] = d.read_index[2] = d.fifo.write_index = 188;

    set_pid_ts(fifo, new_pid); // first packet, expected 1000
    set_pid_ts(fifo + 188, new_pid2);
    set_pid_ts(fifo + 376, new_pid2);
    fifo_push(&d.fifo, fifo, sizeof(fifo));
    expected_pid = new_pid;
    _writev = (mywritev)&xwritev;
    d.last_pmt = getTick(); // prevent adding PMT/EPG
    ddci_process_ts(&ad, &d);
    if (is_err)
        LOG_AND_RETURN(1, "is_err is set");
    if (!did_write)
        LOG_AND_RETURN(1, "no writev called");
    if (PID_FROM_TS(ad.buf) != 1000)
        LOG_AND_RETURN(1, "expected pid 1000 in the adapter buffer, got %d",
                       PID_FROM_TS(ad.buf));
    if (PID_FROM_TS(ad.buf + 2 * 188) != 2000)
        LOG_AND_RETURN(1, "expected pid 2000 in the adapter buffer, got %d",
                       PID_FROM_TS(ad.buf + 2 * 188));
    free_hash(&d.mapping);
    free_fifo(&d.fifo);
    pmts[0] = save;
    return 0;
}
int test_create_pat() {
    ddci_device_t d;
    uint8_t psi[188];
    uint8_t packet[188];
    int pid = 4096;
    int pmt_id = pmt_add(0, 0x66, pid);
    int16_t cc;
    int psi_len;
    SFilter f;
    adapter ad;
    create_adapter(&ad, 0);

    memset(&d, 0, sizeof(d));
    d.id = 0;
    d.enabled = 1;
    d.max_channels = 1;
    SPMT *pmt = get_pmt(pmt_id);
    create_hash_table(&d.mapping, 30);
    memset(ddci_devices, 0, sizeof(ddci_devices));
    ddci_devices[0] = &d;

    add_pid_mapping_table(9, pid, 0, &d, 0);
    int dpid = add_pid_mapping_table(0, pid, 0, &d, 0);
    f.flags = FILTER_CRC;
    f.id = 0;
    f.adapter = 0;
    d.pmt[0].id = pmt->id;
    psi_len = ddci_create_pat(&d, psi);
    cc = 1;
    buffer_to_ts(packet, 188, psi, psi_len, &cc, 0);
    int len = assemble_packet(&f, packet);
    if (!len)
        return 1;
    int new_sid = packet[17] * 256 + packet[18];
    int new_pid = packet[19] * 256 + packet[20];
    new_pid &= 0x1FFF;
    if (new_pid != dpid)
        LOG_AND_RETURN(1, "PAT pid %d != mapping table pid %d", new_pid, dpid);
    if (new_sid != pmt->sid)
        LOG_AND_RETURN(1, "PAT sid %d != pmt sid %d", new_sid, pmt->sid);
    free_hash(&d.mapping);
    return 0;
}

int test_create_sdt() {
    // Create an adapter
    adapter ad;
    create_adapter(&ad, 0);

    // Create a DDCI device
    ddci_device_t d;
    memset(&d, 0, sizeof(d));
    d.id = 0;
    d.enabled = 1;
    d.max_channels = 2;
    memset(&d.pmt, -1, sizeof(d.pmt));
    create_hash_table(&d.mapping, 30);
    memset(ddci_devices, 0, sizeof(ddci_devices));
    ddci_devices[0] = &d;

    // Create two PMTs (SID 0x100 and 0x101)
    int pid1 = 4096;
    int pid2 = 4097;
    int sid1 = 0x100;
    int sid2 = 0x101;
    int pmt_id1 = pmt_add(ad.id, sid1, pid1);
    int pmt_id2 = pmt_add(ad.id, sid2, pid2);
    add_pid_mapping_table(ad.id, pid1, pmt_id1, &d, 0);
    add_pid_mapping_table(ad.id, pid2, pmt_id2, &d, 0);
    d.pmt[0].id = pmt_id1;
    d.pmt[1].id = pmt_id2;

    /*ASSERT(ddci_process_pmt(&ad, pmt1) == TABLES_RESULT_OK,
           "Adding PMT1 to DD");
    ASSERT(ddci_process_pmt(&ad, pmt2) == TABLES_RESULT_OK,
           "Adding PMT2 to DD");*/

    // Create the SDT
    uint8_t sdt[188];
    memset(&sdt, 0, sizeof(sdt));
    int sdt_len = ddci_create_sdt(&d, sdt);
    LOG("SDT length: %d", sdt_len);
    _hexdump("SDT", sdt, sdt_len);

    // Create the TS packet
    SFilter f;
    f.flags = FILTER_CRC;
    f.id = 0;
    f.adapter = 0;
    uint8_t packet[188];
    int16_t cc = 1;
    int ts_len = buffer_to_ts(packet, 188, sdt, sdt_len, &cc, 17);
    LOG("TS length: %d", ts_len);
    _hexdump("TS", packet, ts_len);
    int len = assemble_packet(&f, packet);
    LOG("Packet length %d", len);
    if (!len)
        return 1;

    // Check that the SDT contains two services with running status = 4 and
    // free CA mode = 1
    int sdt_sid1 = (packet[16] << 8) + packet[17];
    int sdt_running_status1 = packet[19] >> 5;
    int sdt_free_CA_mode1 = (packet[19] & (1 << 4)) > 0;
    ASSERT(sdt_sid1 == 0x100, "SDT SID 1 != 0x100");
    ASSERT(sdt_running_status1 == 4, "SDT SID 1 running_status != 4");
    ASSERT(sdt_free_CA_mode1 != 0, "SDT SID 1 free_CA_mode != 1");
    int sdt_sid2 = (packet[21] << 8) + packet[22];
    int sdt_running_status2 = packet[24] >> 5;
    int sdt_free_CA_mode2 = (packet[24] & (1 << 4)) > 0;
    ASSERT(sdt_sid2 == 0x101, "SDT SID 1 != 0x101");
    ASSERT(sdt_running_status2 == 4, "SDT SID 1 running_status != 4");
    ASSERT(sdt_free_CA_mode2 != 0, "SDT SID 1 free_CA_mode != 1");

    free_hash(&d.mapping);
    return 0;
}

int test_create_pmt() {
    ddci_device_t d;
    uint8_t psi[188];
    uint8_t packet[188];
    adapter ad;
    int16_t cc;
    int psi_len;
    SFilter f;
    memset(&d, 0, sizeof(d));
    d.id = 0;
    d.enabled = 1;
    a[0] = 0;
    create_hash_table(&d.mapping, 30);
    memset(ddci_devices, 0, sizeof(ddci_devices));
    ddci_devices[0] = &d;
    int pid = 1023;
    int capid = 0x100;
    int dpid = add_pid_mapping_table(0, pid, 0, &d, 0);
    int dcapid = add_pid_mapping_table(0, capid, 0, &d, 0);
    f.flags = FILTER_CRC;
    f.id = 0;
    f.adapter = 0;
    f.pid = pid;
    f.next_filter = -1;
    f.enabled = 1;
    filters[0] = &f;
    SPMT *pmt = create_pmt(0, 0x66, pid, 0x55, 0x100, 0x500);
    d.pmt[0].id = pmt->id; // set to pmt 1
    pmt->pid = pid - 1;
    pmt->version = 0;
    strcpy((char *)pmt->name, "TEST CHANNEL HD");
    ddci_pmt_t dp = {.ver = 0, .pcr_pid = 8191};
    ca_devices[0] = NULL;
    ca_devices[1] = NULL;

    psi_len = ddci_create_pmt(&d, pmt, psi, sizeof(psi), &dp);
    cc = 1;
    _hexdump("PACK: ", psi, psi_len);
    buffer_to_ts(packet, 188, psi, psi_len, &cc, 0x63);
    _hexdump("TS: ", packet, 188);
    int len = assemble_packet(&f, packet);
    if (!len) {
        LOG("Assemble packet failed");
        return 1;
    }
    int new_pid = packet[18] * 256 + packet[19];
    int new_capid = packet[9] * 256 + packet[10];
    new_pid &= 0x1FFF;
    new_capid &= 0x1FFF;
    if (new_pid != dpid)
        LOG_AND_RETURN(1, "PMT stream pid %04X != mapping table pid %04X",
                       new_pid, dpid);
    if (new_capid != dcapid)
        LOG_AND_RETURN(1, "PMT PSI pid %04X != mapping table pid %04X",
                       new_capid, dcapid);

    SPMT *new_pmt = get_pmt(pmt_add(0, 200, 200));
    ad.id = 0;
    ad.enabled = 1;
    a[0] = &ad;
    ad.pids[0].pid = pid;
    ad.pids[0].flags = 1;

    ad.active_pmts = 1;
    ad.active_pmt[0] = pmt->id;

    process_pmt(0, psi + 1, psi_len, new_pmt);
    filters[0] = NULL;
    ASSERT_EQUAL(
        pmt->stream_pids, new_pmt->stream_pids,
        "Number of streampids does not matches between generated PMT and "
        "read PMT");
    ASSERT_EQUAL(pmt->caids, new_pmt->caids,
                 "Number of caids does not matches "
                 "between generated PMT and read PMT");

    free_hash(&d.mapping);
    return 0;
}

int main() {
    opts.log = 65535 ^ LOG_LOCK ^ LOG_UTILS;
    opts.debug = 0;
    opts.cache_dir = "/tmp";
    strcpy(thread_info[thread_index].thread_name, "test_ddci");
    TEST_FUNC(test_channels(), "testing test_channels");
    TEST_FUNC(test_add_del_pmt(), "testing adding and removing pmts");
    TEST_FUNC(test_copy_ts_from_ddci(), "testing test_copy_ts_from_ddci");
    TEST_FUNC(test_ddci_process_ts(), "testing ddci_process_ts");
    TEST_FUNC(test_create_pat(), "testing create_pat");
    TEST_FUNC(test_create_sdt(), "testing create_sdt");
    TEST_FUNC(test_create_pmt(), "testing create_pmt");
    free_all_pmts();
    fflush(stdout);
    return 0;
}
