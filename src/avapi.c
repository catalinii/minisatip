#include <sys/ioctl.h>
#include <sys/mman.h>

#include <fcntl.h>
/* GXAPI */
#include <avapi.h>
#include <gxav_ioctl.h>
#include <gxcore_os.h>
/* End */
#include <assert.h>

#define CHECK_PARAM(param) if ((param) == NULL) return GXCORE_INVALID_POINTER;

handle_t GxAVCreateDevice(int chipid)
{
	char devname[16];
	int handle;

	if (chipid >= 0)
		sprintf(devname, "/dev/gxav%d", chipid);
	else
		strcpy(devname, "/dev/gxav");

	handle = open(devname, O_RDWR);

	return handle;
}

GxStatus GxAVDestroyDevice(int dev)
{
	return close(dev);
}

handle_t GxAVOpenModule(int dev, int module_type, uint32_t id)
{
	GxAvIOCTL_ModuleOpen param;

	memset(&param, 0, sizeof(param));
	param.module_type = (GxAvModuleType)module_type;
	param.channel_id = id;

	if (ioctl(dev, GXAV_IOCTL_MODULE_OPEN, &param) >= 0)
		return (handle_t)param.module_id;

	return GXCORE_INVALID_POINTER;
}

GxStatus GxAVCloseModule(int dev, handle_t module)
{
	GxAvIOCTL_ModuleClose param;

	memset(&param, 0, sizeof(param));
	param.module_id = module;

	if (ioctl(dev, GXAV_IOCTL_MODULE_CLOSE, &param) >= 0)
		return (GxStatus)param.ret;

	return GXCORE_ERROR;
}

GxStatus GxAVSetProperty(int dev, handle_t module, uint32_t property_id, void *value, uint32_t value_size)
{
	GxAvIOCTL_PropertySet param;

	memset(&param, 0, sizeof(param));

	param.module_id = (unsigned int)module;
	param.prop_id = property_id;
	param.prop_val = value;
	param.prop_size = value_size;

	if (ioctl(dev, GXAV_IOCTL_PROPERTY_SET, &param) >= 0)
		return param.ret;

	return GXCORE_ERROR;
}

GxStatus GxAVGetProperty(int dev, handle_t module, uint32_t property_id, void *value, uint32_t value_size)
{
	GxAvIOCTL_PropertyGet param;

	CHECK_PARAM(value);

	memset(&param, 0, sizeof(param));

	param.module_id = (unsigned int)module;
	param.prop_id = property_id;
	param.prop_val = value;	
	param.prop_size = value_size;

	if (ioctl(dev, GXAV_IOCTL_PROPERTY_GET, &param) >= 0) {
 		memcpy(value, param.prop_val, value_size); 
 		return param.ret;
	}

	return GXCORE_ERROR;
}

GxStatus GxAVWaitEvents (int dev, handle_t module, uint32_t event_mask, int timeout_us, uint32_t *event)
{
	GxAvIOCTL_EventWait param;

	CHECK_PARAM(event);

	memset(&param, 0, sizeof(param));
    
	param.module_id = (unsigned int)module;
	param.event_mask = event_mask;
	param.timeout_us = timeout_us;

	if (ioctl(dev, GXAV_IOCTL_EVENT_WAIT, &param) >= 0) {
		*event = param.ret;
		if (param.ret > 0)
			return GXCORE_SUCCESS;
	}

	return GXCORE_ERROR;
}

GxStatus GxAVResetEvent(int dev, handle_t module, uint32_t event_mask)
{
	GxAvIOCTL_EventReset param;

	memset(&param, 0, sizeof(param));
	param.module_id = (unsigned int)module;
	param.event_mask = event_mask;

	if (ioctl(dev, GXAV_IOCTL_EVENT_RESET, &param) >= 0) {
		if (param.ret != -1)
		return GXCORE_SUCCESS;
	}

	return GXCORE_ERROR;
}

