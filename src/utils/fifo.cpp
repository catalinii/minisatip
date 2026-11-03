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
#include "fifo.h"
#include "opts.h"
#include "utils.h"
#include "utils/logging/logging.h"

#include <stdlib.h>
#include <string.h>

#define min(a, b) (a < b) ? a : b
#define LEN_SIZE 4

int _create_fifo(SFIFO *f, size_t no, const char *file, int line) {
    if (f->size > 0)
        return 0;
    void *data = malloc(no);
    if (!data) {
        LOG_AND_RETURN(1, "Could not allocate FIFO %zu", no);
    }
    f->file = file;
    f->line = line;
    f->data = data;
    f->read_index.store(0, std::memory_order_relaxed);
    f->write_index.store(0, std::memory_order_relaxed);
    f->size = no;
    return 0;
}

void free_fifo(SFIFO *f) {
    if (f->size <= 0)
        return;

    free(f->data);
    f->data = nullptr;
    f->file = nullptr;
    f->line = 0;
    f->read_index.store(0, std::memory_order_relaxed);
    f->write_index.store(0, std::memory_order_relaxed);
    f->size = 0;
    return;
}

// push len bytes into the fifo only if there is space available
// force push allows overwriting data that was not read
// as it does not account for read index
size_t fifo_push_force(SFIFO *fifo, void *src, size_t len, int force) {
    size_t size = fifo->size;
    if (len == 0 || len > size)
        return 0;
    uint64_t wi = fifo->write_index.load(std::memory_order_acquire);
    uint64_t ri = fifo->read_index.load(std::memory_order_acquire);
    size_t available = size - (wi - ri);

    // Full without force: report 0, the caller owns
    // the reporting policy for refused bytes.
    if (!force && (len > available))
        return 0;
    if ((ri > 0) && (len > available))
        LOG("Overwriting %zu bytes in the FIFO created at %s:%d",
            len - available, fifo->file, fifo->line);

    size_t off = wi % size;
    size_t l = min(len, size - off);

    memcpy((char *)fifo->data + off, src, l);
    if (len > l)
        memcpy(fifo->data, (char *)src + l, len - l);
    fifo->write_index.fetch_add(len, std::memory_order_release);
    return len;
}

// pops maximum len bytes from the buffer from the offset passed as pointer
size_t fifo_pop_offset(SFIFO *fifo, void *dst, size_t len, uint64_t *offset) {
    uint64_t size = fifo->size;
    uint64_t wi = fifo->write_index.load(std::memory_order_acquire);
    size_t l;
    if (wi <= *offset)
        return 0;

    if ((*offset < wi - size) && (wi > size)) {
        if (*offset > 0) {
            LOG("Lost %jd bytes in fifo created at %s:%d",
                (intmax_t)(wi - size - *offset), fifo->file, fifo->line);
        }

        *offset = wi - size;
    }
    size_t available = wi - *offset;

    if (len > available)
        len = available;

    size_t off = *offset % size;
    l = min(len, size - off);

    memcpy(dst, (char *)fifo->data + off, l);
    if (len > l)
        memcpy((char *)dst + l, fifo->data, len - l);
    *offset += len;
    return len;
}

// Pops from the fifo's own read index, advancing it past the copy.
size_t fifo_pop(SFIFO *fifo, void *dst, size_t len) {
    uint64_t size = fifo->size;
    uint64_t wi = fifo->write_index.load(std::memory_order_acquire);
    uint64_t ri = fifo->read_index.load(std::memory_order_acquire);
    if (wi <= ri)
        return 0;

    if ((ri < wi - size) && (wi > size)) {
        if (ri > 0) {
            LOG("Lost %jd bytes in fifo created at %s:%d",
                (intmax_t)(wi - size - ri), fifo->file, fifo->line);
        }

        ri = wi - size;
    }
    size_t available = wi - ri;

    if (len > available)
        len = available;

    size_t off = ri % size;
    size_t l = min(len, size - off);

    memcpy(dst, (char *)fifo->data + off, l);
    if (len > l)
        memcpy((char *)dst + l, fifo->data, len - l);
    fifo->read_index.store(ri + len, std::memory_order_release);
    return len;
}

// returns a pointer to the internal buffer with the size as the return value
// to get the entire buffer it's needed 2 calls:
// - first returns from the read_index + relative_offset until the end of buffer
// - second returns the pointer for the remaining size
size_t fifo_peek(SFIFO *fifo, void **dst, size_t len, size_t relative_offset) {
    size_t size = fifo->size;
    uint64_t ri = fifo->read_index.load(std::memory_order_acquire);
    uint64_t wi = fifo->write_index.load(std::memory_order_acquire);
    size_t l;

    if (ri + relative_offset > wi)
        return 0;

    size_t off = (ri + relative_offset) % size;
    l = min(len, size - off);
    *dst = (char *)fifo->data + off;
    return l;
}

uint32_t fifo_peek_32(SFIFO *fifo, uint64_t offset) {
    uint8_t *data = (uint8_t *)fifo->data;
    uint64_t wi = fifo->write_index.load(std::memory_order_acquire);
    if (wi - offset < LEN_SIZE)
        return 0;
    uint64_t size = fifo->size;
    return ((unsigned int)data[(offset + 0) % size] << 24) |
           ((unsigned int)data[(offset + 1) % size] << 16) |
           ((unsigned int)data[(offset + 2) % size] << 8) |
           (unsigned int)data[(offset + 3) % size];
}
