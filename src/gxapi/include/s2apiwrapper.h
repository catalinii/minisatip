/*
 * s2apiwrapper.h:
 * Wrapper to translate DVB S2API to DVB 3.0 API calls
 *
 * See the main source file 'vdr.c' for copyright information and
 * how to reach the author.
 *
 * $Id: $
 */

#ifndef __S2API_WRAPPER_H
#define __S2API_WRAPPER_H

#include <linux/dvb/frontend.h>
#include <linux/dvb/version.h>
#include <sys/ioctl.h>

#if DVB_API_VERSION < 3
#error VDR requires Linux DVB driver API version 3!
#endif

#if !defined(DVB_S2API_RUNTIME)
#define DVB_S2API_RUNTIME 1
#endif

#if DVB_S2API_RUNTIME == 0
#undef DVB_S2API_RUNTIME
#endif

#if !defined(DVB_S2API_WRAPPER) \
    && (DVB_API_VERSION < 5 || defined(DVB_S2API_RUNTIME))
#define DVB_S2API_WRAPPER 1
#endif

#if defined(DVB_S2API_WRAPPER) && DVB_S2API_WRAPPER == 0
#undef DVB_S2API_WRAPPER
#endif

#if DVB_API_VERSION < 5

// DVB API is missing S2API structs, so we declare them.
// The following is based on frontend.h from s2api DVB driver


enum fe_s2caps {
	FE_HAS_EXTENDED_CAPS = 0x800000,   // We need more bitspace for newer APIs, indicate this.
	FE_CAN_2G_MODULATION = 0x10000000, // frontend supports "2nd generation modulation" (DVB-S2)
};

enum fe_s2code_rate {
	FEC_3_5 = FEC_AUTO + 1,
	FEC_9_10,
};

//enum fe_s2modulation {
//	PSK_8 = VSB_16 + 1,
//	APSK_16,
//	APSK_32,
//	DQPSK,
//};

enum fe_s2transmit_mode {
	TRANSMISSION_MODE_4K = TRANSMISSION_MODE_AUTO + 1,
	TRANSMISSION_MODE_1K,
	TRANSMISSION_MODE_16K,
	TRANSMISSION_MODE_32K,
};

enum fe_s2bandwidth {
	BANDWIDTH_5_MHZ = BANDWIDTH_AUTO + 1,
	BANDWIDTH_10_MHZ,
	BANDWIDTH_1_712_MHZ,
};


enum fe_s2guard_interval {
	GUARD_INTERVAL_1_128 = GUARD_INTERVAL_AUTO + 1,
	GUARD_INTERVAL_19_128,
	GUARD_INTERVAL_19_256,
};


/* S2API Commands */
#define DTV_UNDEFINED		0
#define DTV_TUNE		1
#define DTV_CLEAR		2
#define DTV_FREQUENCY		3
#define DTV_MODULATION		4
#define DTV_BANDWIDTH_HZ	5
#define DTV_INVERSION		6
#define DTV_DISEQC_MASTER	7
#define DTV_SYMBOL_RATE		8
#define DTV_INNER_FEC		9
#define DTV_VOLTAGE		10
#define DTV_TONE		11
#define DTV_PILOT		12
#define DTV_ROLLOFF		13
#define DTV_DISEQC_SLAVE_REPLY	14

/* Basic enumeration set for querying unlimited capabilities */
#define DTV_FE_CAPABILITY_COUNT	15
#define DTV_FE_CAPABILITY	16
#define DTV_DELIVERY_SYSTEM	17

/* ISDB-T and ISDB-Tsb */
#define DTV_ISDBT_PARTIAL_RECEPTION	18
#define DTV_ISDBT_SOUND_BROADCASTING	19

#define DTV_ISDBT_SB_SUBCHANNEL_ID	20
#define DTV_ISDBT_SB_SEGMENT_IDX	21
#define DTV_ISDBT_SB_SEGMENT_COUNT	22

#define DTV_ISDBT_LAYERA_FEC			23
#define DTV_ISDBT_LAYERA_MODULATION		24
#define DTV_ISDBT_LAYERA_SEGMENT_COUNT		25
#define DTV_ISDBT_LAYERA_TIME_INTERLEAVING	26

#define DTV_ISDBT_LAYERB_FEC			27
#define DTV_ISDBT_LAYERB_MODULATION		28
#define DTV_ISDBT_LAYERB_SEGMENT_COUNT		29
#define DTV_ISDBT_LAYERB_TIME_INTERLEAVING	30

