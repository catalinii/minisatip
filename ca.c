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
#include "utils.h"

#define MAX_MSG_LEN 4096
#define READ_DELAY 0
#define MAX_CA_DEVICES 8
#define MAX_CA_DEVICE_RESOURCES 8

typedef struct ca_device
{
	int enabled;
	int fd;
	int slot_id;
	int tc;
	int id;
	struct en50221_transport_layer *tl;
	struct en50221_session_layer * sl;

	struct en50221_app_send_functions sf;
	struct en50221_app_rm *rm_resource;
	struct en50221_app_ai *ai_resource;
	struct en50221_app_ca *ca_resource;
	struct en50221_app_datetime *dt_resource;
	int ca_session_number;
	uint16_t ai_session_number;
} ca_device_t;

static struct ca_device *ca_devices[MAX_ADAPTERS];

// this contains all known resource ids so we can see if the cam asks for something exotic
uint32_t resource_ids[] =
{ EN50221_APP_TELETEXT_RESOURCEID, EN50221_APP_SMARTCARD_RESOURCEID(1),
EN50221_APP_RM_RESOURCEID, EN50221_APP_MMI_RESOURCEID,
		EN50221_APP_LOWSPEED_RESOURCEID(1, 1), EN50221_APP_EPG_RESOURCEID(1),
		EN50221_APP_DVB_RESOURCEID, EN50221_APP_CA_RESOURCEID,
		EN50221_APP_DATETIME_RESOURCEID, EN50221_APP_AUTH_RESOURCEID,
		EN50221_APP_AI_RESOURCEID, };
int resource_ids_count = sizeof(resource_ids) / 4;

void hexdump(uint8_t buffer[], int len)
{
#define HEXDUMP_LINE_LEN	16
	int i;
	char s[HEXDUMP_LINE_LEN + 1];
	bzero(s, HEXDUMP_LINE_LEN+1);

	for (i = 0; i < len; i++)
	{
		if (!(i % HEXDUMP_LINE_LEN))
		{
			if (s[0])
				LOG("[%s]", s);
			LOG("%05x: ", i);
			bzero(s, HEXDUMP_LINE_LEN);
		}
		s[i % HEXDUMP_LINE_LEN] = isprint(buffer[i]) ? buffer[i] : '.';
		LOG("%02x ", buffer[i]);
	}
	while (i++ % HEXDUMP_LINE_LEN)
		LOG("   ");

	LOG("[%s]", s);
}

int dvbca_process_pmt(adapter *ad, void *arg)
{
	ca_device_t * d = ca_devices[ad->id];
	uint16_t pid, sid, ver;
	SPid *p;
	int len, rc;
	SPMT *spmt = (SPMT *) arg;
	uint8_t *b = spmt->pmt;

	if (!d)
		return -1;

	pid = spmt->pid;
	len = spmt->pmt_len;

	ver = (b[5] >> 1) & 0x1f;
	sid = (b[3] << 8) | b[4];

	LOG("PMT CA pid %u len %u ver %u sid %u (%x)", pid, len, ver, sid, sid);
	uint8_t capmt[4096];
	int size;
	struct section *section = section_codec(b, len + 3);

	if (!section)
	{
		LOG("failed to decode section");
		return -1;
	}

	struct section_ext *result = section_ext_decode(section, 0);
	if (!section)
	{
		LOG("failed to decode ext_section");
		return -1;
	}

	struct mpeg_pmt_section *pmt = mpeg_pmt_section_codec(result);
	if (!pmt)
	{
		LOG("failed to decode pmt");
		return -1;
	}

	if ((size = en50221_ca_format_pmt((struct mpeg_pmt_section *) b, capmt,
			sizeof(capmt), 0, CA_LIST_MANAGEMENT_ONLY,
			CA_PMT_CMD_ID_OK_DESCRAMBLING)) < 0)
		LOG("Failed to format CA PMT object");
	if ((rc = en50221_app_ca_pmt(d->ca_resource, d->ca_session_number, capmt,
			size)))
	{
		LOG("Adapter %d, Failed to send CA PMT object, error %d", ad->id, rc);
	}
//	hexdump(&capmt[0], size);

	return 0;
}

