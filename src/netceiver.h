#ifndef NETCEIVERCLIENT_H
#define NETCEIVERCLIENT_H

#include "adapter.h"

#define API_SOCK
#ifndef DISABLE_LINUXDVB
#undef AOT_CA_PMT
#include "headers.h"
#else
#include "netceiver_mcli_defs.h"
#endif

// undefine non-standard Netceiver FEC numbering
#undef FEC_1_4
#undef FEC_1_3
#undef FEC_2_5
#undef FEC_3_5
#undef FEC_9_10


void find_netcv_adapter(adapter **a);

typedef struct struct_netceiver
{
	recv_info_t *ncv_rec;		// pointer to libmcli receiver instance
	int err;			// error during receiver instance creation
	int pwfd;			// file descriptor to writeable end of pipe for TS data
	uint16_t npid[MAX_PIDS];	// active pids
	int lp;				// number of active pids
	char want_tune, want_commit;	// tuining & and PID handling state machine
} SNetceiver;


#endif
