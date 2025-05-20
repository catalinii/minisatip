#ifndef ALLOC_H
#define ALLOC_H

void *malloc1(int a, const char *f, int l, int r);
void *realloc1(void *p, int a, const char *f, int l, int r);
void free1(void *x, const char *f, int l);
int _ensure_allocated(void **x, int struct_size, int pointer_size,
                      int ensure_length, int min_elements, const char *file,
                      int line);

void init_alloc();
void free_alloc();

#define _malloc(a) malloc1(a, __FILE__, __LINE__, 1)
#define _free(a) free1(a, __FILE__, __LINE__)
#define _realloc(a, b) realloc1(a, b, __FILE__, __LINE__, 1)
#define ensure_allocated(p, struct_size, pointer_size, len, min_elements)      \
    _ensure_allocated(p, struct_size, pointer_size, len, min_elements,         \
                      __FILE__, __LINE__)

#endif
