#ifndef SATIPCLIENT_H
#define SATIPCLIENT_H

#include "adapter.h"

#define SATIP_STR_LEN 5000
#define SATIP_MAX_STRENGTH 255
#define SATIP_MAX_QUALITY 15

#define SATIP_STATE_DISCONNECTED 0
#define SATIP_STATE_SETUP 1
#define SATIP_STATE_PLAY 2
#define SATIP_STATE_TEARDOWN 3
#define SATIP_STATE_INACTIVE 0

void find_satip_adapter(adapter **a);
int satip_getxml(void *);
char *init_satip_pointer(int len);
#endif
