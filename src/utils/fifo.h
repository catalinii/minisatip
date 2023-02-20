#ifndef FIFO_H
#define FIFO_H
#define _GNU_SOURCE

#include "utils/mutex.h"
#include <stdint.h>

// Circular Buffer/Ring Buffer implementation:
// Uses 64 bit read and write index and prevents wrapping to be able to use
// modulo operation. Allows also to peek from an offset related to the
// read_index and exposes the internal buffer to enable 0-copy use cases (write
// to a handle)

typedef struct fifo {
    char init;
    char *file;
    int line;
    uint64_t read_index;
    uint64_t write_index;
    uint64_t size;
    void *data;
} SFIFO;

int _create_fifo(SFIFO *f, int no, char *file, int line);
void free_fifo(SFIFO *f);
static inline int fifo_available(SFIFO *f) {
    return f->size - (f->write_index - f->read_index);
}

static inline int fifo_used(SFIFO *f) { return f->write_index - f->read_index; }

int fifo_push_force(SFIFO *fifo, void *src, unsigned int len, int force);

uint32_t fifo_pop_offset(SFIFO *fifo, void *dst, unsigned int len,
                         uint64_t *offset);
uint32_t fifo_peek(SFIFO *fifo, void **dst, unsigned int len,
                   unsigned int relative_offset);

int fifo_push_record(SFIFO *fifo, void *src, uint32_t len);

uint32_t fifo_pop_record(SFIFO *fifo, void *dst, uint32_t len);

#define create_fifo(f, x) _create_fifo(f, x, __FILE__, __LINE__)
#define fifo_push(a, b, c) fifo_push_force(a, b, c, 0)
#define fifo_pop(a, b, c) fifo_pop_offset(a, b, c, &((a)->read_index))
#define create_fifo(f, x) _create_fifo(f, x, __FILE__, __LINE__)
#endif
