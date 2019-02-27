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
#include "socketworks.h"

/*
// external references of utils.c
int run_loop;
char pid_file[50];
__thread pthread_t tid;
char app_name[] = "test";
__thread char *strcpy(thread_name, "test");;

struct struct_opts opts;
*/

typedef struct
{
	int key, value;
} Skey_value;

#define MAX_KV 100
Skey_value kv[MAX_KV];
#define KEY_FOR_INDEX(i) (((i % 7) << 16) + i)

int test_get_index_hash()
{
	extern int64_t hash_calls, hash_conflicts;
	int64_t calls50 = 0, conflicts50 = 0;
	int idx, i;
	hash_calls = 0, hash_conflicts = 0;
	memset(kv, -1, sizeof(kv));
	for (i = 0; i < MAX_KV; i++)
	{
		idx = get_index_hash(&kv[0].key, MAX_KV, sizeof(Skey_value), KEY_FOR_INDEX(i), KEY_FOR_INDEX(i));
		if (idx != -1)
			LOG_AND_RETURN(1, "found already existing elemets with the same key %d", i);
		idx = get_index_hash(&kv[0].key, MAX_KV, sizeof(Skey_value), KEY_FOR_INDEX(i), (uint32_t)-1);
		if (idx == -1)
			LOG_AND_RETURN(1, "found no elemets for key %d", i)
		else if (idx < 0 || idx >= MAX_KV)
			LOG_AND_RETURN(1, "value outside of bounds for key %d, val %d", i, idx);
		if (kv[idx].value != -1)
			LOG_AND_RETURN(1, "re-using old item at pos %d, old index %d old key %d", i, kv[idx].value, kv[idx].value);
		kv[idx].key = KEY_FOR_INDEX(i);
		kv[idx].value = idx;
	}
	idx = get_index_hash(&kv[0].key, MAX_KV, sizeof(Skey_value), KEY_FOR_INDEX(i), (uint32_t)-1);
	if (idx != -1)
		LOG_AND_RETURN(1, "there should be key with value -1 present, but found at position %d", idx);
	hash_calls = 0, hash_conflicts = 0;
	for (i = 0; i < MAX_KV; i++)
	{
		idx = get_index_hash(&kv[0].key, MAX_KV, sizeof(Skey_value), KEY_FOR_INDEX(i), KEY_FOR_INDEX(i));
		if (idx < 0 || idx >= MAX_KV)
			LOG_AND_RETURN(1, "value outside of bounds when checking key %d", KEY_FOR_INDEX(i));
		if (idx == -1)
			LOG_AND_RETURN(1, "did not find any key with value %d", i);
		if (idx != kv[idx].value || (KEY_FOR_INDEX(i) != kv[idx].key))
			LOG_AND_RETURN(1, "inconsistent results found for key %d, index %d, key %d value %d", KEY_FOR_INDEX(i), idx, kv[idx].key, kv[idx].value);
		if (i > MAX_KV / 2 && !calls50)
		{
			calls50 = hash_calls;
			conflicts50 = hash_conflicts;
		}
	}
	for (i = 0; i < MAX_KV; i++)
	{
		if (kv[i].key == -1)
			LOG_AND_RETURN(1, "inconsistent key found at index %d", i);
		if (kv[i].value != i)
			LOG_AND_RETURN(1, "inconsistent value found at index %d, value %d", i, kv[i].value);
	}
	LOG("hash_calls = %d, hash_conflicts = %d, at 50 %% hash_calls = %d, hash_conflicts = %d", hash_calls, hash_conflicts, calls50, conflicts50);
	return 0;
}
int main()
{
	opts.log = 1;
	strcpy(thread_name, "test");;
	TEST_FUNC(test_get_index_hash(), "testing get_index_hash");
	return 0;
}
