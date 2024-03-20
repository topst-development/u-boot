/*
 * Copyright (c) 2016 Synopsys, Inc.
 *
 * Synopsys DP TX Linux Software Driver and documentation (hereinafter,
 * "Software") is an Unsupported proprietary work of Synopsys, Inc. unless
 * otherwise expressly agreed to in writing between Synopsys and you.
 *
 * The Software IS NOT an item of Licensed Software or Licensed Product under
 * any End User Software License Agreement or Agreement for Licensed Product
 * with Synopsys or any supplement thereto. You are permitted to use and
 * redistribute this Software in source and binary forms, with or without
 * modification, provided that redistributions of source code must retain this
 * notice. You may not view, use, disclose, copy or distribute this file or
 * any information contained herein except pursuant to this license grant from
 * Synopsys. If you do not agree with this notice, including the disclaimer
 * below, then you are not authorized to use the Software.
 *
 * THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS" BASIS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
*/

/*
* Modified by Telechips Inc.
*/

#ifndef __DPTX_V14_H__
#define __DPTX_V14_H__

#include <linux/compat.h>

#define TCC_DPTX_DRV_MAJOR_VER			2
#define TCC_DPTX_DRV_MINOR_VER			6
#define TCC_DPTX_DRV_SUBTITLE_VER		0

#define DP_DDIBUS_BASE_REG_ADDRESS		0x12400000
#define DP_MICOM_BASE_REG_ADDRESS		0x1BD00000
#define DP_HDCP_OFFSET				0x00040000
#define DP_REGISTER_BANK_OFFSET			0x00080000
#define DP_CKC_OFFSET				0x000C0000
#define DP_PROTECT_OFFSET			0x000D0000
#define DP_MAX_OFFSET                           0x01000000

#define DPTX_REGISTER_ACCESS_DDIBUS		true
#define DPTX_REGISTER_ACCESS_MICOM		false

#define DP_CUSTOM_1025_DTD_VIC			1025	/** 1025 : 1024x600 : AV080WSM-NW0*/
#define DP_CUSTOM_1026_DTD_VIC			1026	/** 1026 : 5760x900@54p */
#define DP_CUSTOM_1027_DTD_VIC			1027	/** 1027 : 1920x720 : PVLBJT_020_01 */

#define DPTX_VIC_READ_PANEL_EDID		0

#define DPTX_SINK_CAP_SIZE			0x100
#define DPTX_SDP_NUM				0x10
#define DPTX_SDP_LEN				0x9
#define DPTX_SDP_SIZE				(9 * 4)

#define DPTX_MAX_LINK_LANES			4
#define DPTX_MAX_LINK_SYMBOLS		64
#define DPTX_MAX_LINK_SLOTS			64

#define DP_LINK_STATUS_SIZE	  		6

#define DPTX_EDID_BUFLEN			128
#define DPTX_DEFAULT_VIDEO_CODE			4
#define DPTX_NUM_OF_EQ_G			4

#define DPTX_EDID_MAX_IN_CHANNELS               8

#define DPTX_RETURN_NO_ERROR	0
#define DPTX_RETURN_EPERM		EPERM  /*Operation not permitted*/
#define DPTX_RETURN_ENOENT		ENOENT /*No such file or directory*/
#define DPTX_RETURN_ENOMEM		ENOMEM /*Out of memory*/
#define DPTX_RETURN_EACCES		EACCES /*Permission denied*/
#define DPTX_RETURN_EBUSY		EBUSY  /*Device or resource busy*/
#define DPTX_RETURN_ENODEV		ENODEV /*No such device*/
#define DPTX_RETURN_EINVAL		EINVAL /*Invalid argument*/
#define DPTX_RETURN_ENOSPC		ENOSPC /*No space left on device*/
#define DPTX_RETURN_ESPIPE		ESPIPE /*Illegal seek*/
#define DPTX_RETURN_I2C_OVER_AUX_NO_ACK 1000 /* No ack from I2C Over Aux */
#define DPTX_RETURN_MST_ACT_TIMEOUT			 1001 /* MST Act timeout */


