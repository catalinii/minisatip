#ifndef DISABLE_PMT
#ifndef PMT_H
#define PMT_H
#include "dvb.h"
#include "adapter.h"

#define MAX_CAID 20
#define MAX_ACTIVE_PIDS 20
#define MAX_PMT_PIDS (2 * MAX_ACTIVE_PIDS)
#define CA_ALGO_DVBCSA 0
#define CA_ALGO_DES 1
#define CA_ALGO_AES128 2
#define CA_ALGO_AES128_ECB 2
#define CA_ALGO_AES128_CBC 3

#define CA_MODE_ECB 0
#define CA_MODE_CBC 1

#define MAX_PMT 256
#define MAX_CW 80
#define MAX_CW_TIME 25000 // 25s
#define MAX_BATCH_SIZE 128

#define FILTER_SIZE 16 // based on DMX_FILTER_SIZE
#define FILTER_PACKET_SIZE 4000

#define MAX_FILTERS 200
#define FILTER_ADD_REMOVE 1
#define FILTER_PERMANENT 4
#define FILTER_REVERSE 8

#define FILTER_CRC 16
#define FILTER_EMM 32

#define MAX_PI_LEN 1500

typedef struct struct_batch // same as struct dvbcsa_bs_batch_s
{
	unsigned char *data; /* pointer to payload */
	unsigned int len;	/* payload bytes lenght */
} SPMT_batch;

typedef void (*Create_CW)(void *);
typedef void (*Delete_CW)(void *);
typedef int (*Batch_size)(void);
typedef void (*Set_CW)(void *cw, void *pmt);
typedef void (*Decrypt_Stream)(void *cw, SPMT_batch *batch, int batch_len);

typedef int (*filter_function)(int filter, void *buf, int len, void *opaque);

typedef struct struct_pmt_op
{
	int algo;
	Create_CW create_cw;
	Delete_CW delete_cw;
	Batch_size batch_size;
	Set_CW set_cw, stop_cw;
	Decrypt_Stream decrypt_stream;
} SCW_op;

typedef struct struct_internal_op
{
	char enabled;
	SCW_op *op;
} _SCW_op;

typedef struct struct_cw
{
	char enabled;
	unsigned char cw[32], iv[32];
	int64_t time;
	void *key;
	int algo;
	int16_t pmt;
	SCW_op *op;
	char adapter;
	char parity;
	char cw_len;
	int prio;
	char low_prio;
	int16_t id;
	int64_t expiry, set_time;

} SCW;

typedef struct struct_pmt_pid
{
	int type;
	int pid;
} SPMTPid;

typedef struct struct_pmt
{
	char enabled;
	SMutex mutex;
	int sid;
	int pid;
	int adapter;
	int version;
	uint16_t caid[MAX_CAID], capid[MAX_CAID];
	uint16_t caids;
	int active_pid[MAX_ACTIVE_PIDS];
	int active_pids;
	SPMTPid stream_pid[MAX_PMT_PIDS];
	int stream_pids;
	int id;
	unsigned char pmt[MAX_PI_LEN];
	int pmt_len;
	unsigned char pi[MAX_PI_LEN];
	int pi_len;
	int blen;
	int ca_mask, disabled_ca_mask;
	SPMT_batch batch[MAX_BATCH_SIZE + 2];
	int8_t parity, invalidated;
	int64_t last_parity_change;
	int16_t master_pmt; //  the pmt that contains the same pids as this PMT
	SCW *cw;
	SPid *p;
	char provider[50], name[50];
	void *opaque;
	char skip_first;
	char active;  // PMT structure was already filled
	char running; // PMT has channels running
	char encrypted;
	int first_active_pid;
	int64_t grace_time;
	uint16_t filter;
	int clean_pos, clean_cc;
	uint8_t *clean;
} SPMT;

// filters can be setup for specific pids and masks
// it calls assemble_packet to reassemble the packets and then calls the callback function
// flags - 0 - does not add the pid
// flags - FILTER_ADD_REMOVE - adds the pids if there is at least one filter with this type and removes the pid from the list of pids, if there is no filter
// flags - FITLER_PERMANENT - keeps the filter active after changing the frequency and does not add the pid to the list of pids (pid 0 is permanent)
// PAT, PMT, CAT, ... can be processed also using filters
// another use cases: ECM EMM
// could be extended at later time with timeout or close callbacks

typedef struct struct_filter
{
	char enabled;
	SMutex mutex;
	int id;
	int pid;
	int adapter;
	void *opaque;
	int len, mask_len;
	char match;
	filter_function callback;
	unsigned char filter[FILTER_SIZE];
	unsigned char mask[FILTER_SIZE];
	unsigned char data[FILTER_PACKET_SIZE + 2];
	int flags;
	int next_filter, master_filter;
} SFilter;

int register_algo(SCW_op *o);
int send_cw(int pmt_id, int algo, int parity, uint8_t *cw, uint8_t *iv, int64_t expiry);

extern int npmts;
static inline SPMT *get_pmt(int id)
{
	extern SPMT *pmts[];

	if (id < 0 || id >= npmts || !pmts[id] || !pmts[id]->enabled)
		//		LOG_AND_RETURN(NULL, "PMT not found for id %d", id);
		return NULL;
	return pmts[id];
}

static inline SFilter *get_filter(int id)
{
	extern SFilter *filters[];
	return (id >= 0 && id < MAX_FILTERS && filters[id] && filters[id]->enabled) ? filters[id] : NULL;
}
int process_pmt(int filter, unsigned char *b, int len, void *opaque);
void pmt_pid_del(adapter *ad, int pid);
void pmt_pid_add(adapter *ad, int pid, int existing);
int pmt_init_device(adapter *ad); // will call action[CA_INIT_DEVICE] for the adapter if CA is registered for it in adapter_mask,
//if the call succeds, the PMTs will be sent to that CA for that adapter
int pmt_close_device(adapter *ad);
int pmt_init();
int pmt_destroy();
void start_pmt(SPMT *pmt, adapter *ad);
int pmt_init_device(adapter *ad);
int tables_tune(adapter *ad);
int delete_pmt_for_adapter(int aid);
int pmt_tune(adapter *ad);
int get_active_filters_for_pid(int master_filter, int aid, int pid, int flags);
int add_filter(int aid, int pid, void *callback, void *opaque, int flags);
int add_filter_mask(int aid, int pid, void *callback, void *opaque, int flags, uint8_t *filter, uint8_t *mask);
int del_filter(int id);
int set_filter_mask(int id, uint8_t *filter, uint8_t *mask);
int set_filter_flags(int id, int flags);
int get_pid_filter(int aid, int pid);
int get_filter_pid(int filter);
int get_filter_adapter(int filter);
int assemble_packet(SFilter *f, uint8_t *b);
int clean_psi_buffer(uint8_t *pmt, uint8_t *clean, int clean_size);
void disable_cw(int master_pmt);
void expire_cw_for_pmt(int master_pmt, int parity, int64_t min_expiry);
#endif
#endif
