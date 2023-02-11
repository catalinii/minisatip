/*
 * Copyright (C) 2014-2020 Catalin Toda <catalinii@yahoo.com>
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
#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#define UTILS_C

#include "utils.h"
#include "api/variables.h"
#include "dvb.h"
#include "minisatip.h"
#include "pmt.h"
#include "socketworks.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#if !defined(__mips__) && !defined(NO_BACKTRACE)
#include <execinfo.h>
#endif

#define DEFAULT_LOG LOG_UTILS

char pn[256];

SMutex utils_mutex;

int split(char **rv, char *s, int lrv, char sep) {
    int i = 0, j = 0;

    if (!s)
        return 0;
    for (i = 0; s[i] && s[i] == sep && s[i] < 32; i++)
        ;

    rv[j++] = &s[i];
    //      LOG("start %d %d\n",i,j);
    while (j < lrv - 1) {
        if (s[i] == 0 || s[i + 1] == 0)
            break;
        if (s[i] == sep || s[i] < 33) {
            s[i] = 0;
            if (s[i + 1] != sep && s[i + 1] > 32)
                rv[j++] = &s[i + 1];
        } else if (s[i] < 14)
            s[i] = 0;
        //              LOG("i=%d j=%d %d %c \n",i,j,s[i],s[i]);
        i++;
    }
    if (s[i] == sep)
        s[i] = 0;
    rv[j] = NULL;
    return j;
}

char *strip(char *s) // strip spaces from the front of a string
{
    if (s < (char *)1000)
        return NULL;

    while (*s && *s == ' ')
        s++;
    return s;
}

int map_intd(char *s, char **v, int dv) {
    int i, n = dv;

    if (s == NULL) {
        LOG_AND_RETURN(dv, "map_intd: s=>NULL, v=%p, %s %s", v,
                       v ? v[0] : "NULL", v ? v[1] : "NULL");
    }

    s = strip(s);

    if (!*s)
        LOG_AND_RETURN(dv, "map_intd: s is empty");

    if (v == NULL) {
        if (s[0] != '+' && s[0] != '-' && (s[0] < '0' || s[0] > '9'))
            LOG_AND_RETURN(dv, "map_intd: s not a number: %s, v=%p, %s %s", s,
                           v, v ? v[0] : "NULL", v ? v[1] : "NULL");
        return atoi(s);
    }
    for (i = 0; v[i]; i++)
        if (!strncasecmp(s, v[i], strlen(v[i])))
            n = i;
    return n;
}

int check_strs(char *s, char **v, int dv) {
    int i, n = dv;

    if (s == NULL) {
        LOG_AND_RETURN(dv, "check_strs: s=>NULL, v=%p, %s %s", v,
                       v ? v[0] : "NULL", v ? v[1] : "NULL");
    }
    if (v == NULL) {
        LOG_AND_RETURN(dv, "check_strs: v is empty");
    }

    s = strip(s);

    if (!*s)
        LOG_AND_RETURN(dv, "check_strs: s is empty");

    for (i = 0; v[i]; i++)
        if (strncasecmp(s, v[i], strlen(s)) == 0)
            n = i;
    return n;
}

char *header_parameter(char **arg,
                       int i) // get the value of a header parameter
{
    int len = strlen(arg[i]);
    char *result;

    if (arg[i][len - 1] == ':')
        return arg[i + 1];

    result = strchr(arg[i], ':');
    if (result)
        return result + 1;

    if (strcmp(arg[i + 1], ":") == 0)
        return arg[i + 2];
    return NULL;
}

int map_float(char *s, int mul) {
    float f;
    int r;

    if (s == NULL)
        LOG_AND_RETURN(0, "map_float: s=>NULL, mul=%d", mul);
    if (s[0] != '+' && s[0] != '-' && (s[0] < '0' || s[0] > '9'))
        LOG_AND_RETURN(0, "map_float: s not a number: %s, mul=%d", s, mul);

    f = atof(s);
    r = (int)(f * mul);
    //      LOG("atof returned %.1f, mul = %d, result=%d",f,mul,r);
    return r;
}

int map_int(char *s, char **v) { return map_intd(s, v, 0); }

int end_of_header(char *buf) {
    return buf[0] == 0x0d && buf[1] == 0x0a && buf[2] == 0x0d && buf[3] == 0x0a;
}

void posix_signal_handler(int sig, siginfo_t *siginfo, ucontext_t *ctx);
void set_signal_handler(char *argv0) {
    struct sigaction sig_action = {};
    sig_action.sa_sigaction =
        (void (*)(int, siginfo_t *, void *))posix_signal_handler;
    sigemptyset(&sig_action.sa_mask);

    memset(pn, 0, sizeof(pn));
    safe_strncpy(pn, argv0);

    sig_action.sa_flags = SA_SIGINFO | SA_ONSTACK;

#ifndef __mips__
    if (sigaction(SIGBUS, &sig_action, NULL) != 0) {
        LOG("Could not set signal SIGBUS");
    }
    if (sigaction(SIGSEGV, &sig_action, NULL) != 0) {
        LOG("Could not set signal SIGSEGV");
    }
    if (sigaction(SIGABRT, &sig_action, NULL) != 0) {
        LOG("Could not set signal SIGABRT");
    }
    if (sigaction(SIGFPE, &sig_action, NULL) != 0) {
        LOG("Could not set signal SIGFPE");
    }
    if (sigaction(SIGILL, &sig_action, NULL) != 0) {
        LOG("Could not set signal SIGILL");
    }
#endif
    if (sigaction(SIGINT, &sig_action, NULL) != 0) {
        LOG("Could not set signal SIGINT");
    }

    if (sigaction(SIGTERM, &sig_action, NULL) != 0) {
        LOG("Could not set signal SIGTERM");
    }
    if (signal(SIGHUP, SIG_IGN) != 0) {
        LOG("Could not ignore signal SIGHUP");
    }
    if (signal(SIGPIPE, SIG_IGN) != 0) {
        LOG("Could not ignore signal SIGPIPE");
    }
}

int addr2line(char const *const program_name, void const *const addr) {
    char addr2line_cmd[512] = {0};

    sprintf(addr2line_cmd, "addr2line -f -p -e %.256s %p", program_name, addr);
    return system(addr2line_cmd);
}

void print_trace(void) {
    void *array[10];
    size_t size;
    size_t i;
#if !defined(NO_BACKTRACE)

    size = backtrace(array, 10);

    printf("Obtained %zu stack frames.\n", size);

    for (i = 0; i < size; i++) {
        printf("%p : ", array[i]);
        if (addr2line(pn, array[i]))
            printf("\n");
    }
#else
    printf(" No backtrace defined\n");
#endif
}

extern int run_loop;

void posix_signal_handler(int sig, siginfo_t *siginfo, ucontext_t *ctx) {
    uint64_t sp = 0, ip = 0;

    if (sig == SIGINT || sig == SIGTERM) {
        run_loop = 0;
        return;
    }
#ifdef __mips__
    sp = ctx->uc_mcontext.gregs[29];
    ip = ctx->uc_mcontext.pc;
#endif
#ifdef __sh__
    sp = ctx->uc_mcontext.pr;
    ip = ctx->uc_mcontext.pc;
#endif
    printf("RECEIVED SIGNAL %d - SP=%lX IP=%lX\n", sig, (long unsigned int)sp,
           (long unsigned int)ip);

    print_trace();
    exit(1);
}

int /* Returns 0 on success, -1 on error */
becomeDaemon() {
    int maxfd, fd, fdi, fdo, pid = -1;
    __attribute__((unused)) int rv;
    struct stat sb;
    FILE *f;
    char path[255];
    char buf[255];
    char log_file[sizeof(buf)];

    memset(path, 0, sizeof(path));
    if ((f = fopen(pid_file, "rt"))) {
        char tmp_buf[10];
        memset(tmp_buf, 0, sizeof(tmp_buf));
        if (fgets(tmp_buf, sizeof(tmp_buf), f))
            pid = atoi(tmp_buf);
        fclose(f);
        snprintf(buf, sizeof(buf) - 1, "/proc/%d/exe", pid);

        if (0 < readlink(buf, path, sizeof(path) - 1) &&
            0 == strcmp(pn, path)) {
            LOG("Found %s running with pid %d, killing....", app_name, pid);
            kill(pid, SIGINT);
            sleep_msec(1);
        }
    }

    LOG("running %s in background and logging to %s", app_name, opts.log_file);

    switch (fork()) { /* Become background process */
    case -1:
        return -1;
    case 0:
        break; /* Child falls through... */
    default:
        _exit(0); /* while parent terminates */
    }

    if (setsid() == -1) /* Become leader of new session */
        return -1;

    switch ((pid = fork())) { /* Ensure we are not session leader */
    case -1:
        return -1;
    case 0:
        break;
    default:
        _exit(0);
    }

    umask(0); /* Clear file mode creation mask */

    maxfd = sysconf(_SC_OPEN_MAX);
    if (maxfd == -1)  /* Limit is indeterminate... */
        maxfd = 1024; /* so take a guess */

    for (fd = 0; fd < maxfd; fd++)
        close(fd);

    close(STDIN_FILENO); /* Reopen standard fd's to /dev/null */
    //	chdir ("/tmp");				 /* Change to root
    // directory */

    fdi = open("/dev/null", O_RDWR);
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf) - 1, "%s", opts.log_file);
    safe_strncpy(log_file, buf);
    if (fdi != STDIN_FILENO) /* 'fdi' should be 0 */
    {
        if (fdi >= 0)
            close(fdi);
        return -1;
    }
    if (stat(buf, &sb) == -1)
        fdo = open(log_file, O_RDWR | O_CREAT, 0666);
    else
        fdo = open(log_file, O_RDWR | O_APPEND);
    if (fdo != STDOUT_FILENO) /* 'fd' should be 1 */
    {
        if (fdo >= 0)
            close(fdo);
        return -1;
    }
    if (dup2(STDOUT_FILENO, STDERR_FILENO) != STDERR_FILENO)
        return -1;

    return 0;
}

