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

#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <net/if.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include <sys/ucontext.h>
#include <unistd.h>
#include <syslog.h>
#include <stdarg.h>
#include <sys/mman.h>
#include "dvb.h"
#include "utils.h"
#include "minisatip.h"

#if !defined(__mips__) && !defined(NO_BACKTRACE)
#include <execinfo.h>
#endif

extern struct struct_opts opts;

#define MAX_DATA 1500 // 16384
#define MAX_SINFO 100
char pn[256];

typedef struct tmpinfo
{
	unsigned char enabled;
	int len;
	int64_t key;
	uint16_t id;
	int max_size;
	int timeout;
	int64_t last_updated;
	unsigned char *data;
} STmpinfo;
STmpinfo sinfo[MAX_SINFO];

SMutex utils_mutex;

STmpinfo *getItemPos(int64_t key)
{
	static STmpinfo *last;
	int i;
	if (last && (last->enabled) && (last->key == key))
	{
		return last;
	}
	for (i = 0; i < MAX_SINFO; i++)
		if (sinfo[i].enabled && sinfo[i].key == key)
		{
			last = sinfo + i;
			return last;
		}
	return NULL;
}

STmpinfo *getFreeItemPos(int64_t key)
{
	int i;
	int64_t tick = getTick();
	for (i = 0; i < MAX_SINFO; i++)
		if (!sinfo[i].enabled
				|| (sinfo[i].timeout
						&& (tick - sinfo[i].last_updated > sinfo[i].timeout)))
		{
			sinfo[i].id = i;
			sinfo[i].timeout = 0;
			LOGL(2,
					"Requested new Item for key %jX, returning %d (enabled %d last_updated %jd timeout %d tick %jd)",
					key, i, sinfo[i].enabled, sinfo[i].last_updated,
					sinfo[i].timeout, tick);
			return sinfo + i;
		}
	return NULL;
}

unsigned char *getItem(int64_t key)
{
	STmpinfo *s = getItemPos(key);
	if (s)
		s->last_updated = getTick();
	return s ? s->data : NULL;
}

int getItemLen(int64_t key)
{
	STmpinfo *s = getItemPos(key);
	return s ? s->len : 0;
}

int getItemSize(int64_t key)
{
	STmpinfo *s = getItemPos(key);
	if (!s)
		return 0;
	return s->max_size;
}

int setItemSize(int64_t key, uint32_t max_size)
{
	STmpinfo *s = getItemPos(key);
	if (!s)
		return -1;
	if (s->max_size == max_size)
		return 0;
	s->max_size = max_size;
	if (s->data)
		free1(s->data);
	s->data = malloc1(s->max_size + 100);
	if (!s->data)
		return -1;
	return 0;
}

int setItemTimeout(int64_t key, int tmout)
{
	STmpinfo *s = getItemPos(key);
	if (!s)
		return -1;
	s->timeout = tmout;
	if (!s->data)
		return -1;
	return 0;
}

int setItem(int64_t key, unsigned char *data, int len, int pos) // pos = -1 -> append, owerwrite the existing key
{
	int new_key = 0;
	STmpinfo *s = getItemPos(key);
	if (!s)
	{
		s = getFreeItemPos(key);
		new_key = 1;
	}
	if (!s)
		return -1;

	if (s->max_size == 0)
		s->max_size = MAX_DATA + 10;
	if (!s->data)
		s->data = malloc1(s->max_size);
	if (!s->data)
		return -1;
	s->enabled = 1;
	s->key = key;
	s->last_updated = getTick();
	if (pos == -1)
		pos = s->len;
	if (pos + len >= s->max_size) // make sure we do not overflow the data buffer
		len = s->max_size - pos;

	s->len = pos + len;
	memcpy(s->data + pos, data, len);
	return 0;
}

int delItem(int64_t key)
{
	STmpinfo *s = getItemPos(key);
	s->enabled = 0;
	s->len = 0;
	s->key = 0;
	LOG("Deleted Item Pos %d", s->id);
	return 0;
}

