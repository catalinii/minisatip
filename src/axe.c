/*
 * Copyright (C) 2014-2020 Jaroslav Kysela
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
#include "api/symbols.h"
#include "api/variables.h"
#include "dvb.h"
#include "minisatip.h"
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
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "axe.h"

#define DEFAULT_LOG LOG_AXE

#ifndef DISABLE_LINUXDVB

// satip-axe support DVB-S2X: https://github.com/perexg/satip-axe/issues/126
#ifndef DTV_SCRAMBLING_SEQUENCE_INDEX
#define DTV_SCRAMBLING_SEQUENCE_INDEX 70
#endif
#ifndef DTV_STREAM_ID
#define DTV_STREAM_ID 42
#endif

void get_signal_old(adapter *ad, int *status, uint32_t *ber, uint16_t *strength,
                uint16_t *snr, uint16_t *db);
int send_jess(adapter *ad, int fd, int freq, int pos, int pol, int hiband,
              diseqc *d);
int send_unicable(adapter *ad, int fd, int freq, int pos, int pol, int hiband,
                  diseqc *d);
int send_diseqc(adapter *ad, int fd, int pos, int pos_change, int pol,
                int hiband, diseqc *d);

int axe_fp_fd = -1;

static inline void axe_fp_fd_open(void) {
    if (axe_fp_fd < 0)
        axe_fp_fd = open("/dev/axe/fp-0", O_WRONLY);
}

static inline void axe_fp_fd_write(const char *s) {
    const char *b;
    size_t len;
    ssize_t r;

    axe_fp_fd_open();
    if (axe_fp_fd < 0)
        return;
    len = strlen(b = s);
    while (len > 0) {
        r = write(axe_fp_fd, b, len);
        if (r > 0) {
            len -= r;
            b += r;
        }
    }
}

void axe_set_tuner_led(int tuner, int on) {
    static int state = 0;
    char buf[16];
    if (((state >> tuner) & 1) != !!on) {
        sprintf(buf, "T%d_LED %d\n", tuner, on ? 1 : 0);
        axe_fp_fd_write(buf);
        if (on)
            state |= 1 << tuner;
        else
            state &= ~(1 << tuner);
    }
}

void axe_set_network_led(int on) {
    static int state = -1;
    if (state != on) {
        axe_fp_fd_write(on ? "NET_LED 1\n" : "NET_LED 0\n");
        state = on;
    }
}

int axe_read(int socket, void *buf, int len, sockets *ss, int *rv) {
    *rv = read(socket, buf, len);
    //	if(*rv < 0 || *rv == 0 || errno == -EAGAIN)
    if (*rv < 0 || *rv == 0 || errno == -EAGAIN) {
        *rv = 0;
        return 1;
    }
    return (*rv > 0);
}

int axe_open_device(adapter *ad) {
    char buf[100];
    LOG("trying to open [%d] adapter %d and frontend %d", ad->id, ad->pa,
        ad->fn);
    sprintf(buf, "/dev/axe/frontend-%d", ad->pa);
    if (ad->fe2 > 0)
        ad->fe = ad->fe2;
    else
        ad->fe = ad->fe2 = open(buf, O_RDWR | O_NONBLOCK);
    sprintf(buf, "/dev/axe/demuxts-%d", ad->pa);
    ad->dvr = open(buf, O_RDONLY | O_NONBLOCK);
    if (ad->fe < 0 || ad->dvr < 0) {
        sprintf(buf, "/dev/axe/frontend-%d", ad->pa);
        LOG("Could not open %s in RW mode (fe: %d, dvr: %d)", buf, ad->fe,
            ad->dvr);
        if (ad->fe >= 0)
            close(ad->fe);
        if (ad->dvr >= 0)
            close(ad->dvr);
        ad->fe = ad->fe2 = ad->dvr = -1;
        return 1;
    }
    ad->type = ADAPTER_DVB;
    ad->dmx = -1;
    LOG("opened DVB adapter %d fe:%d dvr:%d", ad->id, ad->fe, ad->dvr);
    return 0;
}

int axe_post_init(adapter *ad) {
    sockets_setread(ad->sock, axe_read);
    return 0;
}

static void axe_stv0900_i2c_4(const char *name, int pa, int v) {
    char buf[64];
    const char *b;
    int fd;
    size_t len;
    ssize_t r;

    snprintf(buf, sizeof(buf),
             "/sys/devices/platform/i2c-stm.0/i2c-0/stv0900_%s%d", name,
             pa + 1);
    fd = open(buf, O_WRONLY);
    if (fd < 0)
        return;
    snprintf(buf, sizeof(buf), "%d", v);
    len = strlen(b = buf);
    while (len > 0) {
        r = write(fd, b, len);
        if (r > 0) {
            len -= r;
            b += r;
        }
    }
    close(fd);
}

static void axe_pls_isi(adapter *ad, transponder *tp) {
    static int isi[4] = {-2, -2, -2, -2};
    static int pls_code[4] = {-2, -2, -2, -2};
    int v;
    LOGM("axe: isi %d pls %d mode %d", tp->plp_isi, tp->pls_code, tp->pls_mode);
    if (tp->plp_isi != isi[ad->pa]) {
        v = tp->plp_isi < 0 ? -1 : (tp->plp_isi & 0xff);
        axe_stv0900_i2c_4("mis", ad->pa, v);
        isi[ad->pa] = tp->plp_isi;
    }
    if (tp->pls_code != pls_code[ad->pa]) {
        v = tp->pls_code < 0 ? 0 : (tp->pls_code & 0x3ffff);
        if (tp->pls_mode == PLS_MODE_GOLD || tp->pls_mode < 0)
            v |= 0x40000;
        else if (tp->pls_mode == PLS_MODE_COMBO)
            v |= 0x80000; /* really? */
        axe_stv0900_i2c_4("pls", ad->pa, v);
        pls_code[ad->pa] = tp->pls_code;
    }
}

