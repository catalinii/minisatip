/*
 * RTSP loop tests for the satipc client against a scripted fake server.
 */

#include "adapter.h"
#include "dvb.h"
#include "fake_satip_server.h"
#include "minisatip.h"
#include "satipc.h"
#include "socketworks.h"
#include "srt.h"
#include "utils.h"
#include "utils/testing.h"
#include "utils/ticks.h"

#include <dirent.h>
#include <fcntl.h>
#include <linux/dvb/frontend.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>

extern int satipc_tune(int aid, transponder *tp);
extern int satipc_commit(adapter *ad);
extern int satipc_set_pid(adapter *ad, int pid);
extern int satip_standby_device(adapter *ad);
extern int satipc_open_device(adapter *ad);

static const char *SETUP_OK =
    "RTSP/1.0 200 OK\r\nCSeq: 1\r\n"
    "Transport: RTP/AVP;unicast;destination=192.168.1.37;client_port=6508-6509;"
    "server_port=6974-6975\r\n"
    "Session: EDCD7C81;timeout=60\r\n"
    "com.ses.streamID: 308\r\n\r\n";
static const char *PLAY_OK =
    "RTSP/1.0 200 OK\r\nCSeq: 2\r\nSession: EDCD7C81\r\n\r\n";
static const char *PLAY_503 =
    "RTSP/1.0 503 Service Unavailable\r\nCSeq: 2\r\n\r\n";
static const char *PLAY_454 =
    "RTSP/1.0 454 Session Not Found\r\nCSeq: 2\r\n\r\n";
static const char *TEARDOWN_OK =
    "RTSP/1.0 200 OK\r\nCSeq: 3\r\nSession: EDCD7C81\r\n\r\n";
static const char *DESCRIBE_OK =
    "RTSP/1.0 200 OK\r\nCSeq: 4\r\nContent-Type: application/sdp\r\n\r\n"
    "tuner=1,215,1,12,0,0,0,0\r\n";
static const char *OPTIONS_OK =
    "RTSP/1.0 200 OK\r\nCSeq: 9\r\n"
    "Public: OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN\r\n\r\n";

struct RtspFixture {
    adapter ad;
    satipc sip;
    sockets rsock;
    int pipefd[2];
    int rtsp_id;
    int pid_slots;
    char reply_buf[8192];
    adapter *saved_a;
    satipc *saved_sip;

    RtspFixture()
        : ad(), sip(), rsock(), pipefd{-1, -1}, rtsp_id(-1), pid_slots(0),
          saved_a(nullptr), saved_sip(nullptr) {
        memset(reply_buf, 0, sizeof(reply_buf));
    }

    int setup() {
        if (pipe(pipefd))
            LOG_AND_RETURN(1, "pipe failed");
        int flags = fcntl(pipefd[0], F_GETFL, 0);
        fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);
        rtsp_id = sockets_add(pipefd[1], NULL, 0, TYPE_TCP, NULL, NULL, NULL);
        if (rtsp_id < 0)
            LOG_AND_RETURN(1, "sockets_add failed");

        ad.id = 0;
        ad.enabled = 1;
        ad.tp.clear();
        ad.tp.diseqc = 1;
        ad.tp.freq = 11362000;
        ad.tp.sys = SYS_DVBS2;
        ad.tp.pol = 2;
        ad.tp.sr = 22000000;
        ad.tp.fec = FEC_2_3;
        ad.sys[0] = SYS_DVBS2;
        ad.sid_cnt = 1;
        ad.sock = -1;
        ad.fe_sock = rtsp_id;
        ad.master_source = -1;

        sip.enabled = 1;
        sip.state = SATIP_STATE_SETUP;
        sip.stream_id = -1;
        sip.cseq = 1;
        sip.timeout_ms = 30000;
        strcpy(sip.sip, "192.168.1.43");
        sip.sport = 554;

