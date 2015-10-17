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
#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>
#include <linux/dvb/ca.h>
#include <fcntl.h>
#include <ctype.h>
#include "dvbapi.h"
#include "tables.h"
#include "ca.h"
#include "minisatip.h"

extern struct struct_opts opts;

#define TEST_WRITE(a) if((a)<=0){LOG("%s:%d: write to dvbapi socket failed, closing socket %d",__FILE__,__LINE__,sock);sockets_del(dvbapi_sock);sock = 0;dvbapi_sock = -1;isEnabled = 0;}

static uint32_t crc_tab[256] =
{ 0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
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
		0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4 };

int process_pat(unsigned char *b, adapter *ad)
{
	int pat_len = 0, i, tid = 0, sid, pid, ver, csid = 0;
	int64_t pid_key = TABLES_ITEM + ((1 + ad->id) << 24) + 0;
	int16_t *pids;
	SPid *p;

	if (((b[1] & 0x1F) != 0) || (b[2] != 0))
		return 0;

	if (b[0] != 0x47)
		return 0;

	if ((b[1] & 0x40) && ((b[4] != 0) || (b[5] != 0)))
		return 0;

	if (b[1] & 0x40)
	{
		tid = b[8] * 256 + b[9];
		ver = b[10] & 0x3E;
		pat_len = (b[6] & 0xF) * 256 + b[7];
		if (((ad->transponder_id != tid) || (ad->pat_ver != ver))
				&& (pat_len > 0) && (pat_len < 1500))
		{
			ad->pat_processed = 0;
		}
	}

	if (ad->pat_processed)
		return 0;

	if (!(pat_len = assemble_packet(&b, ad, 1)))
		return 0;

	dvbapi_delete_keys_for_adapter(ad->id);
//	LOG("tid %d pat_len %d: %02X %02X %02X %02X %02X %02X %02X %02X", tid, pat_len, b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7]);
	tid = b[3] * 256 + b[4];
	ad->pat_ver = b[5] & 0x3E;
	setItem(pid_key, b, 1, 0);
	setItemSize(pid_key, 8192 * sizeof(*pids));
	pids = (int16_t *) getItem(pid_key);
	memset(pids, 0, 8192 * sizeof(*pids));
	pat_len -= 9;
	b += 8;
	LOGL(2, "PAT Adapter %d, Transponder ID %d, len %d, version %d", ad->id,
			tid, pat_len, ad->pat_ver);
	if (pat_len > 1500)
		return 0;

	ad->transponder_id = tid;
	for (i = 0; i < pat_len; i += 4)
	{
		sid = b[i] * 256 + b[i + 1];
		pid = (b[i + 2] & 0x1F) * 256 + b[i + 3];
		LOGL(2, "Adapter %d, PMT sid %d, pid %d", ad->id, sid, pid);
		if (sid > 0)
		{
			pids[pid] = -TYPE_PMT;
			p = find_pid(ad->id, pid);
#ifndef DISABLE_DVBCSA
			if (!p)
				mark_pid_add(-1, ad->id, pid);
#endif
			if ((p = find_pid(ad->id, pid)))
			{
				p->type = TYPE_PMT;
				csid = pid;
				if (p->flags == 3)
					p->flags = 1;
			}
		}
	}
	update_pids(ad->id);
	ad->pat_processed = 1;
	return csid;
}

#define ASSEMBLE_TIMEOUT 1000

