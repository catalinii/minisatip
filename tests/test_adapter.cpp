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
#include "minisatip.h"
#include "utils.h"
#include "utils/testing.h"

#include <linux/dvb/frontend.h>
#include <string.h>

int test_get_lnb_hiband_universal() {
    transponder tp;
    diseqc diseqc_param = {
        .lnb_low = 9750000, .lnb_high = 10600000, .lnb_switch = 11700000};

    tp.freq = 10778000;
    int hiband = get_lnb_hiband(&tp, &diseqc_param);
    ASSERT(hiband == 0, "Universal LNB hiband parsed incorrectly");

    tp.freq = 12322000;
    hiband = get_lnb_hiband(&tp, &diseqc_param);
    ASSERT(hiband == 1, "Universal LNB hiband parsed incorrectly");

    return 0;
}

int test_get_lnb_int_freq_universal() {
    transponder tp;
    diseqc diseqc_param = {
        .lnb_low = 9750000, .lnb_high = 10600000, .lnb_switch = 11700000};

    tp.freq = 10778000;
    int freq = get_lnb_int_freq(&tp, &diseqc_param);
    ASSERT(freq == 1028000, "Universal LNB IF parsed incorrectly");

    tp.freq = 12322000;
    freq = get_lnb_int_freq(&tp, &diseqc_param);
    ASSERT(freq == 1722000, "Universal LNB IF parsed incorrectly");

    return 0;
}

int test_get_lnb_hiband_kuband() {
    transponder tp;
    diseqc diseqc_param = {
        .lnb_low = 10750000,
        .lnb_high = 0,
        .lnb_switch = 0,
    };

    tp.freq = 12267000;
    int hiband = get_lnb_hiband(&tp, &diseqc_param);
    ASSERT(hiband == 0, "Ku-band LNB hiband parsed incorrectly");

    return 0;
}

int test_get_lnb_int_freq_kuband() {
    transponder tp;
    diseqc diseqc_param = {
        .lnb_low = 10750000,
        .lnb_high = 0,
        .lnb_switch = 0,
    };

    tp.freq = 12267000;
    int freq = get_lnb_int_freq(&tp, &diseqc_param);
    ASSERT(freq == 1517000, "Ku-band LNB IF parsed incorrectly");

    return 0;
}

int test_get_lnb_hiband_cband() {
    transponder tp;
    diseqc diseqc_param = {
        .lnb_low = 5150000,
        .lnb_high = 0,
        .lnb_switch = 0,
    };

    tp.freq = 3773000;
    int hiband = get_lnb_hiband(&tp, &diseqc_param);
    ASSERT(hiband == 0, "C-band LNB hiband parsed incorrectly");

    return 0;
}

int test_get_lnb_int_freq_cband() {
    transponder tp;
    diseqc diseqc_param = {.lnb_low = 5150000, .lnb_high = 0, .lnb_switch = 0};

    tp.freq = 3773000;
    int freq = get_lnb_int_freq(&tp, &diseqc_param);
    ASSERT(freq == 1377000, "C-band LNB IF parsed incorrectly");

    // Should also work with low = high = switch
    diseqc_param.lnb_high = diseqc_param.lnb_switch = 5150000;
    freq = get_lnb_int_freq(&tp, &diseqc_param);
    ASSERT(freq == 1377000, "C-band LNB IF parsed incorrectly");

    return 0;
}

int test_update_pids() {
    adapter ad;
    a[0] = &ad;
    ad.enabled = 1;
    for (int i = 0; i < 8; i++) {
        ad.pids[i].flags = i % 4;
        ad.pids[i].pid = i;
        ad.pids[i].packets = i;
    }
    update_pids(0);
    ASSERT(ad.pids[0].pid == 5,
           "Expected pid with flags = 1 and largest number of packets");
    ASSERT(ad.pids[1].pid == 1,
           "Expected pid with flags = 1 and second largest number of packets");
    ASSERT(ad.pids[2].pid == 2 && ad.pids[2].packets == 0,
           "Expected pid with flags = 1 and 0 packets");
    ASSERT(ad.pids[3].pid == 6 && ad.pids[2].packets == 0,
           "Expected pid with flags = 1 and 0 packets");
    ASSERT(ad.pids[4].pid == 0 && ad.pids[4].flags == 0,
           "Expected pid with flags = 0");
    return 0;
}

int main() {
    opts.log = 1;
    opts.debug = 255;
    strcpy(thread_info[thread_index].thread_name, "test_adapter");

    TEST_FUNC(test_get_lnb_hiband_universal(),
              "test test_get_lnb_hiband with universal LNB parameters");
    TEST_FUNC(test_get_lnb_int_freq_universal(),
              "test get_lnb_int_freq with universal LNB parameters");
    TEST_FUNC(test_get_lnb_hiband_kuband(),
              "test get_lnb_hiband with Ku-band linear LNB parameters");
    TEST_FUNC(
        test_get_lnb_int_freq_kuband(),
        "test get_lnb_int_freq with typical Ku-band linear LNB parameters");
    TEST_FUNC(test_get_lnb_hiband_cband(),
              "test get_lnb_hiband with C-band linear LNB parameters");
    TEST_FUNC(test_get_lnb_int_freq_cband(),
              "test get_lnb_int_freq with C-band LNB parameters");
    TEST_FUNC(test_update_pids(), "test update_pids and sort_pids");

    return 0;
}