        saved_a = a[0];
        saved_sip = satip[0];
        a[0] = &ad;
        satip[0] = &sip;

        rsock.sid = 0;
        rsock.id = 1;
        rsock.sock = -1;
        rsock.buf = (unsigned char *)reply_buf;
        rsock.rlen = 0;
        return 0;
    }

    void teardown() {
        a[0] = saved_a;
        satip[0] = saved_sip;
        if (pipefd[0] >= 0)
            close(pipefd[0]);
        if (pipefd[1] >= 0)
            close(pipefd[1]);
    }
};

// Mirrors update_pids: queues the pid with satipc and marks it on
// the adapter, so adapter-level and delta pids agree.
static void add_pid(RtspFixture &fx, int pid) {
    satipc_set_pid(&fx.ad, pid);
    fx.ad.pids[fx.pid_slots].pid = pid;
    fx.ad.pids[fx.pid_slots].flags = PID_STATE_ACTIVE;
    fx.pid_slots++;
}

static int get_cseq(const std::string &req) {
    const char *p = strstr(req.c_str(), "CSeq: ");
    return p ? atoi(p + 6) : -1;
}

static int has_prefix(const std::string &req, const char *prefix) {
    return req.compare(0, strlen(prefix), prefix) == 0;
}

static int has_part(const std::string &req, const char *part) {
    return strstr(req.c_str(), part) != nullptr;
}

int test_rtsp_happy_loop() {
    RtspFixture fx;
    FakeSatipServer srv;
    srv.script = {{"SETUP ", SETUP_OK},
                  {"PLAY ", PLAY_OK},
                  {"TEARDOWN ", TEARDOWN_OK},
                  {"DESCRIBE ", DESCRIBE_OK}};
    if (fx.setup())
        return 1;

    satipc_tune(0, &fx.ad.tp);
    add_pid(fx, 0);
    add_pid(fx, 16);
    add_pid(fx, 17);
    satipc_commit(&fx.ad);
    ASSERT(srv.run(&fx.rsock, fx.pipefd[0], fx.reply_buf,
                   sizeof(fx.reply_buf)) == 3,
           "tune must exchange SETUP, PLAY and DESCRIBE");
    ASSERT(srv.requests.size() == 3, "expected exactly 3 requests");
    ASSERT(has_prefix(srv.requests[0], "SETUP ") &&
               has_part(srv.requests[0], "pids=none"),
           "first request must SETUP with pids=none");
    ASSERT(has_prefix(srv.requests[1], "PLAY ") &&
               has_part(srv.requests[1], "stream=308") &&
               has_part(srv.requests[1], "Session: EDCD7C81") &&
               has_part(srv.requests[1], "addpids=0,16,17"),
           "PLAY must carry the session, stream and pids");
    ASSERT(has_prefix(srv.requests[2], "DESCRIBE "),
           "idle client must ask for signal with DESCRIBE");
    ASSERT(get_cseq(srv.requests[0]) == 1 && get_cseq(srv.requests[1]) == 2 &&
               get_cseq(srv.requests[2]) == 3,
           "CSeq must count up from 1");
    ASSERT(fx.ad.strength != 0, "signal must come from DESCRIBE");
    ASSERT(fx.sip.state == SATIP_STATE_PLAY, "client must end up playing");

    // Closing the last stream mirrors close_adapter_for_stream: standby
    // first, then the transponder is cleared so nothing restarts.
    satip_standby_device(&fx.ad);
    fx.ad.tp.clear();
    ASSERT(srv.run(&fx.rsock, fx.pipefd[0], fx.reply_buf,
                   sizeof(fx.reply_buf)) == 1,
           "standby must exchange only TEARDOWN");
    ASSERT(has_prefix(srv.requests[3], "TEARDOWN ") &&
               has_part(srv.requests[3], "stream=308"),
           "last request must tear down the stream");
    ASSERT(fx.sip.state == SATIP_STATE_INACTIVE,
           "client must go inactive after TEARDOWN");

    fx.teardown();
    return 0;
}

