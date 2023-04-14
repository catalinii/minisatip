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

#include "dvbapi.h"
#include "adapter.h"
#include "api/symbols.h"
#include "api/variables.h"
#include "dvb.h"
#include "minisatip.h"
#include "pmt.h"
#include "socketworks.h"
#include "tables.h"
#include "utils.h"
#include "utils/ticks.h"

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

#define DEFAULT_LOG LOG_DVBAPI

const int64_t DVBAPI_ITEM = 0x1000000000000;
int dvbapi_sock = -1;
int sock;
int dvbapi_is_enabled = 0;
int enabledKeys = 0;
int network_mode = 1;
int dvbapi_protocol_version = DVBAPI_PROTOCOL_VERSION;
int dvbapi_ca = -1;
int oscam_version;

uint64_t dvbapi_last_close = 0;

SKey *keys[MAX_KEYS];
SMutex keys_mutex;
unsigned char read_buffer[8192];
extern char *listmgmt_str[];

#define TEST_WRITE(a, xlen)                                                    \
    {                                                                          \
        int x;                                                                 \
        mutex_lock(&keys_mutex);                                               \
        if ((x = (a)) != (xlen)) {                                             \
            LOG("write to dvbapi socket failed (%d out of %d), closing "       \
                "socket %d, "                                                  \
                "errno %d, error: %s",                                         \
                x, (int)xlen, sock, errno, strerror(errno));                   \
            dvbapi_close_socket();                                             \
        }                                                                      \
        mutex_unlock(&keys_mutex);                                             \
    }

static inline SKey *get_key(int i) {
    if (i < 0 || i >= MAX_KEYS || !keys[i] || !keys[i]->enabled)
        return NULL;
    return keys[i];
}

void dvbapi_close_socket() {
    sockets_del(dvbapi_sock);
    sock = 0;
    dvbapi_sock = -1;
    dvbapi_is_enabled = 0;
}

void invalidate_adapter(int aid) { return; }

int get_index_for_filter(SKey *k, int filter) {
    int i;
    for (i = 0; i < MAX_KEY_FILTERS; i++)
        if (k->filter_id[i] == filter)
            return i;
    return -1;
}

#define dvbapi_copy32r(v, a, i)                                                \
    if (change_endianness)                                                     \
    copy32rr(v, a, i) else copy32r(v, a, i)
#define dvbapi_copy16r(v, a, i)                                                \
    if (change_endianness)                                                     \
    copy16rr(v, a, i) else copy16r(v, a, i)

