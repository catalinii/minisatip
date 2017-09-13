/*
 * Copyright (C) 2015 Damjan Marion <damjan.marion@gmail.com>
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

#include <linux/dvb/ca.h>
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
#define DEFAULT_LOG LOG_DVBCA
#define MAX_CA_PMT 20

#define TS101699_APP_RM_RESOURCEID MKRID(1, 1, 2)
#define TS101699_APP_AI_RESOURCEID MKRID(2, 1, 2)
#define CIPLUS13_APP_AI_RESOURCEID MKRID(2, 1, 3)
#define CIPLUS_APP_DVB_RESOURCEID MKRID(32, 1, 2)
//#define CIPLUS_APP_LOWSPEED_RESOURCEID	(DEVICE_TYPE, DEVICE_NUMBER) MKRID(96,((DEVICE_TYPE)<<2)|((DEVICE_NUMBER) & 0x03),2)
//#define CIPLUS_APP_LOWSPEED_RESOURCEID_TWO	(DEVICE_TYPE, DEVICE_NUMBER) MKRID(96,((DEVICE_TYPE)<<2)|((DEVICE_NUMBER) & 0x03),3) //CI+ v1.3
#define CIPLUS_APP_CC_RESOURCEID MKRID(140, 64, 1)
#define CIPLUS_APP_CC_RESOURCEID_TWO MKRID(140, 64, 2) //CI+ v1.3
#define CIPLUS_APP_LANG_RESOURCEID MKRID(141, 64, 1)
#define CIPLUS_APP_UPGR_RESOURCEID MKRID(142, 64, 1)
#define CIPLUS_APP_OPRF_RESOURCEID MKRID(143, 64, 1)
#define CIPLUS_APP_SAS_RESOURCEID MKRID(150, 64, 1)
#define TS101699_APP_AMMI_RESOURCEID MKRID(65, 1, 1)
#define CIPLUS_APP_AMMI_RESOURCEID MKRID(65, 1, 2)

#define CIPLUS_TAG_CC_OPEN_REQ 0x9f9001
#define CIPLUS_TAG_CC_OPEN_CNF 0x9f9002
#define CIPLUS_TAG_CC_DATA_REQ 0x9f9003
#define CIPLUS_TAG_CC_DATA_CNF 0x9f9004
#define CIPLUS_TAG_CC_SYNC_REQ 0x9f9005
#define CIPLUS_TAG_CC_SAC_DATA_REQ 0x9f9007
#define CIPLUS_TAG_CC_SAC_SYNC_REQ 0x9f9009
#define CIPLUS_TAG_APP_INFO 0x9f8021
#define CIPLUS_TAG_CICAM_RESET 0x9f8023

typedef struct ca_device
{
	int enabled;
	int fd;
	int slot_id;
	int tc;
	int id;
	int ignore_close;
	int init_ok, pmt_id[MAX_CA_PMT];

	int ca_high_bitrate_mode;
	int ca_ai_version;
	pthread_t stackthread;
	struct en50221_transport_layer *tl;
	struct en50221_session_layer *sl;

	struct en50221_app_send_functions sf;
	struct en50221_app_rm *rm_resource;
	struct en50221_app_ai *ai_resource;
	struct en50221_app_ca *ca_resource;
	struct en50221_app_datetime *dt_resource;
	int ca_session_number;
	int ai_version;
	uint16_t ai_session_number;
	uint8_t key[2][16], iv[2][16];

} ca_device_t;

int dvbca_id;
static struct ca_device *ca_devices[MAX_ADAPTERS];
#define TS101699_APP_AI_RESOURCEID MKRID(2, 1, 2)
#define CIPLUS13_APP_AI_RESOURCEID MKRID(2, 1, 3)

typedef enum {
	CIPLUS13_DATA_RATE_72_MBPS = 0,
	CIPLUS13_DATA_RATE_96_MBPS = 1,
} ciplus13_data_rate_t;

int dvbca_close_device(ca_device_t *c);

static int
ciplus13_app_ai_data_rate_info(ca_device_t *d, ciplus13_data_rate_t rate)
{
	uint8_t data[] = {0x9f, 0x80, 0x24, 0x01, (uint8_t)rate};

	/* only version 3 (CI+ 1.3) supports data_rate_info */
	if (d->ai_version != 3)
		return 0;

	LOG("setting CI+ CAM data rate to %s Mbps", rate ? "96" : "72");

	return en50221_sl_send_data(d->sl, d->ai_session_number, data, sizeof(data));
}

