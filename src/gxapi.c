#include "gxapi.h"

int gx_check_ts_lock(adapter *ad) {
	int ret = -1;
	GxDemuxProperty_TSLockQuery ts_lock_status = { TS_SYNC_UNLOCKED };

	ret = GxAVGetProperty(ad->dvr, ad->module, GxDemuxPropertyID_TSLockQuery,
				&ts_lock_status, sizeof(GxDemuxProperty_TSLockQuery));
	if(ret < 0)
	{
		printf("TS: GxDemuxPropertyID_TSLockQuery Problem...\n");
		LOG_AND_RETURN(0, "GXAPI TS: GxDemuxPropertyID_TSLockQuery Problem...");
	}
	return ((ts_lock_status.ts_lock == TS_SYNC_LOCKED) ? 1 : 0);
}

int gx_read_ts(void *buf, int len, sockets *ss)
{
	int aid = ss->sid;
	int ret = 0;
	unsigned int EventRet = 0;
	GxDemuxProperty_FilterRead DmxFilterRead;
	adapter *ad = get_adapter(aid);

	ret = GxAVWaitEvents(ad->dvr, ad->module, EVENT_DEMUX0_FILTRATE_TS_END, 1000000, &EventRet);
	if(ret < 0) {
		LOG("GXAPI TS read: GxAVWaitEvents Problem...");
		return -1;
	}

	DmxFilterRead.filter_id = ad->muxfilter.filter_id;
	DmxFilterRead.buffer    = buf;
	DmxFilterRead.max_size  = len;

	ret = GxAVGetProperty(ad->dvr, ad->module, GxDemuxPropertyID_FilterRead,
			(void*)&DmxFilterRead, sizeof(GxDemuxProperty_FilterRead));
	if(ret < 0)
	{
		LOG("GXAPI TS read: GxDemuxProperty_FilterRead Failed...");
		return -1;
	}

	if(DmxFilterRead.read_size < 0)
		DmxFilterRead.read_size = 0;

	return DmxFilterRead.read_size;
}