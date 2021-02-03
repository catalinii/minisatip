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

#include "dvb.h"
#include "minisatip.h"
#include "socketworks.h"
#include "utils.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ucontext.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

/*
// external references of utils.c
int run_loop;
char pid_file[50];
__thread pthread_t tid;
char app_name[] = "test";
__thread char *strcpy(thread_name, "test");;

struct struct_opts opts;
*/

#define MAX_HASH 100
#define KEY_FOR_INDEX(i) (i)

int test_hash_table() {
    SHashTable h;
    int i;
    ASSERT(create_hash_table(&h, MAX_HASH - 10) == 0,
           "expected to create hash successfully");
    for (i = 0; i < MAX_HASH; i++) {
        ASSERT(0 == setItem(&h, KEY_FOR_INDEX(i), &i, sizeof(i)),
               "setItem failed");
    }

    ASSERT(0 == delItem(&h, KEY_FOR_INDEX(0)), "delItem failed");
    ASSERT(NULL == getItem(&h, KEY_FOR_INDEX(0)),
           "getItem is expected to fail after delete");
    for (i = 1; i < MAX_HASH; i++) {
        int *p = getItem(&h, KEY_FOR_INDEX(i));
        ASSERT(p != NULL, "getItem should not fail");
        ASSERT(i == *p, "getItem should return the correct key");
        ASSERT(0 == delItem(&h, KEY_FOR_INDEX(i)),
               "delItem should work for existing items");
    }

    for (i = 0; i < h.size; i++)
        if (h.items[i].len > 0)
            ASSERT(h.items[i].len, "len > 0 for existing items");

    LOG("Hash conflicts %d, len %d, size %d", h.conflicts, h.len, h.size);

    // no copy
    _setItem(&h, 0, (void *)23, 4, 0);
    i = 23;
    ASSERT(23 == (uintptr_t)getItem(&h, 0), "expected the correct item");
    setItem(&h, 0, &i, sizeof(i));

    free_hash(&h);

    return 0;
}
int main() {
    opts.log = 255;
    strcpy(thread_name, "test");
    TEST_FUNC(test_hash_table(), "testing hash table");
    return 0;
}