// this contains all known resource ids so we can see if the cam asks for something exotic
uint32_t resource_ids[] =
	{
		EN50221_APP_TELETEXT_RESOURCEID, EN50221_APP_SMARTCARD_RESOURCEID(1),
		EN50221_APP_RM_RESOURCEID, EN50221_APP_MMI_RESOURCEID,
		EN50221_APP_LOWSPEED_RESOURCEID(1, 1), EN50221_APP_EPG_RESOURCEID(1),
		EN50221_APP_DVB_RESOURCEID, EN50221_APP_CA_RESOURCEID,
		EN50221_APP_DATETIME_RESOURCEID, EN50221_APP_AUTH_RESOURCEID,
		EN50221_APP_AI_RESOURCEID, TS101699_APP_AI_RESOURCEID, CIPLUS13_APP_AI_RESOURCEID};
int resource_ids_count = sizeof(resource_ids) / 4;

int dvbca_process_pmt(adapter *ad, SPMT *spmt)
{
	ca_device_t *d = ca_devices[ad->id];
	uint16_t pid, sid, ver;
	int len, rc, listmgmt, i;
	uint8_t *b = spmt->pmt;

	if (!d)
		return TABLES_RESULT_ERROR_NORETRY;
	if (!d->init_ok)
		LOG_AND_RETURN(TABLES_RESULT_ERROR_RETRY, "CAM not yet initialized");

	pid = spmt->pid;
	len = spmt->pmt_len;
	for (i = 0; i < MAX_CA_PMT; i++)
		if ((d->pmt_id[i] == -1) || (d->pmt_id[i] == spmt->id))
			break;

	if (i < MAX_CA_PMT)
		d->pmt_id[i] = spmt->id;
	else
		LOG_AND_RETURN(TABLES_RESULT_ERROR_RETRY, "pmt_id full for device %d", d->id);

	ver = (b[5] >> 1) & 0x1f;
	sid = (b[3] << 8) | b[4];

	LOG("PMT CA pid %u len %u ver %u sid %u (%x)", pid, len, ver, sid, sid);
	uint8_t capmt[8192];
	int size;
	struct section *section = section_codec(b, len);

	//	d->sp = 0;

	if (!section)
	{
		LOG("failed to decode section");
		return TABLES_RESULT_ERROR_RETRY;
	}

	struct section_ext *result = section_ext_decode(section, 0);
	if (!section)
	{
		LOG("failed to decode ext_section");
		return TABLES_RESULT_ERROR_RETRY;
	}

	struct mpeg_pmt_section *pmt = mpeg_pmt_section_codec(result);
	if (!pmt)
	{
		LOG("failed to decode pmt");
		return TABLES_RESULT_ERROR_RETRY;
	}

	listmgmt = CA_LIST_MANAGEMENT_ONLY;
	for (i = 0; i < MAX_CA_PMT; i++)
		if (d->pmt_id[i] != spmt->id)
			listmgmt = CA_LIST_MANAGEMENT_ADD;

	if ((size = en50221_ca_format_pmt((struct mpeg_pmt_section *)b, capmt,
									  sizeof(capmt), 0, listmgmt,
									  CA_PMT_CMD_ID_OK_DESCRAMBLING)) < 0)
		LOG("Failed to format CA PMT object");
	if ((rc = en50221_app_ca_pmt(d->ca_resource, d->ca_session_number, capmt,
								 size)))
	{
		LOG("Adapter %d, Failed to send CA PMT object, error %d", ad->id, rc);
	}

	if (d->key[0][0])
		send_cw(spmt->id, CA_ALGO_AES128_CBC, 0, d->key[0], d->iv[0], 3600); // 1 hour
	if (d->key[1][0])
		send_cw(spmt->id, CA_ALGO_AES128_CBC, 1, d->key[1], d->iv[1], 3600);

	return 0;
}

