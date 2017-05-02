#ifndef SATIPCLIENT_H
#define SATIPCLIENT_H

#include "adapter.h"
#define SATIP_STR_LEN 500

void find_satip_adapter(adapter **a);
int satip_getxml(void *);
char *init_satip_pointer(int len);
#endif
