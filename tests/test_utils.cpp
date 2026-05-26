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

// write a test for is_rtsp_response
int test_is_rtsp_response() {
    char header1[] = "RTSP/1.0 200 OK\r\n\r\n";
    char header2[] = "RTSP/1.0 200 OK\r\nContent-Length: 23\r\n\r\n";
    char header3[] = "RTSP/1.0 200 OK\r\nContent-Length: 4\r\n\r\ntest";
    char header4[] =
        "OPTIONS rtsp://1.2.3.4 RTSP/1.0\r\nContent-Length: 2\r\n\r\nbu";
    ASSERT_EQUAL(1, is_rtsp_response(header1, sizeof(header1)),
                 "Expected 1 for header1");
    ASSERT_EQUAL(0, is_rtsp_response(header2, sizeof(header2)),
                 "Expected 0 for header2");
    ASSERT_EQUAL(1, is_rtsp_response(header3, sizeof(header3)),
                 "Expected 1 for header3");
    ASSERT_EQUAL(1, is_rtsp_request(header4, sizeof(header4)),
                 "Expected1 for header4");
    return 0;
}

int test_split() {
    auto res1 = split("a,b,c", ',');
    ASSERT_EQUAL(res1.size(), 3, "Expected 3 elements");
    ASSERT(res1[0] == "a", "Expected 'a'");
    ASSERT(res1[1] == "b", "Expected 'b'");
    ASSERT(res1[2] == "c", "Expected 'c'");

    auto res2 = split("  a , b \n, c\r", ',');
    ASSERT_EQUAL(res2.size(), 3,
                 "Expected 3 elements after filtering control characters");
    ASSERT(res2[0] == "a", "Expected 'a'");
    ASSERT(res2[1] == "b", "Expected 'b'");
    ASSERT(res2[2] == "c", "Expected 'c'");

    auto res3 = split("a,,b", ',');
    ASSERT_EQUAL(res3.size(), 2, "Expected 2 elements, empty elements ignored");
    ASSERT(res3[0] == "a", "Expected 'a'");
    ASSERT(res3[1] == "b", "Expected 'b'");

    return 0;
}

int test_strip() {
    ASSERT(strip("  hello") == "hello", "Expected leading spaces stripped");
    ASSERT(strip("world  ") == "world  ",
           "Expected trailing spaces not stripped");
    ASSERT(strip("   ") == "", "Expected empty string view for all spaces");
    ASSERT(strip("") == "", "Expected empty string view for empty input");
    return 0;
}

int test_map_int() {
    ASSERT_EQUAL(map_int("123"), 123, "Expected 123");
    ASSERT_EQUAL(map_int("  -456"), -456, "Expected -456");
    ASSERT_EQUAL(map_int("+789"), 789, "Expected 789");

    const char *const map[] = {"zero", "one", "two", nullptr};
    ASSERT_EQUAL(map_int("one", map), 1, "Expected index 1 for 'one'");
    ASSERT_EQUAL(map_int("two", map), 2, "Expected index 2 for 'two'");
    ASSERT_EQUAL(map_int("three", map), 0,
                 "Expected default 0 for invalid value");

    ASSERT_EQUAL(map_intd("three", map, 42), 42, "Expected default 42");

    return 0;
}

int test_check_strs() {
    const char *const map[] = {"zero", "one", "two", nullptr};
    ASSERT_EQUAL(check_strs("one", map, -1), 1, "Expected index 1");
    ASSERT_EQUAL(check_strs("two", map, -1), 2, "Expected index 2");
    ASSERT_EQUAL(check_strs("invalid", map, -1), -1, "Expected default -1");
    return 0;
}

int test_header_parameter() {
    std::vector<std::string_view> args1 = {"Session:", "12345"};
    ASSERT(header_parameter(args1, 0) == "12345", "Expected '12345'");

    std::vector<std::string_view> args2 = {"Session:54321"};
    ASSERT(header_parameter(args2, 0) == "54321", "Expected '54321'");

    std::vector<std::string_view> args3 = {"Session", ":", "abcde"};
    ASSERT(header_parameter(args3, 0) == "abcde", "Expected 'abcde'");

    return 0;
}

int main() {
    opts.log = 255;
    strcpy(thread_info[thread_index].thread_name, "test_utils");
    TEST_FUNC(test_mkdir_recursive(), "testing directory creation");
    TEST_FUNC(test_fifo(), "testing fifo");
    TEST_FUNC(test_is_rtsp_response(), "testing is_rtsp_response");
    TEST_FUNC(test_split(), "testing split");
    TEST_FUNC(test_strip(), "testing strip");
    TEST_FUNC(test_map_int(), "testing map_int");
    TEST_FUNC(test_check_strs(), "testing check_strs");
    TEST_FUNC(test_header_parameter(), "testing header_parameter");
    return 0;
}
