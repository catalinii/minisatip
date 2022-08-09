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
#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64

#include "utils.h"
#include "utils/testing.h"
#include "opts.h"
#include <string.h>
#include <sys/stat.h>

extern __thread char thread_name[100];

int test_mkdir_recursive() {
    const char *dir = "/tmp/minisatip/test";
    mkdir_recursive(dir);
    
    struct stat sb;
    ASSERT(stat(dir, &sb) == 0 && S_ISDIR(sb.st_mode), "stat should report that directory exists");

    return 0;
}

int main() {
    opts.log = 255;
    strcpy(thread_name, "test");
    TEST_FUNC(test_mkdir_recursive(), "testing directory creation");
    return 0;
}