enum MST_INPUT_PORT_TYPE {
	INPUT_PORT_TYPE_TX			= 0,
	INPUT_PORT_TYPE_RX			= 1,
	INPUT_PORT_TYPE_INVALID		= 2
};

enum MST_PEER_DEV_TYPE {
	PEER_NO_DEV_CONNECTED = 0,
	PEER_SOURCE_DEV = 1,
	PEER_BRANCHING_DEV = 2,
	PEER_STREAM_SINK_DEV = 3,
	PEER_DP_TO_LEGECY_CONV = 4,
	PEER_DP_TO_WIRELESS_CONV     = 5,
	PEER_WIRELESS_TO_DP_CONV = 6,
	MST_PEER_DEV_INVALID = 0xFF
};

enum REG_DIV_CFG {
	DIV_CFG_CLK_INVALID = 0,
	DIV_CFG_CLK_200HMZ = 0x83,
	DIV_CFG_CLK_160HMZ = 0xB1,
	DIV_CFG_CLK_100HMZ = 0x87,
	DIV_CFG_CLK_40HMZ = 0x93
};

enum PHY_POWER_STATE {
	PHY_POWER_ON = 0,
	PHY_POWER_DOWN_SWITCHING_RATE = 0x02,
	PHY_POWER_DOWN_PHY_CLOCK = 0x03,
	PHY_POWER_DOWN_REF_CLOCK = 0x0C,
	PHY_POWER_STATE_INVALID = 0xFF
};

enum PHY_RATE {
	RATE_RBR = 0,/* 1.62 Gbs */
	RATE_HBR = 1,/* 2.7 Gbs */
	RATE_HBR2 = 2,/* 5.4 Gbs */
	RATE_HBR3 = 3,/* 8.1 Gbs */
	RATE_MAX = 4
};

enum PHY_LINK_LANE {
	PHY_LANE_0 = 0,
	PHY_LANE_1 = 1,
	PHY_LANE_2 =  2,
	PHY_LANE_4 = 4,
	PHY_LANE_MAX = 5
};

enum PHY_PRE_EMPHASIS_LEVEL {
	PRE_EMPHASIS_LEVEL_0 = 0,
	PRE_EMPHASIS_LEVEL_1 = 1,
	PRE_EMPHASIS_LEVEL_2 = 2,
	PRE_EMPHASIS_LEVEL_3 = 3,
	PRE_EMPHASIS_LEVEL_MAX = 4
};

enum PHY_VOLTAGE_SWING_LEVEL {
	VOLTAGE_SWING_LEVEL_0 = 0,
	VOLTAGE_SWING_LEVEL_1 = 1,
	VOLTAGE_SWING_LEVEL_2 = 2,
	VOLTAGE_SWING_LEVEL_3 = 3,
	VOLTAGE_SWING_LEVEL_MAX = 4
};

enum PHY_INPUT_STREAM_INDEX {
	PHY_INPUT_STREAM_0 = 0,
	PHY_INPUT_STREAM_1 = 1,
	PHY_INPUT_STREAM_2 = 2,
	PHY_INPUT_STREAM_3 = 3,
	PHY_INPUT_STREAM_MAX = 4
};

enum PHY_REF_CLK_SELECTION {
	PHY_REF_CLK_DIRECT_XIN = 0,
	PHY_REF_CLK_PRIVATE_PAD = 4,
	PHY_REF_CLK_DIRECT_XIN_2 = 8,
	PHY_REF_CLK_CKC_OUTPUT = 12,
	PHY_REF_CLK_MAX = 13
};

enum VIDEO_PATTERN_MODE {
	TILE = 0,
	RAMP = 1,
	CHESS = 2,
	COLRAMP = 3,
	VIDEO_PATTERN_INVALID = 0xFF
};

