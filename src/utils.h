#ifndef UTILS_H
#define UTILS_H
#define _GNU_SOURCE
#include <pthread.h>

#include <stdint.h>
#include <sys/types.h>
#include <sys/uio.h>

#define VAR_UINT8 1
#define VAR_INT8 2
#define VAR_UINT16 3
#define VAR_INT16 4
#define VAR_INT 5
#define VAR_INT64 6
#define VAR_STRING 7
#define VAR_PSTRING 8
#define VAR_FLOAT 9
#define VAR_HEX 10
#define VAR_ARRAY 16
#define VAR_ARRAY_UINT8 (VAR_ARRAY + VAR_UINT8)
#define VAR_ARRAY_INT8 (VAR_ARRAY + VAR_INT8)
#define VAR_ARRAY_UINT16 (VAR_ARRAY + VAR_UINT16)
#define VAR_ARRAY_INT16 (VAR_ARRAY + VAR_INT16)
#define VAR_ARRAY_INT (VAR_ARRAY + VAR_INT)
#define VAR_ARRAY_INT64 (VAR_ARRAY + VAR_INT64)
#define VAR_ARRAY_FLOAT (VAR_ARRAY + VAR_FLOAT)
#define VAR_ARRAY_HEX (VAR_ARRAY + VAR_HEX)
#define VAR_ARRAY_STRING (VAR_ARRAY + VAR_STRING)
#define VAR_ARRAY_PSTRING (VAR_ARRAY + VAR_PSTRING)
#define VAR_AARRAY 32
#define VAR_AARRAY_UINT8 (VAR_AARRAY + VAR_UINT8)
#define VAR_AARRAY_INT8 (VAR_AARRAY + VAR_INT8)
#define VAR_AARRAY_UINT16 (VAR_AARRAY + VAR_UINT16)
#define VAR_AARRAY_INT16 (VAR_AARRAY + VAR_INT16)
#define VAR_AARRAY_INT (VAR_AARRAY + VAR_INT)
#define VAR_AARRAY_INT64 (VAR_AARRAY + VAR_INT64)
#define VAR_AARRAY_FLOAT (VAR_AARRAY + VAR_FLOAT)
#define VAR_AARRAY_HEX (VAR_AARRAY + VAR_HEX)
#define VAR_AARRAY_STRING (VAR_AARRAY + VAR_STRING)
#define VAR_AARRAY_PSTRING (VAR_AARRAY + VAR_PSTRING)
#define VAR_FUNCTION 48
#define VAR_FUNCTION_INT (VAR_FUNCTION + VAR_INT)
#define VAR_FUNCTION_INT64 (VAR_FUNCTION + VAR_INT64)
#define VAR_FUNCTION_STRING (VAR_FUNCTION + VAR_STRING)

typedef int (*get_data_int)(int p);
typedef int64_t (*get_data_int64)(int p);
typedef char *(*get_data_string)(int p, char *dest, int max_len);

typedef struct struct_symbols {
    const char *name;
    int type;
    void *addr;
    float multiplier; // multiply the value of the variable
    int len;
    int skip;
} _symbols;

typedef struct struct_mutex {
    int enabled;
    pthread_mutex_t mtx;
    int state;
    int line;
    pthread_t tid;
    int64_t lock_time, create_time;
    char *file;
} SMutex;

struct struct_http_client;
typedef int (*http_client_action)(void *s, int len, void *opaque,
                                  struct struct_http_client *h);

typedef struct struct_http_client {
    char enabled;
    SMutex mutex;
    int state;
    http_client_action action;
    void *opaque;
    char host[200];
    char req[200];
    int port;
    int id;
} Shttp_client;

typedef struct hash_item {
    uint16_t len;
    uint16_t max_size;
    uint8_t is_alloc;
    int64_t key;
    void *data;
} SHashItem;

typedef struct hash_table {
    char init, resize;
    int len;
    int size;
    SHashItem *items;
    int64_t conflicts;
    SMutex mutex;
} SHashTable;

#define MAX_HTTPC 100

#define get_httpc(i)                                                           \
    ((i >= 0 && i < MAX_HTTPC && httpc[i] && httpc[i]->enabled) ? httpc[i]     \
                                                                : NULL)

extern Shttp_client *httpc[MAX_HTTPC];

int http_client(char *url, char *request, void *callback, void *opaque);

