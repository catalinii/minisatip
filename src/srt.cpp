/*
 * Copyright (C) 2014-2020 Catalin Toda <catalinii@yahoo.com>
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
#include "srt.h"

#ifndef DISABLE_SRT
#include "minisatip.h"
#include "socketworks.h"
#include "stream.h"
#include <atomic>
#include <mutex>
#include <srt/srt.h>
#include <stdio.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <unordered_map>

#define DEFAULT_LOG LOG_STREAM
#define SRT_PENDING_MAX 64

// Mutex guards map access only, never srt_* calls (SRT lock-order safety).
static std::atomic<SRTSOCKET> srt_listener_sock{SRT_INVALID_SOCK};
static int srt_listener_udp_fd = -1;
static std::mutex srt_pending_mutex;
static std::unordered_map<std::string, SRTSOCKET> srt_pending;

// Check if SRT listener is initialized
int srt_listener_is_init() { return srt_listener_sock != SRT_INVALID_SOCK; }

// Check if SRT socket is in connected state
int srt_socket_is_connected(SRTSOCKET sock) {
    if (sock == SRT_INVALID_SOCK)
        return 0;
    SRT_SOCKSTATUS state = srt_getsockstate(sock);
    return state == SRTS_CONNECTED;
}

// Try to match a pending accepted socket to a stream by streamid.
// Called from decode_transport_srt when a stream is ready.
// Returns the accepted SRTSOCKET or SRT_INVALID_SOCK if not found.
SRTSOCKET srt_pending_take(const std::string &streamid) {
    std::lock_guard<std::mutex> lock(srt_pending_mutex);
    auto it = srt_pending.find(streamid);
    if (it == srt_pending.end())
        return SRT_INVALID_SOCK;
    SRTSOCKET s = it->second;
    srt_pending.erase(it);
    return s;
}

// Return a connected SRT socket to the pending pool for reuse.
// If the socket is not connected, close it instead.
void srt_pending_return(SRTSOCKET sock, const std::string &streamid) {
    if (sock == SRT_INVALID_SOCK)
        return;
    // Only reuse connected sockets
    if (!srt_socket_is_connected(sock)) {
        srt_close(sock);
        LOG("SRT socket %d closed (not connected), streamid='%s'", sock,
            streamid.c_str());
        return;
    }
    SRTSOCKET old = SRT_INVALID_SOCK;
    {
        std::lock_guard<std::mutex> lock(srt_pending_mutex);
        auto it = srt_pending.find(streamid);
        if (it != srt_pending.end() && it->second != sock) {
            old = it->second;
            srt_pending.erase(it);
        }
        srt_pending[streamid] = sock;
    }
    if (old != SRT_INVALID_SOCK)
        srt_close(old);
    LOG("SRT socket %d returned to pending pool for reuse, streamid='%s'", sock,
        streamid.c_str());
}

// Pre-accept callback function - called by SRT during handshake for each
// incoming connection
static int srt_accept_poll(void *opaq, SRTSOCKET ns, int hsversion,
                           const struct sockaddr *peeraddr,
                           const char *streamid) {
    if (!streamid || streamid[0] == '\0') {
        LOG("SRT accept: empty streamid, rejecting connection");
        return -1; // reject
    }
    SRTSOCKET old = SRT_INVALID_SOCK;
    size_t pending = 0;
    {
        std::lock_guard<std::mutex> lock(srt_pending_mutex);
        if (srt_pending.size() >= SRT_PENDING_MAX) {
            LOG("SRT accept: pending pool full (%d), rejecting connection",
                SRT_PENDING_MAX);
            return -1; // reject
        }
        auto it = srt_pending.find(streamid);
        if (it != srt_pending.end() && it->second != ns) {
            old = it->second;
            srt_pending.erase(it);
        }
        // No waiting stream found, queue in pending
        srt_pending[streamid] = ns;
        pending = srt_pending.size();
    }
    if (old != SRT_INVALID_SOCK)
        srt_close(old);
    LOG("SRT accepted connection srt_sock=%d, streamid='%s' (%zu pending)", ns,
        streamid, pending);
    return 0; // accept
}

// The callback uses accepted sockets directly, but each still occupies an
// accept-queue slot until srt_accept pops it: drain the queue so the
// backlog never fills. srt_close wakes the blocked accept on shutdown.
static void srt_accept_drain() {
    while (srt_listener_sock != SRT_INVALID_SOCK) {
        if (srt_accept(srt_listener_sock, NULL, NULL) == SRT_INVALID_SOCK)
            break;
    }
}

int srt_listener_init() {
    // Step 1: Create UDP socket on the RTSP port (TCP and UDP are separate)
    srt_listener_udp_fd =
        udp_bind(opts.bind, opts.rtsp_port, opts.use_ipv4_only);
    if (srt_listener_udp_fd < 0)
        LOG_AND_RETURN(-1, "SRT listener: UDP bind failed on port %d",
                       opts.rtsp_port);

    // Step 2: Create SRT socket
    srt_listener_sock = srt_create_socket();
    if (srt_listener_sock == SRT_INVALID_SOCK) {
        close(srt_listener_udp_fd);
        srt_listener_udp_fd = -1;
        LOG_AND_RETURN(-1, "SRT listener: srt_create_socket failed: %s",
                       srt_getlasterror_str());
    }

    // Step 3: Configure non-blocking, acquire UDP fd, listen
    int recv_no = 0;
    srt_setsockflag(srt_listener_sock, SRTO_RCVSYN, &recv_no, sizeof(recv_no));

    if (srt_bind_acquire(srt_listener_sock, srt_listener_udp_fd) == SRT_ERROR) {
        close(srt_listener_udp_fd);
        srt_listener_udp_fd = -1;
        srt_close(srt_listener_sock);
        srt_listener_sock = SRT_INVALID_SOCK;
        LOG_AND_RETURN(-1, "SRT listener: srt_bind_acquire failed: %s",
                       srt_getlasterror_str());
    }

    // Step 4: Register the pre-accept hook — must be done before srt_listen()
    if (srt_listen_callback(srt_listener_sock, srt_accept_poll, NULL) ==
        SRT_ERROR) {
        srt_close(srt_listener_sock);
        srt_listener_sock = SRT_INVALID_SOCK;
        srt_listener_udp_fd = -1;
        LOG_AND_RETURN(-1, "SRT listener: srt_listen_callback failed: %s",
                       srt_getlasterror_str());
    }

    if (srt_listen(srt_listener_sock, 10) == SRT_ERROR) {
        srt_close(srt_listener_sock);
        srt_listener_sock = SRT_INVALID_SOCK;
        srt_listener_udp_fd = -1;
        LOG_AND_RETURN(-1, "SRT listener: srt_listen failed: %s",
                       srt_getlasterror_str());
    }

    {
        std::lock_guard<std::mutex> lock(srt_pending_mutex);
        srt_pending.clear();
    }
    std::thread(srt_accept_drain).detach();
    LOG("SRT listener started on port %d, srt_sock=%d, udp_fd=%d",
        opts.rtsp_port, srt_listener_sock.load(), srt_listener_udp_fd);
    return 0;
}

void srt_listener_close() {
    if (srt_listener_sock != SRT_INVALID_SOCK) {
        srt_close(srt_listener_sock); // closes acquired udp_fd too
        srt_listener_sock = SRT_INVALID_SOCK;
        srt_listener_udp_fd = -1;
    }
    // Close any pending accepted sockets
    std::unordered_map<std::string, SRTSOCKET> dropped;
    {
        std::lock_guard<std::mutex> lock(srt_pending_mutex);
        dropped.swap(srt_pending);
    }
    for (auto &p : dropped)
        srt_close(p.second);
}

#endif // DISABLE_SRT
