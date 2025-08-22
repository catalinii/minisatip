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
#include "hash_table.h"
#include "opts.h"
#include "utils.h"
#include "utils/logging/logging.h"

#include <stdlib.h>
#include <string.h>

#define UNUSED_KEY 0

// Hash function from
// https://stackoverflow.com/questions/664014/what-integer-hash-function-are-good-that-accepts-an-integer-hash-key
static inline uint64_t hash_func(uint64_t x) {
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
}

int get_index_hash(SHashTable *hash, uint64_t key) {
    int pos;
    int start_pos = hash_func(key) % hash->size;

    pos = start_pos;
    do {
        if (HASH_ITEM_ENABLED(hash->items[pos]) && hash->items[pos].key == key)
            return pos;
        if (!HASH_ITEM_ENABLED(hash->items[pos]))
            break;
        pos = (pos + 1) % hash->size;
    } while (pos != start_pos);

    return -1;
}

int _create_hash_table(SHashTable *hash, int no, const char *file, int line) {
    if (hash->init == 1)
        return 0;
    hash->items = (SHashItem *)malloc(no * sizeof(SHashItem));
    if (!hash->items) {
        LOG_AND_RETURN(1, "Could not allocate Hash Items %d", no);
    }
    memset(hash->items, 0, no * sizeof(SHashItem));
    hash->size = no;
    hash->init = 1;
    hash->file = file;
    hash->line = line;

    return 0;
}

void *getItem(SHashTable *hash, uint64_t key) {
    int i;
    void *result = NULL;
    std::lock_guard<SMutex> lock(hash->mutex);

    i = get_index_hash(hash, key);
    result = i >= 0 ? hash->items[i].data : NULL;
    return result;
}

int getItemLen(SHashTable *hash, uint64_t key) {
    int i, result = 0;
    std::lock_guard<SMutex> lock(hash->mutex);

    i = get_index_hash(hash, key);
    result = i >= 0 ? hash->items[i].len : 0;
    return result;
}

int getItemSize(SHashTable *hash, uint64_t key) {
    int i;
    int result = 0;
    std::lock_guard<SMutex> lock(hash->mutex);
    i = get_index_hash(hash, key);
    result = i >= 0 ? hash->items[i].max_size : 0;
    return result;
}

// copy = 1 - do allocation and copy content
// is_alloc = 1 - memory allocated
int setItemSize(SHashTable *hash, SHashItem *s, uint32_t max_size, int copy) {
    if (s->max_size >= max_size && s->is_alloc == copy)
        return 0;
    if (s->is_alloc)
        free(s->data);
    s->is_alloc = 0;
    if (copy) {
        s->data = malloc(max_size + 10);
        if (!s->data)
            LOG_AND_RETURN(-1, "%s: Could not resize from %d to %d",
                           __FUNCTION__, s->max_size, max_size);
        s->is_alloc = 1;
    }
    s->max_size = max_size;

    return 0;
}

int _setItem(SHashTable *hash, uint64_t key, void *data, int len, int copy) {
    std::lock_guard<SMutex> lock(hash->mutex);
    SHashItem *s = NULL;
    int i = get_index_hash(hash, key);
    if (i >= 0)
        s = hash->items + i;
    if (!s) {
        // Add new element
        int start_pos = hash_func(key) % hash->size;
        int pos;
        pos = start_pos;
        do {
            if (!HASH_ITEM_ENABLED(hash->items[pos])) {
                s = hash->items + pos;
                break;
            }
            hash->conflicts++;
            pos = (pos + 1) % hash->size;
        } while (pos != start_pos);
    }

    if (!s) {
        LOG_AND_RETURN(-1, "%s failed for key %jx", __FUNCTION__, key);
    }

    if (setItemSize(hash, s, len, copy)) {
        return 1;
    }

    s->key = key;
    s->len = len;
    if (copy)
        memcpy(s->data, data, len);
    else
        s->data = data;

    if (hash->resize) {
        return 0;
    }
    if (++hash->len > hash->size / 2) {
        int new_size = hash->size * 2;
        SHashTable ht;
        ht.init = 0;

        // Do not fail, hash table full will fail before this code.
        if (_create_hash_table(&ht, new_size, hash->file, hash->line)) {
            LOG_AND_RETURN(0, "Resizing hash_table at %p from %d to %d", hash,
                           hash->size, new_size);
        }
        hash->resize = 1;
        ht.resize = 1;
        copy_hash_table(hash, &ht);
        free_hash(hash);
        memcpy(hash, &ht, sizeof(SHashTable));
        hash->resize = 0;
    }
    return 0;
}

void copy_hash_table(SHashTable *s, SHashTable *d) {
    int i;
    for (i = 0; i < s->size; i++)
        if (HASH_ITEM_ENABLED(s->items[i])) {
            _setItem(d, s->items[i].key, s->items[i].data, s->items[i].len, 0);
            int di = get_index_hash(d, s->items[i].key);
            if (di == -1)
                continue;
            memcpy(d->items + di, s->items + i, sizeof(SHashItem));
            memset(s->items + i, 0, sizeof(SHashItem));
        }
}

int delItem(SHashTable *hash, uint64_t key) {
    std::lock_guard<SMutex> lock(hash->mutex);

    int empty = get_index_hash(hash, key);
    if (empty == -1) {
        return 0;
    }
    int pos;
    for (pos = (empty + 1) % hash->size; HASH_ITEM_ENABLED(hash->items[pos]);
         pos = (pos + 1) % hash->size) {
        int k = hash_func(hash->items[pos].key) % hash->size;
        if ((pos > empty && (k <= empty || k > pos)) ||
            (pos < empty && (k <= empty && k > pos))) {
            SHashItem it;
            memcpy(&it, hash->items + empty, sizeof(hash->items[0]));
            memcpy(hash->items + empty, hash->items + pos,
                   sizeof(hash->items[0]));
            memcpy(hash->items + pos, &it, sizeof(hash->items[0]));
            empty = pos;
        }
    }
    SHashItem *s = hash->items + empty;
    hash->len--;
    s->len = 0;
    s->key = UNUSED_KEY;
    return 0;
}

int delItemP(SHashTable *hash, void *p) {
    int i;
    for (i = 0; i < hash->size; i++)
        if (HASH_ITEM_ENABLED(hash->items[i]) && hash->items[i].data == p)
            delItem(hash, hash->items[i].key);
    return 0;
}

void free_hash(SHashTable *hash) {
    int i;
    if (hash->init != 1)
        return;

    std::lock_guard<SMutex> lock(hash->mutex);
    for (i = 0; i < hash->size; i++)
        if (hash->items[i].is_alloc) {
            free(hash->items[i].data);
        }
    void *items = hash->items;
    free(items);
    hash->items = NULL;
    hash->size = 0;
    return;
}