int assemble_packet(uint8_t **b1, adapter *ad, int check_crc)
{
	int len = 0, pid;
	uint32_t crc, current_crc;
	int64_t item_key;
	uint8_t *b = *b1;

	if ((b[0] != 0x47)) // make sure we are dealing with TS
		return 0;

	pid = (b[1] & 0x1F) * 256 + b[2];

	if ((b[1] & 0x40) == 0x40)
		len = ((b[6] & 0xF) << 8) + b[7];

	if (len > 1500 || len < 0)
		LOG_AND_RETURN(0, "assemble_packet: len %d not valid for pid %d", len,
				pid);

	item_key = TABLES_ITEM + (ad->id << 16) + pid;

	if (len > 183)
	{
		setItem(item_key, b + 5, 183, 0);
		setItemTimeout(item_key, ASSEMBLE_TIMEOUT);
		return 0;

	}
	else if (len > 0)
	{
		b = b + 5;
	}
	else // pmt_len == 0 - next part from the pmt
	{
		setItem(item_key, b + 4, 184, -1);
		setItemTimeout(item_key, ASSEMBLE_TIMEOUT);
		b = getItem(item_key);
		len = ((b[1] & 0xF) << 8) + b[2];
		if (getItemLen(item_key) < len)
			return 0;
	}
	*b1 = b;
	if (check_crc) // check the crc for PAT and PMT
	{
		if (len < 4 || len > 1500)
			LOG_AND_RETURN(0, "assemble_packet: len %d not valid for pid %d",
					len, pid);
		crc = crc32(b, len - 1);
		copy32r(current_crc, b, len - 1)
		if (crc != current_crc)
			LOG_AND_RETURN(0, "pid %d (%04X) CRC failed %08X != %08X", pid, pid,
					crc, current_crc);
	}
	return len;
}

void free_assemble_packet(int pid, adapter *ad)
{
	delItem(TABLES_ITEM + (ad->id << 16) + pid);
}

uint32_t crc32(const uint8_t *data, int datalen)
{
	uint32_t crc = 0xFFFFFFFF;
	if (datalen < 0)
		return crc;
	while (datalen--)
		crc = (crc << 8) ^ crc_tab[((crc >> 24) ^ *data++) & 0xff];

	return crc;
}

void mark_pid_null(uint8_t *b)
{
	b[1] |= 0x1F;
	b[2] |= 0xFF;
}

