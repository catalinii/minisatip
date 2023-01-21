/*
 * Copyright (C) 2014-2022 Catalin Toda <catalinii@yahoo.com>,
                           Sam Stenvall <neggelandia@gmail.com>,
                           et al.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 *
 */
#include "api/variables.h"
#include "api/symbols.h"
#include "utils.h"
#include "utils/logging/logging.h"
#include "utils/mutex.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int snprintf_pointer(char *dest, int max_len, int type, void *p,
                     float multiplier) {
    int nb;
    switch (type & 0xF) {
    case VAR_UINT8:
        nb = snprintf(dest, max_len, "%d",
                      (int)((*(unsigned char *)p) * multiplier));
        break;
    case VAR_INT8:
        nb = snprintf(dest, max_len, "%d", (int)((*(char *)p) * multiplier));
        break;
    case VAR_UINT16:
        nb =
            snprintf(dest, max_len, "%d", (int)((*(uint16_t *)p) * multiplier));
        break;
    case VAR_INT16:
        nb = snprintf(dest, max_len, "%d", (int)((*(int16_t *)p) * multiplier));
        break;
    case VAR_INT:
        nb = snprintf(dest, max_len, "%d", (int)((*(int *)p) * multiplier));
        break;

    case VAR_INT64:
        nb = snprintf(dest, max_len, "%jd",
                      (int64_t)((*(int64_t *)p) * multiplier));
        break;

    case VAR_STRING:
        nb = snprintf(dest, max_len, "%s", (char *)p);
        break;

    case VAR_PSTRING:
        nb = snprintf(dest, max_len, "%s", (*(char **)p) ? (*(char **)p) : "");
        break;

    case VAR_FLOAT:
        nb =
            snprintf(dest, max_len, "%f", (double)((*(float *)p) * multiplier));
        break;

    case VAR_HEX:
        nb = snprintf(dest, max_len, "0x%x", (int)((*(int *)p) * multiplier));
        break;

    default:
        nb = 0;
        break;
    }
    if (nb > max_len) /* see man 'snprintf' */
        nb = max_len;
    return nb;
}

int var_eval(char *orig, int len, char *dest, int max_len) {
    char var[VAR_LENGTH + 1];
    char storage[64 * 5]; // variable max len
    float multiplier;
    int type = 0;
    void *p;
    int nb = 0;
    memset(var, 0, sizeof(var));
    strncpy(var, orig + 1, len - 1);
    p = get_var_address(var, &multiplier, &type, storage, sizeof(storage));
    if (p)
        nb = snprintf_pointer(dest, max_len, type, p, multiplier);
    return nb;
}

int is_var(char *s) {
    int i = 0;
    if (*s != '$')
        return 0;
    while (s[++i] != 0) {
        if (s[i] == '$')
            return i;
        if (i >= VAR_LENGTH)
            return 0;
    }
    return 0;
}

