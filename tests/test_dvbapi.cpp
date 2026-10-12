/*
 * Copyright (C) 2026 Dirk Nehring <dnehring@gmx.net>
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */
#include "dvbapi.h"
#include "minisatip.h"
#include "pmt.h"
#include "socketworks.h"
#include "tables.h"
#include "utils.h"
#include "utils/testing.h"
#include "utils/ticks.h"

#include <atomic>
#include <poll.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define DEFAULT_LOG LOG_DVBAPI

extern SPMT *pmts[MAX_PMT];
extern SKey *keys[MAX_KEYS];
extern adapter *a[MAX_ADAPTERS];
extern std::atomic<int> dvbapi_is_enabled;
extern std::atomic<int> sock;
extern std::atomic<int> dvbapi_sock;
extern std::atomic<int> dvbapi_ca;
extern char *get_channel_for_key(int key, char *dest, int max_size);
int dvbapi_add_pmt(adapter *ad, SPMT *pmt, int update);
void dvbapi_close_socket();
int pmt_del(int id);

static int count_enabled_keys() {
    int n = 0;
    for (int i = 0; i < MAX_KEYS; i++)
        if (keys[i] && keys[i]->enabled)
            n++;
    return n;
}

// A PMT update re-send reuses the live key, keeping the demux index
// stable instead of leaking a key per version bump (#1346).
int test_add_pmt_reuses_live_key() {
    SPMT pmt = {};
    adapter ad = {};
    int sv[2];
    ASSERT(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0, "no socketpair");
    int saved_sock = sock;
    sock = sv[0];

    ad.enabled = 1;
    ad.id = 0;
    a[0] = &ad;
    pmt.enabled = 1;
    pmt.id = 0;
    pmt.sid = 100;
    pmt.pid = 48;
    pmts[0] = &pmt;
    npmts = 1;
    int before = count_enabled_keys();

    ASSERT(dvbapi_add_pmt(&ad, &pmt, 0) == TABLES_RESULT_OK,
           "first add failed");
    SKey *k = (SKey *)pmt.opaque;
    ASSERT(k && k->enabled, "no live key after add");
    ASSERT_EQUAL(count_enabled_keys(), before + 1, "one key expected");
    int demux = k->demux_index;

    pmt.sid = 200;
    ASSERT(dvbapi_add_pmt(&ad, &pmt, 1) == TABLES_RESULT_OK, "re-send failed");
    ASSERT(pmt.opaque == k, "re-send must reuse the live key");
    ASSERT(k->enabled, "reused key must stay enabled");
    ASSERT_EQUAL(k->demux_index, demux, "demux index must stay stable");
    ASSERT_EQUAL(k->sid, 200, "re-send must refresh the key");
    ASSERT_EQUAL(count_enabled_keys(), before + 1, "re-send must add no key");

    // a dead key must fall through to a fresh allocation, not be reused
    k->ver = 42;
    keys_del(k->id);
    ASSERT(dvbapi_add_pmt(&ad, &pmt, 0) == TABLES_RESULT_OK, "re-add failed");
    ASSERT_EQUAL(count_enabled_keys(), before + 1, "one key expected again");
    k = (SKey *)pmt.opaque;
    ASSERT(k && k->enabled, "no live key after re-add");
    ASSERT_EQUAL(k->ver, -1, "re-add must allocate a fresh key");

    keys_del(k->id);
    sock = saved_sock;
    close(sv[0]);
    close(sv[1]);
    pmts[0] = NULL;
    npmts = 0;
    a[0] = NULL;
    return 0;
}

// A VideoGuard ECM in iCAM mode `mode`, which both the fixed offset 0x15 and
// the low nibble of the last byte carry; mode 0 is a plain CSA ECM.
static int build_ecm(uint8_t *s, int table, int mode) {
    int len = 0x20;
    memset(s, 0, len);
    s[0] = table;
    s[1] = 0x70;
    s[2] = len - 3;
    s[4] = mode ? s[2] - 4 : 0; // s[2] - s[4] == 4 marks iCAM
    s[0x15] = mode;
    s[len - 1] = mode;
    return len;
}

