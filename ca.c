/*
 * Copyright (C) 2015 Damjan Marion <damjan.marion@gmail.com>
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

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>
#include <linux/dvb/ca.h>
#include <fcntl.h>
#include <ctype.h>

#include <libdvben50221/en50221_session.h>
#include <libdvben50221/en50221_app_utils.h>
#include <libdvben50221/en50221_app_ai.h>
#include <libdvben50221/en50221_app_rm.h>
#include <libdvben50221/en50221_app_ca.h>
#include <libdvben50221/en50221_app_dvb.h>
#include <libdvben50221/en50221_app_datetime.h>
#include <libdvben50221/en50221_app_smartcard.h>
#include <libdvben50221/en50221_app_teletext.h>
#include <libdvben50221/en50221_app_mmi.h>
#include <libdvben50221/en50221_app_epg.h>
#include <libdvben50221/en50221_app_auth.h>
#include <libdvben50221/en50221_app_lowspeed.h>
#include <libdvbapi/dvbca.h>

#include "dvb.h"
#include "socketworks.h"
#include "minisatip.h"
#include "dvbapi.h"
#include "adapter.h"
#include "search.h"
#include "ca.h"

int shutdown_stackthread = 0;

#define MAX_MSG_LEN 4096
#define READ_DELAY 0
#define MAX_CA_DEVICES 8
#define MAX_CA_DEVICE_RESOURCES 8

typedef struct {
    int missing_fragments;
    uint16_t len;
    uint16_t tid;
    uint8_t ver;
    uint8_t buffer[1024];
} pat_t;

typedef struct ca_device {
  int fd;
  int slot_id;
  int tc;
  struct en50221_transport_layer *tl;
  struct en50221_session_layer * sl;

  struct en50221_app_send_functions sf;
  struct en50221_app_rm *rm_resource;
  struct en50221_app_ai *ai_resource;
  struct en50221_app_ca *ca_resource;
  struct en50221_app_datetime *dt_resource;
  int ca_session_number;

  uint16_t ai_session_number;

  pat_t pat;
  uint16_t pmt_pids[MAX_PIDS];
  int num_pmt_pids;
  uint16_t last_pmt_pid;
  int last_pmt_sid;
  int last_pmt_ver;
  int last_pmt_len;
} ca_device_t;

static struct ca_device ca_devices[MAX_CA_DEVICES];
static int ca_devices_count = 0;


// this contains all known resource ids so we can see if the cam asks for something exotic
uint32_t resource_ids[] = { EN50221_APP_TELETEXT_RESOURCEID,
                            EN50221_APP_SMARTCARD_RESOURCEID(1),
                            EN50221_APP_RM_RESOURCEID,
                            EN50221_APP_MMI_RESOURCEID,
                            EN50221_APP_LOWSPEED_RESOURCEID(1,1),
                            EN50221_APP_EPG_RESOURCEID(1),
                            EN50221_APP_DVB_RESOURCEID,
                            EN50221_APP_CA_RESOURCEID,
                            EN50221_APP_DATETIME_RESOURCEID,
                            EN50221_APP_AUTH_RESOURCEID,
                            EN50221_APP_AI_RESOURCEID, };
int resource_ids_count = sizeof(resource_ids)/4;


void hexdump(uint8_t buffer[], int len)
{
#define HEXDUMP_LINE_LEN	16
	int i;
	char s[HEXDUMP_LINE_LEN+1];
	bzero(s, HEXDUMP_LINE_LEN+1);

	for(i=0; i < len; i++) {
		if (!(i%HEXDUMP_LINE_LEN)) {
			if (s[0])
				printf("[%s]",s);
			printf("\n%05x: ", i);
			bzero(s, HEXDUMP_LINE_LEN);
		}
		s[i%HEXDUMP_LINE_LEN]=isprint(buffer[i])?buffer[i]:'.';
		printf("%02x ", buffer[i]);
	}
	while(i++%HEXDUMP_LINE_LEN)
		printf("   ");

	printf("[%s]\n", s);
}

static void
pat_packet(adapter *a, uint8_t * b)
{
    ca_device_t * d = a->ca_device;
    pat_t * pat = &d->pat;
    int len, ver, tid;
    int pat_buffer_len;
    int p;

    if (!d)
        return;

    if (b[1] & 0x40) { /* Payload Unit Start Indicator */

        if (b[4])   /* pointer field should be 0 */
            return;

        if (b[5])   /* table_id should be 0 (PAT) */
            return;

        if (b[10] & 1 == 0) /* only active PAT */
            return;

        len = ((b[6] & 0x03) << 8) | b[7];
        ver = (b[10] >> 1) & 0x1f;
        tid = (b[8] << 8) | b[9];

        if (len > 1021) /* spec says it cannot be bigger */
            return;

        //printf("first fragment: len %u (%u) ver %u(%u) tid %u(%u)\n",
        //    len, pat->len, ver, pat->ver, tid, pat->tid);

        /* we already have it */
        if (len == pat->len && tid == pat->tid && ver == pat->ver)
            return;

        pat->tid = tid;
        pat->ver = ver;
        pat->len = len;
        pat->missing_fragments = (len + 3) / 184;
    printf("1 mf %d len %u\n", pat->missing_fragments, pat->len);

        memcpy(&pat->buffer[0], &b[5], 188-5);
        pat_buffer_len = 188 - 5;
    } else {
        if (pat->missing_fragments) {
            memcpy(&pat->buffer[pat_buffer_len], &b[4], 188 - 4);
            pat_buffer_len += 188 - 4;
            pat->missing_fragments--;
        } else
            return;
    }

    if (pat->missing_fragments || pat->len == 0)
        return;

    len = 3 + (((pat->buffer[1] & 0x03) << 8) | pat->buffer[2]);
    //hexdump( (char *) &pat->buffer[0], len);

    p = 8;
    d->num_pmt_pids = 0;
    while (p < (len - 4)) {
        int sid, pid;
        sid = (pat->buffer[p] << 8) | pat->buffer[p+1];
        pid = 0x1FFF & ((pat->buffer[p+2] << 8) | pat->buffer[p+3]);
        if (sid) {
            printf("sid %x (%u) pmt_pid %x (%u)\n",
                sid,sid,pid,pid);

            if (d->num_pmt_pids < MAX_PIDS)
                d->pmt_pids[d->num_pmt_pids++] = pid;
        }
        p += 4;
    }
}

