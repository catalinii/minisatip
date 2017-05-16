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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
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
#include "dvb.h"
#include "dvbapi.h"
#include "tables.h"
#include "ca.h"
#include "minisatip.h"
#include "adapter.h"

extern struct struct_opts opts;

SCA ca[MAX_CA];
int nca;
SMutex ca_mutex;

int add_ca(SCA *c)
{
	int i, new_ca;
	del_ca(c);
	for (i = 0; i < MAX_CA; i++)
		if (!ca[i].enabled)
		{
			mutex_lock(&ca_mutex);
			if (!ca[i].enabled)
				break;
			mutex_unlock(&ca_mutex);
		}
	if (i == MAX_CA)
		LOG_AND_RETURN(0, "No free CA slots for %p", ca);
	new_ca = i;

	ca[new_ca].enabled = 1;
	ca[new_ca].adapter_mask = c->adapter_mask;
	ca[new_ca].id = new_ca;

	for (i = 0; i < sizeof(ca[0].action) / sizeof(ca_action); i++)
	{
		ca[new_ca].action[i] = c->action[i];
	}
	if (new_ca >= nca)
		nca = new_ca + 1;

	init_ca_device(&ca[new_ca]);
	mutex_unlock(&ca_mutex);
	return new_ca;
}

void del_ca(SCA *c)
{
	int i, j, k, eq, mask = 1;
	adapter *ad;
	mutex_lock(&ca_mutex);

	for (i = 0; i < MAX_CA; i++)
	{
		if (ca[i].enabled)
		{
			eq = 1;
			for (j = 0; j < sizeof(ca[0].action) / sizeof(ca_action); j++)
				if (ca[i].action[j] != c->action[j])
					eq = 0;
			if (eq)
			{
				ca[i].enabled = 0;
				for (k = 0; k < MAX_ADAPTERS; k++)
				{
					if ((ad = get_adapter_nw(k)))
						ad->ca_mask &= ~mask;
				}
			}
		}
		mask = mask << 1;
	}
	i = MAX_CA;
	while (--i >= 0 && !ca[i].enabled)
		;
	nca = i + 1;

	//	if (nca == 1)
	//		nca = 0;

	mutex_unlock(&ca_mutex);
}

int run_ca_action(int action_id, adapter *ad, void *arg)
{
	int i, mask = 1;
	int rv = 0;
	for (i = 0; i < nca; i++)
	{
		if (ca[i].enabled && (ad->ca_mask & mask) && ca[i].action[action_id])
		{
			rv += ca[i].action[action_id](ad, arg);
		}
		mask = mask << 1;
	}
	return rv;
}

int tables_init_ca_for_device(int i, adapter *ad)
{
	int mask = (1 << i);
	int rv = 0;
	int action_id = CA_INIT_DEVICE;

	if (i < 0 || i >= nca)
		return 0;

	if ((ca[i].adapter_mask & mask) && !(ad->ca_mask & mask)) // CA registered and not already initialized
	{
		if (ca[i].enabled && ca[i].action[action_id])
			if (ca[i].action[action_id](ad, NULL))
			{
				ad->ca_mask = ad->ca_mask | mask;
				rv = 1;
			}
	}
	return rv;
}

void send_pmt_to_ca_for_device(SCA *c, adapter *ad)
{
	SPMT *pmt;
	int i;
	for (i = 0; i < MAX_PMT; i++)
		if ((pmt = get_pmt(i)) && pmt->adapter == ad->id && pmt->running)
		{
			if (c->action[CA_ADD_PMT])
				c->action[CA_ADD_PMT](ad, pmt);
		}
}

int register_ca_for_adapter(int i, int aid)
{
	adapter *ad = get_adapter(aid);
	int mask, rv;
	if (i < 0 || i >= nca)
		return 1;
	if (!ad)
		return 1;

	mask = (1 << ad->id);
	if ((ca[i].adapter_mask & mask) == 0)
	{
		ca[i].adapter_mask |= mask;
		rv = tables_init_ca_for_device(i, ad);
		if (rv)
			send_pmt_to_ca_for_device(&ca[i], ad);
		return 0;
	}
	return 2;
}

int unregister_ca_for_adapter(int i, int aid)
{
	adapter *ad = get_adapter(aid);
	int mask;
	if (i < 0 || i >= nca)
		return 1;
	if (!ad)
		return 1;

	mask = (1 << ad->id);
	ca[i].adapter_mask &= ~mask;
	mask = (1 << i);
	if (ad->ca_mask & mask)
	{
		run_ca_action(CA_CLOSE_DEVICE, ad, NULL);
		ad->ca_mask &= ~mask;
		LOG("Unregistering CA %d for adapter %d", i, ad->id);
	}
	return 0;
}

int tables_init_device(adapter *ad)
{
	//	ad->ca_mask = run_ca_action(CA_INIT_DEVICE, ad, NULL);
	int i;
	int rv = 0;
	for (i = 0; i < nca; i++)
		if (ca[i].enabled)
			rv += tables_init_ca_for_device(i, ad);
	return rv;
}

void init_ca_device(SCA *c)
{
	int i, init_cm;
	adapter *ad;
	if (!c->action[CA_ADD_PMT])
		return;

	for (i = 0; i < MAX_ADAPTERS; i++)
		if ((ad = get_adapter_nw(i)))
		{
			init_cm = ad->ca_mask;
			//			tables_init_device(ad);
			tables_init_ca_for_device(c->id, ad);
			if (init_cm != ad->ca_mask)
			{
				send_pmt_to_ca_for_device(c, ad);
			}
		}
}

int tables_close_device(adapter *ad)
{
	int rv = run_ca_action(CA_CLOSE_DEVICE, ad, NULL);
	ad->ca_mask = 0;
	return rv;
}

int tables_init()
{
	mutex_init(&ca_mutex);
#ifndef DISABLE_DVBCA
	dvbca_init();
#endif
#ifndef DISABLE_DVBAPI
	init_dvbapi();
#endif
	return 0;
}

int tables_destroy()
{
	adapter tmp;
	tmp.ca_mask = -1;
	run_ca_action(CA_CLOSE, &tmp, NULL);
	return 0;
}
