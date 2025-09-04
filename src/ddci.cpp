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
#include <string>

#include "utils/fifo.h"
#include "utils/ticks.h"
#include <linux/dvb/ca.h>

#define DEFAULT_LOG LOG_DVBCA
#define CONFIG_FILE_NAME "ddci.conf"

extern int dvbca_id;
extern SCA ca[MAX_CA];
extern SPMT *pmts[];
std::unordered_map<int, Sddci_channel> channels;

#define get_ddci(i)                                                            \
    ((i >= 0 && i < MAX_ADAPTERS && ddci_devices[i] &&                         \
      ddci_devices[i]->enabled)                                                \
         ? ddci_devices[i]                                                     \
         : NULL)

#define MAKE_KEY(ad, pid) (((ad) << 16) | (pid))

int ddci_id = -1;
ddci_device_t *ddci_devices[MAX_ADAPTERS];

int ddci_process_cat(int filter, unsigned char *b, int len, void *opaque);

// get mapping from ddci, ddci_pid
ddci_mapping_table_t *get_ddci_pid(ddci_device_t *d, int dpid) {
    for (const auto &n : d->mapping) {
        if (n.second.ddci_pid == dpid)
            return (ddci_mapping_table *)&n.second;
    }

    return NULL;
}

#define get_pid_mapping(d, ad, pid) d->mapping.find(MAKE_KEY(ad, pid))
#define has_pid_mapping(d, ad, pid) (d->mapping.count(MAKE_KEY(ad, pid)) > 0)

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
    int ddci_pid = -1;
    ddci_mapping_table_t *m;
    if (!has_pid_mapping(d, ad, pid)) {
        ddci_pid = find_ddci_pid(d, pid);
        m = &d->mapping[MAKE_KEY(ad, pid)];
        m->ad = ad;
        m->pid = pid;
        m->ddci_pid = ddci_pid;
        m->rewrite = 1;
        m->ddci = d->id;
    }
    m = &d->mapping[MAKE_KEY(ad, pid)];
    ddci_pid = m->ddci_pid;
    if (ddci_pid == -1)
        LOG_AND_RETURN(
            -1, "could not add pid %d and ad %d to the mapping table", pid, ad);

    int add_pid = 1;
    if (m->pmt.count(pmt) > 0)
        LOG_AND_RETURN(
            0, "Adapter %d pid %d already mapped to pmt %d, already added %d",
            ad, pid, pmt, m->pid_added);

    add_pid = m->pmt.size() == 0;
    m->pmt.insert(pmt);

    if (add_pid && force_add_pid) {
        m->pid_added = 1;
        mark_pid_add(DDCI_SID, ad, pid);
        if (pid == 1)
            m->filter_id =
                add_filter(ad, 1, (void *)ddci_process_cat, d, FILTER_CRC);
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
            auto it = get_pid_mapping(ddci_devices[i], ad, pid);
            if (it != ddci_devices[i]->mapping.end())
                return &(it->second);
        }
    return NULL;
}

void dump_mapping_table() {
    int j;
    LOGM("Mapping Table for all devices");
    for (j = 0; j < MAX_ADAPTERS; j++)
        if (ddci_devices[j] && ddci_devices[j]->enabled) {
            for (const auto &[key, m] : ddci_devices[j]->mapping) {
                std::string out;
                for (const auto &elem : m.pmt)
                    out += std::to_string(elem) + " ";

                LOGM("DD %d, ddpid %d, adapter %d pid %d, rewrite %d: %d PMTS "
                     "%s",
                     m.ddci, m.ddci_pid, m.ad, m.pid, m.rewrite, m.pmt.size(),
                     out.c_str());
            }
        }
    return;
}

int set_pid_rewrite(ddci_device_t *d, int ad, int pid, int rewrite) {
    auto it = get_pid_mapping(d, ad, pid);
    if (it == d->mapping.end())
        return -1;
    it->second.rewrite = rewrite;
    return 0;
}

