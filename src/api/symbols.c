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
#include "symbols.h"

#include "stdlib.h"

// Define all symbols
_symbols *sym[] = {adapters_sym, stream_sym, minisatip_sym,
#ifndef DISABLE_DVBCA
                   ca_sym,
#endif
#ifndef DISABLE_DVBAPI
                   dvbapi_sym,
#endif
#ifndef DISABLE_SATIPCLIENT
                   satipc_sym,
#endif
#ifndef DISABLE_PMT
                   pmt_sym,
#endif
#ifdef AXE
                   axe_sym,
#endif
                   NULL};
