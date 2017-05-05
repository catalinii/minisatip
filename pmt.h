#ifndef DISABLE_TABLES
#ifndef PMT_H
#define PMT_H

#include "dvb.h"

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

typedef struct struct_batch  // same as struct dvbcsa_bs_batch_s
{
	unsigned char         *data;   /* pointer to payload */
	unsigned int len;              /* payload bytes lenght */
} SPMT_batch;


typedef void *(*Create_cwkey)(void );
typedef void (*Delete_cwkey)(void *);
typedef int (*Batch_size)(void);
typedef void (*Set_CW)(unsigned char *cw,void *key);
typedef void (*Decrypt_Stream)(void *key, SPMT_batch *batch, int batch_len);


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
	int8_t cw_id, parity;
	int64_t last_parity_change;
	int16_t master_pmt;
	SPid *p;
	void *opaque;
} SPMT;

int register_algo(SPMT_op *o);
int send_cw(int pmt_id, int type, int parity, uint8_t *cw);
SPMT *get_pmt(int id);
int pmt_enabled_channels(int id);

#endif
#endif
