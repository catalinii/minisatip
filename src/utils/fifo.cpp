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

int _create_fifo(SFIFO *f, int no, const char *file, int line) {
    if (f->size > 0)
        return 0;
    memset(f, 0, sizeof(SFIFO));
    f->file = file;
    f->line = line;
    f->data = malloc(no);
    if (!f->data) {
        LOG_AND_RETURN(1, "Could not allocate FIFO %d", no);
    }
    f->size = no;
    return 0;
}

void free_fifo(SFIFO *f) {
    if (f->size <= 0)
        return;

    free(f->data);
    memset(f, 0, sizeof(SFIFO));
    return;
}

// push len bytes into the fifo only if there is space available
// force push allows overwriting data that was not read
// as it does not account for read index
int fifo_push_force(SFIFO *fifo, void *src, unsigned int len, int force) {
    uint32_t size = fifo->size;
    uint32_t available = fifo_available(fifo);

    if (len > size)
        return 0;
    if (!force && (len > available)) {
        LOG("Not enough space in FIFO created at %s:%d: need %u, "
            "available %u",
            fifo->file, fifo->line, len, available);
        return 0;
    }
    if ((fifo->read_index > 0) && (len > available))
        LOG("Overwriting %d bytes in the FIFO created at %s:%d",
            len - available, fifo->file, fifo->line);

    uint32_t off = fifo->write_index % size;
    uint32_t l = min(len, size - off);

    memcpy((char *)fifo->data + off, src, l);
    if (len > l)
        memcpy(fifo->data, (char *)src + l, len - l);
    fifo->write_index += len;
    return len;
}

// pops maximum len bytes from the buffer from the offset passed as pointer
uint32_t fifo_pop_offset(SFIFO *fifo, void *dst, unsigned int len,
                         uint64_t *offset) {
    uint64_t size = fifo->size;
    uint32_t l;
    if (fifo->write_index <= *offset)
        return 0;

    if ((*offset < fifo->write_index - size) && (fifo->write_index > size)) {
        if (*offset > 0) {
            LOG("Lost %jd bytes in fifo created at %s:%d",
                fifo->write_index - size - *offset, fifo->file, fifo->line);
        }

        *offset = fifo->write_index - size;
    }
    uint32_t available = fifo->write_index - *offset;

    if (len > available)
        len = available;

    uint32_t off = *offset % size;
    l = min(len, size - off);

    memcpy(dst, (char *)fifo->data + off, l);
    if (len > l)
        memcpy((char *)dst + l, fifo->data, len - l);
    *offset += len;
    return len;
}

// returns a pointer to the internal buffer with the size as the return value
// to get the entire buffer it's needed 2 calls:
// - first returns from the read_index + relative_offset until the end of buffer
// - second returns the pointer for the remaining size
uint32_t fifo_peek(SFIFO *fifo, void **dst, unsigned int len,
                   unsigned int relative_offset) {
    unsigned int size = fifo->size;
    uint32_t l;

    if (fifo->read_index + relative_offset > fifo->write_index)
        return 0;

    uint32_t off = (fifo->read_index + relative_offset) % size;
    l = min(len, size - off);
    *dst = (char *)fifo->data + off;
    return l;
}

uint32_t fifo_peek_32(SFIFO *fifo, uint64_t offset) {
    uint8_t *data = (uint8_t *)fifo->data;
    if (fifo->write_index - offset < LEN_SIZE)
        return 0;
    uint32_t size = fifo->size;
    return ((unsigned int)data[(offset + 0) % size] << 24) |
           ((unsigned int)data[(offset + 1) % size] << 16) |
           ((unsigned int)data[(offset + 2) % size] << 8) |
           (unsigned int)data[(offset + 3) % size];
}
