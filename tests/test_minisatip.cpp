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

int main() {
    TEST_FUNC(test_get_command_line_string(), "test get_command_line_string()");
    TEST_FUNC(test_log_module_mask(), "test -l/-v module name to log bit");
    TEST_FUNC(test_version_var_eval(), "test $version$ expansion");

    return 0;
}
