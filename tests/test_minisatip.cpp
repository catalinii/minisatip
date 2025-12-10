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
#include "minisatip.h"
#include "utils/testing.h"
#include <stdlib.h>
#include <string.h>

int test_get_command_line_string() {
    int argc = 4;
    char *argv[] = {"./minisatip", "-f", "-l", "general,ca"};
    char *actual_command_line = get_command_line_string(argc, argv);
    const char *expected_command_line = "./minisatip -f -l general,ca";

    ASSERT(strcmp(actual_command_line, expected_command_line) == 0,
           "command line string assembly failed");

    free(actual_command_line);

    return 0;
}

int main() {
    TEST_FUNC(test_get_command_line_string(), "test get_command_line_string()");

    return 0;
}