int axe_wakeup(adapter *_ad, int fe_fd, int voltage) {
    int i, mask;
    adapter *ad;
    if (opts.axe_power < 2)
        return 0;
    for (i = 0; i < 4; i++) {
        ad = get_adapter(i);
        if (ad == NULL || is_adapter_disabled(i))
            continue;
        if (ad->old_pol >= 0)
            return 0;
    }
    LOG("AXE wakeup");
    for (i = mask = 0; i < 4; i++) {
        /* lowband enabled */
        if (opts.quattro && opts.quattro_hiband == 1 && i < 2) {
            mask = 3;
            continue;
        }
        /* hiband enabled */
        if (opts.quattro && opts.quattro_hiband == 2 && i >= 2) {
            mask = 3 << 2;
            continue;
        }
        mask |= 1 << i;
    }
    for (i = 0; i < 4 && mask; i++) {
        if (((1 << i) & mask) == 0)
            continue;
        ad = get_adapter(i);
        if (ad == NULL || is_adapter_disabled(i))
            continue;
        LOG("axe_wakeup: waking adapter %d, fd %d", i, ad->fe);
        if (ioctl(ad->fe, FE_SET_VOLTAGE, voltage) == -1)
            LOG("axe_wakeup: FE_SET_VOLTAGE failed fd %d: %s", ad->fe,
                strerror(errno));
    }
    return 0;
}

static inline int extra_quattro(int input, int diseqc, int *equattro) {
    if (diseqc <= 0)
        *equattro = 0;
    /* lowband allowed - control the hiband inputs independently for positions
     * src=2+ */
    else if (opts.quattro && opts.quattro_hiband == 1 && input < 2)
        *equattro = diseqc;
    /* hiband allowed - control the lowband inputs independently for positions
     * src=2+ */
    else if (opts.quattro && opts.quattro_hiband == 2 && input >= 2 &&
             input < 4)
        *equattro = diseqc;
    else
        *equattro = 0;
    return *equattro;
}

adapter *axe_use_adapter(int input) {
    int input2 = input < 4 ? input : -1;
    adapter *ad = get_configured_adapter(input2);
    char buf[32];
    if (ad) {
        if (ad->fe2 <= 0) {
            sprintf(buf, "/dev/axe/frontend-%d", input);
            ad->fe2 = open(buf, O_RDONLY | O_NONBLOCK);
            LOG("adapter %d force open, fe2: %d", input, ad->fe2);
            if (ad->fe2 < 0)
                ad = NULL;
        }
    }
    return ad;
}

