#ifndef _GXAV_MODULE_PROPERTY_H_
#define _GXAV_MODULE_PROPERTY_H_

typedef enum {
	GxAVGenericPropertyID_ClearEventMask      = 1,
	GxAVGenericPropertyID_ModuleLinkChannel   = 2,
	GxAVGenericPropertyID_ModuleUnLinkChannel = 3,
	GxAVGenericPropertyID_GetErrno            = 4
} GxAVGenericPropertyID;

typedef enum {
	GxDemuxPropertyID_Config                  = 1001,

	GxDemuxPropertyID_SlotAlloc               = 1002,
	GxDemuxPropertyID_SlotConfig              = 1003,
	GxDemuxPropertyID_SlotEnable              = 1004,
	GxDemuxPropertyID_SlotDisable             = 1005,
	GxDemuxPropertyID_SlotFree                = 1006,

	GxDemuxPropertyID_FilterAlloc             = 1007,
	GxDemuxPropertyID_FilterConfig            = 1008,
	GxDemuxPropertyID_FilterEnable            = 1009,
	GxDemuxPropertyID_FilterDisable           = 1010,
	GxDemuxPropertyID_FilterFIFOReset         = 1011,
	GxDemuxPropertyID_FilterFIFOQuery         = 1012,
	GxDemuxPropertyID_FilterFree              = 1013,

	GxDemuxPropertyID_CAAlloc                 = 1014,
	GxDemuxPropertyID_CAConfig                = 1015,
	GxDemuxPropertyID_CAFree                  = 1016,

	GxDemuxPropertyID_FilterRead              = 1017,

	GxDemuxPropertyID_Pcr                     = 1018,
	GxDemuxPropertyID_TSLockQuery             = 1019,
	GxDemuxPropertyID_SlotQueryByPid          = 1020,
	GxDemuxPropertyID_ReadStc                 = 1021,
	GxDemuxPropertyID_MTCCAConfig             = 1022,
	GxDemuxPropertyID_FilterFifoFreeSize      = 1023,
} GxDemuxPropertyID;

typedef enum {
	GxVideoDecoderPropertyID_Config           = 2001,
	GxVideoDecoderPropertyID_Run              = 2002,
	GxVideoDecoderPropertyID_Stop             = 2003,
	GxVideoDecoderPropertyID_Pause            = 2004,
	GxVideoDecoderPropertyID_Resume           = 2005,
	GxVideoDecoderPropertyID_Skip             = 2006,
	GxVideoDecoderPropertyID_Speed            = 2007,
	GxVideoDecoderPropertyID_FrameInfo        = 2008,
	GxVideoDecoderPropertyID_Pts              = 2009,
	GxVideoDecoderPropertyID_State            = 2010,
	GxVideoDecoderPropertyID_Refresh          = 2011,
	GxVideoDecoderPropertyID_Update           = 2012,
	GxVideoDecoderPropertyID_Capability       = 2013,
	GxVideoDecoderPropertyID_DecMode          = 2014,
	GxVideoDecoderPropertyID_PtsOffset        = 2015,
	GxVideoDecoderPropertyID_Tolerance        = 2016,
	GxVideoDecoderPropertyID_OneFrameOver     = 2017,
	GxVideoDecoderPropertyID_Probe            = 2018,
	GxVideoDecoderPropertyID_SyncMode         = 2019,
} GxVideoDecoderPropertyID;

typedef enum GxAudioDecoderPropertyID {
	GxAudioDecoderPropertyID_Config           = 3001,
	GxAudioDecoderPropertyID_Run              = 3002,
	GxAudioDecoderPropertyID_Stop             = 3003,
	GxAudioDecoderPropertyID_Pause            = 3004,
	GxAudioDecoderPropertyID_Resume           = 3005,
	GxAudioDecoderPropertyID_State            = 3006,
	GxAudioDecoderPropertyID_Pts              = 3007,
	GxAudioDecoderPropertyID_FrameInfo        = 3008,
	GxAudioDecoderPropertyID_Update           = 3009,
	GxAudioDecoderPropertyID_Capability       = 3010,
	GxAudioDecoderPropertyID_OutInfo          = 3011
} GxAudioDecoderPropertyID;

