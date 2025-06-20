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

#ifndef DPTGX_V14_HEADER
#define DPTGX_V14_HEADER

#include <linux/compat.h>
#include <linux/types.h>
#include <telechips/dpv14_ctrl.h>

#define TCC_DPTX_DRV_MAJOR_VER			3
#define TCC_DPTX_DRV_MINOR_VER			5
#define TCC_DPTX_DRV_SUBTITLE_VER		1

#define DP_DDIBUS_BASE_REG_ADDRESS		0x12400000
#define DP_MICOM_BASE_REG_ADDRESS		0x1BD00000
#define DP_HDCP_OFFSET				0x00040000
#define DP_REGISTER_BANK_OFFSET			0x00080000
#define DP_CKC_OFFSET				0x000C0000
#define DP_PROTECT_OFFSET			0x000D0000
#define DP_SEC_PHY_OFFSET			0x000E0000
#define DP_MAX_OFFSET				0x01000000

#define DPTX_REGISTER_ACCESS_DDIBUS		(bool)true
#define DPTX_REGISTER_ACCESS_MICOM		(bool)false

#define DP_CUSTOM_1025_DTD_VIC			1025	/** 1025 : 1024x600 : AV080WSM-NW0*/
#define DP_CUSTOM_1026_DTD_VIC			1026	/** 1026 : 5760x900@54p */
#define DP_CUSTOM_1027_DTD_VIC			1027	/** 1027 : 1920x720 : PVLBJT_020_01 */
#define DP_CUSTOM_1028_DTD_VIC			1028	/** 1028 : 1920x720 : AUO */

#define DPTX_VIC_READ_PANEL_EDID		0

#define DPTX_SINK_CAP_SIZE			0x100
#define DPTX_SDP_NUM				0x10
#define DPTX_SDP_LEN				0x9
#define DPTX_SDP_SIZE				(9 * 4)

#define DPTX_MAX_LINK_LANES			4
#define DPTX_MAX_LINK_SYMBOLS			64
#define DPTX_MAX_LINK_SLOTS			64u

#define DP_LINK_STATUS_SIZE	  		6

#define EDID_I2C_OVER_AUX_ADDR			0x50
#define EDID_I2C_OVER_AUX_SEGMENT_ADDR		0x30
#define INVALID_MST_PORT_NUM			0xFF
#define DPTX_EDID_BUFLEN			512
#define DPTX_ONE_EDID_BLK_LEN			128

#define DPTX_PIXEL_CLOCK_KHZ_MAX		600000u

#define DPTX_DEFAULT_VIDEO_CODE			4

#define DPTX_RETURN_NO_ERROR 		0
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
#define DPTX_RETURN_MST_ACT_TIMEOUT	1001 /* MST Act timeout */

#define DPTX_RETURN_SUCCESS(ret)	((ret) == DPTX_RETURN_NO_ERROR)
#define DPTX_RETURN_ERROR(ret)		((ret) != DPTX_RETURN_NO_ERROR)

enum PHY_DEVICE_MODEL {
	PHY_DEVICE_SNPS = 0,
	PHY_DEVICE_SEC = 1,
	PHY_DEVICE_UNKNOWN = 2
};

enum PHY_INPUT_STREAM_INDEX {
	PHY_INPUT_STREAM_0 = 0,
	PHY_INPUT_STREAM_1 = 1,
	PHY_INPUT_STREAM_2 = 2,
	PHY_INPUT_STREAM_3 = 3,
	PHY_INPUT_STREAM_MAX = 4
};

#define DIV_CFG_CLK_INVALID 0
#define DIV_CFG_CLK_400HMZ 0x81
#define DIV_CFG_CLK_200HMZ 0x83
#define DIV_CFG_CLK_160HMZ 0xB1
#define DIV_CFG_CLK_100HMZ 0x87

enum PHY_POWER_STATE {
	PHY_POWER_ON = 0,
	PHY_POWER_DOWN_SWITCHING_RATE = 0x02,
	PHY_POWER_DOWN_PHY_CLOCK = 0x03,
	PHY_POWER_DOWN_REF_CLOCK = 0x0C,
	PHY_POWER_STATE_INVALID = 0xFF
};

