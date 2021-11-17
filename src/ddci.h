#ifndef DDCI_H
#define DDCI_H
#include "adapter.h"
#include "pmt.h"
#include "tables.h"

// number of pids for each ddci adapter to be stored in the mapping table
#define MAX_CHANNELS_ON_CI 4
#define PIDS_FOR_ADAPTER 128
#define MAX_CA_PIDS 20

#define DDCI_BUFFER (100000 * 188)

// keeps PMT informations for the channels that are enabled on this ddci_device
typedef struct ddci_pmt {
    int id;
    int ver;
    int pcr_pid;
    char cc;
} ddci_pmt_t;

typedef struct ddci_device {
    SMutex mutex;
    int enabled;
    int id;
    int fd;
    int channels;
    int max_channels;
    ddci_pmt_t pmt[MAX_CHANNELS_ON_CI + 1];
    int cat_processed;
    int capid[MAX_CA_PIDS + 1];
    int ncapid;
    unsigned char *out;
    int ro[MAX_ADAPTERS], wo; // read offset per adapter
    uint64_t last_pat, last_pmt;
    int tid, ver;
    char pat_cc;
    SHashTable mapping;
} ddci_device_t;

// Use a hash table to store mapping_table entries
// the key is ad << 16 | pid to be able to search faster
// based on this key. This key is needed mostly for processing CAT, PMT, PAT
// to map from pids in original tables to destination ones.
typedef struct ddci_mapping_table {
    int ad;
    int pid;
    int ddci_pid;
    int ddci;
    char rewrite;
    int pmt[MAX_CHANNELS_ON_CI + 1];
    int npmt;
    int filter_id;
    int pid_added;
} ddci_mapping_table_t;

typedef struct ddci_channel {
    struct SDDCI {
        uint8_t ddci;
        uint64_t blacklisted_until;
    } ddci[MAX_ADAPTERS];
    int ddcis;
    int pos;
    int sid;
    char name[50];
    char locked;
} Sddci_channel;

void find_ddci_adapter(adapter **a);
void ddci_init();
int add_pid_mapping_table(int ad, int pid, int pmt, ddci_device_t *d,
                          int force_add_pid);
int push_ts_to_adapter(adapter *ad, unsigned char *b, int new_pid, int *ad_pos);
int push_ts_to_ddci_buffer(ddci_device_t *d, unsigned char *b, int len);
void set_pid_ts(unsigned char *b, int pid);
int ddci_process_ts(adapter *ad, ddci_device_t *d);
int ddci_create_pat(ddci_device_t *d, uint8_t *b);
int ddci_create_pmt(ddci_device_t *d, SPMT *pmt, uint8_t *new_pmt, int pmt_size,
                    int ver, int pcr_pid);
ddci_mapping_table_t *get_pid_mapping_allddci(int ad, int pid);
void save_channels(SHashTable *ch, char *file);
void load_channels(SHashTable *ch, char *file);
int ddci_process_pmt(adapter *ad, SPMT *pmt);
void blacklist_pmt_for_ddci(SPMT *pmt, int ddid);
int ddci_del_pmt(adapter *ad, SPMT *spmt);
#endif