void dvbca_del_pmt(adapter *ad, void *arg)
{

}

int ca_ai_callback(void *arg, uint8_t slot_id, uint16_t session_number,
		uint8_t application_type, uint16_t application_manufacturer,
		uint16_t manufacturer_code, uint8_t menu_string_length,
		uint8_t *menu_string)
{
	ca_device_t * d = arg;

	LOG("%02x:%s", slot_id, __func__);
	LOG("  Application type: %02x", application_type);
	LOG("  Application manufacturer: %04x", application_manufacturer);
	LOG("  Manufacturer code: %04x", manufacturer_code);
	LOG("  Menu string: %.*s", menu_string_length, menu_string);

	d->ai_session_number = session_number;

	return 0;
}

extern __thread char *thread_name;
void *
stackthread_func(void* arg)
{
	char name[100];
	ca_device_t * d = arg;
	int lasterror = 0;
	sprintf(name, "CA%d", d->id);
	thread_name = name;
	LOG("%s: start", __func__);

	while (d->enabled)
	{
		int error;
		if ((error = en50221_tl_poll(d->tl)) != 0)
		{
			if (error != lasterror)
			{
				LOG("Error reported by stack slot:%i error:%i",
						en50221_tl_get_error_slot(d->tl),
						en50221_tl_get_error(d->tl));
			}
			lasterror = error;
		}
	}

	return 0;
}

static int ca_session_callback(void *arg, int reason, uint8_t slot_id,
		uint16_t session_number, uint32_t resource_id)
{
	ca_device_t * d = arg;
	LOG("%s: reason %d slot_id %u session_number %u resource_id %x", __func__,
			reason, slot_id, session_number, resource_id);

	switch (reason)
	{
	case S_SCALLBACK_REASON_CAMCONNECTING:
		LOG("CAM connecting")
		;
		break;
	case S_SCALLBACK_REASON_CAMCONNECTED:
		LOG("CAM connected")
		;
		if (resource_id == EN50221_APP_RM_RESOURCEID)
		{
			en50221_app_rm_enq(d->rm_resource, session_number);
		}
		else if (resource_id == EN50221_APP_AI_RESOURCEID)
		{
			en50221_app_ai_enquiry(d->ai_resource, session_number);
		}
		else if (resource_id == EN50221_APP_CA_RESOURCEID)
		{
			en50221_app_ca_info_enq(d->ca_resource, session_number);
			d->ca_session_number = session_number;
		}
		break;
	}
	return 0;
}

static int ca_lookup_callback(void * arg, uint8_t slot_id,
		uint32_t requested_resource_id,
		en50221_sl_resource_callback *callback_out, void **arg_out,
		uint32_t *connected_resource_id)
{
	ca_device_t * d = arg;

	LOG("%s: slot_id %u requested_resource_id %x", __func__, slot_id,
			requested_resource_id);

	switch (requested_resource_id)
	{
	case EN50221_APP_RM_RESOURCEID:
		*callback_out = (en50221_sl_resource_callback) en50221_app_rm_message;
		*arg_out = d->rm_resource;
		*connected_resource_id = EN50221_APP_RM_RESOURCEID;
		break;
	case EN50221_APP_AI_RESOURCEID:
		*callback_out = (en50221_sl_resource_callback) en50221_app_ai_message;
		*arg_out = d->ai_resource;
		*connected_resource_id = EN50221_APP_AI_RESOURCEID;
		break;
	case EN50221_APP_CA_RESOURCEID:
		*callback_out = (en50221_sl_resource_callback) en50221_app_ca_message;
		*arg_out = d->ca_resource;
		*connected_resource_id = EN50221_APP_CA_RESOURCEID;
		break;
	case EN50221_APP_DATETIME_RESOURCEID:
		*callback_out =
				(en50221_sl_resource_callback) en50221_app_datetime_message;
		*arg_out = d->dt_resource;
		*connected_resource_id = EN50221_APP_DATETIME_RESOURCEID;
		break;
	default:
		LOG("unknown resource id")
		;
		return -1;
	}
	return 0;
}