int del_pmt_mapping_table(ddci_device_t *d, int ad, int pmt) {
    int ddci_pid = -1, i;
    int filter_id, pid_added;
    int to_del[MAX_PIDS], n = 0;
    for (const auto &u : d->mapping) {
        ddci_mapping_table_t *m = (ddci_mapping_table_t *)&u.second;
        if (m->ad == ad) {
            m->pmt.erase(pmt);

            if (m->pmt.size() > 0)
                continue;

            ddci_pid = m->ddci_pid;
            filter_id = m->filter_id;
            pid_added = m->pid_added;
            int pid = m->pid;
            to_del[n++] = pid;

            SPid *p = find_pid(d->id, ddci_pid);
            LOG("Deleting ad %d pmt %d pid %d ddci_pid %d", ad, pmt, pid,
                ddci_pid);
            if (p)
                mark_pid_deleted(d->id, DDCI_SID, ddci_pid, NULL);
            if (filter_id >= 0)
                del_filter(filter_id);
            if (pid_added >= 0) {
                LOGM("Marking pid %d deleted on adapter %d (initial ad %d pid "
                     "%d)",
                     ddci_pid, d->id, ad, pid);
                mark_pid_deleted(ad, DDCI_SID, pid, NULL);
            }
        }
    }

    for (i = 0; i < n; i++)
        d->mapping.erase(MAKE_KEY(ad, to_del[i]));

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

int ddci_close() { return 0; }
int ddci_close_adapter(adapter *a) { return 0; }

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
                    c->ddci[c->ddcis++].ddci = i;
                    c->sid = pmt->sid;
                    safe_strncpy(c->name, pmt->name);
                }
        }
    return 0;
}

/**
 * Find the DDCI device that should be used to handle the specified PMT. The
 * first suitable device is returned. If a suitable device is found but the
 * device is still initializing, -1 is returned. If no suitable device is found,
 * -2 is returned.
 */
int find_ddci_for_pmt(Sddci_channel *c, SPMT *pmt) {
    int ddid = -100; // -100 means we didn't find a suitable device

    int i = 0;
    for (i = 0; i < c->ddcis; i++) {
        int candidate = c->ddci[i].ddci;
        ddci_device_t *d = get_ddci(candidate);
        if (!d) {
            LOG("%s: DDCI %d not enabled, skipping", __FUNCTION__, candidate);
            continue;
        }

        if (d->channels >= d->max_channels) {
            LOG("%s: DDCI %d cannot be used for PMT %d, pid %d, sid, %d (used "
                "channels "
                "%d max %d), skipping",
                __FUNCTION__, candidate, pmt->id, pmt->pid, pmt->sid,
                d ? d->channels : -1, d ? d->max_channels : -1);
            continue;
        }

        ddid = candidate;
        break;
    }

    if (ddid == -100) {
        // Distinguish between otherwise not found and deliberately not assigned
        if (c->ddcis == 0) {
            LOG("%s: no suitable DDCI found for PMT %d (sid %d): not mapped to "
                "any DDCI "
                "device in ddci.conf",
                __FUNCTION__, pmt->id, pmt->sid);
        } else {
            LOG("%s: no suitable DDCI found for PMT %d (sid %d)", __FUNCTION__,
                pmt->id, pmt->sid);
        }

        return -TABLES_RESULT_ERROR_NORETRY;
    } else if (is_ca_initializing(ddid)) {
        LOG("%s: DDCI %d is initializing, retrying", __FUNCTION__, ddid);
        return -TABLES_RESULT_ERROR_RETRY;
    } else {
        LOG("%s: Using DDCI %d", __FUNCTION__, ddid);
        return ddid;
    }
}