int dvbapi_reply(sockets *s) {
    unsigned char *b = s->buf;
    SKey *k;
    int change_endianness = 0;
    unsigned int op, _pid;
    int k_id, a_id = 0, pos = 0;
    int demux, filter;

    if (s->rlen == 0) {
        send_client_info(s);
        return 0;
    }
    while (pos < s->rlen) {
        int op1;
        b = s->buf + pos;
        copy32r(op, b, 0);
        op1 = op & 0xFFFFFF;
        change_endianness = 0;
        if (op1 == CA_SET_DESCR_X || op1 == CA_SET_DESCR_AES_X ||
            op1 == CA_SET_PID_X || op1 == DMX_STOP_X ||
            op1 == DMX_SET_FILTER_X ||
            op1 == CA_SET_DESCR_AES_X) { // change endianness
            op = 0x40000000 | ((op1 & 0xFF) << 16) | (op1 & 0xFF00) |
                 ((op1 & 0xFF0000) >> 16);
            if (!(op & 0xFF0000))
                op &= 0xFFFFFF;
            LOG("dvbapi: changing endianness from %06X to %08X", op1, op);
            // b ++;
            // pos ++;
            b[4] = b[0];
            change_endianness = 1;
        }
        LOG("dvbapi read from socket %d the following data (%d bytes), pos = "
            "%d, "
            "op %08X, key %d",
            s->sock, s->rlen, pos, op, b[4]);
        //		LOGL(3, "dvbapi read from socket %d the following data
        //(%d
        // bytes), pos = %d, op %08X, key %d -> %02X %02X %02X %02X %02X %02X
        // %02X %02X %02X %02X %02X", s->sock, s->rlen, pos, op, b[4], b[0],
        // b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10]);

        switch (op) {

        case DVBAPI_SERVER_INFO:

            if (s->rlen < 6)
                return 0;
            dvbapi_copy16r(dvbapi_protocol_version, b, 4);
            char *oscam_version_str = strstr((const char *)b + 7, "build r");
            if (oscam_version_str)
                oscam_version = map_intd(oscam_version_str + 7, NULL, 0);
            LOG("dvbapi: server version %d, build %d, found, name = %s",
                dvbapi_protocol_version, oscam_version, b + 7);
            if (dvbapi_protocol_version > DVBAPI_PROTOCOL_VERSION)
                dvbapi_protocol_version = DVBAPI_PROTOCOL_VERSION;
            dvbapi_is_enabled = 1;
            pos = 6 + strlen((const char *)b + 6) + 1;
            if (oscam_version > 0 && oscam_version < 11533) {
                LOG0(
                    "Oscam build %d (< 11553) is not supported after minisatip "
                    "version 1.0.1. Upgrade your oscam binary",
                    oscam_version);
            } else {
                register_dvbapi();
            }
            break;

        case DVBAPI_DMX_SET_FILTER: {
            SKey *k;
            int not_found = 1;
            int i;
            if (change_endianness)
                pos += 2; // for some reason the packet is longer with 2 bytes
            pos += 65;
            dvbapi_copy16r(_pid, b, 7);
            _pid &= 0x1FFF;
            k_id = b[4] - opts.dvbapi_offset;
            k = get_key(k_id);
            a_id = -1;
            if (k)
                a_id = k->adapter;
            demux = b[5];
            filter = b[6];
            i = -1;
            int fid = -1;
            int isEMM = b[9] & b[25];
            if (isEMM >= 0x82 && isEMM <= 0x8F)
                isEMM = FILTER_EMM;
            else
                isEMM = 0;
            LOG("dvbapi set filter for pid %04X (%d), key %d, demux %d, filter "
                "%d "
                "%s%s",
                _pid, _pid, k_id, demux, filter, !k ? "(KEY NOT VALID)" : "",
                isEMM ? " EMM" : "");
            LOGM("filter: %02X %02X %02X %02X %02X, mask: %02X %02X %02X %02X "
                 "%02X",
                 b[9], b[10], b[11], b[12], b[13], b[25], b[26], b[27], b[28],
                 b[29]);

            if (k) {
                int new_filter_id = -1, fpos = -1;

                for (i = 0; i < MAX_KEY_FILTERS; i++)
                    if (k->filter_id[i] >= 0 && k->pid[i] == _pid &&
                        k->demux[i] == demux && k->filter[i] == filter) {
                        not_found = 0;
                        fpos = i;
                        new_filter_id = k->filter_id[i];
                        break;
                    }
                if (not_found) {

                    fid = add_filter_mask(k->adapter, _pid, (void *)send_ecm,
                                          (void *)k, FILTER_ADD_REMOVE | isEMM,
                                          b + 9, b + 25);
                    i = get_index_for_filter(k, -1);
                } else {
                    LOG("dvbapi: filter for pid %d and key %d already exists, "
                        "fid %d",
                        _pid, k->id, new_filter_id);

                    if (set_filter_flags(new_filter_id,
                                         FILTER_ADD_REMOVE | isEMM)) {
                        k->filter_id[fpos] = -1;
                        k->pid[fpos] = -1;
                    }
                    break;
                }
            }
            if (i >= 0 && fid >= 0 && k) {
                k->filter_id[i] = fid;
                k->filter[i] = filter;
                k->demux[i] = demux;
                k->pid[i] = _pid;
                k->ecm_parity[i] = -1;
                k->ecms++;
                update_pids(a_id);
            } else if (not_found)
                LOG("dvbapi: DMX_SET_FILTER failed, fid %d, index %d", fid, i);
            break;
        }
        case DVBAPI_DMX_STOP: {
            int i;
            k_id = b[4] - opts.dvbapi_offset;
            demux = b[5];
            filter = b[6];
            pos += 9;
            k = get_key(k_id);
            if (!k)
                break;
            a_id = k->adapter;
            dvbapi_copy16r(_pid, b, 7) _pid &= 0x1FFF;
            for (i = 0; i < MAX_KEY_FILTERS; i++)
                if (k->filter[i] == filter && k->demux[i] == demux &&
                    k->pid[i] == _pid)
                    break;
            LOG("dvbapi: received DMX_STOP for key %d, index %d, adapter %d, "
                "demux "
                "%d, filter %d, pid %X (%d)",
                k_id, i, a_id, demux, filter, _pid, _pid);
            if (i < MAX_KEY_FILTERS && i >= 0) {
                del_filter(k->filter_id[i]);
                k->filter_id[i] = -1;
                k->pid[i] = -1;
            }

            if (k) {
                if (k->ecms > 0)
                    k->ecms--;
                k->last_dmx_stop = getTick();
            }

            break;
        }
        case DVBAPI_CA_SET_PID: {
            LOG("received DVBAPI_CA_SET_PID");
            pos += 13;
            break;
        }
        case DVBAPI_CA_SET_DESCR: {
            int index, parity, k_id;
            SKey *k;
            unsigned char *cw;

            pos += 21;
            k_id = b[4] - opts.dvbapi_offset;
            dvbapi_copy32r(index, b, 5);
            dvbapi_copy32r(parity, b, 9);
            cw = b + 13;
            k = get_key(k_id);
            if (k && (parity < 2)) {
                int correct = (((cw[0] + cw[1] + cw[2]) & 0xFF) == cw[3]) &&
                              (((cw[4] + cw[5] + cw[6]) & 0xFF) == cw[7]);
                mutex_lock(&k->mutex);

                k->key_len = 8;
                memcpy(k->cw[parity], cw, k->key_len);

                LOG("dvbapi: received DVBAPI_CA_SET_DESCR, key %d parity %d, "
                    "index %d, "
                    "CW %s: %02X %02X %02X %02X %02X %02X %02X %02X",
                    k_id, parity, index, correct ? "OK" : "NOK", cw[0], cw[1],
                    cw[2], cw[3], cw[4], cw[5], cw[6], cw[7]);

                send_cw(k->pmt_id, k->algo, parity, cw, NULL, 0, &k->icam_ecm);

                mutex_unlock(&k->mutex);
            } else
                LOG("dvbapi: invalid DVBAPI_CA_SET_DESCR, key %d parity %d, k "
                    "%p, "
                    "index %d, CW: %02X %02X %02X %02X %02X %02X %02X %02X",
                    k_id, parity, k, index, cw[0], cw[1], cw[2], cw[3], cw[4],
                    cw[5], cw[6], cw[7]);
            break;
        }

        case DVBAPI_ECM_INFO: {
            int pos1 = s->rlen - pos;
            SKey *k = get_key(b[4] - opts.dvbapi_offset);
            unsigned char cardsystem[255];
            unsigned char reader[255];
            unsigned char from[255];
            unsigned char protocol[255];
            unsigned char len = 0;
            unsigned char *msg[5] = {cardsystem, reader, from, protocol, NULL};
            int i = 5, j = 0;
            uint16_t sid;

            copy16r(sid, b, i);

            if (k) {
                mutex_lock(&k->mutex);
                msg[0] = k->cardsystem;
                msg[1] = k->reader;
                msg[2] = k->from;
                msg[3] = k->protocol;
                copy16r(k->caid, b, i + 2);
                copy16r(k->info_pid, b, i + 4);
                copy32r(k->prid, b, i + 6);
                copy32r(k->ecmtime, b, i + 10);
            }
            i += 14;
            while (msg[j] && i < pos1) {
                len = b[i++];
                memset(msg[j], 0, sizeof(k->cardsystem));
                if (len >= sizeof(k->cardsystem) - 2)
                    len = sizeof(k->cardsystem) - 2;
                memcpy(msg[j], b + i, len);
                msg[j][len] = 0;
                i += len;
                j++;
            }
            if (i < pos1 && k)
                k->hops = b[i++];
            if (k)
                mutex_unlock(&k->mutex);
            pos += i;
            LOG("dvbapi: ECM_INFO: key %d, SID = %04X, CAID = %04X (%s), PID = "
                "%d "
                "(%04X), ProvID = %06X, ECM time = %d ms, reader = %s, from = "
                "%s, "
                "protocol = %s, hops = %d",
                k ? k->id : -1, sid, k ? k->caid : 0, msg[0],
                k ? k->info_pid : 0, k ? k->info_pid : 0, k ? k->prid : 0,
                k ? k->ecmtime : -1, msg[1], msg[2], msg[3], k ? k->hops : 0);
            break;
        }

        case CA_SET_DESCR_MODE: {
            int k_id, algo, mode;
            SKey *k;
            pos += 17;
            k_id = b[4] - opts.dvbapi_offset;
            dvbapi_copy32r(algo, b, 5);
            dvbapi_copy32r(mode, b, 9);
            LOG("Key %d, Algo set to %d, Mode set to %d", k_id, algo, mode);
            k = get_key(k_id);
            if (!k)
                break;
            set_algo(k, algo, mode);
            break;
        }

        default: {
            LOG("dvbapi: unknown operation: %02X %02X %02X %02X %02X %02X %02X "
                "%02X "
                "%02X %02X %02X",
                b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8], b[9],
                b[10]);
            pos = s->rlen;
        }
        }
    }
    s->rlen = 0;
    return 0;
}

