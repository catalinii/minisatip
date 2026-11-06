/*
 * SRT listener and pending-pool tests: the accept drain must survive
 * idle, duplicates must not evict, and returns cannot exceed the cap.
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
#include <vector>

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

// Closes every tracked socket and the listener on every exit path: a
// failed ASSERT returns early, and leaked SRT sockets hang exit.
struct srt_case_guard {
    int saved_port;
    int saved_ipv4;
    std::vector<SRTSOCKET> socks;
    srt_case_guard(int port, int ipv4) : saved_port(port), saved_ipv4(ipv4) {}
    ~srt_case_guard() {
        for (SRTSOCKET s : socks)
            srt_close(s);
        srt_listener_close();
        opts.rtsp_port = saved_port;
        opts.use_ipv4_only = saved_ipv4;
        srt_cleanup();
    }
};

int test_srt_listener_drain_and_close() {
    srt_startup();
    srt_case_guard g{opts.rtsp_port, opts.use_ipv4_only};
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
        g.socks.push_back(c);
        char sid[32];
        snprintf(sid, sizeof(sid), "test-%d", i);
        srt_setsockflag(c, SRTO_STREAMID, sid, static_cast<int>(strlen(sid)));
        int conntimeo = 1500;
        srt_setsockflag(c, SRTO_CONNTIMEO, &conntimeo, sizeof(conntimeo));
        struct sockaddr_in sa = {};
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        sa.sin_port = htons(static_cast<uint16_t>(opts.rtsp_port));
        int rc = srt_connect(c, (struct sockaddr *)&sa, sizeof(sa));
        ASSERT(rc != SRT_ERROR, "client connect must succeed");
        SRTSOCKET srv = SRT_INVALID_SOCK;
        for (int t = 0; t < 100 && srv == SRT_INVALID_SOCK; t++) {
            srv = srt_pending_take(sid);
            if (srv == SRT_INVALID_SOCK)
                usleep(20000);
        }
        ASSERT(srv != SRT_INVALID_SOCK, "server must queue the accept");
        g.socks.push_back(srv);
    }

    srt_listener_close();
    ASSERT(!srt_listener_is_init(), "listener must report closed");
    // A second cycle proves close left no wedged drain behind.
    ASSERT(srt_listener_init() == 0, "reinit must work after close");
    return 0;
}

// A duplicate streamid must not evict the first claim: the later
// handshake is rejected and the victim's socket survives to take.
int test_srt_duplicate_streamid_rejected() {
    srt_startup();
    srt_case_guard g{opts.rtsp_port, opts.use_ipv4_only};
    opts.use_ipv4_only = 1;
    opts.rtsp_port = free_udp_port();
    ASSERT(opts.rtsp_port > 0, "need a free UDP port");
    ASSERT(srt_listener_init() == 0, "listener must init");

    struct sockaddr_in sa = {};
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    sa.sin_port = htons(static_cast<uint16_t>(opts.rtsp_port));

    SRTSOCKET a = srt_create_socket();
    g.socks.push_back(a);
    const char *dup = "dup-test";
    srt_setsockflag(a, SRTO_STREAMID, dup, static_cast<int>(strlen(dup)));
    ASSERT(srt_connect(a, (struct sockaddr *)&sa, sizeof(sa)) != SRT_ERROR,
           "first claim connects");
    // Wait until the accept lands, then re-queue it: the victim
    // must be pending before the duplicate arrives.
    SRTSOCKET srv = SRT_INVALID_SOCK;
    for (int t = 0; t < 100 && srv == SRT_INVALID_SOCK; t++) {
        srv = srt_pending_take(dup);
        if (srv == SRT_INVALID_SOCK)
            usleep(20000);
    }
    ASSERT(srv != SRT_INVALID_SOCK, "server must queue the accept");
    g.socks.push_back(srv);
    srt_pending_return(srv, dup);

    SRTSOCKET b = srt_create_socket();
    g.socks.push_back(b);
    srt_setsockflag(b, SRTO_STREAMID, dup, static_cast<int>(strlen(dup)));
    int conntimeo = 1500;
    srt_setsockflag(b, SRTO_CONNTIMEO, &conntimeo, sizeof(conntimeo));
    ASSERT(srt_connect(b, (struct sockaddr *)&sa, sizeof(sa)) == SRT_ERROR,
           "duplicate streamid rejected");
    ASSERT(srt_pending_take(dup) == srv, "first claim survives");
    return 0;
}

// Filling the pool to the cap, then returning with a fresh key must
// close instead of storing: the return path cannot grow past 64.
int test_srt_pending_return_caps_pool() {
    srt_startup();
    srt_case_guard g{opts.rtsp_port, opts.use_ipv4_only};
    opts.use_ipv4_only = 1;
    opts.rtsp_port = free_udp_port();
    ASSERT(opts.rtsp_port > 0, "need a free UDP port");
    ASSERT(srt_listener_init() == 0, "listener must init");

    struct sockaddr_in sa = {};
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    sa.sin_port = htons(static_cast<uint16_t>(opts.rtsp_port));

    char sid[32];
    for (int i = 0; i < 64; i++) {
        SRTSOCKET c = srt_create_socket();
        ASSERT(c != SRT_INVALID_SOCK, "client socket create");
        g.socks.push_back(c);
        snprintf(sid, sizeof(sid), "fill-%d", i);
        srt_setsockflag(c, SRTO_STREAMID, sid, static_cast<int>(strlen(sid)));
        int rc = srt_connect(c, (struct sockaddr *)&sa, sizeof(sa));
        ASSERT(rc != SRT_ERROR, "fill connect accepted");
    }
    // Control: the accept path caps at 64.
    SRTSOCKET extra = srt_create_socket();
    ASSERT(extra != SRT_INVALID_SOCK, "client socket create");
    g.socks.push_back(extra);
    srt_setsockflag(extra, SRTO_STREAMID, "fill-65", 7);
    int conntimeo = 1500;
    srt_setsockflag(extra, SRTO_CONNTIMEO, &conntimeo, sizeof(conntimeo));
    ASSERT(srt_connect(extra, (struct sockaddr *)&sa, sizeof(sa)) == SRT_ERROR,
           "65th handshake rejected");

    // Below-cap return still stores.
    SRTSOCKET s = srt_pending_take("fill-0");
    ASSERT(s != SRT_INVALID_SOCK, "take works");
    g.socks.push_back(s);
    srt_pending_return(s, "fill-0");
    ASSERT(srt_pending_take("fill-0") == s, "below-cap return stored");

    // At the cap, a fresh-key return closes instead of storing.
    // (No take between the two returns: each take would free the
    // slot the cap probe needs.)
    srt_pending_return(s, "cap-a");
    srt_pending_return(s, "cap-b");
    ASSERT(srt_pending_take("cap-a") == s, "refill to the cap stored");
    ASSERT(srt_pending_take("cap-b") == SRT_INVALID_SOCK,
           "over-cap return not stored");
    // Close is async: poll for the socket's removal, bounded.
    for (int t = 0; t < 100 && srt_getsockstate(s) != SRTS_NONEXIST; t++)
        usleep(20000);
    ASSERT(srt_getsockstate(s) == SRTS_NONEXIST, "over-cap return closed");
    return 0;
}
#endif

int main() {
    opts.log = 1;
    strcpy(thread_info[thread_index].thread_name, "test_srt");
#ifndef DISABLE_SRT
    TEST_FUNC(test_srt_listener_drain_and_close(),
              "test SRT drain survives idle and close wakes it");
    TEST_FUNC(test_srt_duplicate_streamid_rejected(),
              "test SRT duplicate streamid rejected");
    TEST_FUNC(test_srt_pending_return_caps_pool(),
              "test SRT pending return caps pool");
#endif
    fflush(stdout);
    return 0;
}