int is_pmt_running(SPMT *pmt) {
    ddci_mapping_table_t *m =
        get_pid_mapping_allddci(pmt->adapter, pmt->stream_pid[0]->pid);
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
                    if (m && m->pid == dpmt->pid) {
                        safe_strncpy(pmt->name, dpmt->name);
                    }
                }
            }

        return TABLES_RESULT_OK;
    }

    ddid = is_pmt_running(pmt);

    LOG("%s: adapter %d, pmt %d, pid %d, sid %d, ddid %d, name: %s",
        __FUNCTION__, ad->id, pmt->id, pmt->pid, pmt->sid, ddid, pmt->name);

    auto it = channels.find(pmt->sid);
    if (it == channels.end()) {
        Sddci_channel *c = &channels[pmt->sid];
        int result = create_channel_for_pmt(c, pmt);
        if (result) {
            channels.erase(pmt->sid);
            LOG_AND_RETURN(result, "DDCI not ready or busy at the moment: %s",
                           result == TABLES_RESULT_ERROR_NORETRY ? "no retry"
                                                                 : "retry");
        }
        if (c->ddcis == 0) {
            channels.erase(pmt->sid);
            LOG_AND_RETURN(TABLES_RESULT_ERROR_NORETRY,
                           "no suitable DDCI found");
        }
    }
    channel = &channels[pmt->sid];

    // Determine which DDCI should handle this PMT
    if (ddid == -1)
        ddid = find_ddci_for_pmt(channel, pmt);
    // Negative return values are used to distinguish from valid return values
    // (>= 0)
    if (ddid == -TABLES_RESULT_ERROR_RETRY)
        return TABLES_RESULT_ERROR_RETRY;
    else if (ddid == -TABLES_RESULT_ERROR_NORETRY)
        return TABLES_RESULT_ERROR_NORETRY;

    d = get_ddci(ddid);
    if (!d) {
        LOG("Could not find ddci device for adapter %d, ddci %d", ad->id, ddid);
        return TABLES_RESULT_ERROR_NORETRY;
    }

    std::lock_guard<SMutex> lock(d->mutex);
    int pos = -1;

    for (i = 0; i < d->max_channels; i++)
        if (d->pmt[i].id == pmt->id)
            pos = i;

    if (pos == -1) {
        for (i = 0; i < d->max_channels; i++)
            if (d->pmt[i].id == -1) {
                pos = i;
                break;
            }
    }

    if (pos == -1) {
        LOG_AND_RETURN(TABLES_RESULT_ERROR_RETRY,
                       "No free slot found for pmt %d on DDCI %d", pmt->id,
                       d->id);
    }

    d->pmt[pos].id = pmt->id;
    d->pmt[pos].ver = (d->pmt[pos].ver + 1) & 0xF;

    d->ver = (d->ver + 1) & 0xF;
    if (!d->channels++) { // for first PMT set transponder ID
        d->tid = ad->transponder_id;
    }

    // if the CAT is not mapped, add it
    if (!has_pid_mapping(d, pmt->adapter, 1)) {
        LOG("Mapping CAT to PMT %d from transponder %d, DDCI transponder %d",
            pmt->id, ad->transponder_id, d->tid)
        add_pid_mapping_table(ad->id, 1, pmt->id, d, 1);
        d->cat_processed = 0;
    }

    // Some CAMs need access to the TDT in order to "wake up", so always map it
    // just in case
    if (!has_pid_mapping(d, pmt->adapter, 20)) {
        LOG("Mapping TDT to PMT %d from transponder %d, DDCI transponder %d",
            pmt->id, ad->transponder_id, d->tid);
        add_pid_mapping_table(ad->id, 20, pmt->id, d, 1);
    }

    LOG("found DDCI %d for pmt %d, running channels %d, max_channels %d", ddid,
        pmt->id, d->channels, d->max_channels);

    add_pid_mapping_table(ad->id, pmt->pid, pmt->id, d, 0);
    set_pid_rewrite(d, ad->id, pmt->pid,
                    0); // do not send the PMT pid to the DDCI device

    for (i = 0; i < pmt->caids; i++) {
        LOGM("DD %d adding ECM pid %d", d->id, pmt->ca[i]->pid);
        add_pid_mapping_table(ad->id, pmt->ca[i]->pid, pmt->id, d, 1);
    }

    for (i = 0; i < pmt->stream_pids; i++) {
        LOGM("DD %d adding stream pid %d %s", d->id, pmt->stream_pid[i]->pid,
             pmt->stream_pid[i]->pid == pmt->pcr_pid ? "PCR" : "");

        int ddci_pid = add_pid_mapping_table(ad->id, pmt->stream_pid[i]->pid,
                                             pmt->id, d, 0);
        // map the PCR pid as well
        if (pmt->stream_pid[i]->pid == pmt->pcr_pid) {
            d->pmt[pos].pcr_pid = ddci_pid;
        }
    }

    update_pids(ad->id);
    update_pids(d->id);
    rv = TABLES_RESULT_OK;
    dump_mapping_table();

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

    for (i = 0; i < d->max_channels; i++)
        if (d->pmt[i].id == pmt) {
            d->pmt[i].id = -1;
        }

    del_pmt_mapping_table(d, ad->id, pmt);
    update_pids(d->id);
    dump_mapping_table();
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
    b[6] = 0xC1 | (d->ver << 1);
    b[7] = b[8] = 0;
    // Channel ID 0
    b[9] = b[10] = 0;
    // PID 16
    b[11] = 0x00;
    b[12] = 0x10;
    len = 13;
    for (i = 0; i < d->max_channels; i++)
        if ((pmt = get_pmt(d->pmt[i].id))) {
            auto it = get_pid_mapping(d, pmt->adapter, pmt->pid);
            if (it == d->mapping.end()) {
                LOG("adapter %d pid %d not found in the mapping table",
                    pmt->adapter, pmt->pid);
                continue;
            }

            copy16(b, len, pmt->sid);
            copy16(b, len + 2, 0xE000 | it->second.ddci_pid);
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
    hexdump((const char *)buf, b + 1, len1 - 1);
    return len;
}