int dvbapi_send_pmt(SKey *k, int cmd_id) {
    unsigned char buf[2500];
    int len;
    int listmgmt = CLM_UPDATE;
    SPMT *pmt = get_pmt(k->pmt_id);
    if (!pmt)
        return 1;
    mutex_lock(&keys_mutex);
    adapter *ad = get_adapter(k->adapter);
    int demux = 0;
    int adapter = 0;
    if (network_mode) {
        demux = k->id + opts.dvbapi_offset;
        adapter = k->id + opts.dvbapi_offset;
    } else if (ad) {
        demux = ad->fn;
        adapter = ad->pa;
    }

    if (cmd_id == CMD_ID_OK_DESCRAMBLING) {
        listmgmt = CLM_ONLY;
        int i;
        for (i = 0; i < MAX_KEYS; i++)
            if (keys[i] && i != k->id && keys[i]->enabled)
                listmgmt = CLM_ADD;
    }
    memset(buf, 0, sizeof(buf));
    copy32(buf, 0, AOT_CA_PMT);
    copy16(buf, 7, k->sid);
    buf[9] = 1;
    k->demux_index = demux;
    k->adapter_index = adapter;

    buf[12] = cmd_id;

    copy16(buf, 13, 0x8301); // adapter_device_descriptor, works only in
                             // newer versions (> 11500)
    buf[15] = adapter;
    copy16(buf, 16, 0x8402); // pmt_pid_descriptor
    copy16(buf, 18, pmt->pid);

    copy16(buf, 20, 0x8601); // demux_id_descriptor
    buf[22] = demux;

    copy16(buf, 23, 0x8701); // ca_device_descriptor (caX)
    buf[25] = demux;

    len =
        26 + pmt_add_ca_descriptor(pmt, buf + 26, dvbapi_ca); // CA description

    // Pids associated with the PMT
    copy16(buf, 10, len - 12);
    int i;
    for (i = 0; i < pmt->stream_pids; i++) {
        len += 5;
        int type = pmt->stream_pid[i].type;
        int pid = pmt->stream_pid[i].pid;
        buf[len - 5] = type;
        copy16(buf, len - 4, pid);
        copy16(buf, len - 2, 0);
        LOGM("Key %d adding stream pid %d (%X) type %d (%x)", k->id, pid, pid,
             type, type);
    }

    copy16(buf, 4, len - 6);

    buf[6] = listmgmt;
    mutex_unlock(&keys_mutex);
    if (sock > 0) {
        LOG("Sending pmt %d to dvbapi server for pid %d, Channel ID %04X, key "
            "%d, "
            "adapter %d, demux %d, using socket %d (%s)",
            k->pmt_id, k->pmt_pid, k->sid, k->id, adapter, demux, sock,
            listmgmt_str[listmgmt]);
        TEST_WRITE(write(sock, buf, len), len);
    }
    return 0;
}