enum VIDEO_RGB_TYPE {
	Legacy_RGB	 = 0, /* ITU 601 */
	S_RGB = 1,/* ITU 709 */
	VIDEO_RGB_TYPE_INVALID = 0xFF
};

enum VIDEO_COLORIMETRY_TYPE {
	ITU601 = 1,
	ITU709 = 2,
	VIDEO_COLORIMETRY_INVALID = 0xFF
};

enum VIDEO_PIXEL_COLOR_DEPTH {
	COLOR_DEPTH_8 = 8,
	COLOR_DEPTH_INVALID = 0xFF
};

enum VIDEO_LINK_BPP {
	VIDEO_LINK_BPP_YCbCr422 = 2,
	VIDEO_LINK_BPP_RGB_YCbCr444 = 3,
	VIDEO_LINK_BPP_INVALID = 0xFF
};

enum VIDEO_SINK_DPCD_BPC {
	VIDEO_SINK_DPCD_8BPC =    0,
	VIDEO_SINK_DPCD_10BPC = 1,
	VIDEO_SINK_DPCD_12BPC = 2,
	VIDEO_SINK_DPCD_16BPC = 3,
	VIDEO_SINK_DPCD_INVALID	= 0xFF
};

enum VIDEO_FORMAT_TYPE {
	CEA_861 = 0,/* CEA-861-F */
	VESA_CVT = 1,/* VESA CVT */
	VESA_DMT = 2,/* VESA DMT */
	VIDEO_FORMAT_INVALID = 0xFF
};

enum VIDEO_BIT_PER_COMPONET {
	BIT_PER_COMPONENT_8BPC	= 8,
	BIT_PER_COMPONENT_10BPC = 10,
	BIT_PER_COMPONENT_12BPC = 12,
	BIT_PER_COMPONENT_16BPC = 16,
	BIT_PER_COMPONENT_INVALID = 0xFF
};

enum AUDIO_INPUT_MUTE {
	AUDIO_INPUT_CLEAR_MUTE_FLAG_VBID = 0,
	AUDIO_INPUT_SET_MUTE_FLAG_VBID = 1
};

enum AUDIO_INPUT_INTERFACE {
	AUDIO_INPUT_INTERFACE_I2S = 0,
	AUDIO_INPUT_INTERFACE_INVALID
};

enum AUDIO_MAX_INPUT_DATA_WIDTH {
	MAX_INPUT_DATA_WIDTH_16BIT = 16,
	MAX_INPUT_DATA_WIDTH_17BIT = 17,
	MAX_INPUT_DATA_WIDTH_18BIT = 18,
	MAX_INPUT_DATA_WIDTH_19BIT = 19,
	MAX_INPUT_DATA_WIDTH_20BIT = 20,
	MAX_INPUT_DATA_WIDTH_21BIT = 21,
	MAX_INPUT_DATA_WIDTH_22BIT = 22,
	MAX_INPUT_DATA_WIDTH_23BIT = 23,
	MAX_INPUT_DATA_WIDTH_24BIT = 24,
	MAX_INPUT_DATA_WIDTH_INVALID = 25
};

enum AUDIO_MAX_GEN_DATA_WIDTH {
	MAX_GEN_DATA_WIDTH_16BIT = 0,
	MAX_GEN_DATA_WIDTH_17BIT = 1,
	MAX_GEN_DATA_WIDTH_18BIT = 2,
	MAX_GEN_DATA_WIDTH_19BIT = 3,
	MAX_GEN_DATA_WIDTH_20BIT = 4,
	MAX_GEN_DATA_WIDTH_21BIT = 5,
	MAX_GEN_DATA_WIDTH_22BIT = 6,
	MAX_GEN_DATA_WIDTH_23BIT = 7,
	MAX_GEN_DATA_WIDTH_24BIT = 8
};

enum PIXEL_ENCODING_TYPE {
	PIXEL_ENCODING_TYPE_RGB = 0,
	PIXEL_ENCODING_TYPE_YCBCR422 = 1,
	PIXEL_ENCODING_TYPE_YCBCR444 = 2,
	PIXEL_ENCODING_TYPE_Invalid
};

