/*
 * Copyright (C) 2016 Catalin Toda <catalinii@yahoo.com>
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

#include "ca.h"
#include "ddci.h"
#include "dvb.h"
#include "minisatip.h"
#include "socketworks.h"
#include "stream.h"
#include "tables.h"
#include "utils.h"
#include <linux/dvb/ca.h>

#define DEFAULT_LOG LOG_DVBCA
#define CONFIG_FILE "ddci.conf"

int ddci_adapters;
extern int dvbca_id;
extern SCA ca[MAX_CA];
extern SPMT *pmts[];
SHashTable channels;

int first_ddci = -1;

#define get_ddci(i)                                                            \
    ((i >= 0 && i < MAX_ADAPTERS && ddci_devices[i] &&                         \
      ddci_devices[i]->enabled)                                                \
         ? ddci_devices[i]                                                     \
         : NULL)

#define MAKE_KEY(ad, pid) (((ad) << 16) | (pid))
// mapping for ddci, dadapter, pid
#define get_pid_mapping(d, ad, pid)                                            \
    ((ddci_mapping_table_t *)getItem(&d->mapping, MAKE_KEY(ad, pid)))

int ddci_id = -1;
ddci_device_t *ddci_devices[MAX_ADAPTERS];

int process_cat(int filter, unsigned char *b, int len, void *opaque);

// get mapping from ddci, ddci_pid
ddci_mapping_table_t *get_ddci_pid(ddci_device_t *d, int dpid) {
    int i;
    ddci_mapping_table_t *e;
    FOREACH_ITEM(&d->mapping, e) {
        if (e->ddci_pid == dpid)
            return e;
    }
    return NULL;
}

int find_ddci_pid(ddci_device_t *d, int pid) {
    int ddci_pid;
    if (pid < 0 || pid > 8191)
        LOG_AND_RETURN(-1, "pid %d invalid", pid);

    for (ddci_pid = pid; (ddci_pid & 0xFFFF) < 8191; ddci_pid++) {
        if (!get_ddci_pid(d, ddci_pid)) {
            return ddci_pid;
        }
    }
    return -1;
}
int add_pid_mapping_table(int ad, int pid, int pmt, ddci_device_t *d,
                          int force_add_pid) {
    int ddci_pid = 0, i;
    ddci_mapping_table_t *m;
    m = getItem(&d->mapping, MAKE_KEY(ad, pid));
    if (!m) {
        ddci_mapping_table_t n;
        memset(&n, -1, sizeof(n));
        n.ad = ad;
        n.pid = pid;
        n.ddci_pid = find_ddci_pid(d, pid);
        n.rewrite = 1;
        n.ddci = d->id;
        setItem(&d->mapping, MAKE_KEY(ad, pid), &n, sizeof(n));
        m = getItem(&d->mapping, MAKE_KEY(ad, pid));
    }

    if (!m)
        LOG_AND_RETURN(-1, "%s: failed to add new pid %d, adapter %d",
                       __FUNCTION__, pid, ad);
    ddci_pid = m->ddci_pid;
    if (ddci_pid == -1)
        LOG_AND_RETURN(
            -1, "could not add pid %d and ad %d to the mapping table", pid, ad);

    int add_pid = 1, add_pmt = 1;
    for (i = 0; i < m->npmt; i++) {
        if (m->pmt[i] >= 0)
            add_pid = 0;
        if (m->pmt[i] == pmt)
            add_pmt = 0;
    }
    if (add_pmt)
        for (i = 0; i < MAX_CHANNELS_ON_CI; i++) {
            if (m->pmt[i] < 0) {
                m->pmt[i] = pmt;
                if (i >= m->npmt)
                    m->npmt = i + 1;
                break;
            }
        }
    if (add_pid && force_add_pid) {
        if (pid != 1) {
            m->pid_added = 1;
            mark_pid_add(DDCI_SID, ad, pid);
        } else
            m->filter_id =
                add_filter(ad, 1, (void *)process_cat, d, FILTER_CRC);
    }
    if (add_pid) {
        // add the pids to the ddci adapter
        mark_pid_add(DDCI_SID, d->id, ddci_pid);
    }

    LOG("mapped adapter %d (%d) pid %d (%d) to %d, pmt %d", ad, m->ad, pid,
        m->pid, ddci_pid, pmt);
    return ddci_pid;
}

// get mapping on all ddci devices for adapter and pid
ddci_mapping_table_t *get_pid_mapping_allddci(int ad, int pid) {
    int i;
    for (i = 0; i < MAX_ADAPTERS; i++)
        if (ddci_devices[i] && ddci_devices[i]->enabled) {
            ddci_mapping_table_t *m = get_pid_mapping(ddci_devices[i], ad, pid);
            if (m)
                return m;
        }
    return NULL;
}

void dump_mapping_table() {
    int i, j;
    LOGM("Mapping Table for all devices");
    for (j = 0; j < MAX_ADAPTERS; j++)
        if (ddci_devices[j] && ddci_devices[j]->enabled) {
            ddci_mapping_table_t *m;
            FOREACH_ITEM(&ddci_devices[j]->mapping, m) {
                LOGM("DD %d, ddpid %d, adapter %d pid %d, rewrite %d: %d %d "
                     "%d %d",
                     m->ddci, m->ddci_pid, m->ad, m->pid, m->rewrite, m->pmt[0],
                     m->pmt[1], m->pmt[2], m->pmt[3]);
            }
        }
    return;
}

int set_pid_rewrite(ddci_device_t *d, int ad, int pid, int rewrite) {
    ddci_mapping_table_t *m = get_pid_mapping(d, ad, pid);
    if (!m)
        return -1;
    m->rewrite = rewrite;
    return 0;
}

int del_pmt_mapping_table(ddci_device_t *d, int ad, int pmt) {
    int ddci_pid = -1, i, j;
    int filter_id, pid_added;
    int to_del[MAX_PIDS], n = 0;
    ddci_mapping_table_t *m;
    FOREACH_ITEM(&d->mapping, m)
    if (m->ad == ad) {
        int pid_used = 0;
        for (j = 0; j < m->npmt; j++) {
            if (m->pmt[j] == pmt)
                m->pmt[j] = -1;
            if (m->pmt[j] >= 0) {
                LOG("%s: ad %d pid %d ddci_pid %d, pid also associated with "
                    "pmt %d",
                    __FUNCTION__, ad, m->pid, m->ddci_pid, m->pmt[j]);
                pid_used = 1;
            }
        }
        if (pid_used)
            continue;
        ddci_pid = m->ddci_pid;
        filter_id = m->filter_id;
        pid_added = m->pid_added;
        int pid = m->pid;
        to_del[n++] = pid;

        SPid *p = find_pid(d->id, ddci_pid);
        LOG("Deleting ad %d pmt %d pid %d ddci_pid %d", ad, pmt, pid, ddci_pid);
        if (p)
            mark_pid_deleted(d->id, DDCI_SID, ddci_pid, NULL);
        if (filter_id >= 0)
            del_filter(filter_id);
        else if (pid_added >= 0) {
            LOGM("Marking pid %d deleted on adapter %d (initial ad %d pid %d)",
                 ddci_pid, d->id, ad, pid);
            mark_pid_deleted(ad, DDCI_SID, pid, NULL);
        }
    }

    for (i = 0; i < n; i++)
        delItem(&d->mapping, MAKE_KEY(ad, to_del[i]));
    return 0;
}

int ddci_init_dev(adapter *ad) { return TABLES_RESULT_OK; }

int ddci_close_dev(adapter *ad) {
    if (ad->fe_sock > 0)
        sockets_del(ad->fe_sock);
    ad->fe_sock = -1;
    return TABLES_RESULT_OK;
}
SCA_op ddci;

void ddci_close_device(ddci_device_t *c) {}

int ddci_close_all() {
    int i;
    for (i = 0; i < MAX_ADAPTERS; i++)
        if (ddci_devices[i] && ddci_devices[i]->enabled) {
            ddci_close_device(ddci_devices[i]);
        }
    return 0;
}

int ddci_close(adapter *a) { return 0; }

// return 0 if
int create_channel_for_pmt(Sddci_channel *c, SPMT *pmt) {
    int i;
    ddci_device_t *d;
    memset(c, 0, sizeof(*c));
    for (i = 0; i < MAX_ADAPTERS; i++)
        if ((d = get_ddci(i))) {
            int j;

            // DDCI exists but not yet initialized
            if (is_ca_initializing(i))
                return TABLES_RESULT_ERROR_RETRY;

            for (j = 0; j < ca[dvbca_id].ad_info[i].caids; j++)
                if (match_caid(pmt, ca[dvbca_id].ad_info[i].caid[j],
                               ca[dvbca_id].ad_info[i].mask[j])) {
                    LOG("DDCI %d CAID %04X and mask %04X matched PMT %d", i,
                        ca[dvbca_id].ad_info[i].caid[j],
                        ca[dvbca_id].ad_info[i].mask[j], pmt->id);
                    c->ddci[c->ddcis].blacklisted_until = 0;
                    c->ddci[c->ddcis++].ddci = i;
                    c->sid = pmt->sid;
                    strncpy(c->name, pmt->name, sizeof(c->name));
                }
        }
    return 0;
}

int find_ddci_for_pmt(Sddci_channel *c, SPMT *pmt) {
    int ctime = getTick();
    int ddid = -100;
    int retry = 0;

    // search always from the beginning when the PMT is started

    // continue where we left off
    for (c->pos = 0; c->pos < c->ddcis; c->pos++) {
        ddid = c->ddci[c->pos].ddci;
        ddci_device_t *d = get_ddci(ddid);
        if (!d) {
            LOG("DDID %d not enabled", ddid);
        }
        if (is_ca_initializing(ddid)) {
            LOG("DD %d is initializing", ddid);
            retry = 1;

        } else if (ctime > c->ddci[c->pos].blacklisted_until) {
            if (d && d->channels < d->max_channels)
                break;
            else
                LOG("DDCI %d cannot be used for PMT %d, pid %d (used "
                    "channels "
                    "%d max %d)",
                    ddid, pmt->id, pmt->pid, d ? d->channels : -1,
                    d ? d->max_channels : -1);

            ddid = -1;
        } else
            LOG("PMT %d blacklisted on DD %d for another %jd s", pmt->id,
                c->ddci[c->pos].ddci,
                (c->ddci[c->pos].blacklisted_until - ctime) / 1000);
        ddid = -100;
    }
    if (retry)
        return -TABLES_RESULT_ERROR_RETRY;
    return ddid;
}

int is_pmt_running(SPMT *pmt) {
    ddci_mapping_table_t *m =
        get_pid_mapping_allddci(pmt->adapter, pmt->stream_pid[0].pid);
    if (!m)
        return -1;
    return m->ddci;
}

// determine if the pids from this PMT needs to be added to the virtual adapter,
// also adds the PIDs to the translation table
int ddci_process_pmt(adapter *ad, SPMT *pmt) {
    int i, ddid = -1;
    int rv = TABLES_RESULT_ERROR_NORETRY;
    Sddci_channel *channel;
    ddci_device_t *d;

    if ((d = get_ddci(ad->id))) {
        LOG("Skip processing pmt for ddci adapter %d", ad->id);
        // grace time for card decrypting lower than the default grace_time
        pmt->grace_time = 20000;
        pmt->start_time = getTick();
        SPMT *dpmt;

        // set the name of the PMT from the DDCI to the original pmt
        for (i = 0; i < d->max_channels; i++)
            if ((dpmt = get_pmt(d->pmt[i].id))) {
                if (dpmt->sid == pmt->sid) {
                    ddci_mapping_table_t *m = get_ddci_pid(d, pmt->pid);
                    if (m->pid == dpmt->pid) {
                        strcpy(pmt->name, dpmt->name);
                    }
                }
            }

        return TABLES_RESULT_OK;
    }

    ddid = is_pmt_running(pmt);

    LOG("%s: adapter %d, pmt %d, pid %d, sid %d, ddid %d, name: %s",
        __FUNCTION__, ad->id, pmt->id, pmt->pid, pmt->sid, ddid, pmt->name);

    channel = getItem(&channels, pmt->sid);
    if (!channel) {
        Sddci_channel c;
        int result = create_channel_for_pmt(&c, pmt);
        if (result)
            LOG_AND_RETURN(result, "DDCI not ready or busy at the moment: %s",
                           result == TABLES_RESULT_ERROR_NORETRY ? "no retry"
                                                                 : "retry");
        if (c.ddcis == 0)
            LOG_AND_RETURN(TABLES_RESULT_ERROR_NORETRY,
                           "no suitable DDCI found");
        setItem(&channels, pmt->sid, &c, sizeof(c));
        channel = getItem(&channels, pmt->sid);
        if (!channel)
            LOG_AND_RETURN(TABLES_RESULT_ERROR_NORETRY,
                           "Could not allocate channel");
    }

    if (ddid == -1)
        ddid = find_ddci_for_pmt(channel, pmt);
    if (ddid == -TABLES_RESULT_ERROR_RETRY)
        return -ddid;

    d = get_ddci(ddid);
    if (!d) {
        LOG("Could not find ddci device for adapter %d, ddci %d", ad->id, ddid);
        return TABLES_RESULT_ERROR_NORETRY;
    }

    mutex_lock(&d->mutex);
    int pos = -1;

    for (i = 0; i < d->max_channels; i++)
        if (d->pmt[i].id == pmt->id)
            pos = i;

    if (pos == -1) {
        for (i = 0; i < d->max_channels; i++)
            if (d->pmt[i].id == -1)
                pos = i;
    }

    if (pos == -1) {
        LOG("No free slot found for pmt %d on DDCI %d", pmt->id, d->id);
        mutex_unlock(&d->mutex);
        return TABLES_RESULT_ERROR_RETRY;
    }

    d->pmt[pos].id = pmt->id;
    d->pmt[pos].ver = (d->pmt[pos].ver + 1) & 0xF;

    d->ver = (d->ver + 1) & 0xF;
    if (!d->channels++) // for first PMT set transponder ID and add CAT
    {
        d->tid = ad->transponder_id;
        add_pid_mapping_table(ad->id, 1, pmt->id, d, 1); // add pid 1
    }

    LOG("found DDCI %d for pmt %d, running channels %d, max_channels %d", ddid, pmt->id,
        d->channels, d->max_channels);

    add_pid_mapping_table(ad->id, pmt->pid, pmt->id, d, 0);
    set_pid_rewrite(d, ad->id, pmt->pid,
                    0); // do not send the PMT pid to the DDCI device

    for (i = 0; i < pmt->caids; i++) {
        LOGM("DD %d adding ECM pid %d", d->id, pmt->ca[i].pid);
        add_pid_mapping_table(ad->id, pmt->ca[i].pid, pmt->id, d, 1);
    }

    for (i = 0; i < pmt->stream_pids; i++) {
        LOGM("DD %d adding stream pid %d %s", d->id, pmt->stream_pid[i].pid, pmt->stream_pid[i].pid == pmt->pcr_pid? "PCR":"");

        int ddci_pid = add_pid_mapping_table(ad->id, pmt->stream_pid[i].pid, pmt->id, d, 0);
        // map the PCR pid as well
        if (pmt->stream_pid[i].pid == pmt->pcr_pid) {
            d->pmt[pos].pcr_pid = ddci_pid;
        }
    }

    update_pids(ad->id);
    update_pids(d->id);
    rv = TABLES_RESULT_OK;
    dump_mapping_table();

    mutex_unlock(&d->mutex);
    return rv;
}

// if the PMT is used by the adapter, the pids will be removed from the
// translation table
int ddci_del_pmt(adapter *ad, SPMT *spmt) {
    int i, pmt = spmt->id;

    if (get_ddci(ad->id)) {
        LOG("Skip deleting pmt for ddci adapter %d", ad->id);
        return 0;
    }

    ddci_mapping_table_t *m = get_pid_mapping_allddci(ad->id, spmt->pid);
    if (!m) {
        dump_mapping_table();
        LOG_AND_RETURN(
            0, "%s: pid mapping for adapter %d, pmt %d and pid %d not found",
            __FUNCTION__, ad->id, spmt->id, spmt->pid);
    }
    ddci_device_t *d = get_ddci(m->ddci);
    if (!d)
        LOG_AND_RETURN(0, "%s: ddci %d already disabled", __FUNCTION__,
                       m->ddci);
    d->ver = (d->ver + 1) & 0xF;
    if (d->channels > 0)
        d->channels--;
    LOG("%s: deleting pmt id %d, sid %d (%X), pid %d, ddci %d, name %s",
        __FUNCTION__, spmt->id, spmt->sid, spmt->sid, m->ddci_pid, m->ddci,
        spmt->name);
    if (d->pmt[0].id == pmt)
        d->cat_processed = 0;

    for (i = 0; i < d->max_channels; i++)
        if (d->pmt[i].id == pmt) {
            d->pmt[i].id = -1;
        }

    del_pmt_mapping_table(d, ad->id, pmt);
    update_pids(d->id);
    dump_mapping_table();
    return 0;
}
void blacklist_pmt_for_ddci(SPMT *pmt, int ddid) {
    Sddci_channel *channel = getItem(&channels, pmt->sid);
    if (!channel)
        return;

    if (channel->pos >= channel->ddcis)
        return;
    // blacklist this channel for this DDCI for 5s
    channel->ddci[channel->pos].blacklisted_until = getTick() + 15000;

    LOG("PMT %d, pid %d, sid %d is blacklisted on DD %d (pmt %d)", pmt->id,
        pmt->pid, pmt->sid, ddid, pmt->id);
}

// once one of the PMTs sent to the CAM cannot be decrypted,
// blacklist all PMTs that have the same master PMT with the one given as
// argument
void blacklist_pmts_for_ddci(SPMT *pmt, int ddid) {
    int i, master = pmt->master_pmt;
    SPMT *p;
    for (i = 0; i < MAX_PMT; i++)
        if ((p = get_pmt(i)) && p->master_pmt == master)
            blacklist_pmt_for_ddci(pmts[i], ddid);
}

// delete the blacklisted PMTs from the adapter thread owning the PMT
void delete_blacklisted_pmt(ddci_device_t *d, adapter *ad) {
    int i;
    SPMT *pmt;
    Sddci_channel *channel;
    uint64_t ctime = getTick();
    for (i = 0; i < d->max_channels; i++)
        if ((pmt = get_pmt(d->pmt[i].id)) && (pmt->adapter == ad->id) &&
            (channel = getItem(&channels, pmt->sid)) &&
            (channel->pos < channel->ddcis) &&
            (channel->ddci[channel->pos].blacklisted_until > ctime)) {
            LOG("PMT %d, pid %d, sid %d is encrypted on DD %d", pmt->id,
                pmt->pid, pmt->sid, d->id);
            close_pmt_for_ca(ddci_id, ad, pmt);
        }
}

int ddci_encrypted(adapter *ad, SPMT *pmt) {
    ddci_device_t *d = get_ddci(ad->id);
    Sddci_channel *channel = getItem(&channels, pmt->sid);
    if(channel && channel->locked) {
        LOG("PMT %d sid %d is encrypted and locked, keeping active", pmt->id, pmt->sid);
        return 0;
    }
    if (d) // only on DDCI adapter we can understand if the channel is encrypted
        blacklist_pmts_for_ddci(pmt, d->id);
    return 0;
}

int ddci_decrypted(adapter *ad, SPMT *pmt) {
    ddci_device_t *d = get_ddci(ad->id);
    if (d) {
        LOG("PMT %d, sid %d is reported decrypted on DD %d", pmt->id, pmt->sid,
            d->id);
    }
    return 0;
}

void set_pid_ts(unsigned char *b, int pid) {
    pid &= 0x1FFF;
    b[1] &= 0xE0;
    b[1] |= (pid >> 8) & 0x1F;
    b[2] = pid & 0xFF;
}

int ddci_create_pat(ddci_device_t *d, uint8_t *b) {
    int len = 0;
    int i;
    SPMT *pmt;
    b[0] = 0;
    b[1] = 0;
    b[2] = 0xb0;
    b[3] = 0; // len
    copy16(b, 4, d->tid);
    b[6] = 0xC0 | (d->ver << 1);
    b[7] = b[8] = 0;
    // Channel ID 0
    b[9] = b[10] = 0;
    // PID 16
    b[11] = 0x00;
    b[12] = 0x10;
    len = 13;
    for (i = 0; i < MAX_CHANNELS_ON_CI; i++)
        if ((pmt = get_pmt(d->pmt[i].id))) {
            ddci_mapping_table_t *m =
                get_pid_mapping(d, pmt->adapter, pmt->pid);
            if (!m) {
                LOG("adapter %d pid %d not found in the mapping table",
                    pmt->adapter, pmt->pid);
                continue;
            }

            copy16(b, len, pmt->sid);
            copy16(b, len + 2, 0xE000 | m->ddci_pid);
            len += 4;
        }
    int len1 = len;
    len += 4;
    b[2] |= (len1 >> 8);
    b[3] |= (len1 & 0xFF);
    uint32_t crc = crc_32(b + 1, len1 - 1);
    copy32(b, len1, crc);
    LOGM("%s: transponder %d version %d, len %d", __FUNCTION__, d->tid, d->ver,
         len1);
    char buf[500];
    sprintf(buf, "PAT Created CRC %08X: ", crc);
    hexdump(buf, b + 1, len1 - 1);
    return len;
}

void ddci_replace_pi(ddci_device_t *d, int adapter, unsigned char *es,
                     int len) {

    int es_len, capid;
    int i;

    for (i = 0; i < len; i += es_len) // reading program info
    {
        es_len = es[i + 1] + 2;
        if (es[i] != 9)
            continue;
        capid = (es[i + 4] & 0x1F) * 256 + es[i + 5];
        ddci_mapping_table_t *m = get_pid_mapping(d, adapter, capid);
        int dpid = capid;

        if (m)
            dpid = m->ddci_pid;

        es[i + 4] &= 0xE0; //~0x1F
        es[i + 4] |= (dpid >> 8);
        es[i + 5] = dpid & 0xFF;
        LOGM("%s: CA pid %d -> pid %d", __FUNCTION__, capid, dpid)
    }
}

uint16_t YMDtoMJD(int Y, int M, int D) {
    int L = (M < 3) ? 1 : 0;
    return 14956 + D + (int)((Y - L) * 365.25) +
           (int)((M + 1 + L * 12) * 30.6001);
}

// Based on vdr implementation provided by Klaus Schmidinger
int ddci_create_epg(ddci_device_t *d, int sid, uint8_t *eit, int version) {
    uint8_t *PayloadStart;
    uint8_t *SectionStart;
    uint8_t *DescriptorsStart;
    static int counter[MAX_ADAPTERS];
    memset(eit, 0xFF, 188);
    struct tm tm_r;
    time_t t =
        time(NULL) - 3600; // let's have the event start one hour in the past
    struct tm *tm = localtime_r(&t, &tm_r);
    uint16_t MJD = YMDtoMJD(tm->tm_year, tm->tm_mon + 1, tm->tm_mday);
    uint8_t *p = eit;
    // TS header:
    *p++ = 0x47;
    *p++ = 0x40;
    *p++ = 0x12;
    *p++ = 0x10 | (counter[d->id]++ & 0x0F); // continuity counter
    *p++ = 0x00; // pointer field (payload unit start indicator is set)
    // payload:
    PayloadStart = p;
    *p++ = 0x4E; // TID present/following event on this transponder
    *p++ = 0xF0;
    *p++ = 0x00; // section length
    SectionStart = p;
    *p++ = sid >> 8;
    *p++ = sid & 0xFF;
    *p++ = 0xC1 | (version << 1);
    *p++ = 0x00;        // section number
    *p++ = 0x00;        // last section number
    *p++ = 0x00;        // transport stream id
    *p++ = 0x00;        // ...
    *p++ = 0x00;        // original network id
    *p++ = 0x00;        // ...
    *p++ = 0x00;        // segment last section number
    *p++ = 0x4E;        // last table id
    *p++ = 0x00;        // event id
    *p++ = 0x01;        // ...
    *p++ = MJD >> 8;    // start time
    *p++ = MJD & 0xFF;  // ...
    *p++ = tm->tm_hour; // ...
    *p++ = tm->tm_min;  // ...
    *p++ = tm->tm_sec;  // ...
    *p++ = 0x24;        // duration (one day, should cover everything)
    *p++ = 0x00;        // ...
    *p++ = 0x00;        // ...
    *p++ = 0x90;        // running status, free/CA mode
    *p++ = 0x00;        // descriptors loop length
    DescriptorsStart = p;
    *p++ = 0x55; // parental descriptor tag
    *p++ = 0x04; // descriptor length
    *p++ = '9';  // country code "902" ("All countries") -> EN 300 468 / 6.2.28;
                 // www.dvbservices.com/country_codes/index.php
    *p++ = '0';
    *p++ = '2';
    *p++ = 0; // ParentalRating
    // fill in lengths:
    *(SectionStart - 1) = p - SectionStart + 4; // +4 = length of CRC
    *(DescriptorsStart - 1) = p - DescriptorsStart;
    // checksum
    int crc = crc_32(PayloadStart, p - PayloadStart);
    *p++ = crc >> 24;
    *p++ = crc >> 16;
    *p++ = crc >> 8;
    *p++ = crc;
    return 188;
}

int safe_get_pid_mapping(ddci_device_t *d, int aid, int pid) {
    ddci_mapping_table_t *m = get_pid_mapping(d, aid, pid);
    if (m)
        return m->ddci_pid;
    return pid;
}

int ddci_create_pmt(ddci_device_t *d, SPMT *pmt, uint8_t *new_pmt, int pmt_size,
                    int ver, int pcr_pid) {
    int pid = pmt->pid, pi_len = 0, i;
    uint8_t *b = new_pmt, *start_pmt, *start_pi_len;
    memset(new_pmt, 0, pmt_size);

    *b++ = 0;
    *b++ = 0x02;
    *b++ = 0; // len
    *b++ = 0;
    start_pmt = b;
    copy16(b, 0, pmt->sid);
    b += 2;
    *b++ = (ver << 1);
    *b++ = 0;    // section number
    *b++ = 0;    // last section number
    *b++ = 0xE0 | ((pcr_pid >> 8) & 0xFF); // PCR PID
    *b++ = pcr_pid & 0xFF;                 // PCR PID

    start_pi_len = b;

    *b++ = 0; // PI LEN
    *b++ = 0;

    LOGM("%s: PMT %d AD %d, pid: %04X (%d), ver %d, sid %04X (%d) %s %s",
         __FUNCTION__, pmt->id, pmt->adapter, pid, pid, ver, pmt->sid, pmt->sid,
         pmt->name[0] ? "channel:" : "", pmt->name);

    // Add CA IDs and CA Pids
    for (i = 0; i < pmt->caids; i++) {
        int private_data_len = pmt->ca[i].private_data_len;
        *b++ = 0x09;
        *b++ = 0x04 + private_data_len;
        copy16(b, 0, pmt->ca[i].id);
        copy16(b, 2, safe_get_pid_mapping(d, pmt->adapter, pmt->ca[i].pid));
        memcpy(b + 4, pmt->ca[i].private_data, private_data_len);
        pi_len += 6 + private_data_len;
        b += 4 + private_data_len;
        LOGM("%s: pmt %d added caid %04X, pid %04X", __FUNCTION__, pmt->id,
             pmt->ca[i].id, pmt->ca[i].pid);
    }
    copy16(start_pi_len, 0, pi_len);

    // Add Stream pids
    // Add CA IDs and CA Pids
    for (i = 0; i < pmt->stream_pids; i++) {
        *b = pmt->stream_pid[i].type;
        copy16(b, 1,
               safe_get_pid_mapping(d, pmt->adapter, pmt->stream_pid[i].pid));
        int desc_len = pmt->stream_pid[i].desc_len;
        copy16(b, 3, desc_len);
        memcpy(b + 5, pmt->stream_pid[i].desc, desc_len);
        b += desc_len + 5;

        LOGM("%s: pmt %d added pid %04X, type %02X", __FUNCTION__, pmt->id,
             pmt->stream_pid[i].pid, pmt->stream_pid[i].type);
    }
    // set the length (b + 4 bytes from crc)
    copy16(start_pmt, -2, 4 + b - start_pmt);

    uint32_t crc = crc_32(new_pmt + 1, b - new_pmt - 1);
    copy32(b, 0, crc);
    b += 4;
    return b - new_pmt;
}

int ddci_add_psi(ddci_device_t *d, uint8_t *dst, int len) {
    unsigned char psi[1500];
    int64_t ctime = getTick();
    int i, pos = 0;
    int psi_len;
    if (ctime - d->last_pat > 500) {
        psi_len = ddci_create_pat(d, psi);
        pos += buffer_to_ts(dst + pos, len - pos, psi, psi_len, &d->pat_cc, 0);
        d->last_pat = ctime;
    }

    if (ctime - d->last_pmt > 100) {
        SPMT *pmt;
        for (i = 0; i < MAX_CHANNELS_ON_CI; i++) {
            if ((pmt = get_pmt(d->pmt[i].id))) {
                psi_len =
                    ddci_create_pmt(d, pmt, psi, sizeof(psi), d->pmt[i].ver, d->pmt[i].pcr_pid);
                ddci_mapping_table_t *m =
                    get_pid_mapping(d, pmt->adapter, pmt->pid);
                if (m)
                    pos += buffer_to_ts(dst + pos, len - pos, psi, psi_len,
                                        &d->pmt[i].cc, m->ddci_pid);
                else
                    LOG("%s: could not find PMT %d adapter %d and pid %d to "
                        "mapping table",
                        __FUNCTION__, d->pmt[i].id, pmt->adapter, pmt->pid);
                // ADD EPG as well
                if (len - pos >= 188)
                    pos +=
                        ddci_create_epg(d, pmt->sid, dst + pos, d->pmt[i].ver);
            }
            if (len - pos >= 188)
                pos += ddci_create_epg(d, MAKE_SID_FOR_CA(d->id, i), dst + pos,
                                       d->pmt[i].ver);
        }
        d->last_pmt = ctime;
    }
    return pos;
}

// find a position starting with *ad_pos or at the end of the ad->buf
int push_ts_to_adapter(adapter *ad, unsigned char *b, int new_pid,
                       int *ad_pos) {
    int i, new_pos = -1;
    for (i = *ad_pos; i < ad->rlen; i += 188)
        if (PID_FROM_TS(ad->buf + i) == 0x1FFF) {
            new_pos = i;
            break;
        }
    if ((new_pos == -1) && (ad->rlen <= ad->lbuf - 188)) {
        new_pos = ad->rlen;
        ad->rlen += 188;
    }
    if (new_pos < 0 || new_pos + 188 > ad->lbuf) {
        LOGM("Could not push more data for adapter %d, rlen %d, lbuf %d, new "
             "pos %d",
             ad->id, ad->rlen, ad->lbuf, new_pos);
        *ad_pos = 0;
        return 1;
    }

    memcpy(ad->buf + new_pos, b, 188);
    set_pid_ts(ad->buf + new_pos, new_pid);
    set_pid_ts(b, 0x1FFF);
    *ad_pos = new_pos;
    return 0;
}

int ddci_process_ts(adapter *ad, ddci_device_t *d) {
    unsigned char *b;
    adapter *ad2 = get_adapter(d->id);
    int rlen = ad->rlen;
    int bytes = 0, ec = 0;
    int iop = 0, iomax = ad->rlen / 188 + 100;
    int pid, dpid, i;
    struct iovec io[iomax];
    unsigned char psi[MAX_CHANNELS_ON_CI * 1500];
    uint16_t ad_dd_pids[8192], dd_ad_pids[8192];

    if (mutex_lock(&d->mutex))
        return 0;
    if (!d->enabled)
        return 0;

    if (!ad2) {
        mutex_unlock(&d->mutex);
        return 0;
    }
    // step 0 - create ad_dd_pid and dd_ad_pid mapping
    memset(ad_dd_pids, 0, sizeof(ad_dd_pids));
    memset(dd_ad_pids, 0, sizeof(dd_ad_pids));
    ddci_mapping_table_t *m;
    FOREACH_ITEM(&d->mapping, m)
    if (m->ad == ad->id && m->rewrite) {
        ad_dd_pids[m->pid] = m->ddci_pid;
        dd_ad_pids[m->ddci_pid] = m->pid;
        ec++;
        DEBUGM("Using pid %d in adapter %d as pid %d in DDCI %d", m->pid,
               ad->id, m->ddci_pid, d->id);
    }

    if (ec == 0) {
        d->ro[ad->id] = -1;
        mutex_unlock(&d->mutex);
        return 0;
    }

    if (d->ro[ad->id] == -1) {
        d->ro[ad->id] = (d->wo + 188) % DDCI_BUFFER;
    }
    // step 1 - fill the IO with TS packets and change the PID as required by
    // mapping table
    for (i = 0; i < rlen; i += 188) {
        b = ad->buf + i;
        if (b[0] != 0x47)
            continue;
        pid = PID_FROM_TS(b);
        dpid = ad_dd_pids[pid];
        if (dpid == 0)
            continue;

        dump_packets("ddci_process_ts -> DD", b, 188, i);

        set_pid_ts(b, dpid);
        // bundle packets to stay under MAX_IOV
        if (iop - 1 >= 0 && io[iop - 1].iov_base + io[iop - 1].iov_len == b)
            io[iop - 1].iov_len += DVB_FRAME;
        else {
            io[iop].iov_base = b;
            io[iop++].iov_len = DVB_FRAME;
        }
        bytes += 188;
        DEBUGM("pos %d of %d, mapping pid %d to %d", iop, rlen / 188, pid,
               dpid);
    }
    // write the TS to the DDCI handle
    if (iop > 0) {
        int psi_len = ddci_add_psi(d, psi, sizeof(psi) - 1);
        hexdump("PSI -> ", psi, psi_len);
        if (psi_len > 0) {
            io[iop].iov_base = psi;
            io[iop].iov_len = psi_len;
            bytes += io[iop].iov_len;
            iop++;
        }

        LOGM("writing %d bytes to DDCI device %d, fd %d, sock %d", bytes, d->id,
             ad2->fe, ad2->fe_sock);
        int rb = writev(ad2->fe, io, iop);
        if (rb != bytes)
            LOG("%s: write incomplete to DDCI %d,fd %d, wrote %d out of %d "
                "(iop %d%s), "
                "errno "
                "%d: %s",
                __FUNCTION__, ad2->id, ad2->fe, rb, bytes, iop,
                iop > 1024 ? " > IOV_MAX" : "", errno, strerror(errno));
    }
    // mark the written TS packets as 8191 (0x1FFF)
    for (i = 0; i < iop; i++) {
        int j;
        for (j = 0; j < io[i].iov_len; j += 188)
            set_pid_ts(io[i].iov_base + j, 0x1FFF);
    }

    // move back TS packets from the DDCI out buffer to the adapter buffer
    int ad_pos = 0;
    LOGM("ddci_process_ts ad %d ro %d, wo %d max %d", ad->id, d->ro[ad->id],
         d->wo, DDCI_BUFFER);

    for (i = d->ro[ad->id]; i != d->wo; i = (i + 188) % DDCI_BUFFER) {
        dump_packets("ddci_process_ts -> AD", d->out + i, 188, i);
        pid = PID_FROM_TS(d->out + i);
        dpid = dd_ad_pids[pid];
        int rv = 0;

        if (dpid)
            rv = push_ts_to_adapter(ad, d->out + i, dpid, &ad_pos);
        if (rv) {
            LOGM("adapter %d buffer full, DD %d, ro %d, wo %d", ad->id, d->id,
                 d->ro[ad->id], d->wo);
            break;
        }
        d->ro[ad->id] = i;
    }

    delete_blacklisted_pmt(d, ad);

    mutex_unlock(&d->mutex);
    return 0;
}

int ddci_ts(adapter *ad) {
    int i;

    // do not process the TS for DDCI devices
    if (ddci_devices[ad->id] && ddci_devices[ad->id]->enabled)
        return 0;

    for (i = 0; i < MAX_ADAPTERS; i++)
        if (ddci_devices[i] && ddci_devices[i]->enabled)
            ddci_process_ts(ad, ddci_devices[i]);

    return 0;
}

void ddci_init() // you can search the devices here and fill the ddci_devices,
                 // then open them here (for example independent CA devices), or
                 // use ddci_init_dev to open them (like in this module)
{
    memset(&ddci, 0, sizeof(ddci));
    ddci.ca_init_dev = ddci_init_dev;
    ddci.ca_close_dev = ddci_close_dev;
    ddci.ca_add_pmt = ddci_process_pmt;
    ddci.ca_del_pmt = ddci_del_pmt;
    ddci.ca_close_ca = ddci_close;
    ddci.ca_ts = ddci_ts;
    ddci.ca_encrypted = ddci_encrypted;
    ddci.ca_decrypted = ddci_decrypted;

    ddci_id = add_ca(&ddci, 0xFFFFFFFF);
    LOG("Registered DDCI CA %d", ddci_id);
}
int ddci_set_pid(adapter *ad, int pid) {
    LOGM("%s: ddci %d add pid %d", __FUNCTION__, ad->id, pid);
    return 100;
}

int ddci_del_filters(adapter *ad, int fd, int pid) {
    LOGM("%s: ddci %d del pid %d", __FUNCTION__, ad->id, pid);
    return 0;
}

void advance_read_offset(ddci_device_t *d, int rlen) {
    int i;
    for (i = 0; i < MAX_ADAPTERS; i++) {
        if (!get_adapter_nw(i))
            d->ro[i] = -1;
        if (d->ro[i] >= 0) {
            if (((d->ro[i] > d->wo) && (d->ro[i] <= d->wo + rlen)) ||
                ((d->ro[i] < d->wo) &&
                 (d->ro[i] + DDCI_BUFFER <= d->wo + rlen))) {
                LOG("DDCI %d, dropping packets for adapter %d", d->id, i);
                d->ro[i] = (d->wo + rlen + 188) % DDCI_BUFFER;
            }
        }
    }
}

// copy the entire len to the ddci buffer and advance read offset
int push_ts_to_ddci_buffer(ddci_device_t *d, unsigned char *b, int len) {
    int _min = len < DDCI_BUFFER - d->wo ? len : DDCI_BUFFER - d->wo;

    memcpy(d->out + d->wo, b, _min);
    memcpy(d->out, b + _min, len - _min);

    advance_read_offset(d, len);
    d->wo = (d->wo + len) % DDCI_BUFFER;
    return 0;
}

int ddci_read_sec_data(sockets *s) {
    unsigned char *b = s->buf;

    read_dmx(s);
    if (s->rlen != 0) {
        DEBUGM("process_dmx not called as s->rlen %d", s->rlen);
        return 0;
    }
    // copy the processed data to d->out buffer
    adapter *ad = get_adapter(s->sid);
    ddci_device_t *d = get_ddci(s->sid);
    b = ad->buf;
    int left = 0, rlen = ad->rlen;

    if ((left = push_ts_to_ddci_buffer(d, b, rlen)) > 0)
        LOG("dropping %d bytes for ddci_adapter %d", left, d->id);
    return 0;
}

int ddci_post_init(adapter *ad) {
    ddci_device_t *d = ddci_devices[ad->id];
    d->max_channels = get_max_pmt_for_ca(ad->id);
    sockets *s = get_sockets(ad->sock);
    s->action = (socket_action)ddci_read_sec_data;
    if (ad->fe_sock >= 0)
        set_socket_thread(ad->fe_sock, get_socket_thread(ad->sock));
    post_tune(ad);
    if (ddci_id < 0)
        ddci_init();
    return 0;
}

int set_property(int fd, uint32_t cmd, uint32_t data) {
    struct dtv_property p;
    struct dtv_properties c;
    int ret;

    p.cmd = cmd;
    c.num = 1;
    c.props = &p;
    p.u.data = data;
    ret = ioctl(fd, FE_SET_PROPERTY, &c);
    if (ret < 0) {
        LOGM("FE_SET_PROPERTY command %d returned %d\n", cmd, errno);
        return -1;
    }
    return 0;
}

int ddci_open_device(adapter *ad) {
    char buf[100];
    int read_fd, write_fd;
    ddci_device_t *d = ddci_devices[ad->id];
    if (!d) {
        unsigned char *out;
        out = malloc1(DDCI_BUFFER + 10);
        if (!out) {
            LOG_AND_RETURN(1,
                           "%s: could not allocated memory for the output "
                           "buffer for adapter %d",
                           __FUNCTION__, ad->id);
        }

        d = ddci_devices[ad->id] = malloc1(sizeof(ddci_device_t));
        if (!d)
            return -1;
        mutex_init(&d->mutex);
        d->id = ad->id;
        memset(out, -1, DDCI_BUFFER + 10);
        d->out = out;
        create_hash_table(&d->mapping, 100);
    }
    LOG("DDCI opening [%d] adapter %d and frontend %d", ad->id, ad->pa, ad->fn);
    sprintf(buf, "/dev/dvb/adapter%d/sec%d", ad->pa, ad->fn);
#ifndef DDCI_TEST
    write_fd = open(buf, O_WRONLY);
    if (write_fd < 0) {
        sprintf(buf, "/dev/dvb/adapter%d/ci%d", ad->pa, ad->fn);
        write_fd = open(buf, O_WRONLY);
        if (write_fd < 0) {
            LOG("%s: could not open %s in WRONLY mode error %d: %s",
                __FUNCTION__, buf, errno, strerror(errno));
            return 1;
        }
    }

    read_fd = open(buf, O_RDONLY);
    if (read_fd < 0) {
        LOG("%s: could not open %s in RDONLY mode error %d: %s", __FUNCTION__,
            buf, errno, strerror(errno));
        if (write_fd >= 0)
            close(write_fd);
        ad->fe = -1;
        return 1;
    }
    // set input bitrate for TBS devices to 100Mbits/s
#define MODULATOR_INPUT_BITRATE 33
    //    set_property(write_fd, MODULATOR_INPUT_BITRATE, 100);
    set_linux_socket_nonblock(read_fd);
    set_linux_socket_nonblock(write_fd);
#else
    int fd[2];
    if (pipe(fd) == -1) {
        LOG("pipe failed errno %d: %s", errno, strerror(errno));
        return 1;
    }
    read_fd = fd[0];
    write_fd = fd[1];

#endif
    mutex_lock(&d->mutex);
    ad->fe = write_fd;
    // creating a non blocking socket for buffering -- not working
    ad->fe_sock = -1;
    //    ad->fe_sock = sockets_add(ad->fe, NULL, ad->id, TYPE_TCP, NULL, NULL,
    //    NULL); if(ad->fe_sock < 0)
    //        LOG_AND_RETURN(ad->fe_sock, "Failed to add sockets for the DDCI
    //        device");
    ad->dvr = read_fd;
    ad->type = ADAPTER_CI;
    ad->dmx = -1;
    ad->sys[0] = 0;
    ad->adapter_timeout = 0;
    memset(d->pmt, -1, sizeof(d->pmt));
    d->ncapid = 0;
    d->max_channels = MAX_CHANNELS_ON_CI;
    d->channels = 0;
    d->wo = 0;
    memset(d->ro, -1, sizeof(d->ro));
    d->last_pmt = d->last_pat = 0;
    d->tid = d->ver = 0;
    d->enabled = 1;
    ad->enabled = 1;
    mutex_unlock(&d->mutex);
    LOG("opened DDCI adapter %d fe:%d dvr:%d", ad->id, ad->fe, ad->dvr);

    return 0;
}

fe_delivery_system_t ddci_delsys(int aid, int fd, fe_delivery_system_t *sys) {
    return 0;
}

int process_cat(int filter, unsigned char *b, int len, void *opaque) {
    int cat_len = 0, i, es_len = 0, caid, add_cat = 1;
    ddci_device_t *d = (ddci_device_t *)opaque;
    cat_len = len - 4; // remove crc
    SFilter *f = get_filter(filter);
    int id;
    if (!f)
        return 0;

    if (b[0] != 1)
        return 0;

    if (!d->enabled)
        LOG_AND_RETURN(0, "DDCI %d no longer enabled, not processing PAT",
                       d->id);

    if (d->cat_processed)
        return 0;

    d->cat_processed = 1;

    cat_len -= 9;
    b += 8;
    LOG("CAT DDCI %d len %d", d->id, cat_len);
    if (cat_len > 1500)
        return 0;

    id = -1;
    for (i = 0; i < cat_len; i += es_len) // reading program info
    {
        es_len = b[i + 1] + 2;
        if (b[i] != 9)
            continue;
        caid = b[i + 2] * 256 + b[i + 3];
        if (++id < MAX_CA_PIDS)
            d->capid[id] = (b[i + 4] & 0x1F) * 256 + b[i + 5];

        LOG("CAT pos %d caid %d, pid %d", id, caid, d->capid[id]);
    }
    id++;

    add_cat = 1;
    mutex_lock(&d->mutex);
    for (i = 0; i < id; i++)
        if (get_ddci_pid(d, d->capid[i])) {
            add_cat = 0;
            LOGM("CAT pid %d already in use, skipping CAT", d->capid[i]);
            break;
        }
    if (!add_cat) {
        mutex_unlock(&d->mutex);
        return 0;
    }

    // sending EMM pids to the CAM
    for (i = 0; i < id; i++) {
        add_pid_mapping_table(f->adapter, d->capid[i], d->pmt[0].id, d, 1);
    }
    d->cat_processed = 1;
    d->ncapid = id;
    mutex_unlock(&d->mutex);
    update_pids(f->adapter);
    update_pids(d->id);
    return 0;
}

void save_channels(SHashTable *ch, char *file) {
    int i, j;
    Sddci_channel *c;
    struct stat buf;
    if (!stat(file, &buf)) {
        LOG("%s already exists, not saving", file);
        return;
    }

    FILE *f = fopen(file, "wt");
    if (!f)
        return;
    fprintf(f, "# Format: SID: DDCI1,DDCI2\n");
    fprintf(f, "# Use # for comments\n");
    fprintf(f, "# To decrypt channel with sid SID it will try in order DDCI1 "
               "then DDCI2\n");
    fprintf(f, "# To prevent the SID from using any DDCI, include an empty "
               "list (eg: SID: )\n");
    fprintf(f, "# When a new channel it will be used, minisatip will determine "
               "the matching DDCIs based on their reported CA IDs\n");

    FOREACH_ITEM(ch, c) {
        fprintf(f, "%d:    ", c->sid);
        for (j = 0; j < c->ddcis; j++)
            fprintf(f, "%s%d", j ? "," : "", c->ddci[j].ddci);
        fprintf(f, " # %s\n", c->name);
    }
    fclose(f);
}
void load_channels(SHashTable *ch, char *file) {
    Sddci_channel c;
    char line[1000];

    FILE *f = fopen(file, "rt");
    int channels = 0;
    int pos = 0;
    if (!f)
        return;
    while (fgets(line, sizeof(line), f)) {
        char buf[1000];
        memset(&c, 0, sizeof(c));
        char *x = strchr(line, '#');
        if (x)
            *x = 0;
        char *cc = strstr(line, ":");
        if (!cc)
            continue;

        c.sid = map_int(line, NULL);
        if (!c.sid)
            continue;
        c.locked = 1;
        cc = strip(cc + 1);

        char *arg[MAX_ADAPTERS];
        int la = split(arg, cc, ARRAY_SIZE(arg), ',');
        int i = 0;
        channels++;
        pos = 0;
        for (i = 0; i < la; i++) {
            int v = map_intd(arg[i], NULL, -1);
            if (v != -1) {
                c.ddci[c.ddcis++].ddci = v;
                strcatf(buf, pos, "%s%d", i ? "," : "", v);
            }
        }
        LOGM("Adding channel %d: DDCIs: %s", c.sid, buf);
        setItem(ch, c.sid, &c, sizeof(c));
    }
    fclose(f);
    LOG("Loaded %d channels from %s", channels, file);
}

void ddci_free(adapter *ad) {
    ddci_device_t *d = ddci_devices[ad->id];
    if (!d)
        return;
    free_hash(&d->mapping);
    if (channels.size) {
        save_channels(&channels, CONFIG_FILE);
        free_hash(&channels);
    }
}

void find_ddci_adapter(adapter **a) {
    int na = -1;
    char buf[100];
    int cnt;
    int i = 0, j = 0;

    ddci_adapters = 0;
    adapter *ad;
    if (opts.disable_dvb) {
        LOG("DVBCI device detection disabled");
        return;
    }

    for (i = 0; i < MAX_ADAPTERS; i++)
        if (a[i])
            na = i;
    na++;
    LOGM("Starting %s with index %d", __FUNCTION__, na);

    for (i = 0; i < MAX_ADAPTERS; i++)
        for (j = 0; j < MAX_ADAPTERS; j++) {
            cnt = 0;
            sprintf(buf, "/dev/dvb/adapter%d/ca%d", i, j);
            if (!access(buf, R_OK))
                cnt++;

            sprintf(buf, "/dev/dvb/adapter%d/sec%d", i, j);
            if (!access(buf, R_OK))
                cnt++;
            else {
                sprintf(buf, "/dev/dvb/adapter%d/ci%d", i, j);
                if (!access(buf, R_OK))
                    cnt++;
            }
#ifdef DDCI_TEST
            cnt = 2;
#endif
            if (cnt == 2) {
                LOGM("%s: adding %d %d to the list of devices", __FUNCTION__, i,
                     j);
                if (!a[na])
                    a[na] = adapter_alloc();

                ad = a[na];
                ad->pa = i;
                ad->fn = j;

                ad->open = ddci_open_device;
                ad->commit = NULL;
                ad->tune = NULL;
                ad->delsys = ddci_delsys;
                ad->post_init = ddci_post_init;
                ad->close = ddci_close;
                ad->get_signal = NULL;
                ad->set_pid = ddci_set_pid;
                ad->del_filters = ddci_del_filters;
                ad->type = ADAPTER_CI;
                ad->drop_encrypted = 0;
                ad->free = ddci_free;
                if (channels.size == 0) {
                    create_hash_table(&channels, 100);
                    load_channels(&channels, CONFIG_FILE);
                }

                ddci_adapters++;
                na++;
                a_count = na; // update adapter counter
                if (na == MAX_ADAPTERS)
                    return;
                if (first_ddci == -1)
                    first_ddci = na - 1;
#ifdef DDCI_TEST
                return;
#endif
            }
        }
    for (; na < MAX_ADAPTERS; na++)
        if (a[na])
            a[na]->pa = a[na]->fn = -1;
}
