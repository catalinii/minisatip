#ifndef __GXAV_DEMUX_PROPERTYTYPES_H__
#define __GXAV_DEMUX_PROPERTYTYPES_H__

/* define the pin_id(0~63) that the demux link fifo */
//pin_id: 0~31     for link av/pts fifo
//pin_id: 32~37    for other fifo of the demux output
#define PIN_ID_SDRAM_OUTPUT0   32
#define PIN_ID_SDRAM_OUTPUT1   33
#define PIN_ID_SDRAM_OUTPUT2   34
#define PIN_ID_SDRAM_OUTPUT3   35
#define PIN_ID_SDRAM_OUTPUT4   36
#define PIN_ID_SDRAM_OUTPUT5   37
#define PIN_ID_SDRAM_OUTPUT6   38
#define PIN_ID_SDRAM_OUTPUT7   39
#define PIN_ID_SDRAM_OUTPUT8   40
#define PIN_ID_SDRAM_OUTPUT9   41
#define PIN_ID_SDRAM_OUTPUT10  42
#define PIN_ID_SDRAM_OUTPUT11  43
#define PIN_ID_SDRAM_OUTPUT12  44
#define PIN_ID_SDRAM_OUTPUT13  45
#define PIN_ID_SDRAM_OUTPUT14  46
#define PIN_ID_SDRAM_OUTPUT15  47

//#define PIN_ID_SDRAM_OUTPUT0  64
#define PIN_ID_SDRAM_OUTPUT    64
//pin_id: 38~43    for other fifo of the demux input
#define PIN_ID_SDRAM_INPUT     128

#define MAX_MULTISLOT_SLOT     (5)

enum dmx_stream_mode {
	DEMUX_PARALLEL,
	DEMUX_SERIAL
};

enum dmx_ts_select {
	FRONTEND,
	OTHER
};

enum dmx__input {
	DEMUX_TS1,
	DEMUX_TS2,
	DEMUX_TS3,
	DEMUX_SDRAM
};

enum sync_lock_flag {
	TS_SYNC_UNLOCKED,
	TS_SYNC_LOCKED
};

enum dmx_slot_type {
	DEMUX_SLOT_PSI,
	DEMUX_SLOT_PES,
	DEMUX_SLOT_PES_AUDIO,
	DEMUX_SLOT_PES_VIDEO,
	DEMUX_SLOT_TS,
	DEMUX_SLOT_MUXTS,
	DEMUX_SLOT_AUDIO,
	DEMUX_SLOT_SPDIF,
	DEMUX_SLOT_AUDIO_SPDIF,
	DEMUX_SLOT_VIDEO
};

enum mtc_arith_type {
	GXMTCCA_DES_ARITH,
	GXMTCCA_3DES_ARITH,
	GXMTCCA_AES128_ARITH,
	GXMTCCA_AES192_ARITH,
	GXMTCCA_AES256_ARITH,
	GXMTCCA_MULTI2_ARITH
};

enum mtc_arith_mode {
	GXMTCCA_ECB_MODE,
	GXMTCCA_CBC_MODE,
	GXMTCCA_CFB_MODE,
	GXMTCCA_OFB_MODE,
	GXMTCCA_CTR_MODE
};

struct dmx_filter_key {
	unsigned char value;
	unsigned char mask;
};

typedef struct {
	unsigned char        sync_lock_gate;
	unsigned char        sync_loss_gate;
	unsigned char        time_gate;
	unsigned char        byt_cnt_err_gate;

	enum dmx_stream_mode stream_mode;
	enum dmx_ts_select   ts_select;
	enum dmx__input       source;
} GxDemuxProperty_ConfigDemux;

typedef struct {
	int                slot_id;
	enum dmx_slot_type type;                    // av channel or common channel
	unsigned short     pid;
	unsigned int       ts_out_pin;

	unsigned int       flags;
#define DMX_CRC_DISABLE           (1 << 0)
#define DMX_REPEAT_MODE           (1 << 1)          // common channel
#define DMX_PTS_INSERT            (1 << 2)          // av channel
#define DMX_PTS_TO_SDRAM          (1 << 3)          // av channel
#define DMX_TSOUT_EN              (1 << 4)
#define DMX_AVOUT_EN              (1 << 5)
#define DMX_DES_EN                (1 << 6)
#define DMX_ERR_DISCARD_EN        (1 << 7)
#define DMX_YUV_FRAME             (1 << 8)
#define DMX_YUV_SYNC              (1 << 9)
#define DMX_DUP_DISCARD_EN        (1 << 10)
} GxDemuxProperty_Slot;

typedef struct {
#define DMX_MSLOT_TY   0
#define DMX_MSLOT_TUV  1
#define DMX_MSLOT_BY   2
#define DMX_MSLOT_BUV  3
#define DMX_MSLOT_SYNC 4
	unsigned int slot_id;
	unsigned int slot_cnt;
	GxDemuxProperty_Slot slot[MAX_MULTISLOT_SLOT];
} GxDemuxProperty_MultiSlot;

typedef struct {
	int          slot_id;
	void*        buffer;
	unsigned int max_size;
	unsigned int read_size;
} GxDemuxProperty_MultiSlotRead;

typedef struct {
	int          slot_id;
	void*        buffer;
	unsigned int max_size;
	unsigned int write_size;
} GxDemuxProperty_MultiSlotWrite;

typedef struct {
	int                   slot_id;
	int                   filter_id;
	unsigned int          sw_buffer_size;
	unsigned int          hw_buffer_size;
	unsigned int          almost_full_gate;

	struct dmx_filter_key key[18];
	int                   depth;

	unsigned int          flags;
#define DMX_CRC_IRQ               1
#define DMX_EQ                    2
#define DMX_SW_FILTER             128
} GxDemuxProperty_Filter;

typedef struct {
	int          filter_id;
	void*        buffer;
	unsigned int max_size;
	unsigned int read_size;
} GxDemuxProperty_FilterRead;

typedef struct {
	int          ca_id;
	int          slot_id;

	unsigned int even_key_high;
	unsigned int even_key_low;
	unsigned int odd_key_high;
	unsigned int odd_key_low;
	unsigned int flags;
#define DMX_CA_KEY_EVEN           1
#define DMX_CA_KEY_ODD            2
#define DMX_CA_DES_MODE           128
} GxDemuxProperty_CA;

typedef struct {
	unsigned short pcr_pid;               // pcr recover
} GxDemuxProperty_Pcr;

typedef struct {
	enum sync_lock_flag ts_lock;
} GxDemuxProperty_TSLockQuery;

typedef struct {
	int filter_id;
} GxDemuxProperty_FilterFifoReset;

typedef struct {
	long long state;
} GxDemuxProperty_FilterFifoQuery;

typedef struct {
	unsigned short pid;
	int slot_id;
} GxDemuxProperty_SlotQueryByPid;

typedef struct {
	unsigned int stc_value;
} GxDemuxProperty_ReadStc;

typedef struct {
	int filter_id;
	unsigned int free_size;
} GxDemuxProperty_FilterFifoFreeSize;

typedef struct {
	int                 ca_id;
	int                 slot_id;

	unsigned char*      eck;
	unsigned int        eck_len;
	unsigned char*      ecw_even;
	unsigned int        ecw_even_len;
	unsigned char*      ecw_odd;
	unsigned int        ecw_odd_len;
	unsigned int        flags;

	enum mtc_arith_type arith_type;
	enum mtc_arith_mode arith_mode;

#define MTC_KEY_EVEN              1
#define MTC_KEY_ODD               2
} GxDemuxProperty_MTCCA;

#endif
