#ifndef DDCI_H
#define DDCI_H
#include "adapter.h"
#include "tables.h"

// number of pids for each ddci adapter to be stored in the mapping table
#define MAX_CHANNELS_ON_CI 4
#define PIDS_FOR_ADAPTER 128
#define MAX_CA_PIDS 20

#define DDCI_BUFFER (100000 * 188)
typedef struct ddci_device
{
	SMutex mutex;
	int enabled;
	int id;
	int fd;
	int pid_mapping[8192];
	int channels;
	int max_channels;
	int pmt[MAX_CHANNELS_ON_CI + 1];
	int pmt_ver[MAX_CHANNELS_ON_CI + 1];
	int cat_processed;
	int capid[MAX_CA_PIDS + 1];
	int ncapid;
	unsigned char *out;
	int ro, wo;
	uint64_t last_pat, last_pmt;
	int tid, ver;
	char pat_cc, pmt_cc[MAX_CHANNELS_ON_CI + 1];
} ddci_device_t;

void find_ddci_adapter(adapter **a);
void ddci_init();
int add_pid_mapping_table(int ad, int pid, int pmt, int ddci_adapter, int force_add_pid);
int push_ts_to_ddci_buffer(ddci_device_t *d, unsigned char *b, int rlen);
int copy_ts_from_ddci_buffer(adapter *ad, ddci_device_t *d, unsigned char *b, int *ad_pos);
void set_pid_ts(unsigned char *b, int pid);
int ddci_process_ts(adapter *ad, ddci_device_t *d);
int ddci_create_pat(ddci_device_t *d, uint8_t *b);
int ddci_create_pmt(ddci_device_t *d, SPMT *pmt, uint8_t *clean, int ver);

#endif