GxStatus GxAVSetEvent(int dev, handle_t module, uint32_t event_mask)
{
	GxAvIOCTL_EventSet param;

	memset(&param, 0, sizeof(param));
	param.module_id = (unsigned int)module;
	param.event_mask = event_mask;

	if (ioctl(dev, GXAV_IOCTL_EVENT_SET, &param) >= 0) {
		if (param.ret != -1)
		return GXCORE_SUCCESS;
	}

	return GXCORE_ERROR;
}

handle_t GxAVFifoCreate(int dev, uint32_t size, GxAvChannelType type)
{
	GxAvIOCTL_FifoCreate param;

	memset(&param, 0, sizeof(param));

	param.data_size = size;
	param.type = type;

	if (ioctl(dev, GXAV_IOCTL_FIFO_CREATE, &param) >= 0) {
		CHECK_PARAM(param.fifo);
		return *((handle_t *)param.fifo);
	}

	return GXCORE_ERROR;
}

GxStatus GxAVFifoDestroy(int dev, handle_t fifo)
{
	GxAvIOCTL_FifoDestroy param;

	CHECK_PARAM(&fifo);

	memset(&param, 0, sizeof(param));

	param.fifo = &fifo;

	if (ioctl(dev, GXAV_IOCTL_FIFO_DESTROY, &param) >= 0) {
		return param.ret;
	}

	return GXCORE_ERROR;
}

GxStatus GxAVFifoConfig(int dev, handle_t fifo, GxAvGateInfo *sdc_info)
{
	GxAvIOCTL_FifoConfig param;

	CHECK_PARAM(&fifo);
	CHECK_PARAM(sdc_info);
	
	memset(&param, 0, sizeof(param));

	param.fifo = &fifo;
	param.sdc_info = sdc_info;
	param.info_size = sizeof(GxAvGateInfo);

	if (ioctl(dev, GXAV_IOCTL_FIFO_CONFIG, &param) >= 0) {
		return param.ret;
	}

	return GXCORE_ERROR;
}

GxStatus GxAVFifoReset(int dev, handle_t fifo)
{
	GxAvIOCTL_FifoReset param;

	CHECK_PARAM(&fifo);

	memset(&param, 0, sizeof(param));

	param.fifo = &fifo;

	if (ioctl(dev, GXAV_IOCTL_FIFO_RESET, &param) >= 0) {
		return param.ret;
	}

	return GXCORE_ERROR;
}

GxStatus GxAVFifoRollback(int dev, handle_t fifo, int size)
{
	GxAvIOCTL_FifoReset param;

	CHECK_PARAM(&fifo);

	memset(&param, 0, sizeof(param));

	param.fifo = &fifo;
	param.data_size = size;

	if (ioctl(dev, GXAV_IOCTL_FIFO_ROLLBACK, &param) >= 0) {
		return param.ret;
	}

	return GXCORE_ERROR;
}


int GxAVFifoGetLength(int dev, handle_t fifo)
{
	GxAvIOCTL_FifoGetLength param;

	CHECK_PARAM(&fifo);

	memset(&param, 0, sizeof(param));

	param.fifo = &fifo;

	if (ioctl(dev, GXAV_IOCTL_FIFO_GET_LENGTH, &param) >= 0) {
		return param.data_size;
	}

	return GXCORE_ERROR;
}

int GxAVFifoGetFlag(int dev, handle_t fifo)
{
	GxAvIOCTL_FifoGetFlag param;

	CHECK_PARAM(&fifo);

	memset(&param, 0, sizeof(param));

	param.fifo = &fifo;

	if (ioctl(dev, GXAV_IOCTL_FIFO_GET_FLAG, &param) >= 0) {
		return param.flag;
	}

	return GXCORE_ERROR;
}

int GxAVFifoGetFreeSize(int dev, handle_t fifo)
{
	GxAvIOCTL_FifoGetFreeSize param;

	CHECK_PARAM(&fifo);

	memset(&param, 0, sizeof(param));

	param.fifo = &fifo;

	if (ioctl(dev, GXAV_IOCTL_FIFO_GET_FREESIZE, &param) >= 0) {
		return param.free_size;
	}

	return GXCORE_ERROR;
}

