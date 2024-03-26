// SPDX-License-Identifier: GPL-2.0-or-later OR MIT
/*
* Copyright (C) Telechips Inc.
*/

#ifndef DPTX_API_H
#define DPTX_API_H

#include <linux/compat.h>

#define DPTX_API_RETURN_I2C_OVER_AUX_NO_ACK 1000
#define DPTX_API_RETURN_MST_ACT_TIMEOUT			 1001

enum DPTX_LINK_RATE {
	DPTX_LINK_RATE_RBR = 0,
	DPTX_LINK_RATE_HBR = 1,
	DPTX_LINK_RATE_HBR2 = 2,
	DPTX_LINK_RATE_HBR3 = 3,
	DPTX_LINK_RATE_MAX = 4
};

enum DPTX_LINK_LANE {
	DPTX_PHY_LANE_1		= 1,
	DPTX_PHY_LANE_2		= 2,
	DPTX_PHY_LANE_4		= 4,
	DPTX_PHY_LANE_MAX	= 5
};

enum DPTX_PRE_EMPHASIS_LEVEL {
	DPTX_PRE_EMPHASIS_LEVEL_0 = 0,
	DPTX_PRE_EMPHASIS_LEVEL_1 = 1,
	DPTX_PRE_EMPHASIS_LEVEL_2 = 2,
	DPTX_PRE_EMPHASIS_LEVEL_3 = 3,
	DPTX_PRE_EMPHASIS_LEVEL_INVALID = 0xFF
};

enum DPTX_VOLTAGE_SWING_LEVEL {
	DPTX_VOLTAGE_SWING_LEVEL_0 = 0,
	DPTX_VOLTAGE_SWING_LEVEL_1 = 1,
	DPTX_VOLTAGE_SWING_LEVEL_2 = 2,
	DPTX_VOLTAGE_SWING_LEVEL_3 = 3,
	DPTX_VOLTAGE_SWING_LEVEL_INVALID = 0xFF
};

enum DPTX_TRAINING_PATTERN_TYPE {
	DPTX_TRAINING_CTRL_TPS_NONE = 0x00,
	DPTX_TRAINING_CTRL_TPS_1_D102 = 0x01,
	DPTX_TRAINING_CTRL_TPS_2	 = 0x02,
	DPTX_TRAINING_CTRL_TPS_3	 = 0x03,
	DPTX_TRAINING_CTRL_TPS_4	 = 0x04,
	DPTX_TRAINING_CTRL_TPS_SYM_ERM = 0x05,
	DPTX_TRAINING_CTRL_TPS_PRBS7     = 0x06,
	DPTX_TRAINING_CTRL_TPS_CUSTOM80 = 0x07,
	DPTX_TRAINING_CTRL_TPS_CP2520_1 = 0x08,
	DPTX_TRAINING_CTRL_TPS_CP2520_2 = 0x09,
	DPTX_TRAINING_CTRL_TPS_INVALID = 0xFF,
};

enum DPTX_INPUT_STREAM_INDEX {
	DPTX_INPUT_STREAM_0		= 0,
	DPTX_INPUT_STREAM_1		= 1,
	DPTX_INPUT_STREAM_2		= 2,
	DPTX_INPUT_STREAM_3		= 3,
	DPTX_INPUT_STREAM_MAX	= 4
};

enum DPTX_VIDEO_ENCODING_TYPE {
	VIDEO_ENCODING_RGB = 0,
	VIDEO_ENCODING_YCBCR422 = 1,
	VIDEO_ENCODING_YCBCR444 = 2,
	VIDEO_ENCODING_MAX = 3
};

enum DPTX_DTD_FORMAT_TYPE {
	DTD_FORMAT_CEA_861		= 0,/* CEA-861-F */
	DTD_FORMAT_VESA_CVT		= 1,/* VESA CVT */
	DTD_FORMAT_VESA_DMT		= 2 /* VESA DMT */
};

enum DPTX_DTD_REFRESH_RATE {
	DTD_REFRESH_RATE_INVALID	= 0,
	DTD_REFRESH_RATE_49920 = 49920,
	DTD_REFRESH_RATE_50080 = 50080,
	DTD_REFRESH_RATE_59940 = 59940,
	DTD_REFRESH_RATE_60000 = 60000,
	DTD_REFRESH_RATE_60054 = 60054
};