int axe_tune_check(adapter *ad, transponder *tp, diseqc *diseqc_param,
                   int diseqc) {
    int pol = (tp->pol - 1) & 1;
    int hiband = get_lnb_hiband(tp, diseqc_param);
    LOGM(
        "axe: tune check for adapter %d, pol %d/%d, hiband %d/%d, diseqc %d/%d",
        ad->id, ad->old_pol, pol, ad->old_hiband, hiband, ad->old_diseqc,
        diseqc);
    if (ad->old_pol != pol)
        return 0;
    if (ad->old_hiband != hiband)
        return 0;
    if (ad->old_diseqc != diseqc)
        return 0;
    return 1;
}

int axe_setup_switch(adapter *ad) {
    int frontend_fd = ad->fe;
    transponder *tp = &ad->tp;
    diseqc *diseqc_param = &tp->diseqc_param;

    int hiband;
    int freq;
    int diseqc = (tp->diseqc > 0) ? tp->diseqc - 1 : 0;
    int pol = (tp->pol - 1) & 1;

    adapter *ad2, *adm;
    int input = 0, aid, pos = 0, equattro = 0, master = -1;

    /* this is a new tune, so clear all adapter<->input mappings */
    for (aid = 0; aid < 4; aid++) {
        ad2 = a[aid];
        ad2->axe_used &= ~(1 << ad->id);
    }

    if (diseqc_param->switch_type != SWITCH_UNICABLE &&
        diseqc_param->switch_type != SWITCH_JESS) {
        input = ad->id;
        if (!opts.quattro || extra_quattro(input, diseqc, &equattro)) {
            if (equattro > 0)
                diseqc = equattro - 1;
            if (absolute_switch && diseqc >= 0 && diseqc < MAX_SOURCES) {
                /* reuse input */
                for (aid = 0; aid < 4; aid++) {
                    pos = get_absolute_source_for_adapter(aid, tp->diseqc,
                                                          SYS_DVBS);
                    if (pos <= 0)
                        continue;
                    pos--;
                    ad2 = get_configured_adapter(aid);
                    if (!ad2)
                        continue;
                    if (ad2->fe2 <= 0)
                        continue;
                    if ((ad2->axe_used & ~(1 << ad->id)) == 0)
                        continue;
                    if (!axe_tune_check(ad2, tp, &ad2->diseqc_param, pos))
                        continue;
                    break;
                }
                /* find _free input */
                if (aid >= 4) {
                    for (aid = 0; aid < 4; aid++) {
                        pos = get_absolute_source_for_adapter(aid, tp->diseqc,
                                                              SYS_DVBS);
                        if (pos <= 0)
                            continue;
                        pos--;
                        ad2 = get_configured_adapter(aid);
                        if (!ad2)
                            continue;
                        LOGM("axe: checking %d used 0x%x in %d", ad->id,
                             ad2->axe_used, ad2->id);
                        if (ad2->axe_used & ~(1 << ad->id))
                            continue;
                        break;
                    }
                }
                if (aid >= 4) {
                    LOG("unable to find input for diseqc %d (absolute switch), "
                        "adapter "
                        "%d",
                        diseqc, input);
                    return 0;
                }
                diseqc = pos;
                master = aid;
                adm = axe_use_adapter(master);
                if (adm == NULL) {
                    LOG("axe_fe: unknown master adapter for input %d", input);
                    return 0;
                }
            } else {
                master = (ad->master_source >= 0) ? ad->master_source : ad->pa;
                adm = axe_use_adapter(master);
                if (adm == NULL) {
                    LOG("axe_fe: unknown master adapter for input %d", input);
                    return 0;
                }
                if (adm->old_pol >= 0) {
                    for (aid = 0; aid < 4; aid++) {
                        ad2 = get_configured_adapter(aid);
                        if (!ad2 || ad2->fe2 <= 0 || ad == ad2)
                            continue;
                        if ((ad2->master_source >= 0) &&
                            ad2->master_source != adm->pa) // adm->id ?
                            continue;
                        if ((ad2->master_source >= 0) && ad2 != adm)
                            continue;
                        if (ad2->sid_cnt > 0)
                            break;
                    }
                    if (adm != ad && aid < 4 &&
                        !axe_tune_check(adm, tp, &adm->diseqc_param, diseqc)) {
                        LOG("unable to use slave adapter %d (master %d)", input,
                            adm->pa);
                        return 0;
                    }
                }
            }
            adm->axe_used |= (1 << ad->id);
            if (master >= 0) {
                input = master;
                diseqc_param = &adm->diseqc_param;
                hiband = get_lnb_hiband(tp, diseqc_param);
                freq = get_lnb_int_freq(tp, diseqc_param);
                if (!axe_tune_check(adm, tp, diseqc_param, diseqc)) {
                    send_diseqc(adm, adm->fe2, diseqc,
                                adm->old_diseqc != diseqc, pol, hiband,
                                diseqc_param);
                    adm->old_pol = pol;
                    adm->old_hiband = hiband;
                    adm->old_diseqc = diseqc;
                }
                goto axe;
            }
        } else if (opts.quattro) {
            hiband = get_lnb_hiband(tp, diseqc_param);
            if (opts.quattro_hiband == 1 && hiband) {
                LOG("axe_fe: hiband is not allowed for quattro config (adapter "
                    "%d)",
                    input);
                return 0;
            }
            if (opts.quattro_hiband == 2 && !hiband) {
                LOG("axe_fe: lowband is not allowed for quattro config "
                    "(adapter %d)",
                    input);
                return 0;
            }
            input = ((hiband ^ 1) << 1) | (pol ^ 1);
            adm = axe_use_adapter(input);
            if (adm == NULL) {
                LOG("axe_fe: unknown master adapter %d", input);
                return 0;
            }
            adm->old_diseqc = diseqc = 0;
            diseqc_param = &adm->diseqc_param;
            hiband = get_lnb_hiband(tp, diseqc_param);
            freq = get_lnb_int_freq(tp, diseqc_param);
            if (!axe_tune_check(adm, tp, diseqc_param, 0)) {
                send_diseqc(adm, adm->fe2, 0, 0, pol, hiband, diseqc_param);
                adm->old_pol = pol;
                adm->old_hiband = hiband;
                adm->old_diseqc = 0;
            }
            adm->axe_used |= (1 << ad->id);
            goto axe;
        }
    } else {
        aid = ad->id & 3;
        input = ad->master_source < 0 ? 0 : ad->master_source;
        frontend_fd = ad->fe;
        ad = axe_use_adapter(input);
        if (ad == NULL) {
            LOGM("axe setup: unable to find adapter %d", input);
            return 0;
        } else
            ad->axe_used |= (1 << aid);

        LOG("adapter %d: using source %d, fe %d fe2 %d", ad->id, input, ad->fe,
            ad->fe2);
    }

    hiband = get_lnb_hiband(tp, diseqc_param);
    freq = get_lnb_int_freq(tp, diseqc_param);

    if (diseqc_param->switch_type == SWITCH_UNICABLE) {
        freq = send_unicable(ad, ad->fe2, freq / 1000, diseqc, pol, hiband,
                             diseqc_param);
    } else if (diseqc_param->switch_type == SWITCH_JESS) {
        freq = send_jess(ad, ad->fe2, freq / 1000, diseqc, pol, hiband,
                         diseqc_param);
    } else if (diseqc_param->switch_type == SWITCH_SLAVE) {
        LOG("FD %d (%d) is a slave adapter", frontend_fd);
    } else {
        if (ad->old_pol != pol || ad->old_hiband != hiband ||
            ad->old_diseqc != diseqc)
            send_diseqc(ad, frontend_fd, diseqc, ad->old_diseqc != diseqc, pol,
                        hiband, diseqc_param);
        else
            LOGM("Skip sending diseqc commands since "
                 "the switch position doesn't need to be changed: "
                 "pol %d, hiband %d, switch position %d",
                 pol, hiband, diseqc);
    }

    ad->old_pol = pol;
    ad->old_hiband = hiband;
    ad->old_diseqc = diseqc;

axe:
    for (aid = 0; aid < 4; aid++) {
        ad2 = get_configured_adapter(aid);
        if (ad2)
            LOGM("axe_fe: used[%d] = 0x%x, pol=%d, hiband=%d, diseqc=%d", aid,
                 ad2->axe_used, ad2->old_pol, ad2->old_hiband, ad2->old_diseqc);
    }
    LOGM("axe_fe: reset for fd %d adapter %d input %d diseqc %d", frontend_fd,
         ad ? ad->pa : -1, input, diseqc);
    if (axe_fe_reset(frontend_fd) < 0)
        LOG("axe_fe: RESET failed for fd %d: %s", frontend_fd, strerror(errno));
    if (axe_fe_input(frontend_fd, input))
        LOG("axe_fe: INPUT failed for fd %d input %d: %s", frontend_fd, input,
            strerror(errno));

    return freq;
}

