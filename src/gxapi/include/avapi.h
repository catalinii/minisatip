#ifndef _AVAPI_H_
#define _AVAPI_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <gxav_common.h>
#include <gxtype.h>
#include <gxcore_version.h>

#define MAX_DECODERS 32

typedef  int GxStatus;

handle_t GxAVCreateDevice   (int chipid);
GxStatus GxAVDestroyDevice  (int dev);

handle_t GxAVOpenModule     (int dev, int module_type, uint32_t id);
GxStatus GxAVCloseModule    (int dev, handle_t module );

GxStatus GxAVSetProperty    (int dev, handle_t module, uint32_t property_id, void *value, uint32_t value_size);
GxStatus GxAVGetProperty    (int dev, handle_t module, uint32_t property_id, void *value, uint32_t value_size);

GxStatus GxAVWaitEvents     (int dev, handle_t module, uint32_t event_mask, int timeout_us, uint32_t *event);
GxStatus GxAVResetEvent     (int dev, handle_t module, uint32_t event_mask );
GxStatus GxAVSetEvent       (int dev, handle_t module, uint32_t event_mask );
GxStatus GxAVLinkFifo       (int dev, handle_t module, handle_t fifo, uint32_t pin_id, GxAvDirection direction);
GxStatus GxAVUnLinkFifo     (int dev, handle_t module, handle_t fifo);

handle_t GxAVFifoCreate     (int dev, uint32_t size, GxAvChannelType type);
GxStatus GxAVFifoDestroy    (int dev, handle_t fifo );
GxStatus GxAVFifoConfig     (int dev, handle_t fifo, GxAvGateInfo *info);
GxStatus GxAVFifoReset      (int dev, handle_t fifo );
GxStatus GxAVFifoRollback   (int dev, handle_t fifo, int size);
int   GxAVFifoGetLength     (int dev, handle_t fifo);
int   GxAVFifoGetFlag       (int dev, handle_t fifo);
int   GxAVFifoGetFreeSize   (int dev, handle_t fifo);
int   GxAVFifoReadData      (int dev, handle_t fifo, void *buffer, int size, int timeout_us);
int   GxAVFifoWriteDataPts  (int dev, handle_t fifo, void *buffer, int size, int timeout_us, int pts);
int   GxAVFifoWriteData     (int dev, handle_t fifo, void *buffer, int size, int timeout_us);
int   GxAVFifoPeekData      (int dev, handle_t fifo, void *buffer, int size, int timeout_us);

int32_t GxKMalloc           (int dev, uint32_t size);
GxStatus GxKFree            (int dev, uint32_t address);

GxStatus GxChipDetect       (int dev);

#ifdef __cplusplus
}
#endif

#endif
