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
#include "api/symbols.h"
#include "api/variables.h"
#include "utils/hash_table.h"
#include "utils/logging/logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

SHashTable mem_alloc_table;
#define DEFAULT_LOG LOG_UTILS

typedef struct struct_memory_allocation {
    void *data;
    char *file;
    int line;
    int struct_size;
    int pointer_size;
    int current_size;
    int min_size;
    int size_bytes;
} SMEMALLOC;
static __thread int running;

int record_allocation(void *old, void *x, int size_bytes, int current_size,
                      int min_len, int pointer_size, int struct_size, char *f,
                      int l) {
    int rv = 0;
    if (running)
        return 0;
    // prevent hash table from calling record_allocation multiple times
    running = 1;
    if (old) {
        SMEMALLOC *s = getItem(&mem_alloc_table, (uintptr_t)old);
        if (s)
            rv = s->size_bytes;
        delItem(&mem_alloc_table, (uintptr_t)old);
    }
    SMEMALLOC s = {.data = x,
                   .file = f,
                   .line = l,
                   .size_bytes = size_bytes,
                   .current_size = current_size,
                   .min_size = min_len,
                   .pointer_size = pointer_size,
                   .struct_size = struct_size};
    _setItem(&mem_alloc_table, (uintptr_t)x, &s, sizeof(s), 1);
    running = 0;
    return rv;
}
void *malloc1(int a, char *f, int l, int record) {
    void *x = malloc(a);
    if (x)
        memset(x, 0, a);
    if (!mem_alloc_table.init)
        return x;
    alloc_sym[0].len = mem_alloc_table.size;
    if (record) {
        record_allocation(NULL, x, a, a, a, 1, 0, f, l);
    }
    LOGM("%s:%d malloc allocated %d bytes at %p", f, l, a, x);
    if (!x)
        LOG0("Failed allocating %d bytes of memory", a)
    return x;
}

void *realloc1(void *p, int a, char *f, int l, int record) {
    int old_mem = 0;
    void *x = realloc(p, a);
    if (record) {
        old_mem = record_allocation(p, x, a, a, a, 1, 0, f, l);
    }
    LOGM("%s:%d realloc allocated %d bytes from %d: %p -> %p", f, l, a, old_mem,
         p, x);
    if (!x) {
        LOG0("Failed allocating %d bytes of memory", a)
        if (!strcmp(f, "socketworks.c"))
            LOG0("Try to decrease the parameters -b and/or -B")
    }
    return x;
}

void free1(void *x, char *f, int l) {
    LOGM("%s:%d free called with argument %p", f, l, x);
    if (mem_alloc_table.init)
        delItem(&mem_alloc_table, (uintptr_t)x);
    free(x);
}

int _ensure_allocated(void **x, int struct_size, int pointer_size,
                      int ensure_length, int min_elements, char *file,
                      int line) {
    int do__realloc = 0, new_size = 0, current_size = 0;
    void *result = NULL;
    SMEMALLOC *s = getItem(&mem_alloc_table, (uintptr_t)*x);
    if (!*x || !s) {
        if (*x)
            LOG("%s:%d expected to find pointer %p in allocation table", file,
                line, x);
        do__realloc = 1;
    }
    if (s && s->current_size >= ensure_length) {
        LOGM("requested size %d is alreqdy allocated %d (%d bytes)",
             ensure_length, s->current_size, s->size_bytes);
        return 0;
    }

    new_size = ((ensure_length / min_elements) + 1) * min_elements;
    if (s && s->current_size < ensure_length) {
        do__realloc = 1;
        current_size = s->current_size;
    }
    if (do__realloc) {
        int cs = struct_size + pointer_size * current_size;
        int ns = struct_size + pointer_size * new_size;
        if (current_size == 0)
            cs = 0;
        result =
            realloc1(*x, struct_size + pointer_size * new_size, file, line, 0);
        if (!result)
            LOG_AND_RETURN(1, "Could not _reallocate %d bytes of memory to %d",
                           cs, ns);

        if (current_size > 0 || !*x) {
            memset(result + cs, 0, ns - cs);
        }
        record_allocation(*x, result, ns, new_size, min_elements, pointer_size,
                          struct_size, file, line);
        *x = result;
        LOGM("%s:%d _reallocated %d (%d) [struct %d, pointer %d] records from "
             "%d (%d) for "
             "index %d",
             file, line, new_size, ns, struct_size, pointer_size, current_size,
             cs, ensure_length);
    }
    return 0;
}

void init_alloc() { create_hash_table(&mem_alloc_table, 100); }
void free_alloc() { free_hash(&mem_alloc_table); }

char *get_alloc_info(int id, char *dest, int max_size) {
    dest[0] = 0;
    if (mem_alloc_table.items[id].len > 0) {
        SMEMALLOC *s = (SMEMALLOC *)mem_alloc_table.items[id].data;
        snprintf(dest, max_size, "%s:%d:%d", s->file, s->line, s->size_bytes);
    }

    return dest;
}

_symbols alloc_sym[] = {
    {"alloc", VAR_FUNCTION_STRING, (void *)&get_alloc_info, 0, 1, 0},
    {NULL, 0, NULL, 0, 0, 0}};