static void
pmt_packet(adapter *a, uint8_t * b)
{
    ca_device_t * d = a->ca_device;
    int len, ver, sid;
    uint16_t pid;
    static int send_once = 1;

    pid = (b[1] & 0x1F) * 256 + b[2];

    if (!d)
        return;

    if (b[1] & 0x40) { /* Payload Unit Start Indicator */

        if (b[4])   /* pointer field should be 0 */
            return;

        if (b[5] != 2)   /* table_id should be 2 (PMT) */
            return;

        if (b[10] & 1 == 0) /* only active PAT */
            return;

        len = ((b[6] & 0x03) << 8) | b[7];
        ver = (b[10] >> 1) & 0x1f;
        sid = (b[8] << 8) | b[9];
        if (len == d->last_pmt_len &&
            ver == d->last_pmt_ver &&
            sid == d->last_pmt_sid &&
            pid == d->last_pmt_pid)
            return;

        d->last_pmt_pid = pid;
        d->last_pmt_sid = sid;
        d->last_pmt_ver = ver;
        d->last_pmt_len = len;


        if (send_once && pid == 560)
        {
            printf("PMT pid %u len %u ver %u pid %u\n", pid, len, ver, sid);
            hexdump(&b[5], len + 3 );
            send_once = 0;

            uint8_t capmt[4096];
            int size;
            struct section *section = section_codec((uint8_t*) &b[5], len + 3);
            if (!section){
                printf("failed to decode section\n");
                return;
            }

            struct section_ext *result = section_ext_decode(section, 0);
            if (!section){
                printf("failed to decode ext_section\n");
                return;
            }

            struct mpeg_pmt_section *pmt = mpeg_pmt_section_codec(result);
            if (!pmt){
                printf("failed to decode pmt\n");
                return;
            }

            if ((size = en50221_ca_format_pmt(&b[5], capmt, sizeof(capmt),
                                         CA_LIST_MANAGEMENT_ONLY, 0,
                                         CA_PMT_CMD_ID_OK_DESCRAMBLING)) < 0) {

                fprintf(stderr, "Failed to format CA PMT object\n");
            }
            if (en50221_app_ca_pmt(d->ca_resource, d->ca_session_number, capmt, size)) {
                fprintf(stderr, "Failed to send CA PMT object\n");
            }
            hexdump(&capmt[0], size);
        }
    }

}

