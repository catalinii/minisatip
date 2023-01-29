#ifndef T2MI_H
#define T2MI_H
#include "adapter.h"

#define T2MI_PID 4096

int t2mi_process_ts(adapter *ad);
void free_t2mi();
#endif
