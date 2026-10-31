#ifndef FIFO_H
#define FIFO_H

#include <stddef.h>
#include <stdint.h>

// Circular Buffer/Ring Buffer implementation:
// Uses 64 bit read and write index and prevents wrapping to be able to use
// modulo operation. Allows also to peek from an offset related to the
// read_index and exposes the internal buffer to enable 0-copy use cases (write
// to a handle)

typedef struct fifo {
    const char *file;
    int line;
    uint64_t read_index;
    uint64_t write_index;
    uint64_t size;
    void *data;
} SFIFO;

int _create_fifo(SFIFO *f, size_t no, const char *file, int line);
void free_fifo(SFIFO *f);
static inline size_t fifo_available(SFIFO *f) {
    return f->size - (f->write_index - f->read_index);
}

static inline size_t fifo_used(SFIFO *f) {
    return f->write_index - f->read_index;
}
static inline void fifo_skip_bytes(SFIFO *f, size_t len) {
    if (f->size > 0)
        f->read_index += len;
}

size_t fifo_push_force(SFIFO *fifo, void *src, size_t len, int force);

size_t fifo_pop_offset(SFIFO *fifo, void *dst, size_t len, uint64_t *offset);
size_t fifo_peek(SFIFO *fifo, void **dst, size_t len, size_t relative_offset);

uint32_t fifo_peek_32(SFIFO *fifo, uint64_t offset);

#define create_fifo(f, x) _create_fifo(f, x, __FILE__, __LINE__)
#define fifo_push(a, b, c) fifo_push_force(a, b, c, 0)
#define fifo_pop(a, b, c) fifo_pop_offset(a, b, c, &((a)->read_index))
#define create_fifo(f, x) _create_fifo(f, x, __FILE__, __LINE__)
#endif
