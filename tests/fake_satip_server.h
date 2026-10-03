#ifndef FAKE_SATIP_SERVER_H
#define FAKE_SATIP_SERVER_H

#include "socketworks.h"
#include "utils/logging/logging.h"

#include <map>
#include <stdio.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <vector>

extern int satipc_reply(sockets *s);

// Scripted fake SAT>IP server for RTSP loop tests. The script maps the
// start of a client request to a canned reply; longest match wins, so a
// generic fallback ("PLAY ") can sit next to specific ones.
struct FakeSatipServer {
    std::map<std::string, std::string> script;
    std::vector<std::string> requests;
    std::string pending;

    const std::string *find_reply(const std::string &req) const {
        const std::string *best = nullptr;
        size_t best_len = 0;
        for (const auto &kv : script)
            if (kv.first.size() > best_len &&
                req.compare(0, kv.first.size(), kv.first) == 0) {
                best = &kv.second;
                best_len = kv.first.size();
            }
        return best;
    }

    // Runs one wave of pending client requests: records each, feeds the
    // mapped reply back through satipc_reply. Returns messages handled,
    // or -1 when a request matches nothing in the script.
    int pump(sockets *s, int fd, char *reply_buf, int reply_size) {
        char tmp[4096];
        ssize_t n;
        int count = 0;
        size_t pos;
        while ((n = read(fd, tmp, sizeof(tmp))) > 0)
            pending.append(tmp, (size_t)n);
        while ((pos = pending.find("\r\n\r\n")) != std::string::npos) {
            std::string req = pending.substr(0, pos + 4);
            pending.erase(0, pos + 4);
            const std::string *reply = find_reply(req);
            if (!reply)
                LOG_AND_RETURN(-1, "fake server has no reply for: %s",
                               req.c_str());
            requests.push_back(req);
            snprintf(reply_buf, reply_size, "%s", reply->c_str());
            s->rlen = strlen(reply_buf);
            satipc_reply(s);
            count++;
        }
        return count;
    }

    // Pumps until the client goes quiet. Fails past 100 messages, which
    // means the loop under test does not settle.
    int run(sockets *s, int fd, char *reply_buf, int reply_size) {
        int total = 0, n;
        while ((n = pump(s, fd, reply_buf, reply_size)) > 0) {
            total += n;
            if (total > 100)
                LOG_AND_RETURN(-1, "fake server did not settle");
        }
        return n < 0 ? -1 : total;
    }

    void dump() const {
        for (size_t i = 0; i < requests.size(); i++)
            LOG("fake server request %zu: %s", i, requests[i].c_str());
    }
};

#endif
