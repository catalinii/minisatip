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

extern ddci_device_t *ddci_devices[MAX_ADAPTERS];
extern adapter *a[MAX_ADAPTERS];
extern SPMT *pmts[MAX_PMT];
extern int npmts;
extern int dvbca_id;
extern ca_device_t *ca_devices[MAX_ADAPTERS];
extern SHashTable channels;
extern SFilter *filters[MAX_FILTERS];

typedef struct ca_device {
    int enabled;
    SCAPMT capmt[MAX_CA_PMT];
    int max_ca_pmt, multiple_pmt;
    int fd;
    int slot_id;
    int tc;
    int id;
    int ignore_close;
    int init_ok;
} ca_device_t;

void create_pmt(SPMT *pmt, int id, int ad, int sid, int pid1, int pid2,
                int caid1, int caid2) {
    memset(pmt, 0, sizeof(*pmt));
    pmt->id = id;
    pmt->sid = sid;
    pmt->adapter = ad;
    pmt->pid = id * 1000;
    pmt->stream_pid[0].pid = pid1;
    pmt->stream_pid[0].type = 2;
    pmt->stream_pid[1].pid = pid2;
    pmt->stream_pid[0].type = 6;
    pmt->stream_pids = 2;
    pmt->ca[0].id = caid1;
    pmt->ca[0].pid = caid1;
    pmt->ca[1].id = caid2;
    pmt->ca[1].pid = caid2;
    pmt->caids = 2;
    pmts[id] = pmt;
    if (npmts <= id) {
        npmts = id + 1;
    }
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
    Sddci_channel c, *t;
    memset(&h, 0, sizeof(h));
    create_hash_table(&h, 10);
    memset(&c, 0, sizeof(c));
    c.sid = 200;
    c.ddci[1].ddci = 1;
    c.ddcis = 1;
    setItem(&h, c.sid, &c, sizeof(c));
    save_channels(&h, "/tmp/minisatip.channels");
    free_hash(&h);
    create_hash_table(&h, 10);
    load_channels(&h, "/tmp/minisatip.channels");
    ASSERT(getItem(&h, 200) != NULL, "Saved SID not found in table");
    int ch = 0;
    FOREACH_ITEM(&h, t) { ch++; }
    ASSERT(ch == 1, "Expected one channel after loading");
    free_hash(&h);
    return 0;
}

