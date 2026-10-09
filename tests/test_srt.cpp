/*
 * SRT listener tests: the accept drain must survive idle periods and
 * shutdown must wake it, so the backlog never stalls new handshakes.
 */
#include "minisatip.h"
#include "srt.h"
#include "utils.h"
#include "utils/testing.h"

#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef DISABLE_SRT
// Finds a free localhost UDP port for the test listener.
static int free_udp_port() {
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
        return -1;
    struct sockaddr_in sa = {};
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(s, (struct sockaddr *)&sa, sizeof(sa))) {
        close(s);
        return -1;
    }
    socklen_t len = sizeof(sa);
    getsockname(s, (struct sockaddr *)&sa, &len);
    close(s);
    return ntohs(sa.sin_port);
}

int test_srt_listener_drain_and_close() {
    srt_startup();
    int saved_port = opts.rtsp_port;
    int saved_ipv4 = opts.use_ipv4_only;
    opts.use_ipv4_only = 1;
    opts.rtsp_port = free_udp_port();
    ASSERT(opts.rtsp_port > 0, "need a free UDP port");
    ASSERT(srt_listener_init() == 0, "listener must init");
    ASSERT(srt_listener_is_init(), "listener must report init");

    // Idle with no clients: a drain that exits on empty would be gone.
    usleep(200000);

    // Backlog is 10 and the drain pops continuously: 12 sequential
    // connects must all succeed (a dead drain stalls past 10).
    for (int i = 0; i < 12; i++) {
        SRTSOCKET c = srt_create_socket();
        ASSERT(c != SRT_INVALID_SOCK, "client socket create");
        char sid[32];
        snprintf(sid, sizeof(sid), "test-%d", i);
        srt_setsockflag(c, SRTO_STREAMID, sid, strlen(sid));
        int conntimeo = 1500;
        srt_setsockflag(c, SRTO_CONNTIMEO, &conntimeo, sizeof(conntimeo));
        struct sockaddr_in sa = {};
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        sa.sin_port = htons(opts.rtsp_port);
        int rc = srt_connect(c, (struct sockaddr *)&sa, sizeof(sa));
        ASSERT(rc != SRT_ERROR, "client connect must succeed");
        SRTSOCKET srv = SRT_INVALID_SOCK;
        for (int t = 0; t < 100 && srv == SRT_INVALID_SOCK; t++) {
            srv = srt_pending_take(sid);
            if (srv == SRT_INVALID_SOCK)
                usleep(20000);
        }
        ASSERT(srv != SRT_INVALID_SOCK, "server must queue the accept");
        srt_close(srv);
        srt_close(c);
    }

    srt_listener_close();
    ASSERT(!srt_listener_is_init(), "listener must report closed");
    // A second cycle proves close left no wedged drain behind.
    ASSERT(srt_listener_init() == 0, "reinit must work after close");
    srt_listener_close();

    opts.rtsp_port = saved_port;
    opts.use_ipv4_only = saved_ipv4;
    srt_cleanup();
    return 0;
}
#endif

int main() {
    opts.log = 1;
    strcpy(thread_info[thread_index].thread_name, "test_srt");
#ifndef DISABLE_SRT
    TEST_FUNC(test_srt_listener_drain_and_close(),
              "test SRT drain survives idle and close wakes it");
#endif
    fflush(stdout);
    return 0;
}
