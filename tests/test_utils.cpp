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

#include "opts.h"
#include "utils.h"
#include "utils/fifo.h"
#include "utils/testing.h"
#include <string.h>
#include <sys/stat.h>

int test_mkdir_recursive() {
    const char *dir = "/tmp/minisatip/test";
    mkdir_recursive(dir);

    struct stat sb;
    ASSERT(stat(dir, &sb) == 0 && S_ISDIR(sb.st_mode),
           "stat should report that directory exists");

    return 0;
}

int test_fifo() {
    SFIFO f;
    char buf[] = {0, 1, 2, 3, 4, 5};
    memset(&f, 0, sizeof(f));
    create_fifo(&f, 10);
    f.write_index = 5;
    f.read_index = 5;
    ASSERT(fifo_push_record(&f, buf, sizeof(buf)) == sizeof(buf),
           "Unable to push record");
    memset(buf, 0, sizeof(buf));
    ASSERT(sizeof(buf) == fifo_pop_record(&f, buf, sizeof(buf)),
           "Unable to pop record");
    int i;
    f.write_index = 8;
    f.read_index = 8;
    for (i = 0; i < sizeof(buf); i++)
        ASSERT_EQUAL(buf[i], i, "Expected value to match")
    ASSERT(fifo_push_record(&f, buf, sizeof(buf) + 1) == 0,
           "Expecting oversized record to fail");
    ASSERT(fifo_push_record(&f, buf, sizeof(buf)) == sizeof(buf),
           "Unable to push oversized record");
    ASSERT(0 == fifo_pop_record(&f, buf, sizeof(buf) - 1),
           "Failed to reject record read");
    ASSERT(sizeof(buf) == fifo_pop_record(&f, buf, sizeof(buf)),
           "Failed to read record when length wraps");

    for (i = 0; i < sizeof(buf); i++)
        ASSERT_EQUAL(buf[i], i, "Expected value to match")

    ASSERT(0 == fifo_pop(&f, buf, sizeof(buf)),
           "Expected 0 bytes to be popped from the fifo");

    // test force writing
    ASSERT(sizeof(buf) == fifo_push(&f, buf, sizeof(buf)),
           "Failed to write record");
    ASSERT(sizeof(buf) == fifo_push_force(&f, buf, sizeof(buf), 1),
           "Failed to write record with force");

    free_fifo(&f);

    return 0;
}

// write a test for is_rtsp_header
int test_is_rtsp_header() {
       char header1[] = "RTSP/1.0 200 OK\r\n\r\n";
       char header2[] = "RTSP/1.0 200 OK\r\nContent-Length: 23\r\n\r\n";
       char header3[] = "RTSP/1.0 200 OK\r\nContent-Length: 4\r\n\r\ntest";
       ASSERT_EQUAL(1, is_rtsp_header(header1, sizeof(header1)), "Expected 1 for header1");
       ASSERT_EQUAL(0, is_rtsp_header(header2, sizeof(header2)), "Expected 0 for header2");
       ASSERT_EQUAL(1, is_rtsp_header(header3, sizeof(header3)), "Expected 1 for header3");
       return 0;

}


int main() {
    opts.log = 255;
    strcpy(thread_info[thread_index].thread_name, "test_utils");
    TEST_FUNC(test_mkdir_recursive(), "testing directory creation");
    TEST_FUNC(test_fifo(), "testing fifo");
    TEST_FUNC(test_is_rtsp_header(), "testing is_rtsp_header");
    return 0;
}
