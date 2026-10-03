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
#include "utils.h"
#include "utils/testing.h"

#include <linux/dvb/frontend.h>
#include <string.h>

extern int dvb_lock_retry_due(adapter *ad, int64_t now);

int test_detect_dvb_parameters_general() {
    transponder tp;
    char query[128] = "?fe=0&src=1&freq=11361.75&pol=h&ro=0.35&msys=dvbs2&"
                      "mtype=qpsk&plts=off&sr=22000&fec=23&pids=0,16,201,302";
    detect_dvb_parameters(query, &tp);

    ASSERT(tp.fe == 0, "fe parsed incorrectly");
    ASSERT(tp.diseqc == 1, "src parsed incorrectly");
    ASSERT(tp.freq == 11361750, "freq parsed incorrectly");
    ASSERT(tp.pol == 2, "pol parsed incorrectly");
    ASSERT(tp.ro == ROLLOFF_35, "ro parsed incorrectly");
    ASSERT(tp.sys == SYS_DVBS2, "msys parsed incorrectly");
    ASSERT(tp.mtype == 0, "mtype parsed incorrectly");
    ASSERT(tp.plts == 1, "plts parsed incorrectly");
    ASSERT(tp.sr == 22000000, "sr parsed incorrectly");
    ASSERT(tp.fec == 2, "fec parsed incorrectly");
    ASSERT(tp.pids.size() == 4 && tp.pids.contains(0) && tp.pids.contains(16) &&
               tp.pids.contains(201) && tp.pids.contains(302),
           "pids parsed incorrectly");

    return 0;
}

int test_detect_dvb_parameters_roll_off() {
    transponder tp;

    // Undefined (should remain nullopt)
    char query[128] = "?freq=11734";
    detect_dvb_parameters(query, &tp);
    ASSERT(tp.ro == std::nullopt, "ro parsed incorrectly");

    // Various recognized values
    strcpy(query, "?freq=11734&ro=0.35");
    detect_dvb_parameters(query, &tp);
    ASSERT(tp.ro == ROLLOFF_35, "ro parsed incorrectly");
    strcpy(query, "?freq=11734&ro=0.25");
    detect_dvb_parameters(query, &tp);
    ASSERT(tp.ro == ROLLOFF_25, "ro parsed incorrectly");
    strcpy(query, "?freq=11734&ro=0.20");
    detect_dvb_parameters(query, &tp);
    ASSERT(tp.ro == ROLLOFF_20, "ro parsed incorrectly");

    return 0;
}

int test_detect_dvb_parameters_edge_cases() {
    transponder tp;

    // Test pids=all mapping to 8192
    char query1[128] = "?freq=11362&pids=all";
    detect_dvb_parameters(query1, &tp);
    ASSERT(tp.pids.size() == 1 && tp.pids.contains(8192),
           "pids=all parsed incorrectly");

    // Test pids=none mapping to empty set
    char query2[128] = "?freq=11362&pids=none";
    detect_dvb_parameters(query2, &tp);
    ASSERT(tp.pids.empty(), "pids=none parsed incorrectly");

    // Test addpids, delpids, and x_pmt
    tp.pids.clear();
    tp.x_pmt = std::nullopt;
    char query3[128] = "?pids=1,2,3&addpids=4,5&delpids=2&x_pmt=123";
    detect_dvb_parameters(query3, &tp);
    ASSERT(tp.pids.size() == 4 && tp.pids.contains(1) && !tp.pids.contains(2) &&
               tp.pids.contains(3) && tp.pids.contains(4) &&
               tp.pids.contains(5),
           "pids/addpids/delpids parsed incorrectly");
    ASSERT(tp.x_pmt == 123, "x_pmt parsed incorrectly");

    // Test pls_mode and pls_code with PLS_MODE_ROOT
    char query4[128] = "?freq=11362&plsm=root&plsc=42";
    detect_dvb_parameters(query4, &tp);
    ASSERT(tp.pls_mode == PLS_MODE_ROOT, "pls_mode parsed incorrectly");
    ASSERT(tp.pls_code == 114384, "pls_code not calculated for ROOT mode");

    return 0;
}

int test_lock_retry_due() {
    adapter ad = {};
    ad.fe = 5;
    ad.sid_cnt = 1;
    ad.status = 0;
    ad.master_source = -1;
    ad.tp.sys = SYS_DVBS2;

    ASSERT(dvb_lock_retry_due(&ad, 5000),
           "retry is due when unlocked 5s after the tune");
    ASSERT(!dvb_lock_retry_due(&ad, 1500),
           "retry waits 2s after the tune before the first resend");

    ad.status = FE_HAS_LOCK;
    ASSERT(!dvb_lock_retry_due(&ad, 5000), "no retry once locked");
    ad.status = 0;

    ad.sid_cnt = 0;
    ASSERT(!dvb_lock_retry_due(&ad, 5000), "no retry with no viewers");
    ad.sid_cnt = 1;

    ad.lock_retries = 5;
    ASSERT(!dvb_lock_retry_due(&ad, 5000), "retries give up after the cap");
    ad.lock_retries = 0;

    ad.master_source = 0;
    ASSERT(!dvb_lock_retry_due(&ad, 5000), "slaves never resend the switch");
    ad.master_source = -1;

    ad.tp.diseqc_param.switch_type = SWITCH_SLAVE;
    ASSERT(!dvb_lock_retry_due(&ad, 5000), "slave switch type never resends");
    ad.tp.diseqc_param.switch_type = 0;

    ad.tp.diseqc_param.fast = 1;
    ASSERT(!dvb_lock_retry_due(&ad, 5000), "fast diseqc mode opts out");
    ad.tp.diseqc_param.fast = 0;

    ad.tp.sys = SYS_DVBT;
    ASSERT(!dvb_lock_retry_due(&ad, 5000), "no resend for non-SAT systems");
    ad.tp.sys = SYS_DVBS;
    ASSERT(dvb_lock_retry_due(&ad, 5000), "DVB-S retries like DVB-S2");

    ad.fe = -1;
    ASSERT(!dvb_lock_retry_due(&ad, 5000), "no retry without frontend");
    return 0;
}

int main() {
    opts.log = 1;
    opts.debug = 255;
    strcpy(thread_info[thread_index].thread_name, "test_dvb");

    TEST_FUNC(test_detect_dvb_parameters_general(),
              "test detect_dvb_parameters with general parameters");
    TEST_FUNC(test_detect_dvb_parameters_roll_off(),
              "test detect_dvb_parameters roll-off parsing");
    TEST_FUNC(test_detect_dvb_parameters_edge_cases(),
              "test detect_dvb_parameters edge cases");
    TEST_FUNC(test_lock_retry_due(), "test lock retry decision matrix");
    return 0;
}
