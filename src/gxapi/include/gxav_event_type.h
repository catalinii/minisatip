#ifndef _GXAV_EVENT_TYPE_H_
#define _GXAV_EVENT_TYPE_H_

/* hw demux specific events  0x01~0xff*/
#define EVENT_DEMUX0_FILTRATE_PSI_END            0x00000001
#define EVENT_DEMUX1_FILTRATE_PSI_END            0x00000002
#define EVENT_DEMUX_FILTRATE_OVER_TIME           0x00000004
#define EVENT_DEMUX_FILTRATE_REIN_TIME           0x00000008
#define EVENT_DEMUX_TS1_LOCk                     0x00000010
#define EVENT_DEMUX_TS2_LOCk                     0x00000020
#define EVENT_DEMUX0_FILTRATE_PES_END            0x00000040
#define EVENT_DEMUX1_FILTRATE_PES_END            0x00000080
#define EVENT_DEMUX0_FILTRATE_TS_END             0x00000100
#define EVENT_DEMUX1_FILTRATE_TS_END             0x00000200
#define EVENT_DEMUX0_TS_OVERFLOW                 0x00000400
#define EVENT_DEMUX1_TS_OVERFLOW                 0x00000800

/* audio decoder specific event */
#define EVENT_AUDIO_FRAMEINFO                    0x00000100

/* audio play spdif specific event */
#define EVENT_AUDIOOUT_FINDSYNC                  0x00000001

/* video decoder specific event */
#define EVENT_VIDEO_0_FRAMEINFO                  (1 << 0)
#define EVENT_VIDEO_1_FRAMEINFO                  (1 << 1)
#define EVENT_VIDEO_0_DECOVER                    (1 << 2)
#define EVENT_VIDEO_1_DECOVER                    (1 << 3)
#define EVENT_VIDEO_ONE_FRAME_OVER               (1 << 4)
#define EVENT_VPU_INTERRUPT                      (1 << 5)

/* hdmi event */
#define EVENT_HDMI_HOTPLUG_IN                    0x00000001
#define EVENT_HDMI_HOTPLUG_OUT                   0x00000002
#define EVENT_HDMI_READ_EDID                     0x00000004
#define EVENT_HDMI_HDCP_FAIL                     0x00000008
#define EVENT_HDMI_HDCP_SUCCESS                  0x00000010
#define EVENT_HDMI_START_AUTH                    0x00000020
#define EVENT_HDMI_READ_EDID_ERR                 0x00000040

/* jpeg decoder specific event */
#define EVENT_JPEG_FRAMEINFO                     0x00000001
#define EVENT_JPEG_DECOVER                       0x00000002
#define EVENT_JPEG_UNSUPPORT                     0x00000004
#define EVENT_JPEG_SLICE                         0x00000008

/* sdc specific event */
#define EVENT_FIFO_FULL(i)                       (1UL << i)
#define EVENT_FIFO_EMPTY(i)                      (1UL << (i + 16))

#define EVENT_IEMM_OVERFLOW             (0x00000001)
#define EVENT_IEMM_DATA_ARRIVER         (0x00000002)

#define EVENT_ICAM_CARD_REMOVE          (0x00000001)
#define EVENT_ICAM_CARD_INSERT          (0x00000002)
#define EVENT_ICAM_DATA_SEND            (0x00000004)
#define EVENT_ICAM_DATA_RECE            (0x00000008)
#define EVENT_ICAM_DATA_OVERFLOW        (0x00000010)
#define EVENT_ICAM_DATA_PARITY_ERROR    (0x00000020)
#define EVENT_ICAM_OTHER_RESERV1        (0x00000040)
#define EVENT_ICAM_OTHER_RESERV2        (0x00000080)

#endif /* _GXAV_EVENT_TYPE_H_ */
