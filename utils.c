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
#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>
#include <linux/dvb/ca.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include <sys/ucontext.h>
#include <unistd.h>
#include <syslog.h>
#include <stdarg.h>
#include <sys/mman.h>
#include "utils.h"
#include "minisatip.h"

extern struct struct_opts opts;
extern char version[], app_name[];

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
	int last_updated;
	uint8_t no_change, prev_no_change;
	unsigned char *data;
} STmpinfo;
STmpinfo sinfo[MAX_SINFO];

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
	int tick = getTick();
	for (i = 0; i < MAX_SINFO; i++)
		if (!sinfo[i].enabled
				|| (sinfo[i].timeout
						&& (tick - sinfo[i].last_updated > sinfo[i].timeout)))
		{
			sinfo[i].id = i;
			sinfo[i].timeout = 0;
			sinfo[i].no_change = 0;
			sinfo[i].prev_no_change = 0;
			LOGL(2, "Requested new Item for key %llX, returning %d", key, i);
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
	if (pos == 0)
	{
		s->prev_no_change = s->no_change;
		s->no_change = 1;
	}
	if (new_key)
		s->no_change = 0;
	if (memcmp(s->data + pos, data, len) != 0)
		s->no_change = 0;

	memcpy(s->data + pos, data, len);
	return 0;
}