typedef enum GxAudioOutPropertyID {
	GxAudioOutPropertyID_ConfigSource         = 4001,
	GxAudioOutPropertyID_ConfigPort           = 4002,
	GxAudioOutPropertyID_ConfigSync           = 4003,
	GxAudioOutPropertyID_Run                  = 4004,
	GxAudioOutPropertyID_Stop                 = 4005,
	GxAudioOutPropertyID_Pause                = 4006,
	GxAudioOutPropertyID_Resume               = 4007,
	GxAudioOutPropertyID_Mute                 = 4008,
	GxAudioOutPropertyID_Channel              = 4009,
	GxAudioOutPropertyID_Volume               = 4010,
	GxAudioOutPropertyID_State                = 4011,
	GxAudioOutPropertyID_Update               = 4012,
	GxAudioOutPropertyID_Pts                  = 4013,
	GxAudioOutPropertyID_PcmMix               = 4014,
	GxAudioOutPropertyID_Speed                = 4015
} GxAudioOutPropertyID;

typedef enum GxAudioReceiverPropertyID {
	GxAudioReceiverPropertyID_Select          = 5001,
	GxAudioReceiverPropertyID_Config          = 5002,
	GxAudioReceiverPropertyID_Run             = 5003,
	GxAudioReceiverPropertyID_Stop            = 5004,
	GxAudioReceiverPropertyID_State           = 5005
} GxAudioReceiverPropertyID;

typedef enum {
	/*Layer support SET & GET */
	GxVpuPropertyID_LayerViewport             = 6001,
	GxVpuPropertyID_LayerMainSurface          = 6002,
	GxVpuPropertyID_LayerEnable               = 6003,
	GxVpuPropertyID_LayerAntiFlicker          = 6004,
	GxVpuPropertyID_LayerOnTop                = 6005,
	GxVpuPropertyID_LayerVideoMode            = 6006,
	GxVpuPropertyID_LayerMixConfig            = 6039,

	/*Capture support Get */
	GxVpuPropertyID_LayerCapture              = 6007,

	/*Surface support Get */
	GxVpuPropertyID_CreateSurface             = 6008,
	GxVpuPropertyID_ReadSurface               = 6009,

	GxVpuPropertyID_TurnSurface               = 6010,
	GxVpuPropertyID_ZoomSurface               = 6011,
	/*Surface support Set */
	GxVpuPropertyID_DestroySurface            = 6012,
	/*Surface support Set & Get */
	GxVpuPropertyID_Palette                   = 6013,
	GxVpuPropertyID_Alpha                     = 6014,
	GxVpuPropertyID_ColorFormat               = 6015,
	GxVpuPropertyID_ColorKey                  = 6016,
	GxVpuPropertyID_BackColor                 = 6017,
	/*Surface support Set */
	GxVpuPropertyID_Point                     = 6018,
	GxVpuPropertyID_DrawLine                  = 6019,
	GxVpuPropertyID_Blit                      = 6020,
	GxVpuPropertyID_FillRect                  = 6021,
	GxVpuPropertyID_FillPolygon               = 6022,
	GxVpuPropertyID_BeginUpdate               = 6023,
	GxVpuPropertyID_EndUpdate                 = 6024,
	GxVpuPropertyID_ConvertColor              = 6025,
	/*support Set & Get */
	GxVpuPropertyID_VirtualResolution         = 6026,

	GxVpuPropertyID_VbiCreateBuffer           = 6027,
	GxVpuPropertyID_VbiDestroyBuffer          = 6028,
	GxVpuPropertyID_VbiReadAddress            = 6029,
	GxVpuPropertyID_VbiEnable                 = 6030,

	GxVpuPropertyID_DestroyPalette            = 6031,
	GxVpuPropertyID_SurfaceBindPalette        = 6032,
	GxVpuPropertyID_CreatePalette             = 6033,
	GxVpuPropertyID_RWPalette                 = 6034,
	GxVpuPropertyID_GetEntries                = 6035,
	GxVpuPropertyID_LayerClipport             = 6036,

	GxVpuPropertyID_DfbBlit                   = 6037,


	GxVpuPropertyID_Complet                   = 6038,

	GxVpuPropertyID_RegistSurface             = 6040,
	GxVpuPropertyID_GetIdleSurface            = 6041,
	GxVpuPropertyID_FlipSurface               = 6042,
	GxVpuPropertyID_UnregistSurface           = 6043,

	GxVpuPropertyID_UnsetLayerMainSurface     = 6044,

	GxVpuPropertyID_BatchDfbBlit              = 6045,
	GxVpuPropertyID_SetGAMode                 = 6046,
	GxVpuPropertyID_WaitUpdate                = 6047,
} GxVpuPropertyID;

