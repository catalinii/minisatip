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

// Eight fixed slots, sized before threads start and never resized,
// so readers scan ca[] without holding ca_mutex.
std::vector<SCA> ca(8);
SMutex ca_mutex;
extern SMutex ca_mask_mutex;
uint32_t ca_teardown_epoch;

int add_ca(SCA_op *op) {
    size_t i = 0;
    std::lock_guard<SMutex> lock(ca_mutex);
    del_ca(op);
    for (auto &c : ca) {
        if (!c.enabled) {
            if (!c.enabled)
                break;
        }
        i++;
    }
    if (i == ca.size())
        LOG_AND_RETURN(0, "No free CA slots for %p", (void *)ca.data());
    size_t new_ca = i;

    // A disconnect leaves op in place, and a reconnect keeps the same op
    if (ca[new_ca].op != op)
        ca[new_ca].op = op;
    ca[new_ca].id = static_cast<int>(new_ca);
    {
        std::lock_guard<SMutex> lock(ca_mask_mutex);
        memset(ca[new_ca].ad_info, 0, sizeof(ca[new_ca].ad_info));
    }
    ca[new_ca].enabled = 1;

    init_ca_device(&ca[new_ca]);
    return static_cast<int>(new_ca);
}
extern SPMT *pmts[];
// Clear one PMT's tables masks for a CA bit. Takes pmts_mutex;
// nest-safe (recursive) for callers already holding it.
void tables_clear_pmt_ca_masks(SPMT *pmt, uint64_t mask, int clear_disabled) {
    extern SMutex pmts_mutex;
    std::lock_guard<SMutex> lock(pmts_mutex);
    if (!pmt)
        return;
    pmt->ca_mask &= ~mask;
    pmt->ca_registered_mask &= ~mask;
    if (clear_disabled)
        pmt->disabled_ca_mask &= ~mask;
    // Always bump, even when the cleared bits were already zero: the
    // table entry was still torn down, so in-flight sets must skip.
    ca_teardown_epoch++;
}

void del_ca(SCA_op *op) {
    int k;
    std::vector<int> found;
    adapter *ad;
    {
        std::lock_guard<SMutex> lock(ca_mutex);
        size_t i = 0;
        for (auto &c : ca) {
            if (c.enabled && c.op == op) {
                c.enabled = 0;
                found.push_back(static_cast<int>(i));
            }
            i++;
        }
    }
    // Mask teardown runs outside ca_mutex: shorter hold, and no
    // nesting order to audit against the socket-thread teardown.
    for (int idx : found) {
        uint64_t mask = 1ULL << idx;
        for (k = 0; k < MAX_ADAPTERS; k++) // delete ca_mask for all adapters
            if ((ad = get_adapter_nw(k))) {
                std::lock_guard<SMutex> lock(ca_mask_mutex);
                ad->ca_mask &= ~mask;
            }
        extern SMutex pmts_mutex;
        std::lock_guard<SMutex> lock(pmts_mutex);
        for (k = 0; k < MAX_PMT; k++) // delete ca_mask for all the PMTs
            if (pmts[k] && pmts[k]->enabled)
                tables_clear_pmt_ca_masks(pmts[k], mask, 0);
    }
}

