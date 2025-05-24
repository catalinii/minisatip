#ifndef DDCI_H
#define DDCI_H
#include "adapter.h"
#include "pmt.h"
#include "tables.h"
#include "utils/hash_table.h"
#include "utils/fifo.h"

// Maximum number of PMTs (channels) supported per adapter
#define MAX_CHANNELS_ON_CI 8
// Maximum number of CA PIDs supported per adapters
#define MAX_CA_PIDS 64

#define DDCI_BUFFER (20000 * 188)

// keeps PMT informations for the channels that are enabled on this ddci_device
typedef struct ddci_pmt {
    int id;
    int ver;
    int pcr_pid;
    int16_t cc;
    uint32_t crc;
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
    int capid[MAX_CA_PIDS];
    uint64_t read_index[MAX_ADAPTERS]; // read index per adapter
    uint64_t last_pat, last_sdt, last_pmt;
    int tid, ver;
    int16_t pat_cc, sdt_cc, eit_cc;
    char disable_cat;
    SHashTable mapping;
    SFIFO fifo;
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
    } ddci[MAX_ADAPTERS];
    int ddcis;
    int sid;
    char name[50];
    char locked;
} Sddci_channel;

void find_ddci_adapter(adapter **a);
void ddci_init();
int add_pid_mapping_table(int ad, int pid, int pmt, ddci_device_t *d,
                          int force_add_pid);
int push_ts_to_adapter(ddci_device_t *d, adapter *ad, uint16_t *mapping);
void set_pid_ts(unsigned char *b, int pid);
int ddci_process_ts(adapter *ad, ddci_device_t *d);
int ddci_create_pat(ddci_device_t *d, uint8_t *b);
int ddci_create_sdt(ddci_device_t *d, uint8_t *b);
int ddci_create_pmt(ddci_device_t *d, SPMT *pmt, uint8_t *new_pmt, int pmt_size,
                    ddci_pmt_t *dp);
ddci_mapping_table_t *get_pid_mapping_allddci(int ad, int pid);
void save_channels(SHashTable *ch);
void load_channels(SHashTable *ch);
int ddci_process_pmt(adapter *ad, SPMT *pmt);
void blacklist_pmt_for_ddci(SPMT *pmt, int ddid);
int ddci_del_pmt(adapter *ad, SPMT *spmt);
void disable_cat_adapters(char *o);
#endif