int dvbapi_close(sockets *s) {
    int i;
    LOG("requested dvbapi close for sock %d, sock_id %d", s->id, s->sock);
    sock = -1;
    dvbapi_is_enabled = 0;
    SKey *k;
    for (i = 0; i < MAX_KEYS; i++)
        if (keys[i] && keys[i]->enabled) {
            k = get_key(i);
            if (!k)
                continue;
            keys_del(i);
        }
    return 0;
}

int dvbapi_timeout(sockets *s) {
    //    if (!enabledKeys)
    //        return 1; // close dvbapi connection
    return 0;
}

int is_adapter_active() {
    extern adapter *a[];
    int i, active_adapters = 0;
    for (i = 0; i < MAX_ADAPTERS; i++)
        if (a[i] && a[i]->enabled) {
            active_adapters = 1;
            break;
        }
    return active_adapters;
}

int connect_dvbapi(void *arg) {
    if ((sock > 0) && dvbapi_is_enabled) // already connected
    {
        int i;
        int64_t ctime = getTick();

        for (i = 0; i < MAX_KEYS; i++) {
            if (network_mode && keys[i] && keys[i]->enabled &&
                (keys[i]->ecms == 0) && (keys[i]->last_dmx_stop > 0) &&
                (ctime - keys[i]->last_dmx_stop > 3000)) {
                int pmt_id = keys[i]->pmt_id, adapter_id = keys[i]->adapter;
                LOG("Key %d active but no active filter, closing ", i);
                keys_del(i);

                // resent the PMT if the decrypting stops
                SPMT *pmt = get_pmt(pmt_id);
                if (pmt)
                    close_pmt_for_ca(dvbapi_ca, get_adapter(adapter_id), pmt);
            }
        }

        if (!is_adapter_active())
            dvbapi_close_socket();

        return 0;
    }

    dvbapi_is_enabled = 0;

    if (!opts.dvbapi_port || !opts.dvbapi_host[0])
        return 0;

    if (!is_adapter_active())
        return 0;

    if (sock <= 0) {
        if (opts.dvbapi_host[0] == '/') {
            network_mode = 0;
            sock = connect_local_socket(opts.dvbapi_host, 1);
        } else
            sock = tcp_connect(opts.dvbapi_host, opts.dvbapi_port, NULL, 0);
        if (sock < 0) {
            unregister_dvbapi();
            LOG_AND_RETURN(0, "%s: connect to %s failed", __FUNCTION__,
                           opts.dvbapi_host);
        }
        dvbapi_sock = sockets_add(sock, NULL, -1, TYPE_TCP | TYPE_CONNECT,
                                  (socket_action)dvbapi_reply,
                                  (socket_action)dvbapi_close,
                                  (socket_action)dvbapi_timeout);
        if (dvbapi_sock < 0)
            LOG_AND_RETURN(0, "%s: sockets_add failed", __FUNCTION__);
        set_socket_buffer(dvbapi_sock, read_buffer, sizeof(read_buffer));
        sockets_timeout(dvbapi_sock, 2000); // 2s timeout to close the socket
        return 0;
    }
    return 0;
}