// OSCam filters the CAT and the SDT through the same key as the ECMs. A CW
// is keyed in the mode of the last ECM, so those tables must not reset it.
int test_icam_mode_only_from_ecms() {
    static const uint8_t cat[] = {0x01, 0xB0, 0x0F, 0xFF, 0xFF, 0xC1,
                                  0x00, 0x00, 0x09, 0x04, 0x09, 0x8C,
                                  0xE0, 0xCC, 0x11, 0x22, 0x33, 0x44};
    static const uint8_t sdt[] = {0x42, 0xF0, 0x0B, 0x00, 0x01, 0xC1, 0x00,
                                  0x00, 0x00, 0x01, 0xFF, 0x11, 0x22, 0x33};
    SPMT pmt = {};
    uint8_t ecm[64];
    int id, fid_ecm, fid_cat, fid_sdt, len;
    SKey *k;

    pmt.enabled = 1;
    pmts[0] = &pmt;
    npmts = 1;
    id = keys_add(-1, 0, 0);
    ASSERT(id >= 0, "no key for the PMT");
    k = keys[id];
    fid_ecm = add_filter(0, 0x1CB6, (void *)send_ecm, k, 0);
    fid_cat = add_filter(0, 0x0001, (void *)send_ecm, k, 0);
    fid_sdt = add_filter(0, 0x0011, (void *)send_ecm, k, 0);
    k->filter_id[0] = fid_ecm;
    k->filter_id[1] = fid_cat;
    k->filter_id[2] = fid_sdt;
    memset(k->ecm_parity, -1, sizeof(k->ecm_parity)); // no ECM seen yet
    dvbapi_is_enabled = 1;

    len = build_ecm(ecm, 0x80, 4);
    send_ecm(fid_ecm, ecm, len, k);
    ASSERT(k->is_icam && k->icam_ecm == 4, "an iCAM ECM did not set the mode");

    send_ecm(fid_cat, (uint8_t *)cat, sizeof(cat), k);
    ASSERT(k->is_icam && k->icam_ecm == 4, "a CAT section reset the iCAM mode");
    send_ecm(fid_sdt, (uint8_t *)sdt, sizeof(sdt), k);
    ASSERT(k->is_icam && k->icam_ecm == 4, "an SDT section reset the mode");

    len = build_ecm(ecm, 0x81, 0);
    send_ecm(fid_ecm, ecm, len, k);
    ASSERT(!k->is_icam && k->icam_ecm == 0, "a plain ECM kept the iCAM mode");

    dvbapi_is_enabled = 0;
    keys_del(id);
    pmts[0] = NULL;
    npmts = 0;
    free_filters();
    return 0;
}

int test_channel_falls_back_to_sid() {
    SPMT pmt = {};
    char dest[64];
    int id;
    SKey *k;

    pmt.enabled = 1;
    pmt.sid = 1234;
    pmts[0] = &pmt;
    npmts = 1;
    id = keys_add(-1, 0, 0);
    ASSERT(id >= 0, "no key for the PMT");
    k = keys[id];
    k->pmt_id = 0;

    // No SDT seen: the name is empty so the channel shows the SID
    get_channel_for_key(id, dest, sizeof(dest));
    ASSERT(strcmp(dest, "SID 1234") == 0,
           "channel must fall back to the SID without SDT");

    // SDT seen: the real name wins over the fallback
    strcpy(pmt.name, "Test Channel");
    get_channel_for_key(id, dest, sizeof(dest));
    ASSERT(strcmp(dest, "Test Channel") == 0,
           "channel must show the SDT name when present");

    keys_del(id);
    pmts[0] = NULL;
    npmts = 0;
    return 0;
}

static int drain(int fd) {
    uint8_t buf[4096];
    int total = 0, n;
    struct pollfd pfd = {fd, POLLIN, 0};
    while (poll(&pfd, 1, 0) > 0) {
        n = read(fd, buf, sizeof(buf));
        if (n <= 0)
            break;
        total += n;
    }
    return total;
}

// An update re-send with identical CAPMT content writes nothing to the
// server; a changed one re-sends (#1346).
int test_update_resend_skips_identical_capmt() {
    SPMT pmt = {};
    adapter ad = {};
    int sv[2];
    ASSERT(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0, "no socketpair");
    int saved_sock = sock;
    sock = sv[0];

    ad.enabled = 1;
    ad.id = 0;
    a[0] = &ad;
    pmt.enabled = 1;
    pmt.id = 0;
    pmt.sid = 100;
    pmt.pid = 48;
    pmts[0] = &pmt;
    npmts = 1;

    ASSERT(dvbapi_add_pmt(&ad, &pmt, 0) == TABLES_RESULT_OK,
           "first add failed");
    SKey *k = (SKey *)pmt.opaque;
    ASSERT(k && k->enabled, "no live key after add");
    ASSERT(drain(sv[1]) > 0, "first add must write the CAPMT");

    ASSERT(dvbapi_add_pmt(&ad, &pmt, 1) == TABLES_RESULT_OK,
           "identical re-send failed");
    ASSERT(drain(sv[1]) == 0, "identical re-send must write nothing");

    // A second live key flips this key's list-management ONLY->ADD;
    // framing alone must not force a re-send either.
    int other = keys_add(-1, 0, 0);
    ASSERT(other >= 0 && keys[other]->enabled, "no second live key");
    ASSERT(dvbapi_add_pmt(&ad, &pmt, 1) == TABLES_RESULT_OK,
           "listmgmt-flip re-send failed");
    ASSERT(drain(sv[1]) == 0, "listmgmt flip must write nothing");
    keys_del(other);
    drain(sv[1]); // discard teardown bytes before the changed re-send

    pmt.sid = 200;
    ASSERT(dvbapi_add_pmt(&ad, &pmt, 1) == TABLES_RESULT_OK,
           "changed re-send failed");
    ASSERT(drain(sv[1]) > 0, "changed re-send must write the CAPMT");

    keys_del(k->id);
    sock = saved_sock;
    close(sv[0]);
    close(sv[1]);
    pmts[0] = NULL;
    npmts = 0;
    a[0] = NULL;
    return 0;
}

