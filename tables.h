#ifndef TABLES_H
#define TABLES_H

#define TABLES_ITEM 0x1000000000000;

#if defined(DISABLE_DVBCSA) && defined(DISABLE_DVBCA)

#define DISABLE_TABLES

#endif

#ifdef DISABLE_TABLES
#define process_pat(b, a) 0
#else

#include "adapter.h"

unsigned char *getItem(int64_t key);
int getItemLen(int64_t key);
int setItem(int64_t key, unsigned char *data, int len, int pos);
int delItem(int64_t key);
int process_pat(unsigned char *b, adapter *ad);
int process_pmt(unsigned char *b, adapter *ad);
#endif
#endif