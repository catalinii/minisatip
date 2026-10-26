#ifndef PMT_H
#define PMT_H
#include "adapter.h"
#include "dvb.h"
#include <atomic>
#include <unordered_map>
#include <vector>

#define MAX_CAID 20
#define MAX_ACTIVE_PIDS 20
#define CA_ALGO_DVBCSA 0
#define CA_ALGO_DES 1
#define CA_ALGO_AES128_ECB 2
#define CA_ALGO_AES128_CBC 3
#define CA_ALGO_DVBCSA_ICAM 4

#define MAX_PMT 25600
#define MAX_CW 200
#define MAX_CW_TIME 45000 // 45s
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

#define PMT_STOPPED 0
#define PMT_STARTING 1
#define PMT_RUNNING 2
#define PMT_STOPPING 3
#define PMT_CACHED 4

#define CLM_MORE 0x00
#define CLM_FIRST 0x01
#define CLM_LAST 0x02
#define CLM_ONLY 0x03
#define CLM_ADD 0x04
#define CLM_UPDATE 0x05

#define CMD_ID_OK_DESCRAMBLING 0x01
#define CMD_ID_OK_MMI 0x02
#define CMD_ID_QUERY 0x03
#define CMD_ID_NOT_SELECTED 0x04

typedef struct struct_batch // same as struct dvbcsa_bs_batch_s
{
    unsigned char *data; /* pointer to payload */
    unsigned int len;    /* payload bytes lenght */
} SPMT_batch;

typedef void (*Create_CW)(void *);
typedef void (*Delete_CW)(void *);
typedef int (*Batch_size)(void);
typedef void (*Set_CW)(void *cw, void *pmt);
typedef void (*Decrypt_Stream)(void *cw, SPMT_batch *batch, int len);

typedef int (*filter_function)(int filter, void *buf, int len, void *opaque);

typedef struct struct_pmt_op {
    int algo;
    Create_CW create_cw;
    Delete_CW delete_cw;
    Set_CW set_cw, stop_cw;
    Decrypt_Stream decrypt_stream;
} SCW_op;

typedef struct struct_internal_op {
    char enabled;
    SCW_op *op;
} _SCW_op;

typedef struct struct_cw {
    char enabled;
    unsigned char cw[32], iv[32];
    int64_t time, expiry;
    void *key;
    int algo;
    int16_t pmt;
    SCW_op *op;
    char adapter;
    char parity;
    char cw_len;
    int16_t id;
    int64_t set_time;
    void *opaque;
} SCW;

typedef struct descriptor {
    uint8_t type;
    uint8_t len;
    std::vector<uint8_t> data;

    bool operator==(const struct descriptor &other) const {
        return this->type == other.type && this->len == other.len &&
               this->data == other.data;
    };

    bool is_ca_descriptor() const { return this->type == 0x09; }

    uint16_t get_ca_descriptor_caid() const {
        return (data[0] * 256) + data[1];
    }

    uint16_t get_ca_descriptor_capid() const {
        return (data[2] & 0x1F) * 256 + data[3];
    }
} descriptor_t;

typedef struct struct_stream_pid {
    int type;
    int pid;
    bool is_audio;
    bool is_video;
    std::vector<descriptor_t> descriptors;
} SStreamPid;

typedef struct struct_pmt_ca {
    int id;
    int pid;
    std::vector<uint8_t> private_data;
} SPMTCA;

typedef struct struct_pmt {
    std::atomic<bool> enabled{false};
    std::atomic<bool> allocated{false};
    SMutex mutex;
    int sid;
    int pid;
    int pcr_pid;
    int adapter;
    int version;
    uint16_t caids;
    SPMTCA *ca[MAX_CAID];
    std::vector<descriptor_t> descriptors;
    std::vector<SStreamPid> stream_pids;
    int id;
    int blen;
    // ca_mask: the PMT was sent to that CA. pmt_add_caid() clears it to force
    // a re-send when a new CA descriptor shows up.
    //
    // ca_registered_mask: that CA holds the PMT and must be told when it
    // stops. Cleared when the registration, CA, or cached PMT is removed.
    // Writers take the PMT lock; readers re-read every pass, so a stale
    // read only delays one cycle (see ca_mask_mutex for the same rule).
    int ca_mask, disabled_ca_mask, ca_registered_mask;
    SPMT_batch *batch;
    // Set on the adapter thread, read from send_cw() on dvbapi/ca threads.
    std::atomic<int8_t> parity, update_cw;
    uint64_t last_update_cw;
    // For CA watchdogs: last batch decrypted with a verified CW and last
    // scrambled packet. Owned by the adapter thread.
    int64_t last_decrypt_ok, last_scrambled_pkt;
    // Active key: stored on the adapter thread, loaded from dvbapi/ca threads.
    std::atomic<SCW *> cw{nullptr};
    SPid *p;
    char provider[50], name[50];
    // Real SDT data for this service, emitted into the generated DDCI SDT.
    // Atomics: written on the owning thread, read holding d->mutex.
    std::atomic<bool> has_sdt{false};
    std::atomic<int> sdt_service_type{0};
    std::atomic<int> sdt_running_status{0}, sdt_ca_mode{0};
    std::atomic<int> sdt_eit_schedule{0}, sdt_eit_pf{0};
    std::atomic<int> sdt_tsid{0}, sdt_onid{0};
    void *opaque;
    char state; // PMT state (PMT_STOPPED, PMT_STARTING, PMT_RUNNING,
                // PMT_STOPPING)
    char best;  // elected into the active set this election
    int filter;
    int64_t start_time;
    std::unordered_map<uint64_t, int> *global_start, *local_start;
    struct_pmt() = default;
    ~struct_pmt();
    struct_pmt(const struct_pmt &) = delete;
    struct_pmt &operator=(const struct_pmt &) = delete;
    struct_pmt(struct_pmt &&) = delete;
    struct_pmt &operator=(struct_pmt &&) = delete;
} SPMT;