int test_rtsp_503_recovery() {
    RtspFixture fx;
    FakeSatipServer srv;
    srv.script = {{"SETUP ", SETUP_OK},
                  {"PLAY rtsp://192.168.1.43:554/stream=308?addpids=5167",
                   PLAY_503},
                  {"PLAY ", PLAY_OK},
                  {"TEARDOWN ", TEARDOWN_OK},
                  {"DESCRIBE ", DESCRIBE_OK}};
    if (fx.setup())
        return 1;

    satipc_tune(0, &fx.ad.tp);
    add_pid(fx, 0);
    satipc_commit(&fx.ad);
    ASSERT(srv.run(&fx.rsock, fx.pipefd[0], fx.reply_buf,
                   sizeof(fx.reply_buf)) > 0,
           "initial tune must complete");

    size_t base = srv.requests.size();
    add_pid(fx, 5167);
    add_pid(fx, 5171);
    satipc_commit(&fx.ad);
    ASSERT(srv.run(&fx.rsock, fx.pipefd[0], fx.reply_buf,
                   sizeof(fx.reply_buf)) == 4,
           "503 must cycle PLAY, TEARDOWN, SETUP, PLAY");
    ASSERT(has_prefix(srv.requests[base], "PLAY ") &&
               has_prefix(srv.requests[base + 1], "TEARDOWN ") &&
               has_prefix(srv.requests[base + 2], "SETUP ") &&
               has_prefix(srv.requests[base + 3], "PLAY "),
           "503 must tear down and re-establish the session");
    ASSERT(has_part(srv.requests[base + 3], "pids=0,5167,5171"),
           "recovery PLAY must resend the full pid list");
    ASSERT(!fx.sip.force_pids, "full pid list goes out once");
    ASSERT(fx.sip.state == SATIP_STATE_PLAY, "client must end up playing");
    ASSERT(fx.sip.stream_id == 308, "stream must be reattached");
    ASSERT(!fx.ad.err, "recovery must not flag the adapter");

    fx.teardown();
    return 0;
}

int test_rtsp_454_recovery() {
    RtspFixture fx;
    FakeSatipServer srv;
    srv.script = {{"SETUP ", SETUP_OK},
                  {"PLAY rtsp://192.168.1.43:554/stream=308?addpids=18",
                   PLAY_454},
                  {"PLAY ", PLAY_OK},
                  {"TEARDOWN ", TEARDOWN_OK},
                  {"DESCRIBE ", DESCRIBE_OK}};
    if (fx.setup())
        return 1;

    satipc_tune(0, &fx.ad.tp);
    add_pid(fx, 0);
    satipc_commit(&fx.ad);
    ASSERT(srv.run(&fx.rsock, fx.pipefd[0], fx.reply_buf,
                   sizeof(fx.reply_buf)) > 0,
           "initial tune must complete");

    size_t base = srv.requests.size();
    add_pid(fx, 18);
    satipc_commit(&fx.ad);
    ASSERT(srv.run(&fx.rsock, fx.pipefd[0], fx.reply_buf,
                   sizeof(fx.reply_buf)) == 3,
           "454 must cycle PLAY, SETUP, PLAY");
    ASSERT(has_prefix(srv.requests[base], "PLAY ") &&
               has_prefix(srv.requests[base + 1], "SETUP ") &&
               has_prefix(srv.requests[base + 2], "PLAY "),
           "454 must restart the session without TEARDOWN");
    ASSERT(has_part(srv.requests[base + 2], "pids=0,18"),
           "recovery PLAY must resend the full pid list");
    ASSERT(!fx.sip.force_pids, "full pid list goes out once");
    ASSERT(fx.sip.state == SATIP_STATE_PLAY, "client must end up playing");

    fx.teardown();
    return 0;
}