int poller_sock;
void init_dvbapi() {
    int sec = 1;
    poller_sock = sockets_add(SOCK_TIMEOUT, NULL, -1, TYPE_UDP, NULL, NULL,
                              (socket_action)connect_dvbapi);
    sockets_timeout(poller_sock, sec * 1000); // try to connect every 1s
    set_sockets_rtime(poller_sock, -sec * 1000);
    mutex_init(&keys_mutex);
}

void send_client_info(sockets *s) {
    char buf[1000];
    unsigned char len;
    memset(buf, 0, sizeof(buf));
    copy32(buf, 0, DVBAPI_CLIENT_INFO);
    copy16(buf, 4, dvbapi_protocol_version) len =
        sprintf(buf + 7, "%s/%s", app_name, version);
    buf[6] = len;
    dvbapi_is_enabled = 1;
    TEST_WRITE(write(s->sock, buf, len + 7), len + 7);
}

int send_ecm(int filter_id, unsigned char *b, int len, void *opaque) {
    SKey *k = NULL;
    SPMT *pmt, *master;
    uint8_t buf[1600];
    int i, pid;
    int filter, demux;
    int old_parity;
    int valid_cw;

    if (!dvbapi_is_enabled)
        return 0;
    pid = get_filter_pid(filter_id);
    if (pid == -1)
        LOG_AND_RETURN(0, "%s: filter not found for pid %d", __FUNCTION__, pid);

    k = (void *)opaque;
    if (!k || !k->enabled)
        LOG_AND_RETURN(0, "%s: key is null pid %d and filter %d", __FUNCTION__,
                       pid, filter_id);
    pmt = get_pmt(k->pmt_id);
    if (!pmt)
        LOG_AND_RETURN(0, "%s: PMT not found for pid %d and filter %d",
                       __FUNCTION__, pid, filter_id);

    i = get_index_for_filter(k, filter_id);
    if (i == -1)
        LOG_AND_RETURN(0, "%s: filter %d not found", __FUNCTION__, filter_id);

    demux = k->demux[i];
    filter = k->filter[i];

    valid_cw = pmt->cw != NULL;
    master = get_pmt(pmt->master_pmt);
    if (master)
        valid_cw = master->cw != NULL;

    if ((getTick() - k->last_ecm > 1000) && !valid_cw)
        k->ecm_parity[i] = -1;

    if (!opts.send_all_ecm && (b[0] == 0x80 || b[0] == 0x81) && (b[0] & 1) == k->ecm_parity[i])
        return 0;

    old_parity = k->ecm_parity[i];
    k->ecm_parity[i] = b[0] & 1;

    len = ((b[1] & 0xF) << 8) + b[2];
    len += 3;
    k->last_ecm = getTick();
    if (b[2] - b[4] == 4)
        k->icam_ecm = b[0x15];
    LOG("dvbapi: sending ECM key %d for pid %04X (%d), ecm_parity = %d, "
        "previous parity %d, demux = %d, filter = %d, icam_ecm %d, len = %d "
        "[%02X %02X %02X "
        "%02X]",
        k->id, pid, pid, k->ecm_parity[i], old_parity, demux, filter,
        k->icam_ecm, len, b[0], b[1], b[2], b[3]);

    if (demux < 0)
        return 0;

    if (len > 559 + 3)
        return -1;

    copy32(buf, 0, DVBAPI_FILTER_DATA);
    buf[4] = demux;
    buf[5] = filter;
    memcpy(buf + 6, b, len);
    //	hexdump("ecm: ", buf, len + 6);
    if (sock > 0)
        TEST_WRITE(write(sock, buf, len + 6), len + 6);
    return 0;
}