void
ca_grab_pmt(adapter *a, int rlen)
{
	int i, j;
	uint8_t * b;
	uint16_t pid, last_pmt_pid;
	SPid *p;
	uint8_t pid_is_pmt[8192];
    ca_device_t * d = a->ca_device;
    pat_t * pat = &d->pat;

	if (!a->ca_device)
		return;

	for(i=0;i<rlen;i+=188)
	{
		b = a->buf + i;

		if (b[0] != 0x47)
			continue;

		if (b[1] & 0x80) /* eror bit set */
			continue;

		pid = (b[1] & 0x1F)*256 + b[2];

		if (pid == 0)
			pat_packet(a, b);

        else if (pid == d->last_pmt_pid)
        {
            //printf("PMT last PID hit %u\n", pid);
            pmt_packet(a, b);
        }

        else for (j=0; j < d->num_pmt_pids; j++) {
                if (d->pmt_pids[j]==pid) {
                   // printf("PMT hit %u j=%u\n", pid,j);
                    pmt_packet(a, b);
                }
        }
	}
}


int ca_ai_callback(void *arg, uint8_t slot_id, uint16_t session_number,
                     uint8_t application_type, uint16_t application_manufacturer,
                     uint16_t manufacturer_code, uint8_t menu_string_length,
                     uint8_t *menu_string)
{
    ca_device_t * d = arg;

    printf("%02x:%s\n", slot_id, __func__);
    printf("  Application type: %02x\n", application_type);
    printf("  Application manufacturer: %04x\n", application_manufacturer);
    printf("  Manufacturer code: %04x\n", manufacturer_code);
    printf("  Menu string: %.*s\n", menu_string_length, menu_string);

    d->ai_session_number = session_number;

    return 0;
}

void *
stackthread_func(void* arg) {

    ca_device_t * d = arg;
    int lasterror = 0;
    printf("%s: start\n", __func__);

    while(!shutdown_stackthread) {
        int error;
        if ((error = en50221_tl_poll(d->tl)) != 0) {
            if (error != lasterror) {
                fprintf(stderr, "Error reported by stack slot:%i error:%i\n",
                        en50221_tl_get_error_slot(d->tl),
                        en50221_tl_get_error(d->tl));
            }
            lasterror = error;
        }
    }

    shutdown_stackthread = 0;
    return 0;
}

static int
ca_session_callback(void *arg,
	                int reason,
	                uint8_t slot_id,
	                uint16_t session_number,
	                uint32_t resource_id)
{
    ca_device_t * d = arg;
	printf("%s: reason %d slot_id %u session_number %u resource_id %x\n",
		__func__, reason, slot_id, session_number, resource_id);

	switch(reason) {
		case S_SCALLBACK_REASON_CAMCONNECTING:
			printf("CAM connecting\n");
			break;
		case S_SCALLBACK_REASON_CAMCONNECTED:
			printf("CAM connected\n");
            if (resource_id == EN50221_APP_RM_RESOURCEID) {
                en50221_app_rm_enq(d->rm_resource, session_number);
            } else if (resource_id == EN50221_APP_AI_RESOURCEID) {
                en50221_app_ai_enquiry(d->ai_resource, session_number);
            } else if (resource_id == EN50221_APP_CA_RESOURCEID) {
                en50221_app_ca_info_enq(d->ca_resource, session_number);
                d->ca_session_number = session_number;
            }
			break;
	}
    return 0;
}

