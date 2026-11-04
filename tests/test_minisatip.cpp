/*
 * Copyright (C) 2014-2022 Catalin Toda <catalinii@yahoo.com>,
 *                         Sam Stenvall <neggelandia@gmail.com>,
 *                         et al.
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
#include "api/variables.h"
#include "ca.h"
#include "minisatip.h"
#include "socketworks.h"
#include "utils/testing.h"
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

int test_get_command_line_string() {
    int argc = 4;
    const char *argv[] = {"./minisatip", "-f", "-l", "general,ca"};
    char *actual_command_line = get_command_line_string(argc, argv);
    const char *expected_command_line = "./minisatip -f -l general,ca";

    ASSERT(strcmp(actual_command_line, expected_command_line) == 0,
           "command line string assembly failed");

    free(actual_command_line);

    return 0;
}

int test_version_var_eval() {
    ASSERT(app_version[0] != '\0', "version should not be empty");

    char var[] = "$version$";
    int len = is_var(var);
    ASSERT(len > 0, "$version$ should be recognized as a variable");

    char dest[256];
    memset(dest, 0, sizeof(dest));
    int nb = var_eval(var, len, dest, sizeof(dest) - 1);
    ASSERT(nb > 0, "var_eval should produce output for $version$");
    ASSERT(strcmp(dest, app_version) == 0,
           "$version$ should expand to the version string");

    return 0;
}

int test_log_module_mask() {
    struct {
        const char *name;
        unsigned int bit;
    } modules[] = {
        {"general", LOG_GENERAL},
        {"http", LOG_HTTP},
        {"socketworks", LOG_SOCKETWORKS},
        {"stream", LOG_STREAM},
        {"adapter", LOG_ADAPTER},
        {"satipc", LOG_SATIPC},
        {"pmt", LOG_PMT},
        {"tables", LOG_TABLES},
        {"dvbapi", LOG_DVBAPI},
        {"lock", LOG_LOCK},
        {"netceiver", LOG_NETCEIVER},
        {"ca", LOG_DVBCA},
        {"socket", LOG_SOCKET},
        {"utils", LOG_UTILS},
        {"dmx", LOG_DMX},
        {"ssdp", LOG_SSDP},
        {"dvb", LOG_DVB},
        {"ddci", LOG_DDCI},
    };
    unsigned int all = log_module_mask("all");

    for (const auto &m : modules) {
        ASSERT(log_module_mask(m.name) == m.bit,
               "module name enables the wrong log bit");
        ASSERT(all & m.bit, "\"all\" misses a module");
    }
    ASSERT(loglevels[std::size(modules)] == NULL,
           "loglevels has a module this test does not cover");
    ASSERT(log_module_mask("nope") == 0, "unknown module should be 0");
    return 0;
}

int test_escape_json_string() {
    char dest[64];

    // Standard RFC 8259 escapes: quotes, backslash, control chars, DEL.
    char s1[] = "a\"b\\c\n\r\t\b\f\x01\x1f\x7fZ";
    int n = escape_json_string(dest, sizeof(dest), s1, (int)strlen(s1));
    const char *exp1 =
        "\"a\\\"b\\\\c\\n\\r\\t\\b\\f\\u0001\\u001F\\u007FZ\"";
    ASSERT(strcmp(dest, exp1) == 0, "RFC 8259 escape sequence mismatch");
    ASSERT(n == (int)strlen(exp1), "returned length should match strlen(dest)");

    // sl > dl must not overflow dest (heap-allocated so ASan checks bounds).
    char *small = (char *)malloc(8);
    char long_src[] = "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c";
    int n_small = escape_json_string(small, 8, long_src, (int)strlen(long_src));
    // dl=8 allows at most 5 payload bytes -> 6-byte \u0001 is omitted cleanly.
    ASSERT(strcmp(small, "\"\"") == 0,
           "6-byte escape that does not fit in dl=8 should be omitted cleanly");
    ASSERT(n_small == 2, "empty quoted string has length 2");
    free(small);

    // Exact fit for one \u00XX escape: '"' + 6 + '"' + '\0' = 9 bytes.
    char *fit9 = (char *)malloc(9);
    int n9 = escape_json_string(fit9, 9, long_src, (int)strlen(long_src));
    ASSERT(strcmp(fit9, "\"\\u0001\"") == 0, "exact fit for \\u0001 in dl=9");
    ASSERT(n9 == 8, "returned length for \"\\u0001\" should be 8");
    free(fit9);

    // Boundary truncation on 2-byte escape: dl=5 holds at most 2 payload bytes.
    char *fit5 = (char *)malloc(5);
    char quotes[] = "\"\"";
    int n5 = escape_json_string(fit5, 5, quotes, 2);
    ASSERT(strcmp(fit5, "\"\\\"\"") == 0, "second quote should be omitted");
    ASSERT(n5 == 4, "returned length for \"\\\"\" should be 4");
    free(fit5);

    // Multibyte UTF-8 must not be split on truncation; invalid high byte -> U+FFFD.
    char utf8_src[] = "A\xC3\xBC";
    char utf8_trunc[5];
    escape_json_string(utf8_trunc, 5, utf8_src, 3);
    ASSERT(strcmp(utf8_trunc, "\"A\"") == 0,
           "2-byte UTF-8 char that does not fit in remaining 1 byte is omitted");
    char bad_utf8[] = "A\xFFZ";
    escape_json_string(dest, sizeof(dest), bad_utf8, 3);
    ASSERT(strcmp(dest, "\"A\xEF\xBF\xBDZ\"") == 0,
           "invalid UTF-8 byte should be replaced with U+FFFD");

    // dl < 3 returns 0 and NUL-terminates when dl > 0.
    char tiny[2] = {'X', 'Y'};
    ASSERT(escape_json_string(tiny, 2, s1, 3) == 0, "dl=2 should return 0");
    ASSERT(tiny[0] == '\0', "dl=2 should NUL-terminate dest[0]");

    return 0;
}

static std::string captured_http;
static ssize_t capture_http_writev(int fd, const struct iovec *io, int len) {
    (void)fd;
    ssize_t total = 0;
    for (int i = 0; i < len; i++) {
        captured_http.append((const char *)io[i].iov_base, io[i].iov_len);
        total += (ssize_t)io[i].iov_len;
    }
    return total;
}

int test_read_http_mmi_json() {
    ca_clear_mmi_messages();
    char dt_buf[64] = {};
    char tr_buf[64] = {};
    opts.datetime_current = dt_buf;
    opts.time_running = tr_buf;
    opts.disable_ssdp = 1;

    int sp[2];
    ASSERT(socketpair(AF_UNIX, SOCK_STREAM, 0, sp) == 0, "socketpair failed");
    int id = sockets_add(sp[0], NULL, -1, TYPE_HTTP, NULL, NULL, NULL);
    ASSERT(id >= 0, "sockets_add failed");
    sockets *s = get_sockets(id);
    ASSERT(s != NULL, "get_sockets returned NULL");

    char req[] = "GET /mmi.json HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n";
    s->buf = (unsigned char *)req;
    s->rlen = static_cast<int>(strlen(req));
    mywritev saved = _writev;
    captured_http.clear();
    _writev = capture_http_writev;
    read_http(s);
    _writev = saved;
    sockets_del(id);
    close(sp[1]);

    ASSERT(captured_http.find("HTTP/1.0 200 OK\r\n") != std::string::npos,
           "missing 200 OK status on /mmi.json");
    ASSERT(captured_http.find(
               "Content-Type: application/json; charset=utf-8\r\n") !=
               std::string::npos,
           "missing JSON Content-Type header on /mmi.json");
    ASSERT(captured_http.find("X-Content-Type-Options: nosniff\r\n") !=
               std::string::npos,
           "missing X-Content-Type-Options: nosniff header on /mmi.json");
    ASSERT(captured_http.find("Access-Control-Allow-Origin: *\r\n") !=
               std::string::npos,
           "missing CORS header on /mmi.json");
    ASSERT(captured_http.find(
               "\r\n\r\n{\n\"last_changed\":0,\n\"last_seq\":0,\n"
               "\"messages\":[]\n}\n") != std::string::npos,
           "missing expected JSON body on /mmi.json");
    return 0;
}

int main() {
    TEST_FUNC(test_get_command_line_string(), "test get_command_line_string()");
    TEST_FUNC(test_log_module_mask(), "test -l/-v module name to log bit");
    TEST_FUNC(test_version_var_eval(), "test $version$ expansion");
    TEST_FUNC(test_escape_json_string(), "test escape_json_string()");
    TEST_FUNC(test_read_http_mmi_json(), "test GET /mmi.json via read_http()");

    return 0;
}
