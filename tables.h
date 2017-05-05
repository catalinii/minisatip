#if !defined(DISABLE_DVBCA) || !defined(DISABLE_DVBAPI)

#ifndef TABLES_H
#define TABLES_H

#define TABLES_ITEM 0x2000000000000

#include "adapter.h"
#include <linux/dvb/dmx.h>

#define PID_FROM_TS(b) ((b[1] & 0x1F)*256 + b[2])
#define MAX_PI_LEN 1500

typedef int (*ca_action)(adapter *ad, void *arg);
typedef int (*filter_function)(int filter, void *buf, int len, void *opaque);

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

#define MAX_FILTERS 200

typedef struct struct_CA
{
	uint8_t enabled;
	ca_action action[9];
	int adapter_mask;  // 1 << x, means enabled for adapter X
	int id;
} SCA;

SMutex filter_mutex;

#define FILTER_ADD_PID 0
#define FILTER_KEEP_PID 1
#define FILTER_PERMANENT 2

typedef struct struct_filter
{
	char enabled;
	SMutex mutex;
	int id;
	int pid;
	int flags;
	int adapter;
	void *opaque;
	filter_function callback;
	unsigned char filter[DMX_FILTER_SIZE];
	unsigned char mask[DMX_FILTER_SIZE];
	unsigned char data[1500];
	int next_filter;
} SFilter;


int add_ca (SCA *c);
void del_ca(SCA *c);
SFilter *get_filter(int id);
int process_pat(int filter, unsigned char *b, int len, void *opaque);
int process_pmt(int filter, unsigned char *b, int len, void *opaque);
int assemble_packet(uint8_t **b1, adapter *ad, int check_crc);
uint32_t crc_32(const uint8_t *data, int datalen);
void tables_pid_del(adapter *ad, int pid);
void tables_pid_add(adapter *ad, int pid, int existing);
void clean_psi(adapter *ad, uint8_t *b);
int tables_init_device(adapter *ad); // will call action[CA_INIT_DEVICE] for the adapter if CA is registered for it in adapter_mask,
//if the call succeds, the PMTs will be sent to that CA for that adapter
int tables_close_device(adapter *ad);
int tables_init();
int tables_destroy();
void init_ca_device(SCA *c); //  calls table_init_device for all the devices
int register_ca_for_adapter(int nca, int aid);  // register a CA just for a device, and run tables_init_device for the CA and Adapter, if succeds, send all the opened PMTs
int unregister_ca_for_adapter(int nca, int aid); // unregister a CA just for a device
int tables_tune(adapter *ad);
int add_filter(int aid, int pid, void *callback, void *opaque, int flags);
int add_filter_mask(int aid, int pid, void * callback, void *opaque, uint8_t *data, uint8_t *mask, int flags);
int del_filter(int id);
int change_filter_mask(int id,uint8_t *data, uint8_t *mask);
int get_pid_filter(int aid, int pid);
#endif

#endif