void *mymalloc(int a, char *f, int l) {
    void *x = malloc(a);
    if (x)
        memset(x, 0, a);
    LOGM("%s:%d allocation_wrapper malloc allocated %d bytes at %p", f, l, a,
         x);
    if (!x)
        LOG0("Failed allocating %d bytes of memory", a)
    return x;
}

void *myrealloc(void *p, int a, char *f, int l) {
    void *x = realloc(p, a);
    if (x)
        memset(x, 0, a);
    LOGM("%s:%d allocation_wrapper realloc allocated %d bytes from %p -> %p", f,
         l, a, p, x);
    if (!x) {
        LOG0("Failed allocating %d bytes of memory", a)
        if (!strcmp(f, "socketworks.c"))
            LOG0("Try to decrease the parameters -b and/or -B")
    }
    return x;
}

void myfree(void *x, char *f, int l) {
    LOGM("%s:%d allocation_wrapper free called with argument %p", f, l, x);
    free(x);
}

char *get_current_timestamp(void) {
    static char date_str[200];
    time_t date;
    struct tm *t;
    char *day[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    char *month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    time(&date);
    t = gmtime(&date);
    if (!t)
        return "Fri, Sat Jan 1 00:00:20 2000 GMT";
    snprintf(date_str, sizeof(date_str), "%s, %s %d %02d:%02d:%02d %d GMT",
             day[t->tm_wday], month[t->tm_mon], t->tm_mday, t->tm_hour,
             t->tm_min, t->tm_sec, t->tm_year + 1900);
    return date_str;
}

int endswith(char *src, char *with) {
    int lw = strlen(with);
    if (strlen(src) > lw && !strcmp(src + strlen(src) - lw, with))
        return 1;
    return 0;
}

// replace $VAR$ with it's value and write the output to the socket
void process_file(void *sock, char *s, int len, char *ctype) {
    char outp[8300];
    int i, io = 0, lv, le, respond = 1;
    sockets *so = (sockets *)sock;
    __attribute__((unused)) int rv;
    LOG("processing_file %p len %d:", s, len);
    for (i = 0; i < len; i++) {
        lv = 0;
        if (s[i] == '$')
            lv = is_var(s + i);
        if (lv == 0) {
            outp[io++] = s[i];
        } else {
            le = var_eval(s + i, lv, outp + io, sizeof(outp) - io - 10);
            io += le;
            i += lv;
        }
        if (io > sizeof(outp) - 100) {
            if (respond) {
                http_response(
                    so, 200, ctype, "", 0,
                    0); // sending back the response without Content-Length
                respond = 0;
            }
            rv = sockets_write(so->id, outp, io);
            outp[io] = 0;
            //			LOG("%s", outp);
            io = 0;
        }
    }
    outp[io] = 0;
    if (respond)
        http_response(so, 200, ctype, outp, 0,
                      0); // sending back the response with Content-Length
                          // if output < 8192
    else {
        strcpy(outp + io, "\r\n\r\n");
        rv = sockets_write(so->id, outp, io + 4);
        outp[io] = 0;
        DEBUGM("%s", outp);
    }
}

char *readfile(char *fn, char *ctype, int *len) {
    char ffn[256];
    char *mem;
    struct stat sb;
    int fd, nl = 0;
    *len = 0;

    if (strstr(fn, ".."))
        return 0;
    snprintf(ffn, sizeof(ffn), "%s/%s", opts.document_root, fn);
    ffn[sizeof(ffn) - 1] = 0;
#ifdef O_LARGEFILE
    if ((fd = open(ffn, O_RDONLY | O_LARGEFILE)) < 0)
#else
    if ((fd = open(ffn, O_RDONLY)) < 0)
#endif
        LOG_AND_RETURN(NULL, "Could not open file %s", ffn);

    if ((fstat(fd, &sb) == -1) || !S_ISREG(sb.st_mode)) {
        LOG("readfile: %s is not a file", ffn);
        close(fd);
        return NULL;
    }
    nl = sb.st_size;
    mem = mmap(0, nl, PROT_READ, MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED) {
        close(fd);
        LOG_AND_RETURN(NULL, "mmap failed for file %s", ffn);
    }
    close(fd);
    LOG("opened %s fd %d at %p - %d bytes", ffn, fd, mem, nl);

    *len = nl;
    if (ctype) {
        ctype[0] = 0;
        if (endswith(fn, "png"))
            strcpy(ctype, "Cache-Control: max-age=3600\r\nContent-type: "
                          "image/png\r\nConnection: close");
        else if (endswith(fn, "jpg") || endswith(fn, "jpeg"))
            strcpy(ctype, "Cache-Control: max-age=3600\r\nContent-type: "
                          "image/jpeg\r\nConnection: close");
        else if (endswith(fn, "css"))
            strcpy(ctype, "Cache-Control: max-age=3600\r\nContent-type: "
                          "text/css\r\nConnection: close");
        else if (endswith(fn, "js"))
            strcpy(ctype, "Cache-Control: max-age=3600\r\nContent-type: "
                          "text/javascript\r\nConnection: close");
        else if (endswith(fn, "htm") || endswith(fn, "html"))
            strcpy(ctype, "Cache-Control: max-age=3600\r\nContent-type: "
                          "text/html\r\nConnection: close");
        else if (endswith(fn, "xml"))
            strcpy(ctype, "Cache-Control: no-cache\r\nContent-type: text/xml");
        else if (endswith(fn, "2.json")) // debug
            strcpy(ctype, "Content-type: application/json");
        else if (endswith(fn, "json"))
            strcpy(ctype, "Cache-Control: no-cache\r\nContent-type: "
                          "application/json");
        else if (endswith(fn, "m3u"))
            strcpy(ctype,
                   "Cache-Control: no-cache\r\nContent-type: video/x-mpegurl");
        else
            strcpy(ctype, "Cache-Control: no-cache\r\nContent-type: "
                          "application/octet-stream");
    }
    return mem;
}

int closefile(char *mem, int len) { return munmap((void *)mem, len); }

pthread_t get_tid() { return pthread_self(); }

pthread_t start_new_thread(char *name) {
    pthread_t tid;
    int rv;
    if (opts.no_threads)
        return get_tid();

    if ((rv = pthread_create(&tid, NULL, &select_and_execute, name))) {
        LOG("Failed to create thread: %s, error %d %s", name, rv, strerror(rv));
        return get_tid();
    }
    return tid;
}

void set_thread_prio(pthread_t tid, int prio) {
    int rv;
    struct sched_param param;
    memset(&param, 0, sizeof(struct sched_param));
    param.sched_priority = prio;
    if ((rv = pthread_setschedparam(pthread_self(), SCHED_RR, &param)))
        LOG("pthread_setschedparam failed with error %d", rv);
    return;
}

struct struct_array {
    char enabled;
    SMutex mutex;
};

// leaves sa[i]->mutex locked
int add_new_lock(void **arr, int count, int size, SMutex *mutex) {
    int i;
    struct struct_array **sa = (struct struct_array **)arr;
    mutex_init(mutex);
    mutex_lock(mutex);
    for (i = 0; i < count; i++)
        if (!sa[i] || !sa[i]->enabled) {
            if (!sa[i]) {
                sa[i] = malloc1(size);
                if (!sa[i]) {
                    mutex_unlock(mutex);
                    LOG("Could not allocate memory for %p index %d", arr, i);
                    return -1;
                }
                memset(sa[i], 0, size);
            }
            mutex_init(&sa[i]->mutex);
            // coverity[use : FALSE]
            mutex_lock(&sa[i]->mutex);
            sa[i]->enabled = 1;
            mutex_unlock(mutex);
            return i;
        }
    mutex_unlock(mutex);
    return -1;
}

pthread_t join_th[100];
int join_pos = 0;
SMutex join_lock;

void add_join_thread(pthread_t t) {
    mutex_init(&join_lock);
    mutex_lock(&join_lock);
    join_th[join_pos++] = t;
    LOG("%s: pthread %lx", __FUNCTION__, t);
    mutex_unlock(&join_lock);
}

void join_thread() {
    int i, rv;
    if (!join_lock.enabled)
        return;
    mutex_lock(&join_lock);
    //	LOG("starting %s", __FUNCTION__);
    for (i = 0; i < join_pos; i++) {
        LOGM("Joining thread %lx", join_th[i]);
        if ((rv = pthread_join(join_th[i], NULL)))
            LOG("Join thread failed for %lx with %d %s", join_th[i], rv,
                strerror(rv));
    }
    join_pos = 0;
    mutex_unlock(&join_lock);
}

int init_utils(char *arg0) {
    set_signal_handler(arg0);
    return 0;
}

void _hexdump(char *desc, void *addr, int len) {
    int i, pos = 0, bl = (len * 6 < 100) ? 100 : len * 6;
    char buff[17];
    char buf[bl];
    unsigned char *pc = (unsigned char *)addr;

    if (len == 0) {
        LOG("%s: ZERO LENGTH", desc);
        return;
    }
    if (len < 0) {
        LOG("%s: NEGATIVE LENGTH: %i\n", desc, len);
        return;
    }
    memset(buf, 0, bl - 1);
    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                strlcatf(buf, bl, pos, "  %s\n", buff);

            // Output the offset.
            strlcatf(buf, bl, pos, "  %04x ", i);
        }

        // Now the hex code for the specific character.
        strlcatf(buf, bl, pos, " %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        strlcatf(buf, bl, pos, "   ");
        i++;
    }

    // And print the final ASCII bit.
    strlcatf(buf, bl, pos, "  %s\n", buff);
    if (!desc)
        LOG("\n%s", buf)
    else
        LOG("%s:\n%s", desc, buf);
}

