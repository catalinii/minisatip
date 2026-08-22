/*
   - * Copyright (C) 2014-2020 Catalin Toda <catalinii@yahoo.com>
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
#include "pmt.h"
#include "socketworks.h"
#include "tables.h"
#include "utils.h"
#include <arpa/inet.h>
#include <ctype.h>
#ifdef __cplusplus
extern "C" {
#endif
#include <dvbcsa/dvbcsa.h>
#ifdef __cplusplus
}
#endif
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_LOG LOG_DVBCA

// libdvbcsa is a C library, so this has to be declared with C linkage. An
// ICAM patched libdvbcsa declares it in dvbcsa.h and the extern "C" there
// applies, but building against a stock libdvbcsa leaves this declaration
// on its own, and a C++ mangled weak reference never binds to the C symbol
// even when an ICAM capable libdvbcsa.so is loaded at run time.
extern "C" void dvbcsa_bs_key_set_ecm(unsigned char ecm, const dvbcsa_cw_t cw,
                                      struct dvbcsa_bs_key_s *key)
    __attribute__((weak));
void dvbcsa_create_key(SCW *cw) { cw->key = dvbcsa_bs_key_alloc(); }

void dvbcsa_delete_key(SCW *cw) { dvbcsa_key_free((dvbcsa_key_s *)cw->key); }

void dvbcsa_set_cw(SCW *cw, SPMT *pmt) {
    dvbcsa_bs_key_set((unsigned char *)cw->cw, (dvbcsa_bs_key_s *)cw->key);
}

void dvbcsa_set_cw_icam(SCW *cw, SPMT *pmt) {
    unsigned char ecm = cw->opaque ? *(unsigned char *)cw->opaque : 0;
    if (!dvbcsa_bs_key_set_ecm) {
        dvbcsa_bs_key_set((unsigned char *)cw->cw, (dvbcsa_bs_key_s *)cw->key);
        LOGL(1, "minisatip required libdvbcsa with ICAM support. Please "
                "see https://github.com/catalinii/minisatip/issues/1003 "
                "for more details");
    } else {
        dvbcsa_bs_key_set_ecm(ecm, (unsigned char *)cw->cw,
                              (struct dvbcsa_bs_key_s *)cw->key);
    }
}

void copy_batch(struct dvbcsa_bs_batch_s *d, SPMT_batch *s, int len) {
    int i = 0;
    for (i = 0; i < len; i++) {
        d[i].data = s[i].data;
        d[i].len = s[i].len;
    }
    d[i].data = 0;
    d[i].len = 0;
}

void dvbcsa_decrypt_stream(SCW *cw, SPMT_batch *batch, int batch_len) {
    int batch_size = dvbcsa_bs_batch_size();
    struct dvbcsa_bs_batch_s b[batch_size + 1];
    int i;
    for (i = 0; i < batch_len; i += batch_size) {
        int len = batch_len - i;
        if (len > batch_size)
            len = batch_size;
        copy_batch(b, batch + i, len);
        dvbcsa_bs_decrypt((const struct dvbcsa_bs_key_s *)cw->key,
                          (const struct dvbcsa_bs_batch_s *)b, 184);
    }
}

SCW_op csa_op = {.algo = CA_ALGO_DVBCSA,
                 .create_cw = (Create_CW)dvbcsa_create_key,
                 .delete_cw = (Delete_CW)dvbcsa_delete_key,
                 .set_cw = (Set_CW)dvbcsa_set_cw,
                 .stop_cw = NULL,
                 .decrypt_stream = (Decrypt_Stream)dvbcsa_decrypt_stream};

SCW_op csa_icam_op = {.algo = CA_ALGO_DVBCSA_ICAM,
                      .create_cw = (Create_CW)dvbcsa_create_key,
                      .delete_cw = (Delete_CW)dvbcsa_delete_key,
                      .set_cw = (Set_CW)dvbcsa_set_cw_icam,
                      .stop_cw = NULL,
                      .decrypt_stream = (Decrypt_Stream)dvbcsa_decrypt_stream};

void init_algo_csa() {
    register_algo(&csa_op);
    if (dvbcsa_bs_key_set_ecm) {
        register_algo(&csa_icam_op);
    } else {
        LOG("libdvbcsa does not support ICAM (dvbcsa_bs_key_set_ecm missing)");
    }
}
