#ifndef CA_H
#define CA_H
#include "adapter.h"
#include "tables.h"
int ca_init(ca_device_t *d);
void ca_grab_pmt(adapter *a, int rlen);
void dvbca_init();

#endif