int delItemP(void *p)
{
	int i;
	for (i = 0; i < MAX_SINFO; i++)
		if (sinfo[i].enabled && sinfo[i].data == p)
			delItem(sinfo[i].key);
	return 0;
}

int split(char **rv, char *s, int lrv, char sep)
{
	int i = 0, j = 0;

	if (!s)
		return 0;
	for (i = 0; s[i] && s[i] == sep && s[i] < 32; i++)
		;

	rv[j++] = &s[i];
	//      LOG("start %d %d\n",i,j);
	while (j < lrv)
	{
		if (s[i] == 0 || s[i + 1] == 0)
			break;
		if (s[i] == sep || s[i] < 33)
		{
			s[i] = 0;
			if (s[i + 1] != sep && s[i + 1] > 32)
				rv[j++] = &s[i + 1];
		}
		else if (s[i] < 14)
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
	if (s < (char *) 1000)
		return NULL;

	while (*s && *s == ' ')
		s++;
	return s;
}

#define LR(s) {LOG("map_int returns %d",s);return s;}
int map_intd(char *s, char ** v, int dv)
{
	int i, n = dv;

	if (s == NULL)
	{
		LOG_AND_RETURN(dv, "map_int: s=>NULL, v=%p, %s %s", v,
				v ? v[0] : "NULL", v ? v[1] : "NULL");
	}

	s = strip(s);

	if (!*s)
		LOG_AND_RETURN(dv, "map_int: s is empty");

	if (v == NULL)
	{
		if (s[0] != '+' && s[0] != '-' && (s[0] < '0' || s[0] > '9'))
			LOG_AND_RETURN(dv, "map_int: s not a number: %s, v=%p, %s %s", s, v,
					v ? v[0] : "NULL", v ? v[1] : "NULL");
		return atoi(s);
	}
	for (i = 0; v[i]; i++)
		if (!strncasecmp(s, v[i], strlen(v[i])))
			n = i;
	return n;
}

char *header_parameter(char **arg, int i) // get the value of a header parameter 
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

int map_float(char *s, int mul)
{
	float f;
	int r;

	if (s == NULL)
		LOG_AND_RETURN(0, "map_float: s=>NULL, mul=%d", mul);
	if (s[0] != '+' && s[0] != '-' && (s[0] < '0' || s[0] > '9'))
		LOG_AND_RETURN(0, "map_float: s not a number: %s, mul=%d", s, mul);

	f = atof(s);
	r = (int) (f * mul);
	//      LOG("atof returned %.1f, mul = %d, result=%d",f,mul,r);
	return r;
}

int map_int(char *s, char ** v)
{
	return map_intd(s, v, 0);
}

int end_of_header(char *buf)
{
	return buf[0] == 0x0d && buf[1] == 0x0a && buf[2] == 0x0d && buf[3] == 0x0a;
}

void posix_signal_handler(int sig, siginfo_t * siginfo, ucontext_t * ctx);
void set_signal_handler(char *argv0)
{
	struct sigaction sig_action =
	{ };
	sig_action.sa_sigaction =
			(void (*)(int, siginfo_t *, void *)) posix_signal_handler;
	sigemptyset(&sig_action.sa_mask);

	memset(pn, 0, sizeof(pn));
	strncpy(pn, argv0, sizeof(pn) - 1);

	sig_action.sa_flags = SA_SIGINFO | SA_ONSTACK;

#ifndef __mips__
	if (sigaction(SIGBUS, &sig_action, NULL) != 0)
	{
		LOG("Could not set signal SIGBUS");
	}
	if (sigaction(SIGSEGV, &sig_action, NULL) != 0)
	{
		LOG("Could not set signal SIGSEGV");
	}
	if (sigaction(SIGABRT, &sig_action, NULL) != 0)
	{
		LOG("Could not set signal SIGABRT");
	}
	if (sigaction(SIGFPE, &sig_action, NULL) != 0)
	{
		LOG("Could not set signal SIGFPE");
	}
	if (sigaction(SIGILL, &sig_action, NULL) != 0)
	{
		LOG("Could not set signal SIGILL");
	}
#endif
	if (sigaction(SIGINT, &sig_action, NULL) != 0)
	{
		LOG("Could not set signal SIGINT");
	}

	//    if (sigaction(SIGTERM, &sig_action, NULL) != 0) { err(1, "sigaction"); }
	if (signal(SIGHUP, SIG_IGN) != 0)
	{
		LOG("Could not ignore signal SIGHUP");
	}
	if (signal(SIGPIPE, SIG_IGN) != 0)
	{
		LOG("Could not ignore signal SIGPIPE");
	}
}

int addr2line(char const * const program_name, void const * const addr)
{
	char addr2line_cmd[512] =
	{ 0 };

	sprintf(addr2line_cmd, "addr2line -f -p -e %.256s %p", program_name, addr);
	return system(addr2line_cmd);
}

void print_trace(void)
{
	void *array[10];
	size_t size;
	char **strings;
	size_t i;
#if !defined(NO_BACKTRACE)

	size = backtrace(array, 10);

	printf("Obtained %zd stack frames.\n", size);

	for (i = 0; i < size; i++)
	{
		printf("%p : ", array[i]);
		if (addr2line(pn, array[i]))
			printf("\n");
	}
#else
	printf( " No backtrace defined\n");
#endif
}

extern int run_loop;

void posix_signal_handler(int sig, siginfo_t * siginfo, ucontext_t * ctx)
{
	int sp = 0, ip = 0;

	if (sig == SIGINT)
	{
		run_loop = 0;
		return;
	}
#ifdef __mips__
	sp = ctx->uc_mcontext.gregs[29];
	ip = ctx->uc_mcontext.pc;
#endif
	printf("RECEIVED SIGNAL %d - SP=%lX IP=%lX\n", sig, (long unsigned int) sp,
			(long unsigned int) ip);

	print_trace();
	exit(1);
}

int /* Returns 0 on success, -1 on error */
becomeDaemon()
{
	int maxfd, fd, pid;
	__attribute__((unused)) int rv;
	struct stat sb;
	FILE *f;
	char path[255];
	char buf[255];

	memset(path, 0, sizeof(path));
	if ((f = fopen(pid_file, "rt")))
	{
		rv = fscanf(f, "%d", &pid);
		fclose(f);
		snprintf(buf, sizeof(buf), "/proc/%d/exe", pid);

		if (0 < readlink(buf, path, sizeof(path)) && 0 == strcmp(pn, path))
		{
			LOGL(0, "Found %s running with pid %d, killing....", app_name, pid);
			kill(pid, SIGINT);
			usleep(500);
		}
	}

	switch (fork())
	{ /* Become background process */
	case -1:
		return -1;
	case 0:
		break; /* Child falls through... */
	default:
		_exit(0);/* while parent terminates */
	}

	if (setsid() == -1) /* Become leader of new session */
		return -1;

	switch ((pid = fork()))
	{ /* Ensure we are not session leader */
	case -1:
		return -1;
	case 0:
		break;
	default:
		_exit(0);
	}

	umask(0); /* Clear file mode creation mask */

	maxfd = sysconf(_SC_OPEN_MAX);
	if (maxfd == -1) /* Limit is indeterminate... */
		maxfd = 1024; /* so take a guess */

	for (fd = 0; fd < maxfd; fd++)
		close(fd);

	close(STDIN_FILENO); /* Reopen standard fd's to /dev/null */
	//	chdir ("/tmp");				 /* Change to root directory */

	fd = open("/dev/null", O_RDWR);

	snprintf(buf, sizeof(buf), "/tmp/%s.log", app_name);

	if (fd != STDIN_FILENO) /* 'fd' should be 0 */
		return -1;
	if (stat(buf, &sb) == -1)
		fd = open(buf, O_RDWR | O_CREAT, 0666);
	else
		fd = open(buf, O_RDWR | O_APPEND);
	if (fd != STDOUT_FILENO) /* 'fd' should be 1 */
		return -1;
	if (dup2(STDOUT_FILENO, STDERR_FILENO) != STDERR_FILENO)
		return -1;

	return 0;
}

void *
mymalloc(int a, char *f, int l)
{
	void *x = malloc(a);
	if (x)
		memset(x, 0, a);
	LOG("%s:%d allocation_wrapper malloc allocated %d bytes at %p", f, l, a, x);
	return x;
}

void myfree(void *x, char *f, int l)
{
	LOG("%s:%d allocation_wrapper free called with argument %p", f, l, x);
	free(x);
}

pthread_mutex_t log_mutex;

void _log(int level, char * file, int line, char *fmt, ...)
{
	va_list arg;
	int len, len1, both;
	static int idx, times;
	int tl;
	char stid[50];
	static char output[2][2000]; // prints just the first 2000 bytes from the message 

	/* Check if the message should be logged */
	opts.last_log = fmt;
	if (opts.log < level)
		return;

	stid[0] = 0;
	if (!opts.no_threads)
	{
		pthread_mutex_lock(&log_mutex);
		snprintf(stid, sizeof(stid) - 2, " %s", thread_name);
		stid[sizeof(stid) - 1] = 0;
	}

	if (!fmt)
	{
		printf("NULL format at %s:%d !!!!!", file, line);
		if (!opts.no_threads)
			pthread_mutex_unlock(&log_mutex);
		return;
	}
	idx = 1 - idx;
	if (idx > 1)
		idx = 1;
	else if (idx < 0)
		idx = 0;
	if (opts.file_line && !opts.slog)
		len1 = snprintf(output[idx], sizeof(output[0]), "[%s%s] %s:%d: ",
				get_current_timestamp_log(), stid, file, line);
	else if (!opts.slog)
		len1 = snprintf(output[idx], sizeof(output[0]), "[%s%s]: ",
				get_current_timestamp_log(), stid);
	else if (opts.file_line)
	{
		len1 = 0;
		output[idx][0] = '\0';
	}
	/* Write the error message */
	len = len1 =
			len1 < (int) sizeof(output[0]) ? len1 : (int) sizeof(output[0]) - 1;
	both = 0;
	va_start(arg, fmt);
	len += vsnprintf(output[idx] + len, sizeof(output[0]) - len, fmt, arg);
	va_end(arg);

	if (strcmp(output[idx] + len1, output[1 - idx] + len1) == 0)
		times++;
	else
	{
		if (times > 0)
		{
			both = 1;
			snprintf(output[1 - idx], sizeof(output[0]),
					"Message repeated %d times", times);
		}
		times = 0;
	}

	if (both)
	{
		if (opts.slog)
			syslog(LOG_NOTICE, "%s", output[1 - idx]);
		else
			puts(output[1 - idx]);
		both = 0;
	}
	if (times == 0)
	{
		if (opts.slog)
			syslog(LOG_NOTICE, "%s", output[idx]);
		else
			puts(output[idx]);
	}
	fflush(stdout);
	if (!opts.no_threads)
		pthread_mutex_unlock(&log_mutex);
}

int endswith(char *src, char *with)
{
	int lw = strlen(with);
	if (strlen(src) > lw && !strcmp(src + strlen(src) - lw, with))
		return 1;
	return 0;
}

#define VAR_LENGTH 20
extern _symbols adapters_sym[];
extern _symbols minisatip_sym[];
extern _symbols stream_sym[];
#ifndef DISABLE_DVBAPI
extern _symbols dvbapi_sym[];
#endif
#ifndef DISABLE_SATIPCLIENT
extern _symbols satipc_sym[];
#endif

_symbols *sym[] =
{ adapters_sym, stream_sym, minisatip_sym,
#ifndef DISABLE_DVBAPI
		dvbapi_sym,
#endif
#ifndef DISABLE_SATIPCLIENT
		satipc_sym,
#endif
		NULL };

int snprintf_pointer(char *dest, int max_len, int type, void *p,
		float multiplier)
{
	int nb;
	switch (type & 0xF)
	{
	case VAR_UINT8:
		nb = snprintf(dest, max_len, "%d",
				(int) ((*(unsigned char *) p) * multiplier));
		break;
	case VAR_INT8:
		nb = snprintf(dest, max_len, "%d", (int) ((*(char *) p) * multiplier));
		break;
	case VAR_UINT16:
		nb = snprintf(dest, max_len, "%d",
				(int) ((*(uint16_t *) p) * multiplier));
		break;
	case VAR_INT16:
		nb = snprintf(dest, max_len, "%d",
				(int) ((*(int16_t *) p) * multiplier));
		break;
	case VAR_INT:
		nb = snprintf(dest, max_len, "%d", (int) ((*(int *) p) * multiplier));
		break;

	case VAR_INT64:
		nb = snprintf(dest, max_len, "%jd",
				(int64_t) ((*(int64_t *) p) * multiplier));
		break;

	case VAR_STRING:
		nb = snprintf(dest, max_len, "%s", (char *) p);
		break;

	case VAR_PSTRING:
		nb = snprintf(dest, max_len, "%s",
				(*(char **) p) ? (*(char **) p) : "");
		break;

	case VAR_FLOAT:
		nb = snprintf(dest, max_len, "%f", (*(float *) p) * multiplier);
		break;

	case VAR_HEX:
		nb = snprintf(dest, max_len, "0x%x", (int) ((*(int *) p) * multiplier));
		break;
	}
	return nb;
}

char zero[16];

void * get_var_address(char *var, float *multiplier, int * type, void *storage,
		int ls)
{
	int nb = 0, i, j, off;
	*multiplier = 0;
	for (i = 0; sym[i] != NULL; i++)
		for (j = 0; sym[i][j].name; j++)
			if (!strncmp(sym[i][j].name, var, strlen(sym[i][j].name)))
			{
				*type = sym[i][j].type;
				if (sym[i][j].type < VAR_ARRAY)
				{
					*multiplier = sym[i][j].multiplier;
					return sym[i][j].addr;
				}
				else if ((sym[i][j].type & 0xF0) == VAR_ARRAY)
				{
					off = map_intd(var + strlen(sym[i][j].name), NULL, 0);
					if (off >= 0 && off < sym[i][j].len)
					{
						*multiplier = sym[i][j].multiplier;
						return (((char *) sym[i][j].addr) + off * sym[i][j].skip);
					}
				}
				else if ((sym[i][j].type & 0xF0) == VAR_AARRAY)
				{
					off = map_intd(var + strlen(sym[i][j].name), NULL, 0);
					if (off >= 0 && off < sym[i][j].len)
					{
						char **p1 = (char **) sym[i][j].addr;
						char *p = p1[off];

						if (!p)
						{
							memset(zero, 0, sizeof(zero));
							p = zero;
						}
						else
							p += sym[i][j].skip;
						*multiplier = sym[i][j].multiplier;
						return p;
					}
				}
				else if (sym[i][j].type == VAR_FUNCTION_INT)
				{
					off = map_intd(var + strlen(sym[i][j].name), NULL, 0);
					get_data_int funi = (get_data_int) sym[i][j].addr;
					*(int *) storage = funi(off);
					*multiplier = 1;
					return storage;
				}
				else if (sym[i][j].type == VAR_FUNCTION_STRING)
				{
					off = map_intd(var + strlen(sym[i][j].name), NULL, 0);
					get_data_string funs = (get_data_string) sym[i][j].addr;
					funs(off, storage, ls);
					return storage;
				}

			}
	return NULL;
}

int var_eval(char *orig, int len, char *dest, int max_len)
{
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

int is_var(char *s)
{
	int i = 0;
	if (*s != '$')
		return 0;
	while (s[++i] != 0)
	{
		if (s[i] == '$')
			return i;
		if (i >= VAR_LENGTH)
			return 0;
	}
	return 0;
}

// replace $VAR$ with it's value and write the output to the socket
void process_file(void *sock, char *s, int len, char *ctype)
{
	char outp[8292];
	int i, io = 0, lv, le, respond = 1;
	sockets *so = (sockets *) sock;
	__attribute__((unused)) int rv;
	LOG("processing_file %x len %d:", s, len);
	for (i = 0; i < len; i++)
	{
		lv = 0;
		if (s[i] == '$')
			lv = is_var(s + i);
		if (lv == 0)
		{
			outp[io++] = s[i];
		}
		else
		{
			le = var_eval(s + i, lv, outp + io, sizeof(outp) - io);
			io += le;
			i += lv;
		}
		if (io > sizeof(outp) - 100)
		{
			if (respond)
			{
				http_response(so, 200, ctype, "", 0, 0); // sending back the response without Content-Length
				respond = 0;
			}
			rv = write(so->sock, outp, io);
			outp[io] = 0;
			LOG("%s", outp);
			io = 0;
		}
	}
	outp[io] = 0;
	if (respond)
		http_response(so, 200, ctype, outp, 0, 0); // sending back the response with Content-Length if output < 8192
	else
	{
		strcpy(outp + io, "\r\n\r\n");
		rv = write(so->sock, outp, io + 4);
		outp[io] = 0;
		LOG("%s", outp);
	}
}

char *readfile(char *fn, char *ctype, int *len)
{
	char ffn[256];
	char *mem;
	struct stat sb;
	int fd, i, nl = 0, sr;
	*len = 0;
	ctype[0] = 0;

	if (strstr(fn, ".."))
		return 0;
	snprintf(ffn, sizeof(ffn), "%s/%s", opts.document_root, fn);
	ffn[sizeof(ffn) - 1] = 0;
	if ((fd = open(ffn, O_RDONLY | O_LARGEFILE)) < 0)
		LOG_AND_RETURN(NULL, "Could not open file %s", ffn);
	if ((fstat(fd, &sb) == -1) || !S_ISREG(sb.st_mode))
	{
		LOG("readfile: %s is not a file", ffn);
		close(fd);
		return NULL;
	}
	nl = sb.st_size;
	mem = mmap(0, nl, PROT_READ, MAP_SHARED, fd, 0);
	if (mem == MAP_FAILED)
		LOG_AND_RETURN(NULL, "mmap failed for file %s", ffn);
	close(fd);
	LOG("opened %s fd %d at %x - %d bytes", ffn, fd, mem, nl);

	*len = nl;
	if (ctype)
	{
		if (endswith(fn, "png"))
			strcpy(ctype, "Content-type: image/png\r\nConnection: close");
		else if (endswith(fn, "jpg") || endswith(fn, "jpeg"))
			strcpy(ctype, "Content-type: image/jpeg\r\nConnection: close");
		else if (endswith(fn, "css"))
			strcpy(ctype, "Content-type: text/css\r\nConnection: close");
		else if (endswith(fn, "js"))
			strcpy(ctype, "Content-type: text/javascript\r\nConnection: close");
		else if (endswith(fn, "htm") || endswith(fn, "html"))
			strcpy(ctype, "CACHE-CONTROL: no-cache\r\nContent-type: text/html");
		else if (endswith(fn, "xml"))
			strcpy(ctype, "CACHE-CONTROL: no-cache\r\nContent-type: text/xml");
		else if (endswith(fn, "m3u"))
			strcpy(ctype,
					"CACHE-CONTROL: no-cache\r\nContent-type: video/x-mpegurl");
	}
	return mem;
}

int closefile(char *mem, int len)
{
	return munmap((void *) mem, len);
}

int mutex_init(SMutex* mutex)
{
	int rv;
	pthread_mutexattr_t attr;

	if (opts.no_threads)
		return 0;
	if (mutex->enabled)
		return 1;

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

	if ((rv = pthread_mutex_init(&mutex->mtx, &attr)))
	{
		LOG("mutex init %p failed with error %d %s", mutex, rv, strerror(rv));
		return rv;
	}

	mutex->create_time = getTick();
	mutex->enabled = 1;
	mutex->state = 0;
	LOG("Mutex init %p", mutex);
	return 0;
}

__thread SMutex *mutexes[50];
__thread int imtx = 0;

int mutex_lock1(char *FILE, int line, SMutex* mutex)
{
	int rv;
	int64_t start_lock = 0;
	if (opts.no_threads)
		return 0;

	if (!mutex || !mutex->enabled)
	{
		LOGL(3, "%s:%d Mutex not enabled %p", FILE, line, mutex);
		return 1;
	}

	if (mutex && mutex->enabled && mutex->state && tid != mutex->tid)
	{
		LOGL(4, "%s:%d Locking mutex %p already locked at %s:%d tid %x", FILE,
				line, mutex, mutex->file, mutex->line, mutex->tid);
		start_lock = getTick();
	}
	else
		LOGL(5, "%s:%d Locking mutex %p", FILE, line, mutex);
	rv = pthread_mutex_lock(&mutex->mtx);
	if (rv != 0)
	{
		LOG("Mutex Lock %p failed", mutex);
		return rv;
	}
	mutex->file = FILE;
	mutex->line = line;
	mutex->state = 1;
	mutex->tid = tid;
	mutex->lock_time = getTick();

	mutexes[imtx++] = mutex;
	if (start_lock > 0)
		LOGL(4, "%s:%d Locked %p after %ld ms", FILE, line, mutex,
				getTick() - start_lock);

	return 0;

}
int mutex_unlock1(char *FILE, int line, SMutex* mutex)
{
	int rv = -1;
	if (opts.no_threads)
		return 0;

	if (!mutex || mutex->enabled)
	{
		LOGL(5, "%s:%d Unlocking mutex %p", FILE, line, mutex);

		rv = pthread_mutex_unlock(&mutex->mtx);
	}
	else
		LOGL(3, "%s:%d Unlock disabled mutex %p", FILE, line, mutex);

	if (rv != 0 && rv != 1 && rv != -1)
	{
		LOGL(3, "mutex_unlock failed at %s:%d: %d %s", FILE, line, rv,
				strerror(rv));
	}
	if (rv == 0 || rv == 1)
		rv = 0;

	if (rv != -1 && imtx > 0)
		if ((imtx >= 1) && mutexes[imtx - 1] == mutex)
			imtx--;
		else if ((imtx >= 2) && mutexes[imtx - 2] == mutex)
		{
			mutexes[imtx - 2] = mutexes[imtx - 1];
			imtx--;
		}
		else
			LOG("mutex_leak: Expected %p got %p", mutex, mutexes[imtx - 1]);

	return rv;
}

int mutex_destroy(SMutex* mutex)
{
	int rv;
	if (opts.no_threads)
		return 0;
	if (!mutex->enabled)
	{
		LOG("destroy disabled mutex %p", mutex);

		return 1;
	}
	mutex->enabled = 0;

	if ((imtx >= 1) && mutexes[imtx - 1] == mutex)
		imtx--;
	else if ((imtx >= 2) && mutexes[imtx - 2] == mutex)
	{
		mutexes[imtx - 2] = mutexes[imtx - 1];
		imtx--;
	}

	if ((rv = pthread_mutex_unlock(&mutex->mtx)) != 1)
		LOG("%s: pthread_mutex_unlock 1 failed for %p with error %d %s",
				__FUNCTION__, mutex, rv, strerror(rv));

	if ((rv = pthread_mutex_unlock(&mutex->mtx)) != 1)
		LOG("%s: pthread_mutex_unlock 2 failed for %p with error %d %s",
				__FUNCTION__, mutex, rv, strerror(rv));

	LOGL(4, "Destroying mutex %p", mutex);
	if ((rv = pthread_mutex_destroy(&mutex->mtx)))
	{
		LOG("mutex destroy %p failed with error %d %s", mutex, rv, strerror(rv));
		mutex->enabled = 1;
		return 1;
	}
	return 0;
}

void clean_mutexes()
{
	int i;
	if (!imtx)
		return;
	if (opts.no_threads)
		return;
//	LOG("mutex_leak: unlock %d mutexes", imtx);
	for (i = imtx - 1; i >= 0; i--)
	{
		if (!mutexes[i])
			continue;
		LOG("mutex_leak: %s unlocking mutex %p from %s:%d", __FUNCTION__,
				mutexes[i], mutexes[i]->file, mutexes[i]->line);
		mutex_unlock(mutexes[i]);
	}
	imtx = 0;
}

pthread_t get_tid()
{
	return pthread_self();
}

pthread_t start_new_thread(char *name)
{
	pthread_t tid;
	int rv;
	if (opts.no_threads)
		return get_tid();

	if ((rv = pthread_create(&tid, NULL, &select_and_execute, name)))
	{
		LOG("Failed to create thread: %s, error %d %s", name, rv, strerror(rv));
		return get_tid();
	}
	return tid;
}

void set_thread_prio(pthread_t tid, int prio)
{
	int rv;
	struct sched_param param;
	memset(&param, 0, sizeof(struct sched_param));
	param.sched_priority = prio;
	if ((rv = pthread_setschedparam(pthread_self(), SCHED_RR, &param)))
		LOG("pthread_setschedparam failed with error %d", rv);
	return;
}

struct struct_array
{
	char enabled;
	SMutex mutex;
};

int add_new_lock(void **arr, int count, int size, SMutex *mutex)
{
	int i;
	struct struct_array **sa = (struct struct_array **) arr;
	mutex_init(mutex);
	mutex_lock(mutex);
	for (i = 0; i < count; i++)
		if (!sa[i] || !sa[i]->enabled)
		{
			if (!sa[i])
			{
				sa[i] = malloc1(size);
				if (!sa[i])
					LOG_AND_RETURN(-1,
							"Could not allocate memory for %p index %d", arr, i);
				memset(sa[i], 0, size);
			}
			mutex_init(&sa[i]->mutex);
			mutex_lock(&sa[i]->mutex);
			sa[i]->enabled = 1;
			mutex_unlock(mutex);
			return i;
		}
	return -1;
}

int64_t init_tick, theTick;

int64_t getTick()
{								 //ms
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	theTick = ts.tv_nsec / 1000000;
	theTick += ts.tv_sec * 1000;
	if (init_tick == 0)
		init_tick = theTick;
	return theTick - init_tick;
}

int64_t getTickUs()
{
	uint64_t utime;
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	utime = ((uint64_t) ts.tv_sec) * 1000000 + ts.tv_nsec / 1000;
	return utime;

}

pthread_t join_th[100];
int join_pos = 0;
SMutex join_lock;

void add_join_thread(pthread_t t)
{
	mutex_init(&join_lock);
	mutex_lock(&join_lock);
	join_th[join_pos++] = t;
	LOG("%s: pthread %x", __FUNCTION__, t);
	mutex_unlock(&join_lock);
}

void join_thread()
{
	int i, rv;
	if (!join_lock.enabled)
		return;
	mutex_lock(&join_lock);
//	LOG("starting %s", __FUNCTION__);	
	for (i = 0; i < join_pos; i++)
	{
		LOGL(3, "Joining thread %x", join_th[i]);
		if ((rv = pthread_join(join_th[i], NULL)))
			LOG("Join thread failed for %x with %d %s", join_th[i], rv,
					strerror(rv));
	}
	join_pos = 0;
	mutex_unlock(&join_lock);
}

