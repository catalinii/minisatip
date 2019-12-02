#ifndef DISABLE_DVBAPI
#ifndef DVBAPI_H
#define DVBAPI_H

#include "adapter.h"
#include "stream.h"
#include "pmt.h"

#define DVBAPI_PROTOCOL_VERSION 2

#define DVBAPI_FILTER_DATA 0xFFFF0000
#define DVBAPI_CLIENT_INFO 0xFFFF0001
#define DVBAPI_SERVER_INFO 0xFFFF0002
#define DVBAPI_DMX_SET_FILTER 0x403c6f2b
#define DVBAPI_CA_SET_PID 0x40086f87
#define DVBAPI_CA_SET_DESCR 0x40106f86
#define DVBAPI_DMX_STOP 0x00006f2a
#define CA_SET_DESCR_X 0x866f10
#define CA_SET_DESCR_AES 0x40106f87
#define CA_SET_DESCR_AES_X 0x876f10
#define CA_SET_PID_X 0x876f08
#define DMX_STOP_X 0x2a6f00
#define DMX_SET_FILTER_X 0x2b6f3c
#define DVBAPI_ECM_INFO 0xFFFF0003
#define CA_SET_DESCR_MODE 0x400c6f88

#define AOT_CA_PMT 0x9F803282

#define CAPMT_LIST_MORE 0x00   // append a 'MORE' CAPMT object the list and start receiving the next object
#define CAPMT_LIST_FIRST 0x01  // clear the list when a 'FIRST' CAPMT object is received, and start receiving the next object
#define CAPMT_LIST_LAST 0x02   // append a 'LAST' CAPMT object to the list and start working with the list
#define CAPMT_LIST_ONLY 0x03   // clear the list when an 'ONLY' CAPMT object is received, and start working with the object
#define CAPMT_LIST_ADD 0x04	// append an 'ADD' CAPMT object to the current list and start working with the updated list
#define CAPMT_LIST_UPDATE 0x05 // replace an entry in the list with an 'UPDATE' CAPMT object, and start working with the updated list

#define MAX_KEYS 255
#define MAX_PMT_DATA 1880
#define MAX_KEY_FILTERS 20
typedef struct struct_key
{
	char enabled;
	SMutex mutex;
	int pmt_id;
	int algo;
	unsigned char cw[2][16];
	uint32_t cw_time[2];
	int key_len;
	int sid;
	int pmt_pid;
	int64_t last_ecm, last_dmx_stop;
	uint8_t hops;
	uint16_t caid, info_pid;
	uint32_t prid;
	int ecmtime;
	int demux_index;
	int id;
	int adapter;
	int ver;
	int ecms;
	int program_id; // pmt sid
	unsigned char *pi, cardsystem[64], reader[64], from[64], protocol[64];
	int pi_len;
	int parity;
	int blen;
	int tsid, onid;
	int filter_id[MAX_KEY_FILTERS], filter[MAX_KEY_FILTERS], demux[MAX_KEY_FILTERS], pid[MAX_KEY_FILTERS], ecm_parity[MAX_KEY_FILTERS];
	int64_t last_parity_change;
} SKey;

void init_dvbapi();
int have_dvbapi();
int dvbapi_enabled();
int send_ecm(int filter_id, unsigned char *b, int len, void *opaque);
int batch_size();
int decrypt_stream(adapter *ad, void *arg);
int keys_add(int i, int adapter, int pmt_id);
int keys_del(int i);
int dvbapi_process_pmt(unsigned char *b, adapter *ad);
void dvbapi_pid_add(adapter *a, int pid, SPid *cp, int existing);
void dvbapi_pid_del(adapter *a, int pid, SPid *cp);
void dvbapi_delete_keys_for_adapter(int aid);
void register_dvbapi();
void unregister_dvbapi();
void send_client_info(sockets *s);
int set_algo(SKey *k, int algo, int mode);

#endif
#endif
