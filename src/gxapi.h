#ifndef __GXAPI_H
#define __GXAPI_H

#include "adapter.h"

int gx_check_ts_lock(adapter *ad);

int gx_read_ts(void *buf, int len, sockets *ss);

#endif