static uint32_t crc_tab[256] = {
    0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
    0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
    0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
    0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
    0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
    0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
    0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
    0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
    0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
    0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
    0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
    0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
    0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
    0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
    0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
    0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
    0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
    0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
    0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
    0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
    0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
    0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
    0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
    0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
    0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
    0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
    0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
    0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
    0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
    0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
    0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
    0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
    0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
    0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
    0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
    0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
    0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
    0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
    0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
    0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
    0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
    0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
    0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4};

uint32_t crc_32(const uint8_t *data, int datalen) {
    uint32_t crc = 0xFFFFFFFF;
    if (datalen < 0)
        return crc;
    hexdump("crc_32 ", (uint8_t *)data, datalen);
    while (datalen--) {
        crc = (crc << 8) ^ crc_tab[((crc >> 24) ^ *data++) & 0xff];
    }
    return crc;
}

void _dump_packets(char *message, unsigned char *b, int len,
                   int packet_offset) {
    int i, pid, cc;
    uint32_t crc;

    for (i = 0; i < len; i += 188) {
        crc = crc_32(b + i + 4, 184); // skip header
        pid = PID_FROM_TS(b + i);
        cc = b[i + 3] & 0xF;
        LOG("%s: pid %04d (%04X) CC=%X CRC=%08X%s pos: %d packet %d : "
            "[%02X "
            "%02X "
            "%02X %02X] %02X %02X %02X %02X",
            message, pid, pid, cc, crc, (b[i + 3] & 0x80) ? "encrypted" : "",
            i + packet_offset, (packet_offset + i) / 188, b[i], b[i + 1],
            b[i + 2], b[i + 3], b[i + 4], b[i + 5], b[i + 6], b[i + 7]);
    }
}