typedef enum {
	GxVideoOutPropertyID_OutputConfig         = 7001,
	GxVideoOutPropertyID_OutputSelect         = 7002,
	GxVideoOutPropertyID_Resolution           = 7003,
	GxVideoOutPropertyID_Brightness           = 7005,
	GxVideoOutPropertyID_Saturation           = 7006,
	GxVideoOutPropertyID_Contrast             = 7007,
	GxVideoOutPropertyID_AspectRatio          = 7009,
	GxVideoOutPropertyID_TvScreen             = 7010,
	GxVideoOutPropertyID_OutDefault           = 7013,
	GxVideoOutPropertyID_HdmiStatus           = 7014,
	GxVideoOutPropertyID_EdidInfo             = 7015,
	GxVideoOutPropertyID_PowerOff             = 7016,
	GxVideoOutPropertyID_PowerOn              = 7017,
	GxVideoOutPropertyID_EnableHDCP           = 7018,
	GxVideoOutPropertyID_StartHDCP            = 7019,
	GxVideoOutPropertyID_Power                = 7020,
} GxVideoOutPropertyID;

typedef enum {
	GxJPEGPropertyID_Run                      = 8001,
	GxJPEGPropertyID_Stop                     = 8002,
	GxJPEGPropertyID_State                    = 8003,
	GxJPEGPropertyID_Info                     = 8004,
	GxJPEGPropertyID_Create                   = 8005,
	GxJPEGPropertyID_Destroy                  = 8006,
	GxJPEGPropertyID_Update                   = 8007,
	GxJPEGPropertyID_Status                   = 8008,
	GxJPEGPropertyID_Config                   = 8009,
	GxJPEGPropertyID_End                      = 8010,
	GxJPEGPropertyID_Line                     = 8011,
	GxJPEGPropertyID_Goon                     = 8012,
	GxJPEGPropertyID_WriteData                = 8013
} GxJPEGPropertyID;

typedef enum {
	GxSTCPropertyID_Config                    = 10001,
	GxSTCPropertyID_TimeResolution            = 10002,
	GxSTCPropertyID_Time                      = 10003,
	GxSTCPropertyID_Play                      = 10005,
	GxSTCPropertyID_Stop                      = 10006,
	GxSTCPropertyID_Pause                     = 10007,
	GxSTCPropertyID_Resume                    = 10008
} GxSTCPropertyID;


typedef enum {
	GxICAMPropertyID_CardClockDivisor         = 20001,
	GxICAMPropertyID_VccLevel                 = 20002,
	GxICAMPropertyID_Convention               = 20003,
	GxICAMPropertyID_UartBaudRate             = 20004,
	GxICAMPropertyID_VccSwitch                = 20005,
	GxICAMPropertyID_GuardTime                = 20006,
	GxICAMPropertyID_ResetCard                = 20007,
	GxICAMPropertyID_UartStatus               = 20008,
	GxICAMPropertyID_UartCommand              = 20009,
	GxICAMPropertyID_SendAndReceive           = 20010,
	GxICAMPropertyID_Receive                  = 20011,
	GxICAMPropertyID_Abort                    = 20012,
	GxICAMPropertyID_InsertRemove             = 20013,
	GxICAMPropertyID_Event                    = 20014,
	GxICAMPropertyID_ChipProperties           = 20015,
	GxICAMPropertyID_ResponseToChallenge      = 20016,
	GxICAMPropertyID_EncryptData              = 20017,
	GxICAMPropertyID_ReadRegister             = 20018,
	GxICAMPropertyID_WriteRegister            = 20019,
	GxICAMPropertyID_ControlWord              = 20020,
	GxICAMPropertyID_ConfigInfo               = 20021,
	GxICAMPropertyID_ReadOTP                  = 20022,
	GxICAMPropertyID_WriteOTP                 = 20023
} GxICAMPropertyID;

typedef enum {
	GxIEMMPropertyID_Request                  = 30001,
	GxIEMMPropertyID_Stop                     = 30002,
	GxIEMMPropertyID_Run                      = 30003,
	GxIEMMPropertyID_InterruptEvent           = 30004,
	GxIEMMPropertyID_UpdateReadIndex          = 30005,
	GxIEMMPropertyID_ReadDataBuff             = 30006,
	GxIEMMPropertyID_ReadDataList             = 30007
} GxIEMMPropertyID;


typedef enum {
	GxMTCPropertyID_M2M_UpdateKey             = 40001,
	GxMTCPropertyID_M2M_SetParamsAndExecute   = 40002,
	GxMTCPropertyID_M2M_Event                 = 40003,
	GxMTCPropertyID_Config                    = 40004,
	GxMTCPropertyID_Run                       = 40005,
	GxMTCPropertyID_Reset                     = 40006
} GxMTCPropertyID;

typedef enum {
	GxDescramblerPropertyID_Alloc             = 50001,
	GxDescramblerPropertyID_Free              = 50002,
	GxDescramblerPropertyID_Link              = 50003,
	GxDescramblerPropertyID_Status            = 50004
} GxDESCRAMBLERPropertyID;

#endif
