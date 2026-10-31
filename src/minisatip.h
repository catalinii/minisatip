#ifndef MINISATIP_H
#define MINISATIP_H

#include "opts.h"
#include "socketworks.h"
#include "stream.h"
#include "utils.h"

#include <type_traits>

void set_options(int argc, char *argv[]);
char *get_command_line_string(int argc, const char *const argv[]);
unsigned int log_module_mask(std::string_view name);

extern char pid_file[];
extern char app_name[], app_version[];

#ifndef offsetof
#define offsetof(st, m) __builtin_offsetof(st, m)
#endif

// The byte goes to the element type of a, whichever char type that is
#define COPY_BYTE(a, i, v)                                                     \
    a[i] = static_cast<std::remove_cvref_t<decltype(a[i])>>((v) & 0xFF)
#define copy32(a, i, v)                                                        \
    {                                                                          \
        COPY_BYTE(a, i, (v) >> 24);                                            \
        COPY_BYTE(a, i + 1, (v) >> 16);                                        \
        COPY_BYTE(a, i + 2, (v) >> 8);                                         \
        COPY_BYTE(a, i + 3, v);                                                \
    }
#define copy16(a, i, v)                                                        \
    {                                                                          \
        COPY_BYTE(a, i, (v) >> 8);                                             \
        COPY_BYTE(a, i + 1, v);                                                \
    }

// The value read goes to the type of v
#define COPY_READ(v, x) v = static_cast<std::remove_cvref_t<decltype(v)>>(x)
#define copy16r(v, a, i)                                                       \
    {                                                                          \
        COPY_READ(v, ((a[i] & 0xFF) << 8) | a[i + 1]);                         \
    }
#define copy16rr(v, a, i)                                                      \
    {                                                                          \
        COPY_READ(v, ((a[i + 1] & 0xFF) << 8) | a[i]);                         \
    }

#define copy32r(v, a, i)                                                       \
    {                                                                          \
        COPY_READ(                                                             \
            v, ((unsigned int)a[i] << 24) | ((unsigned int)a[i + 1] << 16) |   \
                   ((unsigned int)a[i + 2] << 8) | (unsigned int)a[i + 3]);    \
    }
#define copy32rr(v, a, i)                                                      \
    {                                                                          \
        COPY_READ(v, ((unsigned int)a[i + 3] << 24) |                          \
                         ((unsigned int)a[i + 2] << 16) |                      \
                         ((unsigned int)a[i + 1] << 8) | (unsigned int)a[i]);  \
    }
int ssdp_discovery(sockets *s);
int readBootID();
void http_response(sockets *s, int rc, const char *ah, const char *desc,
                   int cseq, int lr);
int read_rtsp(sockets *s);
int read_http(sockets *s);

#endif