int buffer_to_ts(uint8_t *dest, int dstsize, uint8_t *src, int srclen, char *cc,
                 int pid) {
    int pos = 0, left = 0, len = 0;
    uint8_t *b;

    while ((srclen > 0) && (len < dstsize)) {
        if (dstsize - len < 188)
            LOG_AND_RETURN(-1,
                           "Not enough space to copy pid %d, len %d from "
                           "%d, srclen %d",
                           pid, len, dstsize, srclen)
        b = dest + len;
        *cc = ((*cc) + 1) % 16;
        b[0] = 0x47;
        b[1] = pid >> 8;
        if (pos == 0)
            b[1] |= 0x40;
        b[2] = pid & 0xFF;
        b[3] = 0x10 | *cc;
        left = srclen > 184 ? 184 : srclen;
        memcpy(b + 4, src + pos, left);
        pos += left;
        srclen -= left;
        if (left < 184)
            memset(b + left + 4, -1, 184 - left);
        if (opts.debug & DEFAULT_LOG) {
            LOG("pid %d, left -> %d, len %d, cc %d", pid, left, len, *cc);
            hexdump("packet -> ", b, 188);
        }
        len += 188;
    }
    return len;
}

// http://nion.modprobe.de/blog/archives/357-Recursive-directory-creation.html
void mkdir_recursive(const char *path) {
    char tmp[PATH_MAX];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++)
        if (*p == '/') {
            *p = 0;
            if (mkdir(tmp, S_IRWXU))
                LOG("mkdir %s failed", tmp);
            *p = '/';
        }
    if (mkdir(tmp, S_IRWXU))
        LOG("mkdir %s failed", tmp);
}

