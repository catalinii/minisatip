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
#include "tables.h"
#include "utils.h"
#include "utils/testing.h"

#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define DEFAULT_LOG LOG_DVBAPI

extern SPMT *pmts[MAX_PMT];
extern SKey *keys[MAX_KEYS];
extern adapter *a[MAX_ADAPTERS];
extern int dvbapi_is_enabled;
extern int sock;
extern char *get_channel_for_key(int key, char *dest, int max_size);
int dvbapi_add_pmt(adapter *ad, SPMT *pmt);

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

    ASSERT(dvbapi_add_pmt(&ad, &pmt) == TABLES_RESULT_OK, "first add failed");
    SKey *k = (SKey *)pmt.opaque;
    ASSERT(k && k->enabled, "no live key after add");
    ASSERT_EQUAL(count_enabled_keys(), before + 1, "one key expected");
    int demux = k->demux_index;

    pmt.sid = 200;
    ASSERT(dvbapi_add_pmt(&ad, &pmt) == TABLES_RESULT_OK, "re-send failed");
    ASSERT(pmt.opaque == k, "re-send must reuse the live key");
    ASSERT(k->enabled, "reused key must stay enabled");
    ASSERT_EQUAL(k->demux_index, demux, "demux index must stay stable");
    ASSERT_EQUAL(k->sid, 200, "re-send must refresh the key");
    ASSERT_EQUAL(count_enabled_keys(), before + 1, "re-send must add no key");

    // a dead key must fall through to a fresh allocation, not be reused
    k->ver = 42;
    keys_del(k->id);
    ASSERT(dvbapi_add_pmt(&ad, &pmt) == TABLES_RESULT_OK, "re-add failed");
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
    fflush(stdout);
    return 0;
}