int dvbca_del_pmt(adapter *ad, SPMT *spmt)
{
	ca_device_t *d = ca_devices[ad->id];
	int i;
	for (i = 0; i < MAX_CA_PMT; i++)
		if (d->pmt_id[i] == spmt->id)
		{
			LOG("CA %d, removing PMT from position %d", ad->id, i);
			d->pmt_id[i] = -1;
		}

	return 0;
}

int ca_ai_callback(void *arg, uint8_t slot_id, uint16_t session_number,
				   uint8_t application_type, uint16_t application_manufacturer,
				   uint16_t manufacturer_code, uint8_t menu_string_length,
				   uint8_t *menu_string)
{
	ca_device_t *d = arg;

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
stackthread_func(void *arg)
{
	char name[100];
	ca_device_t *d = arg;
	int lasterror = 0;
	adapter *ad;
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
				ad = get_adapter(d->id);
				if (ad)
					ad->adapter_timeout = opts.adapter_timeout;
				d->ignore_close = 0; // force device close..
									 //break;
			}
			lasterror = error;
		}
	}

	return 0;
}

static int ca_session_callback(void *arg, int reason, uint8_t slot_id,
							   uint16_t session_number, uint32_t resource_id)
{
	ca_device_t *d = arg;

	LOG("%s: reason %d slot_id %u session_number %u resource_id %x", __func__,
		reason, slot_id, session_number, resource_id);

	switch (reason)
	{
	case S_SCALLBACK_REASON_CAMCONNECTING: //0
		LOG("%02x:CAM connecting to resource %08x, session_number %i",
			slot_id, resource_id, session_number);
		;
		break;
	case S_SCALLBACK_REASON_CLOSE: //5
		LOG("%02x:Connection to resource %08x, session_number %i closed",
			slot_id, resource_id, session_number);

		if (resource_id == EN50221_APP_CA_RESOURCEID)
		{
			d->ignore_close = 1;
			d->init_ok = 0;
			LOG("_________S_SCALLBACK_REASON_CLOSE___________EN50221_APP_CA_RESOURCEID__________________________");
		}
		else if (resource_id == CIPLUS_APP_CC_RESOURCEID || resource_id == CIPLUS_APP_CC_RESOURCEID_TWO)
		{
			LOG("__________S_SCALLBACK_REASON_CLOSE____________CIPLUS_APP_CC_RESOURCEID________________________");
		}
		else if (resource_id == EN50221_APP_MMI_RESOURCEID)
		{
			LOG("_________S_SCALLBACK_REASON_CLOSE____________EN50221_APP_MMI_RESOURCEID_________________________");
		}
		break;

	case S_SCALLBACK_REASON_TC_CONNECT: //6
		LOG("%02x:Host originated transport connection %i resource 0x%08x connected", slot_id, resource_id, session_number);
		break;
	case S_SCALLBACK_REASON_TC_CAMCONNECT: //7
		LOG("%02x:CAM originated transport connection %i connected", slot_id, session_number);
		break;
	case S_SCALLBACK_REASON_CAMCONNECTED: //1
		LOG("%02x:CAM successfully connected to resource %08x, session_number %i",
			slot_id, resource_id, session_number);

		if (resource_id == EN50221_APP_RM_RESOURCEID)
		{
			en50221_app_rm_enq(d->rm_resource, session_number);
			LOG("--------------------S_SCALLBACK_REASON_CAMCONNECTED---------EN50221_APP_RM_RESOURCEID-------------------------");
		}
		else if (resource_id == EN50221_APP_AI_RESOURCEID ||
				 resource_id == TS101699_APP_AI_RESOURCEID)
		{
			d->ai_session_number = session_number;
			en50221_app_ai_enquiry(d->ai_resource, session_number);
			LOG("--------------------S_SCALLBACK_REASON_CAMCONNECTED---------EN50221_APP_AI_RESOURCEID-------------------------");
		}
		else if (resource_id == CIPLUS13_APP_AI_RESOURCEID)
		{
			d->ca_ai_version = resource_id & 0x3f;
			d->ai_session_number = session_number;
			d->ca_high_bitrate_mode = 1; // 96 MBPS now (should get from command line)
			en50221_app_ai_enquiry(d->ai_resource, session_number);
			ciplus13_app_ai_data_rate_info(d, d->ca_high_bitrate_mode ? CIPLUS13_DATA_RATE_96_MBPS : CIPLUS13_DATA_RATE_72_MBPS);
			LOG("--------------------S_SCALLBACK_REASON_CAMCONNECTED---------CIPLUS13_APP_AI_RESOURCEID-------------------------");
		}
		else if (resource_id == EN50221_APP_CA_RESOURCEID)
		{
			en50221_app_ca_info_enq(d->ca_resource, session_number);
			d->ca_session_number = session_number;
			LOG("--------------------S_SCALLBACK_REASON_CAMCONNECTED---------EN50221_APP_CA_RESOURCEID-------------------------");
		}
		else if (resource_id == EN50221_APP_MMI_RESOURCEID)
		{
			LOG("--------------------S_SCALLBACK_REASON_CAMCONNECTED---------EN50221_APP_MMI_RESOURCEID-------------------------");
		}
		else if (resource_id == EN50221_APP_DVB_RESOURCEID || resource_id == CIPLUS_APP_DVB_RESOURCEID)
		{
			LOG("--------------------S_SCALLBACK_REASON_CAMCONNECTED---------EN50221_APP_DVB_RESOURCEID-------------------------");
		}
		else if (resource_id == CIPLUS_APP_SAS_RESOURCEID)
		{
			LOG("--------------------S_SCALLBACK_REASON_CAMCONNECTED---------EN50221_APP_SAS_RESOURCEID-------------------------");
		}
		else if (resource_id == CIPLUS_APP_OPRF_RESOURCEID)
		{
			LOG("--------------------S_SCALLBACK_REASON_CAMCONNECTED---------EN50221_APP_OPRF_RESOURCEID-------------------------");
		}
		d->ignore_close = 1;
		d->init_ok = 1;
		break;
	case S_SCALLBACK_REASON_CAMCONNECTFAIL: //2
		LOG("%02x:CAM on failed to connect to resource %08x", slot_id, resource_id);
		break;
	case S_SCALLBACK_REASON_CONNECTED: //3
		LOG("%02x:Host connection to resource %08x connected successfully, session_number %i",
			slot_id, resource_id, session_number);
		break;
	case S_SCALLBACK_REASON_CONNECTFAIL: //4
		LOG("%02x:Host connection to resource %08x failed, session_number %i",
			slot_id, resource_id, session_number);
		break;
	}
	return 0;
}

