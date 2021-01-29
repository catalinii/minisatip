/*
 * frontend.h
 *
 * Copyright (C) 2000 Marcus Metzler <marcus@convergence.de>
 *            Ralph  Metzler <ralph@convergence.de>
 *            Holger Waechtler <holger@convergence.de>
 *            Andre Draszik <ad@convergence.de>
 *            for convergence integrated media GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
#ifndef _DVBFRONTEND_H_
#define _DVBFRONTEND_H_

#include <asm/types.h>

typedef enum fe_type {
	FE_QPSK,
	FE_QAM,
	FE_OFDM,
	FE_ATSC,
	FE_DTMB,
} fe_type_t;

typedef enum fe_caps {
	FE_IS_STUPID                  = 0,
	FE_CAN_INVERSION_AUTO         = 0x1,
	FE_CAN_FEC_1_2                = 0x2,
	FE_CAN_FEC_2_3                = 0x4,
	FE_CAN_FEC_3_4                = 0x8,
	FE_CAN_FEC_4_5                = 0x10,
	FE_CAN_FEC_5_6                = 0x20,
	FE_CAN_FEC_6_7                = 0x40,
	FE_CAN_FEC_7_8                = 0x80,
	FE_CAN_FEC_8_9                = 0x100,
	FE_CAN_FEC_AUTO               = 0x200,
	FE_CAN_QPSK                   = 0x400,
	FE_CAN_QAM_16                 = 0x800,
	FE_CAN_QAM_32                 = 0x1000,
	FE_CAN_QAM_64                 = 0x2000,
	FE_CAN_QAM_128                = 0x4000,
	FE_CAN_QAM_256                = 0x8000,
	FE_CAN_QAM_AUTO               = 0x10000,
	FE_CAN_TRANSMISSION_MODE_AUTO = 0x20000,
	FE_CAN_BANDWIDTH_AUTO         = 0x40000,
	FE_CAN_GUARD_INTERVAL_AUTO    = 0x80000,
	FE_CAN_HIERARCHY_AUTO         = 0x100000,
	FE_CAN_8VSB                   = 0x200000,
	FE_CAN_16VSB                  = 0x400000,
	FE_CAN_EXTENDED               = 0x1000000,  // 1132 can not support auto-n t2mi multistream features
	FE_CAN_T2MI                   = 0x2000000,  // 1133 support hard-t2mi
	FE_NEEDS_PREDEMUX             = 0x4000000,  // 3211
	FE_NEEDS_BENDING              = 0x20000000, // not supported anymore, don't use (frontend requires frequency bending)
	FE_CAN_RECOVER                = 0x40000000, // frontend can recover from a cable unplug automatically
	FE_CAN_MUTE_TS                = 0x80000000  // frontend can stop spurious TS data output
} fe_caps_t;

struct dvb_frontend_info {
	char       name[128];
	fe_type_t  type;
	__u32      frequency_min;
	__u32      frequency_max;
	__u32      frequency_stepsize;
	__u32      frequency_tolerance;
	__u32      symbol_rate_min;
	__u32      symbol_rate_max;
	__u32      symbol_rate_tolerance;    /* ppm */
	__u32      notifier_delay;        /* DEPRECATED */
	fe_caps_t  caps;
};

/**
 *  Check out the DiSEqC bus spec available on http://www.eutelsat.org/ for
 *  the meaning of this struct...
 */
struct dvb_diseqc_master_cmd {
	__u8 msg [6];    /*  { framing, address, command, data [3] } */
	__u8 msg_len;    /*  valid values are 3...6  */
};

struct dvb_diseqc_slave_reply {
	__u8 msg [4];    /*  { framing, data [3] } */
	__u8 msg_len;    /*  valid values are 0...4, 0 means no msg  */
	int  timeout;    /*  return from ioctl after timeout ms with */
};            /*  errorcode when no message was received  */

typedef enum fe_sec_voltage {
	SEC_VOLTAGE_13,
	SEC_VOLTAGE_18,
	SEC_VOLTAGE_OFF
} fe_sec_voltage_t;

typedef enum fe_sec_tone_mode {
	SEC_TONE_ON,
	SEC_TONE_OFF
} fe_sec_tone_mode_t;