enum AUDIO_INPUT_MAX_NUM_OF_CH {
	INPUT_MAX_1_CH = 0,
	INPUT_MAX_2_CH = 1,
	INPUT_MAX_3_CH = 2,
	INPUT_MAX_4_CH = 3,
	INPUT_MAX_5_CH = 4,
	INPUT_MAX_6_CH = 5,
	INPUT_MAX_7_CH = 6,
	INPUT_MAX_8_CH = 7
};

enum AUDIO_EDID_MAX_SAMPLE_FREQ {
	SAMPLE_FREQ_32 = 0,
	SAMPLE_FREQ_44_1 = 1,
	SAMPLE_FREQ_48 = 2,
	SAMPLE_FREQ_88_2 = 3,
	SAMPLE_FREQ_96 = 4,
	SAMPLE_FREQ_176_4 = 5,
	SAMPLE_FREQ_192 = 6,
	SAMPLE_FREQ_INVALID = 7
};

enum AUDIO_IEC60958_3_SAMPLE_FREQ {
	IEC60958_3_SAMPLE_FREQ_44_1 = 0,
	IEC60958_3_SAMPLE_FREQ_88_2 = 1,
	IEC60958_3_SAMPLE_FREQ_22_05 = 2,
	IEC60958_3_SAMPLE_FREQ_176_4 = 3,
	IEC60958_3_SAMPLE_FREQ_48 = 4,
	IEC60958_3_SAMPLE_FREQ_96 = 5,
	IEC60958_3_SAMPLE_FREQ_24 = 6,
	IEC60958_3_SAMPLE_FREQ_192 = 7,
	IEC60958_3_SAMPLE_FREQ_32 = 12,
	IEC60958_3_SAMPLE_FREQ_INVALID = 13
};

enum AUDIO_IEC60958_3_ORIGINAL_SAMPLE_FREQ {
	IEC60958_3_ORIGINAL_SAMPLE_FREQ_16 = 1,
	IEC60958_3_ORIGINAL_SAMPLE_FREQ_32 = 3,
	IEC60958_3_ORIGINAL_SAMPLE_FREQ_12 = 4,
	IEC60958_3_ORIGINAL_SAMPLE_FREQ_11_025 = 5,
	IEC60958_3_ORIGINAL_SAMPLE_FREQ_8 = 6,
	IEC60958_3_ORIGINAL_SAMPLE_FREQ_192 = 8,
	IEC60958_3_ORIGINAL_SAMPLE_FREQ_24 = 9,
	IEC60958_3_ORIGINAL_SAMPLE_FREQ_96 = 10,
	IEC60958_3_ORIGINAL_SAMPLE_FREQ_48 = 11,
	IEC60958_3_ORIGINAL_SAMPLE_FREQ_176_4 = 12,
	IEC60958_3_ORIGINAL_SAMPLE_FREQ_22_05 = 13,
	IEC60958_3_ORIGINAL_SAMPLE_FREQ_88_2 = 14,
	IEC60958_3_ORIGINAL_SAMPLE_FREQ_44_1 = 15
};

enum DMT_ESTABLISHED_TIMING {
	DMT_640x480_60hz = 0,
	DMT_800x600_60hz,
	DMT_1024x768_60hz,
	DMT_NONE
};

enum HPD_Detection_Status {
	HPD_STATUS_UNPLUGGED = 0,
	HPD_STATUS_PLUGGED
};

enum HDCP_Detection_Status {
	HDCP_STATUS_NOT_DETECTED = 0,
	HDCP_STATUS_DETECTED
};

enum AUX_REPLY_Status {
	AUX_REPLY_NOT_RECEIVED = 0,
	AUX_REPLY_RECEIVED
};