#define ADD_PROP(c, d)                                                         \
    {                                                                          \
        p_cmd[iProp].cmd = (c);                                                \
        p_cmd[iProp].u.data = (d);                                             \
        iProp++;                                                               \
    }

int axe_tune(int aid, transponder *tp) {
    adapter *ad = get_adapter(aid);
    ssize_t drv;
    char buf[1316];
    char plp_desc[1024];

    int64_t bclear, bpol;
    int iProp = 0;
    int fd_frontend;

    int freq = tp->freq;
    struct dtv_property p_cmd[20];
    struct dtv_properties p = {.num = 0, .props = p_cmd};
    struct dvb_frontend_event ev;

    struct dtv_property p_clear[] = {
        {.cmd = DTV_CLEAR},
    };

    struct dtv_properties cmdseq_clear = {.num = 1, .props = p_clear};

    if (!ad)
        return -404;

    fd_frontend = ad->fe;

    axe_set_tuner_led(aid + 1, 1);
    axe_dmxts_stop(ad->dvr);
    axe_fe_reset(ad->fe);

    // probably can be removed
    do {
        drv = read(ad->dvr, buf, sizeof(buf));
    } while (drv > 0);

    memset(p_cmd, 0, sizeof(p_cmd));
    bclear = getTick();

    if ((ioctl(fd_frontend, FE_SET_PROPERTY, &cmdseq_clear)) == -1) {
        LOG("FE_SET_PROPERTY DTV_CLEAR failed for fd %d: %s", fd_frontend,
            strerror(errno));
        //        return -1;
    }

    switch (tp->sys) {
    case SYS_DVBS:
    case SYS_DVBS2:
        plp_desc[0] = 0;
        int len = 0;

        bpol = getTick();
        freq = axe_setup_switch(ad);
        if (freq < MIN_FRQ_DVBS || freq > MAX_FRQ_DVBS)
            LOG_AND_RETURN(-404, "Frequency %d is not within range ", freq)

        ADD_PROP(DTV_SYMBOL_RATE, tp->sr)
        ADD_PROP(DTV_INNER_FEC, tp->fec)
        if (tp->plp_isi >= 0) {
            ADD_PROP(DTV_STREAM_ID, tp->plp_isi & 0xFF)
            strcatf(plp_desc, len, "stream_id = %d", tp->plp_isi & 0xFF);
        }
        ADD_PROP(DTV_SCRAMBLING_SEQUENCE_INDEX, pls_scrambling_index(tp))
        strcatf(plp_desc, len, "sequence_index = %d", pls_scrambling_index(tp));

        LOG("tuning to %d(%d) pol: %s (%d) sr:%d fec:%s delsys:%s mod:%s "
            "rolloff:%s pilot:%s, ts clear=%jd, ts pol=%jd %s",
            tp->freq, freq, get_pol(tp->pol), tp->pol, tp->sr, fe_fec[tp->fec],
            fe_delsys[tp->sys], fe_modulation[tp->mtype], "auto", "auto",
            bclear, bpol, plp_desc)
        break;

    case SYS_DVBT:
    case SYS_DVBT2:

        if (tp->freq < MIN_FRQ_DVBT || tp->freq > MAX_FRQ_DVBT)
            LOG_AND_RETURN(-404, "Frequency %d is not within range ", tp->freq)

        freq = freq * 1000;
        ADD_PROP(DTV_BANDWIDTH_HZ, tp->bw)
        ADD_PROP(DTV_CODE_RATE_HP, tp->fec)
        ADD_PROP(DTV_CODE_RATE_LP, tp->fec)
        ADD_PROP(DTV_GUARD_INTERVAL, tp->gi)
        ADD_PROP(DTV_TRANSMISSION_MODE, tp->tmode)
        ADD_PROP(DTV_HIERARCHY, HIERARCHY_AUTO)
#if DVBAPIVERSION >= 0x0502
        if (tp->plp_isi >= 0)
            ADD_PROP(DTV_STREAM_ID, tp->plp_isi & 0xFF)
#endif

        LOG("tuning to %d delsys: %s bw:%d inversion:%s mod:%s fec:%s guard:%s "
            "transmission: %s, ts clear = %jd",
            freq, fe_delsys[tp->sys], tp->bw, fe_inversion[tp->inversion],
            fe_modulation[tp->mtype], fe_fec[tp->fec], fe_gi[tp->gi],
            fe_tmode[tp->tmode], bclear)
        break;

    case SYS_DVBC2:
    case SYS_DVBC_ANNEX_A:

        if (tp->freq < MIN_FRQ_DVBC || tp->freq > MAX_FRQ_DVBC)
            LOG_AND_RETURN(-404, "Frequency %d is not within range ", tp->freq)

        freq = freq * 1000;
        ADD_PROP(DTV_SYMBOL_RATE, tp->sr)
#if DVBAPIVERSION >= 0x0502
        if (tp->plp_isi >= 0) {
            int v = tp->plp_isi & 0xFF;
            if (tp->ds >= 0)
                v |= (tp->ds & 0xFF) << 8;
            ADD_PROP(DTV_STREAM_ID, v);
        }
#endif
        // valid for DD DVB-C2 devices

        LOG("tuning to %d sr:%d specinv:%s delsys:%s mod:%s ts clear = %jd",
            freq, tp->sr, fe_inversion[tp->inversion], fe_delsys[tp->sys],
            fe_modulation[tp->mtype], bclear)
        break;

    default:
        LOG("tuning to unknown delsys: %s freq %s ts clear = %jd",
            fe_delsys[tp->sys], freq, bclear)
        break;
    }

    ADD_PROP(DTV_FREQUENCY, freq)
    ADD_PROP(DTV_INVERSION, tp->inversion)
    ADD_PROP(DTV_MODULATION, tp->mtype);
    ADD_PROP(DTV_DELIVERY_SYSTEM, tp->sys);
    ADD_PROP(DTV_TUNE, 0)

    p.num = iProp;
    /* discard stale QPSK events */
    while (1) {
        if (ioctl(fd_frontend, FE_GET_EVENT, &ev) == -1)
            break;
    }

    axe_pls_isi(ad, tp);

    if ((ioctl(fd_frontend, FE_SET_PROPERTY, &p)) == -1)
        if (ioctl(fd_frontend, FE_SET_PROPERTY, &p) == -1) {
            LOG("dvb_tune: set property failed %d %s", errno, strerror(errno));
            axe_set_tuner_led(aid + 1, 0);
            return -404;
        }

    axe_dmxts_start(ad->dvr);
    return 0;
}

