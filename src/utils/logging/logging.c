/*
 * Copyright (C) 2014-2022 Catalin Toda <catalinii@yahoo.com> et al
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
#include "utils/logging/logging.h"
#include "opts.h"
#include "utils.h"

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <syslog.h>

pthread_mutex_t log_mutex;
SThreadInfo thread_info[135];
__thread int thread_index;

void _log(const char *file, int line, const char *fmt, ...) {
    va_list arg;
    int len = 0, len1 = 0, both = 0;
    static int idx, times;
    char stid[102];
    static char output[2][2000]; // prints just the first 2000 bytes from
                                 // the message

    /* Check if the message should be logged */
    stid[0] = 0;
    pthread_mutex_lock(&log_mutex);
    snprintf(stid, sizeof(stid) - 1, " %s",
        thread_info[thread_index].thread_name);
    stid[sizeof(stid) - 1] = 0;

    if (!fmt) {
        printf("NULL format at %s:%d !!!!!", file, line);
        pthread_mutex_unlock(&log_mutex);
        return;
    }
    idx = 1 - idx;
    if (idx > 1)
        idx = 1;
    else if (idx < 0)
        idx = 0;
    if (opts.file_line && !opts.slog)
        len1 = snprintf(output[idx], sizeof(output[0]),
                        "[%s%s] %s:%d: ", get_current_timestamp_log(), stid,
                        file, line);
    else if (!opts.slog)
        len1 = snprintf(output[idx], sizeof(output[0]),
                        "[%s%s]: ", get_current_timestamp_log(), stid);
    else if (opts.file_line) {
        len1 = 0;
        output[idx][0] = '\0';
    }
    /* Write the error message */
    len = len1 =
        len1 < (int)sizeof(output[0]) ? len1 : (int)sizeof(output[0]) - 1;
    both = 0;
    va_start(arg, fmt);
    len += vsnprintf(output[idx] + len, sizeof(output[0]) - len, fmt, arg);
    va_end(arg);

    if (strcmp(output[idx] + len1, output[1 - idx] + len1) == 0)
        times++;
    else {
        if (times > 0) {
            both = 1;
            snprintf(output[1 - idx], sizeof(output[0]),
                     "Message repeated %d times", times);
        }
        times = 0;
    }

    if (both) {
        if (opts.slog)
            syslog(LOG_NOTICE, "%s", output[1 - idx]);
        else
            puts(output[1 - idx]);
        both = 0;
    }
    if (times == 0) {
        if (opts.slog)
            syslog(LOG_NOTICE, "%s", output[idx]);
        else
            puts(output[idx]);
    }
    fflush(stdout);
    pthread_mutex_unlock(&log_mutex);
}

char *get_current_timestamp_log(void) {
    static char date_str[200];
    struct timeval tv;
    struct tm *t;

    if (gettimeofday(&tv, NULL))
        return (char *)"01/01 00:00:20";
    t = localtime(&tv.tv_sec);
    if (!t)
        return (char *)"01/01 00:00:20";
    snprintf(date_str, sizeof(date_str), "%02d/%02d %02d:%02d:%02d.%03d",
             t->tm_mday, t->tm_mon + 1, t->tm_hour, t->tm_min, t->tm_sec,
             (int)(tv.tv_usec / 1000));
    return date_str;
}
