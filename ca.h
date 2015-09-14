#ifndef CA_H
#define CA_H

#ifdef DISABLE_DVBCA 
#define ca_init(a) 0
#define ca_grab_pmt(a,b) 0
#else

#include "adapter.h"
ca_device_t * ca_init(int fd);
void ca_grab_pmt(adapter *a, int rlen);
int dvbca_process_pmt(uint8_t *b, adapter *ad);

#endif
#endif
