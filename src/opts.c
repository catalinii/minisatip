/*
 * Copyright (C) 2014-2023 Catalin Toda <catalinii@yahoo.com> et al
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

#include "opts.h"
#include "utils.h"
#include "utils/logging/logging.h"
#include <string.h>

void parse_dvbapi_opt(char *optarg, struct_opts_t *optz) {
    char buf[100];
    memset(buf, 0, sizeof(buf));
    safe_strncpy(buf, optarg);
    if (buf[0] == '~') {
        optz->pids_all_no_dec = 1;
        memmove(buf, buf + 1, strlen(buf));
    }
    char *sep2 = strchr(buf, ',');
    if (sep2 != NULL) {
        *sep2 = 0;
        optz->dvbapi_offset = map_int(sep2 + 1, NULL);
        memmove(buf, buf, strlen(buf) - 1 - strlen(sep2));
    }
    char *sep1 = strchr(buf, ':');
    if (sep1 != NULL) {
        // Hostname and port
        *sep1 = 0;
        safe_strncpy(optz->dvbapi_host, buf);
        optz->dvbapi_port = map_int(sep1 + 1, NULL);
    } else {
        // Socket file
        safe_strncpy(optz->dvbapi_host, buf);
        optz->dvbapi_port = 9000;
    }

    if (optz->dvbapi_host[0] == '/') {
        LOG("Using DVBAPI socket at %s", optz->dvbapi_host);
    } else {
        LOG("Using DVBAPI server at %s:%d, offset %d", optz->dvbapi_host,
            optz->dvbapi_port, optz->dvbapi_offset);
    }

    if (optz->pids_all_no_dec) {
        LOG("Not filtering out encrypted packets from pids=all streams");
    }
}
