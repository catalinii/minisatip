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

#include "httpc.h"
#include <cstring>

extern void satip_getxml_data(char *data, int len, void *opaque,
                              Shttp_client *h);
extern void satipc_get_pids(adapter *ad, satipc *sip, char *url, int size,
                            int send_pids);

int test_get_s2_url_multistream_isi() {
    adapter ad = {};
    ad.tp.clear();

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
    ad.tp.clear();

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
    ad.tp.clear();

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
    ad.tp.clear();

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
    // plp_isi, pls_mode, pls_code left as std::nullopt

    char url[1000];
    get_s2_url(&ad, url, sizeof(url));

    ASSERT(strstr(url, "isi") == NULL, "ISI should not appear when unset");
    ASSERT(strstr(url, "plsm") == NULL,
           "PLS mode should not appear when unset");
    ASSERT(strstr(url, "plsc") == NULL,
           "PLS code should not appear when unset");

    satip[0] = NULL;
    return 0;
}

int test_satip_getxml_data_parsing() {
    Ssatip_xml_data s = {};
    strcpy(s.xml, "<satip:X_SATIPCAP>DVBS2-4,DVBT-2</satip:X_SATIPCAP>");
    strcpy(s.url, "http://1.2.3.4/desc.xml");

    Shttp_client h = {};
    strcpy(h.host, "1.2.3.4");

    satip_getxml_data(NULL, 0, &s, &h);

    ASSERT(s.tuners[SYS_DVBS2] == 4, "Expected 4 tuners for DVBS2");
    ASSERT(s.tuners[SYS_DVBT] == 2, "Expected 2 tuners for DVBT");

    return 0;
}

int test_satipc_get_pids_add_and_del() {
    adapter ad = {};
    ad.id = 0; // no adapter registered, get_adapter_pids returns ""
    char url[1000];

    // combined add + delete must separate the parameters with '&'
    satipc sip_add_del = {};
    sip_add_del.lap = 1;
    sip_add_del.apid[0] = 202;
    sip_add_del.ldp = 2;
    sip_add_del.dpid[0] = 201;
    sip_add_del.dpid[1] = 213;
    url[0] = 0;
    satipc_get_pids(&ad, &sip_add_del, url, sizeof(url), 0);
    ASSERT(strcmp(url, "addpids=202&delpids=201,213") == 0,
           "addpids and delpids parameters must be separated by '&'");
    ASSERT(sip_add_del.lap == 0 && sip_add_del.ldp == 0,
           "pending add/del pids must be cleared after building the URL");

    // add-only must not gain a separator
    satipc sip_add = {};
    sip_add.lap = 2;
    sip_add.apid[0] = 2211;
    sip_add.apid[1] = 2212;
    url[0] = 0;
    satipc_get_pids(&ad, &sip_add, url, sizeof(url), 0);
    ASSERT(strcmp(url, "addpids=2211,2212") == 0,
           "add-only URL must be plain addpids=");

    // del-only must not gain a leading separator
    satipc sip_del = {};
    sip_del.ldp = 1;
    sip_del.dpid[0] = 18;
    url[0] = 0;
    satipc_get_pids(&ad, &sip_del, url, sizeof(url), 0);
    ASSERT(strcmp(url, "delpids=18") == 0,
           "del-only URL must be plain delpids=");

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
    TEST_FUNC(test_satip_getxml_data_parsing(),
              "test satip_getxml_data parses satipcap delivery systems");
    TEST_FUNC(test_satipc_get_pids_add_and_del(),
              "test satipc_get_pids separates addpids/delpids with '&'");

    return 0;
}