int test_rtsp_keepalive() {
    RtspFixture fx;
    FakeSatipServer srv;
    srv.script = {{"OPTIONS ", OPTIONS_OK}, {"DESCRIBE ", DESCRIBE_OK}};
    if (fx.setup())
        return 1;

    fx.sip.state = SATIP_STATE_PLAY;
    fx.sip.stream_id = 308;
    strcpy(fx.sip.session, "EDCD7C81");
    satipc_timeout(&fx.rsock);
    ASSERT(srv.run(&fx.rsock, fx.pipefd[0], fx.reply_buf,
                   sizeof(fx.reply_buf)) == 2,
           "idle timeout must send OPTIONS then DESCRIBE");
    ASSERT(has_prefix(srv.requests[0], "OPTIONS ") &&
               has_part(srv.requests[0], "Session: EDCD7C81"),
           "OPTIONS must carry the session");
    ASSERT(has_prefix(srv.requests[1], "DESCRIBE "),
           "client must ask for signal while unknown");
    ASSERT(srv.run(&fx.rsock, fx.pipefd[0], fx.reply_buf,
                   sizeof(fx.reply_buf)) == 0,
           "OPTIONS reply must stay quiet");

    fx.sip.want_tune = true;
    satipc_timeout(&fx.rsock);
    ASSERT(srv.run(&fx.rsock, fx.pipefd[0], fx.reply_buf,
                   sizeof(fx.reply_buf)) == 0,
           "timeout with queued tune must not send OPTIONS");

    fx.teardown();
    return 0;
}

#ifndef DISABLE_SRT
// Binds an ephemeral localhost UDP port and releases it, so the SRT
// connect below fails fast against a closed port.
static int closed_udp_port() {
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
        return 9;
    struct sockaddr_in sa = {};
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(s, (struct sockaddr *)&sa, sizeof(sa))) {
        close(s);
        return 9;
    }
    socklen_t len = sizeof(sa);
    getsockname(s, (struct sockaddr *)&sa, &len);
    close(s);
    return ntohs(sa.sin_port);
}

int test_rtsp_srt_failure_keeps_socket() {
    RtspFixture fx;
    if (fx.setup())
        return 1;

    fx.sip.transport_type = SIP_TRANSPORT_SRT;
    fx.sip.srt_sock = SRT_INVALID_SOCK;
    strcpy(fx.sip.sip, "127.0.0.1");
    fx.sip.sport = closed_udp_port();

    // The dispatch deletes the socket on nonzero timeout returns.
    ASSERT(satipc_timeout(&fx.rsock) == 0,
           "SRT failure must not delete the RTSP socket");

    fx.teardown();
    return 0;
}
#endif

// Binds an ephemeral localhost TCP port and releases it, so a connect
// below fails fast with ECONNREFUSED.
static int closed_tcp_port() {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
        return 9;
    struct sockaddr_in sa = {};
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(s, (struct sockaddr *)&sa, sizeof(sa))) {
        close(s);
        return 9;
    }
    socklen_t len = sizeof(sa);
    getsockname(s, (struct sockaddr *)&sa, &len);
    close(s);
    return ntohs(sa.sin_port);
}

// A failed RTSP reopen must stay flagged so the next timeout retries
// instead of wedging the adapter with a dead claimed reopen.
int test_rtsp_timeout_reopen_retries() {
    RtspFixture fx;
    if (fx.setup())
        return 1;

    strcpy(fx.sip.sip, "127.0.0.1");
    fx.sip.sport = closed_tcp_port();
    fx.sip.rtsp_socket_closed = true;

    ASSERT(satipc_timeout(&fx.rsock) == 0, "failed reopen must return 0");
    ASSERT(fx.sip.rtsp_socket_closed, "failed reopen must stay flagged");
    ASSERT(satipc_timeout(&fx.rsock) == 0, "retry must return 0");

    fx.teardown();
    return 0;
}

