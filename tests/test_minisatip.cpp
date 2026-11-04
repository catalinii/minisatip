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
#include "minisatip.h"
#include "utils/testing.h"
#include <stdlib.h>
#include <string.h>

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

int main() {
    TEST_FUNC(test_get_command_line_string(), "test get_command_line_string()");
    TEST_FUNC(test_log_module_mask(), "test -l/-v module name to log bit");
    TEST_FUNC(test_version_var_eval(), "test $version$ expansion");
    TEST_FUNC(test_escape_json_string(), "test escape_json_string()");

    return 0;
}