static int ca_rm_enq_callback(void *arg, uint8_t slot_id,
		uint16_t session_number)
{
	ca_device_t * d = arg;

	LOG("%02x:%s", slot_id, __func__);

	if (en50221_app_rm_reply(d->rm_resource, session_number, resource_ids_count,
			resource_ids))
	{
		LOG("%02x:Failed to send reply to ENQ", slot_id);
	}

	return 0;
}

static int ca_rm_reply_callback(void *arg, uint8_t slot_id,
		uint16_t session_number, uint32_t resource_id_count,
		uint32_t *_resource_ids)
{
	ca_device_t * d = arg;
	LOG("%02x:%s", slot_id, __func__);

	uint32_t i;
	for (i = 0; i < resource_id_count; i++)
	{
		LOG("  CAM provided resource id: %08x", _resource_ids[i]);
	}

	if (en50221_app_rm_changed(d->rm_resource, session_number))
	{
		LOG("%02x:Failed to send REPLY", slot_id);
	}

	return 0;
}

static int ca_rm_changed_callback(void *arg, uint8_t slot_id,
		uint16_t session_number)
{
	ca_device_t * d = arg;
	LOG("%02x:%s", slot_id, __func__);

	if (en50221_app_rm_enq(d->rm_resource, session_number))
	{
		LOG("%02x:Failed to send ENQ", slot_id);
	}

	return 0;
}

static int ca_ca_info_callback(void *arg, uint8_t slot_id,
		uint16_t session_number, uint32_t ca_id_count, uint16_t *ca_ids)
{
	(void) arg;
	(void) session_number;

	LOG("%02x:%s", slot_id, __func__);
	uint32_t i;
	for (i = 0; i < ca_id_count; i++)
	{
		LOG("  Supported CA ID: %04x", ca_ids[i]);
	}

	//ca_connected = 1;
	return 0;
}

static int ca_ca_pmt_reply_callback(void *arg, uint8_t slot_id,
		uint16_t session_number, struct en50221_app_pmt_reply *reply,
		uint32_t reply_size)
{
	(void) arg;
	(void) session_number;
	(void) reply;
	(void) reply_size;

	LOG("%02x:%s", slot_id, __func__);

	return 0;
}

static int ca_dt_enquiry_callback(void *arg, uint8_t slot_id,
		uint16_t session_number, uint8_t response_interval)
{
	ca_device_t * d = arg;

	LOG("%02x:%s", slot_id, __func__);
	LOG("  response_interval:%i", response_interval);

	if (en50221_app_datetime_send(d->dt_resource, session_number, time(NULL),
			-1))
	{
		LOG("%02x:Failed to send datetime", slot_id);
	}

	return 0;
}