static int ca_lookup_callback(void *arg, uint8_t slot_id,
							  uint32_t requested_resource_id,
							  en50221_sl_resource_callback *callback_out, void **arg_out,
							  uint32_t *connected_resource_id)
{
	ca_device_t *d = arg;

	LOG("%s: slot_id %u requested_resource_id %x", __func__, slot_id,
		requested_resource_id);

	switch (requested_resource_id)
	{
	case EN50221_APP_RM_RESOURCEID:
		*callback_out = (en50221_sl_resource_callback)en50221_app_rm_message;
		*arg_out = d->rm_resource;
		*connected_resource_id = EN50221_APP_RM_RESOURCEID;
		break;
	case EN50221_APP_AI_RESOURCEID:
		*callback_out = (en50221_sl_resource_callback)en50221_app_ai_message;
		*arg_out = d->ai_resource;
		*connected_resource_id = EN50221_APP_AI_RESOURCEID;
		break;
	case EN50221_APP_CA_RESOURCEID:
		*callback_out = (en50221_sl_resource_callback)en50221_app_ca_message;
		*arg_out = d->ca_resource;
		*connected_resource_id = EN50221_APP_CA_RESOURCEID;
		break;
	case EN50221_APP_DATETIME_RESOURCEID:
		*callback_out =
			(en50221_sl_resource_callback)en50221_app_datetime_message;
		*arg_out = d->dt_resource;
		*connected_resource_id = EN50221_APP_DATETIME_RESOURCEID;
		break;
	case TS101699_APP_AI_RESOURCEID:
	case CIPLUS13_APP_AI_RESOURCEID:
		*callback_out = (en50221_sl_resource_callback)en50221_app_ai_message;
		*arg_out = d->ai_resource;
		*connected_resource_id = requested_resource_id;
		break;

	default:
		LOG("unknown resource id");
		return -1;
	}
	return 0;
}