// A server silent past timeout_ms must flag a restart and drop the
// session without taking the adapter lock across socket calls.
int test_rtsp_timeout_restart_on_silent_server() {
    RtspFixture fx;
    if (fx.setup())
        return 1;

    fx.sip.expect_reply = true;
    fx.sip.last_response_sent = getTick() - 60000;
    fx.sip.timeout_ms = 30000;

    ASSERT(satipc_timeout(&fx.rsock) == 0, "restart timeout must return 0");
    ASSERT(fx.sip.restart_needed, "silent server must flag restart");
    ASSERT(fx.sip.state == SATIP_STATE_DISCONNECTED,
           "silent server must disconnect");

    fx.teardown();
    return 0;
}

// A failed RTP setup must release the RTSP socket opened just before:
// otherwise the fd and its tracking are orphaned on the next retry.
int test_satipc_open_device_cleans_rtsp_on_rtp_failure() {
    const int aid = 3;
    adapter ad = {};
    satipc sip = {};
    ad.id = aid;
    sip.transport_type = SIP_TRANSPORT_UDP;
    strcpy(sip.sip, "127.0.0.1");

    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0)
        return 1;
    struct sockaddr_in sa = {};
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(srv, (struct sockaddr *)&sa, sizeof(sa)) ||
        listen(srv, 1)) {
        close(srv);
        return 1;
    }
    socklen_t len = sizeof(sa);
    getsockname(srv, (struct sockaddr *)&sa, &len);
    sip.sport = ntohs(sa.sin_port);

    int saved_rtp = opts.start_rtp;
    opts.start_rtp = 5500;
    int listen_udp = opts.start_rtp + 1000 + aid * 2;
    // Occupy the RTP port without reuse so the setup bind fails.
    int block4 = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in ba = {};
    ba.sin_family = AF_INET;
    ba.sin_addr.s_addr = htonl(INADDR_ANY);
    ba.sin_port = htons(listen_udp);
    int block_ok =
        block4 >= 0 && !bind(block4, (struct sockaddr *)&ba, sizeof(ba));
    int block6 = socket(AF_INET6, SOCK_DGRAM, 0);
    if (block6 >= 0) {
        struct sockaddr_in6 b6 = {};
        b6.sin6_family = AF_INET6;
        b6.sin6_port = htons(listen_udp);
        if (bind(block6, (struct sockaddr *)&b6, sizeof(b6)))
            { close(block6); block6 = -1; }
    }
    if (!block_ok) {
        LOG("cannot occupy UDP port %d, failing", listen_udp);
        close(srv);
        if (block4 >= 0)
            close(block4);
        if (block6 >= 0)
            close(block6);
        opts.start_rtp = saved_rtp;
        return 1;
    }

    adapter *saved_a = a[aid];
    satipc *saved_sip = satip[aid];
    a[aid] = &ad;
    satip[aid] = &sip;
    int rv = satipc_open_device(&ad);
    a[aid] = saved_a;
    satip[aid] = saved_sip;

    ASSERT(rv != 0, "RTP setup must fail on the occupied port");
    ASSERT(ad.fe_sock == -1 && ad.fe == -1,
           "failed open must drop the RTSP socket");
    for (int i = 0; i < MAX_SOCKS; i++) {
        sockets *ss = get_sockets(i);
        ASSERT(!(ss && ss->sid == aid), "failed open must not leak tracking");
    }

    close(srv);
    close(block4);
    if (block6 >= 0)
        close(block6);
    opts.start_rtp = saved_rtp;
    return 0;
}

// Listens on an ephemeral localhost TCP port for open/reopen tests.
static int listen_on_loopback(int *port) {
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0)
        return -1;
    struct sockaddr_in sa = {};
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(srv, (struct sockaddr *)&sa, sizeof(sa)) ||
        listen(srv, 1)) {
        close(srv);
        return -1;
    }
    socklen_t len = sizeof(sa);
    getsockname(srv, (struct sockaddr *)&sa, &len);
    *port = ntohs(sa.sin_port);
    return srv;
}

