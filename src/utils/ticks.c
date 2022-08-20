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
#include "utils/ticks.h"

#include <time.h>

int64_t init_tick;

int64_t getTick() { // ms
    int64_t theTick;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    theTick = ts.tv_nsec / 1000000;
    theTick += ts.tv_sec * 1000;
    if (init_tick == 0)
        init_tick = theTick;
    return theTick - init_tick;
}

int64_t getTickUs() {
    int64_t utime;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    utime = ((int64_t)ts.tv_sec) * 1000000 + ts.tv_nsec / 1000;
    return utime;
}
