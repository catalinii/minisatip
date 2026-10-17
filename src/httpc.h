#ifndef HTTPC_H
#define HTTPC_H

#include "socketworks.h"
#include "utils.h"

#define MAX_HTTPC 100

struct struct_http_client;
typedef int (*http_client_action)(void *s, int len, void *opaque,
                                  struct struct_http_client *h);

typedef struct struct_http_client {
    std::atomic<bool> enabled{false};
    std::atomic<bool> allocated{false};
    SMutex mutex;
    int state;
    http_client_action action;
    void *opaque;
    char host[64];
    char req[200];
    int port;
    int id;
} Shttp_client;

extern std::array<std::atomic<Shttp_client *>, MAX_HTTPC> httpc;

static inline Shttp_client *get_httpc(int i) {
    if (i < 0 || i >= MAX_HTTPC)
        return NULL;
    Shttp_client *h = httpc[i].load();
    return (h && h->enabled) ? h : NULL;
}

int http_client(char *url, void *callback, void *opaque);
int http_client_add();
int http_client_del(int i);
int http_client_close(sockets *s);
void http_client_read(sockets *s);

#endif