int axe_set_pid(adapter *ad, int i_pid) {
    if (i_pid > 8192 || ad == NULL)
        LOG_AND_RETURN(-1, "pid %d > 8192 for ADAPTER %d", i_pid,
                       ad ? ad->id : -1);
    if (axe_dmxts_add_pid(ad->dvr, i_pid) < 0) {
        LOG("failed setting filter on PID %d for ADAPTER %d (%s)", i_pid, ad->id,
            strerror(errno));
        return -1;
    }
    LOG("setting filter on PID %d for ADAPTER %d", i_pid, ad->id);
    return ((ad->id + 1) << 16) | i_pid;
}

int axe_del_filters(adapter *ad, int fd, int pid) {
    if (a == NULL)
        return 0; /* closed */
    if ((fd & 0xffff) != pid)
        LOG_AND_RETURN(0, "AXE PID remove on an invalid handle %d, pid %d", fd,
                       pid);
    if (axe_dmxts_remove_pid(ad->dvr, pid) < 0)
        LOG("AXE PID remove failed on PID %d ADAPTER %d: %s", pid, ad->pa,
            strerror(errno))
    else
        LOG("clearing filters on PID %d ADAPTER %d", pid, ad->pa);
    return 0;
}

fe_delivery_system_t axe_delsys(int aid, int fd, fe_delivery_system_t *sys) {
    int i;
    LOG("Delivery System DVB-S/DVB-S2 (AXE)");
    for (i = 0; i < 10; i++)
        sys[i] = 0;
    sys[0] = SYS_DVBS;
    sys[1] = SYS_DVBS2;
    return SYS_DVBS2;
}