// A PMT that decrypted and then gets no CWs is restarted for OSCam: CA_STOP,
// a new key and CAPMT on the next pass, while the PMT itself keeps running.
int test_descramble_watchdog() {
    adapter ad = {};
    int sv[2];
    ASSERT(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0, "no socketpair");
    int saved_sock = sock;
    int saved_enabled = dvbapi_is_enabled;
    sock = sv[0];
    dvbapi_is_enabled = 1;
    ad.enabled = 1;
    ad.id = 0;
    a[0] = &ad;
    register_dvbapi();
    int ca_idx = dvbapi_ca;
    uint64_t dvbapi_mask = 1ULL << ca_idx;
    ASSERT(ad.ca_mask & dvbapi_mask, "dvbapi not on the adapter");

    int id = pmt_add(0, 100, 48);
    SPMT *pmt = get_pmt(id);
    ASSERT(pmt, "no PMT");
    pmt_add_caid(pmt, 0x0100, 0x1000, NULL, 0);
    pmt->state = PMT_RUNNING;
    ad.active_pmt[0] = id;
    ad.active_pmts = 1;
    send_pmt_to_cas(&ad, pmt);
    SKey *k = static_cast<SKey *>(pmt->opaque);
    ASSERT(k && k->enabled, "no live key after the CAPMT");
    drain(sv[1]);

    // getTick() starts at 0 on its first call, and a 0 stamp means never
    while (getTick() < 1)
        usleep(1000);
    int64_t now = getTick();
    k->created = now - 10000; // past the grace of a new key
    pmt->last_scrambled_pkt = now;
    dvbapi_descramble_watchdog(&ad);
    ASSERT_EQUAL(k->stall_start, 0, "a PMT that never decrypted is left alone");

    pmt->last_decrypt_ok = now - 5000;
    dvbapi_descramble_watchdog(&ad);
    ASSERT(k->stall_start > 0, "a stall starts the clock");
    ASSERT(k->enabled, "no restart before the stall time");

    pmt->last_decrypt_ok = now;
    dvbapi_descramble_watchdog(&ad);
    ASSERT_EQUAL(k->stall_start, 0, "a decrypt ends the stall");
    pmt->last_decrypt_ok = now - 5000;

    pmt->last_scrambled_pkt = now - 3000;
    dvbapi_descramble_watchdog(&ad);
    ASSERT_EQUAL(k->stall_start, 0, "no recent scrambled packet, no stall");

    pmt->last_scrambled_pkt = now;
    k->stall_start = now - 4000;
    k->created = now;
    dvbapi_descramble_watchdog(&ad);
    ASSERT(k->enabled, "a new key gets its grace");

    k->created = now - 10000;
    k->pmt_id = id + 1;
    dvbapi_descramble_watchdog(&ad);
    ASSERT(k->enabled, "a key of another PMT is left alone");
    k->pmt_id = id;
    pmt->state = PMT_STARTING;
    dvbapi_descramble_watchdog(&ad);
    ASSERT(k->enabled, "only a running PMT is restarted");
    pmt->state = PMT_RUNNING;
    dvbapi_is_enabled = 0;
    dvbapi_descramble_watchdog(&ad);
    ASSERT(k->enabled, "nothing to restart without OSCam");
    dvbapi_is_enabled = 1;
    dvbapi_ca = -1;
    dvbapi_descramble_watchdog(&ad);
    ASSERT(k->enabled, "nothing to restart once dvbapi is unregistered");
    dvbapi_ca = ca_idx;
    pmt->adapter = 1;
    dvbapi_descramble_watchdog(&ad);
    ASSERT(k->enabled, "a PMT of another adapter is left alone");
    pmt->adapter = 0;
    k->enabled = false;
    dvbapi_descramble_watchdog(&ad);
    ASSERT(pmt->opaque == k, "a key dvbapi deleted is left alone");
    k->enabled = true;

    ASSERT_EQUAL(drain(sv[1]), 0, "nothing sent to OSCam so far");
    dvbapi_descramble_watchdog(&ad);
    ASSERT(!k->enabled && !pmt->opaque, "a stall deletes the dvbapi key");
    ASSERT_EQUAL(pmt->state, PMT_RUNNING, "the PMT keeps running");
    ASSERT(drain(sv[1]) > 0, "OSCam gets a CA_STOP");
    ASSERT(!(pmt->ca_mask & dvbapi_mask), "the next pass resends");

    send_pmt_to_cas(&ad, pmt);
    SKey *k2 = static_cast<SKey *>(pmt->opaque);
    ASSERT(k2 && k2->enabled, "a new key after the restart");
    ASSERT(drain(sv[1]) > 0, "a fresh CAPMT after the restart");

    k2->stall_start = now - 4000;
    k2->created = now - 6000;
    dvbapi_descramble_watchdog(&ad);
    ASSERT(k2->enabled, "a second restart waits twice the grace");
    k2->created = now - 11000;
    dvbapi_descramble_watchdog(&ad);
    ASSERT(!k2->enabled, "the second restart after the doubled grace");

    send_pmt_to_cas(&ad, pmt);
    SKey *k3 = static_cast<SKey *>(pmt->opaque);
    ASSERT(k3 && k3->enabled, "a new key after the second restart");
    pmt->last_decrypt_ok = now - 4500;
    k3->stall_start = now - 4000;
    k3->created = now - 6000;
    dvbapi_descramble_watchdog(&ad);
    ASSERT(!k3->enabled, "a decrypt in between resets the back-off");
    drain(sv[1]);

    start_pmt(pmt, &ad);
    ASSERT(!pmt->last_decrypt_ok && !pmt->last_scrambled_pkt,
           "a started PMT has no stamps");
    pmt->last_decrypt_ok = pmt->last_scrambled_pkt = now;
    unregister_dvbapi();
    pmt_del(id);
    ASSERT_EQUAL(pmt_add(0, 100, 48), id, "the PMT slot is reused");
    ASSERT(!pmt->last_decrypt_ok && !pmt->last_scrambled_pkt,
           "a reused PMT has no stamps");
    pmt_del(id);
    dvbapi_is_enabled = saved_enabled;
    sock = saved_sock;
    close(sv[0]);
    close(sv[1]);
    a[0] = NULL;
    return 0;
}

