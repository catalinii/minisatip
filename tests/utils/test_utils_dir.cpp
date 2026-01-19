/*
 * Copyright (C) 2014-2024 Catalin Toda <catalinii@yahoo.com> et al
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
#include "opts.h"
#include "utils.h"
#include "utils/testing.h"
#include <string.h>

// This doesn't really test anything, visual inspection is required to verify
// the results
int test_hexdump() {
    const char *data = "some string to \x11\x12\x13\x14 on two lines";

    int i;
    for (i = 1; i <= 32; i++) {
        _hexdump(NULL, (void *)data, i);
    }

    return 0;
}

int main() {
    opts.log = 255;
    strcpy(thread_info[thread_index].thread_name, "test_hexdump");
    TEST_FUNC(test_hexdump(), "test hexdump");
    return 0;
}