int axe_get_signal(adapter *ad) {
    uint16_t strength = 0, snr = 0, db = 0, tmp;
    int status = 0;
    uint32_t ber = 0;
    get_signal_old(ad, &status, &ber, &strength, &snr, &db);

    strength = strength * 240 / 24000;
    if (strength > 240)
        strength = 240;
    tmp = snr * 255 / 54000;
    if (tmp > 255)
        tmp = 255;
    if (tmp <= 15)
        tmp = 0;
    snr = tmp;

    // Lock the adapter while doing changes
    adapter_lock(ad->id);
    ad->snr = snr;
    ad->db = db;
    ad->strength = strength;
    ad->status = status;
    ad->ber = ber;

    if (ad->status == 0 &&
        ((ad->tp.diseqc_param.switch_type == SWITCH_JESS) ||
         (ad->tp.diseqc_param.switch_type == SWITCH_UNICABLE))) {
        axe_setup_switch(ad);
    }

    adapter_unlock(ad->id);
    return 0;
}

int axe_commit(adapter *a) { return 0; }

int axe_close(adapter *a2) {
    adapter *c;
    int aid, busy;
    if (a2->fe <= 0)
        return 0;
    a2->fe = -1;
    if (a2->fe2 > 0) {
        axe_dmxts_stop(a2->fe2);
        axe_fe_reset(a2->fe2);
    }
    for (aid = busy = 0; aid < 4; aid++) {
        c = a[aid];
        c->axe_used &= ~(1 << a2->id);
        if (c->axe_used || c->fe > 0)
            busy++;
    }
    if (busy > 0 && opts.axe_power > 1)
        goto nostandby;
    for (aid = 0; aid < 4; aid++) {
        c = a[aid];
        if (opts.axe_power < 2 && c != a2 && busy && c->sock >= 0)
            continue;
        if (c->axe_used != 0 || c->sid_cnt > 0) {
            LOG("AXE standby: adapter %d busy (cnt=%d/used=%04x/fe=%d), "
                "keeping",
                aid, c->sid_cnt, c->axe_used, c->fe);
            continue;
        }
        if (c->fe2 < 0)
            continue;
        LOG("AXE standby: adapter %d", aid);
        axe_fe_standby(c->fe2, -1);
        axe_set_tuner_led(aid + 1, 0);
        if (ioctl(c->fe2, FE_SET_VOLTAGE, SEC_VOLTAGE_OFF) < 0)
            LOG("ioctl FE_SET_VOLTAGE for fd %d failed: %d : %s ", c->fe2,
                errno, strerror(errno));

        close(c->fe2);
        c->fe2 = -1;
        c->old_diseqc = c->old_pol = c->old_hiband = -1;
    }
nostandby:
    axe_set_tuner_led(a2->id + 1, 0);
    return 0;
}

