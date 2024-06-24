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
#define _GNU_SOURCE

#include "adapter.h"
#include "minisatip.h"
#include "utils.h"
#include "utils/testing.h"

#include <string.h>
#include <linux/dvb/frontend.h>

int test_get_lnb_hiband() {
  return 0;
}

int test_get_lnb_int_freq_universal() {
  transponder tp;
  diseqc diseqc_param = {
    .lnb_low = 9750000,
    .lnb_high = 10600000,
    .lnb_switch = 11700000
  };

  tp.freq = 10778000;
  int freq = get_lnb_int_freq(&tp, &diseqc_param);
  ASSERT(freq == 1028000, "Universal LNB IF parsed incorrectly");

  tp.freq = 12322000;
  freq = get_lnb_int_freq(&tp, &diseqc_param);
  ASSERT(freq == 1722000, "Universal LNB IF parsed incorrectly");

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

int test_get_lnb_int_freq_cband() {
  transponder tp;
  diseqc diseqc_param = {
    .lnb_low = 5150000,
    .lnb_high = 0,
    .lnb_switch = 0
  };

  tp.freq = 3773000;
  int freq = get_lnb_int_freq(&tp, &diseqc_param);
  ASSERT(freq == 1377000, "C-band LNB IF parsed incorrectly");

  // Should also work with low = high = switch
  diseqc_param.lnb_high = diseqc_param.lnb_switch = 5150000;
  freq = get_lnb_int_freq(&tp, &diseqc_param);
  ASSERT(freq == 1377000, "C-band LNB IF parsed incorrectly");

  return 0;
}

int main() {
  opts.log = 1;
  opts.debug = 255;
  strcpy(thread_info[thread_index].thread_name, "test_adapter");

  TEST_FUNC(test_get_lnb_hiband(), "test test_get_lnb_hiband with universal LNB parameters");
  TEST_FUNC(test_get_lnb_int_freq_universal(), "test get_lnb_int_freq with universal LNB parameters");
  TEST_FUNC(test_get_lnb_int_freq_kuband(), "test get_lnb_int_freq with typical Ku-band linear LNB parameters");
  TEST_FUNC(test_get_lnb_int_freq_cband(), "test get_lnb_int_freq with C-band LNB parameters");

  return 0;
}