int getItemChange(int64_t key, int *prev)
{
	int current;
	*prev = current = -1;
	STmpinfo *s = getItemPos(key);
	if (!s)
		return 0;
	*prev = s->prev_no_change;
	current = s->no_change;
	return current;
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
void set_signal_handler()
{
	struct sigaction sig_action =
	{ };
	sig_action.sa_sigaction =
			(void (*)(int, siginfo_t *, void *)) posix_signal_handler;
	sigemptyset(&sig_action.sa_mask);

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
#ifndef __mips__

	size = backtrace(array, 10);

	LOGL(0, "Obtained %zd stack frames.\n", size);

	for (i = 0; i < size; i++)
	{
		LOGL(0, "%p : ", array[i]);
		if (addr2line(pn, array[i]))
			LOGL(0, "\n");
	}
#else
	LOGL(0, " No backtrace defined\n");
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
	LOGL(0, "RECEIVED SIGNAL %d - SP=%lX IP=%lX\n", sig,
			(long unsigned int ) sp, (long unsigned int ) ip);

	print_trace();
	exit(1);
}

int /* Returns 0 on success, -1 on error */
becomeDaemon()
{
	int maxfd, fd, pid;
	struct stat sb;
	FILE *f;
	char path[255];
	char buf[255];

	memset(path, 0, sizeof(path));
	snprintf(buf, sizeof(buf), PID_FILE, app_name);
	if ((f = fopen(buf, "rt")))
	{
		fscanf(f, "%d", &pid);
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
	LOG("%s:%d allocation_wrapper malloc allocated %d bytes at %p", f, l, a, x);
	return x;
}

void myfree(void *x, char *f, int l)
{
	LOG("%s:%d allocation_wrapper free called with argument %p", f, l, x);
	free(x);
}

void _log(int level, char * file, int line, char *fmt, ...)
{
	va_list arg;
	int len, len1, both;
	static int idx, times;
	static char output[2][2000]; // prints just the first 2000 bytes from the message 

	/* Check if the message should be logged */
	opts.last_log = fmt;
	if (opts.log < level)
		return;

	if (!fmt)
	{
		printf("NULL format at %s:%d !!!!!", file, line);
		return;
	}
	idx = 1 - idx;
	if (idx > 1)
		idx = 1;
	else if (idx < 0)
		idx = 0;
	if (opts.file_line && !opts.slog)
		len1 = snprintf(output[idx], sizeof(output[0]), "[%s] %s:%d: ",
				get_current_timestamp_log(), file, line);
	else if (!opts.slog)
		len1 = snprintf(output[idx], sizeof(output[0]), "[%s]: ",
				get_current_timestamp_log());
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
extern _symbols dvbapi_sym[];

_symbols *sym[] =
{ adapters_sym, stream_sym, minisatip_sym, dvbapi_sym, NULL };

int var_eval(char *orig, int len, char *dest, int max_len)
{
	int nb = 0, i, j, off;
	char var[VAR_LENGTH + 1];
	memset(var, 0, sizeof(var));
	strncpy(var, orig + 1, len - 1);
	for (i = 0; sym[i] != NULL; i++)
		for (j = 0; sym[i][j].name; j++)
			if (!strncmp(sym[i][j].name, var, strlen(sym[i][j].name)))
			{
				switch (sym[i][j].type)
				{
				case VAR_UINT8:
					nb = snprintf(dest, max_len, "%d",
							(int) ((*(uint8_t *) sym[i][j].addr)
									* sym[i][j].multiplier));
					break;
				case VAR_INT8:
					nb = snprintf(dest, max_len, "%d",
							(int) ((*(int8_t *) sym[i][j].addr)
									* sym[i][j].multiplier));
					break;
				case VAR_UINT16:
					nb = snprintf(dest, max_len, "%d",
							(int) ((*(uint16_t *) sym[i][j].addr)
									* sym[i][j].multiplier));
					break;
				case VAR_INT16:
					nb = snprintf(dest, max_len, "%d",
							(int) ((*(int16_t *) sym[i][j].addr)
									* sym[i][j].multiplier));
					break;
				case VAR_INT:
					nb = snprintf(dest, max_len, "%d",
							(int) ((*(int *) sym[i][j].addr)
									* sym[i][j].multiplier));
					break;

				case VAR_STRING:
					nb = snprintf(dest, max_len, "%s", (char *) sym[i][j].addr);
					break;

				case VAR_PSTRING:
					nb = snprintf(dest, max_len, "%s",
							*(char **) sym[i][j].addr);
					break;

				case VAR_FLOAT:
					nb = snprintf(dest, max_len, "%f",
							(*(float *) sym[i][j].addr) * sym[i][j].multiplier);
					break;

				case VAR_HEX:
					nb = snprintf(dest, max_len, "0x%x",
							*(int *) sym[i][j].addr);
					break;

				case VAR_ARRAY_INT:
				case VAR_ARRAY_FLOAT:
				case VAR_ARRAY_HEX:
				case VAR_ARRAY_UINT16:
				case VAR_ARRAY_INT16:
				case VAR_ARRAY_UINT8:
				case VAR_ARRAY_INT8:
				case VAR_ARRAY_STRING:
				case VAR_ARRAY_PSTRING:
					off = map_intd(var + strlen(sym[i][j].name), NULL, 0);
					if (off >= 0 && off < sym[i][j].len)
					{
						char *p = (((char *) sym[i][j].addr)
								+ off * sym[i][j].skip);
						switch (sym[i][j].type)
						{
						case VAR_ARRAY_UINT8:
							nb =
									snprintf(dest, max_len, "%d",
											(int) (*(uint8_t *) p
													* sym[i][j].multiplier));
							break;
						case VAR_ARRAY_INT8:
							nb =
									snprintf(dest, max_len, "%d",
											(int) (*(int8_t *) p
													* sym[i][j].multiplier));
							break;
						case VAR_ARRAY_UINT16:
							nb = snprintf(dest, max_len, "%d",
									(int) (*(uint16_t *) p
											* sym[i][j].multiplier));
							break;
						case VAR_ARRAY_INT16:
							nb =
									snprintf(dest, max_len, "%d",
											(int) (*(int16_t *) p
													* sym[i][j].multiplier));
							break;
						case VAR_ARRAY_INT:
							nb = snprintf(dest, max_len, "%d",
									(int) (*(int *) p * sym[i][j].multiplier));
							break;
						case VAR_ARRAY_FLOAT:
							nb =
									snprintf(dest, max_len, "%f",
											(float) (*(float *) p
													* sym[i][j].multiplier));
							break;
						case VAR_ARRAY_HEX:
							nb = snprintf(dest, max_len, "0x%x",
									(int) (*(int *) p * sym[i][j].multiplier));
							break;
						case VAR_ARRAY_STRING:
							nb = snprintf(dest, max_len, "%s", p);
							break;
						case VAR_ARRAY_PSTRING:
							nb = snprintf(dest, max_len, "%s", *(char **) p);
							break;
						}
					}
					break;

				case VAR_FUNCTION_INT:
					off = map_intd(var + strlen(sym[i][j].name), NULL, 0);
					get_data_int funi = (get_data_int) sym[i][j].addr;
					nb = snprintf(dest, max_len, "%d", funi(off));
					break;

				case VAR_FUNCTION_STRING:
					off = map_intd(var + strlen(sym[i][j].name), NULL, 0);
					get_data_string funs = (get_data_string) sym[i][j].addr;
					funs(off, dest, max_len);
					nb = strlen(dest);
					if (nb > max_len)
						nb = max_len;
					break;

				}
				return nb;
			}
	return 0;
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
void process_file(sockets *so, char *s, int len, char *ctype)
{
	char outp[8292];
	int i, io = 0, lv, le, respond = 1;
	LOG("processing_file %x len %d:", s, len);
	for (i = 0; i < len; i++)
	{
		lv = 0;
		if (s[i] == '$')
			lv = is_var(s + i);
		if (lv == 0)
		{
			outp[io++] = s[i];
			continue;
		}
		le = var_eval(s + i, lv, outp + io, sizeof(outp) - io);
		io += le;
		i += lv;
		if (io > sizeof(outp) - 100)
		{
			if (respond)
			{
				http_response(so, 200, ctype, "", 0, 0); // sending back the response without Content-Length
				respond = 0;
			}
			write(so->sock, outp, io);
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
		write(so->sock, outp, io + 4);
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
	if ((fd = open(ffn, O_RDONLY)) < 0)
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
		else if (endswith(fn, "htm") || endswith(fn, "html"))
			strcpy(ctype, "CACHE-CONTROL: no-cache\r\nContent-type: text/html");
		else if (endswith(fn, "xml"))
			strcpy(ctype, "CACHE-CONTROL: no-cache\r\nContent-type: text/xml");
		else if (endswith(fn, "m3u"))
			strcpy(ctype, "CACHE-CONTROL: no-cache\r\nContent-type: video/x-mpegurl");		
	}
	return mem;
}

int closefile(char *mem, int len)
{
	return munmap((void *) mem, len);
}