void find_axe_adapter(adapter **a) {
    int na = 0;
    char buf[100];
    int fd;
    int i = 0, j = 0;
    adapter *ad;
    axe_set_network_led(0);
    for (i = 0; i < MAX_ADAPTERS; i++)
        for (j = 0; j < MAX_ADAPTERS; j++) {
            if (i < 4 && j == 0) {
                axe_set_tuner_led(i + 1, 0);
                sprintf(buf, "/dev/axe/frontend-%d", i);
                fd = open(buf, O_RDONLY | O_NONBLOCK);
            } else {
                continue;
            }
            LOG("testing device %s -> fd: %d", buf, fd);
            if (fd >= 0) {
                if (!a[na])
                    a[na] = adapter_alloc();

                ad = a[na];
                ad->pa = i;
                ad->fn = j;

                ad->open = axe_open_device;
                ad->set_pid = axe_set_pid;
                ad->del_filters = axe_del_filters;
                ad->commit = axe_commit;
                ad->tune = axe_tune;
                ad->delsys = axe_delsys;
                ad->post_init = axe_post_init;
                ad->close = axe_close;
                ad->get_signal = axe_get_signal;
                ad->wakeup = axe_wakeup;
                ad->type = ADAPTER_DVB;
                ad->fast_status = 1;
                ad->standby = free_axe_input;
                close(fd);
                na++;
                a_count = na; // update adapter counter
                if (na == MAX_ADAPTERS)
                    return;
            } else {
                if (i < 4) {
                    LOG("AXE - cannot open %s: %i", buf, errno);
                    sleep(60);
                }
            }
        }
    for (; na < MAX_ADAPTERS; na++)
        if (a[na])
            a[na]->pa = a[na]->fn = -1;
}

