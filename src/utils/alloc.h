#ifndef ALLOC_H
#define ALLOC_H
#define _GNU_SOURCE

void *malloc1(int a, char *f, int l, int r);
void *realloc1(void *p, int a, char *f, int l, int r);
void free1(void *x, char *f, int l, int r);
int _ensure_allocated(void **x, int struct_size, int pointer_size,
                      int ensure_length, int min_elements, char *file,
                      int line);

void init_alloc();
void free_alloc();

#define _malloc(a) malloc1(a, __FILE__, __LINE__, 1)
#define _free(a) free1(a, __FILE__, __LINE__, 1)
#define _realloc(a, b) realloc1(a, b, __FILE__, __LINE__, 1)
#define ensure_allocated(p, struct_size, pointer_size, len, min_elements)      \
    _ensure_allocated(p, struct_size, pointer_size, len, min_elements,         \
                      __FILE__, __LINE__)

#endif