// https://trac.streamboard.tv/oscam/browser/trunk/oscam-time.c#L172
void sleep_msec(uint32_t msec) {
    // does not interfere with signals like sleep and usleep do
    struct timespec req_ts;
    req_ts.tv_sec = msec / 1000;
    req_ts.tv_nsec = (msec % 1000) * 1000000L;
    int32_t olderrno = errno; // Some OS (especially MacOSX) seem to set errno
                              // to ETIMEDOUT when sleeping

    while (1) {
        /* Sleep for the time specified in req_ts. If interrupted by a
        signal, place the remaining time left to sleep back into req_ts. */
        int rval = nanosleep(&req_ts, &req_ts);
        if (rval == 0)
            break; // Completed the entire sleep time; all done.
        else if (errno == EINTR)
            continue; // Interrupted by a signal. Try again.
        else
            break; // Some other error; bail out.
    }
    errno = olderrno;
}

int get_random(unsigned char *dest, int len) {
    int fd;
    char *urnd = "/dev/urandom";

    fd = open(urnd, O_RDONLY);
    if (fd < 0) {
        LOG("cannot open %s", urnd);
        return -1;
    }

    if (read(fd, dest, len) != len) {
        LOG("cannot read from %s", urnd);
        close(fd);
        return -2;
    }

    close(fd);

    return len;
}

void _strncpy(char *a, char *b, int n) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
    strncpy(a, b, n);
    a[n - 1] = 0;
#pragma GCC diagnostic pop
}

/*
void write_buf_to_file(char *file, uint8_t *buf, int len)
{
        int x = open(file, O_RDWR);
        if (x >= 0)
        {
                write(x, buf, len);
                close(x);
        }
        else
                LOG("Could not write %d bytes to %s: %d", len, file, errno);
}
*/
