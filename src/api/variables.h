#ifndef API_VARIABLES_H
#define API_VARIABLES_H

#include "stdint.h"

#define VAR_UINT8 1
#define VAR_INT8 2
#define VAR_UINT16 3
#define VAR_INT16 4
#define VAR_INT 5
#define VAR_INT64 6
#define VAR_STRING 7
#define VAR_PSTRING 8
#define VAR_FLOAT 9
#define VAR_HEX 10
#define VAR_ARRAY 16
#define VAR_ARRAY_UINT8 (VAR_ARRAY + VAR_UINT8)
#define VAR_ARRAY_INT8 (VAR_ARRAY + VAR_INT8)
#define VAR_ARRAY_UINT16 (VAR_ARRAY + VAR_UINT16)
#define VAR_ARRAY_INT16 (VAR_ARRAY + VAR_INT16)
#define VAR_ARRAY_INT (VAR_ARRAY + VAR_INT)
#define VAR_ARRAY_INT64 (VAR_ARRAY + VAR_INT64)
#define VAR_ARRAY_FLOAT (VAR_ARRAY + VAR_FLOAT)
#define VAR_ARRAY_HEX (VAR_ARRAY + VAR_HEX)
#define VAR_ARRAY_STRING (VAR_ARRAY + VAR_STRING)
#define VAR_ARRAY_PSTRING (VAR_ARRAY + VAR_PSTRING)
#define VAR_AARRAY 32
#define VAR_AARRAY_UINT8 (VAR_AARRAY + VAR_UINT8)
#define VAR_AARRAY_INT8 (VAR_AARRAY + VAR_INT8)
#define VAR_AARRAY_UINT16 (VAR_AARRAY + VAR_UINT16)
#define VAR_AARRAY_INT16 (VAR_AARRAY + VAR_INT16)
#define VAR_AARRAY_INT (VAR_AARRAY + VAR_INT)
#define VAR_AARRAY_INT64 (VAR_AARRAY + VAR_INT64)
#define VAR_AARRAY_FLOAT (VAR_AARRAY + VAR_FLOAT)
#define VAR_AARRAY_HEX (VAR_AARRAY + VAR_HEX)
#define VAR_AARRAY_STRING (VAR_AARRAY + VAR_STRING)
#define VAR_AARRAY_PSTRING (VAR_AARRAY + VAR_PSTRING)
#define VAR_FUNCTION 48
#define VAR_FUNCTION_INT (VAR_FUNCTION + VAR_INT)
#define VAR_FUNCTION_INT64 (VAR_FUNCTION + VAR_INT64)
#define VAR_FUNCTION_STRING (VAR_FUNCTION + VAR_STRING)

#define VAR_LENGTH 20

typedef int (*get_data_int)(int p);
typedef int64_t (*get_data_int64)(int p);
typedef char *(*get_data_string)(int p, char *dest, int max_len);

int snprintf_pointer(char *dest, int max_len, int type, void *p,
                     float multiplier);
int var_eval(char *orig, int len, char *dest, int max_len);
int is_var(char *s);
void *get_var_address(char *var, float *multiplier, int *type, void *storage,
                      int ls);

int escape_json_string(char *dest, int dl, char *src, int sl);
int get_json_state(char *buf, int len);
int get_json_bandwidth(char *buf, int len);

#endif