// Hash Table implementation:
// it holds the key (int32) and the value (void *) of specified size
// The data is allocated inside of the hash_item and resized if needed
// put will actually copy the data from the argument to the hash_item.data
// the value 0 means unused (do not set a key with value 0)
#define HASH_ITEM_ENABLED(h) (h.len)
#define FOREACH_ITEM(h, a)                                                     \
    for (i = 0; i < (h)->size; i++)                                            \
        if (HASH_ITEM_ENABLED((h)->items[i]) && (a = (h)->items[i].data))

int create_hash_table(SHashTable *hash, int no);
void copy_hash_table(SHashTable *s, SHashTable *d);
void free_hash(SHashTable *hash);
void *getItem(SHashTable *hash, uint32_t key);
int getItemLen(SHashTable *hash, uint32_t key);
#define setItem(a, b, c, d) _setItem(a, b, c, d, 1)
int _setItem(SHashTable *hash, uint32_t key, void *data, int len, int copy);
int delItem(SHashTable *hash, uint32_t key);
int delItemP(SHashTable *hash, void *p);
int getItemSize(SHashTable *hash, uint32_t key);
int setItemLen(SHashTable *hash, uint32_t key, int len);
int split(char **rv, char *s, int lrv, char sep);
int map_int(char *s, char **v);
int map_intd(char *s, char **v, int dv);
int map_float(char *s, int mul);
int check_strs(char *s, char **v, int dv);
void *mymalloc(int a, char *f, int l);
void *myrealloc(void *p, int a, char *f, int l);
void myfree(void *x, char *f, int l);
char *header_parameter(char **arg, int i);
char *get_current_timestamp();
char *get_current_timestamp_log();
void _log(const char *file, int line, const char *fmt, ...);
char *strip(char *s);
int split(char **rv, char *s, int lrv, char sep);
void set_signal_handler(char *argv0);
int becomeDaemon();
int end_of_header(char *buf);
char *readfile(char *fn, char *ctype, int *len);
int get_json_state(char *buf, int len);
int get_json_bandwidth(char *buf, int len);
void process_file(void *sock, char *s, int len, char *ctype);
int closefile(char *mem, int len);

int mutex_init(SMutex *mutex);
int mutex_lock1(char *FILE, int line, SMutex *mutex);
int mutex_unlock1(char *FILE, int line, SMutex *mutex);
int mutex_destroy(SMutex *mutex);
void clean_mutexes();
pthread_t start_new_thread(char *name);
pthread_t get_tid();
void set_thread_prio(pthread_t tid, int prio);

int add_new_lock(void **arr, int count, int size, SMutex *mutex);
int64_t getTick();
int64_t getTickUs();
void join_thread();
void add_join_thread(pthread_t t);
int init_utils(char *arg0);
void _hexdump(char *desc, void *addr, int len);
uint32_t crc_32(const uint8_t *data, int datalen);
void _dump_packets(char *message, unsigned char *b, int len, int packet_offset);
int get_index_hash_search(int start_pos, void *p, int max, int struct_size,
                          uint32_t key, uint32_t value);
int buffer_to_ts(uint8_t *dest, int dstsize, uint8_t *src, int srclen, char *cc,
                 int pid);
void write_buf_to_file(char *file, uint8_t *buf, int len);

#define mutex_lock(m) mutex_lock1(__FILE__, __LINE__, m)
#define mutex_unlock(m) mutex_unlock1(__FILE__, __LINE__, m)
//#define proxy_log(level, fmt, ...) _proxy_log(level, fmt"\n", ##__VA_ARGS__)

