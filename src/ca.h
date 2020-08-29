#ifndef CA_H
#define CA_H
#include "adapter.h"
#include "tables.h"
int ca_init(ca_device_t *d);
void dvbca_init();
int createCAPMT(uint8_t *b, int len, int listmgmt, uint8_t *capmt, int capmt_len, int reason);
int is_ca_initialized(int i);
void set_ca_adapter_pin(char *o);
void set_ca_adapter_force_ci(char *o);
char *get_ca_pin(int i);
#endif
