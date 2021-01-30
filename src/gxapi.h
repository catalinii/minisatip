#ifndef __GXAPI_H
#define __GXAPI_H

#include "adapter.h"

typedef enum _FRONTEND_MODE
{
	DVBS2_NORMAL,
	DVBS2_BLIND,
	DVBT_AUTO_MODE,
	DVBT_NORMAL,
	DVBT2_BASE,
	DVBT2_LITE,
	DVBT2_BASE_LITE,
	DVBC_J83A,
	DVBC_J83B,
	DTMB_C,
	DTMB,
} FRONTEND_MODE;

extern int find_slot(adapter *ad, int pid);

int gx_check_ts_lock(adapter *ad);

int gx_read_ts(void *buf, int len, sockets *ss);

int gx_get_dvb_mode(int sys);

#endif