int ddci_create_sdt(ddci_device_t *d, uint8_t *sdt) {
    uint8_t *b = sdt;

    *b++ = 0x00;
    // Payload
    uint8_t *payload_start = b;
    *b++ = 0x42; // table_id, current transport stream
    // section_syntax_indicator, reserved_future_use, reserved, section_length
    *b++ = 0xF0; // 11110000
    *b++ = 0x00; // 00000000
    uint8_t *section_start = b;
    // transport_stream_id
    copy16(b, 0, 1);
    b += 2;
    // reserved, version_number, current_next_indicator
    *b++ = 0xC1 | (d->ver << 1);
    // section_number, last_section_number
    copy16(b, 0, 0);
    b += 2;
    // original_network_id, reserved_future_use
    copy16(b, 0, 0);
    b += 2;
    // reserved_future_use
    *b++ = 0x00;
    // describe each service
    int i;
    SPMT *pmt;
    for (i = 0; i < d->max_channels; i++) {
        if ((pmt = get_pmt(d->pmt[i].id))) {
            LOGM("Adding PMT %d to SDT, sid %d", d->pmt[i].id, pmt->sid);
            if (!has_pid_mapping(d, pmt->adapter, pmt->pid)) {
                LOG("adapter %d pid %d not found in the mapping table",
                    pmt->adapter, pmt->pid);
                continue;
            }

            // service_id
            copy16(b, 0, pmt->sid);
            b += 2;
            // reserved_future_use, EIT_schedule_flag,
            // EIT_present_following_flag
            *b++ = 0x00;
            // running_status, free_CA_mode, descriptors_length
            uint8_t r = 4 << 5; // running_status = 4
            r ^= 1 << 4;        // free_CA_mode = 1
            *b++ = r;
            *b++ = 0x00;
        }
    }
    // calculate section_length
    *(section_start - 1) = b - section_start + 4;
    // checksum
    int crc = crc_32(payload_start, b - payload_start);
    copy32(b, 0, crc);
    b += 4;

    return b - sdt;
}

uint16_t YMDtoMJD(int Y, int M, int D) {
    int L = (M < 3) ? 1 : 0;
    return 14956 + D + (int)((Y - L) * 365.25) +
           (int)((M + 1 + L * 12) * 30.6001);
}

// Based on vdr implementation provided by Klaus Schmidinger
int ddci_create_eit(ddci_device_t *d, int sid, uint8_t *eit, int version) {
    // Make an event that starts one hour in the past
    struct tm tm_r;
    time_t t =
        time(NULL) - 3600; // let's have the event start one hour in the past
    struct tm *tm = localtime_r(&t, &tm_r);
    uint16_t MJD = YMDtoMJD(tm->tm_year, tm->tm_mon + 1, tm->tm_mday);

    uint8_t *p = eit;

    *p++ = 0x00; // pointer field (payload unit start indicator is set)
    // payload:
    uint8_t *PayloadStart = p;
    *p++ = 0x4E; // TID present/following event on this transponder
    // section_syntax_indicator, reserved_future_use, reserved, section_length
    *p++ = 0xF0; // 11110000
    *p++ = 0x00; // 00000000
    uint8_t *SectionStart = p;
    // service_id
    copy16(p, 0, sid);
    p += 2;
    *p++ = 0xC1 | ((version & 0x0F) << 1);
    *p++ = 0x00;       // section number
    *p++ = 0x00;       // last section number
    *p++ = 0x00;       // transport stream id
    *p++ = 0x00;       // ...
    *p++ = 0x00;       // original network id
    *p++ = 0x00;       // ...
    *p++ = 0x00;       // segment last section number
    *p++ = 0x4E;       // last table id
    *p++ = 0x00;       // event id
    *p++ = 0x01;       // ...
    copy16(p, 0, MJD); // start time
    p += 2;
    *p++ = tm->tm_hour; // ...
    *p++ = tm->tm_min;  // ...
    *p++ = tm->tm_sec;  // ...
    *p++ = 0x24;        // duration (one day, should cover everything)
    *p++ = 0x00;        // ...
    *p++ = 0x00;        // ...
    uint8_t r = 4 << 5; // running_status = 4
    r ^= 1 << 4;        // free_CA_mode = 1
    *p++ = r;
    *p++ = 0x00; // descriptors_length
    uint8_t *DescriptorsStart = p;
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
    copy32(p, 0, crc);
    p += 4;

    return p - eit;
}

int safe_get_pid_mapping(ddci_device_t *d, int aid, int pid) {
    auto it = get_pid_mapping(d, aid, pid);
    if (it != d->mapping.end()) {
        return it->second.ddci_pid;
    }
    return pid;
}

