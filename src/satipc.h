#ifndef SATIPCLIENT_H
#define SATIPCLIENT_H

#include "adapter.h"

#define SATIP_STR_LEN 500
#define SATIP_MAX_STRENGTH 255
#define SATIP_MAX_QUALITY 15

void find_satip_adapter(adapter **a);
int satip_getxml(void *);
char *init_satip_pointer(int len);
#endif
