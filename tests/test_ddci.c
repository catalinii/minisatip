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
int test_push_ts_to_ddci() {
    ddci_device_t d;
    uint8_t buf[188 * 10];
    d.id = 0;
    d.enabled = 1;
    d.out = malloc1(DDCI_BUFFER + 10);
    d.wo = DDCI_BUFFER - 188;
    memset(d.ro, -1, sizeof(d.ro));
    d.ro[0] = 188;
    memset(ddci_devices, 0, sizeof(ddci_devices));
    ddci_devices[0] = &d;
    push_ts_to_ddci_buffer(&d, buf, 376);
    if (d.ro[0] != 376)
        LOG_AND_RETURN(1, "test drop a packet when pushing 2 with wrap");
    push_ts_to_ddci_buffer(&d, buf, 376);
    if (d.ro[0] != 752 || d.wo!= 564)
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
    memset(&ad, 0, sizeof(ad));
    memset(buf, 0, sizeof(buf));
    memset(buf2, 0, sizeof(buf2));
    d.id = 0;
    d.enabled = 1;
    d.out = malloc1(DDCI_BUFFER + 10);
    create_hash_table(&d.mapping, 30);
    memset(ddci_devices, 0, sizeof(ddci_devices));
    ddci_devices[0] = &d;

    ad.id = 1;
    ad.enabled = 1;
    ad.buf = buf2;
    ad.lbuf = sizeof(buf2);
    a[1] = &ad;
    int pid = 1000;
    __attribute__((unused)) int ad_pos = 0;
    buf[0] = buf2[0] = 0x47;
    add_pid_mapping_table(2, pid, 9, &d,
                          0); // forcing mapping to a different pid
    add_pid_mapping_table(1, pid, 0, &d, 0);
    m = get_pid_mapping_allddci(1, pid);
    ASSERT(m != NULL);
    int new_pid = m->ddci_pid;
    ASSERT(new_pid == 1001);
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
    adapter ad;
    uint8_t buf[188 * 10];
    int i;
    memset(&d, 0, sizeof(d));
    memset(&ad, 0, sizeof(ad));
    memset(buf, 0, sizeof(buf));
    d.id = 0;
    d.enabled = 1;
    d.out = malloc1(DDCI_BUFFER + 10);
    create_hash_table(&d.mapping, 30);
    memset(ddci_devices, 0, sizeof(ddci_devices));
    ddci_devices[0] = &d;
    mutex_init(&d.mutex);
    ad.id = 1;
    ad.enabled = 1;
    ad.buf = buf;
    ad.lbuf = sizeof(buf);
    for (i = 0; i < ad.lbuf; i += 188) {
        buf[i] = 0x47;
        set_pid_ts(buf + i, 2121); // unmapped pid
    }
    a[0]->enabled = 1;
    a[1] = &ad;
    buf[0] = 0x47;
    add_pid_mapping_table(5, 1000, 0, &d, 0);
    add_pid_mapping_table(5, 2000, 0, &d, 0);
    int new_pid = add_pid_mapping_table(1, 1000, 0, &d, 0);
    int new_pid2 = add_pid_mapping_table(1, 2000, 0, &d, 0);
    ad.rlen = ad.lbuf - 188; // allow just 1 packet + 1 cleared that it will be
                             // written to the socket
    set_pid_ts(ad.buf + 188, 1000);
    memset(d.ro, -1, sizeof(d.ro));
    d.ro[1] = DDCI_BUFFER - 188;          // 1 packet before end of buffer
    d.wo = 188 * 2;                       // 2 after end of the buffer
    set_pid_ts(d.out + d.ro[1], new_pid); // first packet, expected 1000
    set_pid_ts(d.out, new_pid2);
    set_pid_ts(d.out + 188, new_pid2);
    expected_pid = new_pid;
    _writev = (mywritev)&xwritev;
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
extern SPMT *pmts[MAX_PMT];
extern int npmts;
int test_create_pat() {
    ddci_device_t d;
    uint8_t psi[188];
    uint8_t packet[188];
    SPMT pmt;
    char cc;
    int psi_len;
    SFilter f;
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
    d.pmt[0] = 1;   // set to pmt 1
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

    unsigned char pmt_sample[] =
        "\x02\xb0\x49\x00\x32\xeb\x00\x00\xe3\xff\xf0\x18\x09\x04\x09\xc4"
        "\xfb\x9c\x09\x04\x09\x8c\xfa\x9c\x09\x04\x09\xaf\xff\x9c\x09\x04"
        "\x09\x8d\xfc\x9c\x1b\xe3\xff\xf0\x03\x52\x01\x02\x03\xe4\x00\xf0"
        "\x09\x0a\x04\x64\x65\x75\x01\x52\x01\x03\x03\xe4\x01\xf0\x09\x0a"
        "\x04\x65\x6e\x67\x01\x52\x01\x06\xdc\x54\xdb\x72";

    SPMT pmt;
    char cc;
    int psi_len;
    SFilter f;
    memset(&d, 0, sizeof(d));
    memset(&pmt, 0, sizeof(pmt));
    d.id = 0;
    d.enabled = 1;
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
    d.pmt[0] = 1;   // set to pmt 1
    pmts[1] = &pmt; // enable pmt 1
    npmts = 2;
    pmt.enabled = 1;
    pmt.sid = 0x66;
    pmt.pid = pid;
    strcpy(pmt.name, "TEST CHANNEL HD");
    memcpy(pmt.pmt, pmt_sample, sizeof(pmt_sample));
    pmt.pmt_len = sizeof(pmt_sample) - 1;
    psi_len = ddci_create_pmt(&d, &pmt, psi, 0);
    cc = 1;
    _hexdump("PACK: ", psi, psi_len);
    buffer_to_ts(packet, 188, psi, psi_len, &cc, 0x63);
    _hexdump("TS: ", packet, 188);
    int len = assemble_packet(&f, packet);
    if (!len) {
        LOG("Assemble packet failed");
        return 1;
    }
    int new_pid = packet[42] * 256 + packet[43];
    int new_capid = packet[21] * 256 + packet[22];
    new_pid &= 0x1FFF;
    new_capid &= 0x1FFF;
    if (new_pid != dpid)
        LOG_AND_RETURN(1, "PMT stream pid %04X != mapping table pid %04X",
                       new_pid, dpid);
    if (new_capid != dcapid)
        LOG_AND_RETURN(1, "PMT PSI pid %04X != mapping table pid %04X",
                       new_capid, dcapid);
    free_hash(&d.mapping);
    return 0;
}

int main() {
    opts.log = 1;
    opts.debug = 0;
    strcpy(thread_name, "test");
    find_ddci_adapter(a);
    TEST_FUNC(test_push_ts_to_ddci(), "testing test_push_ts_to_ddci");
    TEST_FUNC(test_copy_ts_from_ddci(), "testing test_copy_ts_from_ddci");
    TEST_FUNC(test_ddci_process_ts(), "testing ddci_process_ts");
    TEST_FUNC(test_create_pat(), "testing create_pat");
    TEST_FUNC(test_create_pmt(), "testing create_pmt");
    fflush(stdout);
    return 0;
}
