#ifndef DISABLE_PMT
#ifndef PMT_H
#define PMT_H
#warning "compile with make DVBAPI=0 DVBCSA=0 DVBAES=0  DVBCA=0"
#include "dvb.h"
#include "adapter.h"

#define MAX_CAID 20
#define MAX_ACTIVE_PIDS 10
#define CA_ALGO_DVBCSA 0
#define CA_ALGO_DES 1
#define CA_ALGO_AES128 2
#define CA_ALGO_AES128_ECB 2
#define CA_ALGO_AES128_CBC 3

#define CA_MODE_ECB 0
#define CA_MODE_CBC 1

#define MAX_PMT 128
#define MAX_CW 20

#define MAX_FILTERS 200
#define FILTER_ADD_PID 0
#define FILTER_KEEP_PID 1
#define FILTER_PERMANENT 2
#define PID_FROM_TS(b) ((b[1] & 0x1F) * 256 + b[2])
#define MAX_PI_LEN 1500

typedef struct struct_batch // same as struct dvbcsa_bs_batch_s
{
	unsigned char *data; /* pointer to payload */
	unsigned int len;	/* payload bytes lenght */
} SPMT_batch;

typedef void *(*Create_cwkey)(void);
typedef void (*Delete_cwkey)(void *);
typedef int (*Batch_size)(void);
typedef void (*Set_CW)(unsigned char *cw, void *key);
typedef void (*Decrypt_Stream)(void *key, SPMT_batch *batch, int batch_len);

typedef int (*filter_function)(int filter, void *buf, int len, void *opaque);

typedef struct struct_pmt_op
{
	int algo;
	int mode;
	Create_cwkey create_cwkey;
	Delete_cwkey delete_cwkey;
	Batch_size batch_size;
	Set_CW set_cw;
	Decrypt_Stream decrypt_stream;
} SPMT_op;

typedef struct struct_internal_op
{
	char enabled;
	SPMT_op *op;
} _Spmt_op;

typedef struct struct_cw
{
	char enabled;
	char cw[16];
	uint64_t time;
	void *key;
	int algo;
	int8_t pmt;
	char prio; // CW priority
	int8_t op_id;
	char adapter;
	char parity;

} SCW;

typedef struct struct_pmt
{
	char enabled;
	SMutex mutex;
	int sid;
	int pid;
	int adapter;
	int caids;
	int version;
	uint16_t caid[MAX_CAID], cais_mask[MAX_CAID];
	uint32_t provid[MAX_CAID];
	int active_pid[MAX_ACTIVE_PIDS];
	int req_pid[MAX_ACTIVE_PIDS];
	int id;
	int ver;
	unsigned char *pmt;
	int pmt_len;
	unsigned char *pi;
	int pi_len;
	int blen;
	SPMT_batch batch[130];
	int8_t cw_id, parity, invalidated;
	int64_t last_parity_change;
	int16_t master_pmt; //  the pmt that contains the same pids as this PMT
	SCW *cw;			// cached
	SPMT_op *op;		// cached
	SPid *p;
	void *opaque;
} SPMT;

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

int register_algo(SPMT_op *o);
int send_cw(int pmt_id, int cw_type, int parity, uint8_t *cw);

static inline SPMT *get_pmt(int id)
{
	extern SPMT *pmts[];

	if (id < 0 || id >= MAX_PMT || !pmts[id] || !pmts[id]->enabled)
		return NULL;
	return pmts[id];
}

static inline SFilter *get_filter(int id)
{
	extern SFilter *filters[];
	return (id >= 0 && id <= MAX_FILTERS && filters[id] && filters[id]->enabled) ? filters[id] : NULL;
}

int pmt_enabled_channels(int id);
void pmt_pid_del(adapter *ad, int pid);
void pmt_pid_add(adapter *ad, int pid, int existing);
int pmt_init_device(adapter *ad); // will call action[CA_INIT_DEVICE] for the adapter if CA is registered for it in adapter_mask,
//if the call succeds, the PMTs will be sent to that CA for that adapter
int pmt_close_device(adapter *ad);
int pmt_init();
int pmt_destroy();
int pmt_init_device(adapter *ad);
int tables_tune(adapter *ad);

int add_filter(int aid, int pid, void *callback, void *opaque, int flags);
int add_filter_mask(int aid, int pid, void *callback, void *opaque, uint8_t *data, uint8_t *mask, int flags);
int del_filter(int id);
int change_filter_mask(int id, uint8_t *data, uint8_t *mask);
int get_pid_filter(int aid, int pid);

#endif
#endif
