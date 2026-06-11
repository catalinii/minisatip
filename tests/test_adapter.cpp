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
    adapter ad = {};
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

int test_compare_slave_parameters() {
    adapter master_ad = {};
    adapter slave_ad = {};

    // Clear global adapter array to ensure clean state
    for (int i = 0; i < MAX_ADAPTERS; i++) {
        a[i] = nullptr;
    }

    // 1. ad is NULL
    ASSERT(compare_slave_parameters(nullptr, nullptr) == 0,
           "compare_slave_parameters with NULL adapter should return 0");

    // Initialize master_ad
    master_ad.id = 0;
    master_ad.enabled = 1;
    master_ad.master_source = -1;
    memset(master_ad.used, 0, sizeof(master_ad.used));
    a[0] = &master_ad;

    // Initialize slave_ad
    slave_ad.id = 1;
    slave_ad.enabled = 1;
    slave_ad.master_source = 0; // slave of master_ad
    memset(slave_ad.used, 0, sizeof(slave_ad.used));
    a[1] = &slave_ad;

    transponder tp;
    auto setup_tp = [](transponder &t) {
        t.clear();
        t.diseqc_param.lnb_low = 9750000;
        t.diseqc_param.lnb_high = 10600000;
        t.diseqc_param.lnb_switch = 11700000;
    };
    setup_tp(tp);

    // 2. master_ad is not a slave and is not marked as used by any slave
    // adapters
    ASSERT(
        compare_slave_parameters(&master_ad, &tp) == 0,
        "Adapter with empty used array and master_source < 0 should return 0");

    // 3. JESS/UNICABLE switch types should always return 0
    slave_ad.diseqc_param.switch_type = SWITCH_JESS;
    tp.pol = 2; // would mismatch but ignored due to JESS
    ASSERT(compare_slave_parameters(&slave_ad, &tp) == 0,
           "UNICABLE/JESS adapter should return 0");
    slave_ad.diseqc_param.switch_type = SWITCH_UNICABLE;
    ASSERT(compare_slave_parameters(&slave_ad, &tp) == 0,
           "UNICABLE/JESS adapter should return 0");

    // Reset switch type
    slave_ad.diseqc_param.switch_type = SWITCH_SLAVE;

    // Configure LNB parameters (Universal LNB)
    slave_ad.diseqc_param.lnb_low = 9750000;
    slave_ad.diseqc_param.lnb_high = 10600000;
    slave_ad.diseqc_param.lnb_switch = 11700000;

    // Configure master adapter parameters
    master_ad.old_pol = 0;    // horizontal/vertical (0)
    master_ad.old_hiband = 0; // lowband (0)
    master_ad.old_diseqc = 0; // diseqc port (0)

    // 4. Test master check with optional parameters

    // No optional parameters set in transponder -> should match
    setup_tp(tp);
    ASSERT(compare_slave_parameters(&slave_ad, &tp) == 0,
           "No parameters set in transponder should not conflict");

    // tp.pol matches master's old_pol
    setup_tp(tp);
    tp.pol = 1; // (*tp.pol - 1) & 1 = 0
    ASSERT(compare_slave_parameters(&slave_ad, &tp) == 0,
           "Matching polarization should return 0");

    // tp.pol = 0 (none/unspecified) maps to pol=1, which conflicts with
    // master's old_pol (0)
    setup_tp(tp);
    tp.pol = 0;
    ASSERT(compare_slave_parameters(&slave_ad, &tp) == 1,
           "Polarization=0 (unspecified) maps to pol=1 and should conflict "
           "with old_pol=0");

    // tp.pol = 0 maps to pol=1, which matches if master's old_pol is 1
    master_ad.old_pol = 1;
    ASSERT(compare_slave_parameters(&slave_ad, &tp) == 0,
           "Polarization=0 (unspecified) maps to pol=1 and should match "
           "old_pol=1");
    master_ad.old_pol = 0; // Restore old_pol to 0

    // tp.pol conflicts with master's old_pol
    setup_tp(tp);
    tp.pol = 2; // (*tp.pol - 1) & 1 = 1
    ASSERT(compare_slave_parameters(&slave_ad, &tp) == 1,
           "Conflicting polarization should return 1");

    // tp.diseqc matches master's old_diseqc
    setup_tp(tp);
    tp.diseqc = 1; // (*tp.diseqc > 0) ? 1 - 1 : 0 = 0
    ASSERT(compare_slave_parameters(&slave_ad, &tp) == 0,
           "Matching diseqc should return 0");

    // tp.diseqc = 0 (none/unspecified) maps to diseqc=0, which matches master's
    // old_diseqc (0)
    setup_tp(tp);
    tp.diseqc = 0;
    ASSERT(compare_slave_parameters(&slave_ad, &tp) == 0,
           "Diseqc=0 (unspecified) maps to diseqc=0 and should match "
           "old_diseqc=0");

    // tp.diseqc = 0 maps to diseqc=0, which conflicts if master's old_diseqc is
    // 1
    master_ad.old_diseqc = 1;
    ASSERT(compare_slave_parameters(&slave_ad, &tp) == 1,
           "Diseqc=0 (unspecified) maps to diseqc=0 and should conflict with "
           "old_diseqc=1");
    master_ad.old_diseqc = 0; // Restore old_diseqc to 0

    // tp.diseqc conflicts with master's old_diseqc
    setup_tp(tp);
    tp.diseqc = 2; // 1
    ASSERT(compare_slave_parameters(&slave_ad, &tp) == 1,
           "Conflicting diseqc should return 1");

    // tp.freq matches master's old_hiband (lowband)
    setup_tp(tp);
    tp.freq = 10778000; // hiband = 0
    ASSERT(compare_slave_parameters(&slave_ad, &tp) == 0,
           "Matching band (lowband) should return 0");

    // tp.freq = 0 (unspecified) should not conflict
    setup_tp(tp);
    tp.freq = 0;
    ASSERT(compare_slave_parameters(&slave_ad, &tp) == 0,
           "Frequency=0 (unspecified) should return 0");

    // tp.freq conflicts with master's old_hiband (hiband)
    setup_tp(tp);
    tp.freq = 12322000; // hiband = 1
    ASSERT(compare_slave_parameters(&slave_ad, &tp) == 1,
           "Conflicting band (hiband) should return 1");

    // 5. Test slave check (when master_ad is used by slave_ad)
    master_ad.used[1] = 1; // master_ad is used by slave_ad (id 1)

    // Set slave adapter parameters
    slave_ad.old_pol = 0;
    slave_ad.old_hiband = 0;
    slave_ad.old_diseqc = 0;

    setup_tp(tp);
    tp.pol = 2; // conflicts with slave_ad (old_pol = 0)
    ASSERT(compare_slave_parameters(&master_ad, &tp) == 1,
           "Conflicting polarization on slave adapter should return 1");

    setup_tp(tp);
    tp.pol = 1; // matches
    ASSERT(compare_slave_parameters(&master_ad, &tp) == 0,
           "Matching polarization on slave adapter should return 0");

    // Reset global array
    a[0] = nullptr;
    a[1] = nullptr;

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
    TEST_FUNC(test_compare_slave_parameters(),
              "test compare_slave_parameters with std::optional");

    return 0;
}