static int
ca_lookup_callback(void * arg,
	               uint8_t slot_id,
	               uint32_t requested_resource_id,
                   en50221_sl_resource_callback *callback_out,
                   void **arg_out,
                   uint32_t *connected_resource_id)
{
    ca_device_t * d = arg;

	printf("%s: slot_id %u requested_resource_id %x\n", __func__, slot_id, requested_resource_id);

    switch (requested_resource_id) {
    	case EN50221_APP_RM_RESOURCEID:
    		printf("BINGO RM\n");
    		*callback_out = (en50221_sl_resource_callback) en50221_app_rm_message;
            *arg_out = d->rm_resource;
    		*connected_resource_id = EN50221_APP_RM_RESOURCEID;
    		break;
    	case EN50221_APP_AI_RESOURCEID:
    		printf("BINGO AI\n");
    		*callback_out = (en50221_sl_resource_callback) en50221_app_ai_message;
            *arg_out = d->ai_resource;
    		*connected_resource_id = EN50221_APP_AI_RESOURCEID;
    		break;
    	case EN50221_APP_CA_RESOURCEID:
    		printf("BINGO CA\n");
    		*callback_out = (en50221_sl_resource_callback) en50221_app_ca_message;
            *arg_out = d->ca_resource;
    		*connected_resource_id = EN50221_APP_CA_RESOURCEID;
    		break;
    	case EN50221_APP_DATETIME_RESOURCEID:
    		printf("BINGO DATETIME\n");
    		*callback_out = (en50221_sl_resource_callback) en50221_app_datetime_message;
            *arg_out = d->dt_resource;
    		*connected_resource_id = EN50221_APP_DATETIME_RESOURCEID;
    		break;
    	default:
    		printf("unknown resource id");
    		return -1;
    }
    return 0;
}

static int
ca_rm_enq_callback(void *arg, uint8_t slot_id, uint16_t session_number)
{
    ca_device_t * d = arg;

    printf("%02x:%s\n", slot_id, __func__);

    if (en50221_app_rm_reply(d->rm_resource, session_number, resource_ids_count, resource_ids)) {
        printf("%02x:Failed to send reply to ENQ\n", slot_id);
    }

    return 0;
}

static int
ca_rm_reply_callback(void *arg, uint8_t slot_id, uint16_t session_number, uint32_t resource_id_count, uint32_t *_resource_ids)
{
    ca_device_t * d = arg;
    printf("%02x:%s\n", slot_id, __func__);

    uint32_t i;
    for(i=0; i< resource_id_count; i++) {
        printf("  CAM provided resource id: %08x\n", _resource_ids[i]);
    }

    if (en50221_app_rm_changed(d->rm_resource, session_number)) {
        printf("%02x:Failed to send REPLY\n", slot_id);
    }

    return 0;
}

static int
ca_rm_changed_callback(void *arg, uint8_t slot_id, uint16_t session_number)
{
    ca_device_t * d = arg;
    printf("%02x:%s\n", slot_id, __func__);

    if (en50221_app_rm_enq(d->rm_resource, session_number)) {
        printf("%02x:Failed to send ENQ\n", slot_id);
    }

    return 0;
}

static int
ca_ca_info_callback(void *arg, uint8_t slot_id, uint16_t session_number, uint32_t ca_id_count, uint16_t *ca_ids)
{
    (void)arg;
    (void)session_number;

    printf("%02x:%s\n", slot_id, __func__);
    uint32_t i;
    for(i=0; i< ca_id_count; i++) {
        printf("  Supported CA ID: %04x\n", ca_ids[i]);
    }

    //ca_connected = 1;
    return 0;
}

static int
ca_ca_pmt_reply_callback(void *arg, uint8_t slot_id, uint16_t session_number,
                               struct en50221_app_pmt_reply *reply, uint32_t reply_size)
{
    (void)arg;
    (void)session_number;
    (void)reply;
    (void)reply_size;