typedef enum fe_sec_mini_cmd {
	SEC_MINI_A,
	SEC_MINI_B
} fe_sec_mini_cmd_t;

typedef enum fe_status {
	FE_HAS_SIGNAL  = 0x01,   /*  found something above the noise level */
	FE_HAS_CARRIER = 0x02,   /*  found a DVB signal  */
	FE_HAS_VITERBI = 0x04,   /*  FEC is stable  */
	FE_HAS_SYNC    = 0x08,   /*  found sync bytes  */
	FE_HAS_LOCK    = 0x10,   /*  everything's working... */
	FE_TIMEDOUT    = 0x20,   /*  no lock within the last ~2 seconds */
	FE_REINIT      = 0x40,    /*  frontend was reinitialized,  */
	FE_AUTO_SCAN   = 0x3f
} fe_status_t;                   /*  application is recommended to reset */
/*  DiSEqC, tone and parameters */

typedef enum fe_spectral_inversion {
	INVERSION_OFF,
	INVERSION_ON,
	INVERSION_AUTO
} fe_spectral_inversion_t;

typedef enum fe_code_rate {
	FEC_NONE = 0,
	FEC_1_2,
	FEC_2_3,
	FEC_3_4,
	FEC_4_5,
	FEC_5_6,
	FEC_6_7,
	FEC_7_8,
	FEC_8_9,
	FEC_AUTO,
	/* below for DVB-S2 */
	FEC_S2_QPSK_1_2,
	FEC_S2_QPSK_2_3,
	FEC_S2_QPSK_3_4,
	FEC_S2_QPSK_5_6,
	FEC_S2_QPSK_7_8,
	FEC_S2_QPSK_8_9,
	FEC_S2_QPSK_3_5,
	FEC_S2_QPSK_4_5,
	FEC_S2_QPSK_9_10,
	FEC_S2_8PSK_1_2,
	FEC_S2_8PSK_2_3,
	FEC_S2_8PSK_3_4,
	FEC_S2_8PSK_5_6,
	FEC_S2_8PSK_7_8,
	FEC_S2_8PSK_8_9,
	FEC_S2_8PSK_3_5,
	FEC_S2_8PSK_4_5,
	FEC_S2_8PSK_9_10
} fe_code_rate_t;

typedef enum fe_modulation {
	QPSK,
	QAM_16,
	QAM_32,
	QAM_64,
	QAM_128,
	QAM_256,
	QAM_AUTO,
	VSB_8,
	VSB_16,
	PSK_8,
	APSK_16,
	APSK_32,
	DQPSK,

	CRXX = 14,//dvbs
	CR12,
	CR23,
	CR34,
	CR45,
	CR56,
	CR67,
	CR78,

	CRXXX = 22,//dvbs2
	QPSK14,
	QPSK13,
	QPSK25,
	QPSK12,
	QPSK35,
	QPSK23,
	QPSK34,
	QPSK45,
	QPSK56,
	QPSK89,
	QPSK910,
	_8PSK35,
	_8PSK23,
	_8PSK34,
	_8PSK56,
	_8PSK89,
	_8PSK910
} fe_modulation_t;

typedef enum fe_transmit_mode {
	TRANSMISSION_MODE_2K,
	TRANSMISSION_MODE_8K,
	TRANSMISSION_MODE_AUTO
} fe_transmit_mode_t;

typedef enum fe_bandwidth {
	BANDWIDTH_8_MHZ,
	BANDWIDTH_7_MHZ,
	BANDWIDTH_6_MHZ,
	BANDWIDTH_AUTO
} fe_bandwidth_t;

typedef enum fe_guard_interval {
	GUARD_INTERVAL_1_32,
	GUARD_INTERVAL_1_16,
	GUARD_INTERVAL_1_8,
	GUARD_INTERVAL_1_4,
	GUARD_INTERVAL_AUTO
} fe_guard_interval_t;

typedef enum fe_hierarchy {
	HIERARCHY_NONE,
	HIERARCHY_1,
	HIERARCHY_2,
	HIERARCHY_4,
	HIERARCHY_AUTO
} fe_hierarchy_t;