static int count_open_fds() {
    DIR *d = opendir("/proc/self/fd");
    if (!d)
        return -1;
    int n = 0;
    while (readdir(d))
        n++;
    closedir(d);
    return n - 2;
}

static int dummy_timeout_action(sockets *s) {
    (void)s;
    return 0;
}

// Only the expected entry is dropped: matches reset and reflag,
// mismatches are left for whoever installed them.
int test_satipc_drop_raced_socket() {
    adapter ad = {};
    satipc sip = {};
    ad.id = 8;
    sip.transport_type = SIP_TRANSPORT_UDP;

    int pipefd[2];
    ASSERT(pipe(pipefd) == 0, "pipe failed");
    int id = sockets_add(pipefd[1], NULL, 8, TYPE_TCP, NULL, NULL, NULL);
    ASSERT(id >= 0, "sockets_add failed");
    ad.fe_sock = id;
    ad.fe = pipefd[1];
    ad.dvr = -1;
    sip.rtsp_socket_closed = false;
    satipc_drop_rtsp_socket(&ad, &sip, id, pipefd[1], -1);
    ASSERT(ad.fe_sock == -1 && ad.fe == -1, "fields must reset");
    ASSERT(sip.rtsp_socket_closed, "flag must return");
    ASSERT(get_sockets(id) == NULL, "entry must be gone");
    ASSERT(fcntl(pipefd[1], F_GETFD) < 0, "fd must be closed");
    close(pipefd[0]);

    ASSERT(pipe(pipefd) == 0, "pipe failed");
    int id2 = sockets_add(pipefd[1], NULL, 8, TYPE_TCP, NULL, NULL, NULL);
    ad.fe_sock = id2;
    ad.fe = pipefd[1];
    sip.rtsp_socket_closed = false;
    satipc_drop_rtsp_socket(&ad, &sip, -1, 9999, -1);
    ASSERT(ad.fe_sock == id2 && ad.fe == pipefd[1], "mismatch must keep");
    ASSERT(!sip.rtsp_socket_closed, "mismatch must not reflag");
    ASSERT(get_sockets(id2) != NULL, "entry must survive");
    sockets_setclose(id2, NULL);
    sockets_del(id2);
    close(pipefd[0]);
    close(pipefd[1]);

    // Untracked fd (sockets_add failed): the raw fd is still released.
    ASSERT(pipe(pipefd) == 0, "pipe failed");
    ad.fe_sock = -1;
    ad.fe = pipefd[1];
    sip.rtsp_socket_closed = false;
    satipc_drop_rtsp_socket(&ad, &sip, -1, pipefd[1], -1);
    ASSERT(ad.fe == -1 && sip.rtsp_socket_closed, "untracked must reset");
    ASSERT(fcntl(pipefd[1], F_GETFD) < 0, "untracked fd must close");
    close(pipefd[0]);

    // TCP: the never-tracked dvr fd is released as well.
    sip.transport_type = SIP_TRANSPORT_TCP;
    int dvrpipe[2];
    ASSERT(pipe(pipefd) == 0 && pipe(dvrpipe) == 0, "pipe failed");
    int tid = sockets_add(pipefd[1], NULL, 8, TYPE_TCP, NULL, NULL, NULL);
    ad.fe_sock = tid;
    ad.fe = pipefd[1];
    ad.dvr = dvrpipe[1];
    sip.rtsp_socket_closed = false;
    satipc_drop_rtsp_socket(&ad, &sip, tid, pipefd[1], dvrpipe[1]);
    ASSERT(ad.dvr == -1, "TCP dvr must reset");
    ASSERT(fcntl(dvrpipe[1], F_GETFD) < 0, "TCP dvr fd must close");
    close(pipefd[0]);
    close(dvrpipe[0]);
    return 0;
}

