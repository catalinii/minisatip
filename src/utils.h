#ifndef UTILS_H
#define UTILS_H

#include "config.h"
#include "utils/logging/logging.h"
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

#include <pthread.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <string_view>
#include <vector>

typedef std::recursive_mutex SMutex;

#include <algorithm>
#include <optional>
#include <strings.h>

template <typename EnumT> class EnumMap {
  public:
    struct Entry {
        std::string_view key;
        EnumT value;
    };

    EnumMap(std::initializer_list<Entry> entries) : entries_(entries) {}

    std::optional<EnumT> lookup(std::string_view s) const {
        auto it =
            std::find_if(entries_.begin(), entries_.end(), [s](const Entry &e) {
                return e.key.size() == s.size() &&
                       strncasecmp(e.key.data(), s.data(), s.size()) == 0;
            });
        if (it != entries_.end()) {
            return it->value;
        }
        return std::nullopt;
    }

  private:
    std::vector<Entry> entries_;
};

std::vector<std::string_view> split(std::string_view s, char sep);
int parse_int(std::string_view s, int dv = 0);
int map_float(char *s, int mul);
int check_strs(std::string_view s, const char *const v[], int dv);
std::string_view header_parameter(const std::vector<std::string_view> &arg,
                                  int i);
char *get_current_timestamp();
std::string_view strip(std::string_view s);
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
void _strncpy(char *a, const char *b, int len);
int is_rtsp_response(char *buf, int len);
int is_rtsp_request(char *buf, int len);
int is_http_request(char *buf, int len);
int is_byte_array_empty(uint8_t *b, int len);
uint32_t get_random_uint32();
template <typename Container>
std::string iterable_to_string(const Container &container,
                               const std::string &delimiter = ", ") {
    std::stringstream ss;
    for (auto it = container.begin(); it != container.end(); ++it) {
        ss << *it;
        if (std::next(it) != container.end()) {
            ss << delimiter;
        }
    }
    return ss.str();
}

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
const char *loglevels[] = {"general", "http",   "socketworks", "stream",
                           "adapter", "satipc", "pmt",         "tables",
                           "dvbapi",  "lock",   "netceiver",   "ca",
                           "socket",  "utils",  "dmx",         "ssdp",
                           "dvb",     "ddci",   NULL};
mywritev _writev = writev;
#else
extern char *loglevels[];
extern mywritev _writev;
#endif

#endif
