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

extern Shttp_client *httpc[MAX_HTTPC];

static inline Shttp_client *get_httpc(int i) {
    if (i < 0 || i >= MAX_HTTPC)
        return nullptr;
    Shttp_client *h = load_slot(httpc[i]);
    return h && h->enabled ? h : nullptr;
}

int http_client(char *url, void *callback, void *opaque);
int http_client_add();
int http_client_del(int i);
int http_client_close(sockets *s);
void http_client_read(sockets *s);

#endif
