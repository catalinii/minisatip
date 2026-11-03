#ifndef FIFO_H
#define FIFO_H

#include <atomic>
#include <stddef.h>
#include <stdint.h>

// Ring buffer with 64-bit never-wrapping indices; peek exposes
// the internal buffer for 0-copy writes straight into the ring.
// Indices are atomic for cross-thread producers/consumers, but
// compound push/peek/skip sequences still need external locking.

typedef struct fifo {
    const char *file;
    int line;
    std::atomic<uint64_t> read_index;
    std::atomic<uint64_t> write_index;
    uint64_t size;
    void *data;
} SFIFO;

int _create_fifo(SFIFO *f, size_t no, const char *file, int line);
void free_fifo(SFIFO *f);
static inline size_t fifo_available(SFIFO *f) {
    return f->size - (f->write_index.load(std::memory_order_acquire) -
                      f->read_index.load(std::memory_order_acquire));
}

static inline size_t fifo_used(SFIFO *f) {
    return f->write_index.load(std::memory_order_acquire) -
           f->read_index.load(std::memory_order_acquire);
}
static inline void fifo_skip_bytes(SFIFO *f, size_t len) {
    if (f->size > 0)
        f->read_index.fetch_add(len, std::memory_order_release);
}

size_t fifo_push_force(SFIFO *fifo, void *src, size_t len, int force);

size_t fifo_pop(SFIFO *fifo, void *dst, size_t len);
size_t fifo_pop_offset(SFIFO *fifo, void *dst, size_t len, uint64_t *offset);
size_t fifo_peek(SFIFO *fifo, void **dst, size_t len, size_t relative_offset);

uint32_t fifo_peek_32(SFIFO *fifo, uint64_t offset);

#define create_fifo(f, x) _create_fifo(f, x, __FILE__, __LINE__)
#define fifo_push(a, b, c) fifo_push_force(a, b, c, 0)
#define create_fifo(f, x) _create_fifo(f, x, __FILE__, __LINE__)
#endif