static int ca_rm_enq_callback(void *arg, uint8_t slot_id,
							  uint16_t session_number)
{
	ca_device_t *d = arg;

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
	ca_device_t *d = arg;
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
	ca_device_t *d = arg;
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
	(void)session_number;
	ca_device_t *d = arg;
	LOG("%02x:%s", slot_id, __func__);
	uint32_t i;
	for (i = 0; i < ca_id_count; i++)
	{
		LOG("  Supported CA ID: %04x for CA %d", ca_ids[i], d->id);
		add_caid_mask(dvbca_id, d->id, ca_ids[i], 0xFFFF);
	}
	return 0;
}

static int ca_ca_pmt_reply_callback(void *arg, uint8_t slot_id,
									uint16_t session_number, struct en50221_app_pmt_reply *reply,
									uint32_t reply_size)
{
	(void)arg;
	(void)session_number;
	(void)reply;
	(void)reply_size;

	LOG("%02x:%s", slot_id, __func__);

	return 0;
}

static int ca_dt_enquiry_callback(void *arg, uint8_t slot_id,
								  uint16_t session_number, uint8_t response_interval)
{
	ca_device_t *d = arg;

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
	int64_t st = getTick();
	info.num = 0;
	int tries = 800; // wait up to 8s for the CAM
	int fd = d->fd;

	d->tl = NULL;
	d->sl = NULL;
	d->slot_id = -1;

#ifdef ENIGMA
	char buf[256];
	read(fd, buf, sizeof(buf));
	if (ioctl(fd, 0))
		LOG_AND_RETURN(0, "%s: Could not reset ca %d", __FUNCTION__, d->id);
#else
	if (ioctl(fd, CA_RESET, &info))
		LOG_AND_RETURN(0, "%s: Could not reset ca %d", __FUNCTION__, d->id);

	do
	{
		if (ioctl(fd, CA_GET_SLOT_INFO, &info))
			LOG_AND_RETURN(0, "%s: Could not get info1 for ca %d", __FUNCTION__, d->id);
		usleep(10000);
	} while (tries-- && !(info.flags & CA_CI_MODULE_READY));

	if (ioctl(fd, CA_GET_SLOT_INFO, &info))
		LOG_AND_RETURN(0, "%s: Could not get info2 for ca %d", __FUNCTION__, d->id);

	if (info.type != CA_CI_LINK)
	{
		LOG("incompatible CA interface");
		goto fail;
	}

	if (!(info.flags & CA_CI_MODULE_READY))
	{
		LOG("CA module not present or not ready");
		goto fail;
	}
#endif
	LOG("initializing CA%d, fd %d type %d flags 0x%x, after %jd ms", d->id, fd, info.type, info.flags, (getTick() - st));

	if ((d->tl = en50221_tl_create(5, 32)) == NULL)
	{
		LOG("failed to create transport layer");
		goto fail;
	}

	if ((d->slot_id = en50221_tl_register_slot(d->tl, fd, 0, 10000, 1000)) < 0)
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
	d->sf.send_data = (en50221_send_data)en50221_sl_send_data;
	d->sf.send_datav = (en50221_send_datav)en50221_sl_send_datav;

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

	pthread_create(&d->stackthread, NULL, stackthread_func, d);

	en50221_sl_register_lookup_callback(d->sl, ca_lookup_callback, d);
	en50221_sl_register_session_callback(d->sl, ca_session_callback, d);

	d->tc = en50221_tl_new_tc(d->tl, d->slot_id);
	LOG("tcid: %i", d->tc);

	return 0;
fail:
	close(fd);
	d->enabled = 0;
	return 1;
}

