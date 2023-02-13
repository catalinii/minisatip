#ifndef HTTPC_H
#define HTTPC_H
#define _GNU_SOURCE

#include "socketworks.h"
#include "utils/mutex.h"

#define MAX_HTTPC 100

struct struct_http_client;
typedef int (*http_client_action)(void *s, int len, void *opaque,
                                  struct struct_http_client *h);

typedef struct struct_http_client {
    char enabled;
    SMutex mutex;
    int state;
    http_client_action action;
    void *opaque;
    char host[64];
    char req[200];
    int port;
    int id;
} Shttp_client;

#define get_httpc(i)                                                           \
    ((i >= 0 && i < MAX_HTTPC && httpc[i] && httpc[i]->enabled) ? httpc[i]     \
                                                                : NULL)

extern Shttp_client *httpc[MAX_HTTPC];

int http_client(char *url, char *request, void *callback, void *opaque);
int http_client_add();
int http_client_del(int i);
int http_client_close(sockets *s);
void http_client_read(sockets *s);

#endif
