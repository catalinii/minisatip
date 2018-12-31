#ifndef CA_H
#define CA_H
#include "adapter.h"
#include "tables.h"
int ca_init(ca_device_t *d);
void dvbca_init();
int createCAPMT(uint8_t *b, int len, int listmgmt, uint8_t *capmt, int capmt_len);
#endif
