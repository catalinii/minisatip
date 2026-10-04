/*
 * RTSP loop tests for the satipc client against a scripted fake server.
 */

#include "adapter.h"
#include "dvb.h"
#include "fake_satip_server.h"
#include "minisatip.h"
#include "satipc.h"
#include "socketworks.h"
#include "utils.h"
#include "utils/testing.h"

#include <fcntl.h>
#include <linux/dvb/frontend.h>
#include <string.h>

extern int satipc_tune(int aid, transponder *tp);
extern int satipc_commit(adapter *ad);
extern int satipc_set_pid(adapter *ad, int pid);
extern int satip_standby_device(adapter *ad);

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

int main() {
    opts.log = 1;
    opts.debug = 255;
    strcpy(thread_info[thread_index].thread_name, "test_satipc_rtsp");
    _writev = writev;

    TEST_FUNC(test_rtsp_happy_loop(), "test full SETUP to TEARDOWN loop");
    TEST_FUNC(test_rtsp_503_recovery(), "test 503 tears down and recovers");
    TEST_FUNC(test_rtsp_454_recovery(), "test 454 restarts the session");
    TEST_FUNC(test_rtsp_keepalive(), "test OPTIONS keep-alive rules");

    fflush(stdout);
    free_all();
    return 0;
}