int set_algo(SKey *k, int algo, int mode) {
    if (algo == CA_ALGO_AES128 && mode == CA_MODE_CBC)
        algo = CA_ALGO_AES128_CBC;
    k->algo = algo;

    return 0;
}

int keys_add(int i, int adapter, int pmt_id) {

    SKey *k;
    SPMT *pmt = get_pmt(pmt_id);
    if (!pmt)
        LOG_AND_RETURN(-1, "%s: PMT %d not found ", __FUNCTION__, pmt_id);
    if (i == -1)
        i = add_new_lock((void **)keys, MAX_KEYS, sizeof(SKey), &keys_mutex);
    else {
        if (keys[i])
            mutex_lock(&keys[i]->mutex);
        else {
            keys[i] = malloc1(sizeof(SKey));
            if (!keys[i])
                LOG_AND_RETURN(-1, "Could not allocate memory for the key %d",
                               i);
            memset(keys[i], 0, sizeof(SKey));
            mutex_init(&keys[i]->mutex);
            mutex_lock(&keys[i]->mutex);
        }
    }
    if (i == -1 || !keys[i]) {
        LOG_AND_RETURN(-1, "Key buffer is full, could not add new keys");
    }

    k = keys[i];

    k->parity = -1;
    k->sid = pmt->sid;
    k->pmt_id = pmt_id;
    k->adapter = adapter;
    k->adapter_index = -1;
    k->demux_index = -1;
    k->id = i;
    k->blen = 0;
    k->enabled = 1;
    k->ver = -1;
    k->ecms = 0;
    k->last_dmx_stop = 0;
    k->onid = 0;
    k->tsid = 0;
    k->icam_ecm = 0;
    memset(k->cw[0], 0, 16);
    memset(k->cw[1], 0, 16);
    memset(k->filter_id, -1, sizeof(k->filter_id));
    memset(k->filter, -1, sizeof(k->filter));
    memset(k->demux, -1, sizeof(k->demux));
    mutex_unlock(&k->mutex);
    invalidate_adapter(adapter);
    __sync_add_and_fetch(&enabledKeys, 1);
    LOG("returning new key %d for adapter %d, pmt %d pid %d sid %04X", i,
        adapter, pmt->id, pmt->pid, k->sid);

    return i;
}