struct Dptx_Link_Params {
	bool bTraining_Done;
	uint8_t aucTraining_Status[DP_LINK_STATUS_SIZE];
	uint8_t ucLinkRate;
	uint8_t ucNumOfLanes;
	uint8_t aucPreEmphasis_level[PRE_EMPHASIS_LEVEL_MAX];
	uint8_t aucVoltageSwing_level[VOLTAGE_SWING_LEVEL_MAX];
};

struct Dptx_Aux_Params {
	uint32_t uiAuxStatus;
	uint32_t auiReadData[4];
};

struct Dptx_Dtd_Params {
	uint8_t interlaced;
	uint8_t h_sync_polarity;
	uint8_t v_sync_polarity;
	uint16_t pixel_repetition_input;
	uint16_t h_active;
	uint16_t h_blanking;
	uint16_t h_image_size;
	uint16_t h_sync_offset;
	uint16_t h_sync_pulse_width;
	uint16_t v_active;
	uint16_t v_blanking;
	uint16_t v_image_size;
	uint16_t v_sync_offset;
	uint16_t v_sync_pulse_width;
	uint32_t uiPixel_Clock;
};

struct Dptx_Video_Params {
	uint8_t aucInput_Mute[PHY_INPUT_STREAM_MAX];
	uint8_t ucPixel_Encoding;
	uint8_t ucPattern_Mode;
	uint8_t ucBitPerComponent;
	uint8_t ucRGB_Standard;
	uint8_t ucAverage_BytesPerTu;
	uint8_t ucAver_BytesPer_Tu_Frac;
	uint8_t ucInit_Threshold;
	uint8_t ucVideo_Format;
	uint8_t ucColorimetry;
	uint32_t uiPeri_Pixel_Clock[PHY_INPUT_STREAM_MAX];
	uint32_t auiVideo_Code[PHY_INPUT_STREAM_MAX];
	uint32_t uiRefresh_Rate;

	struct Dptx_Dtd_Params stDtdParams[PHY_INPUT_STREAM_MAX];
};

struct Dptx_EQ_Tuning_Params {
	uint8_t ucTx_EQ_Main;
	uint8_t ucTx_EQ_Post;
	uint8_t ucTx_EQ_Pre;
	uint8_t ucTx_EQ_VBoost;
};

struct Dptx_Params {
	uint32_t uiDPLink_BaseAddr;
	uint32_t uiHDCP22_RegAddr_Offset;
	uint32_t uiRegBank_RegAddr_Offset;
	uint32_t uiCKC_RegAddr_Offset;
	uint32_t uiProtect_RegAddr_Offset;

	bool bSpreadSpectrum_Clock;
	bool bMultStreamTransport;
	bool bEstablish_Timing_Present;
	bool bPanelDisplay_Mode;
	bool bSideBand_MSG_Supported;

	uint8_t ucNumOfStreams;
	uint8_t ucMax_Rate;
	uint8_t ucMax_Lanes;
	uint8_t ucMultiPixel;
	uint8_t aucStreamSink_PortNumber[PHY_INPUT_STREAM_MAX];
	uint8_t aucRAD_PortNumber[PHY_INPUT_STREAM_MAX];
	uint8_t aucVCP_Id[PHY_INPUT_STREAM_MAX];
	uint8_t aucDPCD_Caps[DPTX_SINK_CAP_SIZE];
	uint8_t *pucEdidBuf;
	uint8_t *pucSecondary_EDID;
	uint8_t aucNumOfSlots[PHY_INPUT_STREAM_MAX];
	uint8_t ucPHY_Ref_Clk;
	uint16_t ausPayloadBandwidthNumber[PHY_INPUT_STREAM_MAX];

	enum DMT_ESTABLISHED_TIMING eEstablished_Timing;
	struct Dptx_Video_Params stVideoParams;

	struct Dptx_Aux_Params stAuxParams;
	struct Dptx_Link_Params stDptxLink;
};