enum PHY_LINK_RATE {
	LINK_RATE_RBR = 0,
	LINK_RATE_HBR,
	LINK_RATE_HBR2,
	LINK_RATE_HBR3,
	LINK_RATE_MAX
};

enum PHY_LINK_LANE {
	PHY_LANE_0 = 0,
	PHY_LANE_1 = 1,
	PHY_LANE_2 = 2,
	PHY_LANE_3 = 3,
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

enum PHY_REF_CLK_SELECTION {
	PHY_REF_CLK_DIRECT_XIN = 0,
	PHY_REF_CLK_PRIVATE_PAD = 4,
	PHY_REF_CLK_DIRECT_XIN_2 = 8,
	PHY_REF_CLK_CKC_OUTPUT = 12,
	PHY_REF_CLK_MAX = 13
};

enum MST_INPUT_PORT_TYPE {
	INPUT_PORT_TYPE_TX = 0,
	INPUT_PORT_TYPE_RX = 1,
	INPUT_PORT_TYPE_INVALID = 2
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

enum PHY_DATA_WIDTH {
	PHY_DATA_WIDTH_20BITS = 0,
	PHY_DATA_WIDTH_40BITS = 1,
	PHY_DATA_WIDTH_MAX = 2
};

enum VIDEO_PATTERN_MODE {
	TILE = 0,
	RAMP = 1,
	CHESS = 2,
	COLRAMP = 3,
	VIDEO_PATTERN_INVALID = 0xFF
};


enum VIDEO_COLORIMERTY {
	COLORIMETRY_RGB = 0,
	COLORIMETRY_SRGB,
	COLORIMETRY_XR8, /** RGB wide gamut fixed point */
	COLORIMETRY_SCRGB, /** RGB wide gamut floating point */
	COLORIMETRY_Y_ONLY,
	COLORIMETRY_RAW,
	COLORIMETRY_YCBCR_601,
	COLORIMETRY_YCBCR_709,
	COLORIMETRY_ADOBERGB,
	COLORIMETRY_DCI_P3,
	COLORIMETRY_COLOR_PROFILE,
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
	VIDEO_SINK_DPCD_8BPC = 0,
	VIDEO_SINK_DPCD_10BPC = 1,
	VIDEO_SINK_DPCD_12BPC = 2,
	VIDEO_SINK_DPCD_16BPC = 3,
	VIDEO_SINK_DPCD_INVALID	= 0xFF
};

enum VIDEO_FORMAT_STANDARD_TYPE {
	VFS_CEA_861 = 0,/* CEA-861-F */
	VFS_VESA_CVT = 1,/* VESA CVT */
	VFS_VESA_DMT = 2,/* VESA DMT */
	VIDEO_FORMAT_INVALID = 0xFF
};

enum VIDEO_BIT_PER_COMPONET {
	BIT_PER_COMPONENT_8BPC	= 8,
	BIT_PER_COMPONENT_10BPC = 10,
	BIT_PER_COMPONENT_12BPC = 12,
	BIT_PER_COMPONENT_16BPC = 16,
	BIT_PER_COMPONENT_INVALID = 0xFF
};

enum PIXEL_ENCODING_TYPE {
	PIXEL_ENCODING_TYPE_RGB = 0,
	PIXEL_ENCODING_TYPE_YCBCR422 = 1,
	PIXEL_ENCODING_TYPE_YCBCR444 = 2,
	PIXEL_ENCODING_TYPE_Invalid
};

enum DMT_ESTABLISHED_TIMING {
	DMT_640x480_60hz = 0,
	DMT_800x600_60hz,
	DMT_1024x768_60hz,
	DMT_NONE
};

/**
 * @enum hpd_detection_status
 * @brief Defines the status of the Hot Plug Detect (HPD) signal in DisplayPort.
 *
 * This enumeration describes the various states of the HPD signal which is used
 * to indicate the connection status between a DisplayPort source and sink device.
 */
enum hpd_detection_status {
    /**
     * @brief HPD signal is in the UNPLUGGED state.
     *
     * This state indicates that the DisplayPort cable is not connected between
     * the source and the sink. There is no signal, and the connection is
     * completely absent.
     */
    HPD_STATUS_UNPLUGGED = 0,