GxStatus GxAVLinkFifo(int dev, handle_t module, handle_t fifo, uint32_t pin_id, GxAvDirection direction)
{
	GxAvIOCTL_FifoLink param;

	CHECK_PARAM(&fifo);

	memset(&param, 0, sizeof(param));

	param.module_id = (unsigned int)module;
	param.channel   = &fifo;
	param.pin_id    = pin_id;
	param.dir       = direction;
	
	if (ioctl(dev, GXAV_IOCTL_FIFO_LINK, &param) >= 0)
		return GXCORE_SUCCESS;

	return GXCORE_ERROR;
}

GxStatus GxAVUnLinkFifo(int dev, handle_t module, handle_t fifo)
{
	GxAvIOCTL_FifoUnlink param;

	CHECK_PARAM(&fifo);

	memset(&param, 0, sizeof(param));

	param.module_id = (unsigned int)module;
	param.channel   = &fifo;
	
	if (ioctl(dev, GXAV_IOCTL_FIFO_UNLINK, &param) >= 0)
		return GXCORE_SUCCESS;

	return GXCORE_ERROR;
}

int GxAVFifoWriteDataPts(int dev, handle_t fifo, void *buffer, int size, int timeout_us, int pts)
{
	GxAvIOCTL_DataSend param;

	CHECK_PARAM(&fifo);
	CHECK_PARAM(buffer);

	memset(&param, 0, sizeof(param));

	param.fifo         = &fifo;
	param.data_buffer  = (unsigned char *)buffer;
	param.data_size    = size;
	param.timeout_us   = timeout_us;
	param.pts          = pts;

	if (ioctl(dev, GXAV_IOCTL_DATA_SEND, &param) >= 0)
	{
		return (size_t) param.ret;
	}
	return GXCORE_ERROR;
}

int GxAVFifoWriteData(int dev, handle_t fifo, void *buffer, int size, int timeout_us)
{
	return GxAVFifoWriteDataPts(dev, fifo, buffer, size, timeout_us, -1);
}

int GxAVFifoReadData(int dev, handle_t fifo, void *buffer, int size, int timeout_us)
{
	GxAvIOCTL_DataReceive param;

	CHECK_PARAM(&fifo);
	CHECK_PARAM(buffer);

	memset(&param, 0, sizeof(param));

	param.fifo         = &fifo;
	param.data_buffer  = (unsigned char *)buffer;
	param.data_size    = size;
	param.timeout_us   = timeout_us;

	if (ioctl(dev, GXAV_IOCTL_DATA_RECEIVE, &param) >= 0) {
		return (size_t) param.ret;
	}

	return GXCORE_ERROR;
}

int GxAVFifoPeekData(int dev, handle_t fifo, void *buffer, int size, int timeout_us)
{
	GxAvIOCTL_DataPeek param;

	CHECK_PARAM(&fifo);
	CHECK_PARAM(buffer);

	memset(&param, 0, sizeof(param));

	param.fifo         = &fifo;
	param.data_buffer  = (unsigned char *)buffer;
	param.data_size    = size;
	param.timeout_us   = timeout_us;

	if (ioctl(dev, GXAV_IOCTL_DATA_PEEK, &param) >= 0)
		return (size_t) param.ret;

	return GXCORE_ERROR;
}


int32_t GxKMalloc(int dev, uint32_t size)
{
	GxAvIOCTL_KMalloc param;

	memset(&param, 0, sizeof(param));

	param.size = size;

	if (ioctl(dev, GXAV_IOCTL_KMALLOC, &param) >= 0) {
		return param.addr;
	}

	return GXCORE_ERROR;
}

GxStatus GxKFree(int dev, uint32_t address)
{
	GxAvIOCTL_KFree param;

	CHECK_PARAM(&address);

	memset(&param, 0, sizeof(param));

	param.addr = address;

	if (ioctl(dev, GXAV_IOCTL_KFREE, &param) >= 0) {
		return GXCORE_SUCCESS;
	}

	return GXCORE_ERROR;
}

GxStatus GxChipDetect(int dev)
{
	GxAvIOCTL_ChipDetect param;

	memset(&param, 0, sizeof(param));

	if (ioctl(dev, GXAV_IOCTL_CHIP_DETECT, &param) >= 0)
		return param.chip_id;

	return GXCORE_ERROR;
}