int test_add_del_pmt() {
    int i;
    SPMT pmt0, pmt1, pmt2, pmt3;
    ddci_device_t d0, d1;
    ca_device_t ca0, ca1;
    adapter ad, a0, a1;

    create_adapter(&ad, 8);
    create_adapter(&a0, 0);
    create_adapter(&a1, 1);

    create_pmt(&pmt0, 0, 8, 100, 101, 102, 0x100, 0x1800);
    create_pmt(&pmt1, 1, 8, 200, 201, 202, 0x100, 0x500);
    create_pmt(&pmt2, 2, 8, 300, 301, 302, 0x500, 0x100);
    create_pmt(&pmt3, 3, 8, 400, 401, 402, 0x600, 0x601);
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

    dvbca_init();
    // DD 0 - 0x100, DD 1 - 0x500
    add_caid_mask(dvbca_id, 0, 0x100, 0xFFFF);
    add_caid_mask(dvbca_id, 1, 0x500, 0xFFFF);
    memset(&ca0, 0, sizeof(ca0));
    memset(&ca1, 0, sizeof(ca1));
    ca0.id = 0;
    ca1.id = 1;
    ca0.enabled = ca1.enabled = 1;
    ca0.init_ok = 0;
    ca1.init_ok = 1;
    d0.max_channels = d1.max_channels = 1;
    ca_devices[0] = &ca0;
    ca_devices[1] = &ca1;
    // No matching DDCI
    ASSERT(ddci_process_pmt(&ad, &pmt3) == TABLES_RESULT_ERROR_RETRY,
           "DDCI not ready, expected retry");

    ca0.init_ok = 1;
    ASSERT(ddci_process_pmt(&ad, &pmt3) == TABLES_RESULT_ERROR_NORETRY,
           "DDCI ready, expected no retry");

    // One matching channel
    ASSERT(ddci_process_pmt(&ad, &pmt0) == TABLES_RESULT_OK,
           "DDCI matching DD 0");
    ASSERT(d0.pmt[0].id == 0, "PMT 0 using DDCI 0");

    ASSERT(ddci_process_pmt(&ad, &pmt1) == TABLES_RESULT_OK,
           "DDCI matching DD 1");
    ASSERT(d1.pmt[0].id == 1, "PMT 1 using DDCI 1");
    d0.max_channels = d1.max_channels = 2;

    // Multiple PMTs
    ASSERT(ddci_process_pmt(&ad, &pmt2) == TABLES_RESULT_OK,
           "DDCI matching DD 0 for second PMT");
    ASSERT(d0.pmt[1].id == 2, "PMT 2 using DDCI 0");
    blacklist_pmt_for_ddci(&pmt2, 0);
    ddci_del_pmt(&ad, &pmt2);

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
    ASSERT(pmt_pids[2] == 0, "PMT 2 expected to NOT have pids");

    ASSERT(ddci_process_pmt(&ad, &pmt2) == TABLES_RESULT_OK,
           "DDCI matching DD 1 for second PMT");
    ASSERT(d1.pmt[1].id == 2, "PMT 2 using DDCI 1");

    int s = pmt2.stream_pids++;
    int c = pmt2.caids++;
    pmt2.stream_pid[s].pid = 0xFF;
    pmt2.stream_pid[s].type = 2;
    pmt2.ca[c].id = 0x502;
    pmt2.ca[c].pid = 0xFE;

    ASSERT(ddci_process_pmt(&ad, &pmt2) == TABLES_RESULT_OK,
           "DDCI matching DD 1 for adding again the PMT");

    m = get_pid_mapping_allddci(ad.id, 0xFF);
    ASSERT(m != NULL, "Newly added pid not found in mapping table");
    m = get_pid_mapping_allddci(ad.id, 0xFE);
    ASSERT(m != NULL, "Newly added capid not found in mapping table");
    blacklist_pmt_for_ddci(&pmt2, 0);
    ddci_del_pmt(&ad, &pmt2);
    ASSERT(ddci_process_pmt(&ad, &pmt2) == TABLES_RESULT_ERROR_NORETRY,
           "DDCI should be blacklisted on all DDCIs");

    ddci_del_pmt(&ad, &pmt1);
    ddci_del_pmt(&ad, &pmt0);

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

int test_push_ts_to_ddci() {
    ddci_device_t d;
    adapter ad;
    uint8_t buf[188 * 10];
    d.id = 0;
    d.enabled = 1;
    d.out = malloc1(DDCI_BUFFER + 10);
    d.wo = DDCI_BUFFER - 188;
    memset(d.ro, -1, sizeof(d.ro));
    d.ro[0] = 188;
    memset(ddci_devices, 0, sizeof(ddci_devices));
    ddci_devices[0] = &d;
    create_adapter(&ad, 0);
    push_ts_to_ddci_buffer(&d, buf, 376);
    if (d.ro[0] != 376)
        LOG_AND_RETURN(1, "test drop a packet when pushing 2 with wrap");
    push_ts_to_ddci_buffer(&d, buf, 376);
    if (d.ro[0] != 752 || d.wo != 564)
        LOG_AND_RETURN(1, "test dropping 2 packets without wrapping");
    d.ro[0] = 752;
    d.wo = 0;
    push_ts_to_ddci_buffer(&d, buf, 376);
    if (d.wo != 376 || d.ro[0] != 752)
        LOG_AND_RETURN(1, "push 376 bytes");
    free1(d.out);
    return 0;
}

int test_copy_ts_from_ddci() {
    ddci_device_t d;
    ddci_mapping_table_t *m;
    adapter ad;
    uint8_t buf[188 * 10], buf2[188 * 10];
    memset(&d, 0, sizeof(d));
    memset(buf, 0, sizeof(buf));
    memset(buf2, 0, sizeof(buf2));
    d.id = 0;
    d.enabled = 1;
    d.out = malloc1(DDCI_BUFFER + 10);
    create_hash_table(&d.mapping, 30);
    memset(ddci_devices, 0, sizeof(ddci_devices));
    ddci_devices[0] = &d;

    create_adapter(&ad, 1);
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

    if (push_ts_to_adapter(&ad, buf, pid, &ad_pos))
        LOG_AND_RETURN(1, "could not copy the packet to the adapter");
    if (PID_FROM_TS(buf2) != 1000)
        LOG_AND_RETURN(1, "found pid %d expected %d", PID_FROM_TS(buf2), pid);
    if (PID_FROM_TS(buf) != 0x1FFF)
        LOG_AND_RETURN(1, "PID from the DDCI buffer not marked correctly %d",
                       PID_FROM_TS(buf));

    set_pid_ts(buf, new_pid);
    if (push_ts_to_adapter(&ad, buf, pid, &ad_pos))
        LOG_AND_RETURN(1, "could not copy the packet to the adapter");
    if (ad.rlen != 376)
        LOG_AND_RETURN(1, "rlen not marked correctly %d", ad.rlen);
    ad.rlen = ad.lbuf;
    set_pid_ts(buf, new_pid);
    if (1 != push_ts_to_adapter(&ad, buf, pid, &ad_pos))
        LOG_AND_RETURN(1, "buffer full not returned correctly");

    free1(d.out);
    free_hash(&d.mapping);
    return 0;
}

int is_err = 0;
int expected_pid = 0;
int did_write = 0;
int xwritev(int fd, const struct iovec *io, int len) {
    unsigned char *b = io[0].iov_base;
    LOGM("called writev with len %d, first pid %d", len, PID_FROM_TS(b));
    did_write = 1;
    if (len != 1) {
        is_err = 1;
        LOG_AND_RETURN(
            -1, "writev did not receive proper arguments, expected 1, got %d",
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
    int i;
    adapter ad;
    memset(&d, 0, sizeof(d));
    memset(buf, 0, sizeof(buf));
    d.id = 0;
    d.enabled = 1;
    d.out = malloc1(DDCI_BUFFER + 10);
    create_hash_table(&d.mapping, 30);
    memset(ddci_devices, 0, sizeof(ddci_devices));
    ddci_devices[0] = &d;
    mutex_init(&d.mutex);
    create_adapter(&ad, 1);
    ad.buf = buf;
    ad.lbuf = sizeof(buf);
    for (i = 0; i < ad.lbuf; i += 188) {
        buf[i] = 0x47;
        set_pid_ts(buf + i, 2121); // unmapped pid
    }
    a[0]->enabled = 1;
    a[1] = &ad;
    buf[0] = 0x47;
    pmts[0] = NULL;
    add_pid_mapping_table(5, 1000, 0, &d, 0);
    add_pid_mapping_table(5, 2000, 0, &d, 0);
    int new_pid = add_pid_mapping_table(1, 1000, 0, &d, 0);
    int new_pid2 = add_pid_mapping_table(1, 2000, 0, &d, 0);
    ad.rlen = ad.lbuf - 188; // allow just 1 packet + 1 cleared that it will
                             // be written to the socket
    set_pid_ts(ad.buf + 188, 1000);
    memset(d.ro, -1, sizeof(d.ro));
    d.ro[1] = DDCI_BUFFER - 188;          // 1 packet before end of buffer
    d.wo = 188 * 2;                       // 2 after end of the buffer
    set_pid_ts(d.out + d.ro[1], new_pid); // first packet, expected 1000
    set_pid_ts(d.out, new_pid2);
    set_pid_ts(d.out + 188, new_pid2);
    expected_pid = new_pid;
    _writev = (mywritev)&xwritev;
    d.last_pmt = getTick(); // prevent adding PMT/EPG
    ddci_process_ts(&ad, &d);
    if (is_err)
        LOG_AND_RETURN(1, "is_err is set");
    if (!did_write)
        LOG_AND_RETURN(1, "no writev called");
    if (PID_FROM_TS(ad.buf + 188) != 1000)
        LOG_AND_RETURN(1, "expected pid 1000 in the adapter buffer, got %d",
                       PID_FROM_TS(ad.buf + 188));
    if (PID_FROM_TS(ad.buf + ad.lbuf - 188) != 2000)
        LOG_AND_RETURN(1, "expected pid 2000 in the adapter buffer, got %d",
                       PID_FROM_TS(ad.buf + ad.lbuf - 188));
    if (ad.rlen != ad.lbuf)
        LOG_AND_RETURN(1, "adapter buffer length mismatch %d != %d", ad.rlen,
                       ad.lbuf);
    if (d.ro[1] != 188 && d.wo != 188 * 2)
        LOG_AND_RETURN(1, "indexes in DDCI devices set wrong ro %d wo %d", d.ro,
                       d.wo);
    free1(d.out);
    free_hash(&d.mapping);
    return 0;
}
int test_create_pat() {
    ddci_device_t d;
    uint8_t psi[188];
    uint8_t packet[188];
    SPMT pmt;
    char cc;
    int psi_len;
    SFilter f;
    adapter ad;
    memset(&pmt, 0, sizeof(pmt));
    create_adapter(&ad, 0);

    memset(&d, 0, sizeof(d));
    d.id = 0;
    d.enabled = 1;
    create_hash_table(&d.mapping, 30);
    memset(ddci_devices, 0, sizeof(ddci_devices));
    ddci_devices[0] = &d;
    int pid = 4096;
    add_pid_mapping_table(9, pid, 0, &d, 0);
    int dpid = add_pid_mapping_table(0, pid, 0, &d, 0);
    f.flags = FILTER_CRC;
    f.id = 0;
    f.adapter = 0;
    d.pmt[0].id = 1;   // set to pmt 1
    pmts[1] = &pmt; // enable pmt 1
    npmts = 2;
    pmt.enabled = 1;
    pmt.sid = 0x66;
    pmt.pid = pid;
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
    if (new_sid != pmt.sid)
        LOG_AND_RETURN(1, "PAT sid %d != pmt sid %d", new_sid, pmt.sid);
    free_hash(&d.mapping);
    return 0;
}

int test_create_pmt() {
    ddci_device_t d;
    uint8_t psi[188];
    uint8_t packet[188];

    SPMT pmt;
    char cc;
    int psi_len;
    SFilter f;
    memset(&d, 0, sizeof(d));
    memset(&pmt, 0, sizeof(pmt));
    d.id = 0;
    d.enabled = 1;
    a[0] = 0;
    create_hash_table(&d.mapping, 30);
    memset(ddci_devices, 0, sizeof(ddci_devices));
    ddci_devices[0] = &d;
    int pid = 1023;
    int capid = 7068;
    int dpid = add_pid_mapping_table(0, pid, 0, &d, 0);
    int dcapid = add_pid_mapping_table(0, capid, 0, &d, 0);
    f.flags = FILTER_CRC;
    f.id = 0;
    f.adapter = 0;
    filters[0] = &f;
    d.pmt[0].id = 1;   // set to pmt 1
    pmts[1] = &pmt; // enable pmt 1
    npmts = 2;
    pmt.enabled = 1;
    pmt.sid = 0x66;
    pmt.pid = pid;
    strcpy(pmt.name, "TEST CHANNEL HD");
    pmt.pmt_len = 0;
    pmt.caids = 1;
    pmt.ca[0].id = 0x100;
    pmt.ca[0].pid = capid;
    pmt.ca[0].private_data_len = 2;
    pmt.ca[0].private_data[0] = 1;
    pmt.ca[0].private_data[1] = 2;
    pmt.stream_pids = 2;
    pmt.stream_pid[0].type = 0x1B;
    pmt.stream_pid[0].pid = pid;
    pmt.stream_pid[0].desc_len = 2;
    pmt.stream_pid[0].desc[0] = 0x54;
    pmt.stream_pid[0].desc[1] = 0;
    pmt.stream_pid[1].type = 3;
    pmt.stream_pid[1].pid = 0x55;

    psi_len = ddci_create_pmt(&d, &pmt, psi, sizeof(psi), 0, 8191);
    cc = 1;
    _hexdump("PACK: ", psi, psi_len);
    buffer_to_ts(packet, 188, psi, psi_len, &cc, 0x63);
    _hexdump("TS: ", packet, 188);
    int len = assemble_packet(&f, packet);
    if (!len) {
        LOG("Assemble packet failed");
        return 1;
    }
    int new_pid = packet[26] * 256 + packet[27];
    int new_capid = packet[21] * 256 + packet[22];
    new_pid &= 0x1FFF;
    new_capid &= 0x1FFF;
    if (new_pid != dpid)
        LOG_AND_RETURN(1, "PMT stream pid %04X != mapping table pid %04X",
                       new_pid, dpid);
    if (new_capid != dcapid)
        LOG_AND_RETURN(1, "PMT PSI pid %04X != mapping table pid %04X",
                       new_capid, dcapid);
    SPMT new_pmt;
    memset(&new_pmt, 0, sizeof(pmt));
    new_pmt.enabled = 1;
    f.adapter = 0;
    f.next_filter = -1;
    f.pid = 0x99;
    f.enabled = 1;
    new_pmt.version = 1;
    adapter ad;
    ad.id = 0;
    ad.enabled = 1;
    a[0] = &ad;
    ad.pids[0].pid = 0x99;
    ad.pids[0].flags=1;
    process_pmt(0, psi + 1, psi_len, &new_pmt);
    filters[0] = NULL;
    ASSERT(new_pmt.stream_pids == pmt.stream_pids,
           "Number of streampids does not matches between generated PMT and "
           "read PMT");
    ASSERT(new_pmt.caids == pmt.caids, "Number of caids does not matches "
                                       "between generated PMT and read PMT");

    free_hash(&d.mapping);
    return 0;
}

int main() {
    opts.log = 65535;
    opts.debug = 0;
    strcpy(thread_name, "test");
    TEST_FUNC(test_channels(), "testing test_channels");
    TEST_FUNC(test_add_del_pmt(), "testing adding and removing pmts");
    TEST_FUNC(test_push_ts_to_ddci(), "testing test_push_ts_to_ddci");
    TEST_FUNC(test_copy_ts_from_ddci(), "testing test_copy_ts_from_ddci");
    TEST_FUNC(test_ddci_process_ts(), "testing ddci_process_ts");
    TEST_FUNC(test_create_pat(), "testing create_pat");
    TEST_FUNC(test_create_pmt(), "testing create_pmt");
    fflush(stdout);
    return 0;
}