int free_axe_input(adapter *ad) {
    int aid;
    adapter *ad2;

    for (aid = 0; aid < 4; aid++) {
        ad2 = get_configured_adapter(aid);
        if (ad2) {
            ad2->axe_used &= ~(1 << ad->id);
            LOGM("axe: _free input %d : %04x", ad2->id, ad2->axe_used);
        }
    }
    return 0;
}

static char *axe_vdevice_read(int aid, char *buf, size_t buflen) {
    size_t len;
    int i, fd;
    if (buflen < 1)
        return NULL;
    buf[buflen - 1] = 0;
    for (i = 0; i < 10; i++) {
        snprintf(buf, buflen, "/proc/STAPI/stpti/PTI%d/vDeviceInfo", aid ^ 1);
        fd = open(buf, O_RDONLY);
        if (fd < 0) {
            LOG("Could not open %s, error %d: %s", buf, errno, strerror(errno));
            continue;
        }
        len = read(fd, buf, buflen - 1);
        close(fd);
        if (len > 200) {
            buf[len] = '\0';
            return buf;
        }
    }
    return NULL;
}

adapter *axe_vdevice_sync(int aid) {
    adapter *ad = get_adapter_nw(aid);
    char buf[1024], *p;
    int64_t t;
    uint32_t addr, pktc, syncerrc, tperrc, ccerr;

    if (!ad)
        return NULL;
    t = getTickUs();
    if (ad->axe_vdevice_last_sync + 1000000 >= t)
        return ad;
    ad->axe_vdevice_last_sync = t;
    p = axe_vdevice_read(aid, buf, sizeof(buf));
    if (p)
        p = strchr(p, '\n');
    if (p) {
        if (sscanf(p + 1, "#%08x:  %08x %08x %08x %08x", &addr, &pktc,
                   &syncerrc, &tperrc, &ccerr) == 5) {
            ad->axe_pktc = pktc;
            ad->axe_ccerr = ccerr;
        }
    }
    return ad;
}

int64_t get_axe_pktc(int aid) {
    adapter *ad = axe_vdevice_sync(aid);
    return ad ? ad->axe_pktc : 0;
}

int64_t get_axe_ccerr(int aid) {
    adapter *ad = axe_vdevice_sync(aid);
    return ad ? ad->axe_ccerr : 0;
}

char *get_axe_coax(int aid, char *dest, int max_size) {
    int i, len;
    adapter *ad;
    dest[0] = 0;
    len = 0;
    if (aid < 0 || aid > 3)
        return dest;

    for (i = 0; i < 4; i++) {
        ad = get_configured_adapter(i);
        if (ad && ad->axe_used & (1 << aid))
            len += snprintf(dest + len, max_size - len, "LNB%d,", i + 1);
    }

    if (len > 0)
        dest[len - 1] = 0;

    return dest;
}

_symbols axe_sym[] = {{"ad_axe_pktc", VAR_FUNCTION_INT64, (void *)&get_axe_pktc,
                       0, MAX_ADAPTERS, 0},
                      {"ad_axe_ccerr", VAR_FUNCTION_INT64,
                       (void *)&get_axe_ccerr, 0, MAX_ADAPTERS, 0},
                      {"ad_axe_coax", VAR_FUNCTION_STRING,
                       (void *)&get_axe_coax, 0, MAX_ADAPTERS, 0},
                      {NULL, 0, NULL, 0, 0, 0}};

#endif // #ifndef DISABLE_LINUXDVB
