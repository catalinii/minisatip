#ifndef _GXAV_COMMON_H_
#define _GXAV_COMMON_H_

#ifndef NULL
#define NULL 0
#endif

/*
 * GX_MIN()/GX_MAX() macros that also do strict
 * type-checking. See the pointer comparison
 * [(void) (&_min1 == &_min2);]that do this.
 */
#define GX_MIN(x, y) ({                   \
		typeof(x) _min1 = (x);            \
		typeof(y) _min2 = (y);            \
		(void) (&_min1 == &_min2);        \
		_min1 < _min2 ? _min1 : _min2; })

#define GX_MAX(x, y) ({                   \
		typeof(x) _max1 = (x);            \
		typeof(y) _max2 = (y);            \
		(void) (&_max1 == &_max2);        \
		_max1 > _max2 ? _max1 : _max2; })

#define GX_ABS(x) ({                      \
		int __x = (x);                    \
		(__x < 0) ? -__x : __x;           \
		})

#define GX_ARRAY_SIZE(arr)  (sizeof(arr) / sizeof((arr)[0]))

typedef enum  gxav_chip_id {
	GXAV_ID_GX3211 = 0x3211,
	GXAV_ID_GX3201 = 0x3201,
	GXAV_ID_GX3113C = 0x6131,
	GXAV_ID_GX6605S = 0x6605,
} GxAvChipId;

typedef enum {
	GXAV_SDC_READ,
	GXAV_SDC_WRITE
} GxAvSdcOPMode;

typedef enum gxav_module_type {
	GXAV_MOD_SDC           = 0x0000,
	GXAV_MOD_DEMUX         = 0x0001,
	GXAV_MOD_VIDEO_DECODE  = 0x0002,
	GXAV_MOD_AUDIO_DECODE  = 0x0003,
	GXAV_MOD_VPU           = 0x0004,
	GXAV_MOD_AUDIO_OUT     = 0x0005,
	GXAV_MOD_VIDEO_OUT     = 0x0006,
	GXAV_MOD_AUDIO_RECEVER = 0x0007,
	GXAV_MOD_JPEG_DECODER  = 0x0008,
	GXAV_MOD_FRONTEND      = 0x0009,
	GXAV_MOD_STC           = 0x000a,
	GXAV_MOD_DESCRAMBLER   = 0x000b,
	GXAV_MOD_MTC           = 0x000c,
	GXAV_MOD_ICAM          = 0x000d,
	GXAV_MOD_IEMM          = 0x000e
} GxAvModuleType;

typedef enum  gxav_module_state {
	GXAV_MOD_RUNNING,
	GXAV_MOD_STOP,
	GXAV_MOD_PAUSE,
} GxAvModuleState;

typedef struct gxav_point {
	unsigned int x;
	unsigned int y;
} GxAvPoint;

typedef struct gxav_rect {
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
} GxAvRect;

typedef enum  gxav_direction {
	GXAV_PIN_INPUT  = 0x1,
	GXAV_PIN_OUTPUT = 0x2
} GxAvDirection;

typedef enum  gxav_pin_id {
	GXAV_PIN_ESA	= 0x1,
	GXAV_PIN_PCM	= 0x2,
	GXAV_PIN_AC3	= 0x3,
	GXAV_PIN_EAC3	= 0x4,
	GXAV_PIN_ADESA	= 0x5,
	GXAV_PIN_ADPCM	= 0x6,
	GXAV_PIN_DTS	= 0x7,
} GxAvPinId;

typedef struct gxav_channel_info {
	unsigned int pin_id;
	void *channel;
	GxAvDirection dir;
} GxAvChanInfo;

typedef enum  gxav_channel_flag {
	N  = 0x0,
	W  = 0x1,
	R  = 0x2,
	RW = 0x3
} GxAvChannelFlag;

typedef enum gxav_channel_type {
	GXAV_NO_PTS_FIFO   = (1 << 0),
	GXAV_PTS_FIFO      = (1 << 1),
	GXAV_WPROTECT_FIFO = (1 << 2),
	GXAV_RPROTECT_FIFO = (1 << 3),
} GxAvChannelType;

typedef struct gxav_gate_info {
	unsigned int almost_empty;
	unsigned int almost_full;
	unsigned int cache_max;
} GxAvGateInfo;

typedef struct gxav_sync_params {
	unsigned int pcr_err_gate;
	unsigned int pcr_err_time;
	unsigned int apts_err_gate;
	unsigned int apts_err_time;
	unsigned int audio_low_tolerance;
	unsigned int audio_high_tolerance;
	int stc_offset;
} GxAvSyncParams;

typedef struct gxav_frame_stat {
	unsigned long long play_frame_cnt;
	unsigned long long error_frame_cnt;
	unsigned long long filter_frame_cnt;
	unsigned long long lose_sync_cnt;
	struct {
		unsigned int enable;
		unsigned int code;
	} AFD;
} GxAvFrameStat;

typedef enum {
	PCM_TYPE     = (1<<0),
	AC3_TYPE     = (1<<1),
	EAC3_TYPE    = (1<<2),
	ADPCM_TYPE   = (1<<3),
	DTS_TYPE     = (1<<4),
} GxAvAudioDataType;

typedef enum {
	AS_NONE = 0,
	AS_ACCESS_RESTRICT = (1<<0),
	AS_ACCESS_ALIGN    = (1<<1),
} GxAvAdvancedSecurity;

typedef struct {
	unsigned int addr;
	unsigned int size;
} GxAvASDataBlock;

typedef enum {
	EDID_AUDIO_LINE_PCM = (1<<0),
	EDID_AUDIO_AC3      = (1<<1),
	EDID_AUDIO_EAC3     = (1<<2),
} GxAvHdmiEdid;

#endif