int ddci_create_pmt(ddci_device_t *d, SPMT *pmt, uint8_t *new_pmt, int pmt_size,
                    ddci_pmt_t *dp) {
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
    *b++ = (dp->ver << 1) | 0x01;
    *b++ = 0;                                  // section number
    *b++ = 0;                                  // last section number
    *b++ = 0xE0 | ((dp->pcr_pid >> 8) & 0xFF); // PCR PID
    *b++ = dp->pcr_pid & 0xFF;                 // PCR PID

    start_pi_len = b;

    *b++ = 0; // PI LEN
    *b++ = 0;

    LOGM("%s: PMT %d AD %d, pid: %04X (%d), ver %d, sid %04X (%d) %s %s",
         __FUNCTION__, pmt->id, pmt->adapter, pid, pid, dp->ver, pmt->sid,
         pmt->sid, pmt->name[0] ? "channel:" : "", pmt->name);

    // Add CA IDs and CA Pids
    for (i = 0; i < pmt->caids; i++) {
        int private_data_len = pmt->ca[i]->private_data_len;
        *b++ = 0x09;
        *b++ = 0x04 + private_data_len;
        copy16(b, 0, pmt->ca[i]->id);
        copy16(b, 2, safe_get_pid_mapping(d, pmt->adapter, pmt->ca[i]->pid));
        memcpy(b + 4, pmt->ca[i]->private_data, private_data_len);
        pi_len += 6 + private_data_len;
        b += 4 + private_data_len;
        LOGM("%s: pmt %d added caid %04X, pid %04X", __FUNCTION__, pmt->id,
             pmt->ca[i]->id, pmt->ca[i]->pid);
    }
    copy16(start_pi_len, 0, pi_len);

    // Add Stream pids
    // Add CA IDs and CA Pids
    for (i = 0; i < pmt->stream_pids; i++) {
        if (get_ca_multiple_pmt(d->id)) {
            // Do not map any pids that are not requested by the client
            SPid *p = find_pid(pmt->adapter, pmt->stream_pid[i]->pid);
            if (!p) {
                p = find_pid(pmt->adapter, 8192); // all pids are requested
            }
            int is_added = 0, j;
            if (p) {
                for (j = 0; j < MAX_STREAMS_PER_PID; j++)
                    if (p->sid[j] >= 0 && p->sid[j] < MAX_STREAMS) {
                        is_added = 1;
                        break;
                    }
            }
            if (is_added == 0) {
                LOGM("%s: adapter %d pid %d not requested by the client",
                     __FUNCTION__, pmt->adapter, pmt->stream_pid[i]->pid);
                continue;
            }
        }

        *b = pmt->stream_pid[i]->type;
        copy16(b, 1,
               safe_get_pid_mapping(d, pmt->adapter, pmt->stream_pid[i]->pid));
        int desc_len = pmt->stream_pid[i]->desc_len;
        copy16(b, 3, desc_len);
        memcpy(b + 5, pmt->stream_pid[i]->desc, desc_len);
        b += desc_len + 5;

        LOGM("%s: pmt %d added pid %04X, type %02X", __FUNCTION__, pmt->id,
             pmt->stream_pid[i]->pid, pmt->stream_pid[i]->type);
    }
    // set the length (b + 4 bytes from crc)
    copy16(start_pmt, -2, 4 + b - start_pmt);

    uint32_t crc = crc_32(new_pmt + 1, b - new_pmt - 1);

    // Check if PMT has changed because the user may have added or removed pids
    if (dp->crc != crc) {
        dp->ver = (dp->ver + 1) & 0xF;
        start_pmt[2] = (dp->ver << 1) | 0x01;
        crc = crc_32(new_pmt + 1, b - new_pmt - 1);
    }
    dp->crc = crc;
    copy32(b, 0, crc);
    b += 4;
    return b - new_pmt;
}

int ddci_add_psi(ddci_device_t *d, uint8_t *dst, int len) {
    unsigned char psi[1500];
    int64_t ctime = getTick();
    int i, pos = 0;
    int psi_len;

    // Add PAT
    if (ctime - d->last_pat > 500) {
        psi_len = ddci_create_pat(d, psi);
        pos += buffer_to_ts(dst + pos, len - pos, psi, psi_len, &d->pat_cc, 0);
        d->last_pat = ctime;
    }

    // Add SDT
    if (ctime - d->last_sdt > 500) {
        psi_len = ddci_create_sdt(d, psi);
        pos += buffer_to_ts(dst + pos, len - pos, psi, psi_len, &d->sdt_cc, 17);
        d->last_sdt = ctime;
    }

    // Add PMTs
    if (ctime - d->last_pmt > 100) {
        SPMT *pmt;
        for (i = 0; i < d->max_channels; i++) {
            if ((pmt = get_pmt(d->pmt[i].id))) {
                psi_len = ddci_create_pmt(d, pmt, psi, sizeof(psi), d->pmt + i);
                auto it = get_pid_mapping(d, pmt->adapter, pmt->pid);
                if (it != d->mapping.end())
                    pos += buffer_to_ts(dst + pos, len - pos, psi, psi_len,
                                        &d->pmt[i].cc, it->second.ddci_pid);
                else
                    LOG("%s: could not find PMT %d adapter %d and pid %d to "
                        "mapping table",
                        __FUNCTION__, d->pmt[i].id, pmt->adapter, pmt->pid);

                // Add an EIT table for each channel
                psi_len = ddci_create_eit(d, pmt->sid, psi, d->pmt[i].ver);
                pos += buffer_to_ts(dst + pos, len - pos, psi, psi_len,
                                    &d->eit_cc, 18);
            }
        }
        d->last_pmt = ctime;
    }
    return pos;
}

