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
#include <unistd.h>

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
    a[0]->sys[0] = SYS_DVBS2;
    a[0]->sys[1] = SYS_UNDEFINED;
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

    fflush(stdout);
    free_all();
    return 0;
}
