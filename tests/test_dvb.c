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

#include "dvb.h"
#include "minisatip.h"
#include "utils.h"

#include <string.h>
#include <linux/dvb/frontend.h>

int test_detect_dvb_parameters_general() {
  transponder tp;
  char query[128] = "?fe=0&src=1&freq=11361.75&pol=h&ro=0.35&msys=dvbs2&mtype=qpsk&plts=off&sr=22000&fec=23&pids=0,16,201,302";
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
  ASSERT(strcmp(tp.pids, "0,16,201,302") == 0, "pids parsed incorrectly");

  return 0;
}

int test_detect_dvb_parameters_roll_off() {
  transponder tp;

  // Undefined (should default to "auto")
  char query[128] = "?freq=11734";
  detect_dvb_parameters(query, &tp);
  ASSERT(tp.ro == ROLLOFF_AUTO, "ro parsed incorrectly");

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

int main() {
  opts.log = 1;
  opts.debug = 255;

  TEST_FUNC(test_detect_dvb_parameters_general(), "test detect_dvb_parameters with general parameters");
  TEST_FUNC(test_detect_dvb_parameters_roll_off(), "test detect_dvb_parameters roll-off parsing");
  return 0;
}