int keys_del(int i) {
    int pmt_pid, sid;
    unsigned char buf[8] = {0x9F, 0x80, 0x3f, 4, 0x83, 2, 0, 0};
    SKey *k;
    char *msg = "";
    k = get_key(i);
    if (!k)
        return 0;

    mutex_lock(&k->mutex);
    if (!k->enabled) {
        mutex_unlock(&k->mutex);
        return 0;
    }
    // current key is not disabled
    int j, ek = 0, ed = 0;
    for (j = 0; j < MAX_KEYS; j++)
        if (i != j && keys[j] && keys[j]->enabled) {
            ek++;
            if (k->demux_index == keys[j]->demux_index)
                ed++;
        }
    if (!ek) {
        buf[7] = 0xFF;
        msg = "ALL";
        if (sock > 0)
            TEST_WRITE(write(sock, buf, sizeof(buf)), sizeof(buf));
    } else if (!ed) {
        buf[7] = k->demux_index;
        msg = "DEMUX";
        if (sock > 0)
            TEST_WRITE(write(sock, buf, sizeof(buf)), sizeof(buf));
    } else { // only local socket mode where multiple channels are connected to
             // the same demux
        msg = "PMT";
        dvbapi_send_pmt(k, CMD_ID_NOT_SELECTED);
    }

    pmt_pid = k->pmt_pid;
    sid = k->sid;

    k->sid = 0;
    k->pmt_pid = 0;
    k->adapter = -1;
    k->last_dmx_stop = 0;
    k->demux_index = -1;
    k->adapter_index = -1;
    k->enabled = 0;
    for (j = 0; j < MAX_KEY_FILTERS; j++)
        if (k->filter_id[j] >= 0)
            del_filter(k->filter_id[j]);

    k->hops = k->caid = k->info_pid = k->prid = k->ecmtime = 0;
    dvbapi_last_close = getTick();

    LOG("Stopped key %d, active keys %d, sock %d, pmt pid %d, sid %04X, op %s",
        i, ek, sock, pmt_pid, sid, msg);

    mutex_destroy(&k->mutex);

    return 0;
}

