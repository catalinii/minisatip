#ifndef LOGGING_H
#define LOGGING_H

#include "opts.h"

#include <stdint.h>
#include <sys/types.h>

void _log(const char *file, int line, const char *fmt, ...);
void _dump_packets(char *message, unsigned char *b, int len, int packet_offset);
void _hexdump(char *desc, void *addr, int len);
char *get_current_timestamp_log();
uint32_t crc_32(const uint8_t *data, int datalen);

#define MAX_THREAD_INFO 135 // 128 + 7
typedef struct {
    char enabled;
    pthread_t tid;
    char thread_name[100];
    const char *last_log;
} SThreadInfo;
extern SThreadInfo thread_info[MAX_THREAD_INFO];
extern __thread int thread_index;

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

#define LOGL(level, a, ...)                                                    \
    {                                                                          \
        thread_info[thread_index].last_log = a;                                \
        if ((level)&opts.log)                                                  \
            _log(__FILE__, __LINE__, a, ##__VA_ARGS__);                        \
    }

#define LOGM(a, ...) LOGL(DEFAULT_LOG, a, ##__VA_ARGS__)

#define LOG(a, ...) LOGL(1, a, ##__VA_ARGS__)

#define DEBUGL(level, a, ...)                                                  \
    {                                                                          \
        thread_info[thread_index].last_log = a;                                \
        if ((level)&opts.debug)                                                \
            _log(__FILE__, __LINE__, a, ##__VA_ARGS__);                        \
    }
#define DEBUGM(a, ...) DEBUGL(DEFAULT_LOG, a, ##__VA_ARGS__)

#define LOG0(a, ...)                                                           \
    {                                                                          \
        thread_info[thread_index].last_log = a;                                \
        _log(__FILE__, __LINE__, a, ##__VA_ARGS__);                            \
    }

#define FAIL(a, ...)                                                           \
    {                                                                          \
        LOG0(a, ##__VA_ARGS__);                                            \
        unlink(pid_file);                                                      \
        exit(1);                                                               \
    }
#define LOG_AND_RETURN(rc, a, ...)                                             \
    {                                                                          \
        LOG(a, ##__VA_ARGS__);                                                 \
        return rc;                                                             \
    }

#endif