//#define LOG(a,...) {opts.last_log=a;if(opts.log){int
// x=getTick();printf(CC([%d.%03d]:
//,a,\n),x/1000,x%1000,##__VA_ARGS__);fflush(stdout);};} #define LOG(a,...)
//{opts.last_log=a;if(opts.log){printf(CC([%s]:\x20,a,\n),get_current_timestamp_log(),##__VA_ARGS__);fflush(stdout);};}
#define LOGL(level, a, ...)                                                    \
    {                                                                          \
        if ((level)&opts.log)                                                  \
            _log(__FILE__, __LINE__, a, ##__VA_ARGS__);                        \
    }

#define LOGM(a, ...) LOGL(DEFAULT_LOG, a, ##__VA_ARGS__)

#define LOG(a, ...) LOGL(1, a, ##__VA_ARGS__)

#define DEBUGL(level, a, ...)                                                  \
    {                                                                          \
        if ((level)&opts.debug)                                                \
            _log(__FILE__, __LINE__, a, ##__VA_ARGS__);                        \
    }
#define DEBUGM(a, ...) DEBUGL(DEFAULT_LOG, a, ##__VA_ARGS__)

#define dump_packets(message, b, len, packet_offset)                           \
    if (DEFAULT_LOG & opts.debug)                                              \
    _dump_packets(message, b, len, packet_offset)
#define hexdump(message, b, len)                                               \
    if (DEFAULT_LOG & opts.debug)                                              \
    _hexdump(message, b, len)

#define LOG0(a, ...)                                                           \
    { _log(__FILE__, __LINE__, a, ##__VA_ARGS__); }

#define FAIL(a, ...)                                                           \
    {                                                                          \
        if (opts.log) {                                                        \
            LOGL(0, a, ##__VA_ARGS__);                                         \
        } else                                                                 \
            LOG0(a, ##__VA_ARGS__);                                            \
        unlink(pid_file);                                                      \
        exit(1);                                                               \
    }
#define LOG_AND_RETURN(rc, a, ...)                                             \
    {                                                                          \
        LOG(a, ##__VA_ARGS__);                                                 \
        return rc;                                                             \
    }
#define malloc1(a) mymalloc(a, __FILE__, __LINE__)
#define free1(a) myfree(a, __FILE__, __LINE__)
#define realloc1(a, b) myrealloc(a, b, __FILE__, __LINE__)

#define strlcatf(buf, size, ptr, fmt...)                                       \
    do {                                                                       \
        int __r = snprintf((buf) + ptr, (size)-ptr, fmt);                      \
        ptr = __r >= (size)-ptr ? (size)-1 : ptr + __r;                        \
    } while (0)

#define strcatf(buf, ptr, fmt, ...)                                            \
    strlcatf(buf, sizeof(buf) - 1, ptr, fmt, ##__VA_ARGS__)

#define SAFE_STRCPY(a, b)                                                      \
    {                                                                          \
        int x = sizeof(a);                                                     \
        if (x < 10)                                                            \
            LOG("sizeof %d is too small at %s:%d", x, __FILE__, __LINE__);     \
        strncpy(a, b, x - 1);                                                  \
        a[x - 1] = 0;                                                          \
    }

#define TEST_FUNC(a, str, ...)                                                 \
    {                                                                          \
        int _tmp_var;                                                          \
        if ((_tmp_var = (a))) {                                                \
            LOG(#a " failed with message: " str, ##__VA_ARGS__);               \
            return _tmp_var;                                                   \
        } else                                                                 \
            LOG("%-40s OK", #a);                                               \
    }

#define LOG_GENERAL 1
#define LOG_HTTP (1 << 1)
#define LOG_SOCKETWORKS (1 << 2)
#define LOG_STREAM (1 << 3)
#define LOG_ADAPTER (1 << 4)
#define LOG_SATIPC (1 << 5)
#define LOG_PMT (1 << 6)
#define LOG_TABLES (1 << 7)
#define LOG_DVBAPI (1 << 8)
#define LOG_LOCK (1 << 9)
#define LOG_NETCEIVER (1 << 10)
#define LOG_DVBCA (1 << 11)
#define LOG_AXE (1 << 12)
#define LOG_SOCKET (1 << 13)
#define LOG_UTILS (1 << 14)
#define LOG_DMX (1 << 15)
#define LOG_SSDP (1 << 16)
#define LOG_DVB (1 << 17)

typedef ssize_t (*mywritev)(int fd, const struct iovec *io, int len);

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define PID_FROM_TS(b) (((b)[1] & 0x1F) * 256 + (b)[2])

#ifdef TESTING

#define ASSERT(cond, msg)                                                      \
    if (!(cond))                                                               \
    LOG_AND_RETURN(1, "%s:%d %s: %s", __FILE__, __LINE__, __FUNCTION__, msg)

#define writev(a, b, c) _writev(a, b, c)

#endif

#ifdef UTILS_C
char *loglevels[] = {"general",   "http", "socketworks", "stream", "adapter",
                     "satipc",    "pmt",  "tables",      "dvbapi", "lock",
                     "netceiver", "ca",   "axe",         "socket", "utils",
                     "dmx",       "ssdp", "dvb",         NULL};
mywritev _writev = writev;
#else
extern char *loglevels[];
extern mywritev _writev;
#endif

#endif