#ifndef DISABLE_SRT
// Abort releases exactly the raced publish: matching state is torn
// down, anything else (or nobody) is left untouched.
int test_satipc_abort_srt() {
    adapter ad = {};
    satipc sip = {};
    ad.id = 9;
    ad.sock = -1;
    srt_startup();
    SRTSOCKET s = srt_create_socket();
    ASSERT(s != SRT_INVALID_SOCK, "srt_create_socket");
    int pipefd[2];
    ASSERT(pipe(pipefd) == 0, "pipe failed");
    int id = sockets_add(pipefd[1], NULL, 9, TYPE_DVR, NULL, NULL, NULL);
    ad.sock = id;
    sip.srt_sock = s;
    sip.srt_streamid = "abort-me";
    sip.udp_sock = 4001;
    ad.dvr = 4002;
    satipc_abort_srt(&ad, &sip, s);
    ASSERT(sip.srt_sock == SRT_INVALID_SOCK, "srt must invalidate");
    ASSERT(sip.srt_streamid.empty(), "streamid must clear");
    ASSERT(sip.udp_sock == -1 && ad.dvr == -1, "udp/dvr must reset");
    ASSERT(get_sockets(id) && get_sockets(id)->sock == SOCK_TIMEOUT,
           "handle must swap");
    ASSERT(!srt_socket_is_connected(s), "closed srt must read down");
    SRTSOCKET s2 = srt_create_socket();
    sip.srt_sock = s2;
    sip.srt_streamid = "keep-me";
    satipc_abort_srt(&ad, &sip, SRT_INVALID_SOCK);
    ASSERT(sip.srt_sock == s2 && sip.srt_streamid == "keep-me",
           "invalid fresh must noop");
    satipc_abort_srt(&ad, &sip, s);
    ASSERT(sip.srt_sock == s2, "stale fresh must noop");
    srt_close(s2);
    sockets_setclose(id, NULL);
    sockets_del(id);
    close(pipefd[0]);
    close(pipefd[1]);
    srt_cleanup();
    return 0;
}
#endif

// A reopen on a live adapter stays published: the reconcile only
// drops sockets installed while the adapter was closing.
int test_rtsp_timeout_reopen_keeps_live_adapter() {
    RtspFixture fx;
    if (fx.setup())
        return 1;
    int port = 0;
    int srv = listen_on_loopback(&port);
    if (srv < 0) {
        fx.teardown();
        return 1;
    }
    strcpy(fx.sip.sip, "127.0.0.1");
    fx.sip.sport = port;
    fx.sip.rtsp_socket_closed = true;
    ASSERT(satipc_timeout(&fx.rsock) == 0, "reopen must return 0");
    ASSERT(!fx.sip.rtsp_socket_closed, "live reopen must stay open");
    ASSERT(fx.ad.fe_sock >= 0, "live reopen must track the socket");
    sockets *ss = get_sockets(fx.ad.fe_sock);
    ASSERT(ss && ss->sid == 0, "reopened socket must be tracked");
    sockets_setclose(fx.ad.fe_sock, NULL);
    sockets_del(fx.ad.fe_sock);
    fx.ad.fe_sock = -1;
    fx.ad.fe = -1;
    close(srv);
    fx.teardown();
    return 0;
}

