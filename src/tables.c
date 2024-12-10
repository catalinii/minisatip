/*
 * Copyright (C) 2014-2020 Catalin Toda <catalinii@yahoo.com>
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

#include "tables.h"
#include "adapter.h"
#include "dvb.h"
#include "dvbapi.h"
#include "minisatip.h"
#include <arpa/inet.h>
#include <ctype.h>
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

#ifndef DISABLE_DDCI
#include "ddci.h"
#endif

#ifndef DISABLE_DVBCA
#include "ca.h"
#endif

#define DEFAULT_LOG LOG_TABLES

SCA ca[MAX_CA];
int nca;
SMutex ca_mutex;

int add_ca(SCA_op *op, uint64_t adapter_mask) {
    int i, new_ca;
    del_ca(op);
    for (i = 0; i < MAX_CA; i++)
        if (!ca[i].enabled) {
            mutex_lock(&ca_mutex);
            if (!ca[i].enabled)
                break;
            mutex_unlock(&ca_mutex);
        }
    if (i == MAX_CA)
        LOG_AND_RETURN(0, "No _free CA slots for %p", ca);
    new_ca = i;

    ca[new_ca].enabled = 1;
    ca[new_ca].adapter_mask = adapter_mask;
    ca[new_ca].id = new_ca;
    ca[new_ca].op = op;
    memset(ca[new_ca].ad_info, 0, sizeof(ca[new_ca].ad_info));

    if (new_ca >= nca)
        nca = new_ca + 1;

    init_ca_device(&ca[new_ca]);
    mutex_unlock(&ca_mutex);
    return new_ca;
}
extern SPMT *pmts[];
void del_ca(SCA_op *op) {
    int i, k, mask = 1;
    adapter *ad;
    mutex_lock(&ca_mutex);

    for (i = 0; i < MAX_CA; i++) {
        if (ca[i].enabled) {
            if (ca[i].op == op) {
                ca[i].enabled = 0;
                for (k = 0; k < MAX_ADAPTERS;
                     k++) // delete ca_mask for all adapters
                {
                    if ((ad = get_adapter_nw(k)))
                        ad->ca_mask &= ~mask;
                }
                for (k = 0; k < MAX_PMT; k++) // delete ca_mask for all the PMTs
                    if (pmts[k] && pmts[k]->enabled &&
                        (pmts[k]->ca_mask & mask))
                        pmts[k]->ca_mask &= ~mask;
            }
        }
        mask = mask << 1;
    }
    i = MAX_CA;
    while (--i >= 0 && !ca[i].enabled)
        ;
    nca = i + 1;

    //	if (nca == 1)
    //		nca = 0;

    mutex_unlock(&ca_mutex);
}

void tables_ca_ts(adapter *ad) {
    int i, mask = 1;

    for (i = 0; i < nca; i++) {
        if (ca[i].enabled && (ad->ca_mask & mask) && ca[i].op->ca_ts) {
            ca[i].op->ca_ts(ad);
        }
        mask = mask << 1;
    }
}

void add_caid_mask(int ica, int aid, int caid, int mask) {
    int i;
    adapter *ad = get_adapter(aid);
    if (!ad) {
        LOG("%s: No adapter %d found ", __FUNCTION__, aid);
        return;
    }
    if (ca[ica].enabled && ca[ica].ad_info[aid].caids < MAX_CAID) {
        for (i = 0; i < ca[ica].ad_info[aid].caids; i++)
            if (ca[ica].ad_info[aid].caid[i] == caid &&
                ca[ica].ad_info[aid].mask[i] == mask)
                return;
        i = ca[ica].ad_info[aid].caids++;
        ca[ica].ad_info[aid].caid[i] = caid;
        ca[ica].ad_info[aid].mask[i] = mask;
        LOG("CA %d can handle CAID %04X mask %04X on adapter %d at position %d",
            ica, caid, mask, ad->id, i);
    } else
        LOG("CA not enabled %d or too many added CAIDs %d", ica,
            ca[ica].ad_info[aid].caids);
}

int tables_init_ca_for_device(int i, adapter *ad) {
    uint64_t mask = (1ULL << i);
    int rv = 0;
    if (i < 0 || i >= nca)
        return 0;

    if ((ca[i].adapter_mask & mask) &&
        !(ad->ca_mask & mask)) // CA registered and not already initialized
    {
        if (ca[i].enabled && ca[i].op->ca_init_dev) {
            if (ca[i].op->ca_init_dev(ad) == TABLES_RESULT_OK) {
                LOGM("CA %d will handle adapter %d", i, ad->id);
                ad->ca_mask = ad->ca_mask | mask;
                rv = 1;
            } else
                LOGM("CA %d is disabled for adapter %d", i, ad->id);
        }
    }
    return rv;
}

int match_caid(SPMT *pmt, int caid, int mask) {
    int i;
    for (i = 0; i < pmt->caids; i++)
        if ((pmt->ca[i]->id & mask) == caid) {
            LOGM("%s: match caid %04X (%d/%d) with CA caid %04X and mask %04X",
                 __FUNCTION__, pmt->ca[i]->id, i, pmt->caids, caid, mask);
            return 1;
        }
    return 0;
}

// return 1 if CA can handle this specific CAID on the specified adapter
int match_ca_caid(int ica, int aid, int caid) {
    int i;
    // no CAID added - it means it can handle all CAIDs
    if (ca[ica].ad_info[aid].caids == 0)
        return 1;
    for (i = 0; i < ca[ica].ad_info[aid].caids; i++) {
        if (ca[ica].ad_info[aid].caid[i] ==
            (caid & ca[ica].ad_info[aid].mask[i]))
            return 1;
    }
    return 0;
}

void close_pmt_for_ca(int i, adapter *ad, SPMT *pmt) {
    uint64_t mask = 1ULL << i;
    if (!ad)
        ad = get_adapter(pmt->adapter);
    if (!ad)
        return;
    if (ca[i].enabled && (ad->ca_mask & mask) && (pmt->ca_mask & mask)) {
        LOGM("Closing pmt %d for ca %d and adapter %d", pmt->id, i, ad->id);
        if (ad && ca[i].op->ca_del_pmt)
            ca[i].op->ca_del_pmt(ad, pmt);
        pmt->ca_mask &= ~mask;
    }
}

int close_pmt_for_cas(adapter *ad, SPMT *pmt) {
    int i;
    if (!pmt || !pmt->ca_mask)
        return 0;

    if (!ad)
        return 0;

    LOGM("Closing pmt %d for adapter %d", pmt->id, ad->id);
    for (i = 0; i < nca; i++)
        if (ca[i].enabled)
            close_pmt_for_ca(i, ad, pmt);
    return 0;
}

void disable_pmt_for_ca(int i, SPMT *pmt) {
    uint64_t mask = 1ULL << i;
    if (i >= MAX_CA || i < 0)
        return;
    if (ca[i].enabled) {
        close_pmt_for_ca(i, NULL, pmt);
        pmt->disabled_ca_mask |= mask;
    }
}

int send_pmt_to_ca(int i, adapter *ad, SPMT *pmt) {
    uint64_t mask;
    int rv = 0, result = 0;
    mask = 1ULL << i;

    if (ca[i].enabled && (ad->ca_mask & mask) && ca[i].op->ca_add_pmt &&
        !(pmt->disabled_ca_mask & mask) && !(pmt->ca_mask & mask)) {
        int j, send = 0;
        for (j = 0; j < ca[i].ad_info[ad->id].caids; j++)
            if (match_caid(pmt, ca[i].ad_info[ad->id].caid[j],
                           ca[i].ad_info[ad->id].mask[j])) {
                LOG("CAID %04X and mask %04X matched PMT %d",
                    ca[i].ad_info[ad->id].caid[j],
                    ca[i].ad_info[ad->id].mask[j], pmt->id);
                send = 1;
                break;
            }
        result = TABLES_RESULT_ERROR_NORETRY;
        if (send || ca[i].ad_info[ad->id].caids == 0) {
            result = ca[i].op->ca_add_pmt(ad, pmt);
        }

        if (result == TABLES_RESULT_OK) {
            pmt->ca_mask |= mask;
        } else if (result == TABLES_RESULT_ERROR_NORETRY)
            pmt->disabled_ca_mask |= mask;
        disable_cw(pmt->id);
        rv += (1 - result);
        LOGM("In processing PMT %d, ca %d, CA matched %d, ca_pmt_add "
             "returned %d, new ca_mask %d new disabled_ca_mask %d",
             pmt->id, i, send, result, pmt->ca_mask, pmt->disabled_ca_mask);
    }
    return rv;
}

int send_pmt_to_cas(adapter *ad, SPMT *pmt) {
    int i, rv = 1;
    if (!ad || (ad->ca_mask == (pmt->disabled_ca_mask | pmt->ca_mask))) {
        LOGM("PMT %d does not require to be sent to any CA: ad_ca_mask %X, "
             "pmt_ca_mask %X, disabled_ca_mask %X",
             pmt->id, ad ? ad->ca_mask : -2, pmt->ca_mask,
             pmt->disabled_ca_mask);

        if (pmt->state == PMT_STARTING)
            pmt->state = PMT_RUNNING;
        return 0;
    }

    if (pmt->caids > 0) {
        LOG("Sending PMT %d to all CAs: ad_ca_mask %X, "
            "pmt_ca_mask %X, disabled_ca_mask %X",
            pmt->id, ad ? ad->ca_mask : -2, pmt->ca_mask,
            pmt->disabled_ca_mask);
        for (i = 0; i < nca; i++)
            if (ca[i].enabled)
                rv += send_pmt_to_ca(i, ad, pmt);
    }

    if (pmt->state == PMT_STARTING)
        pmt->state = PMT_RUNNING;
    return rv;
}

void tables_update_encrypted_status(adapter *ad, SPMT *pmt) {
    int i;
    int status = pmt->encrypted;
    if (!ad)
        return;
    LOGM("Updating status %d for pmt %d, ad mask %08X, pmt mask %08X", status,
         pmt->id, ad->ca_mask, pmt->ca_mask);
    for (i = 0; i < nca; i++)
        if (ca[i].enabled && (ad->ca_mask & (1ULL << i)) &&
            (pmt->ca_mask & (1ULL << i))) {
            LOGM("Updating status %d pmt %d for ca %d and adapter %d", status,
                 pmt->id, i, ad->id);
            if (status == TABLES_CHANNEL_ENCRYPTED && ca[i].op->ca_encrypted)
                ca[i].op->ca_encrypted(ad, pmt);
            else if (status == TABLES_CHANNEL_DECRYPTED &&
                     ca[i].op->ca_decrypted)
                ca[i].op->ca_decrypted(ad, pmt);
        }
}

void tables_add_pid(adapter *ad, SPMT *pmt, int pid) {
    uint64_t i, mask;
    for (i = 0; i < nca; i++) {
        mask = 1ULL << i;
        if (ca[i].enabled && (pmt->ca_mask & mask) && ca[i].op->ca_add_pid)
            ca[i].op->ca_add_pid(ad, pmt, pid);
    }
}

void tables_del_pid(adapter *ad, SPMT *pmt, int pid) {
    uint64_t mask, i;
    for (i = 0; i < nca; i++) {
        mask = 1ULL << i;
        if (ca[i].enabled && (pmt->ca_mask & mask) && ca[i].op->ca_del_pid)
            ca[i].op->ca_del_pid(ad, pmt, pid);
    }
}

int tables_init_device(adapter *ad) {
    int i;
    int rv = 0;
    for (i = 0; i < nca; i++)
        if (ca[i].enabled)
            rv += tables_init_ca_for_device(i, ad);
    return rv;
}

void init_ca_device(SCA *c) {
    int i;
    adapter *ad;
    if (!c->op->ca_add_pmt)
        return;

    for (i = 0; i < MAX_ADAPTERS; i++)
        if ((ad = get_adapter_nw(i))) {
            tables_init_ca_for_device(c->id, ad);
        }
}

int tables_close_device(adapter *ad) {
    uint64_t i, mask = 1;
    int rv = 0;

    for (i = 0; i < nca; i++) {
        if (ca[i].enabled && (ad->ca_mask & mask) && ca[i].op->ca_close_dev) {
            ca[i].op->ca_close_dev(ad);
        }
    }

    ad->ca_mask = 0;
    return rv;
}

int tables_init() {
    mutex_init(&ca_mutex);
#ifndef DISABLE_DVBCA
    dvbca_init();
#endif
#ifndef DISABLE_DVBAPI
    init_dvbapi();
#endif
    return 0;
}

int tables_destroy() {
    int i;
    for (i = 0; i < nca; i++) {
        if (ca[i].enabled && ca[i].op->ca_close_ca)
            ca[i].op->ca_close_ca();
    }
    return 0;
}
