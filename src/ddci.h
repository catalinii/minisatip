#ifndef DDCI_H
#define DDCI_H
#include "adapter.h"
#include "pmt.h"
#include "tables.h"
#include "utils/fifo.h"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <sys/uio.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Maximum number of PMTs (channels) supported per adapter
#define MAX_CHANNELS_ON_CI 8

#define DDCI_BUFFER (20000 * 188)

// Cap for unadmitted push bytes; oldest drop past it.
#define DDCI_PUSH_BACKLOG_MAX (3 * 1024 * 1024)

// Max bytes pushed per call; bigger bursts wedge this driver.
#define DDCI_PUSH_CHUNK (32 * 1024)

// Nothing admitted for this long: the fd latched, reopen it.
#define DDCI_PUSH_STALL_MS 5000

// Max admitted-but-unconsumed push bytes; the rest waits userside.
// Quarter-meg below the 1.5M brim that freezes the demux.
#define DDCI_PUSH_INFLIGHT_MAX (1280 * 1024)

// Mapping-table key: adapter in the high 16 bits, pid below.
#define MAKE_KEY(ad, pid) (((ad) << 16) | (pid))

// keeps PMT informations for the channels that are enabled on this ddci_device
typedef struct ddci_pmt {
    int id;
    int ver;
    int pcr_pid;
    int16_t cc;
    uint32_t crc;
    // Per-channel PMT emission clock (ms); 0 emits on the next tick.
    uint64_t last_pmt;
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
    int pid_added = -1;
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
    uint64_t last_pat, last_sdt;
    bool psi_changed; // channel set changed: send PAT/SDT on the next write
    int tid, ver;
    // SDT section version, split from the PAT ver so SDT-only changes
    // (transponder data arriving late) do not re-version the PAT.
    int sdt_ver;
    int sdt_data_mask; // channels with real SDT data at last SDT generation
    int16_t pat_cc, sdt_cc, eit_cc;
    char disable_cat;
    char loopback;              // enigma DVR loopback, no sec/ci hardware
    int ci_slot;                // claimed /dev/ciN, -1 until bound
    SFIFO push_backlog;         // unadmitted push bytes, oldest first
    size_t push_backlog_cap;    // max backlog bytes, 0 selects default
    int64_t push_admitted_tick; // last push admission (ms)
    // Admitted vs read-back push bytes; the gap caps in flight.
    uint64_t push_admitted_total;
    uint64_t push_consumed_total;
    char push_reopen_blocked; // refused reopen retry: wait for admission
    // E2-style push: backlog feeds a blocking writer, never polled.
    std::thread push_thread;
    std::mutex push_mutex;
    std::condition_variable push_cv;
    std::atomic<bool> push_stop{false};
    std::atomic<bool> push_running{false};
    std::atomic<bool> push_reopen{false};
    uint64_t push_shed_total = 0; // backlog-overflow drops, oldest first
    int desc_fd = -1; // demux-linked ca device for HW keys, E2 style
    std::unordered_map<int, ddci_mapping_table_t> mapping;
    std::unordered_set<int> emm_pids; // MAKE_KEY(ad,pid), never swept
    SFIFO fifo;
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

void find_ddci_adapter();
void ddci_init();
// Last demux/dvr pair with no frontend and no owner; -1 when none.
int ddci_pick_free_pair(const int *demux, int ndemux, const int *dvr, int ndvr,
                        const int *fe, int nfe, const int *used, int nused);

int ddci_pick_free_slot();
// Blocking push loop: drains the backlog to the DVR fd.
void ddci_push_loop(int id);
// Stops the push thread after draining; safe to call twice.
void ddci_push_stop(ddci_device_t *d);
// Queues fresh PSI first, then TS; drops oldest past the cap.
size_t ddci_push_queue(ddci_device_t *d, uint8_t *psi, size_t psi_len,
                       struct iovec *io, int iop);
// E2 descrambler path: HW key/IV and pid routing on ca{fn}.
void ddci_descrambler_key(int id, int slot, uint8_t *key, uint8_t *iv);
void ddci_descrambler_pid(int id, int ddci_pid, int enable);
// True when id is an enabled enigma loopback device.
int ddci_is_loopback(int id);
// True when any enigma loopback claimed this CI slot.
int ddci_loopback_owns_slot(int slot);
// CI slot claimed by loopback id; -1 when it claims none.
int ddci_loopback_slot_for(int id);
void ddci_enigma_route_pvr(ddci_device_t *d);
void ddci_enigma_route_none(ddci_device_t *d);
int add_pid_mapping_table(int ad, int pid, int pmt, ddci_device_t *d,
                          int force_add_pid);
int push_ts_to_adapter(ddci_device_t *d, adapter *ad, uint16_t *mapping);
void set_pid_ts(unsigned char *b, int pid);
int ddci_process_ts(adapter *ad, ddci_device_t *d);
int ddci_process_cat(int filter, unsigned char *b, int len, void *opaque);
int ddci_create_pat(ddci_device_t *d, uint8_t *b);
int ddci_add_psi(ddci_device_t *d, uint8_t *dst, int len, adapter *ad);
int ddci_create_sdt(ddci_device_t *d, uint8_t *b);
int ddci_create_pmt(ddci_device_t *d, SPMT *pmt, uint8_t *new_pmt, int pmt_size,
                    ddci_pmt_t *dp);
ddci_mapping_table_t *get_pid_mapping_allddci(int ad, int pid);
void save_channels();
void load_channels();
int ddci_process_pmt(adapter *ad, SPMT *pmt, int update);
void blacklist_pmt_for_ddci(SPMT *pmt, int ddid);
int ddci_del_pmt(adapter *ad, SPMT *spmt);
void disable_cat_adapters(char *o);
void dump_mapping_table();
#endif