char zero[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void *get_var_address(char *var, float *multiplier, int *type, void *storage,
                      int ls) {
    int i, j, off;
    *multiplier = 0;

    for (i = 0; sym[i] != NULL; i++)
        for (j = 0; sym[i][j].name; j++)
            if (!strncmp(sym[i][j].name, var, strlen(sym[i][j].name))) {
                *type = sym[i][j].type;
                if (sym[i][j].type < VAR_ARRAY) {
                    *multiplier = sym[i][j].multiplier;
                    return sym[i][j].addr;
                } else if ((sym[i][j].type & 0xF0) == VAR_ARRAY) {
                    off = map_intd(var + strlen(sym[i][j].name), NULL, 0);
                    if (off >= 0 && off < sym[i][j].len) {
                        *multiplier = sym[i][j].multiplier;
                        return (((char *)sym[i][j].addr) +
                                off * sym[i][j].skip);
                    }
                } else if ((sym[i][j].type & 0xF0) == VAR_AARRAY) {
                    off = map_intd(var + strlen(sym[i][j].name), NULL, 0);
                    if (off >= 0 && off < sym[i][j].len) {
                        char **p1 = (char **)sym[i][j].addr;
                        char *p = p1[off];

                        if (!p) {
                            p = zero;
                        } else
                            p += sym[i][j].skip;
                        *multiplier = sym[i][j].multiplier;
                        return p;
                    }
                } else if (sym[i][j].type == VAR_FUNCTION_INT) {
                    off = map_intd(var + strlen(sym[i][j].name), NULL, 0);
                    get_data_int funi = (get_data_int)sym[i][j].addr;
                    *(int *)storage = funi(off);
                    *multiplier = 1;
                    return storage;
                } else if (sym[i][j].type == VAR_FUNCTION_INT64) {
                    off = map_intd(var + strlen(sym[i][j].name), NULL, 0);
                    get_data_int64 fun64 = (get_data_int64)sym[i][j].addr;
                    *(int64_t *)storage = fun64(off);
                    *multiplier = 1;
                    return storage;
                } else if (sym[i][j].type == VAR_FUNCTION_STRING) {
                    off = map_intd(var + strlen(sym[i][j].name), NULL, 0);
                    get_data_string funs = (get_data_string)sym[i][j].addr;
                    funs(off, storage, ls);
                    return storage;
                }
            }
    return NULL;
}

int escape_json_string(char *dest, int dl, char *src, int sl) {
    int i, j = 1;
    if (dl < 2)
        LOG_AND_RETURN(0, "%s: dl %d < 2 for %s", __FUNCTION__, dl, src);

    dest[0] = '"';
    for (i = 0; (i < sl) && (j < dl - 1); i++) {
        unsigned char c = (unsigned char)src[i];
        if (c >= 32)
            dest[j++] = src[i];
        else
            strlcatf(dest, sl, j, "%%x%02X", c);
    }
    dest[j++] = '"';
    dest[j] = 0;
    return j;
}

int get_json_state(char *buf, int len) {
    int ptr = 0, first = 1, i, j, off, string;
    _symbols *p;
    char escape[200]; // string variable max len

    memset(escape, 0, sizeof(escape));
    strlcatf(buf, len, ptr, "{\n");
    for (i = 0; sym[i] != NULL; i++) {
        for (j = 0; sym[i][j].name; j++) {
            p = sym[i] + j;
            strlcatf(buf, len, ptr, first ? "\"%s\":" : ",\n\"%s\":", p->name);
            string = 0;
            switch (p->type) {
            case VAR_STRING:
            case VAR_PSTRING:
            case VAR_HEX:
            case VAR_AARRAY_STRING:
            case VAR_AARRAY_PSTRING:
                string = 1;
                break;
            }
            if (p->type < VAR_ARRAY) {
                if (string) {
                    int len2 =
                        snprintf_pointer(escape, sizeof(escape) - 1, p->type,
                                         p->addr, p->multiplier);
                    ptr +=
                        escape_json_string(buf + ptr, len - ptr, escape, len2);
                } else
                    ptr += snprintf_pointer(buf + ptr, len - ptr, p->type,
                                            p->addr, p->multiplier);
            } else if ((p->type & 0xF0) == VAR_ARRAY) {
                strlcatf(buf, len, ptr, "[");
                for (off = 0; off < p->len; off++) {
                    if (off > 0)
                        strlcatf(buf, len, ptr, ",");
                    if (string) {
                        int len2 = snprintf_pointer(
                            escape, sizeof(escape) - 1, p->type,
                            ((char *)p->addr) + off + p->skip, p->multiplier);
                        ptr += escape_json_string(buf + ptr, len - ptr, escape,
                                                  len2);
                    } else
                        ptr += snprintf_pointer(
                            buf + ptr, len - ptr, p->type,
                            ((char *)p->addr) + off + p->skip, p->multiplier);
                }
                strlcatf(buf, len, ptr, "]");
            } else if ((sym[i][j].type & 0xF0) == VAR_AARRAY) {
                strlcatf(buf, len, ptr, "[");
                for (off = 0; off < p->len; off++) {
                    char **p1 = (char **)p->addr;
                    if (off > 0)
                        strlcatf(buf, len, ptr, ",");
                    if (string) {
                        int len2 = snprintf_pointer(
                            escape, sizeof(escape) - 1, p->type,
                            p1[off] ? p1[off] + p->skip : zero, p->multiplier);
                        ptr += escape_json_string(buf + ptr, len - ptr, escape,
                                                  len2);
                    } else
                        ptr += snprintf_pointer(
                            buf + ptr, len - ptr, p->type,
                            p1[off] ? p1[off] + p->skip : zero, p->multiplier);
                }
                strlcatf(buf, len, ptr, "]");
            } else if (sym[i][j].type == VAR_FUNCTION_INT) {
                get_data_int funi = (get_data_int)p->addr;
                strlcatf(buf, len, ptr, "[");
                for (off = 0; off < p->len; off++) {
                    int storage = funi(off);
                    if (off > 0)
                        strlcatf(buf, len, ptr, ",");
                    ptr += snprintf_pointer(buf + ptr, len - ptr, p->type,
                                            &storage, 1);
                }
                strlcatf(buf, len, ptr, "]");
            } else if (sym[i][j].type == VAR_FUNCTION_INT64) {
                get_data_int64 fun64 = (get_data_int64)p->addr;
                strlcatf(buf, len, ptr, "[");
                for (off = 0; off < p->len; off++) {
                    int64_t storage = fun64(off);
                    if (off > 0)
                        strlcatf(buf, len, ptr, ",");
                    ptr += snprintf_pointer(buf + ptr, len - ptr, p->type,
                                            &storage, 1);
                }
                strlcatf(buf, len, ptr, "]");
            } else if (sym[i][j].type == VAR_FUNCTION_STRING) {
                get_data_string funs = (get_data_string)p->addr;
                strlcatf(buf, len, ptr, "[");
                for (off = 0; off < p->len; off++) {
                    memset(escape, 0, sizeof(escape));
                    funs(off, escape, sizeof(escape) - 1);
                    if (off > 0)
                        strlcatf(buf, len, ptr, ",");
                    ptr += escape_json_string(buf + ptr, len - ptr, escape,
                                              strlen(escape));
                }
                strlcatf(buf, len, ptr, "]");
                //				LOG("func_str -> %s", buf);
            } else {
                strlcatf(buf, len, ptr, "\"\"");
            }
            first = 0;
        }
    }
    strlcatf(buf, len, ptr, "\n}\n");
    return ptr;
}

extern SMutex bw_mutex;

// TODO: Refactor this so these re-declarations aren't needed
extern int64_t c_tbw, c_bw, c_bw_dmx, c_buffered, c_dropped;
extern uint32_t c_reads, c_writes, c_failed_writes;
extern int64_t c_ns_read, c_tt;

int get_json_bandwidth(char *buf, int len) {
    int ptr = 0;
    mutex_init(&bw_mutex);
    mutex_lock(&bw_mutex);
    strlcatf(buf, len, ptr, "\
{\n\
\"bw\":%jd,\n\
\"dmx\":%jd,\n\
\"tbw\":%jd,\n\
\"reads\":%u,\n\
\"writes\":%u,\n\
\"fwrites\":%u,\n\
\"ns_read\":%jd,\n\
\"tt\":%jd,\n\
\"buffered\":%jd,\n\
\"dropped\":%jd\n\
}",
             c_bw, c_bw_dmx, c_tbw, c_reads, c_writes, c_failed_writes,
             c_ns_read, c_tt, c_buffered, c_dropped);
    mutex_unlock(&bw_mutex);
    return ptr;
}