// find a position starting with *ad_pos or at the end of the ad->buf
int push_ts_to_adapter(ddci_device_t *d, adapter *ad, uint16_t *mapping) {
    int i;
    int popped = 0;
    for (i = 0; i < ad->lbuf - DVB_FRAME; i += DVB_FRAME) {
        if ((i < ad->rlen) && (PID_FROM_TS(ad->buf + i) != 0x1FFF))
            continue;
        int len = fifo_pop_offset(&d->fifo, ad->buf + i, DVB_FRAME,
                                  &d->read_index[ad->id]);
        if (len == 0)
            break;
        popped += len;
        int pid = PID_FROM_TS(ad->buf + i);
        int dpid = mapping[pid];

        // pid is not recognised mapped to this adapter, ignore it
        if (dpid == 0) {
            DEBUGM("Skipping pid %d, ddci_pid %d on adapter %d DDCI %d", pid,
                   dpid, ad->id, d->id);
            set_pid_ts(ad->buf + i, 0x1FFF);
            i -= DVB_FRAME;
            continue;
        }

        set_pid_ts(ad->buf + i, dpid);
        if (i >= ad->rlen)
            ad->rlen = i + DVB_FRAME;
        DEBUGM(
            "DDCI %d pid %d -> AD %d pid %d, pos %d, len %d, left %jd [ri %jd]",
            d->id, pid, ad->id, dpid, i / DVB_FRAME, len,
            d->fifo.write_index - d->read_index[ad->id], d->read_index[ad->id]);
        dump_packets("DDCI -> AD", ad->buf + i, DVB_FRAME, i);
    }
    LOGM("popped %d bytes from fifo up to index %d, rlen %d, lbuf %d, left in "
         "fifo %jd",
         popped, i, ad->rlen, ad->lbuf,
         d->fifo.write_index - d->read_index[ad->id]);
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

    std::lock_guard<SMutex> lock(d->mutex);
    if (!d->enabled) {
        return 0;
    }

    if (!ad2) {
        return 0;
    }
    // step 0 - create ad_dd_pid and dd_ad_pid mapping
    memset(ad_dd_pids, 0, sizeof(ad_dd_pids));
    memset(dd_ad_pids, 0, sizeof(dd_ad_pids));
    for (const auto &[key, m] : d->mapping)
        if (m.ad == ad->id && m.rewrite) {
            ad_dd_pids[m.pid] = m.ddci_pid;
            dd_ad_pids[m.ddci_pid] = m.pid;
            ec++;
            DEBUGM("Using pid %d in adapter %d as pid %d in DDCI %d", m.pid,
                   ad->id, m.ddci_pid, d->id);
        }

    if (ec == 0) {
        d->read_index[ad->id] = 0;
        return 0;
    }
    // Reset to write index to prevent reading old data from the FIFO
    if (d->read_index[ad->id] == 0)
        d->read_index[ad->id] = d->fifo.write_index;

    // step 1 - fill the IO with TS packets and change the PID as
    // required by mapping table
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
        if (iop - 1 >= 0 &&
            ((uint8_t *)io[iop - 1].iov_base + io[iop - 1].iov_len == b))
            io[iop - 1].iov_len += DVB_FRAME;
        else {
            io[iop].iov_base = b;
            io[iop++].iov_len = DVB_FRAME;
        }
        bytes += 188;
        DEBUGM("pos %d of %d, mapping pid %d to %d, buf pos %d", iop - 1,
               rlen / 188, pid, dpid, i / 188);
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
        uint32_t j;
        for (j = 0; j < io[i].iov_len; j += 188)
            set_pid_ts((uint8_t *)io[i].iov_base + j, 0x1FFF);
    }

    // move back TS packets from the DDCI out buffer to the adapter buffer
    push_ts_to_adapter(d, ad, dd_ad_pids);
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

    ddci_id = add_ca(&ddci);
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