    printf("%02x:%s\n", slot_id, __func__);

    return 0;
}


static int
ca_dt_enquiry_callback(void *arg, uint8_t slot_id, uint16_t session_number, uint8_t response_interval)
{
    ca_device_t * d = arg;

    printf("%02x:%s\n", slot_id, __func__);
    printf("  response_interval:%i\n", response_interval);

    if (en50221_app_datetime_send(d->dt_resource, session_number, time(NULL), -1)) {
        printf("%02x:Failed to send datetime\n", slot_id);
    }

    return 0;
}

struct ca_device *
ca_init(int fd)
{
	ca_slot_info_t info;
	info.num = 0;
	int tries = 10;
	ca_device_t * d = &ca_devices[ca_devices_count];

	if (ca_devices_count == MAX_CA_DEVICES)
		return 0;

	memset(d, 0, sizeof(ca_device_t));
	d->fd = fd;

	if (ioctl(fd, CA_RESET, &info))
            return 0;

    do {
	  if (ioctl(fd, CA_GET_SLOT_INFO, &info))
            return 0;
      usleep(200000);
    } while  (tries-- && !(info.flags & CA_CI_MODULE_READY));

	if (ioctl(fd, CA_GET_SLOT_INFO, &info))
            return 0;
	LOG("initializing CA, fd %d type %d flags 0x%x", fd, info.type, info.flags);

	if (info.type != CA_CI_LINK) {
		LOG("incopatible CA interface");
		goto fail;
	}

	if (!(info.flags & CA_CI_MODULE_READY)) {
		LOG("CA module not present or not ready");
		goto fail;
	}

    if ((d->tl = en50221_tl_create(5, 32)) == NULL) {
        LOG("failed to create transport layer");
		goto fail;
    }

    if ((d->slot_id = en50221_tl_register_slot(d->tl, fd, 0, 1000, 100)) < 0) {
        LOG( "slot registration failed");
        goto fail;
    }
    printf("slotid: %i\n", d->slot_id);

    // create session layer
    d->sl = en50221_sl_create(d->tl, 256);
    if (d->sl == NULL) {
        LOG("failed to create session layer");
        goto fail;
    }

    // create the sendfuncs
    d->sf.arg        = d->sl;
    d->sf.send_data  = (en50221_send_data) en50221_sl_send_data;
    d->sf.send_datav = (en50221_send_datav) en50221_sl_send_datav;

    /* create app resources and assign callbacks */
    d->rm_resource = en50221_app_rm_create(&d->sf);
    en50221_app_rm_register_enq_callback(d->rm_resource, ca_rm_enq_callback, d);
    en50221_app_rm_register_reply_callback(d->rm_resource, ca_rm_reply_callback, d);
    en50221_app_rm_register_changed_callback(d->rm_resource, ca_rm_changed_callback, d);

    d->dt_resource = en50221_app_datetime_create(&d->sf);
    en50221_app_datetime_register_enquiry_callback(d->dt_resource, ca_dt_enquiry_callback, d);

    d->ai_resource = en50221_app_ai_create(&d->sf);
    en50221_app_ai_register_callback(d->ai_resource, ca_ai_callback, d);

    d->ca_resource = en50221_app_ca_create(&d->sf);
    en50221_app_ca_register_info_callback(d->ca_resource, ca_ca_info_callback, d);
    en50221_app_ca_register_pmt_reply_callback(d->ca_resource, ca_ca_pmt_reply_callback, d);

    pthread_t stackthread;
    pthread_create(&stackthread, NULL, stackthread_func, d);

    en50221_sl_register_lookup_callback(d->sl, ca_lookup_callback, d);
    en50221_sl_register_session_callback(d->sl, ca_session_callback, d);

    d->tc = en50221_tl_new_tc(d->tl, d->slot_id);
    printf("tcid: %i\n", d->tc);

    ca_devices_count++;
	return d;
fail:
	close(fd);
	return 0;
}

