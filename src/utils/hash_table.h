#ifndef HASH_TABLE_H
#define HASH_TABLE_H
#define _GNU_SOURCE

#include "utils/mutex.h"
#include <stdint.h>

// Hash Table implementation:
// it holds the key (int32) and the value (void *) of specified size
// The data is allocated inside of the hash_item and resized if needed
// put will actually copy the data from the argument to the hash_item.data
// the value 0 means unused (do not set a key with value 0)

typedef struct hash_item {
    uint16_t len;
    uint16_t max_size;
    uint8_t is_alloc;
    int64_t key;
    void *data;
} SHashItem;

typedef struct hash_table {
    char init, resize;
    int len;
    int size;
    SHashItem *items;
    int64_t conflicts;
    SMutex mutex;
} SHashTable;

#define HASH_ITEM_ENABLED(h) (h.len)
#define FOREACH_ITEM(h, a)                                                     \
    for (i = 0; i < (h)->size; i++)                                            \
        if (HASH_ITEM_ENABLED((h)->items[i]) && (a = (h)->items[i].data))

int create_hash_table(SHashTable *hash, int no);
void copy_hash_table(SHashTable *s, SHashTable *d);
void free_hash(SHashTable *hash);
void *getItem(SHashTable *hash, uint32_t key);
#define setItem(a, b, c, d) _setItem(a, b, c, d, 1)
int _setItem(SHashTable *hash, uint32_t key, void *data, int len, int copy);
int delItem(SHashTable *hash, uint32_t key);

#endif