int ddci_read_sec_data(sockets *s) {
    unsigned char *b = s->buf;

    read_dmx(s);
    if (s->rlen != 0) {
        DEBUGM("process_dmx not called as s->rlen %d", s->rlen);
        return 0;
    }
    // copy the processed data to fifo from the DDCI adapter
    adapter *ad = get_adapter(s->sid);
    if (!ad) {
        return 0;
    }
    ddci_device_t *d = get_ddci(s->sid);
    b = ad->buf;
    int left = 0, rlen = ad->rlen;

    if ((left = fifo_push_force(&d->fifo, b, rlen, 1)) == 0)
        LOG("dropping %d bytes for ddci_adapter %d", left, d->id);
    LOGM("pushed %d bytes to the adapter buffer from %d [write_index %jd]",
         left, rlen, d->fifo.write_index);
    dump_packets("DDCI ->FIFO ", (uint8_t *)d->fifo.data, 188, 0);
    dump_packets("SOURCE ", b, 188, 0);
    return 0;
}

int ddci_post_init(adapter *ad) {
    ddci_device_t *d = ddci_devices[ad->id];

    // Cap max_channels to MAX_CHANNELS_ON_CI
    d->max_channels = get_max_pmt_for_ca(ad->id);
    if (d->max_channels > MAX_CHANNELS_ON_CI) {
        d->max_channels = MAX_CHANNELS_ON_CI;
    }

    sockets *s = get_sockets(ad->sock);
    s->action = (socket_action)ddci_read_sec_data;
    if (ad->fe_sock >= 0)
        set_socket_thread(ad->fe_sock, get_socket_thread(ad->sock));
    post_tune(ad);
    if (ddci_id < 0)
        ddci_init();
    return 0;
}

ddci_device_t *ddci_alloc(int id) {
    ddci_device_t *d;

    d = ddci_devices[id] = new ddci_device_t();
    create_fifo(&d->fifo, DDCI_BUFFER);
    d->id = id;
    return d;
}