int32_t Dptx_Platform_Init(uint8_t ucNumOfStreams, uint8_t ucLink_Rate, uint8_t ucLink_Lanes);
int32_t Dptx_Platform_Deinit(struct Dptx_Params *pstDptx);
int32_t Dptx_Platform_Set_RegisterAccess_Mode(struct Dptx_Params	*pstDptx, bool bAP_Accessable);
int32_t Dptx_Platform_Set_PW(struct Dptx_Params *pstDptx);
int32_t Dptx_Platform_Set_CfgAccess(struct Dptx_Params *pstDptx, bool bAccessable);
int32_t Dptx_Platform_Set_CfgLock(struct Dptx_Params *pstDptx, bool bAccessable);
int32_t Dptx_Platform_Set_RegisterBank(struct Dptx_Params	*pstDptx, enum PHY_RATE eLinkRate);
int32_t Dptx_Platform_Set_PLL_Divisor(struct Dptx_Params	*pstDptx);
int32_t Dptx_Platform_Set_PLL_ClockSource(struct Dptx_Params	*pstDptx, uint8_t ucClockSource);
int32_t Dptx_Platform_Get_PLLLock_Status(struct Dptx_Params	*pstDptx, uint8_t *pucPll_Locked);
int32_t Dptx_Platform_Set_Tx_EQ(struct Dptx_Params *pstDptx, uint32_t uiEQ_Main, uint32_t uiEQ_Post, uint32_t uiEQ_Pre, uint8_t ucEQ_VBoost_7);
void Dptx_Platform_Free_Handle(struct Dptx_Params *pstDptx_Handle);
struct Dptx_Params *Dptx_Platform_Get_Device_Handle(void);



/* Dptx Core */
int32_t Dptx_Core_Link_Power_On(struct Dptx_Params *pstDptx);
int32_t Dptx_Core_Init(struct Dptx_Params *pstDptx);
int32_t Dptx_Core_Deinit(struct Dptx_Params *pstDptx);
void Dptx_Core_Soft_Reset(struct Dptx_Params *pstDptx, uint32_t uiReset_Bits);
void Dptx_Core_Init_PHY(struct Dptx_Params *pstDptx);

int32_t Dptx_Core_Clear_General_Interrupt(struct Dptx_Params *pstDptx, uint32_t uiClear_Bits);
int32_t Dptx_Core_Get_PHY_BUSY_Status(struct Dptx_Params *dptx, uint8_t ucNumOfLanes);
int32_t Dptx_Core_Get_PHY_NumOfLanes(struct Dptx_Params *dptx, uint8_t *pucNumOfLanes);
int32_t Dptx_Core_Get_Sink_SSC_Capability(struct Dptx_Params *dptx, bool *pbSSC_Profiled);
int32_t Dptx_Core_Get_PHY_Rate(struct Dptx_Params *dptx, enum PHY_RATE *pePHY_Rate);
int32_t Dptx_Core_Get_RTL_Configuration_Parameters(struct Dptx_Params *pstDptx);

int32_t Dptx_Core_Set_PHY_PowerState(struct Dptx_Params *pstDptx, enum PHY_POWER_STATE ePowerState);
int32_t Dptx_Core_Set_PHY_NumOfLanes(struct Dptx_Params *pstDptx, uint8_t ucNumOfLanes);
int32_t Dptx_Core_Set_PHY_SSC(struct Dptx_Params *dptx, bool bSink_Supports_SSC);
int32_t Dptx_Core_Set_PHY_Rate(struct Dptx_Params *pstDptx, enum PHY_RATE eRate);
int32_t Dptx_Core_Set_PHY_PreEmphasis(struct Dptx_Params *pstDptx, uint32_t uiLane_Index, enum PHY_PRE_EMPHASIS_LEVEL ePreEmphasisLevel);
int32_t Dptx_Core_Set_PHY_VSW(struct Dptx_Params *pstDptx, uint32_t uiLane_Index, enum PHY_VOLTAGE_SWING_LEVEL eVoltageSwingLevel);
int32_t Dptx_Core_Set_PHY_Pattern(struct Dptx_Params *pstDptx, uint32_t uiPattern);
int32_t Dptx_Core_Enable_PHY_XMIT(struct Dptx_Params *pstDptx, uint32_t iNumOfLanes);
int32_t Dptx_Core_Disable_PHY_XMIT(struct Dptx_Params *pstDptx, uint32_t iNumOfLanes);
int32_t Dptx_Core_PHY_Rate_To_Bandwidth(struct Dptx_Params *pstDptx, uint8_t ucRate, uint8_t *pucBandWidth);
int32_t Dptx_Core_PHY_Bandwidth_To_Rate(struct Dptx_Params *pstDptx, uint8_t ucBandWidth, uint8_t *pucRate);