// A failed OSCam write happens under the adapter lock, so closing only
// marks the socket; the delete runs later, outside that lock.
int test_close_socket_marks_without_deleting() {
    int sv[2];
    ASSERT(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0, "no socketpair");
    int id = sockets_add(sv[0], NULL, -1, TYPE_TCP, NULL, NULL, NULL);
    ASSERT(id >= 0, "could not add socket");
    int saved_dvbapi_sock = dvbapi_sock;
    int saved_enabled = dvbapi_is_enabled;
    dvbapi_sock = id;
    dvbapi_is_enabled = 1;

    dvbapi_close_socket();

    sockets *ss = get_sockets(id);
    ASSERT(ss && ss->force_close, "socket must be marked, not deleted");
    ASSERT(dvbapi_is_enabled == 0, "dvbapi must be disabled");

    sockets_del(id);
    close(sv[1]);
    dvbapi_sock = saved_dvbapi_sock;
    dvbapi_is_enabled = saved_enabled;
    return 0;
}

int main() {
    opts.log = 255;
    opts.debug = 255;
    strcpy(thread_info[thread_index].thread_name, "test_dvbapi");
    TEST_FUNC(test_icam_mode_only_from_ecms(),
              "testing that only ECMs set the iCAM mode");
    TEST_FUNC(test_channel_falls_back_to_sid(),
              "testing the channel falls back to the SID without SDT");
    TEST_FUNC(test_add_pmt_reuses_live_key(),
              "testing a PMT re-send reuses the live key");
    TEST_FUNC(test_update_resend_skips_identical_capmt(),
              "testing an identical re-send writes no CAPMT");
    TEST_FUNC(test_close_socket_marks_without_deleting(),
              "testing close marks the socket without deleting it");
    TEST_FUNC(test_descramble_watchdog(),
              "testing the watchdog restarts a stalled PMT for OSCam");
    fflush(stdout);
    return 0;
}
