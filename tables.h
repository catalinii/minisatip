#if !defined(DISABLE_DVBCA) || !defined(DISABLE_DVBAPI)

#ifndef TABLES_H
#define TABLES_H

#define TABLES_ITEM 0x2000000000000

#include "adapter.h"
#include "pmt.h"

#define MAX_CA 8
#define MAX_CAID_LEN 10

#define TABLES_RESULT_OK 0
#define TABLES_RESULT_ERROR_RETRY 1
#define TABLES_RESULT_ERROR_NORETRY 2

typedef int (*ca_pmt_action)(adapter *ad, SPMT *pmt);
typedef int (*ca_pid_action)(adapter *ad, SPMT *pmt, int pid);
typedef int (*ca_device_action)(adapter *ad);
typedef int (*ca_close_action)();

typedef struct struct_CA_op
{
	ca_pid_action ca_add_pid, ca_del_pid;
	ca_pmt_action ca_add_pmt, ca_del_pmt;
	ca_device_action ca_init_dev, ca_close_dev, ca_ts;
	ca_close_action ca_close_ca;
} SCA_op;

typedef struct struct_CA
{
	uint8_t enabled;
	SCA_op *op;
	int adapter_mask; // 1 << x, means enabled for adapter X
	int id;
	int caid[MAX_CAID_LEN], mask[MAX_CAID_LEN];
	int caids;
} SCA;
int add_ca(SCA_op *c, int adapter_mask);
void del_ca(SCA_op *c);
void add_caid_mask(int ca, int caid, int mask);
void init_ca_device(SCA *c);					 //  calls table_init_device for all the devices
int register_ca_for_adapter(int nca, int aid);   // register a CA just for a device, and run tables_init_device for the CA and Adapter, if succeds, send all the opened PMTs
int unregister_ca_for_adapter(int nca, int aid); // unregister a CA just for a device
int tables_init_device(adapter *ad);
int tables_close_device(adapter *ad);
int tables_init();
int tables_destroy();
void tables_add_pid(adapter *ad, SPMT *pmt, int pid);
void tables_del_pid(adapter *ad, SPMT *pmt, int pid);
int send_pmt_to_cas(adapter *ad, SPMT *pmt);
void close_pmt_for_ca(int i, adapter *ad, SPMT *pmt);
int close_pmt_for_cas(adapter *ad, SPMT *pmt);
void tables_ca_ts(adapter *ad);

#endif

#endif