int dvbca_init_dev(adapter *ad)
{
	ca_device_t *c = ca_devices[ad->id];
	int fd;
	char ca_dev_path[100];

	if (c && c->enabled)
		return TABLES_RESULT_OK;

	if (ad->type != ADAPTER_DVB)
		return TABLES_RESULT_ERROR_NORETRY;
#ifdef ENIGMA
	sprintf(ca_dev_path, "/dev/ci%d", ad->pa);
#else
	sprintf(ca_dev_path, "/dev/dvb/adapter%d/ca0", ad->pa);
#endif
	fd = open(ca_dev_path, O_RDWR);
	if (fd <= 0)
		LOG_AND_RETURN(TABLES_RESULT_ERROR_NORETRY, "No CA device detected on adapter %d", ad->id);
	if (!c)
	{
		c = ca_devices[ad->id] = malloc1(sizeof(ca_device_t));
		if (!c)
		{
			close(fd);
			LOG_AND_RETURN(0, "Could not allocate memory for CA device %d", ad->id);
		}
		memset(c, 0, sizeof(ca_device_t));
	}
	c->enabled = 1;
	c->ignore_close = 0;
	c->fd = fd;
	c->id = ad->id;
	c->ca_high_bitrate_mode = 0;
	c->stackthread = 0;
	c->init_ok = 0;
	memset(c->pmt_id, -1, sizeof(c->pmt_id));
	memset(c->key[0], 0, sizeof(c->key[0]));
	memset(c->key[1], 0, sizeof(c->key[0]));
	memset(c->iv[0], 0, sizeof(c->iv[0]));
	memset(c->iv[1], 0, sizeof(c->iv[0]));
	if (ca_init(c))
	{
		dvbca_close_device(c);
		return TABLES_RESULT_ERROR_NORETRY;
	}
	return TABLES_RESULT_OK;
}

int dvbca_close_device(ca_device_t *c)
{
	LOG("closing CA device %d, fd %d", c->id, c->fd);
	c->enabled = 0;
	if (c->stackthread)
		pthread_join(c->stackthread, NULL);
	if (c->tl && (c->slot_id >= 0))
		en50221_tl_destroy_slot(c->tl, c->slot_id);
	if (c->sl)
		en50221_sl_destroy(c->sl);
	if (c->tl >= 0)
		en50221_tl_destroy(c->tl);
	if (c->fd >= 0)
		close(c->fd);
	return 0;
}
int dvbca_close_dev(adapter *ad)
{
	ca_device_t *c = ca_devices[ad->id];
	if (c && c->enabled && !c->ignore_close) // do not close the CA unless in a bad state
	{
		dvbca_close_device(c);
	}
	return TABLES_RESULT_OK;
}
SCA_op dvbca;

int dvbca_close()
{
	int i;
	for (i = 0; i < MAX_ADAPTERS; i++)
		if (ca_devices[i] && ca_devices[i]->enabled)
		{
			dvbca_close_device(ca_devices[i]);
		}
	return 0;
}

void dvbca_init() // you can search the devices here and fill the ca_devices, then open them here (for example independent CA devices), or use dvbca_init_dev to open them (like in this module)
{
	memset(&dvbca, 0, sizeof(dvbca));
	dvbca.ca_init_dev = dvbca_init_dev;
	dvbca.ca_close_dev = dvbca_close_dev;
	dvbca.ca_add_pmt = dvbca_process_pmt;
	dvbca.ca_del_pmt = dvbca_del_pmt;
	dvbca.ca_close_ca = dvbca_close;
	dvbca_id = add_ca(&dvbca, 0xFFFFFFFF);
}
