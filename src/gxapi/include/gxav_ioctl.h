#ifndef _GXAV_IOCTL_H_
#define _GXAV_IOCTL_H_

#include <gxav_common.h>
#include <gxav_hdcp_key.h>
#include <gxav_module_property.h>

#ifdef __cplusplus
extern "C"{
#endif

#define GXAV_IOCTL_MODULE_OPEN           0x1001
#define GXAV_IOCTL_MODULE_CLOSE          0x1002

#define GXAV_IOCTL_PROPERTY_SET          0x1101
#define GXAV_IOCTL_PROPERTY_GET          0x1102
#define GXAV_IOCTL_WRITE_CTRLINFO        0x1103

#define GXAV_IOCTL_DATA_SEND             0x1203
#define GXAV_IOCTL_DATA_RECEIVE          0x1204
#define GXAV_IOCTL_DATA_PEEK             0x1205

#define GXAV_IOCTL_EVENT_RESET           0x1301
#define GXAV_IOCTL_EVENT_SET             0x1302
#define GXAV_IOCTL_EVENT_WAIT            0x1303

#define GXAV_IOCTL_FIFO_CREATE           0x1400
#define GXAV_IOCTL_FIFO_DESTROY          0x1401
#define GXAV_IOCTL_FIFO_RESET            0x1402
#define GXAV_IOCTL_FIFO_CONFIG           0x1403
#define GXAV_IOCTL_FIFO_GET_LENGTH       0x1404
#define GXAV_IOCTL_FIFO_GET_FLAG         0x1405
#define GXAV_IOCTL_FIFO_LINK             0x1406
#define GXAV_IOCTL_FIFO_UNLINK           0x1407
#define GXAV_IOCTL_FIFO_ROLLBACK         0x1408
#define GXAV_IOCTL_FIFO_GET_FREESIZE     0x1409
#define GXAV_IOCTL_FIFO_GET_MEMORY       0x140a
#define GXAV_IOCTL_FIFO_GET_CAP          0x140b

#define GXAV_IOCTL_KMALLOC               0x1601
#define GXAV_IOCTL_KFREE                 0x1602

#define GXAV_IOCTL_CHIP_DETECT           0x1701
#define GXAV_IOCTL_CODEC_REGISTER        0x1702
#define GXAV_IOCTL_HDCP_KEY_REGISTER     0x1703

#define GXAV_IOCTL_FBINFO_GET            0x1800
#define GXAV_IOCTL_SYNC_PARAMS_SET       0x1900
#define GXAV_IOCTL_MEMHOLE_GET           0x1a00

struct gxav_module_set {
	unsigned int         channel_id;
	int                  module_id;
	GxAvModuleType       module_type;

	int                  ret;
};

struct gxav_property {
	unsigned int         module_id;
	unsigned int         prop_id;
	void*                prop_val;
	unsigned int         prop_size;

	int                  ret;
};

struct gxav_data {
	void*                fifo;
	unsigned char*       data_buffer;
	unsigned int         data_size;
	int                  timeout_us;
	int                  pts;

	int                  ret;
};

struct gxav_ctrlinfo {
	unsigned int        module_id;
	int                 timeout_us;
	void*               ctrl_info;
	unsigned int        ctrl_size;

	int                 ret;
};

struct gxav_event {
	int                  module_id;
	unsigned int         event_mask;
	int                  timeout_us;

	int                  ret;
};

struct gxav_fifo_info {
	unsigned int         info_size;
	unsigned int         data_size;
	unsigned int         free_size;
	void*                fifo;
	void*                memory;
	GxAvGateInfo*        sdc_info;
	GxAvChannelFlag      flag;
	GxAvChannelType      type;
	int                  ret;
};

struct gxav_fifo_link {
	unsigned int         module_id;
	void *               channel;
	unsigned int         pin_id;
	GxAvDirection        dir;
};

struct gxav_kmalloc {
	unsigned int         size;
	unsigned long        addr;
};

struct gxav_chip_detect {
	GxAvChipId           chip_id;
};

struct gxav_fb_info {
	unsigned int start;
	unsigned int size;
	unsigned int main_surface_size;
};

struct gxav_memhole_info {
	unsigned int start; //virt addr
	unsigned int size;
};

typedef struct gxav_module_set   GxAvIOCTL_ModuleOpen;
typedef struct gxav_module_set   GxAvIOCTL_ModuleClose;

typedef struct gxav_property     GxAvIOCTL_PropertySet;
typedef struct gxav_property     GxAvIOCTL_PropertyGet;
typedef struct gxav_ctrlinfo     GxAvIOCTL_CtrlInfo;

typedef struct gxav_data         GxAvIOCTL_DataSend;
typedef struct gxav_data         GxAvIOCTL_DataReceive;
typedef struct gxav_data         GxAvIOCTL_DataPeek;

typedef struct gxav_event        GxAvIOCTL_EventReset;
typedef struct gxav_event        GxAvIOCTL_EventSet;
typedef struct gxav_event        GxAvIOCTL_EventWait;

typedef struct gxav_fifo_info    GxAvIOCTL_FifoCreate;
typedef struct gxav_fifo_info    GxAvIOCTL_FifoDestroy;
typedef struct gxav_fifo_info    GxAvIOCTL_FifoReset;
typedef struct gxav_fifo_info    GxAvIOCTL_FifoConfig;
typedef struct gxav_fifo_info    GxAvIOCTL_FifoGetLength;
typedef struct gxav_fifo_info    GxAvIOCTL_FifoGetFlag;
typedef struct gxav_fifo_info    GxAvIOCTL_FifoGetSize;
typedef struct gxav_fifo_info    GxAvIOCTL_FifoGetFreeSize;
typedef struct gxav_fifo_info    GxAvIOCTL_FifoGetMemory;
typedef struct gxav_fifo_link    GxAvIOCTL_FifoLink;
typedef struct gxav_fifo_link    GxAvIOCTL_FifoUnlink;

typedef struct gxav_kmalloc      GxAvIOCTL_KMalloc;
typedef struct gxav_kmalloc      GxAvIOCTL_KFree;

typedef struct gxav_chip_detect       GxAvIOCTL_ChipDetect;
typedef struct gxav_codec_register    GxAvIOCTL_CodecRegister;
typedef struct gxav_hdcpkey_register  GxAvIOCTL_HdcpKeyRegister;
typedef struct gxav_fb_info           GxAvIOCTL_FBGetInfo;
typedef struct gxav_sync_params       GxAvIOCTL_SyncParams;

#ifdef __cplusplus
}
#endif

#endif