    /**
     * @brief HPD signal is in the PLUS state.
     *
     * This state represents a transient or unstable connection state where the
     * HPD signal temporarily goes low for a duration of 0.5ms to 1ms or
     * between 2ms and less than 100ms. This typically indicates an IRQ event
     * where a connection is present but experiencing a temporary fluctuation
     * or signal interruption, often used to signal changes in configuration
     * or display settings.
     */
    HPD_STATUS_PLUS,

    /**
     * @brief HPD signal is in the PLUGGED state.
     *
     * This state indicates a stable connection where the DisplayPort cable is
     * properly connected and the HPD signal is consistently high, confirming
     * that the source and sink devices are connected and ready for communication.
     */
    HPD_STATUS_PLUGGED
};

enum AUX_REPLY_Status {
	AUX_REPLY_NOT_RECEIVED = 0,
	AUX_REPLY_RECEIVED
};

struct Dptx_Link_Params {
	uint8_t aucTraining_Status[DP_LINK_STATUS_SIZE];
	uint8_t ucLinkRate;
	uint8_t ucNumOfLanes;
	uint8_t aucPreEmphasis_level[DPTX_MAX_LINK_LANES];
	uint8_t aucVoltageSwing_level[DPTX_MAX_LINK_LANES];
	bool ssc_en;
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

struct dptx_video_params {
	enum PIXEL_ENCODING_TYPE pixel_encoding;
	uint8_t bit_per_component;
	uint8_t average_bytes_per_tu;
	uint8_t average_bytes_per_tu_frac;
	uint8_t fifo_threshold;
	enum VIDEO_FORMAT_STANDARD_TYPE video_format_standard;
	enum VIDEO_COLORIMERTY colorimetry_format;
	uint32_t pixel_clock;
	uint32_t video_code;
	uint32_t video_refresh_rate;

	struct Dptx_Dtd_Params dtd_param;
};

struct Dptx_EQ_Tuning_Params {
	uint8_t ucTx_EQ_Main;
	uint8_t ucTx_EQ_Post;
	uint8_t ucTx_EQ_Pre;
	uint8_t ucTx_EQ_VBoost;
};

#if defined(CONFIG_TCC807X)
#define DPTX_EQ_MAIN_MAX 13u
#define DPTX_EQ_POST_MAX 13u
#else
#define DPTX_EQ_MAIN_MAX 31u
#define DPTX_EQ_POST_MAX 31u
#endif

struct Dptx_Params {
	void *pvDPLink_BaseAddr;
	uint32_t uiHDCP22_RegAddr_Offset;
	uint32_t uiRegBank_RegAddr_Offset;
	uint32_t uiCKC_RegAddr_Offset;
	uint32_t uiProtect_RegAddr_Offset;
	uint32_t uiSEC_PHY_Reg_Offset;

	bool bMultStreamTransport;
	bool bEstablish_Timing_Present;
	bool bSdm_Bypass;
	bool bTrvc_Bypass;
	bool bPhy_Lane_Std;

	uint8_t ucNumOfStreams;
	uint8_t ucMax_Rate;
	uint8_t ucMax_Lanes;
	uint8_t aucStreamSink_PortNumber[PHY_INPUT_STREAM_MAX];
	uint8_t aucRAD_PortNumber[PHY_INPUT_STREAM_MAX];
	uint8_t aucVCP_Id[PHY_INPUT_STREAM_MAX];
	uint8_t aucMuxId[PHY_INPUT_STREAM_MAX];
	uint8_t aucDPCD_Caps[DPTX_SINK_CAP_SIZE];
	uint8_t *pucEdidBuf;
	uint8_t aucNumOfSlots[PHY_INPUT_STREAM_MAX];
	uint8_t ucPHY_Ref_Clk;
	uint16_t ausPayloadBandwidthNumber[PHY_INPUT_STREAM_MAX];
	uint32_t auiVIC[PHY_INPUT_STREAM_MAX];

	enum PHY_DEVICE_MODEL ePhy_Dev;
	enum DMT_ESTABLISHED_TIMING eEstablished_Timing;
	struct dptx_video_params video_params[PHY_INPUT_STREAM_MAX];