struct dvb_qpsk_parameters {
	__u32        symbol_rate;  /* symbol rate in Symbols per second */
	fe_code_rate_t    fec_inner;    /* forward error correction (see above) */
	__u32         pls_num;
};

struct dvb_qam_parameters {
	__u32        symbol_rate; /* symbol rate in Symbols per second */
	fe_code_rate_t    fec_inner;   /* forward error correction (see above) */
	fe_modulation_t    modulation;  /* modulation type (see above) */
};

struct dvb_vsb_parameters {
	fe_modulation_t    modulation;  /* modulation type (see above) */
};

struct dvb_ofdm_parameters {
	fe_bandwidth_t      bandwidth;
	fe_code_rate_t      code_rate_HP;  /* high priority stream code rate */
	fe_code_rate_t      code_rate_LP;  /* low priority stream code rate */
	fe_modulation_t     constellation; /* modulation type (see above) */
	fe_transmit_mode_t  transmission_mode;
	fe_guard_interval_t guard_interval;
	fe_hierarchy_t      hierarchy_information;
};

struct TsIdPlsCode {
	unsigned char ts_id;
	unsigned char pls_code;
	unsigned char pls_change;
	unsigned char pls_status;
};

struct TsIdPlsInfo {
	unsigned int  code_num;
	struct TsIdPlsCode code[257];
};
struct dvb_frontend_parameters {
	__u32 frequency;     /* (absolute) frequency in Hz for QAM/OFDM/ATSC */
	/* intermediate frequency in kHz for QPSK */
	fe_spectral_inversion_t inversion;
	union {
		struct dvb_qpsk_parameters qpsk;
		struct dvb_qam_parameters  qam;
		struct dvb_ofdm_parameters ofdm;
		struct dvb_vsb_parameters vsb;
	} u;
	unsigned int unicable_init_fre; //unicable centre freq 按本振计算 (9750/10600...)
	unsigned short tone_if_freq;// user center freq (1210..)
	unsigned char lnb_index;
	unsigned char scrpf;// //unicable ifeq channel (0~7)
	struct TsIdPlsCode ts_code;
};

struct dvb_frontend_event {
	fe_status_t status;
	struct dvb_frontend_parameters parameters;
};

struct fe_tp_parameters {
	__u32 frequency;
	__u32 symbol_rate;
	__u32 fec, inversion, system, modulation, rolloff, pilot;
};

struct fe_blindscan_parameters {
	__u32 fcenter;
	__u32 lpf_bw_window;
	__u32 tp_num;
	int lsymbol_rate;
};

struct tp_node {
	int id;
	int freq;
	int srate;
	struct dvb_frontend_parameters parameter;
};

struct ts_data {
	int tp_id;
	int size;
	void *buffer;
};

struct tp_info {
	__u32 type;//0:DVBS 1:DIRECTV 2:DVBS2 3:DVBS2
	__u32 frequency;
	__u32 symbol_rate;
	__u32 modulation;
};

// For T2MI
#define MAX_PID_ID_NUM		8
#define MAX_PLP_ID_NUM		16

typedef struct {
	unsigned char plp_id;
	unsigned char plp_type;
	unsigned char plp_group_id;
	unsigned char plp_payload_type;
	unsigned char plp_mode;
} GxT2mi_PlpInfo;

typedef struct {
	int pid;
	int plp_num;
	GxT2mi_PlpInfo plp_info[MAX_PLP_ID_NUM];
} GxT2mi_PidInfo;

typedef struct {
	int pid_num;
	GxT2mi_PidInfo pid_info[MAX_PID_ID_NUM];
} GxT2mi_StreamInfo;

typedef struct {
#define STREAM_UNKNOW		0
#define STREAM_TS		1
#define STREAM_T2MI		2
	char stream_type;
	unsigned int stream_size;
} GxT2mi_StreamType;

typedef struct {
	GxT2mi_StreamInfo stream_info;
	GxT2mi_StreamType stream_type;
	int demux_id;
	void * buffer;
	unsigned int buffer_size;
} GxT2mi_ProbeInfo;

