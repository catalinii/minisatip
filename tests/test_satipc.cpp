/*
 * Copyright (C) 2014-2022 Catalin Toda <catalinii@yahoo.com>
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
#include "dvb.h"
#include "minisatip.h"
#include "satipc.h"
#include "utils.h"
#include "utils/testing.h"

#include <linux/dvb/frontend.h>
#include <string.h>

int test_get_s2_url_multistream_isi() {
    adapter ad = {};
    init_dvb_parameters(&ad.tp);

    satipc sip = {};
    sip.enabled = 1;
    sip.satip_fe = 0;
    ad.id = 0;
    satip[0] = &sip;

    ad.tp.diseqc = 1;
    ad.tp.freq = 11362000;
    ad.tp.sys = SYS_DVBS2;
    ad.tp.mtype = QPSK;
    ad.tp.pol = 2; // H
    ad.tp.sr = 22000000;
    ad.tp.fec = FEC_2_3;
    ad.tp.ro = ROLLOFF_35;
    ad.tp.plts = PILOT_OFF;
    ad.tp.plp_isi = 3;

    char url[1000];
    get_s2_url(&ad, url, sizeof(url));

    ASSERT(strstr(url, "&isi=3") != NULL,
           "ISI parameter missing from proxy URL");

    satip[0] = NULL;
    return 0;
}

int test_get_s2_url_pls_mode_root() {
    adapter ad = {};
    init_dvb_parameters(&ad.tp);

    satipc sip = {};
    sip.enabled = 1;
    sip.satip_fe = 0;
    ad.id = 0;
    satip[0] = &sip;

    ad.tp.diseqc = 1;
    ad.tp.freq = 11362000;
    ad.tp.sys = SYS_DVBS2;
    ad.tp.mtype = QPSK;
    ad.tp.pol = 2;
    ad.tp.sr = 22000000;
    ad.tp.fec = FEC_2_3;
    ad.tp.ro = ROLLOFF_35;
    ad.tp.plts = PILOT_OFF;
    ad.tp.plp_isi = 0;
    ad.tp.pls_mode = PLS_MODE_ROOT;
    ad.tp.pls_code = 42;

    char url[1000];
    get_s2_url(&ad, url, sizeof(url));

    ASSERT(strstr(url, "&plsm=root") != NULL,
           "PLS mode ROOT missing from proxy URL");
    ASSERT(strstr(url, "&plsc=42") != NULL,
           "PLS code missing from proxy URL when pls_mode is ROOT");

    satip[0] = NULL;
    return 0;
}

int test_get_s2_url_pls_mode_gold() {
    adapter ad = {};
    init_dvb_parameters(&ad.tp);

    satipc sip = {};
    sip.enabled = 1;
    sip.satip_fe = 0;
    ad.id = 0;
    satip[0] = &sip;

    ad.tp.diseqc = 1;
    ad.tp.freq = 11362000;
    ad.tp.sys = SYS_DVBS2;
    ad.tp.mtype = QPSK;
    ad.tp.pol = 2;
    ad.tp.sr = 22000000;
    ad.tp.fec = FEC_2_3;
    ad.tp.ro = ROLLOFF_35;
    ad.tp.plts = PILOT_OFF;
    ad.tp.plp_isi = 5;
    ad.tp.pls_mode = PLS_MODE_GOLD;
    ad.tp.pls_code = 100;

    char url[1000];
    get_s2_url(&ad, url, sizeof(url));

    ASSERT(strstr(url, "&isi=5") != NULL,
           "ISI parameter missing from proxy URL");
    ASSERT(strstr(url, "&plsm=gold") != NULL,
           "PLS mode GOLD missing from proxy URL");
    ASSERT(strstr(url, "&plsc=100") != NULL,
           "PLS code missing from proxy URL when pls_mode is GOLD");

    satip[0] = NULL;
    return 0;
}

int test_get_s2_url_no_multistream() {
    adapter ad = {};
    init_dvb_parameters(&ad.tp);

    satipc sip = {};
    sip.enabled = 1;
    sip.satip_fe = 0;
    ad.id = 0;
    satip[0] = &sip;

    ad.tp.diseqc = 1;
    ad.tp.freq = 11362000;
    ad.tp.sys = SYS_DVBS2;
    ad.tp.mtype = QPSK;
    ad.tp.pol = 2;
    ad.tp.sr = 22000000;
    ad.tp.fec = FEC_2_3;
    ad.tp.ro = ROLLOFF_35;
    ad.tp.plts = PILOT_OFF;
    // plp_isi, pls_mode, pls_code left as TP_VALUE_UNSET from init

    char url[1000];
    get_s2_url(&ad, url, sizeof(url));

    ASSERT(strstr(url, "isi") == NULL,
           "ISI should not appear when unset");
    ASSERT(strstr(url, "plsm") == NULL,
           "PLS mode should not appear when unset");
    ASSERT(strstr(url, "plsc") == NULL,
           "PLS code should not appear when unset");

    satip[0] = NULL;
    return 0;
}

int main() {
    opts.log = 1;
    opts.debug = 255;
    strcpy(thread_info[thread_index].thread_name, "test_satipc");

    TEST_FUNC(test_get_s2_url_multistream_isi(),
              "test get_s2_url forwards ISI parameter");
    TEST_FUNC(test_get_s2_url_pls_mode_root(),
              "test get_s2_url forwards PLS code with ROOT mode");
    TEST_FUNC(test_get_s2_url_pls_mode_gold(),
              "test get_s2_url forwards PLS code with GOLD mode");
    TEST_FUNC(test_get_s2_url_no_multistream(),
              "test get_s2_url omits multistream params when unset");

    return 0;
}
