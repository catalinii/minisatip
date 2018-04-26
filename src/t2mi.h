#ifndef T2MI_H
#define T2MI_H
#include "adapter.h"

#define T2MI_PID 4096

typedef struct t2mi_device
{
	int id;
	unsigned char *buf;
} t2mi_device_t;

int t2mi_process_ts(adapter *ad);
void free_t2mi();
#endif
