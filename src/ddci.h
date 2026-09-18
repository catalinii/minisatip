#ifndef DDCI_H
#define DDCI_H
#include "adapter.h"
#include "pmt.h"
#include "tables.h"
#include "utils/fifo.h"
#include <sys/uio.h>
#include <unordered_map>
#include <unordered_set>

// Maximum number of PMTs (channels) supported per adapter
#define MAX_CHANNELS_ON_CI 8

#define DDCI_BUFFER (20000 * 188)

// buckets for the read-phase histogram: ms between a read from the CI device
// and the previous writev into it. The first bucket covers the smallest
// read-after-write turnaround the driver can produce
#define DDCI_INSTR_HIST_BUCKETS 6

// per CI-link pid continuity tracking used by --ddci-instrument.
// The same pid space (the remapped pids on the CI link) is tracked
// independently for the packets written to the CAM ([W]) and for the packets
// read back from the CAM ([R]), which allows localizing packet loss between
// minisatip, the kernel driver and the CAM
typedef struct ddci_instr {
    SMutex mutex;
    int8_t wcc[8192]; // last CC written to the CI, -1 = unknown
    int8_t rcc[8192]; // last CC read back from the CI, -1 = unknown
    uint32_t wcnt[8192], rcnt[8192]; // payload packets per pid
    uint32_t werr[8192], rerr[8192]; // continuity errors per pid
    int64_t last_read_tick, last_write_tick;
    int64_t since_prev_writev; // ms since previous writev, captured per read
    int64_t dump_start;
    int64_t max_read_gap, max_write_gap; // ms, windowed
    uint32_t reads, writevs, last_read_len;
    uint64_t read_bytes, write_bytes;     // cumulative
    uint64_t pushed_bytes, drained_bytes; // cumulative fifo in/out
    uint64_t fifo_push_fail, fifo_max_level;
    uint64_t prev_read_bytes, prev_write_bytes, prev_pushed, prev_drained;
    // read-phase histogram (ms since previous writev), windowed; errors are
    // attributed to the bucket of the read batch they were detected in
    uint32_t read_hist[DDCI_INSTR_HIST_BUCKETS];
    uint32_t read_err_hist[DDCI_INSTR_HIST_BUCKETS];
    // TS packets written to the CI since the previous read, windowed
    // min/avg/max, to detect reads driven by accumulated output
    uint32_t wpkts_since_read, wpkts_min, wpkts_max;
    uint64_t wpkts_sum, wpkts_reads;
} ddci_instr_t;

// keeps PMT informations for the channels that are enabled on this ddci_device
typedef struct ddci_pmt {
    int id;
    int ver;
    int pcr_pid;
    int16_t cc;
    uint32_t crc;
} ddci_pmt_t;

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
    std::unordered_set<int> pmt;
    int filter_id = -1;
    int pid_added;
} ddci_mapping_table_t;

typedef struct ddci_device {
    SMutex mutex;
    int enabled;
    int id;
    int fd;
    int channels;
    int max_channels;
    ddci_pmt_t pmt[MAX_CHANNELS_ON_CI + 1];
    int cat_processed;
    uint64_t read_index[MAX_ADAPTERS]; // read index per adapter
    uint64_t last_pat, last_sdt, last_pmt;
    int tid, ver;
    int16_t pat_cc, sdt_cc, eit_cc;
    char disable_cat;
    std::unordered_map<int, ddci_mapping_table_t> mapping;
    SFIFO fifo;
    ddci_instr_t *instr; // allocated only when --ddci-instrument is used
} ddci_device_t;

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
int ddci_process_cat(int filter, unsigned char *b, int len, void *opaque);
int ddci_create_pat(ddci_device_t *d, uint8_t *b);
int ddci_create_sdt(ddci_device_t *d, uint8_t *b);
int ddci_create_pmt(ddci_device_t *d, SPMT *pmt, uint8_t *new_pmt, int pmt_size,
                    ddci_pmt_t *dp);
ddci_mapping_table_t *get_pid_mapping_allddci(int ad, int pid);
void save_channels();
void load_channels();
int ddci_process_pmt(adapter *ad, SPMT *pmt);
void blacklist_pmt_for_ddci(SPMT *pmt, int ddid);
int ddci_del_pmt(adapter *ad, SPMT *spmt);
void disable_cat_adapters(char *o);
void dump_mapping_table();
ddci_instr_t *ddci_instr_alloc();
void ddci_instr_free(ddci_instr_t *in);
void ddci_instrument_read(ddci_device_t *d, uint8_t *b, int len);
void ddci_instrument_write(ddci_device_t *d, struct iovec *io, int iop);
#endif
