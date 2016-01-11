#ifndef UTILS_H
#define UTILS_H
#define _GNU_SOURCE 
#include "pthread.h"

#include <sys/types.h>

#define VAR_UINT8 1
#define VAR_INT8 2
#define VAR_UINT16 3
#define VAR_INT16 4
#define VAR_INT 5
#define VAR_STRING 6
#define VAR_PSTRING 7
#define VAR_FLOAT 8
#define VAR_HEX 9
#define VAR_ARRAY_UINT8 10
#define VAR_ARRAY_INT8 11
#define VAR_ARRAY_UINT16 12
#define VAR_ARRAY_INT16 13
#define VAR_ARRAY_INT 14
#define VAR_ARRAY_FLOAT 15
#define VAR_ARRAY_HEX 16
#define VAR_ARRAY_STRING 17
#define VAR_ARRAY_PSTRING 18
#define VAR_AARRAY_UINT8 19
#define VAR_AARRAY_INT8 20
#define VAR_AARRAY_UINT16 21
#define VAR_AARRAY_INT16 22
#define VAR_AARRAY_INT 23
#define VAR_AARRAY_FLOAT 24
#define VAR_AARRAY_HEX 25
#define VAR_AARRAY_STRING 26
#define VAR_AARRAY_PSTRING 27
#define VAR_FUNCTION_INT 28
#define VAR_FUNCTION_STRING 29


typedef int (*get_data_int)(int p);
typedef char * (*get_data_string)(int p, char *dest, int max_len);

typedef struct struct_symbols
{
	char *name;
	int type;
	void *addr;
	float multiplier;       // multiply the value of the variable
	int len;
	int skip;
} _symbols;

typedef struct struct_mutex
{
	int enabled;
	pthread_mutex_t mtx;
	int state;
	int line;
	pthread_t tid;
	char *file;
} SMutex;


unsigned char *getItem(int64_t key);
int getItemLen(int64_t key);
int setItem(int64_t key, unsigned char *data, int len, int pos);
int delItem(int64_t key);
int delItemP(void *p);
int split(char **rv, char *s, int lrv, char sep);
int setItemSize(int64_t key, uint32_t max_size);
int setItemTimeout(int64_t key, int tmout);
int setItem(int64_t key, unsigned char *data, int len, int pos);
int getItemSize(int64_t key);
int map_int(char *s, char ** v);
int map_intd(char *s, char ** v, int dv);
int map_float(char *s, int mul);
void *mymalloc(int a, char *f, int l);
void myfree(void *x, char *f, int l);
char *header_parameter(char **arg, int i);
void _log(int level, char * file, int line, char *fmt, ...);
char *strip(char *s);
int split(char **rv, char *s, int lrv, char sep);
void set_signal_handler();
int becomeDaemon();
int end_of_header(char *buf);
char *readfile(char *fn, char *ctype, int *len);
void process_file(void *sock, char *s, int len, char *ctype);
int closefile(char *mem, int len);

int mutex_init(SMutex *mutex);
int mutex_lock1(char *FILE, int line, SMutex *mutex);
int mutex_unlock1(char *FILE, int line,  SMutex *mutex);
int mutex_destroy(SMutex *mutex);
void clean_mutexes();
pthread_t start_new_thread(char *name);
pthread_t get_tid();

int add_new_lock(void **arr, int count, int size, SMutex *mutex);


#define mutex_lock(m) mutex_lock1(__FILE__,__LINE__,m)
#define mutex_unlock(m) mutex_unlock1(__FILE__,__LINE__,m)
//#define proxy_log(level, fmt, ...) _proxy_log(level, fmt"\n", ##__VA_ARGS__)

//#define LOG(a,...) {opts.last_log=a;if(opts.log){int x=getTick();printf(CC([%d.%03d]: ,a,\n),x/1000,x%1000,##__VA_ARGS__);fflush(stdout);};}
//#define LOG(a,...) {opts.last_log=a;if(opts.log){printf(CC([%s]:\x20,a,\n),get_current_timestamp_log(),##__VA_ARGS__);fflush(stdout);};}
#define LOG(a,...) { _log(1,__FILE__,__LINE__,a, ##__VA_ARGS__);}
#define LOGL(level,a,...) { if(level<=opts.log)_log(level,__FILE__,__LINE__,a, ##__VA_ARGS__);}
#define LOGLM(level,a,...) { if((!opts.log_mute&&level<=opts.log))_log(level,__FILE__,__LINE__,a, ##__VA_ARGS__);}

#define FAIL(a,...) {LOGL(0,a,##__VA_ARGS__);unlink(pid_file);exit(1);}
#define LOG_AND_RETURN(rc,a,...) {LOG(a,##__VA_ARGS__);return rc;}
#define malloc1(a) mymalloc(a,__FILE__,__LINE__)
#define free1(a) myfree(a,__FILE__,__LINE__)

#endif
