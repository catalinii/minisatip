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
#include "utils/mutex.h"
#include "utils/logging/logging.h"
#include "utils/ticks.h"
#include "opts.h"
#include <string.h>

#undef DEFAULT_LOG
#define DEFAULT_LOG LOG_LOCK

int mutex_init(SMutex *mutex) {
    int rv;
    pthread_mutexattr_t attr;

    if (opts.no_threads)
        return 0;
    if (mutex->enabled)
        return 1;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    if ((rv = pthread_mutex_init(&mutex->mtx, &attr))) {
        LOG("mutex init %p failed with error %d %s", mutex, rv, strerror(rv));
        return rv;
    }

    mutex->create_time = getTick();
    mutex->enabled = 1;
    mutex->state = 0;
    LOG("Mutex init OK %p", mutex);
    return 0;
}

__thread SMutex *mutexes[100];
__thread int imtx = 0;

int mutex_lock1(char *FILE, int line, SMutex *mutex) {
    int rv;
    int64_t start_lock = 0;
    if (opts.no_threads)
        return 0;

    if (!mutex || !mutex->enabled) {
        LOGM("%s:%d Mutex not enabled %p", FILE, line, mutex);
        return 1;
    }
    if (mutex->enabled && mutex->state && pthread_self() != mutex->tid) {
        LOGM("%s:%d Locking mutex %p already locked at %s:%d tid %lx", FILE,
             line, mutex, mutex->file, mutex->line, mutex->tid);
        start_lock = getTick();
    } else
        LOGM("%s:%d Locking mutex %p", FILE, line, mutex);
    rv = pthread_mutex_lock(&mutex->mtx);
    if (!mutex->enabled && rv == 0) {
        pthread_mutex_unlock(&mutex->mtx);
        LOG("Mutex %p destroyed meanwhile", mutex);
        return 1;
    }
    if (rv != 0) {
        LOG("Mutex Lock %p failed", mutex);
        return rv;
    }
    if (start_lock > 0) {
        uint64_t ms = getTick() - start_lock;
        char *prev_file = "none";
        int prev_line = -1;
        if (mutex && mutex->enabled && mutex->file) {
            prev_file = mutex->file;
            prev_line = mutex->line;
        }
        LOGL(ms > 1000 ? LOG_GENERAL : LOG_LOCK,
             "%s:%d Locked %p after %ld ms, previously locked at: %s, line "
             "%d",
             FILE, line, mutex, ms, prev_file, prev_line);
    }
    mutex->file = FILE;
    mutex->line = line;
    mutex->state++;
    mutex->tid = pthread_self();
    mutex->lock_time = getTick();

    mutexes[imtx++] = mutex;
    return 0;
}
int mutex_unlock1(char *FILE, int line, SMutex *mutex) {
    int rv = -1;
    if (opts.no_threads)
        return 0;

    if (!mutex || mutex->enabled) {
        LOGM("%s:%d Unlocking mutex %p", FILE, line, mutex);
        if (mutex) {
            mutex->state--;
            rv = pthread_mutex_unlock(&mutex->mtx);
        }
    } else
        LOG("%s:%d Unlock disabled mutex %p", FILE, line, mutex);

    if (rv != 0 && rv != 1 && rv != -1) {
        LOGM("mutex_unlock failed at %s:%d: %d %s", FILE, line, rv,
             strerror(rv));
    }
    if (rv == 0 || rv == 1)
        rv = 0;

    if (rv != -1 && imtx > 0) {
        if ((imtx >= 1) && mutexes[imtx - 1] == mutex)
            imtx--;
        else if ((imtx >= 2) && mutexes[imtx - 2] == mutex) {
            mutexes[imtx - 2] = mutexes[imtx - 1];
            imtx--;
        } else
            LOG("mutex_leak: Expected %p got %p", mutex, mutexes[imtx - 1]);
    }
    return rv;
}

int mutex_destroy(SMutex *mutex) {
    int rv;
    if (opts.no_threads)
        return 0;
    if (!mutex || !mutex->enabled) {
        LOG("destroy disabled mutex %p", mutex);

        return 1;
    }
    mutex->enabled = 0;

    if ((imtx >= 1) && mutexes[imtx - 1] == mutex)
        imtx--;
    else if ((imtx >= 2) && mutexes[imtx - 2] == mutex) {
        mutexes[imtx - 2] = mutexes[imtx - 1];
        imtx--;
    }

    if ((rv = pthread_mutex_unlock(&mutex->mtx)) != 1 && rv != 0)
        LOG("%s: pthread_mutex_unlock 1 failed for %p with error %d %s",
            __FUNCTION__, mutex, rv, strerror(rv));

    // coverity[use : FALSE]
    if ((rv = pthread_mutex_unlock(&mutex->mtx)) != 1 && rv != 0)
        LOG("%s: pthread_mutex_unlock 2 failed for %p with error %d %s",
            __FUNCTION__, mutex, rv, strerror(rv));

    LOG("Destroying mutex %p", mutex);
    return 0;
}

void clean_mutexes() {
    int i;
    if (!imtx)
        return;
    if (opts.no_threads)
        return;
    //	LOG("mutex_leak: unlock %d mutexes", imtx);
    for (i = imtx - 1; i >= 0; i--) {
        if (!mutexes[i] || !mutexes[i]->enabled)
            continue;
        LOG("mutex_leak: %s unlocking mutex %p from %s:%d", __FUNCTION__,
            mutexes[i], mutexes[i]->file, mutexes[i]->line);
        mutex_unlock(mutexes[i]);
    }
    imtx = 0;
}
