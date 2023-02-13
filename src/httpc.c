/*
 * Copyright (C) 2014-2022 Catalin Toda <catalinii@yahoo.com> et al
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 *
 */
#include "httpc.h"
#include "utils/logging/logging.h"

#include <stdio.h>
#include <string.h>

#define DEFAULT_LOG LOG_HTTP

Shttp_client *httpc[MAX_HTTPC];
SMutex httpc_mutex;

int http_client_add() {

    Shttp_client *h;
    int i = add_new_lock((void **)httpc, MAX_HTTPC, sizeof(Shttp_client),
                         &httpc_mutex);
    if (i == -1) {
        LOG_AND_RETURN(-1, "Could not add new http client");
    }

    h = httpc[i];
    h->id = i;
    h->opaque = NULL;
    memset(h->host, 0, sizeof(h->host));
    memset(h->req, 0, sizeof(h->req));
    h->port = 0;
    mutex_unlock(&h->mutex);
    LOG("returning new http client %d", i);

    return i;
}

int http_client_del(int i) {
    Shttp_client *h;
    h = get_httpc(i);
    if (!h)
        return 0;

    if (mutex_lock(&h->mutex))
        return 0;
    h->enabled = 0;
    mutex_destroy(&h->mutex);
    LOGM("Stopping http client %d", i);
    return 0;
}

int http_client_close(sockets *s) {
    Shttp_client *h = get_httpc(s->sid);
    if (!h) {
        LOG("HTTP Client record not found for sockets id %d, http client "
            "id %d",
            s->id, s->sid);
        return 1;
    }
    if (h->action)
        h->action(NULL, 0, h->opaque, h);

    http_client_del(h->id);
    return 1;
}

void http_client_read(sockets *s) {
    Shttp_client *h = get_httpc(s->sid);
    if (!h) {
        LOG("HTTP Client record not found for sockets id %d, http client "
            "id %d",
            s->id, s->sid);
        return;
    }
    if (!s->rlen && h->req[0]) {
        char headers[500];
        sprintf(headers, "GET %s HTTP/1.0\r\n\r\n", (char *)h->req);
        LOGM("%s: sending to %d: %s", __FUNCTION__, s->sock, (char *)h->req);
        sockets_write(s->id, headers, strlen(headers));
        h->req[0] = 0;
        return;
    }
    if (h->action)
        h->action(s->buf, s->rlen, h->opaque, h);
    s->rlen = 0;
    return;
}

int http_client(char *url, char *request, void *callback, void *opaque) {
    Shttp_client *h;
    int id;
    char *req;
    char *sep;
    int http_client_sock, sock;

    if (strncmp("http", url, 4))
        LOG_AND_RETURN(0, "Only http support for %s", url);

    id = http_client_add();
    h = get_httpc(id);
    if (!h)
        LOG_AND_RETURN(1, "Could not add http client");
    safe_strncpy(h->host, url + 7);
    h->port = 80;
    sep = strchr(h->host, ':');
    if (sep) {
        h->port = map_intd(sep + 1, NULL, 80);
    }
    if (!sep)
        sep = strchr(h->host, '/');
    if (!sep)
        sep = url + strlen(h->host);
    sep[0] = 0;

    req = strchr(url + 7, '/');
    if (!req)
        req = "/";

    sock = tcp_connect(h->host, h->port, NULL, 0);
    if (sock < 0)
        LOG_AND_RETURN(1, "%s: connect to %s:%d failed", __FUNCTION__, h->host,
                       h->port);
    http_client_sock = sockets_add(sock, NULL, -1, TYPE_TCP | TYPE_CONNECT,
                                   (socket_action)http_client_read,
                                   (socket_action)http_client_close,
                                   (socket_action)http_client_close);
    if (http_client_sock < 0)
        LOG_AND_RETURN(1, "%s: sockets_add failed", __FUNCTION__);
    h->opaque = opaque;
    h->action = callback;
    set_sockets_sid(http_client_sock, id);
    safe_strncpy(h->req, req);
    sockets_timeout(http_client_sock, 2000); // 2s timeout
    LOGM("%s url %s using handle %d s_id %d", __FUNCTION__, url, sock,
         http_client_sock);
    return 0;
}