int dvbapi_add_pmt(adapter *ad, SPMT *pmt) {
    SKey *k = NULL;
    int key, pid = pmt->pid;

    if (ad->type == ADAPTER_CI) {
        LOG_AND_RETURN(TABLES_RESULT_ERROR_NORETRY,
                       "%s: Disabling dvbapi on DDCI adapter %d", __FUNCTION__,
                       ad->id);
    }

    key = keys_add(-1, ad->id, pmt->id);
    k = get_key(key);
    if (!k)
        LOG_AND_RETURN(1, "Could not add key for pmt %d", pmt->id);
    mutex_lock(&k->mutex);
    pmt->opaque = k;
    k->sid = pmt->sid;
    k->adapter = ad->id;
    k->pmt_pid = pid;
    k->tsid = ad->transponder_id;
    k->onid = 0;
    k->last_dmx_stop = getTick();
    dvbapi_send_pmt(k, CMD_ID_OK_DESCRAMBLING);

    mutex_unlock(&k->mutex);
    return 0;
}

int dvbapi_del_pmt(adapter *ad, SPMT *pmt) {
    SKey *k = (SKey *)pmt->opaque;
    keys_del(k->id);
    LOG("%s: deleted key %d, PMT pid %d, sid %d (%X), PMT %d", __FUNCTION__,
        k->id, pmt->pid, pmt->sid, pmt->sid, pmt->id);
    return 0;
}

int dvbapi_init_dev(adapter *ad) {
    set_sockets_rtime(poller_sock, 0);
    return TABLES_RESULT_OK;
}

SCA_op dvbapi;

void register_dvbapi() {
    memset(&dvbapi, 0, sizeof(dvbapi));
    dvbapi.ca_init_dev = dvbapi_init_dev;
    dvbapi.ca_add_pmt = dvbapi_add_pmt;
    dvbapi.ca_del_pmt = dvbapi_del_pmt;

    dvbapi_ca = add_ca(&dvbapi, 0xFFFFFFFF);
}

void unregister_dvbapi() {
    LOG("unregistering dvbapi as the socket is closed");
    del_ca(&dvbapi);
    dvbapi_ca = -1;
}

void dvbapi_delete_keys_for_adapter(int aid) {
    int i;
    SKey *k;
    for (i = 0; i < MAX_KEYS; i++)
        if ((k = get_key(i)) && k->adapter == aid)
            keys_del(i);
}

char *get_channel_for_key(int key, char *dest, int max_size) {
    SKey *k = get_key(key);
    SPMT *pmt = NULL;
    dest[0] = 0;
    dest[max_size - 1] = 0;
    if (k)
        pmt = get_pmt(k->pmt_id);
    if (pmt)
        _strncpy(dest, pmt->name, max_size - 1);

    return dest;
}

void free_all_keys(void) {
    int i;
    for (i = 0; i < MAX_KEYS; i++) {
        if (keys[i]) {
            mutex_destroy(&keys[i]->mutex);
            free(keys[i]);
        }
    }
    mutex_destroy(&keys_mutex);
}

_symbols dvbapi_sym[] = {
    {"key_enabled", VAR_AARRAY_INT8, keys, 1, MAX_KEYS,
     offsetof(SKey, enabled)},
    {"key_hops", VAR_AARRAY_INT8, keys, 1, MAX_KEYS, offsetof(SKey, hops)},
    {"key_ecmtime", VAR_AARRAY_INT, keys, 1, MAX_KEYS, offsetof(SKey, ecmtime)},
    {"key_pmt", VAR_AARRAY_INT, keys, 1, MAX_KEYS, offsetof(SKey, pmt_pid)},
    {"key_adapter", VAR_AARRAY_INT, keys, 1, MAX_KEYS, offsetof(SKey, adapter)},
    {"key_cardsystem", VAR_AARRAY_STRING, keys, 1, MAX_KEYS,
     offsetof(SKey, cardsystem)},
    {"key_reader", VAR_AARRAY_STRING, keys, 1, MAX_KEYS,
     offsetof(SKey, reader)},
    {"key_from", VAR_AARRAY_STRING, keys, 1, MAX_KEYS, offsetof(SKey, from)},
    {"key_protocol", VAR_AARRAY_STRING, keys, 1, MAX_KEYS,
     offsetof(SKey, protocol)},
    {"key_channel", VAR_FUNCTION_STRING, (void *)&get_channel_for_key, 0,
     MAX_KEYS, 0},

    {NULL, 0, NULL, 0, 0, 0}};