void tables_ca_ts(adapter *ad) {
    uint64_t mask = 1;

    for (auto &c : ca) {
        if (c.enabled && (ad->ca_mask & mask) && c.op->ca_ts) {
            c.op->ca_ts(ad);
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
    // Written here on the socket thread, read by the send path:
    // serialize both sides (leaf sections, no lock order risk).
    std::lock_guard<SMutex> lock(ca_mask_mutex);
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
    if (i < 0 || static_cast<size_t>(i) >= ca.size())
        return 0;

    if (!(ad->ca_mask & mask)) {
        if (ca[i].enabled && ca[i].op->ca_init_dev) {
            if (ca[i].op->ca_init_dev(ad) == TABLES_RESULT_OK) {
                LOGM("CA %d will handle adapter %d", i, ad->id);
                std::lock_guard<SMutex> lock(ca_mask_mutex);
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
    std::lock_guard<SMutex> lock(ca_mask_mutex);
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
    // Check ca_registered_mask, not ca_mask: pmt_add_caid() clears ca_mask
    // to force a re-send, and a PMT stopped right after was never released.
    if (ca[i].enabled && (ad->ca_mask & mask) &&
        (pmt->ca_registered_mask & mask)) {
        LOGM("Closing pmt %d for ca %d and adapter %d", pmt->id, i, ad->id);
        // Same split as the send path: op outside, mask RMW locked.
        if (ad && ca[i].op->ca_del_pmt)
            ca[i].op->ca_del_pmt(ad, pmt);
        extern SMutex pmts_mutex;
        std::lock_guard<SMutex> lock(pmts_mutex);
        if (!(pmt->ca_registered_mask & mask))
            return;
        tables_clear_pmt_ca_masks(pmt, mask, 0);
    }
}

int close_pmt_for_cas(adapter *ad, SPMT *pmt) {
    if (!pmt || !pmt->ca_registered_mask)
        return 0;

    if (!ad)
        return 0;

    LOGM("Closing pmt %d for adapter %d", pmt->id, ad->id);
    size_t i = 0;
    for (auto &c : ca) {
        if (c.enabled)
            close_pmt_for_ca(static_cast<int>(i), ad, pmt);
        i++;
    }
    return 0;
}

int send_pmt_to_ca(int i, adapter *ad, SPMT *pmt) {
    uint64_t mask;
    int rv = 0, result = 0;
    mask = 1ULL << i;

    if (ca[i].enabled && (ad->ca_mask & mask) && ca[i].op->ca_add_pmt &&
        !(pmt->disabled_ca_mask & mask) && !(pmt->ca_mask & mask)) {
        int j, send = 0, no_caids;
        {
            std::lock_guard<SMutex> lock(ca_mask_mutex);
            for (j = 0; j < ca[i].ad_info[ad->id].caids; j++)
                if (match_caid(pmt, ca[i].ad_info[ad->id].caid[j],
                               ca[i].ad_info[ad->id].mask[j])) {
                    LOG("CAID %04X and mask %04X matched PMT %d",
                        ca[i].ad_info[ad->id].caid[j],
                        ca[i].ad_info[ad->id].mask[j], pmt->id);
                    send = 1;
                    break;
                }
            no_caids = (ca[i].ad_info[ad->id].caids == 0);
        }
        result = TABLES_RESULT_ERROR_NORETRY;
        // Send outside pmts_mutex: CA ops take socket locks, which
        // would deadlock against teardown's s_mutex -> pmts order.
        uint32_t epoch;
        {
            extern SMutex pmts_mutex;
            std::lock_guard<SMutex> lock(pmts_mutex);
            epoch = ca_teardown_epoch;
        }
        if (send || no_caids) {
            result = ca[i].op->ca_add_pmt(ad, pmt);
        }

        extern SMutex pmts_mutex;
        std::lock_guard<SMutex> lock(pmts_mutex);
        if ((pmt->disabled_ca_mask & mask) || (pmt->ca_mask & mask))
            return rv;
        // Skip a set made stale by a teardown that cleared masks
        // mid-send; the PMT is simply re-sent on the next pass.
        if (epoch != ca_teardown_epoch)
            return rv;
        if (result == TABLES_RESULT_OK) {
            pmt->ca_mask |= mask;
            pmt->ca_registered_mask |= mask;
        } else if (result == TABLES_RESULT_ERROR_NORETRY)
            pmt->disabled_ca_mask |= mask;
        disable_cw(pmt->id);
        rv += (1 - result);
        LOGM("In processing PMT %d, ca %d, CA matched %d, ca_pmt_add "
             "returned %d, new ca_mask %d new disabled_ca_mask %d",
             pmt->id, i, send, result, pmt->ca_mask.load(),
             pmt->disabled_ca_mask.load());
    }
    return rv;
}

int send_pmt_to_cas(adapter *ad, SPMT *pmt) {
    int rv = 1;
    if (pmt->caids > 0) {
        LOG("Sending PMT %d to all CAs: ad_ca_mask %X, "
            "pmt_ca_mask %X, disabled_ca_mask %X",
            pmt->id, ad ? ad->ca_mask.load() : -2, pmt->ca_mask.load(),
            pmt->disabled_ca_mask.load());
        size_t i = 0;
        for (auto &c : ca) {
            if (c.enabled)
                rv += send_pmt_to_ca(static_cast<int>(i), ad, pmt);
            i++;
        }
    }

    return rv;
}

void tables_add_pid(adapter *ad, SPMT *pmt, int pid) {
    uint64_t mask;
    size_t i = 0;
    for (auto &c : ca) {
        mask = 1ULL << i;
        if (c.enabled && (pmt->ca_mask & mask) && c.op->ca_add_pid)
            c.op->ca_add_pid(ad, pmt, pid);
        i++;
    }
}

void tables_del_pid(adapter *ad, SPMT *pmt, int pid) {
    uint64_t mask;
    size_t i = 0;
    for (auto &c : ca) {
        mask = 1ULL << i;
        if (c.enabled && (pmt->ca_mask & mask) && c.op->ca_del_pid)
            c.op->ca_del_pid(ad, pmt, pid);
        i++;
    }
}

int tables_init_device(adapter *ad) {
    int rv = 0;
    size_t i = 0;
    for (auto &c : ca) {
        if (c.enabled)
            rv += tables_init_ca_for_device(static_cast<int>(i), ad);
        i++;
    }
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
    uint64_t mask = 1;
    int rv = 0;

    for (auto &c : ca) {
        if (c.enabled && (ad->ca_mask & mask) && c.op->ca_close_dev) {
            c.op->ca_close_dev(ad);
        }
        mask = mask << 1;
    }

    {
        std::lock_guard<SMutex> lock(ca_mask_mutex);
        ad->ca_mask = 0;
    }
    return rv;
}

int tables_init() {
#ifndef DISABLE_DVBCA
    dvbca_init();
#endif
#ifndef DISABLE_DVBAPI
    init_dvbapi();
#endif
    return 0;
}

int tables_destroy() {
    for (auto &c : ca) {
        if (c.enabled && c.op->ca_close_ca)
            c.op->ca_close_ca();
    }
    return 0;
}