/**
 * When set, this flag will disable any zigzagging or other "normal" tuning
 * behaviour. Additionally, there will be no automatic monitoring of the lock
 * status, and hence no frontend events will be generated. If a frontend device
 * is closed, this flag will be automatically turned off when the device is
 * reopened read-write.
 */
#define FE_TUNE_MODE_ONESHOT           0x01

#define FE_GET_INFO                    _IOR('o', 61, struct dvb_frontend_info)

#define FE_DISEQC_RESET_OVERLOAD       _IO('o', 62)
#define FE_DISEQC_SEND_MASTER_CMD      _IOW('o', 63, struct dvb_diseqc_master_cmd)
#define FE_DISEQC_RECV_SLAVE_REPLY     _IOR('o', 64, struct dvb_diseqc_slave_reply)
#define FE_DISEQC_SEND_BURST           _IO('o', 65)  /* fe_sec_mini_cmd_t */

#define FE_SET_TONE                    _IO('o', 66)  /* fe_sec_tone_mode_t */
#define FE_SET_VOLTAGE                 _IO('o', 67)  /* fe_sec_voltage_t */
#define FE_ENABLE_HIGH_LNB_VOLTAGE     _IO('o', 68)  /* int */

#define FE_READ_STATUS                 _IOR('o', 69, fe_status_t)
#define FE_READ_BER                    _IOR('o', 70, __u32)
#define FE_READ_SIGNAL_STRENGTH        _IOR('o', 71, __u16)
#define FE_READ_SNR                    _IOR('o', 72, __u16)
#define FE_READ_UNCORRECTED_BLOCKS     _IOR('o', 73, __u32)

#define FE_SET_FRONTEND                _IOW('o', 76, struct dvb_frontend_parameters)
#define FE_GET_FRONTEND                _IOR('o', 77, struct dvb_frontend_parameters)
#define FE_SET_FRONTEND_TUNE_MODE      _IO('o', 81) /* unsigned int */
#define FE_GET_EVENT                   _IOR('o', 78, struct dvb_frontend_event)

#define FE_SOFT_ADD_TP                 _IOW('o', 90, struct tp_node)
#define FE_SOFT_DEL_TP                 _IOW('o', 91, struct tp_node)
#define FE_SOFT_RESET                  _IO('o', 92)
#define FE_SOFT_WRITE                  _IOW('o', 93, struct ts_data)

#define FE_DISHNETWORK_SEND_LEGACY_CMD _IO('o', 80) /* unsigned int */
#define FE_SET_BLINDSCAN               _IOWR('o', 82, struct fe_blindscan_parameters)
#define FE_GET_BLINDSCAN               _IO('o', 83)
/* #define FE_VERIFY_BLINDSCAN          _IOWR('o', 84, struct eDVBFrontendParametersSatellite) */
#define FE_GET_TP_INFO                 _IOR('o', 85, struct tp_info)

#define FE_SLEEP                       _IO('o', 86)
#define FE_RESET_BER                   _IO('o', 87)
#define FE_ENTER_MULTIPLS              _IO('o', 88)
#define FE_SWITCH_MULTIPLS_TS          _IO('o', 89)
#define FE_EXIT_MULTIPLS               _IO('o', 90)

#define FE_GET_PLS_TS_ID               _IO('o', 100)
#define FE_SET_PLS_TS_ID               _IO('o', 101)
#define FE_GET_PLS_NUM_ID              _IO('o', 102)
#define FE_SET_PLS_SCRAM_N_ID          _IO('o', 103)
#define FE_T2MI_PROBE                  _IOWR('o', 104, GxT2mi_ProbeInfo)
#define FE_SET_PLPID                   _IOWR('o', 105, __u8)
#define FE_SET_PLPID_AUTO              _IO('o', 106)
#define FE_SET_L3REC_MODE              _IOWR('o', 107, __u32)
#define FE_SET_T2MI_STAGE              _IOWR('o', 108, __u8)
#define FE_SET_T2MI_PID                _IOW('o', 109, __u16)
#define FE_SET_DEMOD_TS_CONTROL        _IOR('o', 110, __u32)

#define FE_GET_UNICALBE_IFF_FRE        _IO('o', 186)

#endif /*_DVBFRONTEND_H_*/