#define DTV_ISDBT_LAYERC_FEC			31
#define DTV_ISDBT_LAYERC_MODULATION		32
#define DTV_ISDBT_LAYERC_SEGMENT_COUNT		33
#define DTV_ISDBT_LAYERC_TIME_INTERLEAVING	34

#define DTV_API_VERSION		35

#define DTV_CODE_RATE_HP	36
#define DTV_CODE_RATE_LP	37
#define DTV_GUARD_INTERVAL	38
#define DTV_TRANSMISSION_MODE	39
#define DTV_HIERARCHY		40

#define DTV_ISDBT_LAYER_ENABLED	41

#define DTV_ISDBS_TS_ID		42

#define DTV_DVBT2_PLP_ID	43

#define DTV_MAX_COMMAND				DTV_DVBT2_PLP_ID

typedef enum fe_pilot {
	PILOT_ON,
	PILOT_OFF,
	PILOT_AUTO,
} fe_pilot_t;

typedef enum fe_rolloff {
	ROLLOFF_35, /* Implied value in DVB-S, default for DVB-S2 */
	ROLLOFF_20,
	ROLLOFF_25,
	ROLLOFF_AUTO,
} fe_rolloff_t;

typedef enum fe_delivery_system {
	SYS_UNDEFINED,
	SYS_DVBC_ANNEX_AC,
	SYS_DVBC_ANNEX_B,
	SYS_DVBT,
	SYS_DSS,
	SYS_DVBS,
	SYS_DVBS2,
	SYS_DVBH,
	SYS_ISDBT,
	SYS_ISDBS,
	SYS_ISDBC,
	SYS_ATSC,
	SYS_ATSCMH,
	SYS_DMBTH,
	SYS_CMMB,
	SYS_DAB,
	SYS_DVBT2,
	SYS_TURBO,
} fe_delivery_system_t;

struct dtv_cmds_h {
	char	*name;		/* A display name for debugging purposes */

	__u32	cmd;		/* A unique ID */

	/* Flags */
	__u32	set:1;		/* Either a set or get property */
	__u32	buffer:1;	/* Does this property use the buffer? */
	__u32	reserved:30;	/* Align */
};

struct dtv_property {
	__u32 cmd;
	__u32 reserved[3];
	union {
		__u32 data;
		struct {
			__u8 data[32];
			__u32 len;
			__u32 reserved1[3];
			void *reserved2;
		} buffer;
	} u;
	int result;
} __attribute__ ((packed));

/* num of properties cannot exceed DTV_IOCTL_MAX_MSGS per ioctl */
#define DTV_IOCTL_MAX_MSGS 64

struct dtv_properties {
	__u32 num;
	struct dtv_property *props;
};

#define FE_SET_PROPERTY		   _IOW('o', 82, struct dtv_properties)
#define FE_GET_PROPERTY		   _IOR('o', 83, struct dtv_properties)

// End of copied section of frontend.h

#elif DVB_API_VERSION == 5 && DVB_API_VERSION_MINOR < 3

// Cherry-picking for 5.0 and 5.1

enum fe_s2transmit_mode {
	TRANSMISSION_MODE_1K = TRANSMISSION_MODE_4K + 1,
	TRANSMISSION_MODE_16K,
	TRANSMISSION_MODE_32K,
};

enum fe_s2bandwidth {
	BANDWIDTH_5_MHZ = BANDWIDTH_AUTO + 1,
	BANDWIDTH_10_MHZ,
	BANDWIDTH_1_712_MHZ,
};

enum fe_s2guard_interval {
	GUARD_INTERVAL_1_128 = GUARD_INTERVAL_AUTO + 1,
	GUARD_INTERVAL_19_128,
	GUARD_INTERVAL_19_256,
};

#define DTV_DVBT2_PLP_ID	43
#undef DTV_MAX_COMMAND
#define DTV_MAX_COMMAND				DTV_DVBT2_PLP_ID

enum fe_delivery_system_5_3 {
	SYS_DVBT2 = SYS_DAB,
	SYS_TURBO,
};

#endif // DVB_API_VERSION < 5



#ifdef DVB_S2API_WRAPPER

// Wrapper for S2API ioctl calls:
int S2API_ioctl(int d, int request, void *data);

#ifdef DVB_S2API_RUNTIME
void S2API_SetDvbApiVersion(int ApiVersion);
#endif


#else // ifdef DVB_S2API_WRAPPER

// Null wrapper for s2api ioctl calls:
inline int S2API_ioctl(int d, int request, void *data) { return ioctl(d, request, data); }

#endif // ifdef DVB_S2API_WRAPPER

#endif // ifndef __S2API_WRAPPER_H