int ca_init(ca_device_t *d)
{
	ca_slot_info_t info;
	info.num = 0;
	int tries = 10;
	int fd = d->fd;

	if (ioctl(fd, CA_RESET, &info))
		return 0;

	do
	{
		if (ioctl(fd, CA_GET_SLOT_INFO, &info))
			return 0;
		usleep(200000);
	} while (tries-- && !(info.flags & CA_CI_MODULE_READY));

	if (ioctl(fd, CA_GET_SLOT_INFO, &info))
		return 0;
	LOG("initializing CA, fd %d type %d flags 0x%x", fd, info.type, info.flags);

	if (info.type != CA_CI_LINK)
	{
		LOG("incopatible CA interface");
		goto fail;
	}

	if (!(info.flags & CA_CI_MODULE_READY))
	{
		LOG("CA module not present or not ready");
		goto fail;
	}

	if ((d->tl = en50221_tl_create(5, 32)) == NULL)
	{
		LOG("failed to create transport layer");
		goto fail;
	}

	if ((d->slot_id = en50221_tl_register_slot(d->tl, fd, 0, 1000, 100)) < 0)
	{
		LOG("slot registration failed");
		goto fail;
	}
	LOG("slotid: %i", d->slot_id);

	// create session layer
	d->sl = en50221_sl_create(d->tl, 256);
	if (d->sl == NULL)
	{
		LOG("failed to create session layer");
		goto fail;
	}

	// create the sendfuncs
	d->sf.arg = d->sl;
	d->sf.send_data = (en50221_send_data) en50221_sl_send_data;
	d->sf.send_datav = (en50221_send_datav) en50221_sl_send_datav;

	/* create app resources and assign callbacks */
	d->rm_resource = en50221_app_rm_create(&d->sf);
	en50221_app_rm_register_enq_callback(d->rm_resource, ca_rm_enq_callback, d);
	en50221_app_rm_register_reply_callback(d->rm_resource, ca_rm_reply_callback,
			d);
	en50221_app_rm_register_changed_callback(d->rm_resource,
			ca_rm_changed_callback, d);

	d->dt_resource = en50221_app_datetime_create(&d->sf);
	en50221_app_datetime_register_enquiry_callback(d->dt_resource,
			ca_dt_enquiry_callback, d);

	d->ai_resource = en50221_app_ai_create(&d->sf);
	en50221_app_ai_register_callback(d->ai_resource, ca_ai_callback, d);

	d->ca_resource = en50221_app_ca_create(&d->sf);
	en50221_app_ca_register_info_callback(d->ca_resource, ca_ca_info_callback,
			d);
	en50221_app_ca_register_pmt_reply_callback(d->ca_resource,
			ca_ca_pmt_reply_callback, d);

	pthread_t stackthread;
	pthread_create(&stackthread, NULL, stackthread_func, d);

	en50221_sl_register_lookup_callback(d->sl, ca_lookup_callback, d);
	en50221_sl_register_session_callback(d->sl, ca_session_callback, d);

	d->tc = en50221_tl_new_tc(d->tl, d->slot_id);
	LOG("tcid: %i", d->tc);

	return 1;
	fail: close(fd);
	d->enabled = 0;
	return 0;
}

int dvbca_init_dev(adapter *ad, void *arg)
{
	ca_device_t *c = ca_devices[ad->id];
	int fd;
	char ca_dev_path[100];

	if (c && c->enabled)
		return 1;

	if (ad->type != ADAPTER_DVB)
		return 0;

	sprintf(ca_dev_path, "/dev/dvb/adapter%d/ca0", ad->pa);
	fd = open(ca_dev_path, O_RDWR);
	if (fd <= 0)
		LOG_AND_RETURN(0, "No CA device detected on adapter %d", ad->id);
	if (!c)
	{
		c = ca_devices[ad->id] = malloc1(sizeof(ca_device_t));
		if (!c)
			LOG_AND_RETURN(0, "Could not allocate memory for CA device %d",
					ad->id);
		memset(c, 0, sizeof(ca_device_t));
	}
	c->enabled = 1;
	c->fd = fd;
	c->id = ad->id;
	return ca_init(c);
}

int dvbca_close_dev(adapter *ad, void *arg)
{
	ca_device_t *c = ca_devices[ad->id];
	if (c && c->enabled)
	{
		close(c->fd);
		c->enabled = 0;
	}
	return 1;
}

SCA dvbca;

void dvbca_init()
{
	memset(dvbca.action, 0, sizeof(dvbca.action));
	dvbca.enabled = 1; // ignore it anyway
	dvbca.action[CA_INIT_DEVICE] = (ca_action) &dvbca_init_dev;
	dvbca.action[CA_CLOSE_DEVICE] = (ca_action) &dvbca_close_dev;
	dvbca.action[CA_ADD_PMT] = (ca_action) &dvbca_process_pmt;
	dvbca.action[CA_DEL_PMT] = (ca_action) &dvbca_del_pmt;
	add_ca(&dvbca);
}