// filters can be setup for specific pids and masks
// it calls assemble_packet to reassemble the packets and then calls the
// callback function flags - 0 - does not add the pid flags - FILTER_ADD_REMOVE
// - adds the pids if there is at least one filter with this type and removes
// the pid from the list of pids, if there is no filter flags - FITLER_PERMANENT
// - keeps the filter active after changing the frequency and does not add the
// pid to the list of pids (pid 0 is permanent) PAT, PMT, CAT, ... can be
// processed also using filters another use cases: ECM EMM could be extended at
// later time with timeout or close callbacks

typedef struct struct_filter {
    std::atomic<bool> enabled{false};
    std::atomic<bool> allocated{false};
    int id;
    int pid;
    int adapter;
    void *opaque;
    int len;
    std::atomic<int> mask_len{0};
    int pos;
    char match;
    filter_function callback;
    unsigned char filter[FILTER_SIZE];
    unsigned char mask[FILTER_SIZE];
    unsigned char data[FILTER_PACKET_SIZE + 2];
    std::atomic<int> flags{0};
    std::atomic<int> next_filter{-1}, master_filter{-1};
} SFilter;

int register_algo(SCW_op *o);
SCW_op *get_op_for_algo(int algo);
void init_algo_csa();
int send_cw(int pmt_id, int algo, int parity, uint8_t *cw, uint8_t *iv,
            int64_t expiry, void *opaque);

constexpr size_t PMTS_BUCKETS =
    (MAX_PMT + kSegmentBucketSize - 1) / kSegmentBucketSize;
constexpr size_t FILTERS_BUCKETS =
    (MAX_FILTERS + kSegmentBucketSize - 1) / kSegmentBucketSize;
extern SegmentedPool<SPMT *, PMTS_BUCKETS> pmts;
extern SegmentedPool<SFilter *, FILTERS_BUCKETS, MAX_FILTERS> filters;

static inline SPMT *get_pmt(int id) {
    if (id < 0 || (size_t)id >= pmts.size())
        //		LOG_AND_RETURN(NULL, "PMT not found for id %d", id);
        return NULL;
    SPMT *pmt = pmts[id];
    if (!pmt || !pmt->enabled)
        return NULL;
    return pmt;
}

static inline SFilter *get_filter(int id) {
    if (id < 0 || id >= MAX_FILTERS)
        return NULL;
    SFilter *f = filters[id];
    return (f && f->enabled) ? f : NULL;
}
int process_pmt(int filter, unsigned char *b, int len, void *opaque);
int process_sdt(int filter, unsigned char *sdt, int len, void *opaque);
void pmt_pid_del(adapter *ad, int pid);
void pmt_pid_add(adapter *ad, int pid, int existing);
void pmt_pid_updated_pids(adapter *ad);
int pmt_init_device(
    adapter *ad); // will call action[CA_INIT_DEVICE] for the adapter if CA is
                  // registered for it in adapter_mask,
// if the call succeds, the PMTs will be sent to that CA for that adapter
int pmt_close_device(adapter *ad);
int pmt_init();
void start_pmt(SPMT *pmt, adapter *ad);
int pmt_init_device(adapter *ad);
int tables_tune(adapter *ad);
int pmt_tune(adapter *ad);
int get_active_filters_for_pid(int master_filter, int aid, int pid, int flags);
int add_filter(int aid, int pid, void *callback, void *opaque, int flags);
int add_filter_mask(int aid, int pid, void *callback, void *opaque, int flags,
                    uint8_t *filter, uint8_t *mask);
int del_filter(int id);
int set_filter_mask(int id, uint8_t *filter, uint8_t *mask);
int set_filter_flags(int id, int flags);
int set_filter_opaque(int id, void *opaque);
int get_pid_filter(int aid, int pid);
int assemble_packet(SFilter *f, uint8_t *b);
void disable_cw(int pmt_id);
void expire_cw_for_pmt(int pmt_id, int parity, int64_t min_expiry);
int pmt_add(int aid, int sid, int pmt_pid);
SPMT *get_all_pmt_for_sid(int aid, int sid);
int test_decrypt_packet(SCW *cw, SPMT_batch *start, int len);
void init_algo();
void update_cw(SPMT *pmt);
int pmt_decrypt_stream(adapter *ad);
int wait_pusi(adapter *ad, int len);
int pmt_add_ca_descriptor(SPMT *pmt, uint8_t *buf, int buf_len, int sca_id);
void stop_pmt(SPMT *pmt, adapter *ad);
int pmt_add_stream_pid(SPMT *pmt, int pid, int type, bool is_audio,
                       bool is_video);
void emulate_add_all_pids(adapter *ad);
void pmt_add_caid(SPMT *pmt, uint16_t caid, uint16_t capid, uint8_t *data,
                  int len);
#endif