int32_t Dptx_Avgen_Init(struct Dptx_Params *pstDptx);
int32_t Dptx_Avgen_Set_Video_Code(struct Dptx_Params *pstDptx, uint8_t ucNumOfStreams, uint32_t auiDefaultVideoCode[PHY_INPUT_STREAM_MAX]);
int32_t Dptx_Avgen_Set_Video_PeriPixelClock(struct Dptx_Params *pstDptx, uint8_t ucNumOfStreams, uint32_t auiPeri_PixelClock[PHY_INPUT_STREAM_MAX]);
int32_t Dptx_Avgen_Set_Video_TimingChange(struct Dptx_Params *pstDptx, uint8_t ucStream_Index);
int32_t Dptx_Avgen_Set_Video_TimingChange_FromVIC(struct Dptx_Params *pstDptx, uint32_t uiVideo_Code, uint8_t ucStream_Index);
int32_t Dptx_Avgen_Calculate_Video_Average_TU_Symbols(struct Dptx_Params *pstDptx, int iNumOfLane, int iLinkRate, int iBpc, int iEncodingType, int iPixel_Clock, uint8_t ucStream_Index);
int32_t Dptx_Avgen_Fill_Dtd(struct Dptx_Dtd_Params *pstDtd, uint32_t uiVideo_Code, uint32_t uiRefreshRate, uint8_t ucVideoFormat);
int32_t Dptx_Avgen_Parse_Dtd(struct Dptx_Dtd_Params *pstDtd, uint8_t aucData[18]);
int32_t Dptx_Avgen_Fill_DTD_BasedOn_EST_Timings(struct Dptx_Params *pstDptx, struct Dptx_Dtd_Params *pstDTD);
void Dptx_Avgen_Enable_Video_Stream(struct Dptx_Params *pstDptx, uint8_t ucStream_Index);
void Dptx_Avgen_Disable_Video_Stream(struct Dptx_Params *pstDptx, uint8_t ucStream_Index);
void Dptx_Avgen_Set_Video_Controller_Soft_Reset(struct Dptx_Params *pstDptx, bool bDo_Reset, int uiStream_Index);
void Dptx_Avgen_Disable_Audio_SDP(struct Dptx_Params *pstDptx);
void Dptx_Avgen_Disable_Audio_Timestamp(struct Dptx_Params *dptx);


/* Dptx Link */
int32_t Dptx_Link_Set_MaxLinkRate_Supported(struct Dptx_Params *pstDptx, enum PHY_RATE eLink_Rate);
int32_t Dptx_Link_Set_MaxLinkLane_Supported(struct Dptx_Params *pstDptx, enum PHY_LINK_LANE eLink_Lane);
int32_t Dptx_Link_Perform_Training(struct Dptx_Params *pstDptx, uint8_t ucRate, uint8_t ucNumOfLanes);
int32_t Dptx_link_Get_LinkTraining_Status(struct Dptx_Params *pstDptx);


/* Dptx Interrupt */
int32_t Dptx_Intr_Handle_Hotplug(struct Dptx_Params *pstDptx);
int32_t Dptx_Intr_Handle_HotUnplug(struct Dptx_Params *dptx);
int32_t Dptx_Intr_Handle_Edid(struct Dptx_Params *pstDptx, uint8_t ucStream_Index);
int32_t Dptx_Intr_Get_HotPlug_Status(struct Dptx_Params *pstDptx, bool *pbHotPlug_Status);
int32_t Dptx_Intr_Get_HDCP_Status(struct Dptx_Params *pstDptx, enum HDCP_Detection_Status *peHDCP_Status);