// clean psi from CA info
void clean_psi(uint8_t *b, adapter *ad)
{
	int pid = PID_FROM_TS(b);
	int pmt_len;
	int64_t clean_key = TABLES_ITEM + ((1 + ad->id) << 24) + pid;
	int64_t item_key = TABLES_ITEM + (ad->id << 16) + pid;
	uint8_t *clean, *pmt;
	SPid *p;
	int pi_len, i, j, es_len, desc_len;
	int8_t *cc, _cc;

	p = find_pid(ad->id, pid);
	if (!p || p->sid[0] == -1) // no need to fix this PMT as it not requested by any stream
		return;

	if (!get_key(p->key)) // no key associated with PMT - most likely the channel is clear
		return;

	if (!(p->type & CLEAN_PMT))
	{
//		mark_pid_null(b);
		return;
	}
	clean = getItem(clean_key);
	if (!(pmt = getItem(item_key)))
	{
		pmt_len = ((b[6] & 0xF) << 8) + b[7];
		if ((b[1] & 0x40) && (pmt_len < 183))
			pmt = b + 5;
	}

	if (!clean && !pmt)
	{
		mark_pid_null(b);
		return;
	}

	if (!clean && pmt) // populate clean psi
	{
		uint8_t *n, *o;
		int nlen = 0;
		uint32_t crc;
		setItem(clean_key, pmt, 1, 0);
		if (getItemSize(clean_key) < 1500)
			setItemSize(clean_key, 1500);
		clean = getItem(clean_key);
		if (!clean)
		{
			mark_pid_null(b);
			return;
		}
		setItemTimeout(clean_key, ASSEMBLE_TIMEOUT);
		memset(clean, -1, getItemSize(clean_key));
		setItem(clean_key, pmt, 12, 0);
		pi_len = ((pmt[10] & 0xF) << 8) + pmt[11];
		pmt_len = ((pmt[1] & 0xF) << 8) + pmt[2];
		LOG("Cleaning PMT for pid %d, pmt_len %d, pi_len %d, pmt %p", pid,
				pmt_len, pi_len, pmt);
		n = clean;
		o = pmt + pi_len + 12;
		nlen = 12;
		n[10] &= 0xF0;   // pi_len => 0
		n[11] &= 0x0;

		for (i = 0; i < pmt_len - pi_len - 17; i += (es_len) + 5) // reading streams
		{
			uint8_t *t = o + i;
			int init_len = nlen + 5;
			es_len = (o[i + 3] & 0xF) * 256 + o[i + 4];
			LOGL(4, "es: copy 5 bytes from %d -> %d : %02X %02X %02X %02X %02X",
					i, nlen, t[0], t[1], t[2], t[3], t[4]);
			memcpy(n + nlen, o + i, 5);
			nlen += 5;
			for (j = 0; j < es_len; j += desc_len) // reading program info
			{
				desc_len = o[i + 5 + j + 1] + 2;
				if (o[i + 5 + j] != 9)
				{
					t = o + i + 5 + j;
					LOGL(4, "desc copy %d bytes from %d -> %d : %02X %02X %02X",
							desc_len, i + 5 + j, nlen, t[0], t[1], t[2]);
					memcpy(n + nlen, o + i + 5 + j, desc_len);
					nlen += desc_len;
				}
			}
			int nes_len = nlen - init_len;
			LOGL(4, "clean_psi: setting the new es_len %d at position %d",
					nes_len, init_len - 2);
			n[init_len - 2] = (n[init_len - 2] & 0xF0) | ((nes_len >> 8) & 0xF);
			n[init_len - 1] = (nes_len) & 0xFF;
		}
		nlen += 4 - 3;
		LOGL(4, "new len is %d, old len is %d", nlen, pmt_len);
		n[1] &= 0xF0;
		n[1] |= (nlen >> 8);
		n[2] = nlen & 0xFF;
		n[5] ^= 0x3F; // change version

		crc = crc32(n, nlen - 1);
		copy32(n, nlen - 1, crc);
		copy16(n, 1498, nlen + 1); // set the position at the end of the pmt
		_cc = b[3] & 0xF; // continuity counter
		_cc = (_cc - 1) & 0xF;
		cc = (uint8_t *) n + 1497;
		*cc = _cc;
	}

	if (clean)
	{
		uint16_t *pos = (uint16_t *) clean + 1498;
		pmt_len = ((clean[1] & 0xF) << 8) + clean[2];
		cc = (uint8_t *) clean + 1497;
		if (b[1] & 0x40)
			*pos = 0;
		if (*pos > pmt_len)
		{
			mark_pid_null(b);
			return;
		}
		if (*pos == 0)
		{
			memcpy(b + 5, clean, 183);
			*pos = 183;
		}
		else
		{
			memcpy(b + 4, clean + *pos, 184);
			*pos += 184;
		}
		*cc = (*cc + 1) & 0xF;
		b[3] = (b[3] & 0xF0) | *cc;
		return;
	}
	mark_pid_null(b);

}

int process_stream(adapter *ad, int rlen)
{
	SPid *p;
	int i, isPMT, pid;
	uint8_t *b;
	int dvbapi_on = 0;

	int64_t pid_key = TABLES_ITEM + ((1 + ad->id) << 24) + 0;
	int16_t *pids = (int16_t *) getItem(pid_key);

	for (i = 0; i < rlen; i += 188)
	{
		b = ad->buf + i;
		pid = PID_FROM_TS(b);
		isPMT = 0;
		if (pid == 0)
		{
			dvbapi_on = 0;
#ifndef DISABLE_DVBCSA
			dvbapi_on = dvbapi_enabled();
#endif			
			if (dvbapi_on || ad->ca_device)
				process_pat(b, ad);
			continue;
		}

		if (pids && pids[pid] < 0)
			isPMT = 1;
		/*		else if(!pids)
		 {
		 p = find_pid(ad->id, pid);
		 if(p && (p->type & TYPE_PMT))
		 isPMT = 1;
		 }
		 */
		if (isPMT)
		{
			int process = 0;
#ifndef DISABLE_DVBCA
			dvbca_process_pmt(b, ad);
			process = 1;
#endif				
#ifndef DISABLE_DVBCSA
			dvbapi_process_pmt(b, ad);
			process = 1;
#endif				
			if (process && opts.clean_psi)
				clean_psi(b, ad);
		}
	}
#ifndef DISABLE_DVBCSA
	decrypt_stream(ad, rlen);
#endif	
	return 0;
}

