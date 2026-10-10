/*
 * Copyright (C) 2014-2026 Catalin Toda <catalinii@yahoo.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include "adapter.h"
#include "dvb.h"
#include "minisatip.h"
#include "socketworks.h"
#include "stream.h"
#include "utils.h"
#include "utils/testing.h"

#include <linux/dvb/frontend.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <future>
#include <string>
#include <thread>

#ifndef DISABLE_SRT
#include "srt.h"
// Mock srt_send: interposes the libsrt symbol in this test binary so
// flush_stream() can be exercised without a real SRT connection.
static int mock_srt_fail = 0;
static int mock_srt_calls = 0;
extern "C" int srt_send(SRTSOCKET u, const char *buf, int len) {
    (void)u;
    (void)buf;
    mock_srt_calls++;
    if (mock_srt_fail)
        return SRT_ERROR;
    return len;
}
#endif

int mock_adapter_tune(int aid, transponder *tp) { return 0; }

int mock_set_pid(adapter *ad, int i_pid) {
    return 123; // Dummy fd for the demux filter
}

int mock_del_filters(adapter *ad, int fd, int pid) { return 0; }

void setup_test_env() {
    free_all(); // Clean all streams, sockets, and adapters

    a_count = 1;
    a[0] = new adapter();
    a[0]->enabled = 1;
    a[0]->id = 0;
    a[0]->master_sid = -1;
    a[0]->sid_cnt = 0;
    for (int i = 0; i < MAX_DELSYS; i++)
        a[0]->sys[i] = SYS_UNDEFINED;
    a[0]->sys[0] = SYS_DVBS2;
    a[0]->tune = mock_adapter_tune;
    a[0]->set_pid = mock_set_pid;
    a[0]->del_filters = mock_del_filters;

    for (int i = 0; i < MAX_PIDS; i++) {
        a[0]->pids[i].pid = -1;
        a[0]->pids[i].fd = -1;
        a[0]->pids[i].flags = 0;
    }
}

int test_setup_stream_new() {
    setup_test_env();

    sockets mock_sock = {};
    mock_sock.enabled = 1;
    mock_sock.is_enabled = 1;
    mock_sock.sock = 1000;
    mock_sock.id = 0;
    mock_sock.sid = -1; // Request new stream allocation
    mock_sock.type = TYPE_HTTP;
    mock_sock.buf = (unsigned char *)"SETUP";

    std::string_view req =
        "?src=1&freq=11362&pol=h&sr=22000&msys=dvbs2&pids=0,16";
    streams *sid = setup_stream(req, &mock_sock);

    ASSERT(sid != NULL, "setup_stream returned NULL");
    ASSERT(mock_sock.sid == sid->sid,
           "socket sid not updated to match stream sid");
    ASSERT(sid->enabled == 1, "allocated stream is not enabled");
    ASSERT(sid->sock == 1000, "stream socket not set correctly");
    ASSERT(sid->tp.freq == 11362000,
           "frequency in stream transponder parsed incorrectly");
    ASSERT(sid->tp.sys == SYS_DVBS2,
           "msys in stream transponder parsed incorrectly");
    ASSERT(sid->tp.pol == 2, "pol in stream transponder parsed incorrectly");
    ASSERT(sid->tp.pids.size() == 2 && sid->tp.pids.contains(0) &&
               sid->tp.pids.contains(16),
           "pids in stream transponder parsed incorrectly");

    return 0;
}

int test_setup_stream_existing() {
    setup_test_env();

    sockets mock_sock = {};
    mock_sock.enabled = 1;
    mock_sock.is_enabled = 1;
    mock_sock.sock = 1001;
    mock_sock.id = 1;
    mock_sock.sid = -1; // Request new stream allocation
    mock_sock.type = TYPE_HTTP;
    mock_sock.buf = (unsigned char *)"SETUP";

    std::string_view req1 =
        "?src=1&freq=11362&pol=h&sr=22000&msys=dvbs2&pids=0,16";
    streams *sid1 = setup_stream(req1, &mock_sock);
    ASSERT(sid1 != NULL, "first setup_stream returned NULL");
    int allocated_sid = mock_sock.sid;

    // Call setup_stream again with different parameters on the same socket
    // (reusing the allocated sid)
    std::string_view req2 =
        "?src=1&freq=12543&pol=v&sr=27500&msys=dvbs2&pids=0,17&addpids=100";
    streams *sid2 = setup_stream(req2, &mock_sock);
    ASSERT(sid2 != NULL, "second setup_stream returned NULL");
    ASSERT(sid2->sid == allocated_sid, "did not reuse the same stream ID");
    ASSERT(sid2->tp.freq == 12543000, "frequency was not updated");
    ASSERT(sid2->tp.pol == 1, "polarity was not updated");
    ASSERT(sid2->tp.pids.size() == 3 && sid2->tp.pids.contains(0) &&
               sid2->tp.pids.contains(17) && sid2->tp.pids.contains(100),
           "pids were not updated correctly");

    return 0;
}

int test_start_play_no_transport() {
    setup_test_env();

    sockets mock_sock = {};
    mock_sock.enabled = 1;
    mock_sock.is_enabled = 1;
    mock_sock.sock = 1002;
    mock_sock.id = 2;
    mock_sock.sid = -1;
    mock_sock.type = TYPE_RTSP; // RTSP transport header is checked
    mock_sock.buf = (unsigned char *)"PLAY";

    std::string_view req =
        "?src=1&freq=11362&pol=h&sr=22000&msys=dvbs2&pids=0,16";
    streams *sid = setup_stream(req, &mock_sock);
    ASSERT(sid != NULL, "setup_stream returned NULL");

    // sid->type is 0 because we didn't decode transport yet
    sid->type = 0;

    int res = start_play(sid, &mock_sock);
    ASSERT(res == -454, "start_play should return -454 for empty transport");

    return 0;
}

int test_start_play_success() {
    setup_test_env();

    sockets mock_sock = {};
    mock_sock.enabled = 1;
    mock_sock.is_enabled = 1;
    mock_sock.sock = 1003;
    mock_sock.id = 3;
    mock_sock.sid = -1;
    mock_sock.type = TYPE_HTTP; // HTTP doesn't require RTSP transport headers
    mock_sock.buf = (unsigned char *)"PLAY";

    std::string_view req =
        "?src=1&freq=11362&pol=h&sr=22000&msys=dvbs2&pids=0,16";
    streams *sid = setup_stream(req, &mock_sock);
    ASSERT(sid != NULL, "setup_stream returned NULL");

    // HTTP play will set sid->type to STREAM_HTTP inside start_play itself if
    // sid->type == 0 and s->type == TYPE_HTTP
    int res = start_play(sid, &mock_sock);
    ASSERT(res == 0, "start_play failed with HTTP setup");
    ASSERT(sid->do_play == 1, "sid->do_play was not set to 1");
    ASSERT(sid->type == STREAM_HTTP, "stream type not set to STREAM_HTTP");
    ASSERT(sid->adapter == 0, "stream adapter not associated to 0");

    return 0;
}

int test_setup_stream_unspecified_vs_empty_pids() {
    setup_test_env();

    sockets mock_sock = {};
    mock_sock.enabled = 1;
    mock_sock.is_enabled = 1;
    mock_sock.sock = 1004;
    mock_sock.id = 4;
    mock_sock.sid = -1;
    mock_sock.type = TYPE_HTTP;
    mock_sock.buf = (unsigned char *)"SETUP";

    // Setup initially with some pids
    std::string_view req1 =
        "?src=1&freq=11362&pol=h&sr=22000&msys=dvbs2&pids=0,16&addpids=100";
    streams *sid = setup_stream(req1, &mock_sock);
    ASSERT(sid != NULL, "setup_stream failed");
    ASSERT(sid->tp.pids.size() == 3 && sid->tp.pids.contains(0) &&
               sid->tp.pids.contains(16) && sid->tp.pids.contains(100),
           "pids not set correctly");

    // Call setup_stream again with UNSPECIFIED pids and NO freq= in request.
    // They should inherit the previous values.
    std::string_view req2 = "?pol=v";
    setup_stream(req2, &mock_sock);
    ASSERT(sid->tp.pids.size() == 3 && sid->tp.pids.contains(0) &&
               sid->tp.pids.contains(16) && sid->tp.pids.contains(100),
           "pids should be inherited when unspecified");

    // Call setup_stream again with freq= in request.
    // They should be cleared.
    std::string_view req3 = "?freq=11362&pol=h&sr=22000&msys=dvbs2";
    setup_stream(req3, &mock_sock);
    ASSERT(sid->tp.pids.empty(),
           "pids should be cleared when freq= is specified");

    return 0;
}

#ifndef DISABLE_SRT
extern int64_t bw;
extern uint32_t writes, failed_writes;
int flush_stream(streams *sid, struct iovec *iov, int iiov, int64_t ctime);

int test_flush_stream_srt_accounts_bw() {
    setup_test_env();

    int s_id = streams_add();
    ASSERT(s_id >= 0, "streams_add failed");
    streams *sid = get_sid(s_id);
    ASSERT(sid != NULL, "get_sid returned NULL");
    sid->type = STREAM_RTSP_SRT;
    sid->srt_sock = 12345;
    sid->rsock = -1;
    sid->rsock_id = -1;

    // Larger than MAX_UDP_PACKET_SIZE to exercise multi-chunk accounting
    char buf[10 * DVB_FRAME];
    memset(buf, 0, sizeof(buf));
    struct iovec iov[1];
    iov[0].iov_base = buf;
    iov[0].iov_len = sizeof(buf);

    mock_srt_fail = 0;
    mock_srt_calls = 0;
    int64_t bw0 = bw;
    uint32_t w0 = writes;
    uint32_t sb0 = sid->sb, sp0 = sid->sp;

    int rv = flush_stream(sid, iov, 1, 0);
    ASSERT(rv == (int)sizeof(buf), "SRT flush_stream should return sent bytes");
    ASSERT(mock_srt_calls > 1, "expected chunked srt_send calls");
    ASSERT(bw == bw0 + (int64_t)sizeof(buf), "bw not updated for SRT send");
    ASSERT(writes == w0 + 1, "writes not updated for SRT send");
    ASSERT(sid->sb == sb0 + sizeof(buf), "stream sb not updated");
    ASSERT(sid->sp == sp0 + 1, "stream sp not updated");

    mock_srt_fail = 1;
    int64_t bw1 = bw;
    uint32_t fw1 = failed_writes;
    sid->timeout = 0;
    rv = flush_stream(sid, iov, 1, 0);
    ASSERT(sid->timeout == 1, "timeout not set on SRT send failure");
    ASSERT(failed_writes == fw1 + 1,
           "failed_writes not updated on SRT failure");
    ASSERT(bw == bw1, "bw should not change on failed SRT send");
    mock_srt_fail = 0;
    return 0;
}
#endif

// A demux thread holds the RTP socket and waits for the stream while the
// client moves its RTP port: decode_transport must not deadlock with it.
int test_decode_transport_port_change_no_deadlock() {
    setup_test_env();

    sockets rtsp = {};
    rtsp.enabled = 1;
    rtsp.is_enabled = 1;
    rtsp.sock = 1000;
    rtsp.id = 0;
    rtsp.sid = -1;
    rtsp.type = TYPE_RTSP;
    rtsp.buf = (unsigned char *)"SETUP";

    streams *sid =
        setup_stream("?src=1&freq=11362&pol=h&sr=22000&pids=0", &rtsp);
    ASSERT(sid != NULL, "setup_stream returned NULL");
    opts.start_rtp = 45500;
    char host[] = "127.0.0.1";
    ASSERT(decode_transport(&rtsp, "RTP/AVP;unicast;client_port=46000-46001",
                            host, opts.start_rtp) == 0,
           "first transport failed");
    sockets *rs = get_sockets(sid->rsock_id);
    ASSERT(rs != NULL, "RTP socket not created");

    std::atomic<int> held{0};
    std::thread demux([&] {
        std::lock_guard<SMutex> l1(rs->mutex);
        held = 1;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::lock_guard<SMutex> l2(sid->mutex);
    });
    while (!held)
        usleep(1000);

    auto f = std::async(std::launch::async, [&] {
        return decode_transport(&rtsp,
                                "RTP/AVP;unicast;client_port=46002-46003", host,
                                opts.start_rtp);
    });
    if (f.wait_for(std::chrono::seconds(3)) != std::future_status::ready) {
        LOG("decode_transport deadlocked with the demux thread");
        fflush(stdout);
        _exit(1);
    }
    demux.join();
    ASSERT(f.get() == 0, "second transport failed");
    ASSERT(get_sockets(sid->rsock_id) != NULL, "new RTP socket not created");
    ASSERT(get_stream_rport(sid->sid) == 46002, "client port not updated");

    close_stream(sid->sid);
    return 0;
}

int test_start_play_all_tuners_busy_returns_503() {
    setup_test_env();

    sockets http1 = {};
    http1.enabled = 1;
    http1.is_enabled = 1;
    http1.sock = 1005;
    http1.id = 5;
    http1.sid = -1;
    http1.type = TYPE_HTTP;
    http1.buf = (unsigned char *)"PLAY";

    streams *sid1 = setup_stream(
        "?src=1&freq=11362&pol=h&sr=22000&msys=dvbs2&pids=0,16", &http1);
    ASSERT(sid1 != NULL, "first setup_stream returned NULL");
    ASSERT(start_play(sid1, &http1) == 0, "first start_play failed");

    sockets http2 = {};
    http2.enabled = 1;
    http2.is_enabled = 1;
    http2.sock = 1006;
    http2.id = 6;
    http2.sid = -1;
    http2.type = TYPE_HTTP;
    http2.buf = (unsigned char *)"PLAY";

    streams *sid2 = setup_stream(
        "?src=1&freq=12543&pol=v&sr=27500&msys=dvbs2&pids=0,17", &http2);
    ASSERT(sid2 != NULL, "second setup_stream returned NULL");
    int res = start_play(sid2, &http2);
    ASSERT(res == -503, "start_play should return -503 when busy");
    ASSERT(sid2->rtsp_error == "No-More: frontends",
           "missing No-More: frontends body");
    ASSERT(sid2->adapter == -1, "failed play must not attach an adapter");
    ASSERT(a[0]->sid_cnt == 1 && a[0]->master_sid == sid1->sid,
           "adapter must still belong to the first stream");

    // Free the tuner: the same request must now succeed with no error body.
    close_stream(sid1->sid);
    ASSERT(start_play(sid2, &http2) == 0, "replay after free failed");
    ASSERT(sid2->rtsp_error.empty(), "success must clear the error body");

    return 0;
}

int test_start_play_unmapped_src_returns_403() {
    setup_test_env();
    memset(a[0]->absolute_table, 0, sizeof(a[0]->absolute_table));
    char saved_switch = absolute_switch;
    absolute_switch = 1;

    sockets http = {};
    http.enabled = 1;
    http.is_enabled = 1;
    http.sock = 1007;
    http.id = 7;
    http.sid = -1;
    http.type = TYPE_HTTP;
    http.buf = (unsigned char *)"PLAY";

    streams *sid = setup_stream(
        "?src=3&freq=11362&pol=h&sr=22000&msys=dvbs2&pids=0", &http);
    int res = sid ? start_play(sid, &http) : -999;
    absolute_switch = saved_switch;
    ASSERT(sid != NULL, "setup_stream returned NULL");
    ASSERT(res == -403, "start_play should return -403 for unmapped src");
    ASSERT(sid->rtsp_error == "Out-of-Range: src",
           "missing Out-of-Range: src body");

    return 0;
}

int test_start_play_unsupported_msys_returns_403() {
    setup_test_env();

    sockets http = {};
    http.enabled = 1;
    http.is_enabled = 1;
    http.sock = 1008;
    http.id = 8;
    http.sid = -1;
    http.type = TYPE_HTTP;
    http.buf = (unsigned char *)"PLAY";

    streams *sid = setup_stream("?freq=538&bw=8&msys=dvbt2&pids=0", &http);
    ASSERT(sid != NULL, "setup_stream returned NULL");
    int res = start_play(sid, &http);
    ASSERT(res == -403, "start_play should return -403 for bad msys");
    ASSERT(sid->rtsp_error == "Out-of-Range: msys",
           "missing Out-of-Range: msys body");

    return 0;
}

int test_start_play_pid_exhaustion_returns_503() {
    setup_test_env();

    sockets http1 = {};
    http1.enabled = 1;
    http1.is_enabled = 1;
    http1.sock = 1009;
    http1.id = 9;
    http1.sid = -1;
    http1.type = TYPE_HTTP;
    http1.buf = (unsigned char *)"PLAY";

    streams *sid1 = setup_stream(
        "?src=1&freq=11362&pol=h&sr=22000&msys=dvbs2&pids=0", &http1);
    ASSERT(sid1 != NULL, "first setup_stream returned NULL");
    ASSERT(start_play(sid1, &http1) == 0, "first start_play failed");

    int pid = 100;
    while (pid < 8000 && mark_pid_add(PID_STREAM_ID_UNDEFINED, 0, pid) == 0)
        pid++;
    ASSERT(pid < 8000, "could not fill the pid table");

    sockets http2 = {};
    http2.enabled = 1;
    http2.is_enabled = 1;
    http2.sock = 1010;
    http2.id = 10;
    http2.sid = -1;
    http2.type = TYPE_HTTP;
    http2.buf = (unsigned char *)"PLAY";

    // Same transponder as sid1, so no retune clears the pid table.
    streams *sid2 = setup_stream(
        "?src=1&freq=11362&pol=h&sr=22000&msys=dvbs2&pids=5000", &http2);
    ASSERT(sid2 != NULL, "second setup_stream returned NULL");
    int res = start_play(sid2, &http2);
    ASSERT(res == -503, "start_play should return -503 when pids run out");
    ASSERT(sid2->rtsp_error == "No-More: pids", "missing No-More: pids body");

    return 0;
}

int test_start_play_pinned_fe_mismatch_returns_403() {
    setup_test_env();
    // Pin fe=1 to adapter 0, like production init (rest unmapped).
    int16_t saved_fe[2 * MAX_ADAPTERS];
    memcpy(saved_fe, fe_map, sizeof(fe_map));
    memset(fe_map, -1, sizeof(fe_map));
    fe_map[0] = 0;

    sockets http = {};
    http.enabled = 1;
    http.is_enabled = 1;
    http.sock = 1011;
    http.id = 11;
    http.sid = -1;
    http.type = TYPE_HTTP;
    http.buf = (unsigned char *)"PLAY";

    streams *sid = setup_stream("?fe=1&freq=538&bw=8&msys=dvbt2&pids=0", &http);
    int res = sid ? start_play(sid, &http) : -999;
    memcpy(fe_map, saved_fe, sizeof(fe_map));
    ASSERT(sid != NULL, "setup_stream returned NULL");
    ASSERT(sid->tp.fe == 1, "fe was not parsed");
    ASSERT(res == -403, "start_play should return -403 for bad fe");
    ASSERT(sid->rtsp_error == "Out-of-Range: fe",
           "missing Out-of-Range: fe body");

    return 0;
}

int test_start_play_unmapped_fe_ignored_when_busy() {
    setup_test_env();

    sockets http1 = {};
    http1.enabled = 1;
    http1.is_enabled = 1;
    http1.sock = 1016;
    http1.id = 16;
    http1.sid = -1;
    http1.type = TYPE_HTTP;
    http1.buf = (unsigned char *)"PLAY";

    streams *sid1 = setup_stream(
        "?src=1&freq=11362&pol=h&sr=22000&msys=dvbs2&pids=0", &http1);
    ASSERT(sid1 != NULL, "first setup_stream returned NULL");
    ASSERT(start_play(sid1, &http1) == 0, "first start_play failed");

    int16_t saved_fe[2 * MAX_ADAPTERS];
    memcpy(saved_fe, fe_map, sizeof(fe_map));
    memset(fe_map, -1, sizeof(fe_map));

    sockets http2 = {};
    http2.enabled = 1;
    http2.is_enabled = 1;
    http2.sock = 1017;
    http2.id = 17;
    http2.sid = -1;
    http2.type = TYPE_HTTP;
    http2.buf = (unsigned char *)"PLAY";

    streams *s2 = setup_stream(
        "?fe=99&freq=12543&pol=v&sr=27500&msys=dvbs2&pids=0", &http2);
    int res = s2 ? start_play(s2, &http2) : -999;
    memcpy(fe_map, saved_fe, sizeof(fe_map));
    ASSERT(s2 != NULL, "second setup_stream returned NULL");
    ASSERT(res == -503, "unmapped fe must not shadow busy tuners");
    ASSERT(s2->rtsp_error == "No-More: frontends",
           "missing No-More: frontends body");

    return 0;
}

int mock_adapter_tune_403(int aid, transponder *tp) {
    (void)aid;
    (void)tp;
    return -403;
}

int test_start_play_tune_freq_error_returns_403() {
    setup_test_env();
    a[0]->tune = mock_adapter_tune_403;

    sockets http = {};
    http.enabled = 1;
    http.is_enabled = 1;
    http.sock = 1012;
    http.id = 12;
    http.sid = -1;
    http.type = TYPE_HTTP;
    http.buf = (unsigned char *)"PLAY";

    streams *sid = setup_stream(
        "?src=1&freq=11362&pol=h&sr=22000&msys=dvbs2&pids=0,16", &http);
    ASSERT(sid != NULL, "setup_stream returned NULL");
    int res = start_play(sid, &http);
    ASSERT(res == -403, "tune failure should surface as -403");
    ASSERT(sid->rtsp_error == "Out-of-Range: freq",
           "missing Out-of-Range: freq body");

    return 0;
}

int mock_adapter_tune_500(int aid, transponder *tp) {
    (void)aid;
    (void)tp;
    return -500;
}

int test_tune_missing_adapter_returns_500() {
    setup_test_env();
    ASSERT(tune(5, 0) == -500, "tune must return -500 without an adapter");

    return 0;
}

int test_start_play_tune_error_returns_500() {
    setup_test_env();
    a[0]->tune = mock_adapter_tune_500;

    sockets http = {};
    http.enabled = 1;
    http.is_enabled = 1;
    http.sock = 1013;
    http.id = 13;
    http.sid = -1;
    http.type = TYPE_HTTP;
    http.buf = (unsigned char *)"PLAY";

    streams *sid = setup_stream(
        "?src=1&freq=11362&pol=h&sr=22000&msys=dvbs2&pids=0,16", &http);
    ASSERT(sid != NULL, "setup_stream returned NULL");
    int res = start_play(sid, &http);
    ASSERT(res == -500, "tune failure should surface as -500");
    ASSERT(sid->rtsp_error.empty(), "500 must carry no body");

    return 0;
}

int test_set_adapter_parameters_slave_conflict() {
    setup_test_env();

    sockets http1 = {};
    http1.enabled = 1;
    http1.is_enabled = 1;
    http1.sock = 1014;
    http1.id = 14;
    http1.sid = -1;
    http1.type = TYPE_HTTP;
    http1.buf = (unsigned char *)"PLAY";

    streams *sid1 = setup_stream(
        "?src=1&freq=11362&pol=h&sr=22000&msys=dvbs2&pids=0", &http1);
    ASSERT(sid1 != NULL, "first setup_stream returned NULL");
    ASSERT(start_play(sid1, &http1) == 0, "first start_play failed");

    sockets http2 = {};
    http2.enabled = 1;
    http2.is_enabled = 1;
    http2.sock = 1015;
    http2.id = 15;
    http2.sid = -1;
    http2.type = TYPE_HTTP;
    http2.buf = (unsigned char *)"PLAY";

    streams *s2 = setup_stream(
        "?src=1&freq=12543&pol=v&sr=27500&msys=dvbs2&pids=0", &http2);
    ASSERT(s2 != NULL, "second setup_stream returned NULL");
    ASSERT(set_adapter_parameters(0, s2->sid, &s2->tp) == -2,
           "slave retune must return -2");

    return 0;
}

int test_streams_full() {
    setup_test_env();
    ASSERT(!streams_full(), "streams should not be full initially");
    int ids[MAX_STREAMS];
    for (int i = 0; i < MAX_STREAMS; i++) {
        ids[i] = streams_add();
        ASSERT(ids[i] >= 0, "streams_add failed before MAX_STREAMS");
        streams *s = get_sid(ids[i]);
        s->rtcp = s->rtcp_sock = s->sock = -1;
        s->seq = 0;
    }
    ASSERT(streams_full(), "streams should be full");
    ASSERT(streams_add() == -1, "streams_add should fail when full");
    close_stream(ids[0]);
    ASSERT(!streams_full(), "streams should not be full after close");

    return 0;
}

static std::string captured_rtsp;
static ssize_t capture_writev(int fd, const struct iovec *io, int len) {
    (void)fd;
    ssize_t total = 0;
    for (int i = 0; i < len; i++) {
        captured_rtsp.append((const char *)io[i].iov_base, io[i].iov_len);
        total += (ssize_t)io[i].iov_len;
    }
    return total;
}

// Runs one RTSP request through read_rtsp and captures the raw response.
static int run_rtsp_request(const char *req, std::string &out) {
    int sp[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sp) != 0)
        return -1;
    int id = sockets_add(sp[0], NULL, -1, TYPE_RTSP, NULL, NULL, NULL);
    if (id < 0) {
        close(sp[0]);
        close(sp[1]);
        return -1;
    }
    sockets *s = get_sockets(id);
    char buf[1024];
    snprintf(buf, sizeof(buf), "%s", req);
    s->buf = (unsigned char *)buf;
    s->rlen = strlen(buf);
    mywritev saved = _writev;
    captured_rtsp.clear();
    _writev = capture_writev;
    read_rtsp(s);
    _writev = saved;
    out = captured_rtsp;
    sockets_del(id);
    close(sp[1]);
    return 0;
}

int test_read_rtsp_sessions_exhausted_returns_503() {
    setup_test_env();
    for (int i = 0; i < MAX_STREAMS; i++)
        ASSERT(streams_add() >= 0, "streams_add failed before MAX_STREAMS");
    ASSERT(streams_full(), "streams should be full");

    std::string out;
    ASSERT(
        run_rtsp_request(
            "PLAY rtsp://127.0.0.1/?freq=11362&msys=dvbs2&pids=0 RTSP/1.0\r\n"
            "CSeq: 7\r\n\r\n",
            out) == 0,
        "request helper failed");
    ASSERT(out.find("RTSP/1.0 503 Service Unavailable") != std::string::npos,
           "missing 503 status");
    ASSERT(out.find("Content-Type: text/parameters") != std::string::npos,
           "missing Content-Type");
    ASSERT(out.find("No-More: sessions") != std::string::npos,
           "missing No-More: sessions");
    ASSERT(out.find("CSeq: 7") != std::string::npos, "missing CSeq echo");

    return 0;
}

int test_read_rtsp_options_keeps_public() {
    setup_test_env();
    std::string out;
    ASSERT(run_rtsp_request(
               "OPTIONS rtsp://127.0.0.1/ RTSP/1.0\r\nCSeq: 3\r\n\r\n", out) ==
               0,
           "request helper failed");
    ASSERT(out.find("RTSP/1.0 200 OK") != std::string::npos, "missing 200");
    ASSERT(out.find("Public: OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN") !=
               std::string::npos,
           "OPTIONS must keep Public");

    return 0;
}

int test_read_rtsp_describe_unknown_has_no_public() {
    setup_test_env();
    std::string out;
    ASSERT(run_rtsp_request(
               "DESCRIBE rtsp://127.0.0.1/stream=99 RTSP/1.0\r\nCSeq: 4\r\n"
               "Accept: application/sdp\r\n\r\n",
               out) == 0,
           "request helper failed");
    ASSERT(out.find("RTSP/1.0 404 Not Found") != std::string::npos,
           "missing 404 status");
    ASSERT(out.find("Public:") == std::string::npos,
           "errors must not carry Public");

    return 0;
}

int test_read_rtsp_bogus_session_when_full_returns_454() {
    setup_test_env();
    for (int i = 0; i < MAX_STREAMS; i++)
        ASSERT(streams_add() >= 0, "streams_add failed before MAX_STREAMS");
    for (int i = 0; i < MAX_STREAMS; i++)
        get_sid(i)->ssrc = 1000 + i;

    std::string out;
    ASSERT(
        run_rtsp_request(
            "PLAY rtsp://127.0.0.1/?freq=11362&msys=dvbs2&pids=0 RTSP/1.0\r\n"
            "CSeq: 11\r\nSession: 99999\r\n\r\n",
            out) == 0,
        "request helper failed");
    ASSERT(out.find("RTSP/1.0 454 Session Not Found") != std::string::npos,
           "bogus session must stay 454 when full");
    ASSERT(out.find("No-More:") == std::string::npos,
           "must not report sessions");

    return 0;
}

int test_read_rtsp_play_while_busy_returns_503() {
    setup_test_env();

    sockets http1 = {};
    http1.enabled = 1;
    http1.is_enabled = 1;
    http1.sock = 1018;
    http1.id = 18;
    http1.sid = -1;
    http1.type = TYPE_HTTP;
    http1.buf = (unsigned char *)"PLAY";

    streams *sid1 = setup_stream(
        "?src=1&freq=11362&pol=h&sr=22000&msys=dvbs2&pids=0", &http1);
    ASSERT(sid1 != NULL, "first setup_stream returned NULL");
    ASSERT(start_play(sid1, &http1) == 0, "first start_play failed");

    int saved_rtp = opts.start_rtp;
    char *saved_disc = opts.disc_host;
    opts.start_rtp = 45600;
    opts.disc_host = (char *)"127.0.0.1"; // set at startup in production
    std::string out;
    int rc = run_rtsp_request(
        "PLAY rtsp://127.0.0.1/?src=1&freq=12543&pol=v&sr=27500&msys="
        "dvbs2&pids=0 RTSP/1.0\r\n"
        "CSeq: 9\r\n"
        "Transport: RTP/AVP;unicast;client_port=46100-46101\r\n\r\n",
        out);
    opts.start_rtp = saved_rtp;
    opts.disc_host = saved_disc;
    ASSERT(rc == 0, "request helper failed");
    ASSERT(out.find("RTSP/1.0 503 Service Unavailable") != std::string::npos,
           "missing 503 status");
    ASSERT(out.find("No-More: frontends") != std::string::npos,
           "missing No-More: frontends");
    ASSERT(out.find("Content-Type: text/parameters") != std::string::npos,
           "missing Content-Type");

    return 0;
}

int main() {
    opts.log = 1;
    opts.debug = 255;
    strcpy(thread_info[thread_index].thread_name, "test_stream");

    TEST_FUNC(test_setup_stream_new(), "test setup_stream for a new stream");
    TEST_FUNC(test_setup_stream_existing(),
              "test setup_stream with an existing stream");
    TEST_FUNC(test_setup_stream_unspecified_vs_empty_pids(),
              "test setup_stream unspecified vs empty pids inheritance");
    TEST_FUNC(test_start_play_no_transport(),
              "test start_play returns error when transport is missing");
    TEST_FUNC(test_start_play_success(), "test start_play success under HTTP");
    TEST_FUNC(test_start_play_all_tuners_busy_returns_503(),
              "test start_play returns 503 when all tuners are busy");
    TEST_FUNC(test_start_play_unmapped_src_returns_403(),
              "test start_play returns 403 for unmapped src");
    TEST_FUNC(test_start_play_unsupported_msys_returns_403(),
              "test start_play returns 403 for unsupported msys");
    TEST_FUNC(test_start_play_pid_exhaustion_returns_503(),
              "test start_play returns 503 when pids run out");
    TEST_FUNC(test_streams_full(), "test streams_full tracks exhaustion");
    TEST_FUNC(test_start_play_pinned_fe_mismatch_returns_403(),
              "test start_play returns 403 for pinned fe mismatch");
    TEST_FUNC(test_start_play_tune_freq_error_returns_403(),
              "test tune freq failure surfaces as 403");
    TEST_FUNC(test_tune_missing_adapter_returns_500(),
              "test tune without adapter returns 500");
    TEST_FUNC(test_start_play_tune_error_returns_500(),
              "test tune failure surfaces as bare 500");
    TEST_FUNC(test_set_adapter_parameters_slave_conflict(),
              "test slave retune returns -2");
    TEST_FUNC(test_read_rtsp_sessions_exhausted_returns_503(),
              "test read_rtsp emits 503 with No-More sessions");
    TEST_FUNC(test_read_rtsp_options_keeps_public(),
              "test OPTIONS response keeps Public");
    TEST_FUNC(test_read_rtsp_describe_unknown_has_no_public(),
              "test 404 response carries no Public");
    TEST_FUNC(test_start_play_unmapped_fe_ignored_when_busy(),
              "test unmapped fe ignored when busy");
    TEST_FUNC(test_read_rtsp_bogus_session_when_full_returns_454(),
              "test bogus session stays 454 when full");
    TEST_FUNC(test_read_rtsp_play_while_busy_returns_503(),
              "test read_rtsp emits 503 while busy");
    TEST_FUNC(test_decode_transport_port_change_no_deadlock(),
              "test decode_transport does not deadlock on a port change");
#ifndef DISABLE_SRT
    TEST_FUNC(test_flush_stream_srt_accounts_bw(),
              "test SRT flush_stream updates bw counters");
#endif

    fflush(stdout);
    free_all();
    return 0;
}