/* Dptx EDID */
int32_t Dptx_Edid_Read_EDID_I2C_Over_Aux(struct Dptx_Params *pstDptx);
int32_t Dptx_Edid_Read_EDID_Over_Sideband_Msg(struct Dptx_Params *pstDptx, uint8_t ucStream_Index);
int32_t Dptx_Edid_Verify_EDID(struct Dptx_Params *pstDptx);
int32_t Dptx_Edid_Check_Detailed_Timing_Descriptors(struct Dptx_Params *pstDptx);


/* Dptx Extension */
int32_t Dptx_Ext_Initiate_MST_Act(struct Dptx_Params *pstDptx);
int32_t Dptx_Ext_Set_SideBand_Msg_Supported(struct Dptx_Params *pstDptx, bool bSideBand_MSG_Supported);
int32_t Dptx_Ext_Set_Stream_Capability(struct Dptx_Params *pstDptx);
int32_t Dptx_Ext_Set_VCPID_MST(struct Dptx_Params *pstDptx, uint8_t ucNumOfStreams, uint8_t aucVCP_Id[PHY_INPUT_STREAM_MAX]);
int32_t Dptx_Ext_Set_Link_VCP_Tables(struct Dptx_Params *pstDptx, uint8_t ucStream_Index);
int32_t Dptx_Ext_Set_Sink_VCP_Table_Slots(struct Dptx_Params *pstDptx, uint8_t ucStream_Index);
int32_t Dptx_Ext_Get_Link_PayloadBandwidthNumber(struct Dptx_Params *pstDptx, uint8_t ucStream_Index);
int32_t Dptx_Ext_Clear_VCP_Tables(struct Dptx_Params *pstDptx);
int32_t Dptx_Ext_Get_Topology(struct Dptx_Params *dptx);
int32_t Dptx_Ext_Remote_I2C_Read(struct Dptx_Params *pstDptx, uint8_t ucStream_Count);


/* Dptx Register */
uint32_t  Dptx_Reg_Readl(struct Dptx_Params *pstDptx, uint32_t uiOffset);
void Dptx_Reg_Writel(struct Dptx_Params *pstDptx, uint32_t uiOffset, uint32_t uiData);
uint32_t Dptx_Reg_Direct_Read(uint32_t uiRegAddr);
void Dptx_Reg_Direct_Write(uint32_t uiRegAddr, uint32_t uiData);


/* Dptx Aux */
int32_t Dptx_Aux_Read_DPCD(struct Dptx_Params *pstDptx, uint32_t uiAddr, uint8_t *pucBuffer);
int32_t Dptx_Aux_Read_Bytes_From_DPCD(struct Dptx_Params *pstDptx, uint32_t uiAddr, uint8_t *pucBuffer, uint32_t len);
int32_t Dptx_Aux_Write_DPCD(struct Dptx_Params *pstDptx, uint32_t uiAddr, uint8_t ucBuffer);
int32_t Dptx_Aux_Write_Bytes_To_DPCD(struct Dptx_Params *pstDptx, uint32_t uiAddr, uint8_t *pucBuffer, uint32_t uiLength);

int32_t Dptx_Aux_Read_Bytes_From_I2C(struct Dptx_Params *pstDptx, uint32_t uiDevice_Addr, uint8_t *pucBuffer, uint32_t uiLength);
int32_t Dptx_Aux_Write_Bytes_To_I2C(struct Dptx_Params *pstDptx, uint32_t uiDevice_Addr, uint8_t *pucBuffer, uint32_t uiLength);
int32_t Dptx_Aux_Write_AddressOnly_To_I2C(struct Dptx_Params *pstDptx, unsigned int uiDevice_Addr);

#endif /* __DPTX_API_H__  */
