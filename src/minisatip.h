#ifndef MINISATIP_H
#define MINISATIP_H

#define _GNU_SOURCE

#include "opts.h"
#include "socketworks.h"
#include "stream.h"
#include "utils.h"

#define VERSION (MAJOR MINOR "~" REVISION)

#define EMU_PIDS_ALL_ENFORCED_PIDS_LIST 1, 16, 17, 18, 20, 21

void set_options(int argc, char *argv[]);
char *get_command_line_string(int argc, char *argv[]);

extern char pid_file[];
extern char app_name[], version[];

#ifndef offsetof
#define offsetof(st, m) __builtin_offsetof(st, m)
#endif

#define copy32(a, i, v)                                                        \
    {                                                                          \
        a[i] = ((v) >> 24) & 0xFF;                                             \
        a[i + 1] = ((v) >> 16) & 0xFF;                                         \
        a[i + 2] = ((v) >> 8) & 0xFF;                                          \
        a[i + 3] = (v)&0xFF;                                                   \
    }
#define copy16(a, i, v)                                                        \
    {                                                                          \
        a[i] = ((v) >> 8) & 0xFF;                                              \
        a[i + 1] = (v)&0xFF;                                                   \
    }

#define copy16r(v, a, i)                                                       \
    { v = ((a[i] & 0xFF) << 8) | a[i + 1]; }
#define copy16rr(v, a, i)                                                      \
    { v = ((a[i + 1] & 0xFF) << 8) | a[i]; }

#define copy32r(v, a, i)                                                       \
    {                                                                          \
        v = ((unsigned int)a[i] << 24) | ((unsigned int)a[i + 1] << 16) |      \
            ((unsigned int)a[i + 2] << 8) | (unsigned int)a[i + 3];            \
    }
#define copy32rr(v, a, i)                                                      \
    {                                                                          \
        v = ((unsigned int)a[i + 3] << 24) | ((unsigned int)a[i + 2] << 16) |  \
            ((unsigned int)a[i + 1] << 8) | (unsigned int)a[i];                \
    }
int ssdp_discovery(sockets *s);
int readBootID();
void http_response(sockets *s, int rc, char *ah, char *desc, int cseq, int lr);

#endif
