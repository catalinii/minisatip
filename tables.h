#if !defined(DISABLE_DVBCA) || !defined(DISABLE_DVBAPI)

#ifndef TABLES_H
#define TABLES_H

#define TABLES_ITEM 0x2000000000000

#include "adapter.h"
#include <linux/dvb/dmx.h>

#define MAX_CA 4

#define CA_INIT_DEVICE 0
#define CA_CLOSE_DEVICE 1
#define CA_ADD_PID 2
#define CA_DEL_PID 3
#define CA_ADD_PMT 4
#define CA_DEL_PMT 5
#define CA_ECM 6
#define CA_TS 7
#define CA_CLOSE 8

typedef int (*ca_action)(adapter *ad, void *arg);

typedef struct struct_CA
{
	uint8_t enabled;
	ca_action action[9];
	int adapter_mask; // 1 << x, means enabled for adapter X
	int id;
} SCA;
int add_ca(SCA *c);
void del_ca(SCA *c);
void init_ca_device(SCA *c);					 //  calls table_init_device for all the devices
int register_ca_for_adapter(int nca, int aid);   // register a CA just for a device, and run tables_init_device for the CA and Adapter, if succeds, send all the opened PMTs
int unregister_ca_for_adapter(int nca, int aid); // unregister a CA just for a device
int tables_init_device(adapter *ad);
int run_ca_action(int action_id, adapter *ad, void *arg);
int tables_close_device(adapter *ad);

#endif

#endif
