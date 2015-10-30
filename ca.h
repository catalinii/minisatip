#ifndef CA_H
#define CA_H
#include "adapter.h"
#include "tables.h"
ca_device_t * ca_init(int fd);
void ca_grab_pmt(adapter *a, int rlen);
int dvbca_process_pmt(uint8_t *b, adapter *ad);
void dvbca_init();

#endif