	struct Dptx_Aux_Params stAuxParams;
	struct Dptx_Link_Params stDptxLink;

	struct dptx_hw_config hw_config;
};


int32_t Dptx_V14_Init(const struct dpv14_drv_params *drv_params);
int32_t Dptx_V14_Deinit(struct Dptx_Params *pstDptx);
void Dptx_V14_Free_Handle(struct Dptx_Params *pstDptx_Handle);
struct Dptx_Params *Dptx_V14_Get_Device_Handle(void);


int32_t Dptx_Protect_Set_PW(struct Dptx_Params *pstDptx);
int32_t Dptx_Protect_Set_CfgAccess(struct Dptx_Params *pstDptx, bool bAccessible);
int32_t Dptx_Protect_Set_CfgLock(struct Dptx_Params *pstDptx, bool bLocked);

void Dptx_Clk_Reset_PLL(struct Dptx_Params *pstDptx);
void Dptx_Clk_Set_PLL_Divisor(struct Dptx_Params *pstDptx);
void Dptx_Clk_Set_PLL_ClkSrc(struct Dptx_Params *pstDptx, uint8_t ucClockSource);
void Dptx_Clk_Get_PLLLock_Status(struct Dptx_Params *pstDptx, uint8_t *pucPll_Locked);

int32_t dptx_cfg_set_mux_bypass(struct Dptx_Params *pstDptx);
int32_t Dptx_Cfg_Init(struct Dptx_Params *pstDptx, uint8_t ucLinkRate);
int32_t dptx_cfg_reset_phy(struct Dptx_Params *pstDptx, uint8_t link_rate, uint8_t link_lanes);
int32_t Dptx_Cfg_SoftReset(struct Dptx_Params *pstDptx, uint32_t uiVal);

uint32_t dptx_cfg_get_main_equalization(struct Dptx_Params *dev_param, uint8_t link_rate);
uint32_t dptx_cfg_get_post_equalization(struct Dptx_Params *dev_param, uint8_t link_rate);
int32_t dptx_cfg_set_main_equalization(struct Dptx_Params *dev_param, uint8_t link_rate, uint32_t eq_val);
int32_t dptx_cfg_set_post_equalization(struct Dptx_Params *dev_param, uint8_t link_rate, uint32_t eq_val);
int32_t dptx_cfg_set_manual_phy_signal_quality(struct Dptx_Params *dev_param,
					      uint8_t link_rate,
					      enum PHY_VOLTAGE_SWING_LEVEL voltage_swing,
					      enum PHY_PRE_EMPHASIS_LEVEL pre_emphasis);

int32_t Dptx_Cfg_Set_PHY_Cfg(struct Dptx_Params *pstDptx, uint32_t uiCfg_Val);
int32_t Dptx_Cfg_Check_PHY_Pll(struct Dptx_Params *pstDptx, bool *pbPll_Ready);
int32_t Dptx_Cfg_Check_PHY_Ready(struct Dptx_Params *pstDptx, bool *pbPhy_Ready);
int32_t Dptx_Cfg_Check_Sec_PHY_SB_Ready(struct Dptx_Params *pstDptx, bool *pbSb_Ready);

int32_t Dptx_Sec_PHY_Init(struct Dptx_Params *pstDptx, uint8_t ucLinkRate, uint8_t ucLanes);
int32_t Dptx_Sec_Set_NumOfLanes(struct Dptx_Params *pstDptx, uint8_t ucLanes);

uint32_t dptx_sec_get_main_equalization(struct Dptx_Params *dptx_params, uint8_t lane_idx);
int32_t dptx_sec_set_main_equalization(struct Dptx_Params *dptx_params, uint32_t lane_idx, uint32_t eq_val);
uint32_t dptx_sec_get_post_equalization(struct Dptx_Params *dptx_params, uint8_t lane_idx);
int32_t dptx_sec_set_post_equalization(struct Dptx_Params *dptx_params, uint32_t lane_idx, uint32_t eq_val);
int32_t dptx_sec_set_phy_sigan_quality(struct Dptx_Params *dev_param,
				       uint32_t lane_idx,
				       uint8_t link_rate,
				       enum PHY_VOLTAGE_SWING_LEVEL voltage_swing,
				       enum PHY_PRE_EMPHASIS_LEVEL pre_emphasis);

/* Dptx Core */
int32_t Dptx_Core_Init(struct Dptx_Params *dev_param);
int32_t dptx_core_phy_init(struct Dptx_Params *dev_param);
int32_t Dptx_Core_Deinit(struct Dptx_Params *dev_param);
void Dptx_Core_Soft_Reset(struct Dptx_Params *dev_param, uint32_t uiReset_Bits);
int32_t Dptx_Core_Clear_General_Interrupt(struct Dptx_Params *dev_param, uint32_t uiClear_Bits);
int32_t dptx_core_check_ssc_enable(struct Dptx_Params *dev_param);
int32_t dptx_core_set_phy_spread_specturm_clock(struct Dptx_Params *dev_param);
bool dptx_core_get_phy_spread_specturm_clock(struct Dptx_Params *dev_param);
int32_t Dptx_Core_Get_PHY_NumOfLanes(struct Dptx_Params *dev_param, uint8_t *p_num_of_lanes);
int32_t Dptx_Core_Get_PHY_Rate(struct Dptx_Params *dev_param, uint8_t *pucPHY_Rate);
int32_t Dptx_Core_Get_PHY_BUSY_Status(struct Dptx_Params *dev_param, uint8_t num_of_lanes);
int32_t Dptx_Core_Set_PHY_PowerState(struct Dptx_Params *dev_param, enum PHY_POWER_STATE power_state);
int32_t Dptx_Core_Set_PHY_NumOfLanes(struct Dptx_Params *dev_param, uint8_t num_of_lanes);
int32_t Dptx_Core_Set_PHY_Rate(struct Dptx_Params *dev_param, enum PHY_LINK_RATE eRate);
int32_t Dptx_Core_Set_PHY_Pattern(struct Dptx_Params *dev_param, uint32_t uiPattern);
int32_t dptx_core_set_phy_lane_sigan_quality(struct Dptx_Params *dev_param,
					     uint32_t lane_idx, uint8_t link_rate,
					     enum PHY_VOLTAGE_SWING_LEVEL voltage_swing,
					     enum PHY_PRE_EMPHASIS_LEVEL pre_emphasis);
int32_t dptx_core_set_phy_sigan_quality(struct Dptx_Params *dptx_param);
int32_t Dptx_Core_Enable_PHY_XMIT(struct Dptx_Params *dev_param, uint32_t num_of_lanes);
int32_t Dptx_Core_Disable_PHY_XMIT(struct Dptx_Params *dev_param, uint32_t num_of_lanes);


/* Video stream */
int32_t Dptx_Vidin_Init(struct Dptx_Params *dev_param);
int32_t Dptx_Vidin_Set_Video_PPClk(struct Dptx_Params *dev_param,
				   uint8_t num_of_streams,
				   const uint32_t pixel_clocks[PHY_INPUT_STREAM_MAX]);
int32_t Dptx_Vidin_Set_Video_TimingChange(struct Dptx_Params *dev_param, uint8_t dp_stream_id);
int32_t Dptx_Vidin_Set_Video_TimingChange_FromVIC(struct Dptx_Params *dev_param,
						  uint32_t video_code, uint8_t dp_stream_id);
int32_t dptx_vidin_calculate_average_tu_symbols(struct Dptx_Params *dev_param,
						uint8_t dp_stream_id);
int32_t Dptx_Vidin_Fill_Dtd(struct Dptx_Dtd_Params *dtd_param,
			    uint32_t video_code, uint32_t uiRefreshRate,
			    enum VIDEO_FORMAT_STANDARD_TYPE  video_format_standard);
int32_t Dptx_Vidin_Parse_Dtd(struct Dptx_Dtd_Params *dtd_param, const uint8_t aucData[18]);
int32_t Dptx_Vidin_Fill_DTD_BasedOn_EST_Timings(struct Dptx_Params *dev_param,
						struct Dptx_Dtd_Params *dtd_param,
						uint32_t dp_stream_id);
void Dptx_Vidin_Enable_Video_Stream(struct Dptx_Params *dev_param, uint8_t dp_stream_id);
void Dptx_Vidin_Disable_Video_Stream(struct Dptx_Params *dev_param, uint8_t dp_stream_id);

/* Dptx Link */
int32_t Dptx_Link_Perform_Training(struct Dptx_Params *pstDptx, uint8_t ucRate, uint8_t ucNumOfLanes);
int32_t Dptx_Link_Bandwidth_To_PHY_Rate(const struct Dptx_Params *pstDptx, uint8_t ucBandWidth, uint8_t *pucRate);
int32_t Dptx_Link_PHY_Rate_To_Bandwidth(const struct Dptx_Params *pstDptx, uint8_t ucRate, uint8_t *pucBandWidth);

/* Dptx Interrupt */
int32_t Dptx_Intr_Handle_Hotplug(struct Dptx_Params *dev_param);
int32_t Dptx_Intr_Handle_HotUnplug(struct Dptx_Params *dev_param);
int32_t Dptx_Intr_Handle_Edid(struct Dptx_Params *dev_param, u8 dp_stream_id);
enum hpd_detection_status dptx_intr_get_hotplug_status(struct Dptx_Params *dev_param);

bool dptx_intr_check_hpd_and_wait_hpd_to_plugged(struct Dptx_Params *dev_param);

/* Dptx EDID */
int32_t Dptx_Edid_Read_EDID_I2C_Over_Aux(struct Dptx_Params *pstDptx);
int32_t Dptx_Edid_Read_EDID_Over_Sideband_Msg(struct Dptx_Params *pstDptx, uint8_t ucStream_Index);
int32_t Dptx_Edid_Check_Detailed_Timing_Descriptors(struct Dptx_Params *pstDptx);
int32_t Dptx_Edid_Verify_EDID(uint8_t *pucEDID_Buf);

/* Dptx Extension */
int32_t Dptx_Ext_Set_Stream_Capability(struct Dptx_Params *pstDptx);
int32_t Dptx_Ext_Set_Topology_Configuration(struct Dptx_Params *dptx);
int32_t Dptx_Ext_Remote_I2C_Read(struct Dptx_Params *pstDptx, uint8_t ucStream_Count);

/* Dptx Register */
uint32_t  Dptx_Reg_Readl(struct Dptx_Params *pstDptx, uint32_t uiOffset);
void Dptx_Reg_Writel(struct Dptx_Params *pstDptx, uint32_t uiOffset, uint32_t uiData);
uint32_t Dptx_Reg_Direct_Read(uint32_t uiRegAddr);
void Dptx_Reg_Direct_Write(uint32_t uiRegAddr, uint32_t uiData);

uint32_t dptx_phy_read(struct Dptx_Params *pstDptx, uint32_t address);
int32_t dptx_phy_write(struct Dptx_Params *pstDptx, uint32_t address,
		       uint32_t phy_data);

/* Dptx Aux */
int32_t Dptx_Aux_Read_DPCD(struct Dptx_Params *pstDptx, uint32_t uiAddr, uint8_t *pucBuffer);
int32_t Dptx_Aux_Read_Bytes_From_DPCD(struct Dptx_Params *pstDptx, uint32_t uiAddr, uint8_t *pucBuffer, uint32_t len);
int32_t Dptx_Aux_Write_DPCD(struct Dptx_Params *pstDptx, uint32_t uiAddr, uint8_t ucBuffer);
int32_t Dptx_Aux_Write_Bytes_To_DPCD(struct Dptx_Params *pstDptx, uint32_t uiAddr, uint8_t *pucBuffer, uint32_t uiLength);

int32_t Dptx_Aux_Read_Bytes_From_I2C(struct Dptx_Params *pstDptx, uint32_t uiDevice_Addr, uint8_t *pucBuffer, uint32_t uiLength);
int32_t Dptx_Aux_Write_Bytes_To_I2C(struct Dptx_Params *pstDptx, uint32_t uiDevice_Addr, uint8_t *pucBuffer, uint32_t uiLength);
int32_t Dptx_Aux_Write_AddressOnly_To_I2C(struct Dptx_Params *pstDptx, uint32_t uiDevice_Addr);

int32_t dptx_power_up_sink_device_and_check_aux_hysteresis(struct Dptx_Params *pstDptx);
#endif /* __DPTX_API_H__  */