struct dptx_api_init_params {
	bool bsideband_msg;
	bool bsdm_bypass;
	bool btrvc_bypass;
	bool bphy_lane_swap;
	uint8_t ucnum_of_dps;
	uint8_t ucphy_model;
	uint8_t ucmux_id[DPTX_INPUT_STREAM_MAX];
	uint8_t ucvcp_id[DPTX_INPUT_STREAM_MAX];
	uint32_t uiVIC[DPTX_INPUT_STREAM_MAX];
	enum DPTX_LINK_RATE eMaxRate;
	enum DPTX_LINK_LANE eMaxLanes;
	enum DPTX_VIDEO_ENCODING_TYPE eVidEncodingType;
};

struct   DPTX_Dtd_Params_t {
	u8	interlaced;/* 1 : interlaced, 0 : progressive */
	u8	h_sync_polarity;
	u8	v_sync_polarity;
	u16	pixel_repetition_input;
	u16	h_active;
	u16	h_blanking;
	u16	h_image_size;
	u16	h_sync_offset;
	u16	h_sync_pulse_width;
	u16	v_active;
	u16	v_blanking;
	u16	v_image_size;
	u16	v_sync_offset;
	u16	v_sync_pulse_width;
	u32	uiPixel_Clock;
};

int32_t Dpv14_Tx_API_Init(struct dptx_api_init_params *pstdptx_api_init_params);
int32_t Dpv14_Tx_API_Deinit(void);
int32_t Dpv14_Tx_API_Start(uint8_t ucNum_Of_Dps, uint32_t auiPixel_Clk[DPTX_INPUT_STREAM_MAX]);
int32_t Dpv14_Tx_API_Stop(void);
int32_t Dpv14_Tx_API_Set_CfgLock(bool bAccessible);
int32_t Dpv14_Tx_API_Set_CfgAccess(bool bAccessible);
int32_t Dpv14_Tx_API_Set_MaxLinkRate_Supported(enum DPTX_LINK_RATE eLink_Rate);
int32_t Dpv14_Tx_API_Set_MaxLinkLane_Supported(enum DPTX_LINK_LANE eLink_Lane);
int32_t Dpv14_Tx_API_Perform_HPD_WorkingFlow(void);
int32_t Dpv14_Tx_API_Write_RegisterBank_Value(const uint32_t *puiBuffer, uint32_t uiOffset, uint8_t ucLength);
int32_t Dpv14_Tx_API_Read_RegisterBank_Value(uint32_t *puiBuffer, uint32_t uiOffset, uint8_t ucLength);
int32_t Dpv14_Tx_API_Get_PLL_Status(uint8_t *pucPll_Locked);
int32_t Dpv14_Tx_API_Set_CTS(
			bool bSSC_Enabled,
			uint8_t ucClk_Selection,
			uint32_t uiPattern_Type,
			enum DPTX_LINK_RATE eLink_Rate,
			enum DPTX_PRE_EMPHASIS_LEVEL ePreEmp,
			enum DPTX_VOLTAGE_SWING_LEVEL eVSW);
int32_t Dpv14_Tx_API_Set_EQ(uint32_t uiEQ_Main,
			uint32_t uiEQ_Post,
			uint32_t uiEQ_Pre,
			uint32_t uiEQ_VBoost);
int32_t Dpv14_Tx_API_Set_LinkPrams(uint8_t ucParam, uint8_t ucLevel);
int32_t Dpv14_Tx_API_Get_Dtd_From_VideoCode(
			uint32_t uiVideo_Code,
			struct DPTX_Dtd_Params_t *pstDptx_Dtd_Params,
			uint32_t uiRefreshRate,
			uint8_t ucVideoFormat);
int32_t Dpv14_Tx_API_Get_Dtd_From_Edid(
			struct  DPTX_Dtd_Params_t *pstDptx_Dtd_Params,
			uint8_t ucStream_Index);
#endif /* DPTX_API_H  */