int ddci_open_device(adapter *ad) {
    char buf[100];
    int read_fd, write_fd;
    ddci_device_t *d = ddci_devices[ad->id];
    if (!d && !(d = ddci_alloc(ad->id))) {
        return -1;
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
    int err;
    if ((err = set_linux_socket_nonblock(read_fd)))
        LOG("Failed to set socket %d nonblocking: %d", write_fd, err);

    if ((err = set_linux_socket_nonblock(write_fd)))
        LOG("Failed to set socket %d nonblocking: %d", write_fd, err);
#else
    int fd[2];
    if (pipe(fd) == -1) {
        LOG("pipe failed errno %d: %s", errno, strerror(errno));
        return 1;
    }
    read_fd = fd[0];
    write_fd = fd[1];

#endif
    std::lock_guard<SMutex> lock2(d->mutex);
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
    ad->sys[0] = (fe_delivery_system_t)0;
    ad->adapter_timeout = 0;
    memset(d->pmt, -1, sizeof(d->pmt));
    d->max_channels = MAX_CHANNELS_ON_CI;
    d->channels = 0;
    memset(d->read_index, 0, sizeof(d->read_index));
    d->last_pmt = d->last_pat = 0;
    d->tid = d->ver = 0;
    d->enabled = 1;
    ad->enabled = 1;
    LOG("opened DDCI adapter %d fe:%d dvr:%d", ad->id, ad->fe, ad->dvr);

    return 0;
}

fe_delivery_system_t ddci_delsys(int aid, int fd, fe_delivery_system_t *sys) {
    return (fe_delivery_system_t)0;
}

int ddci_process_cat(int filter, unsigned char *b, int len, void *opaque) {
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

    std::unique_lock<SMutex> lock(d->mutex);

    if (d->cat_processed || d->disable_cat) {
        return 0;
    }

    cat_len -= 9;
    b += 8;
    LOG("CAT DDCI %d len %d", d->id, cat_len);
    if (cat_len > 1500) {
        return 0;
    }

    id = 0;
    for (i = 0; i < cat_len; i += es_len) // reading program info
    {
        es_len = b[i + 1] + 2;
        if (b[i] != 9)
            continue;
        caid = b[i + 2] * 256 + b[i + 3];
        if (id < MAX_CA_PIDS) {
            d->capid[id] = (b[i + 4] & 0x1F) * 256 + b[i + 5];
            LOG("CAT pos %d caid %04X, pid %d", id, caid, d->capid[id]);
        } else {
            LOG("MAX_CA_PIDS (%d) reached for adapter %d", MAX_CA_PIDS, d->id);
        }

        id++;
    }

    add_cat = 1;

    for (i = 0; i < id; i++)
        if (get_ddci_pid(d, d->capid[i])) {
            add_cat = 0;
            LOG("CAT pid %d already in use, skipping CAT", d->capid[i]);
            d->cat_processed = 1;
            break;
        }
    if (!add_cat) {
        return 0;
    }

    // sending EMM pids to the CAM
    for (i = 0; i < id; i++) {
        add_pid_mapping_table(f->adapter, d->capid[i], d->pmt[0].id, d, 1);
    }
    d->cat_processed = 1;
    lock.unlock();
    update_pids(f->adapter);
    update_pids(d->id);
    return 0;
}

void save_channels() {
    int j;
    char ddci_conf_path[256];
    snprintf(ddci_conf_path, sizeof(ddci_conf_path) - 1, "%s/%s",
             opts.cache_dir, CONFIG_FILE_NAME);

    if (!access(ddci_conf_path, R_OK)) {
        LOG("%s already exists, not saving", ddci_conf_path);
        return;
    }

    FILE *f = fopen(ddci_conf_path, "wt");
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

    for (const auto &[key, value] : channels) {
        fprintf(f, "%d:    ", value.sid);
        for (j = 0; j < value.ddcis; j++)
            fprintf(f, "%s%d", j ? "," : "", value.ddci[j].ddci);
        fprintf(f, " # %s\n", value.name);
    }
    fclose(f);
}
void load_channels() {
    char ddci_conf_path[256];
    snprintf(ddci_conf_path, sizeof(ddci_conf_path) - 1, "%s/%s",
             opts.cache_dir, CONFIG_FILE_NAME);

    FILE *f = fopen(ddci_conf_path, "rt");
    char line[1000];
    int nchannels = 0;
    if (!f)
        return;
    while (fgets(line, sizeof(line), f)) {
        int pos = 0;
        char buf[1000];
        memset(buf, 0, sizeof(buf));
        char *x = strchr(line, '#');
        if (x)
            *x = 0;
        char *cc = strstr(line, ":");
        if (!cc)
            continue;

        int sid = map_int(line, NULL);
        if (!sid)
            continue;
        Sddci_channel *c = &channels[sid];
        c->sid = sid;
        c->locked = 1;
        cc = strip(cc + 1);

        char *arg[MAX_ADAPTERS];
        int la = split(arg, cc, ARRAY_SIZE(arg), ',');
        int i = 0;
        nchannels++;
        for (i = 0; i < la; i++) {
            int v = map_intd(arg[i], NULL, -1);
            if (v != -1) {
                c->ddci[c->ddcis++].ddci = v;
                strcatf(buf, pos, "%s%d", i ? "," : "", v);
            }
        }
        LOGM("Adding channel %d: DDCIs: %s", c->sid, buf);
    }
    fclose(f);
    LOG("Loaded %d channels from %s", nchannels, ddci_conf_path);
}

void disable_cat_adapters(char *o) {
    int i, la, st, end, j;
    char buf[strlen(o) + 1], *arg[40], *sep;
    safe_strncpy(buf, o);

    la = split(arg, buf, ARRAY_SIZE(arg), ',');
    for (i = 0; i < la; i++) {
        sep = strchr(arg[i], '-');
        if (sep == NULL) {
            st = map_int(arg[i], NULL);
            ddci_device_t *d = ddci_alloc(st);
            if (d) {
                LOG("Disable passing the CAT to DDCI %d", d->id);
                d->disable_cat = 1;
            }
        } else {
            st = map_int(arg[i], NULL);
            end = map_int(sep + 1, NULL);
            for (j = st; j <= end; j++) {
                ddci_device_t *d = ddci_alloc(j);
                if (d) {
                    LOG("Disable passing the CAT to DDCI %d", d->id);
                    d->disable_cat = 1;
                }
            }
        }
    }
}

void ddci_free(adapter *ad) {
    ddci_device_t *d = ddci_devices[ad->id];
    if (!d)
        return;
    save_channels();
}

void find_ddci_adapter(adapter **a) {
    int na = -1;
    char buf[100];
    int cnt;
    int i = 0, j = 0;

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
                ad->close = ddci_close_adapter;
                ad->get_signal = NULL;
                ad->set_pid = ddci_set_pid;
                ad->del_filters = ddci_del_filters;
                ad->type = ADAPTER_CI;
                ad->drop_encrypted = 0;
                ad->free = ddci_free;
                load_channels();

                na++;
                a_count = na; // update adapter counter
                if (na == MAX_ADAPTERS)
                    return;
#ifdef DDCI_TEST
                return;
#endif
            }
        }
    for (; na < MAX_ADAPTERS; na++)
        if (a[na])
            a[na]->pa = a[na]->fn = -1;
}
