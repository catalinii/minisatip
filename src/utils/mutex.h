#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>
#include <stdint.h>

typedef struct struct_mutex {
    int enabled;
    pthread_mutex_t mtx;
    int state;
    int line;
    pthread_t tid;
    int64_t lock_time, create_time;
    const char *file;
} SMutex;

int mutex_init(SMutex *mutex);
int mutex_lock1(const char *FILE, int line, SMutex *mutex);
int mutex_unlock1(const char *FILE, int line, SMutex *mutex);
int mutex_destroy(SMutex *mutex);
void clean_mutexes();

#define mutex_lock(m) mutex_lock1(__FILE__, __LINE__, m)
#define mutex_unlock(m) mutex_unlock1(__FILE__, __LINE__, m)

#endif
