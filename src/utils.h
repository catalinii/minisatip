#ifndef UTILS_H
#define UTILS_H

#include "utils/logging/logging.h"
#include <mutex>

#include <pthread.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/uio.h>

typedef std::recursive_mutex SMutex;
#define mutex_lock(x) (x)->lock()
#define mutex_unlock(x) (x)->unlock()

int split(char **rv, char *s, int lrv, char sep);
int map_int(char *s, char **v);
int map_intd(char *s, char **v, int dv);
int map_float(char *s, int mul);
int check_strs(char *s, char **v, int dv);
char *header_parameter(char **arg, int i);
char *get_current_timestamp();
char *strip(char *s);
int split(char **rv, char *s, int lrv, char sep);
void set_signal_handler(char *argv0);
int becomeDaemon();
char *readfile(char *fn, char *ctype, int *len);
void process_file(void *sock, char *s, int len, char *ctype);
int closefile(char *mem, int len);

pthread_t start_new_thread(char *name);
pthread_t get_tid();
void set_thread_prio(pthread_t tid, int prio);

int find_new_id(void **arr, int count);
void join_thread();
void add_join_thread(pthread_t t);
int init_utils(char *arg0);
void _hexdump(const char *desc, void *addr, int len);
uint32_t crc_32(const uint8_t *data, int datalen);
void _dump_packets(const char *message, unsigned char *b, int len,
                   int packet_offset);
int buffer_to_ts(uint8_t *dest, int dstsize, uint8_t *src, int srclen,
                 int16_t *cc, int pid);
void write_buf_to_file(char *file, uint8_t *buf, int len);
int mkdir_recursive(const char *path);
void sleep_msec(uint32_t msec);
int get_random(unsigned char *dest, int len);
void _strncpy(char *a, char *b, int len);
int is_rtsp_response(char *buf, int len);
int is_rtsp_request(char *buf, int len);
int is_http_request(char *buf, int len);
int is_byte_array_empty(uint8_t *b, int len);

#define dump_packets(message, b, len, packet_offset)                           \
    if (DEFAULT_LOG & opts.debug)                                              \
    _dump_packets(message, b, len, packet_offset)
#define hexdump(message, b, len)                                               \
    if (DEFAULT_LOG & opts.debug)                                              \
    _hexdump(message, b, len)

#define strlcatf(buf, size, ptr, fmt...)                                       \
    do {                                                                       \
        int __r = snprintf((buf) + ptr, (size) - ptr, fmt);                    \
        ptr += __r;                                                            \
        if (ptr >= (int)(size)) {                                              \
            LOG("%s:%d buffer size too small (%d)", __FUNCTION__, __LINE__,    \
                size);                                                         \
            ptr = (size) - 1;                                                  \
        }                                                                      \
    } while (0)

#define strcatf(buf, ptr, fmt, ...)                                            \
    strlcatf(buf, (int)(sizeof(buf) - 1), ptr, fmt, ##__VA_ARGS__)

#define safe_strncpy(a, b) _strncpy(a, b, sizeof(a))

typedef ssize_t (*mywritev)(int fd, const struct iovec *io, int len);

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define PID_FROM_TS(b) (((b)[1] & 0x1F) * 256 + (b)[2])

#ifdef TESTING
#define writev(a, b, c) _writev(a, b, c)
#endif

#ifdef UTILS_C
const char *loglevels[] = {
    "general", "http",   "socketworks", "stream", "adapter",   "satipc",
    "pmt",     "tables", "dvbapi",      "lock",   "netceiver", "ca",
    "socket",  "utils",  "dmx",         "ssdp",   "dvb",       NULL};
mywritev _writev = writev;
#else
extern char *loglevels[];
extern mywritev _writev;
#endif

#endif
