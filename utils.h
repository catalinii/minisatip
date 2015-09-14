#ifndef UTILS_H
#define UTILS_H
#define _GNU_SOURCE 
#include "socketworks.h"

unsigned char *getItem(int64_t key);
int getItemLen(int64_t key);
int setItem(int64_t key, unsigned char *data, int len, int pos);
int delItem(int64_t key);
int split (char **rv, char *s, int lrv, char sep);
int map_int (char *s, char ** v);
int map_intd (char *s, char ** v, int dv);
int map_float (char *s, int mul);
int getItemChange(int64_t key, int *prev);
void *mymalloc (int a, char *f, int l);
void myfree (void *x, char *f, int l);
char *header_parameter(char **arg, int i);
void _log(int level, char * file, int line, char *fmt, ...);
char *strip(char *s);
int split (char **rv, char *s, int lrv, char sep);
void set_signal_handler ();
int becomeDaemon ();
char *readfile(char *fn, char *ctype, int *len);
void process_file(sockets *so, char *s, int len, char *ctype);

//#define proxy_log(level, fmt, ...) _proxy_log(level, fmt"\n", ##__VA_ARGS__)

//#define LOG(a,...) {opts.last_log=a;if(opts.log){int x=getTick();printf(CC([%d.%03d]: ,a,\n),x/1000,x%1000,##__VA_ARGS__);fflush(stdout);};}
//#define LOG(a,...) {opts.last_log=a;if(opts.log){printf(CC([%s]:\x20,a,\n),get_current_timestamp_log(),##__VA_ARGS__);fflush(stdout);};}
#define LOG(a,...) { _log(1,__FILE__,__LINE__,a, ##__VA_ARGS__);}
#define LOGL(level,a,...) { if(level<=opts.log)_log(level,__FILE__,__LINE__,a, ##__VA_ARGS__);}


#define FAIL(a,...) {LOGL(0,a,##__VA_ARGS__);unlink(PID_FILE);exit(1);}
#define LOG_AND_RETURN(rc,a,...) {LOG(a,##__VA_ARGS__);return rc;}
#define malloc1(a) mymalloc(a,__FILE__,__LINE__)
#define free1(a) myfree(a,__FILE__,__LINE__)

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
#define VAR_FUNCTION_INT 19
#define VAR_FUNCTION_STRING 20

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

#endif