// A full socket table makes the RTSP sockets_add fail while the TCP
// connect succeeds: the failed open must still release the raw fd.
int test_satipc_open_device_full_table_closes_untracked() {
    const int aid = 4;
    int ids[MAX_SOCKS];
    int n = 0;
    for (int i = 0; i < MAX_SOCKS; i++) {
        int id = sockets_add(SOCK_TIMEOUT, NULL, 60, TYPE_UDP, NULL, NULL,
                             (socket_action)dummy_timeout_action);
        if (id < 0)
            break;
        ids[n++] = id;
    }
    int extra = sockets_add(SOCK_TIMEOUT, NULL, 60, TYPE_UDP, NULL, NULL,
                            (socket_action)dummy_timeout_action);
    ASSERT(extra < 0, "table must be full");

    adapter ad = {};
    satipc sip = {};
    ad.id = aid;
    sip.transport_type = SIP_TRANSPORT_UDP;
    strcpy(sip.sip, "127.0.0.1");
    int port = 0;
    int srv = listen_on_loopback(&port);
    if (srv < 0)
        return 1;
    sip.sport = port;

    int saved_rtp = opts.start_rtp;
    opts.start_rtp = 5500;
    int listen_udp = opts.start_rtp + 1000 + aid * 2;
    int block4 = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in ba = {};
    ba.sin_family = AF_INET;
    ba.sin_addr.s_addr = htonl(INADDR_ANY);
    ba.sin_port = htons(listen_udp);
    int block_ok =
        block4 >= 0 && !bind(block4, (struct sockaddr *)&ba, sizeof(ba));
    if (!block_ok) {
        LOG("cannot occupy UDP port %d, failing", listen_udp);
        close(srv);
        close(block4);
        opts.start_rtp = saved_rtp;
        return 1;
    }

    adapter *saved_a = a[aid];
    satipc *saved_sip = satip[aid];
    a[aid] = &ad;
    satip[aid] = &sip;
    int before = count_open_fds();
    int rv = satipc_open_device(&ad);
    int after = count_open_fds();
    a[aid] = saved_a;
    satip[aid] = saved_sip;

    ASSERT(rv != 0, "RTP setup must fail on the occupied port");
    ASSERT(ad.fe == -1, "untracked fd must be released");
    ASSERT(before >= 0 && after == before, "no fd may leak");
    for (int i = 0; i < MAX_SOCKS; i++) {
        sockets *ss = get_sockets(i);
        ASSERT(!(ss && ss->sid == aid), "failed open must not track");
    }

    for (int i = 0; i < n; i++)
        sockets_del(ids[i]);
    close(srv);
    close(block4);
    opts.start_rtp = saved_rtp;
    return 0;
}

int main() {
    opts.log = 1;
    opts.debug = 255;
    strcpy(thread_info[thread_index].thread_name, "test_satipc_rtsp");
    _writev = writev;

    TEST_FUNC(test_rtsp_happy_loop(), "test full SETUP to TEARDOWN loop");
    TEST_FUNC(test_rtsp_503_recovery(), "test 503 tears down and recovers");
    TEST_FUNC(test_rtsp_454_recovery(), "test 454 restarts the session");
    TEST_FUNC(test_rtsp_keepalive(), "test OPTIONS keep-alive rules");
    TEST_FUNC(test_rtsp_timeout_reopen_retries(),
              "test failed RTSP reopen stays flagged for retry");
    TEST_FUNC(test_rtsp_timeout_restart_on_silent_server(),
              "test silent server flags restart and disconnects");
    TEST_FUNC(test_satipc_open_device_cleans_rtsp_on_rtp_failure(),
              "test failed open drops the RTSP socket and tracking");
#ifndef DISABLE_SRT
    TEST_FUNC(test_rtsp_srt_failure_keeps_socket(),
              "test SRT failure keeps the RTSP socket");
    TEST_FUNC(test_satipc_abort_srt(), "test SRT abort releases the publish");
#endif
    TEST_FUNC(test_satipc_drop_raced_socket(),
              "test raced drop only releases its own socket");
    TEST_FUNC(test_rtsp_timeout_reopen_keeps_live_adapter(),
              "test live reopen stays published");
    // Last: fills the whole socket table (restores it afterwards).
    TEST_FUNC(test_satipc_open_device_full_table_closes_untracked(),
              "test full table still releases the untracked fd");

    fflush(stdout);
    free_all();
    return 0;
}
